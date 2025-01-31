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
/* ====================================================================
 * Copyright (c) 2004 The OpenSSL Project.  All rights reserved.
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
 *
 */

#include <string.h>

#include "apps.h"
#include <openssl/bn.h>


#undef PROG
#define PROG prime_main

int MAIN(int, char **);

int MAIN(int argc, char **argv)
    {
    int hex=0;
    int checks=20;
    BIGNUM *bn=NULL;
    BIO *bio_out;

    apps_startup();

    if (bio_err == NULL)
	if ((bio_err=BIO_new(BIO_s_file())) != NULL)
	    BIO_set_fp(bio_err,stderr,BIO_NOCLOSE|BIO_FP_TEXT);

    --argc;
    ++argv;
    while (argc >= 1 && **argv == '-')
	{
	if(!strcmp(*argv,"-hex"))
	    hex=1;
	else if(!strcmp(*argv,"-checks"))
	    if(--argc < 1)
		goto bad;
	    else
		checks=atoi(*++argv);
	else
	    {
	    BIO_printf(bio_err,"Unknown option '%s'\n",*argv);
	    goto bad;
	    }
	--argc;
	++argv;
	}

    if (argv[0] == NULL)
	{
	BIO_printf(bio_err,"No prime specified\n");
	goto bad;
	}

   if ((bio_out=BIO_new(BIO_s_file())) != NULL)
	{
	BIO_set_fp(bio_out,stdout,BIO_NOCLOSE);
#ifdef OPENSSL_SYS_VMS
	    {
	    BIO *tmpbio = BIO_new(BIO_f_linebuffer());
	    bio_out = BIO_push(tmpbio, bio_out);
	    }
#endif
	}

    if(hex)
	BN_hex2bn(&bn,argv[0]);
    else
	BN_dec2bn(&bn,argv[0]);

    BN_print(bio_out,bn);
    BIO_printf(bio_out," is %sprime\n",
	       BN_is_prime_ex(bn,checks,NULL,NULL) ? "" : "not ");

    BN_free(bn);
    BIO_free_all(bio_out);

    return 0;

    bad:
    BIO_printf(bio_err,"options are\n");
    BIO_printf(bio_err,"%-14s hex\n","-hex");
    BIO_printf(bio_err,"%-14s number of checks\n","-checks <n>");
    return 1;
    }
