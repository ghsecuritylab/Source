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
/* crypto/rsa/rsa_sign.c */
/* Copyright (C) 1995-1998 Eric Young (eay@cryptsoft.com)
 * All rights reserved.
 *
 * This package is an SSL implementation written
 * by Eric Young (eay@cryptsoft.com).
 * The implementation was written so as to conform with Netscapes SSL.
 * 
 * This library is free for commercial and non-commercial use as long as
 * the following conditions are aheared to.  The following conditions
 * apply to all code found in this distribution, be it the RC4, RSA,
 * lhash, DES, etc., code; not just the SSL code.  The SSL documentation
 * included with this distribution is covered by the same copyright terms
 * except that the holder is Tim Hudson (tjh@cryptsoft.com).
 * 
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.
 * If this package is used in a product, Eric Young should be given attribution
 * as the author of the parts of the library used.
 * This can be in the form of a textual message at program startup or
 * in documentation (online or textual) provided with the package.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    "This product includes cryptographic software written by
 *     Eric Young (eay@cryptsoft.com)"
 *    The word 'cryptographic' can be left out if the rouines from the library
 *    being used are not cryptographic related :-).
 * 4. If you include any Windows specific code (or a derivative thereof) from 
 *    the apps directory (application code) you must include an acknowledgement:
 *    "This product includes software written by Tim Hudson (tjh@cryptsoft.com)"
 * 
 * THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * The licence and distribution terms for any publically available version or
 * derivative of this code cannot be changed.  i.e. this code cannot simply be
 * copied and put under another distribution licence
 * [including the GNU Public Licence.]
 */

#include <stdio.h>
#include "cryptlib.h"
#include <openssl/bn.h>
#include <openssl/rsa.h>
#include <openssl/objects.h>
#include <openssl/x509.h>

/* Size of an SSL signature: MD5+SHA1 */
#define SSL_SIG_LENGTH	36

int RSA_sign(int type, const unsigned char *m, unsigned int m_len,
	     unsigned char *sigret, unsigned int *siglen, RSA *rsa)
	{
	X509_SIG sig;
	ASN1_TYPE parameter;
	int i,j,ret=1;
	unsigned char *p, *tmps = NULL;
	const unsigned char *s = NULL;
	X509_ALGOR algor;
	ASN1_OCTET_STRING digest;
	if((rsa->flags & RSA_FLAG_SIGN_VER) && rsa->meth->rsa_sign)
		{
		return rsa->meth->rsa_sign(type, m, m_len,
			sigret, siglen, rsa);
		}
	/* Special case: SSL signature, just check the length */
	if(type == NID_md5_sha1) {
		if(m_len != SSL_SIG_LENGTH) {
			RSAerr(RSA_F_RSA_SIGN,RSA_R_INVALID_MESSAGE_LENGTH);
			return(0);
		}
		i = SSL_SIG_LENGTH;
		s = m;
	} else {
		sig.algor= &algor;
		sig.algor->algorithm=OBJ_nid2obj(type);
		if (sig.algor->algorithm == NULL)
			{
			RSAerr(RSA_F_RSA_SIGN,RSA_R_UNKNOWN_ALGORITHM_TYPE);
			return(0);
			}
		if (sig.algor->algorithm->length == 0)
			{
			RSAerr(RSA_F_RSA_SIGN,RSA_R_THE_ASN1_OBJECT_IDENTIFIER_IS_NOT_KNOWN_FOR_THIS_MD);
			return(0);
			}
		parameter.type=V_ASN1_NULL;
		parameter.value.ptr=NULL;
		sig.algor->parameter= &parameter;

		sig.digest= &digest;
		sig.digest->data=(unsigned char *)m; /* TMP UGLY CAST */
		sig.digest->length=m_len;

		i=i2d_X509_SIG(&sig,NULL);
	}
	j=RSA_size(rsa);
	if (i > (j-RSA_PKCS1_PADDING_SIZE))
		{
		RSAerr(RSA_F_RSA_SIGN,RSA_R_DIGEST_TOO_BIG_FOR_RSA_KEY);
		return(0);
		}
	if(type != NID_md5_sha1) {
		tmps=(unsigned char *)OPENSSL_malloc((unsigned int)j+1);
		if (tmps == NULL)
			{
			RSAerr(RSA_F_RSA_SIGN,ERR_R_MALLOC_FAILURE);
			return(0);
			}
		p=tmps;
		i2d_X509_SIG(&sig,&p);
		s=tmps;
	}
	i=RSA_private_encrypt(i,s,sigret,rsa,RSA_PKCS1_PADDING);
	if (i <= 0)
		ret=0;
	else
		*siglen=i;

	if(type != NID_md5_sha1) {
		OPENSSL_cleanse(tmps,(unsigned int)j+1);
		OPENSSL_free(tmps);
	}
	return(ret);
	}

int RSA_verify(int dtype, const unsigned char *m, unsigned int m_len,
	     unsigned char *sigbuf, unsigned int siglen, RSA *rsa)
	{
	int i,ret=0,sigtype;
	unsigned char *s;
	X509_SIG *sig=NULL;

	if (siglen != (unsigned int)RSA_size(rsa))
		{
		RSAerr(RSA_F_RSA_VERIFY,RSA_R_WRONG_SIGNATURE_LENGTH);
		return(0);
		}

	if((rsa->flags & RSA_FLAG_SIGN_VER) && rsa->meth->rsa_verify)
		{
		return rsa->meth->rsa_verify(dtype, m, m_len,
			sigbuf, siglen, rsa);
		}

	s=(unsigned char *)OPENSSL_malloc((unsigned int)siglen);
	if (s == NULL)
		{
		RSAerr(RSA_F_RSA_VERIFY,ERR_R_MALLOC_FAILURE);
		goto err;
		}
	if((dtype == NID_md5_sha1) && (m_len != SSL_SIG_LENGTH) ) {
			RSAerr(RSA_F_RSA_VERIFY,RSA_R_INVALID_MESSAGE_LENGTH);
			goto err;
	}
	i=RSA_public_decrypt((int)siglen,sigbuf,s,rsa,RSA_PKCS1_PADDING);

	if (i <= 0) goto err;

	/* Special case: SSL signature */
	if(dtype == NID_md5_sha1) {
		if((i != SSL_SIG_LENGTH) || memcmp(s, m, SSL_SIG_LENGTH))
				RSAerr(RSA_F_RSA_VERIFY,RSA_R_BAD_SIGNATURE);
		else ret = 1;
	} else {
		const unsigned char *p=s;
		sig=d2i_X509_SIG(NULL,&p,(long)i);

		if (sig == NULL) goto err;

		/* Excess data can be used to create forgeries */
		if(p != s+i)
			{
			RSAerr(RSA_F_RSA_VERIFY,RSA_R_BAD_SIGNATURE);
			goto err;
			}

		/* Parameters to the signature algorithm can also be used to
		   create forgeries */
		if(sig->algor->parameter
		   && ASN1_TYPE_get(sig->algor->parameter) != V_ASN1_NULL)
			{
			RSAerr(RSA_F_RSA_VERIFY,RSA_R_BAD_SIGNATURE);
			goto err;
			}

		sigtype=OBJ_obj2nid(sig->algor->algorithm);


	#ifdef RSA_DEBUG
		/* put a backward compatibility flag in EAY */
		fprintf(stderr,"in(%s) expect(%s)\n",OBJ_nid2ln(sigtype),
			OBJ_nid2ln(dtype));
	#endif
		if (sigtype != dtype)
			{
			if (((dtype == NID_md5) &&
				(sigtype == NID_md5WithRSAEncryption)) ||
				((dtype == NID_md2) &&
				(sigtype == NID_md2WithRSAEncryption)))
				{
				/* ok, we will let it through */
#if !defined(OPENSSL_NO_STDIO) && !defined(OPENSSL_SYS_WIN16)
				fprintf(stderr,"signature has problems, re-make with post SSLeay045\n");
#endif
				}
			else
				{
				RSAerr(RSA_F_RSA_VERIFY,
						RSA_R_ALGORITHM_MISMATCH);
				goto err;
				}
			}
		if (	((unsigned int)sig->digest->length != m_len) ||
			(memcmp(m,sig->digest->data,m_len) != 0))
			{
			RSAerr(RSA_F_RSA_VERIFY,RSA_R_BAD_SIGNATURE);
			}
		else
			ret=1;
	}
err:
	if (sig != NULL) X509_SIG_free(sig);
	if (s != NULL)
		{
		OPENSSL_cleanse(s,(unsigned int)siglen);
		OPENSSL_free(s);
		}
	return(ret);
	}

