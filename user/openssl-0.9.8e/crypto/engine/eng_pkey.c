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
/* crypto/engine/eng_pkey.c */
/* ====================================================================
 * Copyright (c) 1999-2001 The OpenSSL Project.  All rights reserved.
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

#include "eng_int.h"

/* Basic get/set stuff */

int ENGINE_set_load_privkey_function(ENGINE *e, ENGINE_LOAD_KEY_PTR loadpriv_f)
	{
	e->load_privkey = loadpriv_f;
	return 1;
	}

int ENGINE_set_load_pubkey_function(ENGINE *e, ENGINE_LOAD_KEY_PTR loadpub_f)
	{
	e->load_pubkey = loadpub_f;
	return 1;
	}

ENGINE_LOAD_KEY_PTR ENGINE_get_load_privkey_function(const ENGINE *e)
	{
	return e->load_privkey;
	}

ENGINE_LOAD_KEY_PTR ENGINE_get_load_pubkey_function(const ENGINE *e)
	{
	return e->load_pubkey;
	}

/* API functions to load public/private keys */

EVP_PKEY *ENGINE_load_private_key(ENGINE *e, const char *key_id,
	UI_METHOD *ui_method, void *callback_data)
	{
	EVP_PKEY *pkey;

	if(e == NULL)
		{
		ENGINEerr(ENGINE_F_ENGINE_LOAD_PRIVATE_KEY,
			ERR_R_PASSED_NULL_PARAMETER);
		return 0;
		}
	CRYPTO_w_lock(CRYPTO_LOCK_ENGINE);
	if(e->funct_ref == 0)
		{
		CRYPTO_w_unlock(CRYPTO_LOCK_ENGINE);
		ENGINEerr(ENGINE_F_ENGINE_LOAD_PRIVATE_KEY,
			ENGINE_R_NOT_INITIALISED);
		return 0;
		}
	CRYPTO_w_unlock(CRYPTO_LOCK_ENGINE);
	if (!e->load_privkey)
		{
		ENGINEerr(ENGINE_F_ENGINE_LOAD_PRIVATE_KEY,
			ENGINE_R_NO_LOAD_FUNCTION);
		return 0;
		}
	pkey = e->load_privkey(e, key_id, ui_method, callback_data);
	if (!pkey)
		{
		ENGINEerr(ENGINE_F_ENGINE_LOAD_PRIVATE_KEY,
			ENGINE_R_FAILED_LOADING_PRIVATE_KEY);
		return 0;
		}
	return pkey;
	}

EVP_PKEY *ENGINE_load_public_key(ENGINE *e, const char *key_id,
	UI_METHOD *ui_method, void *callback_data)
	{
	EVP_PKEY *pkey;

	if(e == NULL)
		{
		ENGINEerr(ENGINE_F_ENGINE_LOAD_PUBLIC_KEY,
			ERR_R_PASSED_NULL_PARAMETER);
		return 0;
		}
	CRYPTO_w_lock(CRYPTO_LOCK_ENGINE);
	if(e->funct_ref == 0)
		{
		CRYPTO_w_unlock(CRYPTO_LOCK_ENGINE);
		ENGINEerr(ENGINE_F_ENGINE_LOAD_PUBLIC_KEY,
			ENGINE_R_NOT_INITIALISED);
		return 0;
		}
	CRYPTO_w_unlock(CRYPTO_LOCK_ENGINE);
	if (!e->load_pubkey)
		{
		ENGINEerr(ENGINE_F_ENGINE_LOAD_PUBLIC_KEY,
			ENGINE_R_NO_LOAD_FUNCTION);
		return 0;
		}
	pkey = e->load_pubkey(e, key_id, ui_method, callback_data);
	if (!pkey)
		{
		ENGINEerr(ENGINE_F_ENGINE_LOAD_PUBLIC_KEY,
			ENGINE_R_FAILED_LOADING_PUBLIC_KEY);
		return 0;
		}
	return pkey;
	}
