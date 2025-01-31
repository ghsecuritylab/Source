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
 * JFFS2 -- Journalling Flash File System, Version 2.
 *
 * Copyright (C) 2001, 2002 Red Hat, Inc.
 *
 * Created by Arjan van de Ven <arjanv@redhat.com>
 *
 * For licensing information, see the file 'LICENCE' in this directory.
 *
 * $Id: compr_rtime.c,v 1.1.1.1 2006-10-13 02:44:17 steven Exp $
 *
 *
 * Very simple lz77-ish encoder.
 *
 * Theory of operation: Both encoder and decoder have a list of "last
 * occurrences" for every possible source-value; after sending the
 * first source-byte, the second byte indicated the "run" length of
 * matches
 *
 * The algorithm is intended to only send "whole bytes", no bit-messing.
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* _compress returns the compressed size, -1 if bigger */
int jffs2_rtime_compress(unsigned char *data_in, unsigned char *cpage_out, 
		   uint32_t *sourcelen, uint32_t *dstlen)
{
	short positions[256];
	int outpos = 0;
	int pos=0;

	memset(positions,0,sizeof(positions)); 
	
	while (pos < (*sourcelen) && outpos <= (*dstlen)-2) {
		int backpos, runlen=0;
		unsigned char value;
		
		value = data_in[pos];

		cpage_out[outpos++] = data_in[pos++];
		
		backpos = positions[value];
		positions[value]=pos;
		
		while ((backpos < pos) && (pos < (*sourcelen)) &&
		       (data_in[pos]==data_in[backpos++]) && (runlen<255)) {
			pos++;
			runlen++;
		}
		cpage_out[outpos++] = runlen;
	}

	if (outpos >= pos) {
		/* We failed */
		return -1;
	}
	
	/* Tell the caller how much we managed to compress, and how much space it took */
	*sourcelen = pos;
	*dstlen = outpos;
	return 0;
}		   


void jffs2_rtime_decompress(unsigned char *data_in, unsigned char *cpage_out,
		      uint32_t srclen, uint32_t destlen)
{
	short positions[256];
	int outpos = 0;
	int pos=0;
	
	memset(positions,0,sizeof(positions)); 
	
	while (outpos<destlen) {
		unsigned char value;
		int backoffs;
		int repeat;
		
		value = data_in[pos++];
		cpage_out[outpos++] = value; /* first the verbatim copied byte */
		repeat = data_in[pos++];
		backoffs = positions[value];
		
		positions[value]=outpos;
		if (repeat) {
			if (backoffs + repeat >= outpos) {
				while(repeat) {
					cpage_out[outpos++] = cpage_out[backoffs++];
					repeat--;
				}
			} else {
				memcpy(&cpage_out[outpos],&cpage_out[backoffs],repeat);
				outpos+=repeat;		
			}
		}
	}		
}		   


