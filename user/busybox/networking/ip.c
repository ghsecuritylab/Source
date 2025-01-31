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
 * Licensed under the GPL v2 or later, see the file LICENSE in this tarball.
 *
 * Authors:	Alexey Kuznetsov, <kuznet@ms2.inr.ac.ru>
 *
 *
 * Changes:
 *
 * Rani Assaf <rani@magic.metawire.com> 980929:	resolve addresses
 * Bernhard Fischer rewrote to use index_in_substr_array
 */

#include "libbb.h"

#include "libiproute/utils.h"
#include "libiproute/ip_common.h"

#if ENABLE_FEATURE_IP_ADDRESS \
 || ENABLE_FEATURE_IP_ROUTE \
 || ENABLE_FEATURE_IP_LINK \
 || ENABLE_FEATURE_IP_TUNNEL \
 || ENABLE_FEATURE_IP_RULE

static int NORETURN ip_print_help(char **argv UNUSED_PARAM)
{
	bb_show_usage();
}

static int ip_do(int (*ip_func)(char **argv), char **argv)
{
	argv = ip_parse_common_args(argv);
	return ip_func(argv);
}

#if ENABLE_FEATURE_IP_ADDRESS
int ipaddr_main(int argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
int ipaddr_main(int argc UNUSED_PARAM, char **argv)
{
	return ip_do(do_ipaddr, argv);
}
#endif
#if ENABLE_FEATURE_IP_LINK
int iplink_main(int argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
int iplink_main(int argc UNUSED_PARAM, char **argv)
{
	return ip_do(do_iplink, argv);
}
#endif
#if ENABLE_FEATURE_IP_ROUTE
int iproute_main(int argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
int iproute_main(int argc UNUSED_PARAM, char **argv)
{
	return ip_do(do_iproute, argv);
}
#endif
#if ENABLE_FEATURE_IP_RULE
int iprule_main(int argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
int iprule_main(int argc UNUSED_PARAM, char **argv)
{
	return ip_do(do_iprule, argv);
}
#endif
#if ENABLE_FEATURE_IP_TUNNEL
int iptunnel_main(int argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
int iptunnel_main(int argc UNUSED_PARAM, char **argv)
{
	return ip_do(do_iptunnel, argv);
}
#endif


int ip_main(int argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
int ip_main(int argc UNUSED_PARAM, char **argv)
{
	static const char keywords[] ALIGN1 =
		USE_FEATURE_IP_ADDRESS("address\0")
		USE_FEATURE_IP_ROUTE("route\0")
		USE_FEATURE_IP_LINK("link\0")
		USE_FEATURE_IP_TUNNEL("tunnel\0" "tunl\0")
		USE_FEATURE_IP_RULE("rule\0")
		;
	enum {
		USE_FEATURE_IP_ADDRESS(IP_addr,)
		USE_FEATURE_IP_ROUTE(IP_route,)
		USE_FEATURE_IP_LINK(IP_link,)
		USE_FEATURE_IP_TUNNEL(IP_tunnel, IP_tunl,)
		USE_FEATURE_IP_RULE(IP_rule,)
		IP_none
	};
	int (*ip_func)(char**) = ip_print_help;

	argv = ip_parse_common_args(argv + 1);
	if (*argv) {
		int key = index_in_substrings(keywords, *argv);
		argv++;
#if ENABLE_FEATURE_IP_ADDRESS
		if (key == IP_addr)
			ip_func = do_ipaddr;
#endif
#if ENABLE_FEATURE_IP_ROUTE
		if (key == IP_route)
			ip_func = do_iproute;
#endif
#if ENABLE_FEATURE_IP_LINK
		if (key == IP_link)
			ip_func = do_iplink;
#endif
#if ENABLE_FEATURE_IP_TUNNEL
		if (key == IP_tunnel || key == IP_tunl)
			ip_func = do_iptunnel;
#endif
#if ENABLE_FEATURE_IP_RULE
		if (key == IP_rule)
			ip_func = do_iprule;
#endif
	}
	return ip_func(argv);
}

#endif /* any of ENABLE_FEATURE_IP_xxx is 1 */
