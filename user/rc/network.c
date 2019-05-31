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
 * Network services
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
#include <signal.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/route.h>

#include <nvram/bcmnvram.h>

#include <shared.h>

#include "rc.h"
#include "rc_event.h"

void config_loopback(void)
{
	/* Bring up loopback interface */
	ifconfig("lo", IFUP, "127.0.0.1", "255.0.0.0");

	/* Add to routing table */
	route_add("lo", 0, "127.0.0.0", "0.0.0.0", "255.0.0.0");
}

void deconfig_loopback(void)
{
	/* Bring down loopback interface */
	ifconfig("lo", 0, NULL, NULL);

	/* Del to routing table */
	route_del("lo", 0, "127.0.0.0", "0.0.0.0", "255.0.0.0");
}

int
ifconfig(char *name, int flags, char *addr, char *netmask)
{
        int s;
        struct ifreq ifr;
        struct in_addr in_addr, in_netmask, in_broadaddr;

        /* Open a raw socket to the kernel */
        if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
                goto err;

        /* Set interface name */
        strncpy(ifr.ifr_name, name, IFNAMSIZ);

        /* Set interface flags */
        ifr.ifr_flags = flags;
        if (ioctl(s, SIOCSIFFLAGS, &ifr) < 0)
                goto err;

        /* Set IP address */
        if (addr) {
                inet_aton(addr, &in_addr);
                sin_addr(&ifr.ifr_addr).s_addr = in_addr.s_addr;
                ifr.ifr_addr.sa_family = AF_INET;
                if (ioctl(s, SIOCSIFADDR, &ifr) < 0)
                        goto err;
        }
        /* Set IP netmask and broadcast */
        if (addr && netmask) {
                inet_aton(netmask, &in_netmask);
                sin_addr(&ifr.ifr_netmask).s_addr = in_netmask.s_addr;
                ifr.ifr_netmask.sa_family = AF_INET;
                if (ioctl(s, SIOCSIFNETMASK, &ifr) < 0)
                        goto err;

                in_broadaddr.s_addr = (in_addr.s_addr & in_netmask.s_addr) | ~in_netmask.s_addr;
                sin_addr(&ifr.ifr_broadaddr).s_addr = in_broadaddr.s_addr;
                ifr.ifr_broadaddr.sa_family = AF_INET;
                if (ioctl(s, SIOCSIFBRDADDR, &ifr) < 0)
                        goto err;
        }

        close(s);

        return 0;

 err:
        close(s);
        perror(name);
        return errno;
}

void insmod_wifi_ko(void)
{
#if (defined(CONFIG_DEFAULTS_ASUS_EAN66) || defined(CONFIG_DEFAULTS_ASUS_RPN14) || defined(CONFIG_DEFAULTS_ASUS_RPN53) || defined(CONFIG_DEFAULTS_ASUS_WMPN12))
	eval("insmod","-q" ,"/usr/lib/rt2860v2_ap.ko");
#elif (defined(CONFIG_DEFAULTS_ASUS_RPN12))
	eval("insmod","-q" ,"/usr/lib/mt_wifi.ko");
#elif (defined(CONFIG_DEFAULTS_ASUS_RPAC52))
	eval("insmod","-q" ,"/usr/lib/rt2860v2_ap.ko");
	eval("insmod","-q" ,"/usr/lib/MT7610_ap.ko");
#elif (defined(CONFIG_DEFAULTS_ASUS_RPAC56))
	eval("insmod","-q" ,"/usr/lib/mt_wifi.ko");
	eval("insmod","-q" ,"/usr/lib/rlt_wifi.ko");
#else
#error Invalid Model
#endif
}

void rmmod_wifi_ko(void)
{
#if (defined(CONFIG_DEFAULTS_ASUS_EAN66) || defined(CONFIG_DEFAULTS_ASUS_RPN14) || defined(CONFIG_DEFAULTS_ASUS_RPN53) || defined(CONFIG_DEFAULTS_ASUS_WMPN12))
	eval("rmmod", "rt2860v2_ap");
#elif (defined(CONFIG_DEFAULTS_ASUS_RPN12))
	eval("rmmod", "mt_wifi");
#elif (defined(CONFIG_DEFAULTS_ASUS_RPAC52))
	eval("rmmod", "rt2860v2_ap");
	eval("rmmod", "MT7610_ap");
#elif (defined(CONFIG_DEFAULTS_ASUS_RPAC56))
	eval("rmmod", "mt_wifi");
	eval("rmmod", "rlt_wifi");
#endif
}

#ifdef REPEATER_5G_WORKAROUND
void restart_apcliX(int band)
{
	int n;
	char wif[8], *wif_next, aif[8], *aif_next;
#ifdef SWMODE_REPEATER_EXPRESSWAY_SUPPORT
	int re_expressway = atoi(nvram_safe_get("re_expressway"));
#endif
#ifdef SWMODE_REPEATER_MEDIABRIDGE_SUPPORT
	int re_mediabridge = atoi(nvram_safe_get("re_mediabridge"));
#endif

	for2each(n, wif, nvram_safe_get("wl_ifnames"), wif_next, 
			aif, nvram_safe_get("sta_ifnames"), aif_next) {
		if (band == n)
			break;
	}

	ifconfig(wif, 0, NULL, NULL);
	ifconfig(aif, 0, NULL, NULL);

#if (defined(CONFIG_DEFAULTS_ASUS_RPAC52))
	eval("rmmod", "MT7610_ap");
	eval("insmod","-q" ,"/usr/lib/MT7610_ap.ko");
#endif

#ifdef SWMODE_REPEATER_EXPRESSWAY_SUPPORT
	if (IsExpressWayApcli()) {
		ifconfig(wif, IFUP, NULL, NULL);
		ifconfig(aif, IFUP, NULL, NULL);
		ifconfig(wif, 0, NULL, NULL);
	}
	else {
#endif
		ifconfig(wif, IFUP, NULL, NULL);
		ifconfig(aif, IFUP, NULL, NULL);
#ifdef SWMODE_REPEATER_MEDIABRIDGE_SUPPORT
		if (IsMediaBridge())
			ifconfig(wif, 0, NULL, NULL);
		else
#endif
			eval("brctl", "addif", nvram_safe_get("lan_ifname"), wif);
#ifdef SWMODE_REPEATER_EXPRESSWAY_SUPPORT
	}
#endif
}
#endif

void restart_apcli(int wps_done)
{
	char lan_ifname[8];
	int n, is_iNIC;
	char wif[8], *wif_next, aif[8], *aif_next;
	char buf[32];
	unsigned char ea[ETHER_ADDR_LEN];
#ifdef SWMODE_REPEATER_EXPRESSWAY_SUPPORT
	int re_expressway = atoi(nvram_safe_get("re_expressway"));
#endif
#ifdef SWMODE_REPEATER_MEDIABRIDGE_SUPPORT
	int re_mediabridge = atoi(nvram_safe_get("re_mediabridge"));
#endif

#ifdef LED_DRV_SUPPORT
	nvram_set("led_drv_stage", "4");
#endif
	nvram_set("sta0_connStatus", "0");
	nvram_set("sta1_connStatus", "0");
#ifdef SWMODE_REPEATER_EXPRESSWAY_SUPPORT
	nvram_set("wl_unit", re_expressway==2?"1":"0");
#endif

	memset(lan_ifname, 0x0, sizeof(lan_ifname));
	strcpy(lan_ifname, nvram_safe_get("lan_ifname"));

#ifdef WPS_SUPPORT
	if (wps_done) {
		if (!check_if_file_exist("/var/run/re_wpsc0.pid") 
				|| (!check_if_file_exist("/var/run/re_wpsc1.pid") 
					&& strstr(nvram_safe_get("www_support"), "5G") != NULL)) {
			int _x_Setting = atoi(nvram_safe_get("x_Setting"));

			if (check_if_file_exist("/var/run/re_wpsc0.pid"))
				ClearWscStaOldProfile(0);
			else if (check_if_file_exist("/var/run/re_wpsc1.pid"))
				ClearWscStaOldProfile(1);

			if (_x_Setting == 0) {
				stop_httpd();
				nvram_set("x_Setting", "1");
			}
			nvram_commit();

#ifdef WEB_REDIRECT
			restart_wanduck();
#endif
			if (_x_Setting == 0)
				start_httpd();
		}
	}
	else {
#endif
		lan_down(lan_ifname);
		stop_apcli_monitor();
		enable_QIS_delay_bridge();

		strncpy(buf, nvram_safe_get("wl0_macaddr"), sizeof(buf));
		eval("ifconfig", lan_ifname, "hw", "ether", buf);
		if (ether_atoe(buf, ea))
			track_set(ASUS_HIJ_SET_DUT_MAC, ea);

#ifdef WEB_REDIRECT
		restart_wanduck();
#endif
#ifdef WPS_SUPPORT
	}
#endif

	for2each(n, wif, nvram_safe_get("wl_ifnames"), wif_next, 
			aif, nvram_safe_get("sta_ifnames"), aif_next) {
		if (!strncmp(wif, "rai", 3))
			is_iNIC = 1;
		else
			is_iNIC = 0;
		gen_ralink_config(n, is_iNIC);

#ifdef WPS_SUPPORT
		if (wps_done)
			doSystem("rm -f /var/run/re_wpsc%d.pid", n);
#endif

		ifconfig(wif, 0, NULL, NULL);
		ifconfig(aif, 0, NULL, NULL);
	}

	rmmod_wifi_ko();
	insmod_wifi_ko();

	for2each(n, wif, nvram_safe_get("wl_ifnames"), wif_next, 
			aif, nvram_safe_get("sta_ifnames"), aif_next) {
#ifdef SWMODE_REPEATER_EXPRESSWAY_SUPPORT
		if (IsExpressWayCli()) {
			ifconfig(wif, IFUP, NULL, NULL);
			eval("brctl", "addif", lan_ifname, wif);
		}
		else if (IsExpressWayApcli()) {
			ifconfig(wif, IFUP, NULL, NULL);
			ifconfig(aif, IFUP, NULL, NULL);
			ifconfig(wif, 0, NULL, NULL);
		}
		else {
#endif
			ifconfig(wif, IFUP, NULL, NULL);
			ifconfig(aif, IFUP, NULL, NULL);
#ifdef SWMODE_REPEATER_MEDIABRIDGE_SUPPORT
			if (IsMediaBridge())
				ifconfig(wif, 0, NULL, NULL);
			else
#endif
				eval("brctl", "addif", lan_ifname, wif);
#ifdef SWMODE_REPEATER_EXPRESSWAY_SUPPORT
		}
#endif
	}

	start_apcli_monitor();
#ifdef LED_DRV_SUPPORT
	nvram_set("led_drv_stage", "1");
#endif
}

#ifdef SWMODE_ADAPTER_SUPPORT
void restart_sta(int wps_done)
{
	int n, is_iNIC;
	char wif[8], *next;

#ifdef LED_DRV_SUPPORT
	nvram_set("led_drv_stage", "4");
#endif

#ifdef WPS_SUPPORT
	if (wps_done) {
		nvram_commit();
#ifdef WEB_REDIRECT
		restart_wanduck();
#endif
	}
#endif

	for1each(n, wif, nvram_safe_get("wl_ifnames"), next) {
		if (!strncmp(wif, "rai", 3))
			is_iNIC = 1;
		else
			is_iNIC = 0;
		gen_ralink_config(n, is_iNIC);

		ifconfig(wif, 0, NULL, NULL);
		ifconfig(wif, IFUP, NULL, NULL);
	}

#ifdef LED_DRV_SUPPORT
	nvram_set("led_drv_stage", "1");
#endif
}
#endif

static void _restart_wifi(int n, char *wif, char *aif, int _sw_mode)
{
	int is_iNIC;
	char tmp[128], prefix[] = "wlXXXXXXXXXX_";
#ifdef SWMODE_REPEATER_EXPRESSWAY_SUPPORT
	int re_expressway = atoi(nvram_safe_get("re_expressway"));
#endif

	if (!strncmp(wif, "rai", 3))
		is_iNIC = 1;
	else
		is_iNIC = 0;
	gen_ralink_config(n, is_iNIC);

	/* Wireless interface & driver down/up */
#ifdef SWMODE_REPEATER_EXPRESSWAY_SUPPORT
	if (IsExpressWayCli()) {
		ifconfig(wif, 0, NULL, NULL);
		ifconfig(wif, IFUP, NULL, NULL);
	}
	else if (IsExpressWayApcli()) {
		ifconfig(aif, 0, NULL, NULL);
		ifconfig(wif, IFUP, NULL, NULL);
		ifconfig(aif, IFUP, NULL, NULL);
		ifconfig(wif, 0, NULL, NULL);
	}
	else {
#endif
		if (_sw_mode == 2 || _sw_mode == 5)
			ifconfig(aif, 0, NULL, NULL);
		ifconfig(wif, 0, NULL, NULL);
		ifconfig(wif, IFUP, NULL, NULL);
		if (_sw_mode == 2 || _sw_mode == 5)
			ifconfig(aif, IFUP, NULL, NULL);
#ifdef SWMODE_REPEATER_EXPRESSWAY_SUPPORT
	}
#endif

	snprintf(prefix, sizeof(prefix), "wl%d_", n);

	if (nvram_match(strcat_r(prefix, "radio_x", tmp), "0"))
		wireless_radio(n, wif, 0);
	else
		doSystem("nvram set sw%d_radio=1", n);
	nvram_set(strcat_r(prefix, "reload_svc_wl", tmp), "1");
}

void restart_wifi(int band)
{
	int _sw_mode = atoi(nvram_safe_get("sw_mode"));
	int n;
	char wif[8], *wif_next, aif[8], *aif_next;

#ifdef LED_DRV_SUPPORT
	nvram_set("led_drv_stage", "4");
#endif
	if (_sw_mode == 2)
		eval("killall", "-SIGTSTP", "apcli_monitor");	//pause apcli_monitor

	/* shorten the interface into forwarding */
        eval("brctl", "setfd", "br0", "0.1");
        eval("brctl", "sethello", "br0", "0.1");

	for2each(n, wif, nvram_safe_get("wl_ifnames"), wif_next,
			aif, nvram_safe_get("sta_ifnames"), aif_next) {
#ifndef DUAL_BAND_NONCONCURRENT
		if (band == -1 || n == band)
#endif
			_restart_wifi(n, wif, aif, _sw_mode);
	}

#ifdef WPA_ENTERPRISE_SUPPORT
	start_8021x();
#endif
	stop_lltd();
	start_lltd();

	/* recover the interface */
        eval("brctl", "setfd", "br0", "15");
        eval("brctl", "sethello", "br0", "2");

	if (_sw_mode == 2) {
		for1each(n, aif, nvram_safe_get("sta_ifnames"), aif_next) {
#ifndef DUAL_BAND_NONCONCURRENT
			if (band == -1 || n == band)
#endif
				init_apcli_monitor_para(n, aif);
		}

		eval("killall", "-SIGCONT", "apcli_monitor");	//resume apcli_monitor
	}

#ifdef LED_DRV_SUPPORT
	nvram_set("led_drv_stage", "1");
#endif
}

static int
route_manip(int cmd, char *name, int metric, char *dst, char *gateway, char *genmask)
{
        int s;
        struct rtentry rt;

        /* Open a raw socket to the kernel */
        if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
                goto err;
	}

        /* Fill in rtentry */
        memset(&rt, 0, sizeof(rt));
        if (dst){
                inet_aton(dst, &sin_addr(&rt.rt_dst));
	}
        if (gateway){
                inet_aton(gateway, &sin_addr(&rt.rt_gateway));
	}
        if (genmask){	
                inet_aton(genmask, &sin_addr(&rt.rt_genmask));
	}
        rt.rt_metric = metric;
        rt.rt_flags = RTF_UP;
        if (sin_addr(&rt.rt_gateway).s_addr){
                rt.rt_flags |= RTF_GATEWAY;
	}
        if (sin_addr(&rt.rt_genmask).s_addr == INADDR_BROADCAST){
                rt.rt_flags |= RTF_HOST;
	}
        rt.rt_dev = name;

        /* Force address family to AF_INET */
        rt.rt_dst.sa_family = AF_INET;
        rt.rt_gateway.sa_family = AF_INET;
        rt.rt_genmask.sa_family = AF_INET;

        if (ioctl(s, cmd, &rt) < 0){
                goto err;
	}

        close(s);
        return 0;

 err:
        close(s);
        perror(name);
        return errno;
}

int
route_add(char *name, int metric, char *dst, char *gateway, char *genmask)
{
        return route_manip(SIOCADDRT, name, metric, dst, gateway, genmask);
}

int
route_del(char *name, int metric, char *dst, char *gateway, char *genmask)
{
        return route_manip(SIOCDELRT, name, metric, dst, gateway, genmask);
}

static int
add_routes(char *prefix, char *var, char *ifname)
{
	char word[80], *next;
	char *ipaddr, *netmask, *gateway, *metric;
	char tmp[100];

	foreach(word, nvram_safe_get(strcat_r(prefix, var, tmp)), next) {
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
		//if (inet_addr_(gateway) == INADDR_ANY)
		//	gateway = nvram_safe_get("wan_gateway");

		route_add(ifname, 0, ipaddr, gateway, netmask);
	}

	return 0;
}

static void
add_wanx_routes(char *prefix, char *ifname, int metric)
{
       char *routes, *tmp;
       char buf[30];

       char ipaddr[] = "255.255.255.255";
       char gateway[] = "255.255.255.255";
       char netmask[] = "255.255.255.255";

       if (!nvram_match("dr_enable_x", "1"))
               return;

       /* routes */
       routes = strdup(nvram_safe_get(strcat_r(prefix, "routes", buf)));
       for (tmp = routes; tmp && *tmp; )
       {
               char *ipaddr = strsep(&tmp, " ");
               char *gateway = strsep(&tmp, " ");
               if (gateway) {
                       route_add(ifname, metric + 1, ipaddr, gateway, netmask);
               }
       }
       free(routes);
}

static int
del_routes(char *prefix, char *var, char *ifname)
{
	char word[80], *next;
	char *ipaddr, *netmask, *gateway, *metric;
	char tmp[100];

	foreach(word, nvram_safe_get(strcat_r(prefix, var, tmp)), next) {
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
		//if (inet_addr_(gateway) == INADDR_ANY)
		//	gateway = nvram_safe_get("wan_gateway");

		route_del(ifname, atoi(metric) + 1, ipaddr, gateway, netmask);
	}

	return 0;
}

void	// oleg patch , add
start_igmpproxy(char *wan_ifname)
{
	static char *igmpproxy_conf = "/tmp/igmpproxy.conf";
	struct stat     st_buf;
	FILE            *fp;
	char *altnet = nvram_safe_get("mr_altnet_x");
	char *altnet_mask;

	fprintf(stderr, "%s(%s)\n", __FUNCTION__, wan_ifname);

	if (!nvram_match("mr_enable_x", "1"))
        	return;

	if ((fp = fopen(igmpproxy_conf, "w")) == NULL) {
        	perror(igmpproxy_conf);
        	return;
	}
               
	if(altnet && strlen(altnet) > 0)
		altnet_mask = altnet;
	else
		altnet_mask = "0.0.0.0/0";
	fprintf(stderr, "%s: altnet_mask = %s\n", __FUNCTION__, altnet_mask);
	fprintf(fp, "# automagically generated from web settings\n"
        	"quickleave\n\n"
        	"phyint %s upstream\n"
        	"\taltnet %s\n\n"
        	"phyint %s downstream\n\n", 
        	wan_ifname, 
        	altnet_mask, 
        	nvram_safe_get("lan_ifname") ? : "br0");

	fclose(fp);
       		
	eval("/bin/igmpproxy", "-c", igmpproxy_conf);
}

static int
add_lan_routes(char *lan_ifname)
{
	return add_routes("lan_", "route", lan_ifname);
}

static int
del_lan_routes(char *lan_ifname)
{
	return del_routes("lan_", "route", lan_ifname);
}

void create_ifname(int _sw_mode)
{
	char *lan_ifname = nvram_safe_get("lan_ifname");

	fprintf(stderr, "%s\n", __FUNCTION__);

	eval("brctl", "addbr", lan_ifname);

#ifdef VLAN
	if (_sw_mode == 1) {
		/* set vlan i/f name to style "vlan<ID>" */
		eval("vconfig", "set_name_type", "VLAN_PLUS_VID_NO_PAD");

		/* create LAN interface */
		eval("vconfig", "add", "eth2", "1");

		/* create WAN interface */
		eval("vconfig", "add", "eth2", "2");
		if (nvram_invmatch("wan_hwaddr", ""))
			eval("ifconfig", "vlan2", "hw", "ether", nvram_safe_get("wan_hwaddr"));
		else
			eval("ifconfig", "vlan2", "hw", "ether", nvram_safe_get("wl0_macaddr"));

		config_esw(LLLLW);
	}
#endif
}

void destroy_ifname(int _sw_mode)
{
	char *lan_ifname = nvram_safe_get("lan_ifname");

	fprintf(stderr, "%s\n", __FUNCTION__);

#ifdef VLAN
	if (_sw_mode == 1) {
		restore_esw();

		/* destroy WAN interface */
		eval("vconfig", "rem", "vlan2");

		/* destroy LAN interface */
		eval("vconfig", "rem", "vlan1");
	}
#endif

	eval("brctl", "delbr", lan_ifname);
}

void start_wlan(int _sw_mode)
{
	char *lan_ifname = nvram_safe_get("lan_ifname");
	int n;
	char wif[8], *wif_next, aif[8], *aif_next;
	char tmp[128], prefix[] = "wlXXXXXXXXXX_";
	char *nv;
	unsigned char dut_mac[ETHER_ADDR_LEN];
#ifdef SWMODE_REPEATER_EXPRESSWAY_SUPPORT
	int re_expressway = atoi(nvram_safe_get("re_expressway"));
#endif
#ifdef SWMODE_REPEATER_MEDIABRIDGE_SUPPORT
	int re_mediabridge = atoi(nvram_safe_get("re_mediabridge"));
#endif

	fprintf(stderr, "%s\n", __FUNCTION__);

	for2each(n, wif, nvram_safe_get("wl_ifnames"), wif_next,
			aif, nvram_safe_get("sta_ifnames"), aif_next) {
		snprintf(prefix, sizeof(prefix), "wl%d_", n);
		nv = nvram_safe_get(strcat_r(prefix, "macaddr", tmp));

		if (n == 0) {
			eval("ifconfig", "eth2", "hw", "ether", nv);
			eval("ifconfig", lan_ifname, "hw", "ether", nv);
			if (ether_atoe(nv, dut_mac))
				track_set(ASUS_HIJ_SET_DUT_MAC, dut_mac);
		}
		eval("ifconfig", wif, "hw", "ether", nv);
		ifconfig(wif, IFUP, NULL, NULL);

		if (_sw_mode == 2 
#ifdef SWMODE_HOTSPOT_SUPPORT
				|| _sw_mode == 5
#endif
		) {
#ifdef SWMODE_REPEATER_EXPRESSWAY_SUPPORT
			if (IsExpressWayCli())
				;
			else if (IsExpressWayApcli()) {
				ifconfig(aif, IFUP, NULL, NULL);
				ifconfig(wif, 0, NULL, NULL);
			}
			else
#endif
			{
				ifconfig(aif, IFUP, NULL, NULL);
#ifdef SWMODE_REPEATER_MEDIABRIDGE_SUPPORT
				if (IsMediaBridge())
					ifconfig(wif, 0, NULL, NULL);
#endif
			}
		}

#ifdef SWMODE_REPEATER_EXPRESSWAY_SUPPORT
		if (IsExpressWayApcli())
			;
		else
#endif
#ifdef SWMODE_REPEATER_MEDIABRIDGE_SUPPORT
		if (IsMediaBridge())
			;
		else
#endif
			eval("brctl", "addif", lan_ifname, wif);
	}
}

void stop_wlan(int _sw_mode)
{
	char *lan_ifname = nvram_safe_get("lan_ifname");
	int n;
	char wif[8], *wif_next, aif[8], *aif_next;

	fprintf(stderr, "%s\n", __FUNCTION__);

	for2each(n, wif, nvram_safe_get("wl_ifnames"), wif_next,
			aif, nvram_safe_get("sta_ifnames"), aif_next) {
		if (_sw_mode == 2 
#ifdef SWMODE_HOTSPOT_SUPPORT
				|| _sw_mode == 5
#endif
		)
			ifconfig(aif, 0, NULL, NULL);

		ifconfig(wif, 0, NULL, NULL);
	}
}

void start_lan(int _sw_mode)
{
	char *lan_ifname = nvram_safe_get("lan_ifname");

	fprintf(stderr, "%s\n", __FUNCTION__);

	/* Bring up and configure LAN interface */
	ifconfig("eth2", IFUP, NULL, NULL);
#ifdef VLAN
	if (_sw_mode == 1) {
		ifconfig("vlan1", IFUP, NULL, NULL);
		eval("brctl", "addif", lan_ifname, "vlan1");
	}
	else
#endif
		eval("brctl", "addif", lan_ifname, "eth2");
	ifconfig(lan_ifname, IFUP, nvram_safe_get("lan_ipaddr"), nvram_safe_get("lan_netmask"));

	/* 
	* Configure DHCP connection. The DHCP client will run 
	* 'udhcpc bound'/'udhcpc deconfig' upon finishing IP address 
	* renew and release.
	*/
	if (nvram_match("wan_route_x", "0")) {
#ifdef SWMODE_ADAPTER_SUPPORT
		if (_sw_mode == 4) {
			eval("route", "add", "default", "dev", lan_ifname);

#ifdef WPA_SUPPLICANT_SUPPORT
			if (nvram_match("sta0_auth_mode", "wpa") 
					|| nvram_match("sta0_auth_mode", "wpa2")) {
				fprintf(stderr, "wpa supplicant!!!\n");
				setWPAconnect();
			}
			else
				fprintf(stderr, "sta connect!!!\n"); //bypass
#endif
		}
		else 
#endif
		if (_sw_mode == 3) {
			if (nvram_match("lan_proto_x", "1"))
				start_udhcpc(lan_ifname, NULL, nvram_safe_get("lan_hostname"));
			else
				lan_up(lan_ifname);
		}
	}
	else {
		/* Install lan specific static routes */
		add_lan_routes(lan_ifname);
		update_lan_status();
		default_filter_setting();
	}
}

void stop_lan(int _sw_mode)
{
	char *lan_ifname = nvram_safe_get("lan_ifname");

	fprintf(stderr, "%s\n", __FUNCTION__);

	/* clear filter table */
	if (nvram_match("wan_route_x", "1")) {
		eval("iptables", "-F");
		eval("iptables", "-X");
		eval("iptables", "-Z");
	}

	/* Remove static routes */
	del_lan_routes(lan_ifname);

	/* Bring down LAN interface */
	ifconfig(lan_ifname, 0, NULL, NULL);
#ifdef VLAN
	if (_sw_mode == 1)
		ifconfig("vlan1", 0, NULL, NULL);
#endif
	ifconfig("eth2", 0, NULL, NULL);
}

void start_wan(int _sw_mode)
{
	int s;
	struct ifreq ifr;
	char *wan_ifname = nvram_safe_get("wan_ifname");
	char *wan_proto = nvram_safe_get("wan_proto");

	/* check if we need to setup WAN */
	if (nvram_match("wan_route_x", "0") || nvram_match("wan_enable", "0"))
		return;

	fprintf(stderr, "%s\n", __FUNCTION__);

#ifdef VLAN
	/* Bring up WAN interface */
	if (_sw_mode == 1)
		ifconfig("vlan2", IFUP, NULL, NULL);
#endif

	if (nvram_match("lan_stp", "1")) {
		fprintf(stderr, "resume stp forwarding delay and hello time\n");
		eval("brctl", "setfd", "br0", "15");
		eval("brctl", "sethello", "br0", "2");
	}

	update_wan_status(0);

	/* Enable Forwarding */
	FILE *fp;
	if ((fp = fopen("/proc/sys/net/ipv4/ip_forward", "r+"))) {
		fputc('1', fp);
		fclose(fp);
	} else
		perror("/proc/sys/net/ipv4/ip_forward");

	/* 
	* Configure PPPoE connection. The PPPoE client will run 
	* ip-up/ip-down scripts upon link's connect/disconnect.
	*/
	if (strcmp(wan_proto, "pppoe") == 0 
			|| strcmp(wan_proto, "pptp") == 0 
			|| strcmp(wan_proto, "l2tp") == 0) {
		/* Create links */
		mkdir("/tmp/ppp", 0777);
		mkdir("/tmp/ppp/peers", 0777);
		symlink("/sbin/rc", "/tmp/ppp/ip-up");
		symlink("/sbin/rc", "/tmp/ppp/ip-down");

		start_pppoe_relay(wan_ifname);

		int demand = atoi(nvram_safe_get("wan_pppoe_idletime")) 
				&& strcmp(wan_proto, "l2tp") /* L2TP does not support idling */;

		/* update demand option */
		nvram_set("wan_pppoe_demand", demand?"1":"0");

		if (strcmp(wan_proto, "pppoe") != 0) {
			if (nvram_match("wan_dhcpenable_x", "1"))
				start_udhcpc(wan_ifname, NULL, nvram_safe_get("wan_hostname"));
			else {
				/* Assign static IP address to i/f */
				ifconfig(wan_ifname, IFUP, nvram_safe_get("wan_ipaddr"), nvram_safe_get("wan_netmask"));
				/* We are done configuration */
				wan_up(wan_ifname);
			}
		}
		/* launch pppoe client daemon */
		start_pppd_or_l2tpd(wan_proto);

		/* ppp interface name is referenced from this point on */
		wan_ifname = nvram_safe_get("wan_pppoe_ifname");
		/* Pretend that the WAN interface is up */
		if (nvram_match("wan_pppoe_demand", "1")) {
			int timeout = 5;
			/* Wait for pppx to be created */
			while (ifconfig(wan_ifname, IFUP, NULL, NULL) && timeout--)
				sleep(1);

			/* Retrieve IP info */
			if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
				return;
			strncpy(ifr.ifr_name, wan_ifname, IFNAMSIZ);

			/* Set temporary IP address */
			if (ioctl(s, SIOCGIFADDR, &ifr))
				perror(wan_ifname);

			nvram_set("wan_ipaddr", inet_ntoa(sin_addr(&ifr.ifr_addr)));
			nvram_set("wan_netmask", "255.255.255.255");

			/* Set temporary P-t-P address */
			if (ioctl(s, SIOCGIFDSTADDR, &ifr))
				perror(wan_ifname);
			nvram_set("wan_gateway", inet_ntoa(sin_addr(&ifr.ifr_dstaddr)));

			close(s);

			/* 
			* Preset routes so that traffic can be sent to proper pppx even before 
			* the link is brought up.
			*/
			preset_wan_routes(wan_ifname);
		}
	}
	/* 
	* Configure DHCP connection. The DHCP client will run 
	* 'udhcpc bound'/'udhcpc deconfig' upon finishing IP address 
	* renew and release.
	*/
	else if (strcmp(wan_proto, "dhcp") == 0) {
		start_udhcpc(wan_ifname, NULL, nvram_safe_get("wan_hostname"));
	}
	/* Configure static IP connection. */
	else if ((strcmp(wan_proto, "static") == 0)) {
		/* Assign static IP address to i/f */
		ifconfig(wan_ifname, IFUP, nvram_safe_get("wan_ipaddr"), nvram_safe_get("wan_netmask"));
		/* We are done configuration */
		wan_up(wan_ifname);
	}
}

void stop_wan(void)
{
	/* check if we need to stop WAN */
	if (nvram_match("wan_route_x", "0"))
		return;

	fprintf(stderr, "%s\n", __FUNCTION__);

	/* Shutdown and kill all possible tasks */
	eval_dumb("killall", "l2tpd");
	eval_dumb("killall", "pppd");
	eval_dumb("killall", "pptp");
	eval("killall", "-SIGUSR2", "udhcpc");
	eval_dumb("killall", "udhcpc");
	eval_dumb("killall", "igmpproxy");

	/* Bring down WAN interfaces */
	eval("ifconfig", nvram_safe_get("wan_ifname"), "0.0.0.0");
	ifconfig(nvram_safe_get("wan_pppoe_ifname"), 0, NULL, NULL);

	/* Remove dynamically created links */
	eval("rm", "-rf", "/tmp/ppp");

	update_wan_status(0);
}

int update_resolvconf(void)
{
	FILE *fp;
	char word[256], *next;

	fprintf(stderr, "%s\n", __FUNCTION__);

	if (!(fp = fopen("/etc/resolv.conf", "w"))) {
		perror("/etc/resolv.conf");
		return errno;
	}

	/* Write resolv.conf with upstream nameservers */
	if (nvram_match("wan_dnsenable_x", "0")) {
		if (nvram_invmatch("wan_dns1_x",""))
			fprintf(fp, "nameserver %s\n", nvram_safe_get("wan_dns1_x"));
		if (nvram_invmatch("wan_dns2_x",""))
			fprintf(fp, "nameserver %s\n", nvram_safe_get("wan_dns2_x"));
	}
	else {
		if (nvram_match("wan_route_x", "0")) {
			foreach(word, nvram_safe_get("lan_dns"), next)
				fprintf(fp, "nameserver %s\n", word);
		}
		else {
			foreach (word, nvram_safe_get("wan_dns"), next) {
				fprintf(fp, "nameserver %s\n", word);
			}
		}

	}

	fclose(fp);
	restart_dns();

	return 0;
}

void wan_up(char *wan_ifname)
{
        char *wan_proto = nvram_safe_get("wan_proto");
	char *wan_gateway = nvram_safe_get("wan_gateway");
	char *wan_pppoe_gateway = nvram_safe_get("wan_pppoe_gateway");

	fprintf(stderr, "%s(%s)\n", __FUNCTION__, wan_ifname);

	/* default route via default gateway */
	if (!strcmp(wan_ifname, "ppp0")) {
		route_add(wan_ifname, 0, "0.0.0.0", wan_pppoe_gateway, "0.0.0.0");
		/* hack: avoid routing cycles, when both peer and server has the same IP */
		if (!strcmp(wan_proto, "pptp") || !strcmp(wan_proto, "l2tp"))
			/* delete gateway route as it's no longer needed */
			route_del(wan_ifname, 0, wan_pppoe_gateway, "0.0.0.0", "255.255.255.255");
	}
	else {
		if (!strcmp(wan_proto, "pptp") || !strcmp(wan_proto, "l2tp"))
			route_add(wan_ifname, 2, "0.0.0.0", wan_gateway, "0.0.0.0");
		else
			route_add(wan_ifname, 0, "0.0.0.0", wan_gateway, "0.0.0.0");
	}

	/* Install interface dependent static routes */
	add_routes("wan_", "route", wan_ifname);

	/* and one supplied via DHCP */
	if (!strcmp(wan_proto, "dhcp"))
		add_wanx_routes("wan_", wan_ifname, 0);

	/* Add dns servers to resolv.conf */
	update_resolvconf();

	/* Sync time */
	update_wan_status(1);

	start_firewall(wan_ifname, nvram_safe_get("lan_ifname"));

	/* start multicast router */
	if (!strcmp(wan_proto, "dhcp") || !strcmp(wan_proto, "static"))
		start_igmpproxy(wan_ifname);

	/* start ddns and upnp */
	start_ddns();
}

void wan_down(char *wan_ifname)
{
	char *wan_proto = nvram_safe_get("wan_proto");

	/* Remove default route to gateway if specified */
	route_del(wan_ifname, 0, "0.0.0.0", nvram_safe_get("wan_gateway"), "0.0.0.0");

	/* Remove interface dependent static routes */
	del_routes("wan_", "route", wan_ifname);

	/* remove resolv.conf */
	unlink("/etc/resolv.conf");

	if (!strcmp(wan_proto, "static"))
		ifconfig(wan_ifname, IFUP, NULL, NULL);

	update_wan_status(0);
}

void lan_up(char *lan_ifname)
{
	/* Set default route to gateway if specified */
	route_add(lan_ifname, 0, "0.0.0.0", 
			nvram_safe_get("lan_gateway"),
			"0.0.0.0");

	/* Add dns servers to resolv.conf */
	update_resolvconf();

#ifdef WEB_REDIRECT
	rewrite_redirect();
	rewrite_redirect_hij();
	kill_pidfile_s("/var/run/wanduck.pid", SIGUSR2);
#endif
}

void lan_down(char *lan_ifname)
{
	/* Remove default route to gateway if specified */
	route_del(lan_ifname, 0, "0.0.0.0", 
			nvram_safe_get("lan_gateway"),
			"0.0.0.0");

	/* remove resolv.conf */
	unlink("/etc/resolv.conf");
}

int preset_wan_routes(char *wan_ifname)
{
	fprintf(stderr, "%s(%s)\n", __FUNCTION__, wan_ifname);
	/* Install interface dependent static routes */
	add_routes("wan_", "route", wan_ifname);

	return 0;
}
