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
/* crypto/camellia/camellia.h -*- mode:C; c-file-style: "eay" -*- */
/* ====================================================================
 * Copyright (c) 2006 The OpenSSL Project.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit. (http://www.openssl.org/)"
 *
 * 4. The names "OpenSSL Toolkit" and "OpenSSL Project" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For written permission, please contact
 *    openssl-core@openssl.org.
 *
 * 5. Products derived from this software may not be called "OpenSSL"
 *    nor may "OpenSSL" appear in their names without prior written
 *    permission of the OpenSSL Project.
 *
 * 6. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit (http://www.openssl.org/)"
 *
 * THIS SOFTWARE IS PROVIDED BY THE OpenSSL PROJECT ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE OpenSSL PROJECT OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * ====================================================================
 *
 */

#ifndef HEADER_CAMELLIA_H
#define HEADER_CAMELLIA_H

#include <openssl/opensslconf.h>

#ifdef OPENSSL_NO_CAMELLIA
#error CAMELLIA is disabled.
#endif

#define CAMELLIA_ENCRYPT	1
#define CAMELLIA_DECRYPT	0

/* Because array size can't be a const in C, the following two are macros.
   Both sizes are in bytes. */

#ifdef  __cplusplus
extern "C" {
#endif

/* This should be a hidden type, but EVP requires that the size be known */

#define CAMELLIA_BLOCK_SIZE 16
#define CAMELLIA_TABLE_BYTE_LEN 272
#define CAMELLIA_TABLE_WORD_LEN (CAMELLIA_TABLE_BYTE_LEN / 4)

 /* to match with WORD */
typedef unsigned int KEY_TABLE_TYPE[CAMELLIA_TABLE_WORD_LEN];

struct camellia_key_st 
	{
	KEY_TABLE_TYPE rd_key;
	int bitLength;
	void (*enc)(const unsigned int *subkey, unsigned int *io);
	void (*dec)(const unsigned int *subkey, unsigned int *io);
	};

typedef struct camellia_key_st CAMELLIA_KEY;

int Camellia_set_key(const unsigned char *userKey, const int bits,
	CAMELLIA_KEY *key);

void Camellia_encrypt(const unsigned char *in, unsigned char *out,
	const CAMELLIA_KEY *key);
void Camellia_decrypt(const unsigned char *in, unsigned char *out,
	const CAMELLIA_KEY *key);

void Camellia_ecb_encrypt(const unsigned char *in, unsigned char *out,
	const CAMELLIA_KEY *key, const int enc);
void Camellia_cbc_encrypt(const unsigned char *in, unsigned char *out,
	const unsigned long length, const CAMELLIA_KEY *key,
	unsigned char *ivec, const int enc);
void Camellia_cfb128_encrypt(const unsigned char *in, unsigned char *out,
	const unsigned long length, const CAMELLIA_KEY *key,
	unsigned char *ivec, int *num, const int enc);
void Camellia_cfb1_encrypt(const unsigned char *in, unsigned char *out,
	const unsigned long length, const CAMELLIA_KEY *key,
	unsigned char *ivec, int *num, const int enc);
void Camellia_cfb8_encrypt(const unsigned char *in, unsigned char *out,
	const unsigned long length, const CAMELLIA_KEY *key,
	unsigned char *ivec, int *num, const int enc);
void Camellia_cfbr_encrypt_block(const unsigned char *in,unsigned char *out,
	const int nbits,const CAMELLIA_KEY *key,
	unsigned char *ivec,const int enc);
void Camellia_ofb128_encrypt(const unsigned char *in, unsigned char *out,
	const unsigned long length, const CAMELLIA_KEY *key,
	unsigned char *ivec, int *num);
void Camellia_ctr128_encrypt(const unsigned char *in, unsigned char *out,
	const unsigned long length, const CAMELLIA_KEY *key,
	unsigned char ivec[CAMELLIA_BLOCK_SIZE],
	unsigned char ecount_buf[CAMELLIA_BLOCK_SIZE],
	unsigned int *num);

#ifdef  __cplusplus
}
#endif

#endif /* !HEADER_Camellia_H */

