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
/* crypto/ripemd/rmd_dgst.c */
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
#include "rmd_locl.h"
#include <openssl/opensslv.h>

const char RMD160_version[]="RIPE-MD160" OPENSSL_VERSION_PTEXT;

#  ifdef RMD160_ASM
     void ripemd160_block_x86(RIPEMD160_CTX *c, unsigned long *p,size_t num);
#    define ripemd160_block ripemd160_block_x86
#  else
     void ripemd160_block(RIPEMD160_CTX *c, unsigned long *p,size_t num);
#  endif

int RIPEMD160_Init(RIPEMD160_CTX *c)
	{
	c->A=RIPEMD160_A;
	c->B=RIPEMD160_B;
	c->C=RIPEMD160_C;
	c->D=RIPEMD160_D;
	c->E=RIPEMD160_E;
	c->Nl=0;
	c->Nh=0;
	c->num=0;
	return 1;
	}

#ifndef ripemd160_block_host_order
#ifdef X
#undef X
#endif
#define X(i)	XX[i]
void ripemd160_block_host_order (RIPEMD160_CTX *ctx, const void *p, size_t num)
	{
	const RIPEMD160_LONG *XX=p;
	register unsigned MD32_REG_T A,B,C,D,E;
	register unsigned MD32_REG_T a,b,c,d,e;

	for (;num--;XX+=HASH_LBLOCK)
		{

	A=ctx->A; B=ctx->B; C=ctx->C; D=ctx->D; E=ctx->E;

	RIP1(A,B,C,D,E,WL00,SL00);
	RIP1(E,A,B,C,D,WL01,SL01);
	RIP1(D,E,A,B,C,WL02,SL02);
	RIP1(C,D,E,A,B,WL03,SL03);
	RIP1(B,C,D,E,A,WL04,SL04);
	RIP1(A,B,C,D,E,WL05,SL05);
	RIP1(E,A,B,C,D,WL06,SL06);
	RIP1(D,E,A,B,C,WL07,SL07);
	RIP1(C,D,E,A,B,WL08,SL08);
	RIP1(B,C,D,E,A,WL09,SL09);
	RIP1(A,B,C,D,E,WL10,SL10);
	RIP1(E,A,B,C,D,WL11,SL11);
	RIP1(D,E,A,B,C,WL12,SL12);
	RIP1(C,D,E,A,B,WL13,SL13);
	RIP1(B,C,D,E,A,WL14,SL14);
	RIP1(A,B,C,D,E,WL15,SL15);

	RIP2(E,A,B,C,D,WL16,SL16,KL1);
	RIP2(D,E,A,B,C,WL17,SL17,KL1);
	RIP2(C,D,E,A,B,WL18,SL18,KL1);
	RIP2(B,C,D,E,A,WL19,SL19,KL1);
	RIP2(A,B,C,D,E,WL20,SL20,KL1);
	RIP2(E,A,B,C,D,WL21,SL21,KL1);
	RIP2(D,E,A,B,C,WL22,SL22,KL1);
	RIP2(C,D,E,A,B,WL23,SL23,KL1);
	RIP2(B,C,D,E,A,WL24,SL24,KL1);
	RIP2(A,B,C,D,E,WL25,SL25,KL1);
	RIP2(E,A,B,C,D,WL26,SL26,KL1);
	RIP2(D,E,A,B,C,WL27,SL27,KL1);
	RIP2(C,D,E,A,B,WL28,SL28,KL1);
	RIP2(B,C,D,E,A,WL29,SL29,KL1);
	RIP2(A,B,C,D,E,WL30,SL30,KL1);
	RIP2(E,A,B,C,D,WL31,SL31,KL1);

	RIP3(D,E,A,B,C,WL32,SL32,KL2);
	RIP3(C,D,E,A,B,WL33,SL33,KL2);
	RIP3(B,C,D,E,A,WL34,SL34,KL2);
	RIP3(A,B,C,D,E,WL35,SL35,KL2);
	RIP3(E,A,B,C,D,WL36,SL36,KL2);
	RIP3(D,E,A,B,C,WL37,SL37,KL2);
	RIP3(C,D,E,A,B,WL38,SL38,KL2);
	RIP3(B,C,D,E,A,WL39,SL39,KL2);
	RIP3(A,B,C,D,E,WL40,SL40,KL2);
	RIP3(E,A,B,C,D,WL41,SL41,KL2);
	RIP3(D,E,A,B,C,WL42,SL42,KL2);
	RIP3(C,D,E,A,B,WL43,SL43,KL2);
	RIP3(B,C,D,E,A,WL44,SL44,KL2);
	RIP3(A,B,C,D,E,WL45,SL45,KL2);
	RIP3(E,A,B,C,D,WL46,SL46,KL2);
	RIP3(D,E,A,B,C,WL47,SL47,KL2);

	RIP4(C,D,E,A,B,WL48,SL48,KL3);
	RIP4(B,C,D,E,A,WL49,SL49,KL3);
	RIP4(A,B,C,D,E,WL50,SL50,KL3);
	RIP4(E,A,B,C,D,WL51,SL51,KL3);
	RIP4(D,E,A,B,C,WL52,SL52,KL3);
	RIP4(C,D,E,A,B,WL53,SL53,KL3);
	RIP4(B,C,D,E,A,WL54,SL54,KL3);
	RIP4(A,B,C,D,E,WL55,SL55,KL3);
	RIP4(E,A,B,C,D,WL56,SL56,KL3);
	RIP4(D,E,A,B,C,WL57,SL57,KL3);
	RIP4(C,D,E,A,B,WL58,SL58,KL3);
	RIP4(B,C,D,E,A,WL59,SL59,KL3);
	RIP4(A,B,C,D,E,WL60,SL60,KL3);
	RIP4(E,A,B,C,D,WL61,SL61,KL3);
	RIP4(D,E,A,B,C,WL62,SL62,KL3);
	RIP4(C,D,E,A,B,WL63,SL63,KL3);

	RIP5(B,C,D,E,A,WL64,SL64,KL4);
	RIP5(A,B,C,D,E,WL65,SL65,KL4);
	RIP5(E,A,B,C,D,WL66,SL66,KL4);
	RIP5(D,E,A,B,C,WL67,SL67,KL4);
	RIP5(C,D,E,A,B,WL68,SL68,KL4);
	RIP5(B,C,D,E,A,WL69,SL69,KL4);
	RIP5(A,B,C,D,E,WL70,SL70,KL4);
	RIP5(E,A,B,C,D,WL71,SL71,KL4);
	RIP5(D,E,A,B,C,WL72,SL72,KL4);
	RIP5(C,D,E,A,B,WL73,SL73,KL4);
	RIP5(B,C,D,E,A,WL74,SL74,KL4);
	RIP5(A,B,C,D,E,WL75,SL75,KL4);
	RIP5(E,A,B,C,D,WL76,SL76,KL4);
	RIP5(D,E,A,B,C,WL77,SL77,KL4);
	RIP5(C,D,E,A,B,WL78,SL78,KL4);
	RIP5(B,C,D,E,A,WL79,SL79,KL4);

	a=A; b=B; c=C; d=D; e=E;
	/* Do other half */
	A=ctx->A; B=ctx->B; C=ctx->C; D=ctx->D; E=ctx->E;

	RIP5(A,B,C,D,E,WR00,SR00,KR0);
	RIP5(E,A,B,C,D,WR01,SR01,KR0);
	RIP5(D,E,A,B,C,WR02,SR02,KR0);
	RIP5(C,D,E,A,B,WR03,SR03,KR0);
	RIP5(B,C,D,E,A,WR04,SR04,KR0);
	RIP5(A,B,C,D,E,WR05,SR05,KR0);
	RIP5(E,A,B,C,D,WR06,SR06,KR0);
	RIP5(D,E,A,B,C,WR07,SR07,KR0);
	RIP5(C,D,E,A,B,WR08,SR08,KR0);
	RIP5(B,C,D,E,A,WR09,SR09,KR0);
	RIP5(A,B,C,D,E,WR10,SR10,KR0);
	RIP5(E,A,B,C,D,WR11,SR11,KR0);
	RIP5(D,E,A,B,C,WR12,SR12,KR0);
	RIP5(C,D,E,A,B,WR13,SR13,KR0);
	RIP5(B,C,D,E,A,WR14,SR14,KR0);
	RIP5(A,B,C,D,E,WR15,SR15,KR0);

	RIP4(E,A,B,C,D,WR16,SR16,KR1);
	RIP4(D,E,A,B,C,WR17,SR17,KR1);
	RIP4(C,D,E,A,B,WR18,SR18,KR1);
	RIP4(B,C,D,E,A,WR19,SR19,KR1);
	RIP4(A,B,C,D,E,WR20,SR20,KR1);
	RIP4(E,A,B,C,D,WR21,SR21,KR1);
	RIP4(D,E,A,B,C,WR22,SR22,KR1);
	RIP4(C,D,E,A,B,WR23,SR23,KR1);
	RIP4(B,C,D,E,A,WR24,SR24,KR1);
	RIP4(A,B,C,D,E,WR25,SR25,KR1);
	RIP4(E,A,B,C,D,WR26,SR26,KR1);
	RIP4(D,E,A,B,C,WR27,SR27,KR1);
	RIP4(C,D,E,A,B,WR28,SR28,KR1);
	RIP4(B,C,D,E,A,WR29,SR29,KR1);
	RIP4(A,B,C,D,E,WR30,SR30,KR1);
	RIP4(E,A,B,C,D,WR31,SR31,KR1);

	RIP3(D,E,A,B,C,WR32,SR32,KR2);
	RIP3(C,D,E,A,B,WR33,SR33,KR2);
	RIP3(B,C,D,E,A,WR34,SR34,KR2);
	RIP3(A,B,C,D,E,WR35,SR35,KR2);
	RIP3(E,A,B,C,D,WR36,SR36,KR2);
	RIP3(D,E,A,B,C,WR37,SR37,KR2);
	RIP3(C,D,E,A,B,WR38,SR38,KR2);
	RIP3(B,C,D,E,A,WR39,SR39,KR2);
	RIP3(A,B,C,D,E,WR40,SR40,KR2);
	RIP3(E,A,B,C,D,WR41,SR41,KR2);
	RIP3(D,E,A,B,C,WR42,SR42,KR2);
	RIP3(C,D,E,A,B,WR43,SR43,KR2);
	RIP3(B,C,D,E,A,WR44,SR44,KR2);
	RIP3(A,B,C,D,E,WR45,SR45,KR2);
	RIP3(E,A,B,C,D,WR46,SR46,KR2);
	RIP3(D,E,A,B,C,WR47,SR47,KR2);

	RIP2(C,D,E,A,B,WR48,SR48,KR3);
	RIP2(B,C,D,E,A,WR49,SR49,KR3);
	RIP2(A,B,C,D,E,WR50,SR50,KR3);
	RIP2(E,A,B,C,D,WR51,SR51,KR3);
	RIP2(D,E,A,B,C,WR52,SR52,KR3);
	RIP2(C,D,E,A,B,WR53,SR53,KR3);
	RIP2(B,C,D,E,A,WR54,SR54,KR3);
	RIP2(A,B,C,D,E,WR55,SR55,KR3);
	RIP2(E,A,B,C,D,WR56,SR56,KR3);
	RIP2(D,E,A,B,C,WR57,SR57,KR3);
	RIP2(C,D,E,A,B,WR58,SR58,KR3);
	RIP2(B,C,D,E,A,WR59,SR59,KR3);
	RIP2(A,B,C,D,E,WR60,SR60,KR3);
	RIP2(E,A,B,C,D,WR61,SR61,KR3);
	RIP2(D,E,A,B,C,WR62,SR62,KR3);
	RIP2(C,D,E,A,B,WR63,SR63,KR3);

	RIP1(B,C,D,E,A,WR64,SR64);
	RIP1(A,B,C,D,E,WR65,SR65);
	RIP1(E,A,B,C,D,WR66,SR66);
	RIP1(D,E,A,B,C,WR67,SR67);
	RIP1(C,D,E,A,B,WR68,SR68);
	RIP1(B,C,D,E,A,WR69,SR69);
	RIP1(A,B,C,D,E,WR70,SR70);
	RIP1(E,A,B,C,D,WR71,SR71);
	RIP1(D,E,A,B,C,WR72,SR72);
	RIP1(C,D,E,A,B,WR73,SR73);
	RIP1(B,C,D,E,A,WR74,SR74);
	RIP1(A,B,C,D,E,WR75,SR75);
	RIP1(E,A,B,C,D,WR76,SR76);
	RIP1(D,E,A,B,C,WR77,SR77);
	RIP1(C,D,E,A,B,WR78,SR78);
	RIP1(B,C,D,E,A,WR79,SR79);

	D     =ctx->B+c+D;
	ctx->B=ctx->C+d+E;
	ctx->C=ctx->D+e+A;
	ctx->D=ctx->E+a+B;
	ctx->E=ctx->A+b+C;
	ctx->A=D;

		}
	}
#endif

#ifndef ripemd160_block_data_order
#ifdef X
#undef X
#endif
void ripemd160_block_data_order (RIPEMD160_CTX *ctx, const void *p, size_t num)
	{
	const unsigned char *data=p;
	register unsigned MD32_REG_T A,B,C,D,E;
	unsigned MD32_REG_T a,b,c,d,e,l;
#ifndef MD32_XARRAY
	/* See comment in crypto/sha/sha_locl.h for details. */
	unsigned MD32_REG_T	XX0, XX1, XX2, XX3, XX4, XX5, XX6, XX7,
				XX8, XX9,XX10,XX11,XX12,XX13,XX14,XX15;
# define X(i)	XX##i
#else
	RIPEMD160_LONG	XX[16];
# define X(i)	XX[i]
#endif

	for (;num--;)
		{

	A=ctx->A; B=ctx->B; C=ctx->C; D=ctx->D; E=ctx->E;

	HOST_c2l(data,l); X( 0)=l;	HOST_c2l(data,l); X( 1)=l;
	RIP1(A,B,C,D,E,WL00,SL00);	HOST_c2l(data,l); X( 2)=l;
	RIP1(E,A,B,C,D,WL01,SL01);	HOST_c2l(data,l); X( 3)=l;
	RIP1(D,E,A,B,C,WL02,SL02);	HOST_c2l(data,l); X( 4)=l;
	RIP1(C,D,E,A,B,WL03,SL03);	HOST_c2l(data,l); X( 5)=l;
	RIP1(B,C,D,E,A,WL04,SL04);	HOST_c2l(data,l); X( 6)=l;
	RIP1(A,B,C,D,E,WL05,SL05);	HOST_c2l(data,l); X( 7)=l;
	RIP1(E,A,B,C,D,WL06,SL06);	HOST_c2l(data,l); X( 8)=l;
	RIP1(D,E,A,B,C,WL07,SL07);	HOST_c2l(data,l); X( 9)=l;
	RIP1(C,D,E,A,B,WL08,SL08);	HOST_c2l(data,l); X(10)=l;
	RIP1(B,C,D,E,A,WL09,SL09);	HOST_c2l(data,l); X(11)=l;
	RIP1(A,B,C,D,E,WL10,SL10);	HOST_c2l(data,l); X(12)=l;
	RIP1(E,A,B,C,D,WL11,SL11);	HOST_c2l(data,l); X(13)=l;
	RIP1(D,E,A,B,C,WL12,SL12);	HOST_c2l(data,l); X(14)=l;
	RIP1(C,D,E,A,B,WL13,SL13);	HOST_c2l(data,l); X(15)=l;
	RIP1(B,C,D,E,A,WL14,SL14);
	RIP1(A,B,C,D,E,WL15,SL15);

	RIP2(E,A,B,C,D,WL16,SL16,KL1);
	RIP2(D,E,A,B,C,WL17,SL17,KL1);
	RIP2(C,D,E,A,B,WL18,SL18,KL1);
	RIP2(B,C,D,E,A,WL19,SL19,KL1);
	RIP2(A,B,C,D,E,WL20,SL20,KL1);
	RIP2(E,A,B,C,D,WL21,SL21,KL1);
	RIP2(D,E,A,B,C,WL22,SL22,KL1);
	RIP2(C,D,E,A,B,WL23,SL23,KL1);
	RIP2(B,C,D,E,A,WL24,SL24,KL1);
	RIP2(A,B,C,D,E,WL25,SL25,KL1);
	RIP2(E,A,B,C,D,WL26,SL26,KL1);
	RIP2(D,E,A,B,C,WL27,SL27,KL1);
	RIP2(C,D,E,A,B,WL28,SL28,KL1);
	RIP2(B,C,D,E,A,WL29,SL29,KL1);
	RIP2(A,B,C,D,E,WL30,SL30,KL1);
	RIP2(E,A,B,C,D,WL31,SL31,KL1);

	RIP3(D,E,A,B,C,WL32,SL32,KL2);
	RIP3(C,D,E,A,B,WL33,SL33,KL2);
	RIP3(B,C,D,E,A,WL34,SL34,KL2);
	RIP3(A,B,C,D,E,WL35,SL35,KL2);
	RIP3(E,A,B,C,D,WL36,SL36,KL2);
	RIP3(D,E,A,B,C,WL37,SL37,KL2);
	RIP3(C,D,E,A,B,WL38,SL38,KL2);
	RIP3(B,C,D,E,A,WL39,SL39,KL2);
	RIP3(A,B,C,D,E,WL40,SL40,KL2);
	RIP3(E,A,B,C,D,WL41,SL41,KL2);
	RIP3(D,E,A,B,C,WL42,SL42,KL2);
	RIP3(C,D,E,A,B,WL43,SL43,KL2);
	RIP3(B,C,D,E,A,WL44,SL44,KL2);
	RIP3(A,B,C,D,E,WL45,SL45,KL2);
	RIP3(E,A,B,C,D,WL46,SL46,KL2);
	RIP3(D,E,A,B,C,WL47,SL47,KL2);

	RIP4(C,D,E,A,B,WL48,SL48,KL3);
	RIP4(B,C,D,E,A,WL49,SL49,KL3);
	RIP4(A,B,C,D,E,WL50,SL50,KL3);
	RIP4(E,A,B,C,D,WL51,SL51,KL3);
	RIP4(D,E,A,B,C,WL52,SL52,KL3);
	RIP4(C,D,E,A,B,WL53,SL53,KL3);
	RIP4(B,C,D,E,A,WL54,SL54,KL3);
	RIP4(A,B,C,D,E,WL55,SL55,KL3);
	RIP4(E,A,B,C,D,WL56,SL56,KL3);
	RIP4(D,E,A,B,C,WL57,SL57,KL3);
	RIP4(C,D,E,A,B,WL58,SL58,KL3);
	RIP4(B,C,D,E,A,WL59,SL59,KL3);
	RIP4(A,B,C,D,E,WL60,SL60,KL3);
	RIP4(E,A,B,C,D,WL61,SL61,KL3);
	RIP4(D,E,A,B,C,WL62,SL62,KL3);
	RIP4(C,D,E,A,B,WL63,SL63,KL3);

	RIP5(B,C,D,E,A,WL64,SL64,KL4);
	RIP5(A,B,C,D,E,WL65,SL65,KL4);
	RIP5(E,A,B,C,D,WL66,SL66,KL4);
	RIP5(D,E,A,B,C,WL67,SL67,KL4);
	RIP5(C,D,E,A,B,WL68,SL68,KL4);
	RIP5(B,C,D,E,A,WL69,SL69,KL4);
	RIP5(A,B,C,D,E,WL70,SL70,KL4);
	RIP5(E,A,B,C,D,WL71,SL71,KL4);
	RIP5(D,E,A,B,C,WL72,SL72,KL4);
	RIP5(C,D,E,A,B,WL73,SL73,KL4);
	RIP5(B,C,D,E,A,WL74,SL74,KL4);
	RIP5(A,B,C,D,E,WL75,SL75,KL4);
	RIP5(E,A,B,C,D,WL76,SL76,KL4);
	RIP5(D,E,A,B,C,WL77,SL77,KL4);
	RIP5(C,D,E,A,B,WL78,SL78,KL4);
	RIP5(B,C,D,E,A,WL79,SL79,KL4);

	a=A; b=B; c=C; d=D; e=E;
	/* Do other half */
	A=ctx->A; B=ctx->B; C=ctx->C; D=ctx->D; E=ctx->E;

	RIP5(A,B,C,D,E,WR00,SR00,KR0);
	RIP5(E,A,B,C,D,WR01,SR01,KR0);
	RIP5(D,E,A,B,C,WR02,SR02,KR0);
	RIP5(C,D,E,A,B,WR03,SR03,KR0);
	RIP5(B,C,D,E,A,WR04,SR04,KR0);
	RIP5(A,B,C,D,E,WR05,SR05,KR0);
	RIP5(E,A,B,C,D,WR06,SR06,KR0);
	RIP5(D,E,A,B,C,WR07,SR07,KR0);
	RIP5(C,D,E,A,B,WR08,SR08,KR0);
	RIP5(B,C,D,E,A,WR09,SR09,KR0);
	RIP5(A,B,C,D,E,WR10,SR10,KR0);
	RIP5(E,A,B,C,D,WR11,SR11,KR0);
	RIP5(D,E,A,B,C,WR12,SR12,KR0);
	RIP5(C,D,E,A,B,WR13,SR13,KR0);
	RIP5(B,C,D,E,A,WR14,SR14,KR0);
	RIP5(A,B,C,D,E,WR15,SR15,KR0);

	RIP4(E,A,B,C,D,WR16,SR16,KR1);
	RIP4(D,E,A,B,C,WR17,SR17,KR1);
	RIP4(C,D,E,A,B,WR18,SR18,KR1);
	RIP4(B,C,D,E,A,WR19,SR19,KR1);
	RIP4(A,B,C,D,E,WR20,SR20,KR1);
	RIP4(E,A,B,C,D,WR21,SR21,KR1);
	RIP4(D,E,A,B,C,WR22,SR22,KR1);
	RIP4(C,D,E,A,B,WR23,SR23,KR1);
	RIP4(B,C,D,E,A,WR24,SR24,KR1);
	RIP4(A,B,C,D,E,WR25,SR25,KR1);
	RIP4(E,A,B,C,D,WR26,SR26,KR1);
	RIP4(D,E,A,B,C,WR27,SR27,KR1);
	RIP4(C,D,E,A,B,WR28,SR28,KR1);
	RIP4(B,C,D,E,A,WR29,SR29,KR1);
	RIP4(A,B,C,D,E,WR30,SR30,KR1);
	RIP4(E,A,B,C,D,WR31,SR31,KR1);

	RIP3(D,E,A,B,C,WR32,SR32,KR2);
	RIP3(C,D,E,A,B,WR33,SR33,KR2);
	RIP3(B,C,D,E,A,WR34,SR34,KR2);
	RIP3(A,B,C,D,E,WR35,SR35,KR2);
	RIP3(E,A,B,C,D,WR36,SR36,KR2);
	RIP3(D,E,A,B,C,WR37,SR37,KR2);
	RIP3(C,D,E,A,B,WR38,SR38,KR2);
	RIP3(B,C,D,E,A,WR39,SR39,KR2);
	RIP3(A,B,C,D,E,WR40,SR40,KR2);
	RIP3(E,A,B,C,D,WR41,SR41,KR2);
	RIP3(D,E,A,B,C,WR42,SR42,KR2);
	RIP3(C,D,E,A,B,WR43,SR43,KR2);
	RIP3(B,C,D,E,A,WR44,SR44,KR2);
	RIP3(A,B,C,D,E,WR45,SR45,KR2);
	RIP3(E,A,B,C,D,WR46,SR46,KR2);
	RIP3(D,E,A,B,C,WR47,SR47,KR2);

	RIP2(C,D,E,A,B,WR48,SR48,KR3);
	RIP2(B,C,D,E,A,WR49,SR49,KR3);
	RIP2(A,B,C,D,E,WR50,SR50,KR3);
	RIP2(E,A,B,C,D,WR51,SR51,KR3);
	RIP2(D,E,A,B,C,WR52,SR52,KR3);
	RIP2(C,D,E,A,B,WR53,SR53,KR3);
	RIP2(B,C,D,E,A,WR54,SR54,KR3);
	RIP2(A,B,C,D,E,WR55,SR55,KR3);
	RIP2(E,A,B,C,D,WR56,SR56,KR3);
	RIP2(D,E,A,B,C,WR57,SR57,KR3);
	RIP2(C,D,E,A,B,WR58,SR58,KR3);
	RIP2(B,C,D,E,A,WR59,SR59,KR3);
	RIP2(A,B,C,D,E,WR60,SR60,KR3);
	RIP2(E,A,B,C,D,WR61,SR61,KR3);
	RIP2(D,E,A,B,C,WR62,SR62,KR3);
	RIP2(C,D,E,A,B,WR63,SR63,KR3);

	RIP1(B,C,D,E,A,WR64,SR64);
	RIP1(A,B,C,D,E,WR65,SR65);
	RIP1(E,A,B,C,D,WR66,SR66);
	RIP1(D,E,A,B,C,WR67,SR67);
	RIP1(C,D,E,A,B,WR68,SR68);
	RIP1(B,C,D,E,A,WR69,SR69);
	RIP1(A,B,C,D,E,WR70,SR70);
	RIP1(E,A,B,C,D,WR71,SR71);
	RIP1(D,E,A,B,C,WR72,SR72);
	RIP1(C,D,E,A,B,WR73,SR73);
	RIP1(B,C,D,E,A,WR74,SR74);
	RIP1(A,B,C,D,E,WR75,SR75);
	RIP1(E,A,B,C,D,WR76,SR76);
	RIP1(D,E,A,B,C,WR77,SR77);
	RIP1(C,D,E,A,B,WR78,SR78);
	RIP1(B,C,D,E,A,WR79,SR79);

	D     =ctx->B+c+D;
	ctx->B=ctx->C+d+E;
	ctx->C=ctx->D+e+A;
	ctx->D=ctx->E+a+B;
	ctx->E=ctx->A+b+C;
	ctx->A=D;

		}
	}
#endif
