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
/* t_spki.c */
/* Written by Dr Stephen N Henson (shenson@bigfoot.com) for the OpenSSL
 * project 1999.
 */
/* ====================================================================
 * Copyright (c) 1999 The OpenSSL Project.  All rights reserved.
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
 *    for use in the OpenSSL Toolkit. (http://www.OpenSSL.org/)"
 *
 * 4. The names "OpenSSL Toolkit" and "OpenSSL Project" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For written permission, please contact
 *    licensing@OpenSSL.org.
 *
 * 5. Products derived from this software may not be called "OpenSSL"
 *    nor may "OpenSSL" appear in their names without prior written
 *    permission of the OpenSSL Project.
 *
 * 6. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit (http://www.OpenSSL.org/)"
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
 * This product includes cryptographic software written by Eric Young
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 */

#include <stdio.h>
#include "cryptlib.h"
#include <openssl/x509.h>
#include <openssl/asn1.h>
#ifndef OPENSSL_NO_RSA
#include <openssl/rsa.h>
#endif
#ifndef OPENSSL_NO_DSA
#include <openssl/dsa.h>
#endif
#include <openssl/bn.h>

/* Print out an SPKI */

int NETSCAPE_SPKI_print(BIO *out, NETSCAPE_SPKI *spki)
{
	EVP_PKEY *pkey;
	ASN1_IA5STRING *chal;
	int i, n;
	char *s;
	BIO_printf(out, "Netscape SPKI:\n");
	i=OBJ_obj2nid(spki->spkac->pubkey->algor->algorithm);
	BIO_printf(out,"  Public Key Algorithm: %s\n",
				(i == NID_undef)?"UNKNOWN":OBJ_nid2ln(i));
	pkey = X509_PUBKEY_get(spki->spkac->pubkey);
	if(!pkey) BIO_printf(out, "  Unable to load public key\n");
	else {
#ifndef OPENSSL_NO_RSA
		if (pkey->type == EVP_PKEY_RSA)
			{
			BIO_printf(out,"  RSA Public Key: (%d bit)\n",
				BN_num_bits(pkey->pkey.rsa->n));
			RSA_print(out,pkey->pkey.rsa,2);
			}
		else 
#endif
#ifndef OPENSSL_NO_DSA
		if (pkey->type == EVP_PKEY_DSA)
		{
		BIO_printf(out,"  DSA Public Key:\n");
		DSA_print(out,pkey->pkey.dsa,2);
		}
		else
#endif
#ifndef OPENSSL_NO_EC
		if (pkey->type == EVP_PKEY_EC)
		{
			BIO_printf(out, "  EC Public Key:\n");
			EC_KEY_print(out, pkey->pkey.ec,2);
		}
		else
#endif

			BIO_printf(out,"  Unknown Public Key:\n");
		EVP_PKEY_free(pkey);
	}
	chal = spki->spkac->challenge;
	if(chal->length)
		BIO_printf(out, "  Challenge String: %s\n", chal->data);
	i=OBJ_obj2nid(spki->sig_algor->algorithm);
	BIO_printf(out,"  Signature Algorithm: %s",
				(i == NID_undef)?"UNKNOWN":OBJ_nid2ln(i));

	n=spki->signature->length;
	s=(char *)spki->signature->data;
	for (i=0; i<n; i++)
		{
		if ((i%18) == 0) BIO_write(out,"\n      ",7);
		BIO_printf(out,"%02x%s",(unsigned char)s[i],
						((i+1) == n)?"":":");
		}
	BIO_write(out,"\n",1);
	return 1;
}
