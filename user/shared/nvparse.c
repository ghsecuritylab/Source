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
 * Routines for managing persistent storage of port mappings, etc.
 *
 * Copyright 2013, ASUSTek Inc.
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>

typedef unsigned char bool;

#include <nvram/typedefs.h>
#include <nvram/bcmnvram.h>
#include <shutils.h>
#include <nvparse.h>
#include <bcmconfig.h>

char *
safe_snprintf(char *str, int *len, const char *fmt, ...)
{
	va_list ap;
	int n;

	va_start(ap, fmt);
	n = vsnprintf(str, *len, fmt, ap);
	va_end(ap);

	if (n > 0) {
		str += n;
		*len -= n;
	} else if (n < 0) {
		*len = 0;
	}

	return str;
}

#ifdef __CONFIG_NAT__
bool
valid_autofw_port(const netconf_app_t *app)
{
	/* Check outbound protocol */
	if (app->match.ipproto != IPPROTO_TCP && app->match.ipproto != IPPROTO_UDP)
		return FALSE;

	/* Check outbound port range */
	if (ntohs(app->match.dst.ports[0]) > ntohs(app->match.dst.ports[1]))
		return FALSE;

	/* Check related protocol */
	if (app->proto != IPPROTO_TCP && app->proto != IPPROTO_UDP)
		return FALSE;

	/* Check related destination port range */
	if (ntohs(app->dport[0]) > ntohs(app->dport[1]))
		return FALSE;

	/* Check mapped destination port range */
	if (ntohs(app->to[0]) > ntohs(app->to[1]))
		return FALSE;

	/* Check port range size */
	if ((ntohs(app->dport[1]) - ntohs(app->dport[0])) !=
	    (ntohs(app->to[1]) - ntohs(app->to[0])))
		return FALSE;

	return TRUE;
}

bool
get_autofw_port(int which, netconf_app_t *app)
{
	char name[] = "autofw_portXXXXXXXXXX", value[1000];
	char *out_proto, *out_start, *out_end, *in_proto, *in_start, *in_end, *to_start, *to_end;
	char *enable, *desc;

	memset(app, 0, sizeof(netconf_app_t));

	/* Parse out_proto:out_start-out_end,in_proto:in_start-in_end>to_start-to_end,enable,desc */
	snprintf(name, sizeof(name), "autofw_port%d", which);
	if (!nvram_invmatch(name, ""))
		return FALSE;
	strncpy(value, nvram_get(name), sizeof(value));

	/* Check for outbound port specification */
	out_start = value;
	out_proto = strsep(&out_start, ":");
	if (!out_start)
		return FALSE;

	/* Check for related protocol specification */
	in_proto = out_start;
	out_start = strsep(&in_proto, ",");
	if (!in_proto)
		return FALSE;

	/* Check for related destination port specification */
	in_start = in_proto;
	in_proto = strsep(&in_start, ":");
	if (!in_start)
		return FALSE;

	/* Check for mapped destination port specification */
	to_start = in_start;
	in_start = strsep(&to_start, ">");
	if (!to_start)
		return FALSE;

	/* Check for enable specification */
	enable = to_start;
	to_end = strsep(&enable, ",");
	if (!enable)
		return FALSE;

	/* Check for description specification (optional) */
	desc = enable;
	enable = strsep(&desc, ",");

	/* Check for outbound port range (optional) */
	out_end = out_start;
	out_start = strsep(&out_end, "-");
	if (!out_end)
		out_end = out_start;
		
	/* Check for related destination port range (optional) */
	in_end = in_start;
	in_start = strsep(&in_end, "-");
	if (!in_end)
		in_end = in_start;

	/* Check for mapped destination port range (optional) */
	to_end = to_start;
	to_start = strsep(&to_end, "-");
	if (!to_end)
		to_end = to_start;

	/* Parse outbound protocol */
	if (!strncasecmp(out_proto, "tcp", 3))
		app->match.ipproto = IPPROTO_TCP;
	else if (!strncasecmp(out_proto, "udp", 3))
		app->match.ipproto = IPPROTO_UDP;
	else
		return FALSE;

	/* Parse outbound port range */
	app->match.dst.ports[0] = htons(atoi(out_start));
	app->match.dst.ports[1] = htons(atoi(out_end));

	/* Parse related protocol */
	if (!strncasecmp(in_proto, "tcp", 3))
		app->proto = IPPROTO_TCP;
	else if (!strncasecmp(in_proto, "udp", 3))
		app->proto = IPPROTO_UDP;
	else
		return FALSE;

	/* Parse related destination port range */
	app->dport[0] = htons(atoi(in_start));
	app->dport[1] = htons(atoi(in_end));

	/* Parse mapped destination port range */
	app->to[0] = htons(atoi(to_start));
	app->to[1] = htons(atoi(to_end));

	/* Parse enable */
	if (!strncasecmp(enable, "off", 3))
		app->match.flags = NETCONF_DISABLED;

	/* Parse description */
	if (desc)
		strncpy(app->desc, desc, sizeof(app->desc));

	/* Set interface name (match packets entering LAN interface) */
	strncpy(app->match.in.name, nvram_safe_get("lan_ifname"), IFNAMSIZ);

	/* Set LAN source port range (match packets from any source port) */
	app->match.src.ports[1] = htons(0xffff);

	/* Set target (application specific port forward) */
	app->target = NETCONF_APP;

	return valid_autofw_port(app);
}

bool
set_autofw_port(int which, const netconf_app_t *app)
{
	char name[] = "autofw_portXXXXXXXXXX", value[1000], *cur = value;
	int len;

	if (!valid_autofw_port(app))
		return FALSE;

	/* Set out_proto:out_start-out_end,in_proto:in_start-in_end>to_start-to_end,enable,desc */
	snprintf(name, sizeof(name), "autofw_port%d", which);
	len = sizeof(value);

	/* Set outbound protocol */
	if (app->match.ipproto == IPPROTO_TCP)
		cur = safe_snprintf(cur, &len, "tcp");
	else if (app->match.ipproto == IPPROTO_UDP)
		cur = safe_snprintf(cur, &len, "udp");

	/* Set outbound port */
	cur = safe_snprintf(cur, &len, ":");
	cur = safe_snprintf(cur, &len, "%d", ntohs(app->match.dst.ports[0]));
	cur = safe_snprintf(cur, &len, "-");
	cur = safe_snprintf(cur, &len, "%d", ntohs(app->match.dst.ports[1]));

	/* Set related protocol */
	cur = safe_snprintf(cur, &len, ",");
	if (app->proto == IPPROTO_TCP)
		cur = safe_snprintf(cur, &len, "tcp");
	else if (app->proto == IPPROTO_UDP)
		cur = safe_snprintf(cur, &len, "udp");

	/* Set related destination port range */
	cur = safe_snprintf(cur, &len, ":");
	cur = safe_snprintf(cur, &len, "%d", ntohs(app->dport[0]));
	cur = safe_snprintf(cur, &len, "-");
	cur = safe_snprintf(cur, &len, "%d", ntohs(app->dport[1]));

	/* Set mapped destination port range */
	cur = safe_snprintf(cur, &len, ">");
	cur = safe_snprintf(cur, &len, "%d", ntohs(app->to[0]));
	cur = safe_snprintf(cur, &len, "-");
	cur = safe_snprintf(cur, &len, "%d", ntohs(app->to[1]));

	/* Set enable */
	cur = safe_snprintf(cur, &len, ",");
	if (app->match.flags & NETCONF_DISABLED)
		cur = safe_snprintf(cur, &len, "off");
	else
		cur = safe_snprintf(cur, &len, "on");

	/* Set description */
	if (*app->desc) {
		cur = safe_snprintf(cur, &len, ",");
		cur = safe_snprintf(cur, &len, app->desc);
	}

	/* Do it */
	if (nvram_set(name, value))
		return FALSE;

	return TRUE;
}

bool
del_autofw_port(int which)
{
	char name[] = "autofw_portXXXXXXXXXX";

	snprintf(name, sizeof(name), "autofw_port%d", which);
	return (nvram_unset(name) == 0) ? TRUE : FALSE;
}

bool
valid_forward_port(const netconf_nat_t *nat)
{
	/* Check WAN destination port range */
	if (ntohs(nat->match.dst.ports[0]) > ntohs(nat->match.dst.ports[1]))
		return FALSE;

	/* Check protocol */
	if (nat->match.ipproto != IPPROTO_TCP && nat->match.ipproto != IPPROTO_UDP)
		return FALSE;

	/* Check LAN IP address */
	if (nat->ipaddr.s_addr == htonl(0))
		return FALSE;

	/* Check LAN destination port range */
	if (ntohs(nat->ports[0]) > ntohs(nat->ports[1]))
		return FALSE;

	/* Check port range size */
	if ((ntohs(nat->match.dst.ports[1]) - ntohs(nat->match.dst.ports[0])) !=
	    (ntohs(nat->ports[1]) - ntohs(nat->ports[0])))
		return FALSE;

	return TRUE;
}

bool
get_forward_port(int which, netconf_nat_t *nat)
{
	char name[] = "forward_portXXXXXXXXXX", value[1000];
	char *wan_port0, *wan_port1, *lan_ipaddr, *lan_port0, *lan_port1, *proto;
	char *enable, *desc;

	memset(nat, 0, sizeof(netconf_nat_t));

	/* Parse wan_port0-wan_port1>lan_ipaddr:lan_port0-lan_port1[:,]proto[:,]enable[:,]desc */
	snprintf(name, sizeof(name), "forward_port%d", which);
	if (!nvram_invmatch(name, ""))
		return FALSE;
	strncpy(value, nvram_get(name), sizeof(value));

	/* Check for LAN IP address specification */
	lan_ipaddr = value;
	wan_port0 = strsep(&lan_ipaddr, ">");
	if (!lan_ipaddr)
		return FALSE;

	/* Check for LAN destination port specification */
	lan_port0 = lan_ipaddr;
	lan_ipaddr = strsep(&lan_port0, ":");
	if (!lan_port0)
		return FALSE;

	/* Check for protocol specification */
	proto = lan_port0;
	lan_port0 = strsep(&proto, ":,");
	if (!proto)
		return FALSE;

	/* Check for enable specification */
	enable = proto;
	proto = strsep(&enable, ":,");
	if (!enable)
		return FALSE;

	/* Check for description specification (optional) */
	desc = enable;
	enable = strsep(&desc, ":,");

	/* Check for WAN destination port range (optional) */
	wan_port1 = wan_port0;
	wan_port0 = strsep(&wan_port1, "-");
	if (!wan_port1)
		wan_port1 = wan_port0;

	/* Check for LAN destination port range (optional) */
	lan_port1 = lan_port0;
	lan_port0 = strsep(&lan_port1, "-");
	if (!lan_port1)
		lan_port1 = lan_port0;

	/* Parse WAN destination port range */
	nat->match.dst.ports[0] = htons(atoi(wan_port0));
	nat->match.dst.ports[1] = htons(atoi(wan_port1));

	/* Parse LAN IP address */
	(void) inet_aton(lan_ipaddr, &nat->ipaddr);

	/* Parse LAN destination port range */
	nat->ports[0] = htons(atoi(lan_port0));
	nat->ports[1] = htons(atoi(lan_port1));

	/* Parse protocol */
	if (!strncasecmp(proto, "tcp", 3))
		nat->match.ipproto = IPPROTO_TCP;
	else if (!strncasecmp(proto, "udp", 3))
		nat->match.ipproto = IPPROTO_UDP;
	else
		return FALSE;

	/* Parse enable */
	if (!strncasecmp(enable, "off", 3))
		nat->match.flags = NETCONF_DISABLED;

	/* Parse description */
	if (desc)
		strncpy(nat->desc, desc, sizeof(nat->desc));
	/* Set WAN source port range (match packets from any source port) */
	nat->match.src.ports[1] = htons(0xffff);

	/* Set target (DNAT) */
	nat->target = NETCONF_DNAT;

	return valid_forward_port(nat);
}	

bool
set_forward_port(int which, const netconf_nat_t *nat)
{
	char name[] = "forward_portXXXXXXXXXX", value[1000], *cur = value;
	int len;

	if (!valid_forward_port(nat))
		return FALSE;

	/* Set wan_port0-wan_port1>lan_ipaddr:lan_port0-lan_port1,proto,enable,desc */
	snprintf(name, sizeof(name), "forward_port%d", which);
	len = sizeof(value);

	/* Set WAN destination port range */
	cur = safe_snprintf(cur, &len, "%d", ntohs(nat->match.dst.ports[0]));
	cur = safe_snprintf(cur, &len, "-");
	cur = safe_snprintf(cur, &len, "%d", ntohs(nat->match.dst.ports[1]));

	/* Set LAN IP address */
	cur = safe_snprintf(cur, &len, ">");
	cur = safe_snprintf(cur, &len, inet_ntoa(nat->ipaddr));

	/* Set LAN destination port range */
	cur = safe_snprintf(cur, &len, ":");
	cur = safe_snprintf(cur, &len, "%d", ntohs(nat->ports[0]));
	cur = safe_snprintf(cur, &len, "-");
	cur = safe_snprintf(cur, &len, "%d", ntohs(nat->ports[1]));

	/* Set protocol */
	cur = safe_snprintf(cur, &len, ",");
	if (nat->match.ipproto == IPPROTO_TCP)
		cur = safe_snprintf(cur, &len, "tcp");
	else if (nat->match.ipproto == IPPROTO_UDP)
		cur = safe_snprintf(cur, &len, "udp");

	/* Set enable */
	cur = safe_snprintf(cur, &len, ",");
	if (nat->match.flags & NETCONF_DISABLED)
		cur = safe_snprintf(cur, &len, "off");
	else
		cur = safe_snprintf(cur, &len, "on");

	/* Set description */
	if (*nat->desc) {
		cur = safe_snprintf(cur, &len, ",");
		cur = safe_snprintf(cur, &len, nat->desc);
	}

	/* Do it */
	if (nvram_set(name, value))
		return FALSE;

	return TRUE;
}

bool
del_forward_port(int which)
{
	char name[] = "forward_portXXXXXXXXXX";

	snprintf(name, sizeof(name), "forward_port%d", which);
	return (nvram_unset(name) == 0) ? TRUE : FALSE;
}

static void
convert_forward_proto(const char *name, int ipproto)
{
	char var[1000], *next;
	char *wan_port0, *wan_port1, *lan_ipaddr, *lan_port0, *lan_port1;
	netconf_nat_t nat, unused;
	bool valid;
	int i;

	foreach(var, nvram_safe_get(name), next) {
		/* Parse wan_port0-wan_port1>lan_ipaddr:lan_port0-lan_port1 */
		lan_ipaddr = var;
		wan_port0 = strsep(&lan_ipaddr, ">");
		if (!lan_ipaddr)
			continue;
		lan_port0 = lan_ipaddr;
		lan_ipaddr = strsep(&lan_port0, ":");
		if (!lan_port0)
			continue;
		wan_port1 = wan_port0;
		wan_port0 = strsep(&wan_port1, "-");
		if (!wan_port1)
			wan_port1 = wan_port0;
		lan_port1 = lan_port0;
		lan_port0 = strsep(&lan_port1, "-");
		if (!lan_port1)
			lan_port1 = lan_port0;

		/* Set up parameters */
		memset(&nat, 0, sizeof(netconf_nat_t));
		nat.match.ipproto = ipproto;
		nat.match.dst.ports[0] = htons(atoi(wan_port0));
		nat.match.dst.ports[1] = htons(atoi(wan_port1));
		(void) inet_aton(lan_ipaddr, &nat.ipaddr);
		nat.ports[0] = htons(atoi(lan_port0));
		nat.ports[1] = htons(atoi(lan_port1));

		/* Replace an unused or invalid entry */
		for (i = 0; get_forward_port(i, &unused); i++);
		valid = set_forward_port(i, &nat);
		assert(valid);
	}

	nvram_unset(name);
}

bool
valid_filter_client(const netconf_filter_t *start, const netconf_filter_t *end)
{
	/* Check address range */
	if (start->match.src.netmask.s_addr) {
		if (start->match.src.netmask.s_addr != htonl(0xffffffff) ||
		    start->match.src.netmask.s_addr != end->match.src.netmask.s_addr)
			return FALSE;
		if (ntohl(start->match.src.ipaddr.s_addr) > ntohl(end->match.src.ipaddr.s_addr))
			return FALSE;
	}

	/* Check destination port range */
	if (ntohs(start->match.dst.ports[0]) > ntohs(start->match.dst.ports[1]) ||
	    start->match.dst.ports[0] != end->match.dst.ports[0] ||
	    start->match.dst.ports[1] != end->match.dst.ports[1])
		return FALSE;

	/* Check protocol */
	if ((start->match.ipproto != IPPROTO_TCP && start->match.ipproto != IPPROTO_UDP) ||
	    start->match.ipproto != end->match.ipproto)
		return FALSE;

	/* Check day range */
	if (start->match.days[0] < 0 || start->match.days[0] > 6 ||
	    start->match.days[1] < 0 || start->match.days[1] > 6 ||
	    start->match.days[0] != end->match.days[0] ||
	    start->match.days[1] != end->match.days[1])
		return FALSE;

	/* Check time range */
	if (start->match.secs[0] < 0 || start->match.secs[0] >= (24*60*60) ||
	    start->match.secs[1] < 0 || start->match.secs[1] >= (24*60*60) ||
	    start->match.secs[0] != end->match.secs[0] ||
	    start->match.secs[1] != end->match.secs[1])
		return FALSE;

	return TRUE;
}

bool
get_filter_client(int which, netconf_filter_t *start, netconf_filter_t *end)
{
	char name[] = "filter_clientXXXXXXXXXX", value[1000];
	char *lan_ipaddr0, *lan_ipaddr1, *lan_port0, *lan_port1, *proto;
	char *day_start, *day_end, *sec_start, *sec_end;
	char *enable, *desc;

	memset(start, 0, sizeof(netconf_filter_t));
	memset(end, 0, sizeof(netconf_filter_t));

	/* Parse [lan_ipaddr0-lan_ipaddr1|*]:lan_port0-lan_port1,proto,day_start-day_end,sec_start-sec_end,enable,desc */
	snprintf(name, sizeof(name), "filter_client%d", which);
	if (!nvram_invmatch(name, ""))
		return FALSE;
	strncpy(value, nvram_get(name), sizeof(value));

	/* Check for port specification */
	lan_port0 = value;
	lan_ipaddr0 = strsep(&lan_port0, ":");
	if (!lan_port0)
		return FALSE;

	/* Check for protocol specification */
	proto = lan_port0;
	lan_port0 = strsep(&proto, ",");
	if (!proto)
		return FALSE;

	/* Check for day specification */
	day_start = proto;
	proto = strsep(&day_start, ",");
	if (!day_start)
		return FALSE;

	/* Check for time specification */
	sec_start = day_start;
	day_start = strsep(&sec_start, ",");
	if (!sec_start)
		return FALSE;

	/* Check for enable specification */
	enable = sec_start;
	sec_start = strsep(&enable, ",");
	if (!enable)
		return FALSE;

	/* Check for description specification (optional) */
	desc = enable;
	enable = strsep(&desc, ",");

	/* Check for address range (optional) */
	lan_ipaddr1 = lan_ipaddr0; 
	lan_ipaddr0 = strsep(&lan_ipaddr1, "-");
	if (!lan_ipaddr1)
		lan_ipaddr1 = lan_ipaddr0;

	/* Check for port range (optional) */
	lan_port1 = lan_port0;
	lan_port0 = strsep(&lan_port1, "-");
	if (!lan_port1)
		lan_port1 = lan_port0;

	/* Check for day range (optional) */
	day_end = day_start;
	day_start = strsep(&day_end, "-");
	if (!day_end)
		day_end = day_start;

	/* Check for time range (optional) */
	sec_end = sec_start;
	sec_start = strsep(&sec_end, "-");
	if (!sec_end)
		sec_end = sec_start;

	/* Parse address range */
	if (*lan_ipaddr0 == '*') {
		/* Match any IP address */
		start->match.src.ipaddr.s_addr = end->match.src.ipaddr.s_addr = htonl(0);
		start->match.src.netmask.s_addr = end->match.src.netmask.s_addr = htonl(0);
	} else {
		/* Match a range of IP addresses */
		inet_aton(lan_ipaddr0, &start->match.src.ipaddr);
		inet_aton(lan_ipaddr1, &end->match.src.ipaddr);
		start->match.src.netmask.s_addr = end->match.src.netmask.s_addr = htonl(0xffffffff);
	}

	/* Parse destination port range */
	start->match.dst.ports[0] = end->match.dst.ports[0] = htons(atoi(lan_port0));
	start->match.dst.ports[1] = end->match.dst.ports[1] = htons(atoi(lan_port1));

	/* Parse protocol */
	if (!strncasecmp(proto, "tcp", 3))
		start->match.ipproto = end->match.ipproto = IPPROTO_TCP;
	else if (!strncasecmp(proto, "udp", 3))
		start->match.ipproto = end->match.ipproto = IPPROTO_UDP;
	else
		return FALSE;

	/* Parse day range */
	start->match.days[0] = end->match.days[0] = atoi(day_start);
	start->match.days[1] = end->match.days[1] = atoi(day_end);

	/* Parse time range */
	start->match.secs[0] = end->match.secs[0] = atoi(sec_start);
	start->match.secs[1] = end->match.secs[1] = atoi(sec_end);

	/* Parse enable */
	if (!strncasecmp(enable, "off", 3))
		start->match.flags = end->match.flags = NETCONF_DISABLED;

	/* Parse description */
	if (desc) {
		strncpy(start->desc, desc, sizeof(start->desc));
		strncpy(end->desc, desc, sizeof(end->desc));
	}

	/* Set interface name (match packets entering LAN interface) */
	strncpy(start->match.in.name, nvram_safe_get("lan_ifname"), IFNAMSIZ);

	/* Set source port range (match packets from any source port) */
	start->match.src.ports[1] = end->match.src.ports[1] = htons(0xffff);

	/* Set default target (drop) */
	start->target = NETCONF_DROP;

	return valid_filter_client(start, end);
}

bool
set_filter_client(int which, const netconf_filter_t *start, const netconf_filter_t *end)
{
	char name[] = "filter_clientXXXXXXXXXX", value[1000], *cur = value;
	int len;

	if (!valid_filter_client(start, end))
		return FALSE;

	/* Set [lan_ipaddr0-lan_ipaddr1|*]:lan_port0-lan_port1,proto,day_start-day_end,sec_start-sec_end,enable,desc */
	snprintf(name, sizeof(name), "filter_client%d", which);
	len = sizeof(value);

	/* Set address range */
	if (start->match.src.ipaddr.s_addr == htonl(0) && end->match.src.ipaddr.s_addr == htonl(0) &&
	    start->match.src.netmask.s_addr == htonl(0) && end->match.src.netmask.s_addr == htonl(0))
		cur = safe_snprintf(cur, &len, "*");
	else {
		cur = safe_snprintf(cur, &len, inet_ntoa(start->match.src.ipaddr));
		cur = safe_snprintf(cur, &len, "-");
		cur = safe_snprintf(cur, &len, inet_ntoa(end->match.src.ipaddr));
	}

	/* Set port range */
	cur = safe_snprintf(cur, &len, ":");
	cur = safe_snprintf(cur, &len, "%d", ntohs(start->match.dst.ports[0]));
	cur = safe_snprintf(cur, &len, "-");
	cur = safe_snprintf(cur, &len, "%d", ntohs(start->match.dst.ports[1]));

	/* Set protocol */
	cur = safe_snprintf(cur, &len, ",");
	if (start->match.ipproto == IPPROTO_TCP)
		cur = safe_snprintf(cur, &len, "tcp");
	else if (start->match.ipproto == IPPROTO_UDP)
		cur = safe_snprintf(cur, &len, "udp");

	/* Set day range */
	cur = safe_snprintf(cur, &len, ",");
	cur = safe_snprintf(cur, &len, "%d", start->match.days[0]);
	cur = safe_snprintf(cur, &len, "-");
	cur = safe_snprintf(cur, &len, "%d", start->match.days[1]);

	/* Set time range */
	cur = safe_snprintf(cur, &len, ",");
	cur = safe_snprintf(cur, &len, "%d", start->match.secs[0]);
	cur = safe_snprintf(cur, &len, "-");
	cur = safe_snprintf(cur, &len, "%d", start->match.secs[1]);

	/* Set enable */
	cur = safe_snprintf(cur, &len, ",");
	if (start->match.flags & NETCONF_DISABLED)
		cur = safe_snprintf(cur, &len, "off");
	else
		cur = safe_snprintf(cur, &len, "on");

	/* Set description */
	if (*start->desc) {
		cur = safe_snprintf(cur, &len, ",");
		cur = safe_snprintf(cur, &len, start->desc);
	}

	/* Do it */
	if (nvram_set(name, value))
		return FALSE;

	return TRUE;
}

bool
del_filter_client(int which)
{
	char name[] = "filter_clientXXXXXXXXXX";

	snprintf(name, sizeof(name), "filter_client%d", which);
	return (nvram_unset(name) == 0) ? TRUE : FALSE;
}
#endif	/* __CONFIG_NAT__ */

/*
 * wl_wds<N> is authentication protocol dependant.
 * when auth is "psk":
 *	wl_wds<N>=mac,role,crypto,auth,ssid,passphrase
 */
bool
get_wds_wsec(int unit, int which, char *mac, char *role, 
	     char *crypto, char *auth, ...)
{
	char name[] = "wlXXXXXXX_wdsXXXXXXX", value[1000], *next;
	
	snprintf(name, sizeof(name), "wl%d_wds%d", unit, which);
	strncpy(value, nvram_safe_get(name), sizeof(value));
	next = value;

	/* separate mac */
	strcpy(mac, strsep(&next, ","));
	if (!next)
		return FALSE;
	
	/* separate role */
	strcpy(role, strsep(&next, ","));
	if (!next)
		return FALSE;

	/* separate crypto */
	strcpy(crypto, strsep(&next, ","));
	if (!next)
		return FALSE;

	/* separate auth */
	strcpy(auth, strsep(&next, ","));
	if (!next)
		return FALSE;

	if (!strcmp(auth, "psk")) {
		va_list va;

		va_start(va, auth);

		/* separate ssid */
		strcpy(va_arg(va, char *), strsep(&next, ","));
		if (!next)
			goto fail;

		/* separate passphrase */
		strcpy(va_arg(va, char *), next);

		va_end(va);
		return TRUE;
fail:
		va_end(va);
		return FALSE;
	}

	return FALSE;
}

bool
set_wds_wsec(int unit, int which, char *mac, char *role,
	     char *crypto, char *auth, ...)
{
	char name[] = "wlXXXXXXX_wdsXXXXXXX", value[10000];

	snprintf(name, sizeof(name), "wl%d_wds%d", unit, which);
	snprintf(value, sizeof(value), "%s,%s,%s,%s",
		 mac, role, crypto, auth);

	if (!strcmp(auth, "psk")) {
		int offset;
		char *str1, *str2;
		va_list va;
		
		va_start(va, auth);
		offset = strlen(value);
		str1 = va_arg(va, char *);
		str2 = va_arg(va, char *);
		
		snprintf(&value[offset], sizeof(value) - offset, ",%s,%s", str1, str2);
		va_end(va);

		if (nvram_set(name, value))
			return FALSE;

		return TRUE;
	}

	return FALSE;
}

bool
del_wds_wsec(int unit, int which)
{
	char name[] = "wlXXXXXXX_wdsXXXXXXX";

	snprintf(name, sizeof(name), "wl%d_wds%d", unit, which);

	nvram_unset(name);

	return TRUE;
}

#ifdef __CONFIG_NAT__
static void
convert_filter_ip(void)
{
	char var[1000], *next;
	char *lan_ipaddr0, *lan_ipaddr1;
	netconf_filter_t start, end, unused;
	bool valid;
	int i;

	foreach(var, nvram_safe_get("filter_ip"), next) {
		/* Parse lan_ipaddr0-lan_ipaddr1 */
		lan_ipaddr1 = var;
		lan_ipaddr0 = strsep(&lan_ipaddr1, "-");
		if (!lan_ipaddr0 || !lan_ipaddr1)
			continue;

		/* Set up parameters */
		memset(&start, 0, sizeof(netconf_filter_t));
		(void) inet_aton(lan_ipaddr0, &start.match.src.ipaddr);
		start.match.src.netmask.s_addr = htonl(0xffffffff);
		start.match.dst.ports[1] = htons(0xffff);
		memcpy(&end, &start, sizeof(netconf_filter_t));
		(void) inet_aton(lan_ipaddr1, &end.match.src.ipaddr);

		/* Replace an unused or invalid entry */
		start.match.ipproto = end.match.ipproto = IPPROTO_TCP;
		for (i = 0; get_filter_client(i, &unused, &unused); i++);
		valid = set_filter_client(i, &start, &end);
		assert(valid);

		/* Replace an unused or invalid entry */
		start.match.ipproto = end.match.ipproto = IPPROTO_UDP;
		for (; get_filter_client(i, &unused, &unused); i++);
		valid = set_filter_client(i, &start, &end);
		assert(valid);
	}

	nvram_unset("filter_ip");
}

static void
convert_filter_proto(const char *name, int ipproto)
{
	char var[1000], *next;
	char *lan_ipaddr, *lan_port0, *lan_port1;
	netconf_filter_t start, end, unused;
	bool valid;
	int i;

	foreach(var, nvram_safe_get(name), next) {
		/* Parse [lan_ipaddr|*]:lan_port0-lan_port1 */
		lan_port0 = var;
		lan_ipaddr = strsep(&lan_port0, ":");
		if (!lan_ipaddr || !lan_port0)
			continue;
		lan_port1 = lan_port0;
		lan_port0 = strsep(&lan_port1, "-");
		if (!lan_port0 || !lan_port1)
			continue;

		/* Set up parameters */
		memset(&start, 0, sizeof(netconf_filter_t));
		(void) inet_aton(lan_ipaddr, &start.match.src.ipaddr);
		start.match.src.netmask.s_addr = htonl(0xffffffff);
		start.match.ipproto = ipproto;
		start.match.dst.ports[0] = htons(atoi(lan_port0));
		start.match.dst.ports[1] = htons(atoi(lan_port1));
		memcpy(&end, &start, sizeof(netconf_filter_t));

		/* Replace an unused or invalid entry */
		for (i = 0; get_filter_client(i, &unused, &unused); i++);
		valid = set_filter_client(i, &start, &end);
		assert(valid);
	}

	nvram_unset(name);
}

static void
convert_filter_port(void)
{
	char name[1000], *value, *colon;
	netconf_filter_t start, end, unused;
	bool valid;
	int i, j;

	/* Maximum number of filter_port entries was 6 */
	for (i = 1; i <= 6; i++) {
		snprintf(name, sizeof(name), "filter_port%d", i);
		if (!(value = nvram_get(name)) || !*value)
			goto fail;

		memset(&start, 0, sizeof(netconf_filter_t));
		memset(&end, 0, sizeof(netconf_filter_t));

		/* Only the last bytes of IP addresses were stored in NVRAM */
		(void) inet_aton(nvram_safe_get("lan_ipaddr"), &start.match.src.ipaddr);
		(void) inet_aton(nvram_safe_get("lan_ipaddr"), &end.match.src.ipaddr);

		/* id (unused) */
		if (!(colon = strchr(value, ':')))
			goto fail;
		value = colon + 1;

		/* active */
		if (!(colon = strchr(value, ':')))
			goto fail;
		if (!strtoul(value, NULL, 0))
			start.match.flags = end.match.flags = NETCONF_DISABLED;
		value = colon + 1;

		/* flags (unused) */
		if (!(colon = strchr(value, ':')))
			goto fail;
		value = colon + 1;

		/* from_ip */
		if (!(colon = strchr(value, ':')))
			goto fail;
		start.match.src.ipaddr.s_addr = htonl((ntohl(start.match.src.ipaddr.s_addr) & ~0xff) | strtoul(value, NULL, 0));
		value = colon + 1;

		/* to_ip */
		if (!(colon = strchr(value, ':')))
			goto fail;
		end.match.src.ipaddr.s_addr = htonl((ntohl(end.match.src.ipaddr.s_addr) & ~0xff) | strtoul(value, NULL, 0));
		value = colon + 1;

		/* from_port */
		if (!(colon = strchr(value, ':')))
			goto fail;
		start.match.dst.ports[0] = end.match.dst.ports[0] = htons(strtoul(value, NULL, 0));
		value = colon + 1;

		/* to_port */
		if (!(colon = strchr(value, ':')))
			goto fail;
		start.match.dst.ports[1] = end.match.dst.ports[1] = htons(strtoul(value, NULL, 0));
		value = colon + 1;

		/* protocol */
		if (!(colon = strchr(value, ':')))
			goto fail;
		start.match.ipproto = end.match.ipproto = strtoul(value, NULL, 0);
		value = colon + 1;

		/* always */
		if (!(colon = strchr(value, ':')))
			goto fail;
		if (strtoul(value, NULL, 0))
			goto done;
		value = colon + 1;

		/* from_day_of_week */
		if (!(colon = strchr(value, ':')))
			goto fail;
		start.match.days[0] = end.match.days[0] = strtoul(value, NULL, 0);
		value = colon + 1;

		/* to_day_of_week */
		if (!(colon = strchr(value, ':')))
			goto fail;	
		start.match.days[1] = end.match.days[1] = strtoul(value, NULL, 0);
		value = colon + 1;

		/* from_time_of_day */
		if (!(colon = strchr(value, ':')))
			goto fail;
		start.match.secs[0] = end.match.secs[0] = strtoul(value, NULL, 0);
		value = colon + 1;

		/* to_time_of_day */
		start.match.secs[1] = end.match.secs[1] = strtoul(value, NULL, 0);

	done:
		/* Replace an unused or invalid entry */
		for (j = 0; get_filter_client(j, &unused, &unused); j++);
		valid = set_filter_client(j, &start, &end);
		assert(valid);

	fail:
		nvram_unset(name);
	}

	nvram_unset("filter_port");
}
#endif	/* __CONFIG_NAT__ */

static void
convert_maclist(char *prefix)
{
	char mac[1000], maclist[1000], macmode[1000];
	char *value;

	snprintf(mac, sizeof(mac), "%s_mac", prefix);
	snprintf(maclist, sizeof(maclist), "%s_maclist", prefix);
	snprintf(macmode, sizeof(maclist), "%s_macmode", prefix);

	if ((value = nvram_get(mac))) {
		nvram_set(maclist, value);
		/* An empty *_mac used to imply *_macmode=disabled */
		if (!*value)
			nvram_set(macmode, "disabled");
		nvram_unset(mac);
	}
}

#ifdef __CONFIG_NAT__
static void
convert_autofw_port(void)
{
	char name[] = "autofw_portXXXXXXXXXX", value[1000];
	char *out_proto, *out_start, *out_end, *in_proto, *in_start, *in_end, *to_start, *to_end;
	char *enable, *desc;
	netconf_app_t app;
	bool valid;
	int i;

	/* Maximum number of autofw_port entries was 10 */
	for (i = 0; i < 10; i++) {
		memset(&app, 0, sizeof(netconf_app_t));

		/* Parse out_proto:out_port,in_proto:in_start-in_end>to_start-to_end,enable,desc */
		snprintf(name, sizeof(name), "autofw_port%d", i);
		if (!nvram_invmatch(name, ""))
			goto fail;
		strncpy(value, nvram_get(name), sizeof(value));

		/* Check for outbound port specification */
		out_start = value;
		out_proto = strsep(&out_start, ":");
		if (!out_start)
			goto fail;

		/* Check for related protocol specification */
		in_proto = out_start;
		out_start = strsep(&in_proto, ",");
		if (!in_proto)
			goto fail;

		/* Check for related destination port specification */
		in_start = in_proto;
		in_proto = strsep(&in_start, ":");
		if (!in_start)
			goto fail;

		/* Check for mapped destination port specification */
		to_start = in_start;
		in_start = strsep(&to_start, ">");
		if (!to_start)
			goto fail;

		/* Check for enable specification */
		enable = to_start;
		to_end = strsep(&enable, ",");
		if (!enable)
			goto fail;

		/* Check for description specification (optional) */
		desc = enable;
		enable = strsep(&desc, ",");

		/* Check for outbound port range (new format) */
		out_end = out_start;
		out_start = strsep(&out_end, "-");
		if (!out_end)
			out_end = out_start;
		/* !new format already! */
		else
			continue;
		
		/* Check for related destination port range (optional) */
		in_end = in_start;
		in_start = strsep(&in_end, "-");
		if (!in_end)
			in_end = in_start;

		/* Check for mapped destination port range (optional) */
		to_end = to_start;
		to_start = strsep(&to_end, "-");
		if (!to_end)
			to_end = to_start;

		/* Parse outbound protocol */
		if (!strncasecmp(out_proto, "tcp", 3))
			app.match.ipproto = IPPROTO_TCP;
		else if (!strncasecmp(out_proto, "udp", 3))
			app.match.ipproto = IPPROTO_UDP;
		else
			goto fail;

		/* Parse outbound port */
		app.match.dst.ports[0] = htons(atoi(out_start));
		app.match.dst.ports[1] = htons(atoi(out_end));

		/* Parse related protocol */
		if (!strncasecmp(in_proto, "tcp", 3))
			app.proto = IPPROTO_TCP;
		else if (!strncasecmp(in_proto, "udp", 3))
			app.proto = IPPROTO_UDP;
		else
			goto fail;

		/* Parse related destination port range */
		app.dport[0] = htons(atoi(in_start));
		app.dport[1] = htons(atoi(in_end));

		/* Parse mapped destination port range */
		app.to[0] = htons(atoi(to_start));
		app.to[1] = htons(atoi(to_end));

		/* Parse enable */
		if (!strncasecmp(enable, "off", 3))
			app.match.flags = NETCONF_DISABLED;

		/* Parse description */
		if (desc)
			strncpy(app.desc, desc, sizeof(app.desc));

		/* Set interface name (match packets entering LAN interface) */
		strncpy(app.match.in.name, nvram_safe_get("lan_ifname"), IFNAMSIZ);

		/* Set LAN source port range (match packets from any source port) */
		app.match.src.ports[1] = htons(0xffff);

		/* Set target (application specific port forward) */
		app.target = NETCONF_APP;

		/* Replace an unused or invalid entry */
		valid = set_autofw_port(i, &app);
		assert(valid);

		/* Next filter */
		continue;

	fail:
		nvram_unset(name);
	}
}

static void
convert_pppoe(void)
{
	char *old[] = {
		"pppoe_ifname",		/* PPPoE enslaved interface */
		"pppoe_username",	/* PPP username */
		"pppoe_passwd", 	/* PPP password */
		"pppoe_idletime",	/* Dial on demand max idle time (seconds) */
		"pppoe_keepalive",	/* Restore link automatically */
		"pppoe_demand",		/* Dial on demand */
		"pppoe_mru",		/* Negotiate MRU to this value */
		"pppoe_mtu",		/* Negotiate MTU to the smaller of this value or the peer MRU */
		"pppoe_service",	/* PPPoE service name */
		"pppoe_ac",		/* PPPoE access concentrator name */
		NULL
	};
	char new[] = "wan_pppoe_XXXXXXXXXXXXXXXXXX";
	char *value;
	int i;
	for (i = 0; old[i] != NULL; i ++) {
		if ((value = nvram_get(old[i]))) {
			snprintf(new, sizeof(new), "wan_%s", old[i]);
			nvram_set(new, value);
			nvram_unset(old[i]);
		}
	}
}
#endif	/* __CONFIG_NAT__ */

static void
convert_static_route(void)
{
	char word[80], *next;
	char *ipaddr, *netmask, *gateway, *metric, *ifname;
	char lan_route[1000] = "";
	char *lan_cur = lan_route;
#ifdef __CONFIG_NAT__
	char wan_route[1000] = "";
	char *wan_cur = wan_route;
#endif	/* __CONFIG_NAT__ */
	
	foreach(word, nvram_safe_get("static_route"), next) {
		netmask = word;
		ipaddr = strsep(&netmask, ":");
		if (!ipaddr || !netmask)
			continue;
		gateway = netmask;
		netmask = strsep(&gateway, ":");
		if (!netmask || !gateway)
			continue;
		metric = gateway;
		gateway = strsep(&metric, ":");
		if (!gateway || !metric)
			continue;
		ifname = metric;
		metric = strsep(&ifname, ":");
		if (!metric || !ifname)
			continue;

		if (strcmp(ifname, "lan") == 0) {
			lan_cur += snprintf(lan_cur, lan_route + sizeof(lan_route) - lan_cur, "%s%s:%s:%s:%s",
				lan_cur == lan_route ? "" : " ", ipaddr, netmask, gateway, metric);
		}
#ifdef __CONFIG_NAT__
		else if (strcmp(ifname, "wan") == 0) {
			wan_cur += snprintf(wan_cur, wan_route + sizeof(wan_route) - wan_cur, "%s%s:%s:%s:%s",
				wan_cur == wan_route ? "" : " ", ipaddr, netmask, gateway, metric);
		}
#endif	/* __CONFIG_NAT__ */
		/* what to do? */
		else {}
	}

	if (lan_cur != lan_route)
		nvram_set("lan_route", lan_route);

#ifdef __CONFIG_NAT__
	if (wan_cur != wan_route)
		nvram_set("wan_route", wan_route);
#endif	/* __CONFIG_NAT__ */

	nvram_unset("static_route");
}

/*
 * convert wl_wep - separate WEP encryption from WPA cryptos:
 * 	wep|on|restricted|off -> enabled|disabled
 * 	tkip|ase|tkip+aes -> wl_crypto
 * combine wl_auth_mode and wl_auth:
 *	wl_auth 0|1 -> wl_auth_mode open|shared (when wl_auth_mode is disabled)
 */
static void
convert_wsec(void)
{
	char prefix[] = "wlXXXXXXXXXX_", tmp[64], *wep;
	int i;

	for (i = 0; i < MAX_NVPARSE; i ++) {
		sprintf(prefix, "wl%d_", i);
		wep = nvram_get(strcat_r(prefix, "wep", tmp));
		if (!wep)
			continue;
		/* 3.60.xx */
		/* convert wep, restricted, or on to enabled */
		if (!strcmp(wep, "wep") || !strcmp(wep, "restricted") ||
		    !strcmp(wep, "on"))
			nvram_set(strcat_r(prefix, "wep", tmp), "enabled");
		/* split wep and wpa to wl_wep and wl_crypto */
		else if (!strcmp(wep, "tkip") || !strcmp(wep, "aes") ||
			 !strcmp(wep, "tkip+aes")) {
			nvram_set(strcat_r(prefix, "crypto", tmp), wep);
			nvram_set(strcat_r(prefix, "wep", tmp), "disabled");
		}
		/* treat everything else as disabled */
		else if (strcmp(wep, "enabled"))
			nvram_set(strcat_r(prefix, "wep", tmp), "disabled");
		/* combine 802.11 open/shared authentication mode with WPA to wl_auth_mode */
		if (nvram_match(strcat_r(prefix, "auth_mode", tmp), "disabled")) {
			if (nvram_match(strcat_r(prefix, "auth", tmp), "1"))
				nvram_set(strcat_r(prefix, "auth_mode", tmp), "shared");
			else
				nvram_set(strcat_r(prefix, "auth_mode", tmp), "open");
		}
		/* 3.80.xx 
		 *
		 * check only if the wl_auth_mode is set to "shared"
		 * wl_auth_carries a default value of "0" 
		 */
		/* split 802.11 auth from wl_auth_mode */
		if (nvram_match(strcat_r(prefix, "auth_mode", tmp), "shared"))
			nvram_set(strcat_r(prefix, "auth", tmp), "1");
		/* split wpa akm from wl_auth_mode */
		if (nvram_match(strcat_r(prefix, "auth_mode", tmp), "wpa"))
			nvram_set(strcat_r(prefix, "akm", tmp), "wpa");
		else if (nvram_match(strcat_r(prefix, "auth_mode", tmp), "psk"))
			nvram_set(strcat_r(prefix, "akm", tmp), "psk");
		/* preserve radius only in wl_auth_mode */
		if (nvram_invmatch(strcat_r(prefix, "auth_mode", tmp), "radius"))
			nvram_set(strcat_r(prefix, "auth_mode", tmp), "none");
	}
}

void
convert_deprecated(void)
{
#ifdef __CONFIG_NAT__
	/* forward_tcp and forward_udp were space separated lists of port forwards */
	convert_forward_proto("forward_tcp", IPPROTO_TCP);
	convert_forward_proto("forward_udp", IPPROTO_UDP);

	/* filter_ip was a space separated list of IP address ranges */
	convert_filter_ip();

	/* filter_tcp and filter_udp were space separated lists of IP address and TCP port ranges */
	convert_filter_proto("filter_tcp", IPPROTO_TCP);
	convert_filter_proto("filter_udp", IPPROTO_UDP);

	/* filter_port was a short-lived implementation of filter_client */
	convert_filter_port();
#endif	/* __CONFIG_NAT__ */

	convert_maclist("filter");
	convert_maclist("wl");

#ifdef __CONFIG_NAT__
	/* pppoe_ifname used to save the underlying WAN interface name */
	if (nvram_invmatch("pppoe_ifname", "") && nvram_match("wan_proto", "pppoe"))
		nvram_set("wan_ifname", nvram_get("pppoe_ifname"));
	nvram_unset("pppoe_ifname");

	/* autofw_port were single destination port  and now support a port range */
	convert_autofw_port();

	/* pppoe_XXXXXXXX are now wan_pppoe_XXXXXXXX */
	convert_pppoe();
#endif	/* __CONFIG_NAT__ */

	/* static_route used to save routes for LAN and WAN and is now between lan_route and wan_route */
	convert_static_route();

	/* convert wl_wep and combine wl_auth_mode and wl_auth */
	convert_wsec();
}
