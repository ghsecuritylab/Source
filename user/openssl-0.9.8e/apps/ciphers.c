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
/* apps/ciphers.c */
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
#include <stdlib.h>
#include <string.h>
#ifdef OPENSSL_NO_STDIO
#define APPS_WIN16
#endif
#include "apps.h"
#include <openssl/err.h>
#include <openssl/ssl.h>

#undef PROG
#define PROG	ciphers_main

static const char *ciphers_usage[]={
"usage: ciphers args\n",
" -v          - verbose mode, a textual listing of the ciphers in SSLeay\n",
" -ssl2       - SSL2 mode\n",
" -ssl3       - SSL3 mode\n",
" -tls1       - TLS1 mode\n",
NULL
};

int MAIN(int, char **);

int MAIN(int argc, char **argv)
	{
	int ret=1,i;
	int verbose=0;
	const char **pp;
	const char *p;
	int badops=0;
	SSL_CTX *ctx=NULL;
	SSL *ssl=NULL;
	char *ciphers=NULL;
	SSL_METHOD *meth=NULL;
	STACK_OF(SSL_CIPHER) *sk;
	char buf[512];
	BIO *STDout=NULL;

#if !defined(OPENSSL_NO_SSL2) && !defined(OPENSSL_NO_SSL3)
	meth=SSLv23_server_method();
#elif !defined(OPENSSL_NO_SSL3)
	meth=SSLv3_server_method();
#elif !defined(OPENSSL_NO_SSL2)
	meth=SSLv2_server_method();
#endif

	apps_startup();

	if (bio_err == NULL)
		bio_err=BIO_new_fp(stderr,BIO_NOCLOSE);
	STDout=BIO_new_fp(stdout,BIO_NOCLOSE);
#ifdef OPENSSL_SYS_VMS
	{
	BIO *tmpbio = BIO_new(BIO_f_linebuffer());
	STDout = BIO_push(tmpbio, STDout);
	}
#endif

	argc--;
	argv++;
	while (argc >= 1)
		{
		if (strcmp(*argv,"-v") == 0)
			verbose=1;
#ifndef OPENSSL_NO_SSL2
		else if (strcmp(*argv,"-ssl2") == 0)
			meth=SSLv2_client_method();
#endif
#ifndef OPENSSL_NO_SSL3
		else if (strcmp(*argv,"-ssl3") == 0)
			meth=SSLv3_client_method();
#endif
#ifndef OPENSSL_NO_TLS1
		else if (strcmp(*argv,"-tls1") == 0)
			meth=TLSv1_client_method();
#endif
		else if ((strncmp(*argv,"-h",2) == 0) ||
			 (strcmp(*argv,"-?") == 0))
			{
			badops=1;
			break;
			}
		else
			{
			ciphers= *argv;
			}
		argc--;
		argv++;
		}

	if (badops)
		{
		for (pp=ciphers_usage; (*pp != NULL); pp++)
			BIO_printf(bio_err,"%s",*pp);
		goto end;
		}

	OpenSSL_add_ssl_algorithms();

	ctx=SSL_CTX_new(meth);
	if (ctx == NULL) goto err;
	if (ciphers != NULL) {
		if(!SSL_CTX_set_cipher_list(ctx,ciphers)) {
			BIO_printf(bio_err, "Error in cipher list\n");
			goto err;
		}
	}
	ssl=SSL_new(ctx);
	if (ssl == NULL) goto err;


	if (!verbose)
		{
		for (i=0; ; i++)
			{
			p=SSL_get_cipher_list(ssl,i);
			if (p == NULL) break;
			if (i != 0) BIO_printf(STDout,":");
			BIO_printf(STDout,"%s",p);
			}
		BIO_printf(STDout,"\n");
		}
	else
		{
		sk=SSL_get_ciphers(ssl);

		for (i=0; i<sk_SSL_CIPHER_num(sk); i++)
			{
			BIO_puts(STDout,SSL_CIPHER_description(
				sk_SSL_CIPHER_value(sk,i),
				buf,sizeof buf));
			}
		}

	ret=0;
	if (0)
		{
err:
		SSL_load_error_strings();
		ERR_print_errors(bio_err);
		}
end:
	if (ctx != NULL) SSL_CTX_free(ctx);
	if (ssl != NULL) SSL_free(ssl);
	if (STDout != NULL) BIO_free_all(STDout);
	apps_shutdown();
	OPENSSL_EXIT(ret);
	}

