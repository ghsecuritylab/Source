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
/* crypto/ec/ec_check.c */
/* ====================================================================
 * Copyright (c) 1998-2002 The OpenSSL Project.  All rights reserved.
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
 * This product includes cryptographic software written by Eric Young
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 */

#include "ec_lcl.h"
#include <openssl/err.h>

int EC_GROUP_check(const EC_GROUP *group, BN_CTX *ctx)
	{
	int ret = 0;
	BIGNUM *order;
	BN_CTX *new_ctx = NULL;
	EC_POINT *point = NULL;

	if (ctx == NULL)
		{
		ctx = new_ctx = BN_CTX_new();
		if (ctx == NULL)
			{
			ECerr(EC_F_EC_GROUP_CHECK, ERR_R_MALLOC_FAILURE);
			goto err;
			}
		}
	BN_CTX_start(ctx);
	if ((order = BN_CTX_get(ctx)) == NULL) goto err;

	/* check the discriminant */
	if (!EC_GROUP_check_discriminant(group, ctx))
		{
		ECerr(EC_F_EC_GROUP_CHECK, EC_R_DISCRIMINANT_IS_ZERO);
		goto err;
		}

	/* check the generator */
	if (group->generator == NULL)
		{
		ECerr(EC_F_EC_GROUP_CHECK, EC_R_UNDEFINED_GENERATOR);
		goto err;
		}
	if (!EC_POINT_is_on_curve(group, group->generator, ctx))
		{
		ECerr(EC_F_EC_GROUP_CHECK, EC_R_POINT_IS_NOT_ON_CURVE);
		goto err;
		}

	/* check the order of the generator */
	if ((point = EC_POINT_new(group)) == NULL) goto err;
	if (!EC_GROUP_get_order(group, order, ctx)) goto err; 
	if (BN_is_zero(order))
		{
		ECerr(EC_F_EC_GROUP_CHECK, EC_R_UNDEFINED_ORDER);
		goto err;
		}
	
	if (!EC_POINT_mul(group, point, order, NULL, NULL, ctx)) goto err;
	if (!EC_POINT_is_at_infinity(group, point))
		{
		ECerr(EC_F_EC_GROUP_CHECK, EC_R_INVALID_GROUP_ORDER);
		goto err;
		}

	ret = 1;

err:
	if (ctx != NULL)
		BN_CTX_end(ctx);
	if (new_ctx != NULL)
		BN_CTX_free(new_ctx);
	if (point)
		EC_POINT_free(point);
	return ret;
	}
