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
 * ASUS Home Gateway Reference Design
 * Web Page Configuration Support Routines
 *
 * Copyright 2013, ASUSTek Inc.
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of ASUSTek Inc.;
 * the contents of this file may not be disclosed to third parties, copied or
 * duplicated in any form, in whole or in part, without the prior written
 * permission of ASUSTek Inc..
 *
 */

#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <sys/stat.h>

#include <nvram/bcmnvram.h>

#include <shared.h>
#include <iwlib.h>

#include <sys/mman.h>
#ifndef	O_BINARY		/* should be define'd on __WIN32__ */
#define O_BINARY	0
#endif
#include "image.h"
#ifndef MAP_FAILED
#define MAP_FAILED (-1)
#endif

#include "httpd.h"
#include "web-ralink.h"
#ifdef INTERNET_RADIO
#include "web-iradio.h"
#endif
#ifdef APP_SUPPORT
#include "web-app.h"
#endif

#define MIN(a, b)		(((a) < (b)) ? (a) : (b))
#define sys_upload(image)	eval("nvram", "restore", image)
#define sys_download(file)	eval("nvram", "save", file)

char *readfile(char *fname,int *fsize);
int writefile(char *fname,char *content);

#define MAX_GROUP_COUNT		300
#define MAX_LINE_SIZE		512
int delMap[MAX_GROUP_COUNT];
char SystemCmd[64];
char UserID[32] = "";
char UserPass[32] = "";
char ProductID[32] = "";
extern int change_account;
extern int reget_account;
extern int skip_auth;
extern int lock_flag;
extern char referer_host[64];
extern unsigned int login_ip_tmp;

extern time_t login_timestamp;		// the timestamp of the logined ip
extern time_t login_timestamp_tmp;	// the timestamp of the current session.
extern time_t last_login_timestamp;	// the timestamp of the current session.
extern unsigned int login_try;
extern unsigned int MAX_login;

extern struct nvram_tuple router_defaults[];

// used for handling multiple instance
// copy nvram from, ex wl0_ to wl_
// prefix is wl_, wan_, or other function with multiple instance
// [prefix]_unit and [prefix]_subunit must exist
static void copy_index_to_unindex(char *prefix, int unit, int subunit)
{
	struct nvram_tuple *t;
	char *value;
	char name[64], unitname[64], unitptr[32];
	char tmp[64], unitprefix[32];

	// check if unit exist
	snprintf(unitptr, sizeof(unitptr), "%sunit", prefix);
	if ((value = nvram_get(unitptr)) == NULL) return;

	strncpy(tmp, prefix, sizeof(tmp));
	tmp[strlen(prefix) - 1] = 0;
	if (subunit == -1 || subunit == 0)
		snprintf(unitprefix, sizeof(unitprefix), "%s%d_", tmp, unit);
	else snprintf(unitprefix, sizeof(unitprefix), "%s%d.%d_", tmp, unit, subunit);

	/* go through each nvram value */
	for (t = router_defaults; t->name; t++) {
		memset(name, 0, 64);
		sprintf(name, "%s", t->name);

		// exception here
		if (strcmp(name, unitptr) == 0) continue;
		if (strcmp(name, "wan_primary") == 0) continue;

		if (!strncmp(name, prefix, strlen(prefix))) {
			(void)strcat_r(unitprefix, &name[strlen(prefix)], unitname);

			if ((value = nvram_get(unitname)) != NULL)
				nvram_set(name, value);
		}
	}
}

static char *rfctime(const time_t *timep)
{
	static char s[201];
	char buf[20];
	char *pt;
	struct tm tm;

	sprintf(buf, "%s", nvram_safe_get("time_zone_x"));
	pt = getenv("TZ");
	if (!pt || strcmp(pt, buf))
		setenv("TZ", buf, 1);
	memcpy(&tm, localtime(timep), sizeof(struct tm));
	strftime(s, 200, "%a, %d %b %Y %H:%M:%S %z", &tm);
	return s;
}

static void reltime(unsigned int seconds, char *cs)
{
	sprintf(cs, "%d secs", seconds);
}

static void sys_script(char *name)
{
	char scmd[64];

	sprintf(scmd, "/tmp/%s", name);
	//fprintf(stderr, "run %s %d %s\n", name, strlen(name), scmd);

	if (strcmp(name, "syscmd.sh") == 0) {
		if (strcmp(SystemCmd, "") != 0) {
			if (strcmp(SystemCmd, "run_infosvr") == 0) {
				nvram_set("ateCommand_flag", "1");
			} else {
				sprintf(SystemCmd, "%s > /tmp/syscmd.log 2>&1\n", SystemCmd);
				system(SystemCmd);
			}
		}
		else
			system("echo None > /tmp/syscmd.log\n");
	}
	else if (strcmp(name, "syslog.sh") == 0)
		;
	else if (strcmp(name, "wlan11b.sh") == 0)
		;
	else if (strcmp(name, "leases.sh") == 0)
		eval("killall", "-SIGUSR1", "dnsmasq");
	else if (strcmp(name, "iptable.sh") == 0)
		;
	else if (strcmp(name, "route.sh") == 0)
		;
#ifdef ASUS_DDNS
	else if (strcmp(name, "ddnsclient") == 0)
		notify_rc("restart_ddns");
#endif
	else if (strstr(scmd, " ") == 0) // no parameter, run script with eval
		eval(scmd);
	else
		system(scmd);
}

void websScan(char_t *str)
{
	unsigned int i, flag;
	char_t *v1, *v2, *v3, *sp;
	char_t groupid[64];
	char_t value[MAX_LINE_SIZE];
	char_t name[MAX_LINE_SIZE];

	v1 = strchr(str, '?');
	i = 0;
	flag = 0;

	while (v1 != NULL) {
		v2 = strchr(v1+1, '=');
		v3 = strchr(v1+1, '&');

		if (v2 == NULL)
			break;

		if (v3 != NULL) {
			strncpy(value, v2+1, v3-v2-1);
			value[v3-v2-1] = 0;
		}
		else
			strcpy(value, v2+1);

		strncpy(name, v1+1, v2-v1-1);
		name[v2-v1-1] = 0;
		/*printf("Value: %s %s\n", name, value);*/

		if (v2 != NULL && ((sp = strchr(v1+1, ' ')) == NULL || (sp > v2))) {
			if (flag && strncmp(v1+1, groupid, strlen(groupid)) == 0) {
				delMap[i] = atoi(value);
				/*printf("Del Scan : %x\n", delMap[i]);*/
				if (delMap[i] == -1)  break;
				i++;
			}
			else if (strncmp(v1+1, "group_id", 8) == 0) {
				sprintf(groupid, "%s_s", value);
				flag = 1;
			}
		}
		v1 = strchr(v1+1, '&');
	}
	delMap[i] = -1;
	return;
}

void websApply(webs_t wp, char_t *url)
{
#ifdef TRANSLATE_ON_FLY
	do_ej(url, wp);
	websDone(wp, 200);
#else
	FILE *fp;
	char buf[MAX_LINE_SIZE];

	fp = fopen(url, "r");
	if (fp == NULL) return;

	while (fgets(buf, sizeof(buf), fp))
		websWrite(wp, buf);

	websDone(wp, 200);
	fclose(fp);
#endif
}

/*
 * Example: 
 * lan_ipaddr=192.168.1.1
 * <% nvram_get("lan_ipaddr"); %> produces "192.168.1.1"
 * <% nvram_get("undefined"); %> produces ""
 */
static int ej_nvram_get(int eid, webs_t wp, int argc, char_t **argv)
{
	char *name, *c;
	int ret = 0;

	if (ejArgs(argc, argv, "%s", &name) < 1) {
		websError(wp, 400, "Insufficient args\n");
		return -1;
	}

	for (c = nvram_safe_get(name); *c; c++) {
		if (isprint(*c) 
				&& *c != '"' 
				&& *c != '&' 
				&& *c != '<' 
				&& *c != '>')
			ret += websWrite(wp, "%c", *c);
		else
			ret += websWrite(wp, "&#%d", *c);
	}

	return ret;
}

#ifdef ASUS_DDNS
static int ej_nvram_get_ddns(int eid, webs_t wp, int argc, char_t **argv)
{
	char *sid, *name, *c;
	int ret = 0;

	if (ejArgs(argc, argv, "%s %s", &sid, &name) < 2) {
		websError(wp, 400, "Insufficient args\n");
		return -1;
	}

	for (c = nvram_safe_get(name); *c; c++) {
		if (isprint(*c) 
				&& *c != '"' 
				&& *c != '&' 
				&& *c != '<' 
				&& *c != '>')
			ret += websWrite(wp, "%c", *c);
		else
			ret += websWrite(wp, "&#%d", *c);
	}

	if (strcmp(name,"ddns_return_code") == 0)
		nvram_set("ddns_return_code","");
	if (strcmp(name,"ddns_timeout") == 0)
		nvram_set("ddns_timeout","0");

	return ret;
}
#endif

/*
 * Example: 
 * wan_proto=dhcp
 * <% nvram_match("wan_proto", "dhcp", "selected"); %> produces "selected"
 * <% nvram_match("wan_proto", "static", "selected"); %> does not produce
 */
static int ej_nvram_match(int eid, webs_t wp, int argc, char_t **argv)
{
	char *name, *match, *output;

	if (ejArgs(argc, argv, "%s %s %s", &name, &match, &output) < 3) {
		websError(wp, 400, "Insufficient args\n");
		return -1;
	}

	if (nvram_match(name, match))
		return websWrite(wp, output);

	return 0;
}

static int ej_nvram_double_match(int eid, webs_t wp, int argc, char_t **argv)
{
	char *name, *match, *output;
	char *name2, *match2;

	if (ejArgs(argc, argv, "%s %s %s %s %s", &name, &match, &name2, &match2, &output) < 5) {
		websError(wp, 400, "Insufficient args\n");
		return -1;
	}

	if (nvram_match(name, match) && nvram_match(name2, match2))
		return websWrite(wp, output);

	return 0;
}

static int ej_select_channel(int eid, webs_t wp, int argc, char_t **argv)
{
	char ch_list[128];
	int n;
	char wif[8], *next;
	int ret = 0;

#ifdef SWMODE_ADAPTER_SUPPORT
	if (nvram_match("sw_mode", "4"))
		return websWrite(wp, "[\"0\"]");
#endif

	/* find out wireless interface */
	for1each(n, wif, nvram_safe_get("wl_ifnames"), next) {
		if (n == atoi(nvram_safe_get("wl_unit")))
			break;
	}

	if (get_channel_list_via_driver(wif, ch_list) > 0) {
		char *ptr;

		ret = websWrite(wp, "[\"0\"");
		ptr = strtok(ch_list, ",");
		while (ptr != NULL) {
			ret += websWrite(wp, ", \"%s\"", ptr);
			ptr = strtok(NULL, ",");
		}
		ret += websWrite(wp, "]");
	}

	return ret;
}

void char_to_ascii(char *output, char *input)
{
	int i;
	char tmp[10];
	char *ptr;

	ptr = output;

	for (i=0; i<strlen(input); i++) {
		if ((input[i] >= '0' && input[i] <= '9') 
				|| (input[i] >= 'A' && input[i] <= 'Z') 
				|| (input[i] >= 'a' && input[i] <= 'z') 
				|| input[i] == '~' 
				|| input[i] == '!' 
				|| input[i] == '*' 
				|| input[i] == '(' 
				|| input[i] == ')' 
				|| input[i] == '_' 
				|| input[i] == '-' 
				|| input[i] == "'" 
				|| input[i] == '.') {
			*ptr = input[i];
			ptr++;
		}
		else {
			sprintf(tmp, "%%%.02X", input[i]);
			strcpy(ptr, tmp);
			ptr += 3;
		}
	}
	*ptr = '\0';
}

static int ej_nvram_char_to_ascii(int eid, webs_t wp, int argc, char_t **argv)
{
	char *name, tmp[256];

	if (ejArgs(argc, argv, "%s", &name) < 1) {
		websError(wp, 400, "Insufficient args\n");
		return -1;
	}

	memset(tmp, 0x0, sizeof(tmp));
	char_to_ascii(tmp, nvram_safe_get(name));

	return websWrite(wp, "%s", tmp);
}

/* report system up time */
static int ej_uptime(int eid, webs_t wp, int argc, char_t **argv)
{
	char buf[MAX_LINE_SIZE];
	char *str = file2str("/proc/uptime");
	time_t tm;

	time(&tm);
	sprintf(buf, rfctime(&tm));

	if (str) {
		unsigned int up = atoi(str);
		free(str);
		char lease_buf[128];
		memset(lease_buf, 0, sizeof(lease_buf));
		reltime(up, lease_buf);
		sprintf(buf, "%s(%s since boot)", buf, lease_buf);
	}

	return websWrite(wp, buf);
}

#ifdef ROUTING_SUPPORT
enum {
	L_LEASE = 0,
	L_MAC,
	L_IP,
	L_HOST,
	L_CLID
};

/* dnsmasq ex: 43200 00:26:18:57:08:bc 192.168.1.105 mypc-3eaf6880a0 01:00:26:18:57:08:bc */
static int ej_lan_leases(int eid, webs_t wp, int argc, char_t **argv)
{
	FILE *fp = NULL;
	int ret = 0;
	int i;
	char buf[128], *p, *tmp, *lease_log[5];

	ret = websWrite(wp, "Host Name       Mac Address       IP Address      Lease\n");

	if (!(fp = fopen("/tmp/dnsmasq.leases", "r")))
		return ret;

	memset(buf, 0, sizeof(buf));
	while (fgets(buf, sizeof(buf), fp)) {
		tmp = buf;
		for (i = 0; i < 5; ++i) {
			if (i == 0)
				lease_log[0] = tmp;
			else if (p = strchr(tmp, ' ')) {
				*p++ = 0;
				lease_log[i] = tmp = p;
			}
		}
		ret += websWrite(wp, "%-16s", lease_log[L_HOST] ? (*lease_log[L_HOST]=='*' ? "<null>" : lease_log[L_HOST]) : " ");
		ret += websWrite(wp, "%-18s", lease_log[L_MAC] ? lease_log[L_MAC] : " " );
		ret += websWrite(wp, "%-16s", lease_log[L_IP] ? lease_log[L_IP] : " ");
		ret += websWrite(wp, "%s\n", lease_log[L_LEASE] ? lease_log[L_LEASE] : " ");

		memset(buf, 0, sizeof(buf));
	}
	fclose(fp);

	return ret;
}

/* Dump NAT table <tr><td>destination</td><td>MAC</td><td>IP</td><td>expires</td></tr> format */
static int ej_nat_table(int eid, webs_t wp, int argc, char_t **argv)
{
	FILE *fp;
	char *nat_argv[] = {"iptables", "-t", "nat", "-nxL", NULL};
	char line[256], tmp[256];
	char target[16], proto[16];
	char src[sizeof("255.255.255.255")];
	char dst[sizeof("255.255.255.255")];
	char *range, *host, *port, *ptr, *val;
	int ret = 0;

	/* dump nat table including VSERVER and VUPNP chains */
	_eval(nat_argv, ">/tmp/vserver.log", 10, NULL);

	ret = websWrite(wp, "Destination     Proto. Port range  Redirect to     Local port\n");
	/*		      255.255.255.255 other  65535:65535 255.255.255.255 65535:65535 */

	if (!(fp = fopen("/tmp/vserver.log", "r")))
		return ret;

	while (fgets(line, sizeof(line), fp) != NULL) {
		tmp[0] = '\0';
		if (sscanf(line,
			"%15s%*[ \t]"		// target
			"%15s%*[ \t]"		// prot
			"%*s%*[ \t]"		// opt
			"%15[^/]/%*d%*[ \t]"	// source
			"%15[^/]/%*d%*[ \t]"	// destination
			"%255[^\n]",		// options
			target, proto, src, dst, tmp) < 4) continue;

		/* TODO: add port trigger, portmap, etc support */
		if (strcmp(target, "DNAT") != 0)
			continue;

		/* uppercase proto */
		for (ptr = proto; *ptr; ptr++)
			*ptr = toupper(*ptr);
		/* parse destination */
		if (strcmp(dst, "0.0.0.0") == 0)
			strcpy(dst, "ALL");

		/* parse options */
		port = host = range = "";
		ptr = tmp;
		while ((val = strsep(&ptr, " ")) != NULL) {
			if (strncmp(val, "dpt:", 4) == 0)
				range = val + 4;
			else if (strncmp(val, "dpts:", 5) == 0)
				range = val + 5;
			else if (strncmp(val, "to:", 3) == 0) {
				port = host = val + 3;
				strsep(&port, ":");
			}
		}

		ret += websWrite(wp, "%-15s %-6s %-11s %-15s %-11s\n", 
					dst, proto, range, host, port ? : range);
	}
	fclose(fp);
	unlink("/tmp/vserver.log");

	return ret;
}

static int ej_route_table(int eid, webs_t wp, int argc, char_t **argv)
{
	char buf[256];
	int  nl = 0 ;
	struct in_addr dest;
	struct in_addr gw;
	struct in_addr mask;
	int flgs, ref, use, metric, ret = 0;
	char flags[4];
	unsigned long int d,g,m;
	char sdest[16], sgw[16];
	FILE *fp;

	ret = websWrite(wp, "Destination     Gateway         Genmask         Flags Metric Ref    Use Iface\n");

	if (!(fp = fopen("/proc/net/route", "r")))
		return ret;

	while (fgets(buf, sizeof(buf), fp) != NULL ) {
		if (nl) {
			int ifl = 0;
			while (buf[ifl]!=' ' && buf[ifl]!='\t' && buf[ifl]!='\0')
				ifl++;
			buf[ifl] = 0;/* interface */
			if (sscanf(buf+ifl+1, "%lx%lx%d%d%d%d%lx", &d, &g, &flgs, &ref, &use, &metric, &m) != 7) {
				//error_msg_and_die( "Unsuported kernel route format\n");
				//continue;
			}

			ifl = 0;/* parse flags */
			if (flgs & 1)
				flags[ifl++] = 'U';
			if (flgs & 2)
				flags[ifl++] = 'G';
			if (flgs & 4)
				flags[ifl++] = 'H';
			flags[ifl] = 0;
			dest.s_addr = d;
			gw.s_addr   = g;
			mask.s_addr = m;
			strcpy(sdest, (dest.s_addr==0 ? "default" : inet_ntoa(dest)));
			strcpy(sgw, (gw.s_addr==0 ? "*" : inet_ntoa(gw)));
			if (nvram_match("wan_proto","pppoe") && (strstr(buf, "eth0")))
				continue;
			if (strstr(buf, "br0") || strstr(buf, "wl0"))
				ret += websWrite(wp, "%-16s%-16s%-16s%-6s%-6d %-2d %7d LAN\n",
							sdest, sgw, inet_ntoa(mask), flags, metric, ref, use);
			else if (!strstr(buf, "lo"))
				ret += websWrite(wp, "%-16s%-16s%-16s%-6s%-6d %-2d %7d WAN\n",
							sdest, sgw, inet_ntoa(mask), flags, metric, ref, use);
		}
		nl++;
	}
	fclose(fp);
}
#endif

static int dump_file(webs_t wp, char *filename)
{
	FILE *fp;
	char buf[MAX_LINE_SIZE];
	int ret = 0;

	fp = fopen(filename, "r");
	if (fp == NULL)
		return websWrite(wp, "");

	while (fgets(buf, MAX_LINE_SIZE, fp) != NULL)
		ret += websWrite(wp, buf);

	fclose(fp);

	return ret;
}

static int ej_nvram_dump(int eid, webs_t wp, int argc, char_t **argv)
{
	char *file, *script;
	int n;
	char wif[8], *next;
	int ret = 0;
	char buf[32];

	if (ejArgs(argc, argv, "%s %s", &file, &script) < 2) {
		websError(wp, 400, "Insufficient args\n");
		return -1;
	}

	// run scrip first to update some status
	if (strcmp(script, "") != 0)
		sys_script(script);
 
	if (!strcmp(file, "wlan11b.log"))
		ret = ej_wl_status(eid, wp, 0, NULL);
#ifdef ROUTING_SUPPORT
	else if (!strcmp(file, "leases.log")) {
		eval("killall", "-SIGUSR1", "dnsmasq");
		ret = ej_lan_leases(eid, wp, 0, NULL);
	}
	else if (!strcmp(file, "iptable.log"))
		ret = ej_nat_table(eid, wp, 0, NULL);
	else if (!strcmp(file, "route.log"))
		ret = ej_route_table(eid, wp, 0, NULL);
#endif
#ifdef WPS_SUPPORT
	else if (!strcmp(file, "wps_info.log"))
		ret = ej_wps_info(eid, wp, 0, NULL);
#endif
	else if (!strcmp(file, "apscan") || !strcmp(file, "apscan_user")) {
		if (strcmp(file, "apscan_user") == 0) {
			char *fpt, *idx;
			int fsize;

			fpt = readfile("/proc/uptime", &fsize);
			if (fpt) {
				if (idx = strchr(fpt, '.')) {
					*idx = '\0';
					nvram_set("u_scan_time", fpt);
				}
				free(fpt);
			}
		}

		ret = ej_SiteSurvey(eid, wp, 0, NULL);
	}
	else if (!strcmp(file, "syslog.log") 
			|| !strcmp(file, "syscmd.log")) {
		sprintf(buf, "/tmp/%s-1", file);
		ret = dump_file(wp, buf);
		sprintf(buf, "/tmp/%s", file);
		ret += dump_file(wp, buf);
	}

	return ret;
}

/*
 * retreive and convert wl values for specified wl_unit 
 * Example: 
 * <% wl_get_parameter(); %> for coping wl[n]_ to wl_
 */
static int ej_wl_get_parameter(int eid, webs_t wp, int argc, char_t **argv)
{
	int unit = atoi(nvram_get("wl_unit"));

	// handle generate cases first
	(void)copy_index_to_unindex("wl_", unit, 0);

	return websWrite(wp, "");
}

static int ej_surfing_internet(int eid, webs_t wp, int argc, char_t **argv)
{
	int ret = 0;

	if (surfing_internet())
		ret = websWrite(wp, "1");
	else
		ret = websWrite(wp, "0");

	return ret;
}

static int ej_auto_firmware_upgrade(int eid, webs_t wp, int argc, char_t **argv)
{
	notify_rc(websGetVar(wp, "webs_cmd", ""));
	return 0;
}

static int ej_mailto_get_syslog(int eid, webs_t wp, int argc, char_t **argv)
{
	FILE *fp;
	char buf1[MAX_LINE_SIZE], buf2[MAX_LINE_SIZE*4];
	int ret = 0;

	ret = websWrite(wp, "[");

	fp = fopen("/tmp/syslog.log-1", "r");
	if (fp != NULL) {
		while (fgets(buf1, MAX_LINE_SIZE, fp) != NULL) {
			if (buf1[strlen(buf1)-1] == '\n')
				buf1[strlen(buf1)-1] = '\0';
			char_to_ascii(buf2, buf1);
			ret += websWrite(wp, "\"%s\", ", buf2);
		}
		fclose(fp);
	}

	fp = fopen("/tmp/syslog.log", "r");
	if (fp != NULL) {
		while (fgets(buf1, MAX_LINE_SIZE, fp) != NULL) {
			if (buf1[strlen(buf1)-1] == '\n')
				buf1[strlen(buf1)-1] = '\0';
			char_to_ascii(buf2, buf1);
			ret += websWrite(wp, "\"%s\", ", buf2);
		}
		fclose(fp);
	}

	ret += websWrite(wp, "\"\"]");

	return ret;
}

char *get_Channel(char *wif)
{
	char command[32];
	char tmp[512];
	FILE *fp;
	int rlen;
	char *pt1,*pt2;

	sprintf(command, "iwconfig %s", wif);
	fp = popen(command, "r");
	if (fp) {
		rlen = fread(tmp, 1, sizeof(tmp), fp);
		pclose(fp);
		if (rlen > 1) {
			tmp[rlen-1] = '\0';
			pt1 = strstr(tmp, "Channel=");
			if (pt1) {
				pt2 = pt1 + strlen("Channel=");
				pt1 = strstr(pt2," ");
				if (pt1) {
					*pt1++ = '\0';
					return pt2;
				}
			}
		}
	}
	return NULL;
}

static int validate_apply(webs_t wp)
{
	struct nvram_tuple *t;
	char *value;
	char name[64];
	char tmp[100], prefix[32];
	int unit = -1;
	int qis_form = 0;
	int nvram_modified = 0;

	if (value = websGetVar(wp, "wl_unit", NULL)) {
		unit = atoi(value);
		if (!(unit == 0|| unit == 1))
			unit = -1;
		else
			nvram_set("wl_unit", value);
	}
	else if (value = websGetVar(wp, "qis_form", NULL))
		qis_form = 1;

	for (t = router_defaults; t->name; t++) {
		sprintf(name, "%s", t->name);

		if (qis_form && !strncmp(name, "wl_", 3)) {
			int n;

			for (n=0; n<2; n++) {
				sprintf(tmp, "wl%d_%s", n, &name[0]+3);

				if (value = websGetVar(wp, tmp, NULL)) {
					if (strcmp(nvram_safe_get(tmp), value)) {
						fprintf(stderr, "set nvram [%s]=[%s]\n", tmp, value);
						nvram_set(tmp, value);
						nvram_modified = 1;
					}
				}
			}
		}
		else if (value = websGetVar(wp, name, NULL)) {
			if (strcmp(nvram_safe_get(name), value)) {
				nvram_set(t->name, value);
				fprintf(stderr, "set nvram [%s]=[%s]\n", t->name, value);

				if (!strcmp(t->name, "http_username") || !strcmp(t->name, "http_passwd"))
					change_account = 1;

				nvram_modified = 1;

				if (!strncmp(t->name, "wl_", 3)) {
					if (unit != -1) {
						snprintf(prefix, sizeof(prefix), "wl%d_", unit);
						nvram_set(strcat_r(prefix, name+3, tmp), value);
					}
				}
			}
		}
	}

	if (nvram_modified) {
		fprintf(stderr, "%s: done commit!\n", __FUNCTION__);
		_x_Setting = 1;
		nvram_set("x_Setting", "1");
		nvram_commit();
	}

	return nvram_modified;
}

static int ej_update_variables(int eid, webs_t wp, int argc, char_t **argv)
{
	int ret = 0;
	char *action_mode;
	char *action_script;
	char *action_wait;

	// assign control variables
	action_mode = websGetVar(wp, "action_mode", "");
	action_script = websGetVar(wp, "action_script", "");
	action_wait = websGetVar(wp, "action_wait", "3");

	fprintf(stderr, "%s: [%s] [%s]\n", __FUNCTION__, action_mode, action_script);

	if (!strcmp(action_mode, " Apply ")) {
		if (!validate_apply(wp))
			ret = websWrite(wp, "<script>no_changes_and_no_committing();</script>\n");
		else {
			ret = websWrite(wp, "<script>done_committing();</script>\n");

			if (strcmp(action_script, ""))
				notify_rc(action_script);

			fprintf(stderr, "%s: need wait %s seconds...\n", __FUNCTION__, action_wait);
			ret += websWrite(wp, "<script>restart_needed_time(%d);</script>\n", atoi(action_wait));
		}
	}

	return ret;
}

static int ej_link_status(int eid, webs_t wp, int argc, char_t **argv)
{
	char tmp[8];
	FILE *fp;
	int rlen;

	fp = popen("link_status", "r");
	if (fp) {
		rlen=fread(tmp, 1, sizeof(tmp)-1, fp);
		pclose(fp);
		if (rlen > 1) {
			tmp[rlen-1] = '\0';
			return websWrite(wp, "%s", tmp);
		}
	}
	return websWrite(wp, "0");
}

#ifdef ROUTING_SUPPORT
static int ej_wanlink(int eid, webs_t wp, int argc, char_t **argv)
{
	int ret = 0;
	char statusstr[16], type[16], ip[32], netmask[32], gateway[32], dns[128];

	memset(statusstr, 0, sizeof(statusstr));
	strcpy(statusstr, nvram_safe_get("wan_state"));

	memset(type, 0, sizeof(type));
	if (nvram_match("sw_mode", "6"))
		strcpy(type, "USB Modem");
	else
		strcpy(type, nvram_safe_get("wan_proto"));

	memset(ip, 0, sizeof(ip));
	memset(netmask, 0, sizeof(netmask));
	memset(gateway, 0, sizeof(gateway));
	if (!strcmp(statusstr, "Disconnected")) {
		strcpy(ip, "0.0.0.0");
		strcpy(gateway, "0.0.0.0");
	}
	else {
		strcpy(ip, nvram_safe_get("wan_ipaddr"));
		strcpy(netmask, nvram_safe_get("wan_netmask"));
		strcpy(gateway, nvram_safe_get("wan_gateway"));
	}

	memset(dns, 0, sizeof(dns));
	if (nvram_invmatch("wan_dnsenable_x", "1"))
		sprintf(dns, "%s %s", nvram_safe_get("wan_dns1_x"), nvram_safe_get("wan_dns2_x"));
	else
		sprintf(dns, "%s", nvram_safe_get("wan_dns"));

	ret = websWrite(wp, "function wanlink_statusstr() { return '%s';}\n", statusstr);
	ret += websWrite(wp, "function wanlink_type() { return '%s';}\n", type);
	ret += websWrite(wp, "function wanlink_ipaddr() { return '%s';}\n", ip);
	ret += websWrite(wp, "function wanlink_netmask() { return '%s';}\n", netmask);
	ret += websWrite(wp, "function wanlink_gateway() { return '%s';}\n", gateway);
	ret += websWrite(wp, "function wanlink_dns() { return '%s';}\n", dns);

	return ret;
}
#endif

static int ej_get_parameter(int eid, webs_t wp, int argc, char_t **argv)
{
	if (argc < 1) {
		websError(wp, 400,
				"get_parameter() used with no arguments, but at least one "
				"argument is required to specify the parameter name\n");
		return -1;
	}

	return websWrite(wp, "%s", websGetVar(wp, argv[0], ""));
}

static unsigned int getpeerip(webs_t wp)
{
	int fd, ret;
	struct sockaddr peer;
	socklen_t peerlen = sizeof(struct sockaddr);
	struct sockaddr_in *sa;

	fd = fileno((FILE *)wp);
	ret = getpeername(fd, (struct sockaddr *)&peer, &peerlen);
	sa = (struct sockaddr_in *)&peer;

	if (!ret) {
		fprintf(stderr, "peer: %x\n", sa->sin_addr.s_addr);
		return (unsigned int)sa->sin_addr.s_addr;
	}
	else {
		fprintf(stderr, "error: %d %d \n", ret, errno);
		return 0;
	}
}

static int ej_login_state_hook(int eid, webs_t wp, int argc, char_t **argv)
{
	unsigned int ip, login_ip;
	char ip_str[16], login_ip_str[16];
	unsigned long login_timestamp;
	struct in_addr now_ip_addr, login_ip_addr;
	time_t now;
	const int MAX = 80;
	const int VALUELEN = 18;
	char buffer[MAX], values[6][VALUELEN];
	int ret = 0;

	ip = getpeerip(wp);
	now_ip_addr.s_addr = ip;
	memset(ip_str, 0, 16);
	strcpy(ip_str, inet_ntoa(now_ip_addr));
	time(&now);

	login_ip = (unsigned int)atoll(nvram_safe_get("login_ip"));
	login_ip_addr.s_addr = login_ip;
	memset(login_ip_str, 0, 16);
	strcpy(login_ip_str, inet_ntoa(login_ip_addr));
	login_timestamp = (unsigned long)atol(nvram_safe_get("login_timestamp"));

	FILE *fp = fopen("/proc/net/arp", "r");
	if (fp) {
		memset(buffer, 0, MAX);
		memset(values, 0, 6*VALUELEN);

		while (fgets(buffer, MAX, fp)) {
			if (strstr(buffer, "br0") && !strstr(buffer, "00:00:00:00:00:00")) {
				if (sscanf(buffer, "%s%s%s%s%s%s", values[0], values[1], values[2], values[3], values[4], values[5]) == 6) {
					if (!strcmp(values[0], ip_str))
						break;
				}
				memset(values, 0, 6*VALUELEN);
			}
			memset(buffer, 0, MAX);
		}
		fclose(fp);
	}

	if (ip != 0 && login_ip == ip) {
		ret = websWrite(wp, "function login_ip_str() { return '%s'; }\n", login_ip_str);
		time(&login_timestamp);
	}
	else {
		if ((unsigned long)(now-login_timestamp) > 60)	//one minitues
			ret = websWrite(wp, "function login_ip_str() { return '0.0.0.0'; }\n");
		else
			ret = websWrite(wp, "function login_ip_str() { return '%s'; }\n", login_ip_str);
	}
	if (values[3] != NULL)
		ret += websWrite(wp, "function login_mac_str() { return '%s'; }\n", values[3]);
	else
		ret += websWrite(wp, "function login_mac_str() { return ''; }\n");

	return ret;
}

// [[IP, x, x, MAC, x, type], ...]
static int ej_get_arp_table(int eid, webs_t wp, int argc, char_t **argv)
{
	const int MAX = 80;
	const int FIELD_NUM = 6;
	const int VALUELEN = 18;
	char buffer[MAX], values[FIELD_NUM][VALUELEN];
	int num, firstRow, ret = 0;

	FILE *fp = fopen("/proc/net/arp", "r");
	if (fp) {
		memset(buffer, 0, MAX);
		memset(values, 0, FIELD_NUM*VALUELEN);

		firstRow = 1;
		while (fgets(buffer, MAX, fp)) {
			if (strstr(buffer, "br0") && !strstr(buffer, "00:00:00:00:00:00")) {
				if (firstRow == 1)
					firstRow = 0;
				else
					ret += websWrite(wp, ", ");

				if ((num = sscanf(buffer, "%s%s%s%s%s%s", values[0], values[1], values[2], values[3], values[4], values[5])) == FIELD_NUM)
					ret += websWrite(wp, "[\"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\"]", values[0], values[1], values[2], values[3], values[4], values[5]);

				memset(values, 0, FIELD_NUM*VALUELEN);
			}
			memset(buffer, 0, MAX);
		}
		fclose(fp);
	}

	return ret;
}

// for request networkmap
static int ej_refresh_static_client(int eid, webs_t wp, int argc, char_t **argv)
{
	return eval("killall", "-SIGUSR1", "networkmap");
}

// for get static IP's client.
// [[IP, MAC, DeviceName, Type, http, printer, iTune], ...]
static int ej_get_static_client(int eid, webs_t wp, int argc, char_t **argv)
{
	FILE *fp = NULL;
	char buf[1024], *head, *tail, field[1024];
	int len, i, first_client = 1, first_field, ret = 0;

	if (nvram_match("en_networkmap", "0"))
		return 0;

	fp = fopen("/tmp/static_ip.inf", "r");
	if (fp == NULL) {
		//fprintf(stderr, "Don't detect static clients!\n");
		return 0;
	}

	memset(buf, 0x0, sizeof(buf));
	while (fgets(buf, sizeof(buf), fp)) {
		if (first_client == 1)
			first_client = 0;
		else
			ret += websWrite(wp, ", ");

		len = strlen(buf);
		head = buf;
		first_field = 1;
		ret += websWrite(wp, "[");
		for (i = 0; i < 7; ++i) {
			tail = strchr(head, ',');
			if (tail != NULL) {
				memset(field, 0x0, sizeof(field));
				strncpy(field, head, (tail-head));
			}

			if (first_field == 1)
				first_field = 0;
			else
				ret += websWrite(wp, ", ");

			if (strlen(field) > 0)
				ret += websWrite(wp, "\"%s\"", field);
			else
				ret += websWrite(wp, "null");

			head = tail + 1;
		}
		ret += websWrite(wp, "]");
		memset(buf, 0x0, sizeof(buf));
	}
	fclose(fp);

	return ret;
}

static int ej_shown_language_option(int eid, webs_t wp, int argc, char **argv)
{
	FILE *fp = fopen("EN.dict", "r");
	int len, ret = 0;
	struct language_table *pLang = NULL;
	char lang[4];
	char buffer[1024], key[16], target[16];
	char *follow_info, *follow_info_end;

	if (fp == NULL) {
		fprintf(stderr, "No English dictionary!\n");
		return 0;
	}

	memset(lang, 0, 4);
	strcpy(lang, nvram_safe_get("preferred_lang"));

	ret = websWrite(wp, "<li><dl><dt id=\"selected_lang\"></dt>\\n");
	while (1) {
		memset(buffer, 0, sizeof(buffer));
		if ((follow_info = fgets(buffer, sizeof(buffer), fp)) != NULL) {
			if (strncmp(follow_info, "LANG_", 5))
				continue;

			follow_info += 5;
			follow_info_end = strstr(follow_info, "=");
			len = follow_info_end - follow_info;
			memset(key, 0, sizeof(key));
			strncpy(key, follow_info, len);

			for (pLang = language_tables; pLang->Lang != NULL; ++pLang) {
				if (strcmp(key, pLang->Target_Lang))
					continue;

				follow_info = follow_info_end + 1;
				follow_info_end = strstr(follow_info, "\n");
				len = follow_info_end - follow_info;
				memset(target, 0, sizeof(target));
				strncpy(target, follow_info, len);

				if (strcmp(key,lang))
					ret += websWrite(wp, "<dd><a onclick=\"submit_language(this)\" id=\"%s\">%s</a></dd>\\n", key, target);
				break;
			}
		}
		else
			break;
	}
	ret += websWrite(wp, "</dl></li>\\n");
	fclose(fp);

	return ret;
}

struct ej_handler ej_handlers[] = {
	{ "nvram_get", ej_nvram_get },
	{ "nvram_match", ej_nvram_match },
	{ "nvram_double_match", ej_nvram_double_match },
	{ "select_channel", ej_select_channel },
	{ "uptime", ej_uptime },
	{ "nvram_dump", ej_nvram_dump },
	{ "wl_get_parameter", ej_wl_get_parameter },
#ifdef ASUS_DDNS
	{ "nvram_get_ddns", ej_nvram_get_ddns },
#endif
	{ "nvram_char_to_ascii", ej_nvram_char_to_ascii },
	{ "update_variables", ej_update_variables },
	{ "pap_status", ej_pap_status },
	{ "link_status", ej_link_status },
#ifdef ROUTING_SUPPORT
	{ "wanlink", ej_wanlink },
#endif
	{ "get_parameter", ej_get_parameter },
	{ "login_state_hook", ej_login_state_hook },
	{ "get_arp_table", ej_get_arp_table },
	{ "refresh_static_client", ej_refresh_static_client },
	{ "get_static_client", ej_get_static_client },
	{ "wl_auth_list", ej_wl_auth_list },
	{ "shown_language_option", ej_shown_language_option },
	{ "surfing_internet", ej_surfing_internet },
	{ "auto_firmware_upgrade", ej_auto_firmware_upgrade },
	{ "mailto_get_syslog", ej_mailto_get_syslog },
#ifdef RUNTIME_CHECK_PAP_PASSWORD
	{ "check_pap_password", ej_check_pap_password },
#endif
#ifdef INTERNET_RADIO
	/* audio control */
	{ "get_vol_value", ej_get_vol },
	{ "set_vol_value", ej_set_vol },
	{ "audio_mute", ej_audio_mute },
	{ "set_audio_eq", ej_set_audio_eq },
	/* add/del/play internet radio */
	{ "get_upload_table_status", ej_get_upload_table_status },
	{ "get_builtin_radio_list", ej_get_builtin_list },
	{ "get_user_radio_list", ej_get_user_list },
	{ "get_url_table", ej_get_url_table },
	{ "get_now_playnum", ej_get_play_num },
	{ "play_rs", ej_play_rs },
	{ "check_iradio", ej_check_iradio },
	{ "update_radio_list", ej_update_ralist },
	{ "del_radio_list", ej_del_ralist },
	{ "get_status", ej_get_status },
	{ "upgrade_radio_list", ej_upgrade_radio_list },
	{ "detect_audiojack", ej_detect_audiojack },
	{ "test_audio", ej_test_audio },
	{ "auto_radio_list_upgrade", ej_auto_firmware_upgrade },
#endif
	{ NULL, NULL }
};

/**********************************************************************************
 * Generic MIME type handler
 **********************************************************************************/
static char post_buf[10000] = {0};
static char post_buf_backup[10000] = {0};

static void do_html_post_and_get(char *url, FILE *stream, int len, char *boundary)
{
	char *query = NULL;

	init_cgi(NULL);
	memset(post_buf, 0, 10000);
	memset(post_buf_backup, 0, 10000);

	if (fgets(post_buf, MIN(len + 1, sizeof(post_buf)), stream)) {
		len -= strlen(post_buf);

		while (len--)
			(void)fgetc(stream);
	}

	query = url;
	strsep(&query, "?");

	if (query && strlen(query) > 0) {
		if (strlen(post_buf) > 0)
			sprintf(post_buf_backup, "?%s&%s", post_buf, query);
		else
			sprintf(post_buf_backup, "?%s", query);

		sprintf(post_buf, "%s", post_buf_backup + 1);
	}
	else if (strlen(post_buf) > 0)
		sprintf(post_buf_backup, "?%s", post_buf);

	websScan(post_buf_backup);
	init_cgi(post_buf);
}

/* redirect the user to another webs page */
static int websRedirect(webs_t wp, char_t *url)
{
	int ret = 0;

	//fprintf(stderr, "redirect to: %s\n", url);
	ret = websWrite(wp, T("<html><head>\r\n"));
	if (Last_Req_Host[0] != '\0')
		ret += websWrite(wp, T("<meta http-equiv=\"refresh\" content=\"0; url=http://%s/%s\">\r\n"), Last_Req_Host, url);
	else
		ret += websWrite(wp, T("<meta http-equiv=\"refresh\" content=\"0; url=http://%s/%s\">\r\n"), nvram_safe_get("hijdomain"), url);
	ret += websWrite(wp, T("<meta http-equiv=\"Content-Type\" content=\"text/html\">\r\n"));
	ret += websWrite(wp, T("</head></html>\r\n"));
	websDone(wp, 200);

	return ret;
}

static void do_apply_cgi(char *url, FILE *stream)
{
	char *action_mode;
	char *action_script;
	char *current_url;
	char *next_url;

	action_mode = websGetVar(stream, "action_mode", "");
	action_script = websGetVar(stream, "action_script", "");
	current_url = websGetVar(stream, "current_page", "");
	next_url = websGetVar(stream, "next_page", "");

	fprintf(stderr, "%s: [%s] [%s] [%s/%s]\n", __FUNCTION__, action_mode, action_script, current_url, next_url);

	if (!strcmp(action_mode, " Refresh ")) {
		strcpy(SystemCmd, websGetVar(stream, "SystemCmd", ""));

		if (!strcmp(current_url, "Main_AdmStatus_Content.asp")) {
			if (strcmp(SystemCmd, "run_telnetd") == 0)
				sys_script("syscmd.sh");
		}
	}
	else if (!strcmp(action_mode, "Clear")) {
		// current only syslog implement this button
		unlink("/tmp/syslog.log-1");
		unlink("/tmp/syslog.log");
	}
	else if (!strcmp(action_mode, "Restart")) {
		websApply(stream, "Restarting.asp");
		notify_rc("restart_reboot");
		return 0;
	}
	else if (!strcmp(action_mode, "Restore")) {
		websApply(stream, "Restarting.asp");
		sys_default();
		notify_rc("restart_reboot");
		return 0;
	}
	else if (!strcmp(action_mode, "change_wl_unit")) {
		char *unit;

		unit = websGetVar(stream, "wl_unit", "");
		if(unit)
			nvram_set("wl_unit", unit);
	}
#ifdef WPS_SUPPORT
	else if (!strcmp(action_mode, "WPS_apply")) {
		char wps_enable_old[3];

		strcpy(wps_enable_old, nvram_safe_get("wps_enable"));
		nvram_set("wps_enable", websGetVar(stream, "wps_enable", ""));
		nvram_set("wps_method", websGetVar(stream, "wps_method", ""));

		if (nvram_match("wps_enable", "0")) {
			nvram_set("wps_start_flag", "0");
			fprintf(stderr, "%s: wps stop...\n", __FUNCTION__);
			eval("killall", "-SIGTSTP", "watchdog");
		}
		else if (strcmp(wps_enable_old, websGetVar(stream, "wps_enable", ""))) {
			nvram_set("wps_start_flag", "1");
			fprintf(stderr, "%s: wps start...\n", __FUNCTION__);
			eval("killall", "-SIGTSTP", "watchdog");
			sleep(1);
		}
		else {
			nvram_set("wps_start_flag", "2");
			if (nvram_match("wps_method", "1")) {
				char *wps_pin = websGetVar(stream, "wps_pin", NULL);

				nvram_set("wps_mode", "1");
				fprintf(stderr, "%s: WPS: PIN [%s]\n", __FUNCTION__, wps_pin);
				if (strlen(wps_pin) == 8)
					nvram_set("wps_pin_web", wps_pin);
				else
					nvram_set("wps_pin_web", "");
			}
			else {
				nvram_set("wps_mode", "2");
				fprintf(stderr, "%s: WPS: PBC\n", __FUNCTION__);
			}
			eval("killall", "-SIGTSTP", "watchdog");
		}
	}
	else if (!strcmp(action_mode, "Reset_OOB")) {
		nvram_set("wps_start_flag", "3");
		fprintf(stderr, "%s: wps oob...\n", __FUNCTION__);
		eval("killall", "-SIGTSTP", "watchdog");
		sleep(1);
	}
#endif
	else if (!strcmp(action_mode, "Cli_apply")) {
		char *tmp = NULL;

		tmp = websGetVar(stream, "cli_enable", "");
		if (*tmp == '1') {
			nvram_set("en_networkmap", "1");
			notify_rc("start_networkmap");
		}
		else {
			nvram_set("en_networkmap", "0");
			notify_rc("stop_networkmap");
		}
	}
	else {
		if (strcmp(action_script, ""))
			sys_script(action_script);
	}
	return websRedirect(stream, current_url);
}

static void do_auth(char *userid, char *passwd, char *realm)
{
	if (strcmp(ProductID,"") == 0)
		strncpy(ProductID, nvram_safe_get("fac_model_name"), 32);
	if (strcmp(UserID,"") == 0 || strcmp(UserPass, "") == 0 || reget_account == 1) {
		reget_account = 0;
		strncpy(UserID, nvram_safe_get("http_username"), 32);
		strncpy(UserPass, nvram_safe_get("http_passwd"), 32);
	}
	strncpy(userid, UserID, AUTH_MAX);
	strncpy(passwd, UserPass, AUTH_MAX);
	strncpy(realm, ProductID, AUTH_MAX);
}

void do_file(char *path, FILE *stream)
{
	FILE *fp;
	char line[1024];
	int ReadLen;

	fp = fopen(path, "rb");
	if (fp != NULL) {
		while (1) {
			ReadLen = fread(line, 1, sizeof(line), fp);
			if (ReadLen > 0)
				fwrite(line, 1, ReadLen, stream);
			else
				break;
		}
		fclose(fp);
	}
}

#if defined(linux)
#define SWAP_LONG(x) \
		((__u32)( \
		(((__u32)(x) & (__u32)0x000000ffUL) << 24) | \
		(((__u32)(x) & (__u32)0x0000ff00UL) <<  8) | \
		(((__u32)(x) & (__u32)0x00ff0000UL) >>  8) | \
		(((__u32)(x) & (__u32)0xff000000UL) >> 24) ))

static int checkcrc(const char *argv)
{
	int ifd;
	uint32_t checksum;
	struct stat sbuf;
	unsigned char *ptr;
	image_header_t header2;
	image_header_t *hdr, *hdr2 = &header2;
	char *imagefile;
	int ret = 0;

	imagefile = argv;
	//fprintf(stderr, "img file: %s\n", imagefile);

	ifd = open(imagefile, O_RDONLY|O_BINARY);

	if (ifd < 0) {
		fprintf(stderr, "Can't open %s: %s\n", imagefile, strerror(errno));
		ret = -1;
		goto checkcrc_end;
	}

	memset(hdr2, 0, sizeof(image_header_t));

	/* We're a bit of paranoid */
#if defined(_POSIX_SYNCHRONIZED_IO) && !defined(__sun__) && !defined(__FreeBSD__)
	(void) fdatasync(ifd);
#else
	(void) fsync(ifd);
#endif
	if (fstat(ifd, &sbuf) < 0) {
		fprintf(stderr, "Can't stat %s: %s\n", imagefile, strerror(errno));
		ret = -1;
		goto checkcrc_fail;
	}

	ptr = (unsigned char *)mmap(0, sbuf.st_size, PROT_READ, MAP_SHARED, ifd, 0);
	if (ptr == (unsigned char *)MAP_FAILED) {
		fprintf(stderr, "Can't map %s: %s\n", imagefile, strerror(errno));
		ret = -1;
		goto checkcrc_fail;
	}
	hdr = (image_header_t *)ptr;
	memcpy(hdr2, hdr, sizeof(image_header_t));
	memset(&hdr2->ih_hcrc, 0, sizeof(uint32_t));
	checksum = crc32_sp(0, (const char *)hdr2, sizeof(image_header_t));

	fprintf(stderr, "header crc: %X\n", checksum);
	fprintf(stderr, "org header crc: %X\n", SWAP_LONG(hdr->ih_hcrc));

	if (checksum != SWAP_LONG(hdr->ih_hcrc)) {
		ret = -1;
		goto checkcrc_fail;
	}

	(void) munmap((void *)ptr, sbuf.st_size);

	/* We're a bit of paranoid */
checkcrc_fail:
#if defined(_POSIX_SYNCHRONIZED_IO) && !defined(__sun__) && !defined(__FreeBSD__)
	(void) fdatasync(ifd);
#else
	(void) fsync(ifd);
#endif
	if (close(ifd)) {
		fprintf(stderr, "Read error on %s: %s\n", imagefile, strerror(errno));
		ret = -1;
	}

checkcrc_end:
	return ret;
}
#endif

/* file upload from browser */
#define MPU_FW_UPLOAD_PATH	"/tmp/linux.trx"
#define CFG_UPLOAD_PATH		"/tmp/settings.CFG"
#define TMEP_UPLOAD_PATH	"/tmp/upload_file"
int file_post_done;

static void do_file_post(char *url, FILE *stream, int len, char *boundary)
{
	FILE *fifo = NULL;
	char fifo_path[32], buf[4096], tmp;
	int count;
	int chk_header_done = 1;

	fprintf(stderr, "%s: start\n", __FUNCTION__);

	file_post_done = 0;
	memset(fifo_path, 0x0, sizeof(fifo_path));
	if (!strcmp(url, "upgrade.cgi")) {
		strcpy(fifo_path, MPU_FW_UPLOAD_PATH);
		chk_header_done = 0;
		notify_rc_and_wait("stop_services4upload");
	}
#if defined(INTERNET_RADIO) && defined(USB_TO_CM6510)
	else if (!strcmp(url, "upgrade_cm6510.cgi"))
		strcpy(fifo_path, CM6510_FW_UPLOAD_PATH);
#endif
	else if (!strcmp(url, "upload.cgi"))
		strcpy(fifo_path, CFG_UPLOAD_PATH);
#ifdef INTERNET_RADIO
	else if (!strcmp(url, "upload_ini.cgi"))
		strcpy(fifo_path, INI_UPLOAD_PATH);
#endif
	else if (!strcmp(url, "upload_file.cgi")) {
		strcpy(fifo_path, TMEP_UPLOAD_PATH);
		unlink(TMEP_UPLOAD_PATH);
	}

	/* Look for our part */
	while (len > 0) {
		if (!fgets(buf, MIN(len + 1, sizeof(buf)), stream))
			goto err;
		len -= strlen(buf);
		if (!strncasecmp(buf, "Content-Disposition:", 20) && strstr(buf, "name=\"file\""))
			break;
	}
	/* Skip boundary and headers */
	while (len > 0) {
		if (!fgets(buf, MIN(len + 1, sizeof(buf)), stream))
			goto err;
		len -= strlen(buf);
		if (!strcmp(buf, "\n") || !strcmp(buf, "\r\n"))
			break;
	}

	if (!(fifo = fopen(fifo_path, "a+")))
		goto err;

	while (len > 0) {
		if (waitfor(fileno(stream), 10) <= 0)
			break;

		count = fread(buf, 1, MIN(len, sizeof(buf)), stream);

		if (chk_header_done == 0 && count > 48) {
			/* Image Magic Number: 0x27051956 */
			if (!(buf[0] == 0x27 && buf[1] == 0x05 && buf[2] == 0x19 && buf[3] == 0x56)) {
				fprintf(stderr, "Header %x %x %x %x\n", buf[0], buf[1], buf[2], buf[3]);
				len -= count;
				goto err;
			}
			/* check model name */
			if (strncmp(buf+36, MODEL_NAME, sizeof(MODEL_NAME))) {
				fprintf(stderr, "illegal firmware upgrade! model mismatch...\n");
				len -= count;
				goto err;
			}
			chk_header_done = 1;
		}

		len -= count;
		fwrite(buf, 1, count, fifo);
		fprintf(stderr, "#");
	}
	fprintf(stderr, "\n");

	/* Slurp anything remaining in the request */
	while (len-- > 0) {
		tmp = fgetc(stream);
		fwrite(&tmp, 1, 1, fifo);
		fprintf(stderr, ".");
	}
	fprintf(stderr, "\n");

	fseek(fifo, 0, SEEK_END);
	fclose(fifo);
	fifo = NULL;

	file_post_done = 1;
	fprintf(stderr, "%s: done\n", __FUNCTION__);
	return;

err:
	if (fifo)
		fclose(fifo);

	/* Slurp anything remaining in the request */
	while (len-- > 0)
		tmp = fgetc(stream);

	fprintf(stderr, "%s: file upload fail\n", __FUNCTION__);
}

/* upgrade MPU firmware */
static void do_upgrade_mpu_cgi(char *url, FILE *stream)
{
	fprintf(stderr, "%s\n", __FUNCTION__);

	if (file_post_done && checkcrc(MPU_FW_UPLOAD_PATH) == 0) {
		eval("killall", "watchdog");
		eval("killall", "-SIGTSTP", "apcli_monitor");//pause apcli_monitor
		websApply(stream, "Updating.asp");
		fclose(stream);
		kill(1,SIGTTOU);
	}
	else {
		websApply(stream, "UpdateError.asp");
		unlink(MPU_FW_UPLOAD_PATH);
		fclose(stream);
		notify_rc_and_wait("start_services4upload");
	}
}

/* upload nvram setting */
static void do_upload_cfg_cgi(char *url, FILE *stream)
{
	fprintf(stderr, "%s\n", __FUNCTION__);

	if (file_post_done) {
		sys_upload(CFG_UPLOAD_PATH);
		sleep(2);

		if (nvram_match("restore_err", "1")) {
			nvram_unset("restore_err");
			websApply(stream, "UploadError.asp");
		}
		else {
			websApply(stream, "Uploading.asp");
			nvram_commit();
			fclose(stream);
			notify_rc("restart_reboot");
			return;
		}
	}
	else
		websApply(stream, "UploadError.asp");

	unlink(CFG_UPLOAD_PATH);
	fclose(stream);
}

/* upload file result */
static void do_upload_file_cgi(char *url, FILE *stream)
{
	fprintf(stderr, "%s\n", __FUNCTION__);

	if (file_post_done)
		websApply(stream, "FileUploadSuccess.asp");
	else
		websApply(stream, "FileUploadFail.asp");
	fclose(stream);
}

/* download nvram setting */
#define CFG_DOWNLOAD_PATH	"/tmp/settings"
static void do_download_cfg_cgi(char *url, FILE *stream)
{
	nvram_commit();
	sys_download(CFG_DOWNLOAD_PATH);
	do_file(CFG_DOWNLOAD_PATH, stream);
	unlink(CFG_DOWNLOAD_PATH);
}

/* download syslog */
static void do_download_log_cgi(char *path, FILE *stream)
{
	dump_file(stream, "/tmp/syslog.log-1");
	dump_file(stream, "/tmp/syslog.log");
	fputs("\r\n", stream); /* terminator */
	fputs("\r\n", stream); /* terminator */
}

/* Base-64 decoding.  This represents binary data as printable ASCII
** characters.  Three 8-bit binary bytes are turned into four 6-bit
** values, like so:
**
**   [11111111]  [22222222]  [33333333]
**
**   [111111] [112222] [222233] [333333]
**
** Then the 6-bit values are represented using the characters "A-Za-z0-9+/".
*/

static int b64_decode_table[256] = {
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 00-0F */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 10-1F */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,  /* 20-2F */
    52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,  /* 30-3F */
    -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,  /* 40-4F */
    15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,  /* 50-5F */
    -1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,  /* 60-6F */
    41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1,  /* 70-7F */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 80-8F */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 90-9F */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* A0-AF */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* B0-BF */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* C0-CF */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* D0-DF */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* E0-EF */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1   /* F0-FF */
    };

/* Do base-64 decoding on a string.  Ignore any non-base64 bytes.
** Return the actual number of bytes generated.  The decoded size will
** be at most 3/4 the size of the encoded, and may be smaller if there
** are padding characters (blanks, newlines).
*/
static int
b64_decode( const char* str, unsigned char* space, int size )
{
    const char* cp;
    int space_idx, phase;
    int d, prev_d=0;
    unsigned char c;

    space_idx = 0;
    phase = 0;
    for ( cp = str; *cp != '\0'; ++cp )
	{
	d = b64_decode_table[(int)*cp];
	if ( d != -1 )
	    {
	    switch ( phase )
		{
		case 0:
		++phase;
		break;
		case 1:
		c = ( ( prev_d << 2 ) | ( ( d & 0x30 ) >> 4 ) );
		if ( space_idx < size )
		    space[space_idx++] = c;
		++phase;
		break;
		case 2:
		c = ( ( ( prev_d & 0xf ) << 4 ) | ( ( d & 0x3c ) >> 2 ) );
		if ( space_idx < size )
		    space[space_idx++] = c;
		++phase;
		break;
		case 3:
		c = ( ( ( prev_d & 0x03 ) << 6 ) | d );
		if ( space_idx < size )
		    space[space_idx++] = c;
		phase = 0;
		break;
		}
	    prev_d = d;
	    }
	}
    return space_idx;
}

asus_token_t *create_list(char *token)
{
	char login_timestr[32];
	time_t now;
	struct in_addr login_ip_addr;
	char *login_ip_str;

	login_ip_addr.s_addr = login_ip_tmp;
	login_ip_str = inet_ntoa(login_ip_addr);

	now = uptime();

	memset(login_timestr, 0, 32);
	sprintf(login_timestr, "%lu", now);

	asus_token_t *ptr;
	ptr = (asus_token_t *)malloc(sizeof(asus_token_t));
	if (NULL == ptr) {
		printf("\n Node creation failed \n");
		return NULL;
	}
	strncpy(ptr->token, token, 32);
	strncpy(ptr->ipaddr, login_ip_str, 16);
	strncpy(ptr->login_timestampstr, login_timestr, 32);
	strncpy(ptr->host, Last_Req_Host, 64);
	ptr->next = NULL;

	head = curr = ptr;
	return ptr;
}

asus_token_t *add_token_to_list(char *token, int add_to_end)
{
	if (NULL == head)
		return (create_list(token));

	asus_token_t *ptr = (asus_token_t *)malloc(sizeof(asus_token_t));
	if (NULL == ptr) {
		fprintf(stderr, "\n Node creation failed \n");
		return NULL;
	}

	char login_timestr[32];
	time_t now;
	struct in_addr login_ip_addr;
	char *login_ip_str;

	login_ip_addr.s_addr = login_ip_tmp;
	login_ip_str = inet_ntoa(login_ip_addr);

	now = uptime();

	memset(login_timestr, 0, 32);
	sprintf(login_timestr, "%lu", now);

	strncpy(ptr->token, token, 32);
	strncpy(ptr->ipaddr, login_ip_str, 16);
	strncpy(ptr->login_timestampstr, login_timestr, 32);
	strncpy(ptr->host, Last_Req_Host, 64);
	ptr->next = NULL;

	if (add_to_end == 1) {
		curr->next = ptr;
		curr = ptr;
	}
	else {
		ptr->next = head;
		head = ptr;
	}
	return ptr;
}

int get_token_list_length(void)
{
	asus_token_t *p = head;
	int count=0;

	while (p != NULL) {
		count++;
		p = p->next;
	}

	return count;
}

asus_token_t *search_timeout_in_list(asus_token_t **prev)
{
	asus_token_t *ptr = head;
	asus_token_t *tmp = NULL;
	int found = 0;
	time_t now = 0;
	int logout_time = 30;

	if (!nvram_match("http_autologout", "0"))
		logout_time = atoi(nvram_safe_get("http_autologout"));

	now = uptime();

	while (ptr != NULL) {
		if ((unsigned long)(now-atol(ptr->login_timestampstr)) > (logout_time * 60)) {
			found = 1;
			break;
		}
		else {
			tmp = ptr;
			ptr = ptr->next;
		}
	}

	if (found == 1) {
		if (prev)
			*prev = tmp;
		return ptr;
	}
	else
		return NULL;
}

int check_token_timeout_in_list(void)
{
	int i;
	int list_len = get_token_list_length();

	for (i = 0; i < list_len; i++) {
		asus_token_t *prev = NULL;
		asus_token_t *del = NULL;
		del = search_timeout_in_list(&prev);

		if (del == NULL)
			return -1;
		else {
			if (prev != NULL)
			prev->next = del->next;

			if (del == curr)
			    curr = prev;
			if (del == head)
			    head = del->next;
		}
		free(del);
		del = NULL;
	}
	return 0;
}

int check_login_in_list(void)
{
	asus_token_t *prev = NULL;
	asus_token_t *del = NULL;

	del = search_timeout_in_list(&prev);
	if (del == NULL)
		return -1;
	else {
		if (prev != NULL)
			prev->next = del->next;
		if (del == curr)
			curr = prev;
		if (del == head)
			head = del->next;
	}
	free(del);
	del = NULL;
	return 0;
}

void print_list(void)
{
	asus_token_t *ptr = head;

	fprintf(stderr, "\n -------Printing list Start------- \n");
	while (ptr != NULL) {
		fprintf(stderr, "%s\n",ptr->token);
		fprintf(stderr, "%s\n",ptr->ipaddr);
		fprintf(stderr, "%s\n",ptr->login_timestampstr);
		fprintf(stderr, "%s\n",ptr->host);
		ptr = ptr->next;
	}
	fprintf(stderr, "\n -------Printing list End------- \n");

	return;
}

void add_asus_token(char *token)
{
	int ret;

	//print_list();
	ret = check_token_timeout_in_list();
	add_token_to_list(token, 1);
	//print_list();
}

#define RFC1123FMT "%a, %d %b %Y %H:%M:%S GMT"
static int login_cgi(webs_t wp, char_t *urlPrefix, char_t *webDir, int arg, char_t *url, char_t *path, char_t *query)
{
	char *authorization_t;
	char authinfo[500];
	char* authpass;
	int l;
	char asus_token[32];
	char *next_page = NULL;
	char filename[128];

	memset(filename, 0, 128);
	next_page = websGetVar(wp, "next_page", "");
	authorization_t = websGetVar(wp, "login_authorization", "");

	/* Decode it. */
	l = b64_decode(&(authorization_t[0]), (unsigned char*) authinfo, sizeof(authinfo));
	authinfo[l] = '\0';

	authpass = strchr(authinfo, ':');
	if (authpass == (char*) 0) {
		websRedirect(wp, "Main_Login.asp");
		return 0;
	}
	*authpass++ = '\0';

	time_t now;
	char timebuf[100];
	now = time( (time_t*) 0 );

	time_t dt;
	struct in_addr temp_ip_addr;
	char *temp_ip_str;

	login_timestamp_tmp = uptime();
	dt = login_timestamp_tmp - last_login_timestamp;
	if (last_login_timestamp != 0 && dt > 60) {
		login_try = 0;
		last_login_timestamp = 0;
		lock_flag = 0;
	}
	if (MAX_login <= DEFAULT_LOGIN_MAX_NUM)
		MAX_login = DEFAULT_LOGIN_MAX_NUM;
	if (login_try >= MAX_login) {
		lock_flag = 1;
		temp_ip_addr.s_addr = login_ip_tmp;
		temp_ip_str = inet_ntoa(temp_ip_addr);

		if (login_try%MAX_login == 0)
			logmessage("httpd login lock", "Detect abnormal logins at %d times. The newest one was from %s.", login_try, temp_ip_str);

		__send_login_page(LOGINLOCK, NULL, dt);
		return LOGINLOCK;
	}

	websWrite(wp,"%s %d %s\r\n", PROTOCOL, 200, "OK");
	websWrite(wp,"Server: %s\r\n", SERVER_NAME );
	strftime(timebuf, sizeof(timebuf), RFC1123FMT, gmtime(&now));
	websWrite(wp, "Date: %s\r\n", timebuf);
	websWrite(wp, "Content-Type: %s\r\n", "text/html");

	/* Is this the right user and password? */
	if (strcmp( nvram_safe_get("http_username"), authinfo) == 0 && strcmp(nvram_safe_get("http_passwd"), authpass) == 0) {
		login_try = 0;
		last_login_timestamp = 0;
		memset(referer_host, 0, sizeof(referer_host));
		if (strncmp(dut_domain_name, Last_Req_Host, strlen(dut_domain_name)) == 0)
			strcpy(referer_host, nvram_safe_get("lan_ipaddr"));
		else
			snprintf(referer_host, sizeof(Last_Req_Host), "%s", Last_Req_Host);

		strncpy(asus_token, generate_token(), sizeof(asus_token));
		add_asus_token(asus_token);

		websWrite(wp, "Set-Cookie: asus_token=%s; HttpOnly;\r\n", asus_token);
		websWrite(wp, "Connection: close\r\n");
		websWrite(wp, "\r\n");
		snprintf(filename, sizeof(filename), "/www/%s", next_page);

		websWrite(wp, "<HTML><HEAD>\n");
		if (nvram_match("http_passwd", "admin"))
			websWrite(wp, T("<meta http-equiv=\"refresh\" content=\"0; url=Main_Password.asp\">\r\n"));
		else if (next_page == NULL || strcmp(next_page, "") == 0 || strstr(next_page, "http") != NULL || strstr(next_page, "//") != NULL || (!check_if_file_exist(filename)))
			websWrite(wp, T("<meta http-equiv=\"refresh\" content=\"0; url=index.asp\">\r\n"));
		else
			websWrite(wp, T("<meta http-equiv=\"refresh\" content=\"0; url=%s\">\r\n"), next_page);
		websWrite(wp, "</HEAD></HTML>\n");
		return 1;
	}
	else {
		websWrite(wp, "Connection: close\r\n");
		websWrite(wp, "\r\n");
		login_try++;
		last_login_timestamp = login_timestamp_tmp;
		websWrite(wp, "<HTML><HEAD>\n");
		if (login_try >= MAX_login) {
			lock_flag = 1;
#ifdef RTCONFIG_NOTIFICATION_CENTER
			temp_ip_addr.s_addr = login_ip_tmp;
			temp_ip_str = inet_ntoa(temp_ip_addr);
			NOTIFY_EVENT_T *e = initial_nt_event();
			e->event = ADMIN_LOGIN_FAIL_LAN_WEB_EVENT;
			snprintf(e->msg, sizeof(e->msg), "%s", temp_ip_str);
			send_trigger_event(e);
			nt_event_free(e);
#endif
			websWrite(wp, "<script>parent.location.href='/Main_Login.asp?error_status=%d&lock_time=%ld';</script>\n", LOGINLOCK, dt);
		}
		else
			websWrite(wp, "<script>parent.location.href='/Main_Login.asp?error_status=%d';</script>\n", ACCOUNTFAIL);
		websWrite(wp, "</HEAD></HTML>\n");
		return 0;
	}
}

static void do_login_cgi(char *url, FILE *stream)
{
	login_cgi(stream, NULL, NULL, 0, url, NULL, NULL);
}

static char no_cache[] = "Cache-Control: no-cache\r\n"
			 "Pragma: no-cache\r\n"
			 "Expires: 0";

static char syslog_txt[] = "Content-Disposition: attachment;\r\n"
			   "filename=syslog.txt";

struct mime_handler mime_handlers[] = {
	{ "Main_Login.asp|Nologin.asp|error_page.htm*|qis/QIS_fail.htm*|ajax_*", "text/html", no_cache, do_html_post_and_get, do_ej, NULL },
	{ "**.asp*|**.htm*", "text/html", NULL, do_html_post_and_get, do_ej, do_auth },
	{ "**.css", "text/css", NULL, NULL, do_file, NULL },
	{ "**.png", "image/png", NULL, NULL, do_file, NULL },
	{ "**.gif", "image/gif", NULL, NULL, do_file, NULL },
	{ "**.jpg", "image/jpeg", NULL, NULL, do_file, NULL },
	{ "**.js",  "text/javascript", NULL, NULL, do_ej, NULL },
	/* CGI */
	{ "apply.cgi*", "text/html", no_cache, do_html_post_and_get, do_apply_cgi, do_auth },
#if defined(INTERNET_RADIO) && defined(URL_PLAYER)
	{ "url_play.cgi*", "text/html", no_cache, do_html_post_and_get, do_url_cgi, NULL },
#endif
#ifdef APP_SUPPORT
	{ "app_none.cgi*", "text/html", no_cache, do_html_post_and_get, do_app_none_cgi, NULL },
	{ "app_auth.cgi*", "text/html", no_cache, do_html_post_and_get, do_app_auth_cgi, do_auth },
#endif
	/* upload file */
	{ "upgrade.cgi*", "text/html", no_cache, do_file_post, do_upgrade_mpu_cgi, do_auth },
#if defined(INTERNET_RADIO) && defined(USB_TO_CM6510)
	{ "upgrade_cm6510.cgi*", "text/html", no_cache, do_file_post, do_upgrade_cm6510_cgi, do_auth },
#endif
	{ "upload.cgi*", "text/html", no_cache, do_file_post, do_upload_cfg_cgi, do_auth },
#ifdef INTERNET_RADIO
	{ "upload_ini.cgi*", "text/html", no_cache, do_file_post, do_upload_ini_cgi, do_auth },
#endif
	{ "upload_file.cgi*", "text/html", no_cache, do_file_post, do_upload_file_cgi, NULL },
	/* download file */
	{ "Settings_**.CFG", "application/octet-stream", no_cache, NULL, do_download_cfg_cgi, do_auth },
	{ "syslog.txt*", "application/force-download", syslog_txt, do_html_post_and_get, do_download_log_cgi, do_auth },
#ifdef INTERNET_RADIO
	{ "RadioList_**.INI", "application/octet-stream", no_cache, NULL, do_download_ini_cgi, NULL },
#endif
	{ "login.cgi", "text/html", no_cache, do_html_post_and_get, do_login_cgi, NULL },
	{ NULL, NULL, NULL, NULL, NULL, NULL }
};

// some should be referer
struct mime_referer mime_referers[] = {
	{ "start_apply.htm", CHECK_REFERER},
	{ "apply.cgi", CHECK_REFERER},
#ifdef APP_SUPPORT
	{ "app_auth.cgi", CHECK_REFERER},
#endif
	{ "upgrade.cgi", CHECK_REFERER},
#if defined(INTERNET_RADIO) && defined(USB_TO_CM6510)
	{ "upgrade_cm6510.cgi", CHECK_REFERER},
#endif
	{ "upload.cgi", CHECK_REFERER},
#ifdef INTERNET_RADIO
	{ "upload_ini.cgi", CHECK_REFERER},
#endif
	{ NULL, 0 }
};
