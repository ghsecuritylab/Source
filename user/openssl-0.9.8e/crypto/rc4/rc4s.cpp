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
//
// gettsc.inl
//
// gives access to the Pentium's (secret) cycle counter
//
// This software was written by Leonard Janke (janke@unixg.ubc.ca)
// in 1996-7 and is entered, by him, into the public domain.

#if defined(__WATCOMC__)
void GetTSC(unsigned long&);
#pragma aux GetTSC = 0x0f 0x31 "mov [edi], eax" parm [edi] modify [edx eax];
#elif defined(__GNUC__)
inline
void GetTSC(unsigned long& tsc)
{
  asm volatile(".byte 15, 49\n\t"
	       : "=eax" (tsc)
	       :
	       : "%edx", "%eax");
}
#elif defined(_MSC_VER)
inline
void GetTSC(unsigned long& tsc)
{
  unsigned long a;
  __asm _emit 0fh
  __asm _emit 31h
  __asm mov a, eax;
  tsc=a;
}
#endif      

#include <stdio.h>
#include <stdlib.h>
#include <openssl/rc4.h>

void main(int argc,char *argv[])
	{
	unsigned char buffer[1024];
	RC4_KEY ctx;
	unsigned long s1,s2,e1,e2;
	unsigned char k[16];
	unsigned long data[2];
	unsigned char iv[8];
	int i,num=64,numm;
	int j=0;

	if (argc >= 2)
		num=atoi(argv[1]);

	if (num == 0) num=256;
	if (num > 1024-16) num=1024-16;
	numm=num+8;

	for (j=0; j<6; j++)
		{
		for (i=0; i<10; i++) /**/
			{
			RC4(&ctx,numm,buffer,buffer);
			GetTSC(s1);
			RC4(&ctx,numm,buffer,buffer);
			GetTSC(e1);
			GetTSC(s2);
			RC4(&ctx,num,buffer,buffer);
			GetTSC(e2);
			RC4(&ctx,num,buffer,buffer);
			}

		printf("RC4 (%d bytes) %d %d (%d) - 8 bytes\n",num,
			e1-s1,e2-s2,(e1-s1)-(e2-s2));
		}
	}

