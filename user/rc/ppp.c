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
 * ppp scripts
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

/*
 * Called when link comes up
 */
int ipup_main(void)
{
	char *wan_ifname = safe_getenv("IFNAME");
	FILE *fp;
	char *value;
	char buf[256];

	umask(022);
 
	/* Touch connection file */
	if (!(fp = fopen(strcat_r("/tmp/ppp/link.", wan_ifname, buf), "a"))) {
		perror(buf);
		return errno;
	}
	fclose(fp);

	if ((value = getenv("IPLOCAL"))) {
		ifconfig(wan_ifname, IFUP, value, "255.255.255.255");
		nvram_set("wan_pppoe_ipaddr", value);
		nvram_set("wan_pppoe_netmask", "255.255.255.255");
	}
	if ((value = getenv("IPREMOTE")))
		nvram_set("wan_pppoe_gateway", value);
	strcpy(buf, "");
	if (getenv("DNS1"))
		sprintf(buf, "%s", getenv("DNS1"));
	if (getenv("DNS2"))
		sprintf(buf + strlen(buf), "%s%s", strlen(buf) ? " " : "", getenv("DNS2"));
	nvram_set("wan_dns", buf);

	wan_up(wan_ifname);
	logmessage(nvram_safe_get("wan_proto"), "connect to ISP");

	return 0;
}

/*
 * Called when link goes down
 */
int ipdown_main(void)
{
	char *wan_ifname = safe_getenv("IFNAME");
	char buf[256];

	umask(022);

	wan_down(wan_ifname);

	unlink(strcat_r("/tmp/ppp/link.", wan_ifname, buf));
	unlink("/tmp/wanstatus.log");

	preset_wan_routes(wan_ifname);

	logmessage(nvram_safe_get("wan_proto"), "Disconnected");

	return 0;
}
