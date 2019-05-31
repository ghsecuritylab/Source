/*
 * Copyright 2013, ASUSTek Inc.
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND ASUS GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <net/if.h>

#include <nvram/bcmnvram.h>

#include <shared.h>

#include "rc.h"
#include "rc_event.h"

int check_if_dir_exist(char *dir)
{
	DIR *dp;

	if (!(dp = opendir(dir)))
		return 0;
	closedir(dp);
	return 1;
}

int file_to_buf(char *path, char *buf, int len)
{
	FILE *fp;

	memset(buf, 0 , len);

	if ((fp = fopen(path, "r"))) {
		fgets(buf, len, fp);
		fclose(fp);
		return 1;
	}

	return 0;
}

int chk_same_subnet(char *ip1, char *ip2, char *sub)
{
	unsigned int addr1, addr2, submask;

	if (!*ip1 || !*ip2 || !*sub)
		return 0;

	addr1 = ntohl(inet_addr(ip1));
	addr2 = ntohl(inet_addr(ip2));
	submask = ntohl(inet_addr(sub));

	return (addr1 & submask) == (addr2 & submask);
}


/*
 * down/up LAN (port/WiFi)
 *
 */
void link_down(void)
{
	int _sw_mode = atoi(nvram_safe_get("sw_mode"));
	int n;
	char wif[8], *next;
#ifdef SWMODE_REPEATER_EXPRESSWAY_SUPPORT
	int re_expressway = atoi(nvram_safe_get("re_expressway"));
#endif
#ifdef SWMODE_REPEATER_MEDIABRIDGE_SUPPORT
	int re_mediabridge = atoi(nvram_safe_get("re_mediabridge"));
#endif

	if (_sw_mode != 1 && _sw_mode != 3)
#ifdef EXTERNAL_PHY_SUPPORT
	{
		char phy_addr[5];
		sprintf(phy_addr, "%u", ra_get_phy_addr());
		eval("mii_mgr", "-s", "-p", phy_addr, "-r", "0", "-v", "0x1940");
	}
#else
		eval("mii_mgr", "-s", "-p", STR(ESW_WAN_PORT), "-r", "0", "-v", "0x3900");
#endif

	for1each(n, wif, nvram_safe_get("wl_ifnames"), next) {
		if (_sw_mode == 1 || _sw_mode == 3 || _sw_mode == 6)
			ap_set(wif, "RadioOn=0");
		else if (
#ifndef SWMODE_REPEATER_V2_SUPPORT
				_sw_mode == 2 || 
#endif
				_sw_mode == 5) {
#ifdef SWMODE_REPEATER_EXPRESSWAY_SUPPORT
			if (IsExpressWayApcli())
				;
			else if (IsExpressWayCli())
				ap_set(wif, "RadioOn=0");
			else
#endif
#ifdef SWMODE_REPEATER_MEDIABRIDGE_SUPPORT
			if (IsMediaBridge())
				;
			else
#endif
				ifconfig(wif, 0, NULL, NULL);
		}
	}
}

void link_up(void)
{
	int _sw_mode = atoi(nvram_safe_get("sw_mode"));
	int more_sleep = 1;
	int n;
	char wif[8], *next;
#ifdef SWMODE_REPEATER_EXPRESSWAY_SUPPORT
	int re_expressway = atoi(nvram_safe_get("re_expressway"));
#endif
#ifdef SWMODE_REPEATER_MEDIABRIDGE_SUPPORT
	int re_mediabridge = atoi(nvram_safe_get("re_mediabridge"));
#endif

	if (_sw_mode != 1 && _sw_mode != 3)
#ifdef EXTERNAL_PHY_SUPPORT
	{
		char phy_addr[5];
		sprintf(phy_addr, "%u", ra_get_phy_addr());
		eval("mii_mgr", "-s", "-p", phy_addr, "-r", "0", "-v", "0x1140");
	}
#else
		eval("mii_mgr", "-s", "-p", STR(ESW_WAN_PORT), "-r", "0", "-v", "0x3300");
#endif

	for1each(n, wif, nvram_safe_get("wl_ifnames"), next) {
		if (_sw_mode == 1 || _sw_mode == 3 || _sw_mode == 6) {
			char tmp[16];
			snprintf(tmp, sizeof(tmp), "sw%d_radio", n);
			if (nvram_match(tmp, "1")) {
				if (more_sleep) {
					sleep(9);
					more_sleep = 0;
				}

				wireless_radio(n, wif, 1);
			}
		}
		else if (
#ifndef SWMODE_REPEATER_V2_SUPPORT
				_sw_mode == 2 || 
#endif
				_sw_mode == 5) {
#ifdef SWMODE_REPEATER_EXPRESSWAY_SUPPORT
			if (IsExpressWayApcli())
				;
			else if (IsExpressWayCli())
				ap_set(wif, "RadioOn=1");
			else
#endif
#ifdef SWMODE_REPEATER_MEDIABRIDGE_SUPPORT
			if (IsMediaBridge())
				;
			else
#endif
				ifconfig(wif, IFUP, NULL, NULL);
		}
	}
}


/*
 * infosvr
 *
 */
int start_infosvr(void)
{
	char *_argv[] = {"/usr/sbin/infosvr", "br0", NULL};
	pid_t _pid;

	return _eval(_argv, NULL, 0, &_pid);
}

int stop_infosvr(void)
{
	return eval_dumb("killall", "infosvr");
}


#ifdef WPA_ENTERPRISE_SUPPORT
/*
 * WPA enterprise
 * rt2860apd & rtinicapd
 *
 */
int start_8021x(void)
{
	int n;
	char wif[8], *next;
	char tmp[128], prefix[] = "wlXXXXXXX_";
	int ret = 0;

	for1each(n, wif, nvram_safe_get("wl_ifnames"), next) {
		snprintf(prefix, sizeof(prefix), "wl%d_", n);

		if (nvram_match(strcat_r(prefix, "auth_mode", tmp), "wpa") 
				|| nvram_match(strcat_r(prefix, "auth_mode", tmp), "wpa2") 
				|| nvram_match(strcat_r(prefix, "auth_mode", tmp), "radius")) {
			if (!strncmp(wif, "rai", 3)) {
				if (check_if_file_exist("/var/run/rtinicapd.pid"))
					ret += kill_pidfile_s("/var/run/rtinicapd.pid", SIGHUP);
				else {
					char *_argv[] = {"rtinicapd", NULL};
					pid_t _pid;
					ret += _eval(_argv, NULL, 0, &_pid);
				}
			}
			else {
				if (check_if_file_exist("/var/run/rt2860apd.pid"))
					ret += kill_pidfile_s("/var/run/rt2860apd.pid", SIGHUP);
				else {
					char *_argv[] = {"rt2860apd", NULL};
					pid_t _pid;
					ret += _eval(_argv, NULL, 0, &_pid);
				}
			}
		}
	}

	return ret;
}
#endif


/*
 * dnsmasq (DNS + DHCP server)
 *
 */
void simple_dhcp_range(char *ip, char *dip1, char *dip2, char *mask)
{
	struct in_addr ina;
	unsigned int new_start, new_end, lmask;

	new_start = (ntohl(inet_addr(ip)) & (lmask = ntohl(inet_addr(mask)))) + 1;
	new_end = ntohl(inet_addr(ip)) & (lmask = ntohl(inet_addr(mask))) | ~(lmask) - 1;

	ina.s_addr = htonl(new_start);
	strcpy(dip1, inet_ntoa(ina));
	ina.s_addr = htonl(new_end);
	strcpy(dip2, inet_ntoa(ina));
}

int chk_valid_startend(char *ip, char *ip1, char *ip2, char *sub)
{
	int result1, result2;

	result1 = chk_same_subnet(ip, ip1, sub);
	result2 = chk_same_subnet(ip, ip2, sub);

        if (!result1 || !result2) {
		simple_dhcp_range(ip, ip1, ip2, sub);
		return 0;
	}
	return 1;
}

void write_static_leases(char *file)
{
	FILE *fp;
	char *nv, *nvp, *b;
	char *mac, *ip;

	if (!(fp = fopen(file, "w")))
		return;

	nv = nvp = strdup(nvram_safe_get("dhcp_staticlist"));

	while (nv && (b = strsep(&nvp, "<")) != NULL) {
		if ((vstrsep(b, ">", &mac, &ip) != 2))
			continue;
		if (strlen(mac) == 0 || strlen(ip) == 0)
			continue;

		fprintf(fp, "%s %s\n", mac, ip);
	}
	free(nv);

	fclose(fp);
}

int start_dnsmasq(void)
{
	FILE *fp;
	int _wan_route_x = atoi(nvram_safe_get("wan_route_x"));
	char lan_ipaddr[16];
	char *nv;
	char *_argv[] = {"dnsmasq", NULL};
	pid_t _pid;

	fprintf(stderr, "%s\n", __FUNCTION__);

	strcpy(lan_ipaddr, nvram_safe_get("lan_ipaddr"));

	/* local Host */
	if (!(fp = fopen("/etc/hosts", "w+"))) {
		perror("/etc/hosts");
		return errno;
	}

	fprintf(fp, "127.0.0.1 localhost.localdomain localhost\n");

	nv = nvram_safe_get("hijdomain");
	if (*nv)
		fprintf(fp, "%s %s\n", lan_ipaddr, nv);
	fprintf(fp, "%s %s\n", lan_ipaddr, "www.asusnetwork.net");
	fprintf(fp, "%s %s\n", lan_ipaddr, "www.asusrouter.com");
	fprintf(fp, "%s %s\n", lan_ipaddr, "repeaters.asus.com");

	nv = nvram_safe_get("lan_hostname");
	if (*nv)
		fprintf(fp, "%s %s.%s %s\n", lan_ipaddr, nv, nvram_safe_get("lan_domain"), nv);

	fclose(fp);

	/* dnsmasq conf */
	int dnsqmode = atoi(nvram_safe_get("dnsqmode"));

	if (!(fp = fopen("/etc/dnsmasq.conf", "w+"))) {
		perror("/etc/dnsmasq.conf");
		return errno;
	}

	if (nvram_match("dhcp_enable_x", "1") && dnsqmode == 2) {
		fprintf(fp, "interface=%s\n", nvram_safe_get("lan_ifname"));

		if (_wan_route_x == 1) {
			/* lan domain */
			nv = nvram_safe_get("lan_domain");
			if (*nv)
				fprintf(fp, "expand-hosts\n"
					    "domain=%s\n", nv);
		}

		/* DHCP range */
		char dhcp_start[16], dhcp_end[16], lan_netmask[16];

		strcpy(dhcp_start, nvram_safe_get("dhcp_start"));
		strcpy(dhcp_end, nvram_safe_get("dhcp_end"));
		strcpy(lan_netmask, nvram_safe_get("lan_netmask"));

		if (!chk_valid_startend(lan_ipaddr, dhcp_start, dhcp_end, lan_netmask)) {
			fprintf(stderr, "reset DHCP range: %s ~ %s\n", dhcp_start, dhcp_end);
			nvram_set("dhcp_start", dhcp_start);
			nvram_set("dhcp_end", dhcp_end);
		}

		fprintf(fp, "dhcp-range=lan,%s,%s,%s,%ss\n", dhcp_start, dhcp_end, lan_netmask, nvram_safe_get("dhcp_lease"));
		fprintf(fp, "dhcp-leasefile=/tmp/dnsmasq.leases\n");

		if (_wan_route_x == 1) {
			/* Static IP MAC binding */
			if (nvram_match("dhcp_static_x","1")) {
				fprintf(fp, "read-ethers\n");
				write_static_leases("/etc/ethers");
			}
		}

		/* Gateway, if not set, force use lan ipaddr to avoid repeater issue */
		nv = nvram_safe_get("dhcp_gateway_x");
		nv = (*nv && inet_addr(nv)) ? nv : lan_ipaddr;
		fprintf(fp, "dhcp-option=lan,3,%s\n", nv);

		if (_wan_route_x == 1) {
			/* DNS server and additional router address */
			nv = nvram_safe_get("dhcp_dns1_x");
			if (*nv && inet_addr(nv))
				fprintf(fp, "dhcp-option=lan,6,%s,0.0.0.0\n", nv);

			/* LAN Domain */
			nv = nvram_safe_get("lan_domain");
			if (*nv)
				fprintf(fp, "dhcp-option=lan,15,%s\n", nv);

			/* WINS server */
			nv = nvram_safe_get("dhcp_wins_x");
			if (*nv && inet_addr(nv))
				fprintf(fp, "dhcp-option=lan,44,%s\n", nv);
		}

		/* Faster for moving clients, if authoritative */
		fprintf(fp, "dhcp-authoritative\n");

		/* caching */
		fprintf(fp, "cache-size=1500\n"
			    "no-negcache\n");
	}
	else
		fprintf(fp, "no-dhcp-interface=%s\n", nvram_safe_get("lan_ifname"));

        fclose(fp);

	return _eval(_argv, NULL, 0, &_pid);
}

int stop_dnsmasq(void)
{
	int ret = 0;

	ret += unlink("/etc/dnsmasq.conf");
	ret += unlink("/tmp/dnsmasq.leases");
	ret += eval_dumb("killall", "dnsmasq");

	return ret;
}

int restart_dnsmasq(int need_link_DownUp)
{
	if (need_link_DownUp) {
		link_down();
		sleep(9);
	}

	stop_dnsmasq();
	sleep(1);
	start_dnsmasq();

	if (need_link_DownUp)
		link_up();

	return 0;
}

int restart_dns(void)
{
	return eval("killall", "-SIGHUP", "dnsmasq");
}


/*
 * hijack domain name
 *
 */
int start_hij_dns(void)
{
	if (nvram_match("wan_route_x", "1"))
		return;

	track_set(ASUS_HIJ_SET_TARGET_HOST, nvram_safe_get("hijdomain"));
	track_set(ASUS_HIJ_SET_TARGET_IP, nvram_safe_get("lan_ipaddr"));
	if (nvram_match("dhcp_enable_x", "1"))
		track_set(ASUS_HIJ_ENABLE, NULL);
}

int stop_hij_dns(void)
{
	track_set(ASUS_HIJ_DISABLE, NULL);
}


/*
 * DDNS
 *
 */
int ddns_updated_main(void)
{
	FILE *fp;
	char buf[64], *ip;

	if (!(fp=fopen("/tmp/ddns.cache", "r"))) return 0;

	fgets(buf, sizeof(buf), fp);
	fclose(fp);

	if (!(ip=strchr(buf, ','))) return 0;

	nvram_set("ddns_cache", buf);
	nvram_set("ddns_ipaddr", ip+1);
	nvram_set("ddns_status", "1");
	nvram_set("ddns_updated", "1");

	logmessage("ddns", "ddns update ok");

	return 0;
}

int start_ddns(void)
{
	FILE *fp;
	char *wan_ip, *wan_ifname;
	char *ddns_cache;
	char *server;
	char *user;
	char *passwd;
	char *host;
	char *service;
	char usrstr[64];
	int wild = nvram_match("ddns_wildcard_x", "1");

	if (nvram_match("wan_route_x", "0") || nvram_match("ddns_enable_x", "0"))
		return -1;

	wan_ip = nvram_safe_get("wan_ipaddr");
	if (!wan_ip || !strcmp(wan_ip, "") || !inet_addr(wan_ip)) {
		logmessage("ddns", "WAN IP is empty");
		return -1;
	}

	if (inet_addr(wan_ip) == inet_addr(nvram_safe_get("ddns_ipaddr"))) {
		logmessage("ddns", "IP address has not changed since the last update");
		return -1;
	}

	// TODO : Check /tmp/ddns.cache to see current IP in DDNS
	// update when,
	// 	1. if ipaddr!= ipaddr in cache
	// 	
	// update
	// * nvram ddns_cache, the same with /tmp/ddns.cache

	if (!(fp=fopen("/tmp/ddns.cache", "r")) && (ddns_cache = nvram_safe_get("ddns_cache"))) {
		if ((fp = fopen("/tmp/ddns.cache", "w+"))) {
			fprintf(fp, "%s", ddns_cache);
			fclose(fp);
		}
	}

	server = nvram_safe_get("ddns_server_x");
	user = nvram_safe_get("ddns_username_x");
	passwd = nvram_safe_get("ddns_passwd_x");
	host = nvram_safe_get("ddns_hostname_x");

	if (strcmp(server, "WWW.DYNDNS.ORG")==0)
		service = "dyndns";
	else if (strcmp(server, "WWW.DYNDNS.ORG(CUSTOM)")==0)
		service = "dyndns-custom";
	else if (strcmp(server, "WWW.DYNDNS.ORG(STATIC)")==0)
		service = "dyndns-static";
	else if (strcmp(server, "WWW.TZO.COM")==0)
		service = "tzo";
	else if (strcmp(server, "WWW.ZONEEDIT.COM")==0)
		service = "zoneedit";
	else if (strcmp(server, "WWW.JUSTLINUX.COM")==0)
		service = "justlinux";
	else if (strcmp(server, "WWW.EASYDNS.COM")==0)
		service = "easydns";
	else if (strcmp(server, "WWW.DNSOMATIC.COM")==0)
		service = "dnsomatic";
	else if (strcmp(server, "WWW.TUNNELBROKER.NET")==0)
		service = "heipv6tb";
	else if (strcmp(server, "WWW.NO-IP.COM")==0)
		service = "noip";
#ifdef ASUS_DDNS
	else if (strcmp(server, "WWW.ASUS.COM")==0)
		service = "dyndns";
#endif
	else {
		logmessage("ddns", "Error ddns server name: %s\n", server);
		return 0;
	}

	sprintf(usrstr, "%s:%s", user, passwd);

	if (nvram_match("wan_proto", "pppoe") 
			|| nvram_match("wan_proto", "pptp") 
			|| nvram_match("wan_proto", "l2tp"))
		wan_ifname = nvram_safe_get("wan_pppoe_ifname");
	else
		wan_ifname = nvram_safe_get("wan_ifname");

	nvram_unset("ddns_cache");
	nvram_unset("ddns_ipaddr");
	nvram_unset("ddns_status");
	nvram_unset("ddns_updated");

	fprintf(stderr, "%s: update %s %s\n", __FUNCTION__, server, service);

#ifdef ASUS_DDNS
	if (strcmp(server, "WWW.ASUS.COM")==0) {
		char *nserver = nvram_invmatch("ddns_serverhost_x", "") ? 
					nvram_safe_get("ddns_serverhost_x") : 
					"ns1.asuscomm.com";
		eval("ez-ipupdate", 
		     "-S", service, "-i", wan_ifname, "-h", host, 
		     "-A", "2", "-s", nserver, 
		     "-e", "/sbin/ddns_updated", "-b", "/tmp/ddns.cache");
	}
	else 
#endif
	if (*service) {
		eval("ez-ipupdate", 
		     "-S", service, "-i", wan_ifname, "-h", host, 
		     "-u", usrstr, wild ? "-w" : "", 
		     "-e", "/sbin/ddns_updated", "-b", "/tmp/ddns.cache");
	}

	return 0;
}

int stop_ddns(void)
{
	return eval_dumb("killall", "ez-ipupdate");
}


/*
 * syslog & klog
 *
 */
int start_syslogd(void)
{
	char *timezone = nvram_safe_get("time_zone_x");
	char *_log_ipaddr = nvram_safe_get("log_ipaddr");
	pid_t _pid;

	if (strcmp(_log_ipaddr, "")) {
		char *_argv[] = {"/sbin/syslogd", "-m", "0", "-t", timezone, "-O", "/tmp/syslog.log", "-R", _log_ipaddr, "-L", NULL};

		return _eval(_argv, NULL, 0, &_pid);
	}
	else {
		char *_argv[] = {"/sbin/syslogd", "-m", "0", "-t", timezone, "-O", "/tmp/syslog.log", NULL};

		return _eval(_argv, NULL, 0, &_pid);
	}
}

int stop_syslogd(void)
{
	return eval_dumb("killall", "syslogd");
}

int start_klogd(void)
{
	char *_argv[] = {"/sbin/klogd", NULL};
	pid_t _pid;

	return _eval(_argv, NULL, 0, &_pid);
}

int stop_klogd(void)
{
	return eval_dumb("killall", "klogd");
}

int start_logger(void)
{
	int ret = 0;

	ret += start_syslogd();
	ret += start_klogd();

	return ret;
}

int stop_logger(void)
{
	int ret = 0;

	ret += stop_klogd();
	ret += stop_syslogd();

	return ret;
}


/*
 * networkmap
 *
 */
int start_networkmap(void)
{
	char *_argv[] = {"networkmap", NULL};
	pid_t _pid;

	if (nvram_match("en_networkmap", "0") 
#ifdef SWMODE_ADAPTER_SUPPORT
			&& nvram_invmatch("sw_mode", "4")
#endif
	)
		return 0;
	return _eval(_argv, NULL, 0, &_pid);
}

int stop_networkmap(void)
{
	int ret = 0;

	ret += unlink("/var/run/networkmap.pid");
	ret += unlink("/tmp/static_ip.inf");
	ret += eval_dumb("killall", "networkmap");
	nvram_set("networkmap_fullscan", "0");

	return ret;
}


/*
 * telnetd
 *
 */
int chpass(char *user, char *pass)
{
	if (!user)
		user = "admin";
	if (!pass)
		pass = "admin";

	doSystem("echo '%s::0:0:Adminstrator:/:/bin/sh' > /etc/passwd", user);
	doSystem("echo '%s:x:0:%s' > /etc/group", user, user);
	doSystem("echo '%s:%s' > /tmp/tmpchpw", user, pass);
	doSystem("chpasswd < /tmp/tmpchpw");
	doSystem("rm -f /tmp/tmpchpw");

	return 0;
}

int start_telnetd(int is_force)
{
	char *_argv[] = {"telnetd", NULL};
	pid_t _pid;
	int ret = 0;

	if (!is_force && !nvram_match("telnetd", "1"))
		return 0;

	chpass(nvram_safe_get("http_username"), nvram_safe_get("http_passwd"));
	ret += eval("hostname", MODEL_NAME);
	ret += _eval(_argv, NULL, 0, &_pid);

	return ret;
}

int stop_telnetd(void)
{
	return eval_dumb("killall", "telnetd");
}


/*
 * httpd
 *
 */
int start_httpd(void)
{
	char *_argv[] = {"httpd", NULL};
	pid_t _pid;
	int ret = 0;

	chdir("/www");
	ret += _eval(_argv, NULL, 0, &_pid);
	chdir("/");
	logmessage(LOGNAME, "start httpd");

	return ret;
}

int stop_httpd(void)
{
	return eval_dumb("killall", "httpd");
}


#ifdef AUDIO_SUPPORT
/*
 * audiod
 *
 */
int start_audiod(void)
{
	char opt[64];
	char *_argv[] = {"audiod", opt, NULL};
	pid_t _pid;

	sprintf(opt, "%s(%s)", nvram_get("fac_model_name"), nvram_get("wl0_macaddr"));

	return _eval(_argv, NULL, 0, &_pid);
}

int stop_audiod(void)
{
	int ret = 0;

	ret += eval_dumb("killall", "mDNSPublish");
	ret += eval_dumb("killall", "mDNSResponder");
	ret += eval_dumb("killall", "audiod"); // it will stop m3player & shairport
	ret += eval_dumb("killall", "-9", "mDNSResponder");

	return ret;
}

#ifdef INTERNET_RADIO
int clear_kbps(void)
{
	char usr_url[10];
	char buf[1024], renew[1024];
	int t, i;
	char *pt1, *pt2, *pt3;
	for (t=0 ; t<10 ; t++) {
		memset(renew,0,sizeof(renew));
		memset(buf,0,sizeof(buf));
		memset(usr_url,0,sizeof(usr_url));
		sprintf(usr_url,"usr_url%d",t);
		strcpy(buf,nvram_safe_get(usr_url));
		pt1=buf;
		for (i=0 ; i<3 ; i++) {
			if (pt2 = strstr(pt1,","))
				pt1 = pt2 + 1;
			else
				return -1;
		}
		strncpy(renew,buf,strlen(buf)-strlen(pt2));
		strcat(renew,",\"-\"");
		if (pt2 = strstr(pt1,","))
			strncat(renew,pt2,strlen(pt2));
		else
			return -1;

		nvram_unset(usr_url);
		nvram_set(usr_url,renew);
	}
	return 0;
}
#endif
#endif


/*
 * ntp
 *
 */
int start_ntpc(void)
{
#ifdef SWMODE_ADAPTER_SUPPORT
	if (nvram_match("sw_mode", "4"))
		return 0;
#endif

	char *_argv[] = {"ntp", NULL};
	pid_t _pid;

	return _eval(_argv, NULL, 0, &_pid);
}

int stop_ntpc(void)
{
	int ret = 0;

	ret += eval_dumb("killall", "ntp");
	ret += eval_dumb("killall", "ntpclient");

	return ret;
}


/*
 * lltd
 *
 */
int start_lltd(void)
{
	char *_argv[] = {"lld2d", "br0", NULL};
	pid_t _pid;

	return _eval(_argv, NULL, 0, &_pid);
}

int stop_lltd(void)
{
	return eval_dumb("killall", "lld2d");
}


/*
 * watchdog
 *
 */
int start_watchdog(void)
{
	char *_argv[] = {"watchdog", NULL};
	int _pid;

	return _eval(_argv, NULL, 0, &_pid);
}

int stop_watchdog(void)
{
	return eval_dumb("killall", "watchdog");
}


/*
 * apcli_monitor
 *
 */
int start_apcli_monitor(void)
{
	char *_argv[] = {"apcli_monitor", NULL};
	int _pid;
	int _sw_mode = atoi(nvram_safe_get("sw_mode"));

	if (_sw_mode != 2 && _sw_mode != 5)
		return 0;
	return _eval(_argv, NULL, 0, &_pid);
}

int stop_apcli_monitor(void)
{
	return eval_dumb("killall", "apcli_monitor");
}


/*
 * detectWAN_arp
 *
 */
int start_detectWAN_arp(void)
{
	char *_argv[] = {"detectWAN_arp", NULL};
	int _pid;
	int _sw_mode = atoi(nvram_safe_get("sw_mode"));

	if ((_sw_mode != 1 && _sw_mode != 3) 
			|| nvram_match("wan_enable", "0"))
		return 0;
	return _eval(_argv, NULL, 0, &_pid);
}

int stop_detectWAN_arp(void)
{
	return eval_dumb("killall", "detectWAN_arp");
}


#ifdef WEB_REDIRECT
/*
 * wanduck
 *
 */
int start_wanduck(void)
{
	if (nvram_match("wan_pppoe_relay_x", "1"))
		return -1;

	char *_argv[] = {"/sbin/wanduck", NULL};
	pid_t _pid;

	return _eval(_argv, NULL, 0, &_pid);
}

int stop_wanduck(void)
{
	kill_pidfile_s("/var/run/wanduck.pid", SIGTERM);
	return 0;
}

int restart_wanduck(void)
{
	kill_pidfile_s("/var/run/wanduck.pid", SIGUSR1);
	return 0;
}
#endif


/*
 * udhcpc
 *
 */
int start_udhcpc(char *ifname1, char *ifname2, char *hostname)
{
	char cmd[128];

	symlink("/sbin/rc", "/tmp/udhcpc");

	memset(cmd, 0x0, sizeof(cmd));
	sprintf(cmd, "udhcpc -i %s", ifname1);
	if (ifname2 != NULL)
		sprintf(cmd, "%s -w %s", cmd, ifname2);

	if (nvram_match("wan_route_x", "0"))
		sprintf(cmd, "%s -p %s -s /tmp/udhcpc", 
					cmd, 
					"/var/run/udhcpc_lan.pid");

	else
		sprintf(cmd, "%s -p %s -s /tmp/udhcpc", 
					cmd, 
					"/var/run/udhcpc_wan.pid");

	if (hostname && *hostname)
		sprintf(cmd, "%s -H %s", cmd, hostname);

	sprintf(cmd, "%s &", cmd);

	return system(cmd);
}

int stop_udhcpc(void)
{
	int ret = 0;

	ret += unlink("/tmp/udhcpc");
	ret += eval_dumb("killall", "udhcpc");

	return ret;
}


/*
 * pppd & l2tpd
 *
 */
int start_pppd_or_l2tpd(char *wan_proto)
{
	FILE *fp;
	char options[64];
	int ret = 0;

	fprintf(stderr, "%s\n", __FUNCTION__);

	sprintf(options, "/tmp/ppp/options.wan");
	/* Generate options file */
	if (!(fp = fopen(options, "w"))) {
		perror(options);
		return -1;
	}

	/* do not authenticate peer and do not use eap */
	fprintf(fp, "noauth refuse-eap\n");
	fprintf(fp, "user '%s'\n", nvram_safe_get("wan_pppoe_username"));
	fprintf(fp, "password '%s'\n", nvram_safe_get("wan_pppoe_passwd"));

	if (!strcmp(wan_proto, "pptp")) {
		fprintf(fp, "plugin pptp.so\n");
		fprintf(fp, "pptp_server '%s'\n",
				nvram_invmatch("wan_heartbeat_x", "") ?
				nvram_safe_get("wan_heartbeat_x") :
				nvram_safe_get("wan_pppoe_gateway"));
		/* see KB Q189595 -- historyless & mtu */
		fprintf(fp, "nomppe-stateful %s mtu 1400\n", nvram_safe_get("wan_pptp_options_x"));
	} else
		fprintf(fp, "nomppe nomppc\n");

	if (!strcmp(wan_proto, "pppoe")) {
		fprintf(fp, "plugin rp-pppoe.so");
		if (nvram_invmatch("wan_pppoe_service", ""))
			fprintf(fp, " rp_pppoe_service '%s'", nvram_safe_get("wan_pppoe_service"));
		if (nvram_invmatch("wan_pppoe_ac", ""))
			fprintf(fp, " rp_pppoe_ac '%s'", nvram_safe_get("wan_pppoe_ac"));
		fprintf(fp, " nic-%s\n", nvram_safe_get("wan_ifname"));
		fprintf(fp, "mru %s mtu %s\n", nvram_safe_get("wan_pppoe_mru"), nvram_safe_get("wan_pppoe_mtu"));
	}

	if (atoi(nvram_safe_get("wan_pppoe_idletime")) && nvram_match("wan_pppoe_demand", "1")) {
		fprintf(fp, "idle %s ", nvram_safe_get("wan_pppoe_idletime"));
		if (nvram_invmatch("wan_pppoe_txonly_x", "0"))
			fprintf(fp, "tx_only ");
		fprintf(fp, "demand\n");
	}
	else
		fprintf(fp, "persist\n");

	fprintf(fp, "maxfail 0\n");
	fprintf(fp, "holdoff 10\n");	// pppd re-call-time(s)

	if (nvram_invmatch("wan_dnsenable_x", "0"))
		fprintf(fp, "usepeerdns\n");

	fprintf(fp, "ipcp-accept-remote ipcp-accept-local noipdefault\n");
	fprintf(fp, "ktune\n");

	/* pppoe set these options automatically */
	/* looks like pptp also likes them */
	fprintf(fp, "default-asyncmap nopcomp noaccomp\n");

	/* pppoe disables "vj bsdcomp deflate" automagically */
	/* ccp should still be enabled - mppe/mppc requires this */
	fprintf(fp, "novj nobsdcomp nodeflate\n");

	/* echo failures */
	fprintf(fp, "lcp-echo-interval 6\n");
	fprintf(fp, "lcp-echo-failure 10\n");

	fprintf(fp, "unit 0\n");

	/* user specific options */
	fprintf(fp, "%s\n", nvram_safe_get("wan_pppoe_options_x"));

	fclose(fp);

	if (!strcmp(wan_proto, "l2tp")) {
		if (!(fp = fopen("/tmp/l2tp.conf", "w"))) {
			perror("/tmp/l2tp.conf");
			return -1;
		}

		fprintf(stderr, "\n\nbuild l2tp.conf\n");
		fprintf(fp, "# automagically generated\n"
				"global\n\n"
				"load-handler \"sync-pppd.so\"\n"
				"load-handler \"cmd.so\"\n\n"
				"section sync-pppd\n\n"
				"lac-pppd-opts \"file %s\"\n\n"
				"section peer\n"
				"peername %s\n"
				"hostname %s\n"
				"lac-handler sync-pppd\n"
				"persist yes\n"
				"maxfail %s\n"
				"holdoff %s\n"
				"section cmd\n\n",
					options,
					nvram_invmatch("wan_heartbeat_x", "") ?
						nvram_safe_get("wan_heartbeat_x") :
						nvram_safe_get("wan_pppoe_gateway"),
					nvram_invmatch("wan_hostname", "") ?
						nvram_safe_get("wan_hostname") : "localhost",
					nvram_invmatch("wan_pppoe_maxfail", "") ?
						nvram_safe_get("wan_pppoe_maxfail") : "32767",
					nvram_invmatch("wan_pppoe_holdoff", "") ?
						nvram_safe_get("wan_pppoe_holdoff") : "30");

		fclose(fp);

		/* launch l2tp */
		ret += eval("/usr/sbin/l2tpd");
		sleep(1);
		/* start-session */
		ret += eval("/usr/sbin/l2tp-control", "start-session 0.0.0.0");

		/* pppd sync nodetach noaccomp nobsdcomp nodeflate */
		/* nopcomp novj novjccomp file /tmp/ppp/options.l2tp */
	} else {
		char *_argv[] = { "/usr/sbin/pppd", "file", options, NULL};
		int _pid;

		ret += _eval(_argv, NULL, 0, &_pid);
	}

	return ret;
}


/*
 * pppoe-relay
 *
 */
int start_pppoe_relay(char *wan_if)
{
	char *_argv[] = {"/usr/sbin/pppoe-relay", "-C", "br0", "-S", wan_if, "-F", NULL};
	pid_t _pid;

	if (nvram_match("wan_pppoe_relay_x", "0"))
		return 0;

	return _eval(_argv, NULL, 0, &_pid);
}


/*
 * start/stop services
 *
 */
int start_services(void)
{
	fprintf(stderr, "%s\n", __FUNCTION__);

#if (defined(AUDIO_SUPPORT) && defined(INTERNET_RADIO))
	clear_kbps();
#endif			
#ifdef WPA_ENTERPRISE_SUPPORT
	start_8021x();
#endif
	start_httpd();
	start_dnsmasq();
	start_infosvr();
	start_logger();
	start_telnetd(0);
#ifdef AUDIO_SUPPORT
	start_audiod();
#endif
	start_lltd();
	start_watchdog();
	start_apcli_monitor();
	start_detectWAN_arp();
	start_hij_dns();
	start_networkmap();

	return 0;
}

int stop_services(void)
{
	fprintf(stderr, "%s\n", __FUNCTION__);

	stop_watchdog();
	stop_networkmap();
	stop_detectWAN_arp();
	stop_apcli_monitor();
	stop_ntpc();
	stop_lltd();
#ifdef AUDIO_SUPPORT
	stop_audiod();
#endif
	stop_telnetd();
	stop_logger();
	stop_infosvr();
	stop_dnsmasq();
	stop_httpd();
	stop_udhcpc();
	stop_hij_dns();

	return 0;
}

/*
 * rc_service handle
 *
 */
struct RC_SERVICE_PROC {
	char *name;
	int (*start_proc)(void);
	int (*stop_proc)(void);
};

static struct RC_SERVICE_PROC rsp[] = {
	{"networkmap", start_networkmap, stop_networkmap}, 
#ifdef ASUS_DDNS
	{"ddns", start_ddns, stop_ddns}, 
#endif
	{NULL, NULL, NULL}};

#define RC_SERVICE_STOP		0x01
#define RC_SERVICE_START	0x02

INIT_STATES handle_notifications(void)
{
	INIT_STATES state = IDLE;
	char cmd[64], *script;
	int action = 0;

	strcpy(cmd, nvram_safe_get("rc_service"));
	if (!strncmp(cmd, "start_", 6)) {
		action = RC_SERVICE_START;
		script = &cmd[6];
	}
	else if (!strncmp(cmd, "stop_", 5)) {
		action = RC_SERVICE_STOP;
		script = &cmd[5];
	}
	else if (!strncmp(cmd, "restart_", 8)) {
		action = (RC_SERVICE_START | RC_SERVICE_STOP);
		script = &cmd[8];
	}
	else {
		action = 0;
		script = cmd;
	}

	fprintf(stderr, "%s: running %d,%s\n", __FUNCTION__, action, script);

	if (!strcmp(script, "reboot")) {
		sleep(1);// wait httpd sends the page to the browser
		eval("reboot");
	}
	else if (!strcmp(script, "system")) {
		sleep(1);// wait httpd sends the page to the browser
		state = RESTART;
	}
	else if (!strcmp(script, "dns")) {
		restart_dns();
	}
	else if (!strcmp(script, "dhcpd")) {
		if (nvram_match("wan_route_x", "1")) {
			sleep(2);// wait for httpd event done
			restart_dnsmasq(1);
			stop_httpd();
			start_httpd();
		}
		else {
			if (nvram_match("dhcp_enable_x", "0")) {
				// disable dhcp server
				if (nvram_match("dnsqmode", "2")) {
					nvram_set("dnsqmode", "1");
					restart_dnsmasq(0);
				}
				// disable hij mechanism
				track_set(ASUS_HIJ_DISABLE, NULL);
			}
			else {
				// enable dhcp server
				if (nvram_match("dnsqmode", "1") && nvram_invmatch("wan_state", "1")) {
					nvram_set("dnsqmode", "2");
#ifdef SWMODE_ADAPTER_SUPPORT
					if (nvram_match("sw_mode", "4"))
						restart_dnsmasq(1);
					else
#endif
						restart_dnsmasq(0);
				}
				// enable hij mechanism
				track_set(ASUS_HIJ_ENABLE, NULL);
			}
		}
	}
	else if (!strcmp(script, "firewall")) {
		start_firewall(nvram_safe_get("wan_ifname"), "br0");
	}
	else if (!strcmp(script, "time")) {
		eval_dumb("killall", "-9", "sh");
		time_zone_x_mapping();
		do_timer();

		stop_ntpc();
		stop_telnetd();
		stop_logger();
#ifdef LED_BRIGHTNESS_SUPPORT
		eval("rmmod", "led-drv");
		eval("insmod", "-q", "/usr/lib/led-drv.ko");
#endif
		start_logger();
		start_telnetd(0);
		start_ntpc();
	}
	else if (!strcmp(script, "apcli")) {
		sleep(1);// wait httpd sends the page to the browser
		restart_apcli(0);
	}
	else if (!strcmp(script, "wifi")) {
		restart_wifi(-1);
	}
	else if (!strcmp(script, "wifi0")) {
		restart_wifi(0);
	}
	else if (!strcmp(script, "wifi1")) {
		restart_wifi(1);
	}
#ifdef SWMODE_REPEATER_V2_SUPPORT
	else if (!strcmp(script, "wif")) {
		int n;
		char wif[8], *next;

		if (action&RC_SERVICE_STOP) {
			for1each(n, wif, nvram_safe_get("wl_ifnames"), next)
				ifconfig(wif, 0, NULL, NULL);
		}

		if (action&RC_SERVICE_START) {
			for1each(n, wif, nvram_safe_get("wl_ifnames"), next)
				ifconfig(wif, IFUP, NULL, NULL);
		}
	}
#endif
	else if (!strcmp(script, "wan")) {
		if (action&RC_SERVICE_STOP) {
			stop_detectWAN_arp();
			stop_wan();
		}

		if (action&RC_SERVICE_START) {
			init_wan_nvram();
			start_wan(nvram_safe_get("sw_mode"));
#ifdef WEB_REDIRECT
			restart_wanduck();
#endif
			start_detectWAN_arp();
		}
	}
	else if (!strcmp(script, "services4upload")) {
		if (action&RC_SERVICE_STOP) {
			FILE *fp;

			stop_networkmap();
			stop_ntpc();
#ifdef WEB_REDIRECT
			stop_wanduck();
#endif
			stop_lltd();
#ifdef AUDIO_SUPPORT
			stop_audiod();
#endif
			stop_telnetd();
			stop_logger();
			stop_infosvr();
			stop_dnsmasq();
#ifdef WPS_SUPPORT
			stop_wsc();
#endif
			if ((fp = fopen("/proc/sys/vm/drop_caches", "w+"))) {
				fputs("3", fp);
				fclose(fp);
			}
		}

		if (action&RC_SERVICE_START) {
			start_dnsmasq();
			start_infosvr();
			start_logger();
			start_telnetd(0);
#ifdef AUDIO_SUPPORT
			start_audiod();
#endif
			start_lltd();
#ifdef WEB_REDIRECT
			start_wanduck();
			sleep(1);
#endif
		}
	}
	else if (!strncmp(script, "webs_up", 7)) {
		if (action&RC_SERVICE_STOP) eval_dumb("killall", "wget");

		if (action&RC_SERVICE_START) {
			char cmd[32];
			char *_argv[] = {cmd, NULL};
			pid_t _pid;
			sprintf(cmd, "%s.sh", script);
			_eval(_argv, NULL, 0, &_pid);
		}
	}
	else {
		struct RC_SERVICE_PROC *p;

		for (p = rsp; p->name; p++) {
			if (!strcmp(script, p->name)) {
				if (action&RC_SERVICE_STOP) p->stop_proc();
				if (action&RC_SERVICE_START) p->start_proc();
				break;
			}
		}
		if (!p->name)
			fprintf(stderr, "WARNING: unknown event!\n");
	}

	nvram_set("rc_service", "");
	fprintf(stderr, "%s: done!\n", __FUNCTION__);

	return state;
}
