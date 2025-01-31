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
/* vi: set sw=4 ts=4: */
/*
 * CRC32 table fill function
 * Copyright (C) 2006 by Rob Sullivan <cogito.ergo.cogito@gmail.com>
 * (I can't really claim much credit however, as the algorithm is
 * very well-known)
 *
 * The following function creates a CRC32 table depending on whether
 * a big-endian (0x04c11db7) or little-endian (0xedb88320) CRC32 is
 * required. Admittedly, there are other CRC32 polynomials floating
 * around, but Busybox doesn't use them.
 *
 * endian = 1: big-endian
 * endian = 0: little-endian
 */

#include "libbb.h"

uint32_t* FAST_FUNC crc32_filltable(uint32_t *crc_table, int endian)
{
	uint32_t polynomial = endian ? 0x04c11db7 : 0xedb88320;
	uint32_t c;
	int i, j;

	if (!crc_table)
		crc_table = xmalloc(256 * sizeof(uint32_t));

	for (i = 0; i < 256; i++) {
		c = endian ? (i << 24) : i;
		for (j = 8; j; j--) {
			if (endian)
				c = (c&0x80000000) ? ((c << 1) ^ polynomial) : (c << 1);
			else
				c = (c&1) ? ((c >> 1) ^ polynomial) : (c >> 1);
		}
		*crc_table++ = c;
	}

	return crc_table - 256;
}
