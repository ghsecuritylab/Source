/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
/*
 * Copyright (c) 2002 Bob Beck <beck@openbsd.org>
 * Copyright (c) 2002 Theo de Raadt
 * Copyright (c) 2002 Markus Friedl
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <openssl/objects.h>
#include <openssl/engine.h>
#include <openssl/evp.h>
#include <openssl/bn.h>

#if (defined(__unix__) || defined(unix)) && !defined(USG) && \
	(defined(OpenBSD) || defined(__FreeBSD_version))
#include <sys/param.h>
# if (OpenBSD >= 200112) || ((__FreeBSD_version >= 470101 && __FreeBSD_version < 500000) || __FreeBSD_version >= 500041)
#  define HAVE_CRYPTODEV
# endif
# if (OpenBSD >= 200110)
#  define HAVE_SYSLOG_R
# endif
#endif

#ifndef HAVE_CRYPTODEV

void
ENGINE_load_cryptodev(void)
{
	/* This is a NOP on platforms without /dev/crypto */
	return;
}

#else 
 
#include <sys/types.h>
#include <crypto/cryptodev.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <syslog.h>
#include <errno.h>
#include <string.h>

struct dev_crypto_state {
	struct session_op d_sess;
	int d_fd;
};

static u_int32_t cryptodev_asymfeat = 0;

static int get_asym_dev_crypto(void);
static int open_dev_crypto(void);
static int get_dev_crypto(void);
static int cryptodev_max_iv(int cipher);
static int cryptodev_key_length_valid(int cipher, int len);
static int cipher_nid_to_cryptodev(int nid);
static int get_cryptodev_ciphers(const int **cnids);
static int get_cryptodev_digests(const int **cnids);
static int cryptodev_usable_ciphers(const int **nids);
static int cryptodev_usable_digests(const int **nids);
static int cryptodev_cipher(EVP_CIPHER_CTX *ctx, unsigned char *out,
    const unsigned char *in, unsigned int inl);
static int cryptodev_init_key(EVP_CIPHER_CTX *ctx, const unsigned char *key,
    const unsigned char *iv, int enc);
static int cryptodev_cleanup(EVP_CIPHER_CTX *ctx);
static int cryptodev_engine_ciphers(ENGINE *e, const EVP_CIPHER **cipher,
    const int **nids, int nid);
static int cryptodev_engine_digests(ENGINE *e, const EVP_MD **digest,
    const int **nids, int nid);
static int bn2crparam(const BIGNUM *a, struct crparam *crp);
static int crparam2bn(struct crparam *crp, BIGNUM *a);
static void zapparams(struct crypt_kop *kop);
static int cryptodev_asym(struct crypt_kop *kop, int rlen, BIGNUM *r,
    int slen, BIGNUM *s);

static int cryptodev_bn_mod_exp(BIGNUM *r, const BIGNUM *a,
    const BIGNUM *p, const BIGNUM *m, BN_CTX *ctx, BN_MONT_CTX *m_ctx);
static int cryptodev_rsa_nocrt_mod_exp(BIGNUM *r0, const BIGNUM *I,
    RSA *rsa);
static int cryptodev_rsa_mod_exp(BIGNUM *r0, const BIGNUM *I, RSA *rsa, BN_CTX *ctx);
static int cryptodev_dsa_bn_mod_exp(DSA *dsa, BIGNUM *r, BIGNUM *a,
    const BIGNUM *p, const BIGNUM *m, BN_CTX *ctx, BN_MONT_CTX *m_ctx);
static int cryptodev_dsa_dsa_mod_exp(DSA *dsa, BIGNUM *t1, BIGNUM *g,
    BIGNUM *u1, BIGNUM *pub_key, BIGNUM *u2, BIGNUM *p,
    BN_CTX *ctx, BN_MONT_CTX *mont);
static DSA_SIG *cryptodev_dsa_do_sign(const unsigned char *dgst,
    int dlen, DSA *dsa);
static int cryptodev_dsa_verify(const unsigned char *dgst, int dgst_len,
    DSA_SIG *sig, DSA *dsa);
static int cryptodev_mod_exp_dh(const DH *dh, BIGNUM *r, const BIGNUM *a,
    const BIGNUM *p, const BIGNUM *m, BN_CTX *ctx,
    BN_MONT_CTX *m_ctx);
static int cryptodev_dh_compute_key(unsigned char *key,
    const BIGNUM *pub_key, DH *dh);
static int cryptodev_ctrl(ENGINE *e, int cmd, long i, void *p,
    void (*f)());
void ENGINE_load_cryptodev(void);

static const ENGINE_CMD_DEFN cryptodev_defns[] = {
	{ 0, NULL, NULL, 0 }
};

static struct {
	int	id;
	int	nid;
	int	ivmax;
	int	keylen;
} ciphers[] = {
	{ CRYPTO_DES_CBC,		NID_des_cbc,		8,	 8, },
	{ CRYPTO_3DES_CBC,		NID_des_ede3_cbc,	8,	24, },
	{ CRYPTO_AES_CBC,		NID_aes_128_cbc,	16,	16, },
	{ CRYPTO_BLF_CBC,		NID_bf_cbc,		8,	16, },
	{ CRYPTO_CAST_CBC,		NID_cast5_cbc,		8,	16, },
	{ CRYPTO_SKIPJACK_CBC,		NID_undef,		0,	 0, },
	{ 0,				NID_undef,		0,	 0, },
};

static struct {
	int	id;
	int	nid;
} digests[] = {
	{ CRYPTO_SHA1_HMAC,		NID_hmacWithSHA1,	},
	{ CRYPTO_RIPEMD160_HMAC,	NID_ripemd160,		},
	{ CRYPTO_MD5_KPDK,		NID_undef,		},
	{ CRYPTO_SHA1_KPDK,		NID_undef,		},
	{ CRYPTO_MD5,			NID_md5,		},
	{ CRYPTO_SHA1,			NID_undef,		},
	{ 0,				NID_undef,		},
};

/*
 * Return a fd if /dev/crypto seems usable, 0 otherwise.
 */
static int
open_dev_crypto(void)
{
	static int fd = -1;

	if (fd == -1) {
		if ((fd = open("/dev/crypto", O_RDWR, 0)) == -1)
			return (-1);
		/* close on exec */
		if (fcntl(fd, F_SETFD, 1) == -1) {
			close(fd);
			fd = -1;
			return (-1);
		}
	}
	return (fd);
}

static int
get_dev_crypto(void)
{
	int fd, retfd;

	if ((fd = open_dev_crypto()) == -1)
		return (-1);
	if (ioctl(fd, CRIOGET, &retfd) == -1)
		return (-1);

	/* close on exec */
	if (fcntl(retfd, F_SETFD, 1) == -1) {
		close(retfd);
		return (-1);
	}
	return (retfd);
}

/* Caching version for asym operations */
static int
get_asym_dev_crypto(void)
{
	static int fd = -1;

	if (fd == -1)
		fd = get_dev_crypto();
	return fd;
}

/*
 * XXXX this needs to be set for each alg - and determined from
 * a running card.
 */
static int
cryptodev_max_iv(int cipher)
{
	int i;

	for (i = 0; ciphers[i].id; i++)
		if (ciphers[i].id == cipher)
			return (ciphers[i].ivmax);
	return (0);
}

/*
 * XXXX this needs to be set for each alg - and determined from
 * a running card. For now, fake it out - but most of these
 * for real devices should return 1 for the supported key
 * sizes the device can handle.
 */
static int
cryptodev_key_length_valid(int cipher, int len)
{
	int i;

	for (i = 0; ciphers[i].id; i++)
		if (ciphers[i].id == cipher)
			return (ciphers[i].keylen == len);
	return (0);
}

/* convert libcrypto nids to cryptodev */
static int
cipher_nid_to_cryptodev(int nid)
{
	int i;

	for (i = 0; ciphers[i].id; i++)
		if (ciphers[i].nid == nid)
			return (ciphers[i].id);
	return (0);
}

/*
 * Find out what ciphers /dev/crypto will let us have a session for.
 * XXX note, that some of these openssl doesn't deal with yet!
 * returning them here is harmless, as long as we return NULL
 * when asked for a handler in the cryptodev_engine_ciphers routine
 */
static int
get_cryptodev_ciphers(const int **cnids)
{
	static int nids[CRYPTO_ALGORITHM_MAX];
	struct session_op sess;
	int fd, i, count = 0;

	if ((fd = get_dev_crypto()) < 0) {
		*cnids = NULL;
		return (0);
	}
	memset(&sess, 0, sizeof(sess));
	sess.key = (caddr_t)"123456781234567812345678";

	for (i = 0; ciphers[i].id && count < CRYPTO_ALGORITHM_MAX; i++) {
		if (ciphers[i].nid == NID_undef)
			continue;
		sess.cipher = ciphers[i].id;
		sess.keylen = ciphers[i].keylen;
		sess.mac = 0;
		if (ioctl(fd, CIOCGSESSION, &sess) != -1 &&
		    ioctl(fd, CIOCFSESSION, &sess.ses) != -1)
			nids[count++] = ciphers[i].nid;
	}
	close(fd);

	if (count > 0)
		*cnids = nids;
	else
		*cnids = NULL;
	return (count);
}

/*
 * Find out what digests /dev/crypto will let us have a session for.
 * XXX note, that some of these openssl doesn't deal with yet!
 * returning them here is harmless, as long as we return NULL
 * when asked for a handler in the cryptodev_engine_digests routine
 */
static int
get_cryptodev_digests(const int **cnids)
{
	static int nids[CRYPTO_ALGORITHM_MAX];
	struct session_op sess;
	int fd, i, count = 0;

	if ((fd = get_dev_crypto()) < 0) {
		*cnids = NULL;
		return (0);
	}
	memset(&sess, 0, sizeof(sess));
	for (i = 0; digests[i].id && count < CRYPTO_ALGORITHM_MAX; i++) {
		if (digests[i].nid == NID_undef)
			continue;
		sess.mac = digests[i].id;
		sess.cipher = 0;
		if (ioctl(fd, CIOCGSESSION, &sess) != -1 &&
		    ioctl(fd, CIOCFSESSION, &sess.ses) != -1)
			nids[count++] = digests[i].nid;
	}
	close(fd);

	if (count > 0)
		*cnids = nids;
	else
		*cnids = NULL;
	return (count);
}

/*
 * Find the useable ciphers|digests from dev/crypto - this is the first
 * thing called by the engine init crud which determines what it
 * can use for ciphers from this engine. We want to return
 * only what we can do, anythine else is handled by software.
 *
 * If we can't initialize the device to do anything useful for
 * any reason, we want to return a NULL array, and 0 length,
 * which forces everything to be done is software. By putting
 * the initalization of the device in here, we ensure we can
 * use this engine as the default, and if for whatever reason
 * /dev/crypto won't do what we want it will just be done in
 * software
 *
 * This can (should) be greatly expanded to perhaps take into
 * account speed of the device, and what we want to do.
 * (although the disabling of particular alg's could be controlled
 * by the device driver with sysctl's.) - this is where we
 * want most of the decisions made about what we actually want
 * to use from /dev/crypto.
 */
static int
cryptodev_usable_ciphers(const int **nids)
{
	return (get_cryptodev_ciphers(nids));
}

static int
cryptodev_usable_digests(const int **nids)
{
	/*
	 * XXXX just disable all digests for now, because it sucks.
	 * we need a better way to decide this - i.e. I may not
	 * want digests on slow cards like hifn on fast machines,
	 * but might want them on slow or loaded machines, etc.
	 * will also want them when using crypto cards that don't
	 * suck moose gonads - would be nice to be able to decide something
	 * as reasonable default without having hackery that's card dependent.
	 * of course, the default should probably be just do everything,
	 * with perhaps a sysctl to turn algoritms off (or have them off
	 * by default) on cards that generally suck like the hifn.
	 */
	*nids = NULL;
	return (0);
}

static int
cryptodev_cipher(EVP_CIPHER_CTX *ctx, unsigned char *out,
    const unsigned char *in, unsigned int inl)
{
	struct crypt_op cryp;
	struct dev_crypto_state *state = ctx->cipher_data;
	struct session_op *sess = &state->d_sess;
	void *iiv;
	unsigned char save_iv[EVP_MAX_IV_LENGTH];

	if (state->d_fd < 0)
		return (0);
	if (!inl)
		return (1);
	if ((inl % ctx->cipher->block_size) != 0)
		return (0);

	memset(&cryp, 0, sizeof(cryp));

	cryp.ses = sess->ses;
	cryp.flags = 0;
	cryp.len = inl;
	cryp.src = (caddr_t) in;
	cryp.dst = (caddr_t) out;
	cryp.mac = 0;

	cryp.op = ctx->encrypt ? COP_ENCRYPT : COP_DECRYPT;

	if (ctx->cipher->iv_len) {
		cryp.iv = (caddr_t) ctx->iv;
		if (!ctx->encrypt) {
			iiv = (void *) in + inl - ctx->cipher->iv_len;
			memcpy(save_iv, iiv, ctx->cipher->iv_len);
		}
	} else
		cryp.iv = NULL;

	if (ioctl(state->d_fd, CIOCCRYPT, &cryp) == -1) {
		/* XXX need better errror handling
		 * this can fail for a number of different reasons.
		 */
		return (0);
	}

	if (ctx->cipher->iv_len) {
		if (ctx->encrypt)
			iiv = (void *) out + inl - ctx->cipher->iv_len;
		else
			iiv = save_iv;
		memcpy(ctx->iv, iiv, ctx->cipher->iv_len);
	}
	return (1);
}

static int
cryptodev_init_key(EVP_CIPHER_CTX *ctx, const unsigned char *key,
    const unsigned char *iv, int enc)
{
	struct dev_crypto_state *state = ctx->cipher_data;
	struct session_op *sess = &state->d_sess;
	int cipher;

	if ((cipher = cipher_nid_to_cryptodev(ctx->cipher->nid)) == NID_undef)
		return (0);

	if (ctx->cipher->iv_len > cryptodev_max_iv(cipher))
		return (0);

	if (!cryptodev_key_length_valid(cipher, ctx->key_len))
		return (0);

	memset(sess, 0, sizeof(struct session_op));

	if ((state->d_fd = get_dev_crypto()) < 0)
		return (0);

	sess->key = (unsigned char *)key;
	sess->keylen = ctx->key_len;
	sess->cipher = cipher;

	if (ioctl(state->d_fd, CIOCGSESSION, sess) == -1) {
		close(state->d_fd);
		state->d_fd = -1;
		return (0);
	}
	return (1);
}

/*
 * free anything we allocated earlier when initting a
 * session, and close the session.
 */
static int
cryptodev_cleanup(EVP_CIPHER_CTX *ctx)
{
	int ret = 0;
	struct dev_crypto_state *state = ctx->cipher_data;
	struct session_op *sess = &state->d_sess;

	if (state->d_fd < 0)
		return (0);

	/* XXX if this ioctl fails, someting's wrong. the invoker
	 * may have called us with a bogus ctx, or we could
	 * have a device that for whatever reason just doesn't
	 * want to play ball - it's not clear what's right
	 * here - should this be an error? should it just
	 * increase a counter, hmm. For right now, we return
	 * 0 - I don't believe that to be "right". we could
	 * call the gorpy openssl lib error handlers that
	 * print messages to users of the library. hmm..
	 */

	if (ioctl(state->d_fd, CIOCFSESSION, &sess->ses) == -1) {
		ret = 0;
	} else {
		ret = 1;
	}
	close(state->d_fd);
	state->d_fd = -1;

	return (ret);
}

/*
 * libcrypto EVP stuff - this is how we get wired to EVP so the engine
 * gets called when libcrypto requests a cipher NID.
 */

/* DES CBC EVP */
const EVP_CIPHER cryptodev_des_cbc = {
	NID_des_cbc,
	8, 8, 8,
	EVP_CIPH_CBC_MODE,
	cryptodev_init_key,
	cryptodev_cipher,
	cryptodev_cleanup,
	sizeof(struct dev_crypto_state),
	EVP_CIPHER_set_asn1_iv,
	EVP_CIPHER_get_asn1_iv,
	NULL
};

/* 3DES CBC EVP */
const EVP_CIPHER cryptodev_3des_cbc = {
	NID_des_ede3_cbc,
	8, 24, 8,
	EVP_CIPH_CBC_MODE,
	cryptodev_init_key,
	cryptodev_cipher,
	cryptodev_cleanup,
	sizeof(struct dev_crypto_state),
	EVP_CIPHER_set_asn1_iv,
	EVP_CIPHER_get_asn1_iv,
	NULL
};

const EVP_CIPHER cryptodev_bf_cbc = {
	NID_bf_cbc,
	8, 16, 8,
	EVP_CIPH_CBC_MODE,
	cryptodev_init_key,
	cryptodev_cipher,
	cryptodev_cleanup,
	sizeof(struct dev_crypto_state),
	EVP_CIPHER_set_asn1_iv,
	EVP_CIPHER_get_asn1_iv,
	NULL
};

const EVP_CIPHER cryptodev_cast_cbc = {
	NID_cast5_cbc,
	8, 16, 8,
	EVP_CIPH_CBC_MODE,
	cryptodev_init_key,
	cryptodev_cipher,
	cryptodev_cleanup,
	sizeof(struct dev_crypto_state),
	EVP_CIPHER_set_asn1_iv,
	EVP_CIPHER_get_asn1_iv,
	NULL
};

const EVP_CIPHER cryptodev_aes_cbc = {
	NID_aes_128_cbc,
	16, 16, 16,
	EVP_CIPH_CBC_MODE,
	cryptodev_init_key,
	cryptodev_cipher,
	cryptodev_cleanup,
	sizeof(struct dev_crypto_state),
	EVP_CIPHER_set_asn1_iv,
	EVP_CIPHER_get_asn1_iv,
	NULL
};

/*
 * Registered by the ENGINE when used to find out how to deal with
 * a particular NID in the ENGINE. this says what we'll do at the
 * top level - note, that list is restricted by what we answer with
 */
static int
cryptodev_engine_ciphers(ENGINE *e, const EVP_CIPHER **cipher,
    const int **nids, int nid)
{
	if (!cipher)
		return (cryptodev_usable_ciphers(nids));

	switch (nid) {
	case NID_des_ede3_cbc:
		*cipher = &cryptodev_3des_cbc;
		break;
	case NID_des_cbc:
		*cipher = &cryptodev_des_cbc;
		break;
	case NID_bf_cbc:
		*cipher = &cryptodev_bf_cbc;
		break;
	case NID_cast5_cbc:
		*cipher = &cryptodev_cast_cbc;
		break;
	case NID_aes_128_cbc:
		*cipher = &cryptodev_aes_cbc;
		break;
	default:
		*cipher = NULL;
		break;
	}
	return (*cipher != NULL);
}

static int
cryptodev_engine_digests(ENGINE *e, const EVP_MD **digest,
    const int **nids, int nid)
{
	if (!digest)
		return (cryptodev_usable_digests(nids));

	switch (nid) {
	case NID_md5:
		*digest = NULL; /* need to make a clean md5 critter */
		break;
	default:
		*digest = NULL;
		break;
	}
	return (*digest != NULL);
}

/*
 * Convert a BIGNUM to the representation that /dev/crypto needs.
 * Upon completion of use, the caller is responsible for freeing
 * crp->crp_p.
 */
static int
bn2crparam(const BIGNUM *a, struct crparam *crp)
{
	int i, j, k;
	ssize_t words, bytes, bits;
	u_char *b;

	crp->crp_p = NULL;
	crp->crp_nbits = 0;

	bits = BN_num_bits(a);
	bytes = (bits + 7) / 8;

	b = malloc(bytes);
	if (b == NULL)
		return (1);

	crp->crp_p = b;
	crp->crp_nbits = bits;

	for (i = 0, j = 0; i < a->top; i++) {
		for (k = 0; k < BN_BITS2 / 8; k++) {
			if ((j + k) >= bytes)
				return (0);
			b[j + k] = a->d[i] >> (k * 8);
		}
		j += BN_BITS2 / 8;
	}
	return (0);
}

/* Convert a /dev/crypto parameter to a BIGNUM */
static int
crparam2bn(struct crparam *crp, BIGNUM *a)
{
	u_int8_t *pd;
	int i, bytes;

	bytes = (crp->crp_nbits + 7) / 8;

	if (bytes == 0)
		return (-1);

	if ((pd = (u_int8_t *) malloc(bytes)) == NULL)
		return (-1);

	for (i = 0; i < bytes; i++)
		pd[i] = crp->crp_p[bytes - i - 1];

	BN_bin2bn(pd, bytes, a);
	free(pd);

	return (0);
}

static void
zapparams(struct crypt_kop *kop)
{
	int i;

	for (i = 0; i <= kop->crk_iparams + kop->crk_oparams; i++) {
		if (kop->crk_param[i].crp_p)
			free(kop->crk_param[i].crp_p);
		kop->crk_param[i].crp_p = NULL;
		kop->crk_param[i].crp_nbits = 0;
	}
}

static int
cryptodev_asym(struct crypt_kop *kop, int rlen, BIGNUM *r, int slen, BIGNUM *s)
{
	int fd, ret = -1;

	if ((fd = get_asym_dev_crypto()) < 0)
		return (ret);

	if (r) {
		kop->crk_param[kop->crk_iparams].crp_p = calloc(rlen, sizeof(char));
		kop->crk_param[kop->crk_iparams].crp_nbits = rlen * 8;
		kop->crk_oparams++;
	}
	if (s) {
		kop->crk_param[kop->crk_iparams+1].crp_p = calloc(slen, sizeof(char));
		kop->crk_param[kop->crk_iparams+1].crp_nbits = slen * 8;
		kop->crk_oparams++;
	}

	if (ioctl(fd, CIOCKEY, kop) == 0) {
		if (r)
			crparam2bn(&kop->crk_param[kop->crk_iparams], r);
		if (s)
			crparam2bn(&kop->crk_param[kop->crk_iparams+1], s);
		ret = 0;
	}

	return (ret);
}

static int
cryptodev_bn_mod_exp(BIGNUM *r, const BIGNUM *a, const BIGNUM *p,
    const BIGNUM *m, BN_CTX *ctx, BN_MONT_CTX *in_mont)
{
	struct crypt_kop kop;
	int ret = 1;

	/* Currently, we know we can do mod exp iff we can do any
	 * asymmetric operations at all.
	 */
	if (cryptodev_asymfeat == 0) {
		ret = BN_mod_exp(r, a, p, m, ctx);
		return (ret);
	}

	memset(&kop, 0, sizeof kop);
	kop.crk_op = CRK_MOD_EXP;

	/* inputs: a^p % m */
	if (bn2crparam(a, &kop.crk_param[0]))
		goto err;
	if (bn2crparam(p, &kop.crk_param[1]))
		goto err;
	if (bn2crparam(m, &kop.crk_param[2]))
		goto err;
	kop.crk_iparams = 3;

	if (cryptodev_asym(&kop, BN_num_bytes(m), r, 0, NULL) == -1) {
		const RSA_METHOD *meth = RSA_PKCS1_SSLeay();
		ret = meth->bn_mod_exp(r, a, p, m, ctx, in_mont);
	}
err:
	zapparams(&kop);
	return (ret);
}

static int
cryptodev_rsa_nocrt_mod_exp(BIGNUM *r0, const BIGNUM *I, RSA *rsa)
{
	int r;
	BN_CTX *ctx;

	ctx = BN_CTX_new();
	r = cryptodev_bn_mod_exp(r0, I, rsa->d, rsa->n, ctx, NULL);
	BN_CTX_free(ctx);
	return (r);
}

static int
cryptodev_rsa_mod_exp(BIGNUM *r0, const BIGNUM *I, RSA *rsa, BN_CTX *ctx)
{
	struct crypt_kop kop;
	int ret = 1;

	if (!rsa->p || !rsa->q || !rsa->dmp1 || !rsa->dmq1 || !rsa->iqmp) {
		/* XXX 0 means failure?? */
		return (0);
	}

	memset(&kop, 0, sizeof kop);
	kop.crk_op = CRK_MOD_EXP_CRT;
	/* inputs: rsa->p rsa->q I rsa->dmp1 rsa->dmq1 rsa->iqmp */
	if (bn2crparam(rsa->p, &kop.crk_param[0]))
		goto err;
	if (bn2crparam(rsa->q, &kop.crk_param[1]))
		goto err;
	if (bn2crparam(I, &kop.crk_param[2]))
		goto err;
	if (bn2crparam(rsa->dmp1, &kop.crk_param[3]))
		goto err;
	if (bn2crparam(rsa->dmq1, &kop.crk_param[4]))
		goto err;
	if (bn2crparam(rsa->iqmp, &kop.crk_param[5]))
		goto err;
	kop.crk_iparams = 6;

	if (cryptodev_asym(&kop, BN_num_bytes(rsa->n), r0, 0, NULL) == -1) {
		const RSA_METHOD *meth = RSA_PKCS1_SSLeay();
		ret = (*meth->rsa_mod_exp)(r0, I, rsa, ctx);
	}
err:
	zapparams(&kop);
	return (ret);
}

static RSA_METHOD cryptodev_rsa = {
	"cryptodev RSA method",
	NULL,				/* rsa_pub_enc */
	NULL,				/* rsa_pub_dec */
	NULL,				/* rsa_priv_enc */
	NULL,				/* rsa_priv_dec */
	NULL,
	NULL,
	NULL,				/* init */
	NULL,				/* finish */
	0,				/* flags */
	NULL,				/* app_data */
	NULL,				/* rsa_sign */
	NULL				/* rsa_verify */
};

static int
cryptodev_dsa_bn_mod_exp(DSA *dsa, BIGNUM *r, BIGNUM *a, const BIGNUM *p,
    const BIGNUM *m, BN_CTX *ctx, BN_MONT_CTX *m_ctx)
{
	return (cryptodev_bn_mod_exp(r, a, p, m, ctx, m_ctx));
}

static int
cryptodev_dsa_dsa_mod_exp(DSA *dsa, BIGNUM *t1, BIGNUM *g,
    BIGNUM *u1, BIGNUM *pub_key, BIGNUM *u2, BIGNUM *p,
    BN_CTX *ctx, BN_MONT_CTX *mont)
{
	BIGNUM t2;
	int ret = 0;

	BN_init(&t2);

	/* v = ( g^u1 * y^u2 mod p ) mod q */
	/* let t1 = g ^ u1 mod p */
	ret = 0;

	if (!dsa->meth->bn_mod_exp(dsa,t1,dsa->g,u1,dsa->p,ctx,mont))
		goto err;

	/* let t2 = y ^ u2 mod p */
	if (!dsa->meth->bn_mod_exp(dsa,&t2,dsa->pub_key,u2,dsa->p,ctx,mont))
		goto err;
	/* let u1 = t1 * t2 mod p */
	if (!BN_mod_mul(u1,t1,&t2,dsa->p,ctx))
		goto err;

	BN_copy(t1,u1);

	ret = 1;
err:
	BN_free(&t2);
	return(ret);
}

static DSA_SIG *
cryptodev_dsa_do_sign(const unsigned char *dgst, int dlen, DSA *dsa)
{
	struct crypt_kop kop;
	BIGNUM *r = NULL, *s = NULL;
	DSA_SIG *dsaret = NULL;

	if ((r = BN_new()) == NULL)
		goto err;
	if ((s = BN_new()) == NULL) {
		BN_free(r);
		goto err;
	}

	memset(&kop, 0, sizeof kop);
	kop.crk_op = CRK_DSA_SIGN;

	/* inputs: dgst dsa->p dsa->q dsa->g dsa->priv_key */
	kop.crk_param[0].crp_p = (caddr_t)dgst;
	kop.crk_param[0].crp_nbits = dlen * 8;
	if (bn2crparam(dsa->p, &kop.crk_param[1]))
		goto err;
	if (bn2crparam(dsa->q, &kop.crk_param[2]))
		goto err;
	if (bn2crparam(dsa->g, &kop.crk_param[3]))
		goto err;
	if (bn2crparam(dsa->priv_key, &kop.crk_param[4]))
		goto err;
	kop.crk_iparams = 5;

	if (cryptodev_asym(&kop, BN_num_bytes(dsa->q), r,
	    BN_num_bytes(dsa->q), s) == 0) {
		dsaret = DSA_SIG_new();
		dsaret->r = r;
		dsaret->s = s;
	} else {
		const DSA_METHOD *meth = DSA_OpenSSL();
		BN_free(r);
		BN_free(s);
		dsaret = (meth->dsa_do_sign)(dgst, dlen, dsa);
	}
err:
	kop.crk_param[0].crp_p = NULL;
	zapparams(&kop);
	return (dsaret);
}

static int
cryptodev_dsa_verify(const unsigned char *dgst, int dlen,
    DSA_SIG *sig, DSA *dsa)
{
	struct crypt_kop kop;
	int dsaret = 1;

	memset(&kop, 0, sizeof kop);
	kop.crk_op = CRK_DSA_VERIFY;

	/* inputs: dgst dsa->p dsa->q dsa->g dsa->pub_key sig->r sig->s */
	kop.crk_param[0].crp_p = (caddr_t)dgst;
	kop.crk_param[0].crp_nbits = dlen * 8;
	if (bn2crparam(dsa->p, &kop.crk_param[1]))
		goto err;
	if (bn2crparam(dsa->q, &kop.crk_param[2]))
		goto err;
	if (bn2crparam(dsa->g, &kop.crk_param[3]))
		goto err;
	if (bn2crparam(dsa->pub_key, &kop.crk_param[4]))
		goto err;
	if (bn2crparam(sig->r, &kop.crk_param[5]))
		goto err;
	if (bn2crparam(sig->s, &kop.crk_param[6]))
		goto err;
	kop.crk_iparams = 7;

	if (cryptodev_asym(&kop, 0, NULL, 0, NULL) == 0) {
		dsaret = kop.crk_status;
	} else {
		const DSA_METHOD *meth = DSA_OpenSSL();

		dsaret = (meth->dsa_do_verify)(dgst, dlen, sig, dsa);
	}
err:
	kop.crk_param[0].crp_p = NULL;
	zapparams(&kop);
	return (dsaret);
}

static DSA_METHOD cryptodev_dsa = {
	"cryptodev DSA method",
	NULL,
	NULL,				/* dsa_sign_setup */
	NULL,
	NULL,				/* dsa_mod_exp */
	NULL,
	NULL,				/* init */
	NULL,				/* finish */
	0,	/* flags */
	NULL	/* app_data */
};

static int
cryptodev_mod_exp_dh(const DH *dh, BIGNUM *r, const BIGNUM *a,
    const BIGNUM *p, const BIGNUM *m, BN_CTX *ctx,
    BN_MONT_CTX *m_ctx)
{
	return (cryptodev_bn_mod_exp(r, a, p, m, ctx, m_ctx));
}

static int
cryptodev_dh_compute_key(unsigned char *key, const BIGNUM *pub_key, DH *dh)
{
	struct crypt_kop kop;
	int dhret = 1;
	int fd, keylen;

	if ((fd = get_asym_dev_crypto()) < 0) {
		const DH_METHOD *meth = DH_OpenSSL();

		return ((meth->compute_key)(key, pub_key, dh));
	}

	keylen = BN_num_bits(dh->p);

	memset(&kop, 0, sizeof kop);
	kop.crk_op = CRK_DH_COMPUTE_KEY;

	/* inputs: dh->priv_key pub_key dh->p key */
	if (bn2crparam(dh->priv_key, &kop.crk_param[0]))
		goto err;
	if (bn2crparam(pub_key, &kop.crk_param[1]))
		goto err;
	if (bn2crparam(dh->p, &kop.crk_param[2]))
		goto err;
	kop.crk_iparams = 3;

	kop.crk_param[3].crp_p = key;
	kop.crk_param[3].crp_nbits = keylen * 8;
	kop.crk_oparams = 1;

	if (ioctl(fd, CIOCKEY, &kop) == -1) {
		const DH_METHOD *meth = DH_OpenSSL();

		dhret = (meth->compute_key)(key, pub_key, dh);
	}
err:
	kop.crk_param[3].crp_p = NULL;
	zapparams(&kop);
	return (dhret);
}

static DH_METHOD cryptodev_dh = {
	"cryptodev DH method",
	NULL,				/* cryptodev_dh_generate_key */
	NULL,
	NULL,
	NULL,
	NULL,
	0,	/* flags */
	NULL	/* app_data */
};

/*
 * ctrl right now is just a wrapper that doesn't do much
 * but I expect we'll want some options soon.
 */
static int
cryptodev_ctrl(ENGINE *e, int cmd, long i, void *p, void (*f)())
{
#ifdef HAVE_SYSLOG_R
	struct syslog_data sd = SYSLOG_DATA_INIT;
#endif

	switch (cmd) {
	default:
#ifdef HAVE_SYSLOG_R
		syslog_r(LOG_ERR, &sd,
		    "cryptodev_ctrl: unknown command %d", cmd);
#else
		syslog(LOG_ERR, "cryptodev_ctrl: unknown command %d", cmd);
#endif
		break;
	}
	return (1);
}

void
ENGINE_load_cryptodev(void)
{
	ENGINE *engine = ENGINE_new();
	int fd;

	if (engine == NULL)
		return;
	if ((fd = get_dev_crypto()) < 0) {
		ENGINE_free(engine);
		return;
	}

	/*
	 * find out what asymmetric crypto algorithms we support
	 */
	if (ioctl(fd, CIOCASYMFEAT, &cryptodev_asymfeat) == -1) {
		close(fd);
		ENGINE_free(engine);
		return;
	}
	close(fd);

	if (!ENGINE_set_id(engine, "cryptodev") ||
	    !ENGINE_set_name(engine, "BSD cryptodev engine") ||
	    !ENGINE_set_ciphers(engine, cryptodev_engine_ciphers) ||
	    !ENGINE_set_digests(engine, cryptodev_engine_digests) ||
	    !ENGINE_set_ctrl_function(engine, cryptodev_ctrl) ||
	    !ENGINE_set_cmd_defns(engine, cryptodev_defns)) {
		ENGINE_free(engine);
		return;
	}

	if (ENGINE_set_RSA(engine, &cryptodev_rsa)) {
		const RSA_METHOD *rsa_meth = RSA_PKCS1_SSLeay();

		cryptodev_rsa.bn_mod_exp = rsa_meth->bn_mod_exp;
		cryptodev_rsa.rsa_mod_exp = rsa_meth->rsa_mod_exp;
		cryptodev_rsa.rsa_pub_enc = rsa_meth->rsa_pub_enc;
		cryptodev_rsa.rsa_pub_dec = rsa_meth->rsa_pub_dec;
		cryptodev_rsa.rsa_priv_enc = rsa_meth->rsa_priv_enc;
		cryptodev_rsa.rsa_priv_dec = rsa_meth->rsa_priv_dec;
		if (cryptodev_asymfeat & CRF_MOD_EXP) {
			cryptodev_rsa.bn_mod_exp = cryptodev_bn_mod_exp;
			if (cryptodev_asymfeat & CRF_MOD_EXP_CRT)
				cryptodev_rsa.rsa_mod_exp =
				    cryptodev_rsa_mod_exp;
			else
				cryptodev_rsa.rsa_mod_exp =
				    cryptodev_rsa_nocrt_mod_exp;
		}
	}

	if (ENGINE_set_DSA(engine, &cryptodev_dsa)) {
		const DSA_METHOD *meth = DSA_OpenSSL();

		memcpy(&cryptodev_dsa, meth, sizeof(DSA_METHOD));
		if (cryptodev_asymfeat & CRF_DSA_SIGN)
			cryptodev_dsa.dsa_do_sign = cryptodev_dsa_do_sign;
		if (cryptodev_asymfeat & CRF_MOD_EXP) {
			cryptodev_dsa.bn_mod_exp = cryptodev_dsa_bn_mod_exp;
			cryptodev_dsa.dsa_mod_exp = cryptodev_dsa_dsa_mod_exp;
		}
		if (cryptodev_asymfeat & CRF_DSA_VERIFY)
			cryptodev_dsa.dsa_do_verify = cryptodev_dsa_verify;
	}

	if (ENGINE_set_DH(engine, &cryptodev_dh)){
		const DH_METHOD *dh_meth = DH_OpenSSL();

		cryptodev_dh.generate_key = dh_meth->generate_key;
		cryptodev_dh.compute_key = dh_meth->compute_key;
		cryptodev_dh.bn_mod_exp = dh_meth->bn_mod_exp;
		if (cryptodev_asymfeat & CRF_MOD_EXP) {
			cryptodev_dh.bn_mod_exp = cryptodev_mod_exp_dh;
			if (cryptodev_asymfeat & CRF_DH_COMPUTE_KEY)
				cryptodev_dh.compute_key =
				    cryptodev_dh_compute_key;
		}
	}

	ENGINE_add(engine);
	ENGINE_free(engine);
	ERR_clear_error();
}

#endif /* HAVE_CRYPTODEV */
