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
 * wpa_supplicant/hostapd: TLSv1 common routines
 * Copyright (c) 2006, Jouni Malinen <jkmaline@cc.hut.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 */

#include "includes.h"

#include "common.h"
#include "md5.h"
#include "sha1.h"
#include "crypto.h"
#include "x509v3.h"
#include "tlsv1_common.h"


/*
 * TODO:
 * RFC 2246 Section 9: Mandatory to implement TLS_DHE_DSS_WITH_3DES_EDE_CBC_SHA
 * Add support for commonly used cipher suites; don't bother with exportable
 * suites.
 */ 

static const struct tls_cipher_suite tls_cipher_suites[] = {
	{ TLS_NULL_WITH_NULL_NULL, TLS_KEY_X_NULL, TLS_CIPHER_NULL,
	  TLS_HASH_NULL },
	{ TLS_RSA_WITH_RC4_128_MD5, TLS_KEY_X_RSA, TLS_CIPHER_RC4_128,
	  TLS_HASH_MD5 },
	{ TLS_RSA_WITH_RC4_128_SHA, TLS_KEY_X_RSA, TLS_CIPHER_RC4_128,
	  TLS_HASH_SHA },
	{ TLS_RSA_WITH_DES_CBC_SHA, TLS_KEY_X_RSA, TLS_CIPHER_DES_CBC,
	  TLS_HASH_SHA },
	{ TLS_RSA_WITH_3DES_EDE_CBC_SHA, TLS_KEY_X_RSA,
	  TLS_CIPHER_3DES_EDE_CBC, TLS_HASH_SHA },
 	{ TLS_DH_anon_WITH_RC4_128_MD5, TLS_KEY_X_DH_anon,
	  TLS_CIPHER_RC4_128, TLS_HASH_MD5 },
 	{ TLS_DH_anon_WITH_DES_CBC_SHA, TLS_KEY_X_DH_anon,
	  TLS_CIPHER_DES_CBC, TLS_HASH_SHA },
 	{ TLS_DH_anon_WITH_3DES_EDE_CBC_SHA, TLS_KEY_X_DH_anon,
	  TLS_CIPHER_3DES_EDE_CBC, TLS_HASH_SHA },
	{ TLS_RSA_WITH_AES_128_CBC_SHA, TLS_KEY_X_RSA, TLS_CIPHER_AES_128_CBC,
	  TLS_HASH_SHA },
	{ TLS_DH_anon_WITH_AES_128_CBC_SHA, TLS_KEY_X_DH_anon,
	  TLS_CIPHER_AES_128_CBC, TLS_HASH_SHA },
	{ TLS_RSA_WITH_AES_256_CBC_SHA, TLS_KEY_X_RSA, TLS_CIPHER_AES_256_CBC,
	  TLS_HASH_SHA },
	{ TLS_DH_anon_WITH_AES_256_CBC_SHA, TLS_KEY_X_DH_anon,
	  TLS_CIPHER_AES_256_CBC, TLS_HASH_SHA }
};

#define NUM_ELEMS(a) (sizeof(a) / sizeof((a)[0]))
#define NUM_TLS_CIPHER_SUITES NUM_ELEMS(tls_cipher_suites)


static const struct tls_cipher_data tls_ciphers[] = {
	{ TLS_CIPHER_NULL,         TLS_CIPHER_STREAM,  0,  0,  0,
	  CRYPTO_CIPHER_NULL },
	{ TLS_CIPHER_IDEA_CBC,     TLS_CIPHER_BLOCK,  16, 16,  8,
	  CRYPTO_CIPHER_NULL },
	{ TLS_CIPHER_RC2_CBC_40,   TLS_CIPHER_BLOCK,   5, 16,  0,
	  CRYPTO_CIPHER_ALG_RC2 },
	{ TLS_CIPHER_RC4_40,       TLS_CIPHER_STREAM,  5, 16,  0,
	  CRYPTO_CIPHER_ALG_RC4 },
	{ TLS_CIPHER_RC4_128,      TLS_CIPHER_STREAM, 16, 16,  0,
	  CRYPTO_CIPHER_ALG_RC4 },
	{ TLS_CIPHER_DES40_CBC,    TLS_CIPHER_BLOCK,   5,  8,  8,
	  CRYPTO_CIPHER_ALG_DES },
	{ TLS_CIPHER_DES_CBC,      TLS_CIPHER_BLOCK,   8,  8,  8,
	  CRYPTO_CIPHER_ALG_DES },
	{ TLS_CIPHER_3DES_EDE_CBC, TLS_CIPHER_BLOCK,  24, 24,  8,
	  CRYPTO_CIPHER_ALG_3DES },
	{ TLS_CIPHER_AES_128_CBC,  TLS_CIPHER_BLOCK,  16, 16, 16,
	  CRYPTO_CIPHER_ALG_AES },
	{ TLS_CIPHER_AES_256_CBC,  TLS_CIPHER_BLOCK,  32, 32, 16,
	  CRYPTO_CIPHER_ALG_AES }
};

#define NUM_TLS_CIPHER_DATA NUM_ELEMS(tls_ciphers)


/**
 * tls_get_cipher_suite - Get TLS cipher suite
 * @suite: Cipher suite identifier
 * Returns: Pointer to the cipher data or %NULL if not found
 */
const struct tls_cipher_suite * tls_get_cipher_suite(u16 suite)
{
	size_t i;
	for (i = 0; i < NUM_TLS_CIPHER_SUITES; i++)
		if (tls_cipher_suites[i].suite == suite)
			return &tls_cipher_suites[i];
	return NULL;
}


static const struct tls_cipher_data * tls_get_cipher_data(tls_cipher cipher)
{
	size_t i;
	for (i = 0; i < NUM_TLS_CIPHER_DATA; i++)
		if (tls_ciphers[i].cipher == cipher)
			return &tls_ciphers[i];
	return NULL;
}


/**
 * tls_parse_cert - Parse DER encoded X.509 certificate and get public key
 * @buf: ASN.1 DER encoded certificate
 * @len: Length of the buffer
 * @pk: Buffer for returning the allocated public key
 * Returns: 0 on success, -1 on failure
 *
 * This functions parses an ASN.1 DER encoded X.509 certificate and retrieves
 * the public key from it. The caller is responsible for freeing the public key
 * by calling crypto_public_key_free().
 */
int tls_parse_cert(const u8 *buf, size_t len, struct crypto_public_key **pk)
{
	struct x509_certificate *cert;

	wpa_hexdump(MSG_MSGDUMP, "TLSv1: Parse ASN.1 DER certificate",
		    buf, len);

	*pk = crypto_public_key_from_cert(buf, len);
	if (*pk)
		return 0;

	cert = x509_certificate_parse(buf, len);
	if (cert == NULL) {
		wpa_printf(MSG_DEBUG, "TLSv1: Failed to parse X.509 "
			   "certificate");
		return -1;
	}

	/* TODO
	 * verify key usage (must allow encryption)
	 *
	 * All certificate profiles, key and cryptographic formats are
	 * defined by the IETF PKIX working group [PKIX]. When a key
	 * usage extension is present, the digitalSignature bit must be
	 * set for the key to be eligible for signing, as described
	 * above, and the keyEncipherment bit must be present to allow
	 * encryption, as described above. The keyAgreement bit must be
	 * set on Diffie-Hellman certificates. (PKIX: RFC 3280)
	 */

	*pk = crypto_public_key_import(cert->public_key, cert->public_key_len);
	x509_certificate_free(cert);

	if (*pk == NULL) {
		wpa_printf(MSG_ERROR, "TLSv1: Failed to import "
			   "server public key");
		return -1;
	}

	return 0;
}


/**
 * tlsv1_record_set_cipher_suite - TLS record layer: Set cipher suite
 * @rl: Pointer to TLS record layer data
 * @cipher_suite: New cipher suite
 * Returns: 0 on success, -1 on failure
 *
 * This function is used to prepare TLS record layer for cipher suite change.
 * tlsv1_record_change_write_cipher() and
 * tlsv1_record_change_read_cipher() functions can then be used to change the
 * currently used ciphers.
 */
int tlsv1_record_set_cipher_suite(struct tlsv1_record_layer *rl,
				  u16 cipher_suite)
{
	const struct tls_cipher_suite *suite;
	const struct tls_cipher_data *data;

	wpa_printf(MSG_DEBUG, "TLSv1: Selected cipher suite: 0x%04x",
		   cipher_suite);
	rl->cipher_suite = cipher_suite;

	suite = tls_get_cipher_suite(cipher_suite);
	if (suite == NULL)
		return -1;

	if (suite->hash == TLS_HASH_MD5) {
		rl->hash_alg = CRYPTO_HASH_ALG_HMAC_MD5;
		rl->hash_size = MD5_MAC_LEN;
	} else if (suite->hash == TLS_HASH_SHA) {
		rl->hash_alg = CRYPTO_HASH_ALG_HMAC_SHA1;
		rl->hash_size = SHA1_MAC_LEN;
	}

	data = tls_get_cipher_data(suite->cipher);
	if (data == NULL)
		return -1;

	rl->key_material_len = data->key_material;
	rl->iv_size = data->block_size;
	rl->cipher_alg = data->alg;

	return 0;
}


/**
 * tlsv1_record_change_write_cipher - TLS record layer: Change write cipher
 * @rl: Pointer to TLS record layer data
 * Returns: 0 on success (cipher changed), -1 on failure
 *
 * This function changes TLS record layer to use the new cipher suite
 * configured with tlsv1_record_set_cipher_suite() for writing.
 */
int tlsv1_record_change_write_cipher(struct tlsv1_record_layer *rl)
{
	wpa_printf(MSG_DEBUG, "TLSv1: Record Layer - New write cipher suite "
		   "0x%04x", rl->cipher_suite);
	rl->write_cipher_suite = rl->cipher_suite;
	os_memset(rl->write_seq_num, 0, TLS_SEQ_NUM_LEN);

	if (rl->write_cbc) {
		crypto_cipher_deinit(rl->write_cbc);
		rl->write_cbc = NULL;
	}
	if (rl->cipher_alg != CRYPTO_CIPHER_NULL) {
		rl->write_cbc = crypto_cipher_init(rl->cipher_alg,
						   rl->write_iv, rl->write_key,
						   rl->key_material_len);
		if (rl->write_cbc == NULL) {
			wpa_printf(MSG_DEBUG, "TLSv1: Failed to initialize "
				   "cipher");
			return -1;
		}
	}

	return 0;
}


/**
 * tlsv1_record_change_read_cipher - TLS record layer: Change read cipher
 * @rl: Pointer to TLS record layer data
 * Returns: 0 on success (cipher changed), -1 on failure
 *
 * This function changes TLS record layer to use the new cipher suite
 * configured with tlsv1_record_set_cipher_suite() for reading.
 */
int tlsv1_record_change_read_cipher(struct tlsv1_record_layer *rl)
{
	wpa_printf(MSG_DEBUG, "TLSv1: Record Layer - New read cipher suite "
		   "0x%04x", rl->cipher_suite);
	rl->read_cipher_suite = rl->cipher_suite;
	os_memset(rl->read_seq_num, 0, TLS_SEQ_NUM_LEN);

	if (rl->read_cbc) {
		crypto_cipher_deinit(rl->read_cbc);
		rl->read_cbc = NULL;
	}
	if (rl->cipher_alg != CRYPTO_CIPHER_NULL) {
		rl->read_cbc = crypto_cipher_init(rl->cipher_alg,
						  rl->read_iv, rl->read_key,
						  rl->key_material_len);
		if (rl->read_cbc == NULL) {
			wpa_printf(MSG_DEBUG, "TLSv1: Failed to initialize "
				   "cipher");
			return -1;
		}
	}

	return 0;
}


/**
 * tlsv1_record_send - TLS record layer: Send a message
 * @rl: Pointer to TLS record layer data
 * @content_type: Content type (TLS_CONTENT_TYPE_*)
 * @buf: Buffer to send (with TLS_RECORD_HEADER_LEN octets reserved in the
 * beginning for record layer to fill in; payload filled in after this and
 * extra space in the end for HMAC).
 * @buf_size: Maximum buf size
 * @payload_len: Length of the payload
 * @out_len: Buffer for returning the used buf length
 * Returns: 0 on success, -1 on failure
 *
 * This function fills in the TLS record layer header, adds HMAC, and encrypts
 * the data using the current write cipher.
 */
int tlsv1_record_send(struct tlsv1_record_layer *rl, u8 content_type, u8 *buf,
		      size_t buf_size, size_t payload_len, size_t *out_len)
{
	u8 *pos, *ct_start, *length, *payload;
	struct crypto_hash *hmac;
	size_t clen;

	pos = buf;
	/* ContentType type */
	ct_start = pos;
	*pos++ = content_type;
	/* ProtocolVersion version */
	WPA_PUT_BE16(pos, TLS_VERSION);
	pos += 2;
	/* uint16 length */
	length = pos;
	WPA_PUT_BE16(length, payload_len);
	pos += 2;

	/* opaque fragment[TLSPlaintext.length] */
	payload = pos;
	pos += payload_len;

	if (rl->write_cipher_suite != TLS_NULL_WITH_NULL_NULL) {
		hmac = crypto_hash_init(rl->hash_alg, rl->write_mac_secret,
					rl->hash_size);
		if (hmac == NULL) {
			wpa_printf(MSG_DEBUG, "TLSv1: Record Layer - Failed "
				   "to initialize HMAC");
			return -1;
		}
		crypto_hash_update(hmac, rl->write_seq_num, TLS_SEQ_NUM_LEN);
		/* type + version + length + fragment */
		crypto_hash_update(hmac, ct_start, pos - ct_start);
		clen = buf + buf_size - pos;
		if (clen < rl->hash_size) {
			wpa_printf(MSG_DEBUG, "TLSv1: Record Layer - Not "
				   "enough room for MAC");
			crypto_hash_finish(hmac, NULL, NULL);
			return -1;
		}

		if (crypto_hash_finish(hmac, pos, &clen) < 0) {
			wpa_printf(MSG_DEBUG, "TLSv1: Record Layer - Failed "
				   "to calculate HMAC");
			return -1;
		}
		wpa_hexdump(MSG_MSGDUMP, "TLSv1: Record Layer - Write HMAC",
			    pos, clen);
		pos += clen;
		if (rl->iv_size) {
			size_t len = pos - payload;
			size_t pad;
			pad = (len + 1) % rl->iv_size;
			if (pad)
				pad = rl->iv_size - pad;
			if (pos + pad + 1 > buf + buf_size) {
				wpa_printf(MSG_DEBUG, "TLSv1: No room for "
					   "block cipher padding");
				return -1;
			}
			os_memset(pos, pad, pad + 1);
			pos += pad + 1;
		}

		if (crypto_cipher_encrypt(rl->write_cbc, payload,
					  payload, pos - payload) < 0)
			return -1;
	}

	WPA_PUT_BE16(length, pos - length - 2);
	inc_byte_array(rl->write_seq_num, TLS_SEQ_NUM_LEN);

	*out_len = pos - buf;

	return 0;
}


/**
 * tlsv1_record_receive - TLS record layer: Process a received message
 * @rl: Pointer to TLS record layer data
 * @in_data: Received data
 * @in_len: Length of the received data
 * @out_data: Buffer for output data (must be at least as long as in_data)
 * @out_len: Set to maximum out_data length by caller; used to return the
 * length of the used data
 * @alert: Buffer for returning an alert value on failure
 * Returns: 0 on success, -1 on failure
 *
 * This function decrypts the received message, verifies HMAC and TLS record
 * layer header.
 */
int tlsv1_record_receive(struct tlsv1_record_layer *rl,
			 const u8 *in_data, size_t in_len,
			 u8 *out_data, size_t *out_len, u8 *alert)
{
	size_t i, rlen, hlen;
	u8 padlen;
	struct crypto_hash *hmac;
	u8 len[2], hash[100];

	wpa_hexdump(MSG_MSGDUMP, "TLSv1: Record Layer - Received",
		    in_data, in_len);

	if (in_len < TLS_RECORD_HEADER_LEN) {
		wpa_printf(MSG_DEBUG, "TLSv1: Too short record (in_len=%lu)",
			   (unsigned long) in_len);
		*alert = TLS_ALERT_DECODE_ERROR;
		return -1;
	}

	wpa_printf(MSG_DEBUG, "TLSv1: Received content type %d version %d.%d "
		   "length %d", in_data[0], in_data[1], in_data[2],
		   WPA_GET_BE16(in_data + 3));

	if (in_data[0] != TLS_CONTENT_TYPE_HANDSHAKE &&
	    in_data[0] != TLS_CONTENT_TYPE_CHANGE_CIPHER_SPEC &&
	    in_data[0] != TLS_CONTENT_TYPE_APPLICATION_DATA) {
		wpa_printf(MSG_DEBUG, "TLSv1: Unexpected content type 0x%x",
			   in_data[0]);
		*alert = TLS_ALERT_UNEXPECTED_MESSAGE;
		return -1;
	}

	if (WPA_GET_BE16(in_data + 1) != TLS_VERSION) {
		wpa_printf(MSG_DEBUG, "TLSv1: Unexpected protocol version "
			   "%d.%d", in_data[1], in_data[2]);
		*alert = TLS_ALERT_PROTOCOL_VERSION;
		return -1;
	}

	rlen = WPA_GET_BE16(in_data + 3);

	/* TLSCiphertext must not be more than 2^14+2048 bytes */
	if (TLS_RECORD_HEADER_LEN + rlen > 18432) {
		wpa_printf(MSG_DEBUG, "TLSv1: Record overflow (len=%lu)",
			   (unsigned long) (TLS_RECORD_HEADER_LEN + rlen));
		*alert = TLS_ALERT_RECORD_OVERFLOW;
		return -1;
	}

	in_data += TLS_RECORD_HEADER_LEN;
	in_len -= TLS_RECORD_HEADER_LEN;

	if (rlen > in_len) {
		wpa_printf(MSG_DEBUG, "TLSv1: Not all record data included "
			   "(rlen=%lu > in_len=%lu)",
			   (unsigned long) rlen, (unsigned long) in_len);
		*alert = TLS_ALERT_DECODE_ERROR;
		return -1;
	}

	in_len = rlen;

	if (*out_len < in_len) {
		wpa_printf(MSG_DEBUG, "TLSv1: Not enough output buffer for "
			   "processing received record");
		*alert = TLS_ALERT_INTERNAL_ERROR;
		return -1;
	}

	os_memcpy(out_data, in_data, in_len);
	*out_len = in_len;

	if (rl->read_cipher_suite != TLS_NULL_WITH_NULL_NULL) {
		if (crypto_cipher_decrypt(rl->read_cbc, out_data,
					  out_data, in_len) < 0) {
			*alert = TLS_ALERT_DECRYPTION_FAILED;
			return -1;
		}
		if (rl->iv_size) {
			if (in_len == 0) {
				wpa_printf(MSG_DEBUG, "TLSv1: Too short record"
					   " (no pad)");
				*alert = TLS_ALERT_DECODE_ERROR;
				return -1;
			}
			padlen = out_data[in_len - 1];
			if (padlen >= in_len) {
				wpa_printf(MSG_DEBUG, "TLSv1: Incorrect pad "
					   "length (%u, in_len=%lu) in "
					   "received record",
					   padlen, (unsigned long) in_len);
				*alert = TLS_ALERT_DECRYPTION_FAILED;
				return -1;
			}
			for (i = in_len - padlen; i < in_len; i++) {
				if (out_data[i] != padlen) {
					wpa_hexdump(MSG_DEBUG,
						    "TLSv1: Invalid pad in "
						    "received record",
						    out_data + in_len - padlen,
						    padlen);
					*alert = TLS_ALERT_DECRYPTION_FAILED;
					return -1;
				}
			}

			*out_len -= padlen + 1;
		}

		wpa_hexdump(MSG_MSGDUMP,
			    "TLSv1: Record Layer - Decrypted data",
			    out_data, in_len);

		if (*out_len < rl->hash_size) {
			wpa_printf(MSG_DEBUG, "TLSv1: Too short record; no "
				   "hash value");
			*alert = TLS_ALERT_INTERNAL_ERROR;
			return -1;
		}

		*out_len -= rl->hash_size;

		hmac = crypto_hash_init(rl->hash_alg, rl->read_mac_secret,
					rl->hash_size);
		if (hmac == NULL) {
			wpa_printf(MSG_DEBUG, "TLSv1: Record Layer - Failed "
				   "to initialize HMAC");
			*alert = TLS_ALERT_INTERNAL_ERROR;
			return -1;
		}

		crypto_hash_update(hmac, rl->read_seq_num, TLS_SEQ_NUM_LEN);
		/* type + version + length + fragment */
		crypto_hash_update(hmac, in_data - TLS_RECORD_HEADER_LEN, 3);
		WPA_PUT_BE16(len, *out_len);
		crypto_hash_update(hmac, len, 2);
		crypto_hash_update(hmac, out_data, *out_len);
		hlen = sizeof(hash);
		if (crypto_hash_finish(hmac, hash, &hlen) < 0) {
			wpa_printf(MSG_DEBUG, "TLSv1: Record Layer - Failed "
				   "to calculate HMAC");
			return -1;
		}
		if (hlen != rl->hash_size ||
		    os_memcmp(hash, out_data + *out_len, hlen) != 0) {
			wpa_printf(MSG_DEBUG, "TLSv1: Invalid HMAC value in "
				   "received message");
			*alert = TLS_ALERT_BAD_RECORD_MAC;
			return -1;
		}
	}

	/* TLSCompressed must not be more than 2^14+1024 bytes */
	if (TLS_RECORD_HEADER_LEN + *out_len > 17408) {
		wpa_printf(MSG_DEBUG, "TLSv1: Record overflow (len=%lu)",
			   (unsigned long) (TLS_RECORD_HEADER_LEN + *out_len));
		*alert = TLS_ALERT_RECORD_OVERFLOW;
		return -1;
	}

	inc_byte_array(rl->read_seq_num, TLS_SEQ_NUM_LEN);

	return 0;
}
