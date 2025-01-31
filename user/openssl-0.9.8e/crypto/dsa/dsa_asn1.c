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
/* dsa_asn1.c */
/* Written by Dr Stephen N Henson (shenson@bigfoot.com) for the OpenSSL
 * project 2000.
 */
/* ====================================================================
 * Copyright (c) 2000 The OpenSSL Project.  All rights reserved.
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
#include <openssl/dsa.h>
#include <openssl/asn1.h>
#include <openssl/asn1t.h>

/* Override the default new methods */
static int sig_cb(int operation, ASN1_VALUE **pval, const ASN1_ITEM *it)
{
	if(operation == ASN1_OP_NEW_PRE) {
		DSA_SIG *sig;
		sig = OPENSSL_malloc(sizeof(DSA_SIG));
		sig->r = NULL;
		sig->s = NULL;
		*pval = (ASN1_VALUE *)sig;
		if(sig) return 2;
		DSAerr(DSA_F_SIG_CB, ERR_R_MALLOC_FAILURE);
		return 0;
	}
	return 1;
}

ASN1_SEQUENCE_cb(DSA_SIG, sig_cb) = {
	ASN1_SIMPLE(DSA_SIG, r, CBIGNUM),
	ASN1_SIMPLE(DSA_SIG, s, CBIGNUM)
} ASN1_SEQUENCE_END_cb(DSA_SIG, DSA_SIG)

IMPLEMENT_ASN1_FUNCTIONS_const(DSA_SIG)

/* Override the default free and new methods */
static int dsa_cb(int operation, ASN1_VALUE **pval, const ASN1_ITEM *it)
{
	if(operation == ASN1_OP_NEW_PRE) {
		*pval = (ASN1_VALUE *)DSA_new();
		if(*pval) return 2;
		return 0;
	} else if(operation == ASN1_OP_FREE_PRE) {
		DSA_free((DSA *)*pval);
		*pval = NULL;
		return 2;
	}
	return 1;
}

ASN1_SEQUENCE_cb(DSAPrivateKey, dsa_cb) = {
	ASN1_SIMPLE(DSA, version, LONG),
	ASN1_SIMPLE(DSA, p, BIGNUM),
	ASN1_SIMPLE(DSA, q, BIGNUM),
	ASN1_SIMPLE(DSA, g, BIGNUM),
	ASN1_SIMPLE(DSA, pub_key, BIGNUM),
	ASN1_SIMPLE(DSA, priv_key, BIGNUM)
} ASN1_SEQUENCE_END_cb(DSA, DSAPrivateKey)

IMPLEMENT_ASN1_ENCODE_FUNCTIONS_const_fname(DSA, DSAPrivateKey, DSAPrivateKey)

ASN1_SEQUENCE_cb(DSAparams, dsa_cb) = {
	ASN1_SIMPLE(DSA, p, BIGNUM),
	ASN1_SIMPLE(DSA, q, BIGNUM),
	ASN1_SIMPLE(DSA, g, BIGNUM),
} ASN1_SEQUENCE_END_cb(DSA, DSAparams)

IMPLEMENT_ASN1_ENCODE_FUNCTIONS_const_fname(DSA, DSAparams, DSAparams)

/* DSA public key is a bit trickier... its effectively a CHOICE type
 * decided by a field called write_params which can either write out
 * just the public key as an INTEGER or the parameters and public key
 * in a SEQUENCE
 */

ASN1_SEQUENCE(dsa_pub_internal) = {
	ASN1_SIMPLE(DSA, pub_key, BIGNUM),
	ASN1_SIMPLE(DSA, p, BIGNUM),
	ASN1_SIMPLE(DSA, q, BIGNUM),
	ASN1_SIMPLE(DSA, g, BIGNUM)
} ASN1_SEQUENCE_END_name(DSA, dsa_pub_internal)

ASN1_CHOICE_cb(DSAPublicKey, dsa_cb) = {
	ASN1_SIMPLE(DSA, pub_key, BIGNUM),
	ASN1_EX_COMBINE(0, 0, dsa_pub_internal)
} ASN1_CHOICE_END_cb(DSA, DSAPublicKey, write_params)

IMPLEMENT_ASN1_ENCODE_FUNCTIONS_const_fname(DSA, DSAPublicKey, DSAPublicKey)
