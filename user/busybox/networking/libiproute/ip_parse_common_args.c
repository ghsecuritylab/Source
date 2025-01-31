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
 * ip.c		"ip" utility frontend.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Alexey Kuznetsov, <kuznet@ms2.inr.ac.ru>
 *
 *
 * Changes:
 *
 * Rani Assaf <rani@magic.metawire.com> 980929:	resolve addresses
 */

#include "ip_common.h"  /* #include "libbb.h" is inside */
#include "utils.h"

family_t preferred_family = AF_UNSPEC;
smallint oneline;
char _SL_;

char **ip_parse_common_args(char **argv)
{
	static const char ip_common_commands[] ALIGN1 =
		"oneline" "\0"
		"family" "\0"
		"4" "\0"
		"6" "\0"
		"0" "\0"
		;
	enum {
		ARG_oneline,
		ARG_family,
		ARG_IPv4,
		ARG_IPv6,
		ARG_packet,
	};
	static const family_t af_numbers[] = { AF_INET, AF_INET6, AF_PACKET };
	int arg;

	while (*argv) {
		char *opt = *argv;

		if (opt[0] != '-')
			break;
		opt++;
		if (opt[0] == '-') {
			opt++;
			if (!opt[0]) { /* "--" */
				argv++;
				break;
			}
		}
		arg = index_in_substrings(ip_common_commands, opt);
		if (arg < 0)
			bb_show_usage();
		if (arg == ARG_oneline) {
			oneline = 1;
			argv++;
			continue;
		}
		if (arg == ARG_family) {
			static const char families[] ALIGN1 =
				"inet" "\0" "inet6" "\0" "link" "\0";
			argv++;
			if (!*argv)
				bb_show_usage();
			arg = index_in_strings(families, *argv);
			if (arg < 0)
				invarg(*argv, "protocol family");
			/* now arg == 0, 1 or 2 */
		} else {
			arg -= ARG_IPv4;
			/* now arg == 0, 1 or 2 */
		}
		preferred_family = af_numbers[arg];
		argv++;
	}
	_SL_ = oneline ? '\\' : '\n';
	return argv;
}
