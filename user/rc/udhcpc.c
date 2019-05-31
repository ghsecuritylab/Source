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
 *
 * udhcpc scripts
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
#include <errno.h>
#include <net/if.h>

#include <nvram/bcmnvram.h>

#include <shared.h>

#include "rc.h"
#include "rc_event.h"

static char udhcpstate[8];

static int expires(unsigned int in)
{
	time_t now;
	FILE *fp;
	char tmp[100];

	time(&now);
	snprintf(tmp, sizeof(tmp), "/tmp/udhcpc.expires");
	if (!(fp = fopen(tmp, "w"))) {
		perror(tmp);
		return errno;
	}
	fprintf(fp, "%d", (unsigned int) now + in);
	fclose(fp);
	return 0;
}

/* 
 * deconfig: This argument is used when udhcpc starts, and when a
 * leases is lost. The script should put the interface in an up, but
 * deconfigured state.
*/
static int deconfig(void)
{
	char *_ifname = safe_getenv("interface");

	expires(0);

	if (nvram_match("wan_route_x", "1")) {
		char *_proto = nvram_safe_get("wan_proto");

		if (!strcmp(_proto, "pptp") || !strcmp(_proto, "l2tp"))
			logmessage("dhcp client", "skipping resetting IP address to 0.0.0.0");
		else
			ifconfig(_ifname, IFUP, "0.0.0.0", NULL);

		wan_down(_ifname);
	}
	else
		lan_down(_ifname);

	logmessage("dhcp client", "[%s] re-initialize", udhcpstate);

	return 0;
}

/*
 * bound: This argument is used when udhcpc moves from an unbound, to
 * a bound state. All of the paramaters are set in enviromental
 * variables, The script should configure the interface, and set any
 * other relavent parameters (default gateway, dns server, etc).
*/
static int bound(void)
{
	int _wan_route_x = atoi(nvram_safe_get("wan_route_x"));
	char *_ifname = safe_getenv("interface");
	char *value;
	char tmp[100], prefix[] = "lanXXXXXXXXXX_";

	if (_wan_route_x == 1)
		sprintf(prefix, "wan_");
	else
		sprintf(prefix, "lan_");

	if ((value = getenv("ip")))
		nvram_set(strcat_r(prefix, "ipaddr", tmp), trim_r(value));
	if ((value = getenv("subnet")))
		nvram_set(strcat_r(prefix, "netmask", tmp), trim_r(value));
	if ((value = getenv("router")))
		nvram_set(strcat_r(prefix, "gateway", tmp), trim_r(value));
	if ((value = getenv("dns")))
		nvram_set(strcat_r(prefix, "dns", tmp), trim_r(value));
	if ((value = getenv("wins")))
		nvram_set(strcat_r(prefix, "wins", tmp), trim_r(value));
	if ((value = getenv("routes")))
		nvram_set(strcat_r(prefix, "routes", tmp), trim_r(value));
	if ((value = getenv("domain")))
		nvram_set(strcat_r(prefix, "domain", tmp), trim_r(value));
	if ((value = getenv("lease"))) {
		nvram_set(strcat_r(prefix, "lease", tmp), trim_r(value));
		expires(atoi(value));
	}

	fprintf(stderr, "Get lease %s/%s from %s\n", 
			nvram_safe_get(strcat_r(prefix, "ipaddr", tmp)), 
			nvram_safe_get(strcat_r(prefix, "netmask", tmp)), 
			nvram_safe_get(strcat_r(prefix, "gateway", tmp)));
	logmessage("dhcp client", "[%s IP] %s/%s from %s", 
			udhcpstate, 
			nvram_safe_get(strcat_r(prefix, "ipaddr", tmp)), 
			nvram_safe_get(strcat_r(prefix, "netmask", tmp)), 
			nvram_safe_get(strcat_r(prefix, "gateway", tmp)));

	ifconfig(_ifname, IFUP, 
			nvram_safe_get(strcat_r(prefix, "ipaddr", tmp)), 
			nvram_safe_get(strcat_r(prefix, "netmask", tmp)));

	if (_wan_route_x == 1)
		wan_up(_ifname);
	else {
		set_unknown_domain_ip(nvram_safe_get("lan_ipaddr"), nvram_safe_get("lan_netmask"));
		lan_up(_ifname);
		sleep(1);

		int need_link_DownUp = 0;
		if (!nvram_match("dnsqmode", "1") 
				|| strcmp(nvram_safe_get("lan_gateway_old"), nvram_safe_get("lan_gateway"))) {
			need_link_DownUp = 1;
			nvram_set("lan_gateway_old", nvram_safe_get("lan_gateway"));
		}
		nvram_set("dnsqmode", "1");
		track_set(ASUS_HIJ_SET_TARGET_IP, nvram_safe_get("lan_ipaddr"));
		restart_dnsmasq(need_link_DownUp);

#ifdef AUDIO_SUPPORT
		/* restart dmr */
		stop_audiod();
		start_audiod();
#endif

#ifdef WPA_ENTERPRISE_SUPPORT
		if (nvram_match("sw_mode", "3"))
			start_8021x();
#endif
	}

	stop_ntpc();
	start_ntpc();

	notify_rc("start_webs_update");

	return 0;
}

/*
 * renew: This argument is used when a DHCP lease is renewed. All of
 * the paramaters are set in enviromental variables. This argument is
 * used when the interface is already configured, so the IP address,
 * will not change, however, the other DHCP paramaters, such as the
 * default gateway, subnet mask, and dns server may change.
 */
static int renew(void)
{
	bound();

	return 0;
}

int udhcpc_main(int argc, char **argv)
{
	if (argv[1]) strcpy(udhcpstate, argv[1]);

	if (!argv[1])
		return EINVAL;
	else if (strstr(argv[1], "deconfig"))
		return deconfig();
	else if (strstr(argv[1], "bound"))
		return bound();
	else if (strstr(argv[1], "renew"))
		return renew();
	else if (strstr(argv[1], "leasefail"))
		return 0;
	else return deconfig();
}
