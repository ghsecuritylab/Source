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
 * Test program for EAP-SIM PRF
 * Copyright (c) 2004-2006, Jouni Malinen <jkmaline@cc.hut.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 */

#include "eap_sim_common.c"


static int test_eap_sim_prf(void)
{
	/* http://csrc.nist.gov/encryption/dss/Examples-1024bit.pdf */
	u8 xkey[] = {
		0xbd, 0x02, 0x9b, 0xbe, 0x7f, 0x51, 0x96, 0x0b,
		0xcf, 0x9e, 0xdb, 0x2b, 0x61, 0xf0, 0x6f, 0x0f,
		0xeb, 0x5a, 0x38, 0xb6
	};
	u8 w[] = {
		0x20, 0x70, 0xb3, 0x22, 0x3d, 0xba, 0x37, 0x2f,
		0xde, 0x1c, 0x0f, 0xfc, 0x7b, 0x2e, 0x3b, 0x49,
		0x8b, 0x26, 0x06, 0x14, 0x3c, 0x6c, 0x18, 0xba,
		0xcb, 0x0f, 0x6c, 0x55, 0xba, 0xbb, 0x13, 0x78,
		0x8e, 0x20, 0xd7, 0x37, 0xa3, 0x27, 0x51, 0x16
	};
	u8 buf[40];

	printf("Testing EAP-SIM PRF (FIPS 186-2 + change notice 1)\n");
	eap_sim_prf(xkey, buf, sizeof(buf));
	if (memcmp(w, buf, sizeof(w) != 0)) {
		printf("eap_sim_prf failed\n");
		return 1;
	}

	return 0;
}


int main(int argc, char *argv[])
{
	int errors = 0;

	errors += test_eap_sim_prf();

	return errors;
}
