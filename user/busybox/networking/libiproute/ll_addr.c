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
 * ll_addr.c
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Alexey Kuznetsov, <kuznet@ms2.inr.ac.ru>
 */

#include <net/if_arp.h>

#include "libbb.h"
#include "rt_names.h"
#include "utils.h"


const char *ll_addr_n2a(unsigned char *addr, int alen, int type, char *buf, int blen)
{
	int i;
	int l;

	if (alen == 4 &&
	    (type == ARPHRD_TUNNEL || type == ARPHRD_SIT || type == ARPHRD_IPGRE)) {
		return inet_ntop(AF_INET, addr, buf, blen);
	}
	l = 0;
	for (i=0; i<alen; i++) {
		if (i==0) {
			snprintf(buf+l, blen, ":%02x"+1, addr[i]);
			blen -= 2;
			l += 2;
		} else {
			snprintf(buf+l, blen, ":%02x", addr[i]);
			blen -= 3;
			l += 3;
		}
	}
	return buf;
}

int ll_addr_a2n(unsigned char *lladdr, int len, char *arg)
{
	if (strchr(arg, '.')) {
		inet_prefix pfx;
		if (get_addr_1(&pfx, arg, AF_INET)) {
			bb_error_msg("\"%s\" is invalid lladdr", arg);
			return -1;
		}
		if (len < 4) {
			return -1;
		}
		memcpy(lladdr, pfx.data, 4);
		return 4;
	} else {
		int i;

		for (i=0; i<len; i++) {
			int temp;
			char *cp = strchr(arg, ':');
			if (cp) {
				*cp = 0;
				cp++;
			}
			if (sscanf(arg, "%x", &temp) != 1 || (temp < 0 || temp > 255)) {
				bb_error_msg("\"%s\" is invalid lladdr", arg);
				return -1;
			}
			lladdr[i] = temp;
			if (!cp) {
				break;
			}
			arg = cp;
		}
		return i+1;
	}
}
