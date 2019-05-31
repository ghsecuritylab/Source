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
#include <dirent.h>

#include <nvram/bcmnvram.h>

#include <shared.h>

#include "rc.h"

char *mac_conv(char *mac_name, int idx, char *buf);
void portforwarding_setting(FILE *fp);
void porttrigger_setting(FILE *fp, char *wan_if, int is_nat);

char *proto_conv(char *proto, char *buf)
{
	if (!strncasecmp(proto, "BOTH", 3)) strcpy(buf, "both");
	else if (!strncasecmp(proto, "TCP", 3)) strcpy(buf, "tcp");
	else if (!strncasecmp(proto, "UDP", 3)) strcpy(buf, "udp");
	else if (!strncasecmp(proto, "OTHER", 5)) strcpy(buf, "other");
	else strcpy(buf,"tcp");
	return buf;
}

char *protoflag_conv(char *proto, char *buf, int isFlag)
{
	if (!isFlag) {
		if (!strncasecmp(proto, "UDP", 3)) strcpy(buf, "udp");
		else strcpy(buf, "tcp");
	}
	else {
		if (strlen(proto) > 3) strcpy(buf, proto+4);
		else strcpy(buf,"");
	}
	return buf;
}

char *filter_conv(char *proto, char *flag, char *srcip, char *srcport, char *dstip, char *dstport, char *buf)
{
	//printf("filter : %s,%s,%s,%s,%s,%s\n", proto, flag, srcip, srcport, dstip, dstport);

	memset(buf, 0, sizeof(buf));

	if (strcmp(proto, ""))
		sprintf(buf, "%s -p %s", buf, proto);

	if (strcmp(flag, ""))
		sprintf(buf, "%s --tcp-flags %s %s", buf, flag, flag);

	if (strcmp(srcip, "")) {
		if (strchr(srcip , '-'))
			sprintf(buf, "%s --src-range %s", buf, srcip);
		else
			sprintf(buf, "%s -s %s", buf, srcip);
	}

	if (strcmp(srcport, ""))
		sprintf(buf, "%s --sport %s", buf, srcport);

	if (strcmp(dstip, "")) {
		if (strchr(dstip, '-'))
			sprintf(buf, "%s --dst-range %s", buf, dstip);
		else
			sprintf(buf, "%s -d %s", buf, dstip);
	}

	if (strcmp(dstport, ""))
		sprintf(buf, "%s --dport %s", buf, dstport);

	return buf;
}

char *iprange_ex_conv(char *ip, char *buf)
{
	char startip[16], endip[16];
	int i, j, k;
	int mask;

	strcpy(buf, "");

	// scan all ip string
	i=j=k=0;
	mask=32;

	while (*(ip+i)) {
		if (*(ip+i) == '*') {
			startip[j++] = '0';
			endip[k++] = '0';
			// 255 is for broadcast
			mask-=8;
		}
		else {
			startip[j++] = *(ip+i);
			endip[k++] = *(ip+i);
		}
		++i;
	}

	startip[j++] = 0;
	endip[k++] = 0;

	if (mask == 32)
		sprintf(buf, "%s", startip);
	else if (mask == 0)
		strcpy(buf, "");
	else sprintf(buf, "%s/%d", startip, mask);

	//fprintf(stderr, "mask:%d, buf:%s\n", mask, buf);

	return buf;
}

void 
ip2class(char *lan_ip, char *netmask, char *buf)
{
	unsigned int val, ip;
	struct in_addr in;
	int i=0;

	// only handle class A,B,C	
	val = (unsigned int)inet_addr(netmask);
	ip = (unsigned int)inet_addr(lan_ip);
        in.s_addr = ip & val;

        for (val = ntohl(val); val; i++) 
               val <<= 1;
       
        sprintf(buf, "%s/%d", inet_ntoa(in), i);
	dprintf(buf);	
}

void convert_routes(void)
{
	char *nv, *nvp, *b;
	char *ip, *netmask, *gateway, *metric, *interface;
	char wroutes[1024], lroutes[1024], mroutes[1024];

	/* Disable Static if it's not enable */
	wroutes[0] = 0;
	lroutes[0] = 0;
	mroutes[0] = 0;

	if (nvram_match("sr_enable_x", "1")) {
		nv = nvp = strdup(nvram_safe_get("sr_rulelist"));
		while (nv && (b = strsep(&nvp, "<")) != NULL) {
			char *routes;

			if ((vstrsep(b, ">", &ip, &netmask, &gateway, &metric, &interface) != 5))
				continue;
			else if (!strcmp(interface, "WAN"))
				routes = wroutes;
			else if (!strcmp(interface, "MAN"))
				routes = mroutes;
			else if (!strcmp(interface, "LAN"))
				routes = lroutes;
			else
				continue;

			sprintf(routes, "%s %s:%s:%s:%d", routes, ip, netmask, gateway, atoi(metric)+1);
		}
		free(nv);
	}

	nvram_set("lan_route", lroutes);
	nvram_set("wan_route", wroutes);
	nvram_set("wan_mroute", mroutes);
}

void write_upnp_forward(FILE *fp, char *wan_if, char *wan_ip, char *lan_if, char *lan_ip, char *lan_class, char *logaccept, char *logdrop)
{
	char name[] = "forward_portXXXXXXXXXX", value[512];
        char *wan_port0, *wan_port1, *lan_ipaddr, *lan_port0, *lan_port1, *proto;
        char *enable, *desc;
	int i;

	/* Set wan_port0-wan_port1>lan_ipaddr:lan_port0-lan_port1,proto,enable,desc */
	for(i=0 ; i<15 ; i++)
	{
		snprintf(name, sizeof(name), "forward_port%d", i);

		strncpy(value, nvram_safe_get(name), sizeof(value));

		/* Check for LAN IP address specification */
		lan_ipaddr = value;
		wan_port0 = strsep(&lan_ipaddr, ">");
		if (!lan_ipaddr)
			continue;

		/* Check for LAN destination port specification */
		lan_port0 = lan_ipaddr;
		lan_ipaddr = strsep(&lan_port0, ":");
		if (!lan_port0)
			continue;

		/* Check for protocol specification */
		proto = lan_port0;
		lan_port0 = strsep(&proto, ":,");
		if (!proto)
			continue;

		/* Check for enable specification */
		enable = proto;
		proto = strsep(&enable, ":,");
		if (!enable)
			continue;

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

		/* skip if it's disabled */
		if( strcmp(enable, "off") == 0 )
			continue;

		/* -A PREROUTING -p tcp -m tcp --dport 823 -j DNAT 
		                 --to-destination 192.168.1.88:23  */
		if( !strcmp(proto,"tcp") || !strcmp(proto,"both") )
		{
	   		fprintf(fp, "-A VSERVER -p tcp -m tcp --dport %s "	// oleg patch
				  "-j DNAT --to-destination %s:%s\n"
			, wan_port0, lan_ipaddr, lan_port0);		// oleg patch
		}
		if( !strcmp(proto,"udp") || !strcmp(proto,"both") ){
	   		fprintf(fp, "-A VSERVER -p udp -m udp --dport %s "		// oleg patch
				  "-j DNAT --to-destination %s:%s\n"
			, wan_port0, lan_ipaddr, lan_port0);					// oleg patch
		}
	}
}

void nat_setting(char *wan_if, char *lan_if, char *logaccept, char *logdrop)
{
	FILE *fp;
	char *wan_ipaddr = nvram_safe_get("wan_ipaddr");
	char *wan_pppoe_ipaddr = nvram_safe_get("wan_pppoe_ipaddr");
	char *lan_ipaddr = nvram_safe_get("lan_ipaddr");
	char *lan_netmask = nvram_safe_get("lan_netmask");
        char lan_class[32];
	char dstips[32], dstports[12];

	if ((fp=fopen("/tmp/nat_rules", "w"))==NULL) return;

	fprintf(fp, "*nat\n"
			":PREROUTING ACCEPT [0:0]\n"
			":POSTROUTING ACCEPT [0:0]\n"
			":OUTPUT ACCEPT [0:0]\n"
			":VSERVER - [0:0]\n");

	/* VSERVER chain */
	if (inet_addr_(wan_pppoe_ipaddr))
		fprintf(fp, "-A PREROUTING -d %s -j VSERVER\n", wan_pppoe_ipaddr);
	if (inet_addr_(wan_ipaddr))
		fprintf(fp, "-A PREROUTING -d %s -j VSERVER\n", wan_ipaddr);

   	if (nvram_match("misc_http_x", "1"))
	{
		int wan_port = 8080;
   		if (nvram_invmatch("misc_httpport_x", ""))
      			wan_port=atoi(nvram_safe_get("misc_httpport_x")); 	
                fprintf(fp, "-A VSERVER -p tcp -m tcp --dport %d -j DNAT --to-destination %s:%s\n",
                        wan_port, lan_ipaddr, nvram_safe_get("http_lanport"));
	}
	
	// upnp port forward
   	if (nvram_match("wan_nat_x", "1") && nvram_invmatch("upnp_enable", "0"))
		write_upnp_forward(fp, wan_if, wan_ipaddr, lan_if, lan_ipaddr, lan_class, logaccept, logdrop);

	// Port forwarding or Virtual Server
	portforwarding_setting(fp);

	// Port Trigger settings
	porttrigger_setting(fp, wan_if, 1);

	if (nvram_match("wan_nat_x", "1") && nvram_invmatch("sp_battle_ips", "0")) {
		#define BASEPORT 6112
		ip2class(lan_ipaddr, lan_netmask, lan_class);

		/* run starcraft patch anyway */
		if (inet_addr_(wan_pppoe_ipaddr)) {
			fprintf(fp, "-A PREROUTING -p udp -d %s --sport %d -j NETMAP --to %s\n", 
							wan_pppoe_ipaddr, BASEPORT, lan_class);
			fprintf(fp, "-A POSTROUTING -p udp -s %s --dport %d -j NETMAP --to %s\n", 
							lan_class, BASEPORT, wan_pppoe_ipaddr);
		}
		else if (inet_addr_(wan_ipaddr)) {
			fprintf(fp, "-A PREROUTING -p udp -d %s --sport %d -j NETMAP --to %s\n", 
							wan_ipaddr, BASEPORT, lan_class);
			fprintf(fp, "-A POSTROUTING -p udp -s %s --dport %d -j NETMAP --to %s\n", 
							lan_class, BASEPORT, wan_ipaddr);
		}
	}

	// Exposed station
	if (nvram_match("wan_nat_x", "1") && nvram_invmatch("dmz_ip", ""))
		fprintf(fp, "-A VSERVER -j DNAT --to %s\n", nvram_safe_get("dmz_ip"));

	if (nvram_match("wan_nat_x", "1")) {
		/* masquerade physical WAN port connection */
		if (inet_addr_(wan_pppoe_ipaddr))
			fprintf(fp, "-A POSTROUTING -o %s ! -s %s -j MASQUERADE\n", 
					nvram_get("wan_pppoe_ifname"), wan_pppoe_ipaddr);
		if (inet_addr_(wan_ipaddr))
			fprintf(fp, "-A POSTROUTING -o %s ! -s %s -j MASQUERADE\n", nvram_get("wan_ifname"), wan_ipaddr);

		// masquerade lan to lan
		ip2class(lan_ipaddr, lan_netmask, lan_class);
		fprintf(fp, "-A POSTROUTING -o %s -s %s -d %s -j MASQUERADE\n", lan_if, lan_class, lan_class);
	}

	fprintf(fp, "COMMIT\n");
	fclose(fp);

#ifdef WEB_REDIRECT
	redirect_setting();
#endif
}

#ifdef WEB_REDIRECT
void rewrite_redirect(void)
{
        FILE *redirect_fp = fopen("/tmp/redirect_rules", "w+");
        char http_rule[256], dns_rule[256];
        char *lan_ipaddr = nvram_safe_get("lan_ipaddr");
        char *lan_netmask = nvram_safe_get("lan_netmask");

        if(redirect_fp == NULL){
                fprintf(stderr, "*** Can't make the file of the redirect rules! ***\n");
                return;
        }
        fprintf(redirect_fp, "*nat\n");
        fprintf(redirect_fp, ":PREROUTING ACCEPT [0:0]\n");

        memset(http_rule, 0, sizeof(http_rule));
        memset(dns_rule, 0, sizeof(dns_rule));

        sprintf(http_rule, "-A PREROUTING ! -d %s/%s -p tcp --dport 80 -j DNAT --to-destination %s:18017\n", lan_ipaddr, lan_netmask, lan_ipaddr);
        sprintf(dns_rule,  "-A PREROUTING -p udp --dport 53 -j DNAT --to-destination %s:18018\n", lan_ipaddr);

        fprintf(redirect_fp, "%s%s", http_rule, dns_rule);
        fprintf(redirect_fp, "COMMIT\n");
        fclose(redirect_fp);
}

void rewrite_redirect_hij(void)
{
        FILE *redirect_fp = fopen("/tmp/hij_rules", "w+");
        char dns_rule[256];
        char *lan_ipaddr = nvram_safe_get("lan_ipaddr");

        if(redirect_fp == NULL){
                fprintf(stderr, "*** Can't make the file of the redirect rules! ***\n");
                return;
        }
        fprintf(redirect_fp, "*nat\n");
        fprintf(redirect_fp, ":PREROUTING ACCEPT [0:0]\n");

        memset(dns_rule, 0, sizeof(dns_rule));
        sprintf(dns_rule,  "-A PREROUTING -p udp --dport 53 -j DNAT --to-destination %s:53\n", lan_ipaddr);
        fprintf(redirect_fp, "%s", dns_rule);

        fprintf(redirect_fp, "COMMIT\n");
        fclose(redirect_fp);
}

void redirect_setting(void)
{
        FILE *nat_fp = fopen("/tmp/nat_rules", "r");
        FILE *redirect_fp = fopen("/tmp/redirect_rules", "w+");
        char tmp_buf[1024];
        char http_rule[256], dns_rule[256];
        char *lan_ipaddr = nvram_safe_get("lan_ipaddr");
        char *lan_netmask = nvram_safe_get("lan_netmask");

        if(redirect_fp == NULL){
                fprintf(stderr, "*** Can't make the file of the redirect rules! ***\n");
                return;
        }

        if(nat_fp != NULL){
                memset(tmp_buf, 0, sizeof(tmp_buf));
                while((fgets(tmp_buf, sizeof(tmp_buf), nat_fp)) != NULL
                                && strncmp(tmp_buf, "COMMIT", 6) != 0){
                        fprintf(redirect_fp, "%s", tmp_buf);
                        memset(tmp_buf, 0, sizeof(tmp_buf));
                }

                fclose(nat_fp);
        }
        else{
                fprintf(redirect_fp, "*nat\n");
                fprintf(redirect_fp, ":PREROUTING ACCEPT [0:0]\n");
        }

        memset(http_rule, 0, sizeof(http_rule));
        memset(dns_rule, 0, sizeof(dns_rule));

        sprintf(http_rule, "-A PREROUTING ! -d %s/%s -p tcp --dport 80 -j DNAT --to-destination %s:18017\n", lan_ipaddr, lan_netmask, lan_ipaddr);
        sprintf(dns_rule, "-A PREROUTING -p udp --dport 53 -j DNAT --to-destination %s:18018\n", lan_ipaddr);

        fprintf(redirect_fp, "%s%s", http_rule, dns_rule);
        fprintf(redirect_fp, "COMMIT\n");

        fclose(redirect_fp);
}
#endif

/* Rules for LW Filter and MAC Filter
 * MAC ACCEPT
 *     ACCEPT -> MACS
 *            	 -> LW Disabled
 *                  MACS ACCEPT
 *               -> LW Default Accept: 
 *                  MACS DROP in rules
 *                  MACS ACCEPT Default
 *               -> LW Default Drop: 
 *                  MACS ACCEPT in rules
 *                  MACS DROP Default
 *     DROP    -> FORWARD DROP 
 *
 * MAC DROP
 *     DROP -> FORWARD DROP
 *     ACCEPT -> FORWARD ACCEPT 
 */
int default_filter_setting()
{
	FILE *fp;

	printf("\nset default filter settings\n");
	if ((fp=fopen("/tmp/filter.default", "w"))==NULL) return -1;

	fprintf(fp, "*filter\n:INPUT ACCEPT [0:0]\n:FORWARD ACCEPT [0:0]\n:OUTPUT ACCEPT [0:0]\n:logaccept - [0:0]\n:logdrop - [0:0]\n");
	fprintf(fp, "-A INPUT -m state --state INVALID -j DROP\n");
	fprintf(fp, "-A INPUT -m state --state RELATED,ESTABLISHED -j ACCEPT\n");
	fprintf(fp, "-A INPUT -i lo -m state --state NEW -j ACCEPT\n");
	fprintf(fp, "-A INPUT -i br0 -m state --state NEW -j ACCEPT\n");
	fprintf(fp, "-A INPUT -j DROP\n");
	fprintf(fp, "-A FORWARD -m state --state INVALID -j DROP\n");
	fprintf(fp, "-A FORWARD -m state --state ESTABLISHED,RELATED -j ACCEPT\n");
	fprintf(fp, "-A FORWARD -i br0 -o br0 -j ACCEPT\n");
	fprintf(fp, "-A FORWARD -i lo -o lo -j ACCEPT\n");
	fprintf(fp, "-A FORWARD -j DROP\n");
        fprintf(fp, "-A logaccept -m state --state NEW -j LOG --log-prefix \"ACCEPT \" "
                  "--log-tcp-sequence --log-tcp-options --log-ip-options\n"
                  "-A logaccept -j ACCEPT\n");

        fprintf(fp,"-A logdrop -m state --state NEW -j LOG --log-prefix \"DROP\" "
                  "--log-tcp-sequence --log-tcp-options --log-ip-options\n"
                  "-A logdrop -j DROP\n");
        fprintf(fp, "COMMIT\n\n");
        fclose(fp);

        eval("iptables-restore", "/tmp/filter.default");
}

void lanwanfilter_setting(FILE *fp, char *wan_if, char *lan_if, char *logaccept, char *logdrop)
{
	char chain[8];
	char *ftype, *dtype;

	if (nvram_match("macfilter_enable_x", "1"))
		strcpy(chain, "MACS");
	else
		strcpy(chain, "FORWARD");

	if (nvram_match("filter_lw_default_x", "DROP")) {
		dtype = logdrop;
		ftype = logaccept;
	}
	else {
		dtype = logaccept;
		ftype = logdrop;
	}

	char *lw_time1 = nvram_get("filter_lw_time_x");
	char *lw_time2 = nvram_get("filter_lw_time2_x");
	char *lw_date = nvram_get("filter_lw_date_x");
	char lw_date1[8], lw_date2[8], timef1[256], timef2[256];
	int t1, t2;

	// Mon/Tue/Wed/Thu/Fri
	memset(lw_date1, 0, sizeof(lw_date1));
	sprintf(lw_date1, "0%c%c%c%c%c0", lw_date[1], lw_date[2], lw_date[3], lw_date[4], lw_date[5]);
	t1 = makeTimestr(lw_time1, lw_date1, timef1);
	// Sun/Sat
	memset(lw_date2, 0, sizeof(lw_date2));
	sprintf(lw_date2, "%c00000%c", lw_date[0], lw_date[6]);
	t2 = makeTimestr(lw_time2, lw_date2, timef2);

	if (!t1 || !t2) {
		// LAN/WAN filter
		char *proto, *srcip, *srcport, *dstip, *dstport;
		char protoptr[16], flagptr[16], srcipbuf[32], dstipbuf[32], buf[256];
		char *nv, *nvp, *b;
		nv = nvp = strdup(nvram_safe_get("filter_lwlist"));
		while (nv && (b = strsep(&nvp, "<")) != NULL) {
			if ((vstrsep(b, ">", &srcip, &srcport, &dstip, &dstport, &proto) != 5))
				continue;

			(void)protoflag_conv(proto, protoptr, 0);
			(void)protoflag_conv(proto, flagptr, 1);
			(void)iprange_ex_conv(srcip, srcipbuf);
			(void)iprange_ex_conv(dstip, dstipbuf);
			filter_conv(protoptr, flagptr, srcipbuf, srcport, dstipbuf, dstport, buf);

			if (!t1)
				fprintf(fp, "-A %s %s -i %s -o %s %s -j %s\n", chain, timef1, lan_if, wan_if, buf, ftype);
			if (!t2)
				fprintf(fp, "-A %s %s -i %s -o %s %s -j %s\n", chain, timef2, lan_if, wan_if, buf, ftype);
		}
		free(nv);

		// ICMP
		char ptr[32], *icmplist;
		foreach(ptr, nvram_safe_get("filter_lw_icmp_x"), icmplist) {
			if (!t1)
				fprintf(fp, "-A %s %s -i %s -o %s -p icmp --icmp-type %s -j %s\n", chain, timef1, lan_if, wan_if, ptr, ftype);
			if (!t2)
				fprintf(fp, "-A %s %s -i %s -o %s -p icmp --icmp-type %s -j %s\n", chain, timef2, lan_if, wan_if, ptr, ftype);
		}

		// Default
		fprintf(fp, "-A %s -i %s -o %s -j %s\n", chain, lan_if, wan_if, dtype);
	}
}

void wanlanfilter_setting(FILE *fp, char *wan_if, char *lan_if, char *logaccept, char *logdrop)
{
	char *ftype, *dtype;

	if (nvram_match("filter_wl_default_x", "DROP")) {
		dtype = logdrop;
		ftype = logaccept;
	}
	else {
		dtype = logaccept;
		ftype = logdrop;
	}

	char *wl_time1 = nvram_get("filter_wl_time_x");
	char *wl_time2 = nvram_get("filter_wl_time2_x");
	char *wl_date = nvram_get("filter_wl_date_x");
	char wl_date1[8], wl_date2[8], timef1[256], timef2[256];
	int t1, t2;

	// Mon/Tue/Wed/Thu/Fri
	memset(wl_date1, 0, sizeof(wl_date1));
	sprintf(wl_date1, "0%c%c%c%c%c0", wl_date[1], wl_date[2], wl_date[3], wl_date[4], wl_date[5]);
	t1 = makeTimestr(wl_time1, wl_date1, timef1);
	// Sun/Sat
	memset(wl_date2, 0, sizeof(wl_date2));
	sprintf(wl_date2, "%c00000%c", wl_date[0], wl_date[6]);
	t2 = makeTimestr(wl_time2, wl_date2, timef2);

	if (!t1 || !t2) {
		char *proto, *srcip, *srcport, *dstip, *dstport;
		char protoptr[16], flagptr[16], srcipbuf[32], dstipbuf[32], buf[256];
		char *nv, *nvp, *b;
		nv = nvp = strdup(nvram_safe_get("filter_wllist"));
		while (nv && (b = strsep(&nvp, "<")) != NULL) {
			if ((vstrsep(b, ">", &srcip, &srcport, &dstip, &dstport, &proto) != 5))
				continue;

			(void)protoflag_conv(proto, protoptr, 0);
			(void)protoflag_conv(proto, flagptr, 1);
			(void)iprange_ex_conv(srcip, srcipbuf);
			(void)iprange_ex_conv(dstip, dstipbuf);
			filter_conv(protoptr, flagptr, srcipbuf, srcport, dstipbuf, dstport, buf);

			if (!t1)
				fprintf(fp, "-A FORWARD %s -i %s -o %s %s -j %s\n", timef1, wan_if, lan_if, buf, ftype);
			if (!t2)
				fprintf(fp, "-A FORWARD %s -i %s -o %s %s -j %s\n", timef2, wan_if, lan_if, buf, ftype);
		}
		free(nv);

		// ICMP
		char ptr[32], *icmplist;
		foreach(ptr, nvram_safe_get("filter_wl_icmp_x"), icmplist) {
			if (!t1)
				fprintf(fp, "-A FORWARD %s -i %s -o %s -p icmp --icmp-type %s -j %s\n", timef1, wan_if, lan_if, ptr, ftype);
			if (!t2)
				fprintf(fp, "-A FORWARD %s -i %s -o %s -p icmp --icmp-type %s -j %s\n", timef2, wan_if, lan_if, ptr, ftype);
		}

		// Default
		//fprintf(fp, "-A FORWARD -i %s -o %s -j %s\n", wan_if, lan_if, dtype);
	}
}

#ifdef WEBSTRFILTER
void urlfilter_setting(FILE *fp)
{
	char *url_time = nvram_get("url_time_x");
	char *url_date = nvram_get("url_date_x");
	char timef[256], *filterstr;
	char *nv, *nvp, *b;

	if (!makeTimestr(url_time, url_date, timef)) {
		nv = nvp = strdup(nvram_safe_get("url_rulelist"));
		while (nv && (b = strsep(&nvp, "<")) != NULL) {
			if (vstrsep(b, ">", &filterstr) != 1)
				continue;
			if (strlen(filterstr)==0)
				continue;

			fprintf(fp,"-I FORWARD -p tcp %s -m webstr --url \"%s\" -j DROP\n", timef, filterstr);
		}
		free(nv);
	}
}

void keywordfilter_setting(FILE *fp)
{
	char *keyword_time = nvram_get("keyword_time_x");
	char *keyword_date = nvram_get("keyword_date_x");
	char timef[256], *filterstr;
	char *nv, *nvp, *b;

	if (!makeTimestr(keyword_time, keyword_date, timef)) {
		nv = nvp = strdup(nvram_safe_get("keyword_rulelist"));
		while (nv && (b = strsep(&nvp, "<")) != NULL) {
			if (vstrsep(b, ">", &filterstr) != 1)
				continue;

			if (strcmp(filterstr, ""))
				fprintf(fp,"-I FORWARD -p tcp --sport 80 %s -m string --string \"%s\" --algo bm -j DROP\n", timef, filterstr);
		}
		free(nv);
	}
}
#endif

int makeTimestr(char *_time, char *_date, char *tf)
{
	static const char *days[7] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
	int i, comma = 0;

	if ((!_time) || (!_date) || !strcmp(_date, "0000000")) {
		//fprintf(stderr, "URL filter get time fail\n");
		return -1;
	}

	sprintf(tf, "-m time --timestart %c%c:%c%c --timestop %c%c:%c%c --kerneltz --weekdays ", 
		_time[0], _time[1], _time[2], _time[3], _time[4], _time[5], _time[6], _time[7]);

	for (i=0; i<7; ++i) {
		if (_date[i] == '1') {
			if (comma == 1)
				strncat(tf, ",", 1);

			strncat(tf, days[i], 3);
			comma = 1;
		}
	}

	//fprintf(stderr, "URL filter time module str is [%s]\n", tf);

	return 0;
}

int filter_setting(char *wan_if, char *lan_if, char *logaccept, char *logdrop)
{

	FILE *fp;
        char *proto, *flag, *srcip, *srcport, *dstip, *dstport;
	char line[256];
	char macaccept[32];
	char *ftype, *dtype, *fftype;
	int num;
	int i;
	int wan_port;

	if ((fp=fopen("/tmp/filter_rules", "w"))==NULL) return -1;

	fprintf(fp, "*filter\n:INPUT ACCEPT [0:0]\n:FORWARD ACCEPT [0:0]\n:OUTPUT ACCEPT [0:0]\n:MACS - [0:0]\n:logaccept - [0:0]\n:logdrop - [0:0]\n");

	strcpy(macaccept, "");

	// FILTER from LAN to WAN Source MAC
	if(nvram_invmatch("macfilter_enable_x", "0"))
	{   		
		// LAN/WAN filter		
		if (nvram_match("macfilter_enable_x", "2"))
		{
			dtype = logaccept;
			ftype = logdrop;
			fftype = logdrop;
		}
		else
		{
			dtype = logdrop;
			ftype = logaccept;

			strcpy(macaccept, "MACS");
			fftype = macaccept;
		}
	
		num = atoi(nvram_safe_get("macfilter_num_x"));

		for(i=0;i<num;i++)
		{	
         		fprintf(fp, "-A INPUT -i %s -m mac --mac-source %s -j %s\n", lan_if, mac_conv("macfilter_list_x", i, line), ftype);
         		fprintf(fp, "-A FORWARD -i %s -m mac --mac-source %s -j %s\n", lan_if, mac_conv("macfilter_list_x", i, line), fftype);
		} 
	} 

	if (nvram_invmatch("fw_enable_x", "1"))
	{
		if (nvram_match("macfilter_enable_x", "1"))
		{
			/* Filter known SPI state */
			fprintf(fp, "-A INPUT -i %s -m state --state NEW -j %s\n"
			,lan_if, logdrop);
		}
	}
	else
	{	
		if (nvram_match("macfilter_enable_x", "1"))
		{
			/* Filter known SPI state */
			fprintf(fp, "-A INPUT -m state --state INVALID -j %s\n"
		          "-A INPUT -m state --state RELATED,ESTABLISHED -j %s\n"
	        	  "-A INPUT -i lo -m state --state NEW -j %s\n"
		          "-A INPUT -i %s -m state --state NEW -j %s\n"
			,logdrop, logaccept, "ACCEPT", lan_if, logdrop);
		}
		else
		{
			/* Filter known SPI state */
			fprintf(fp, "-A INPUT -m state --state INVALID -j %s\n"
		          "-A INPUT -m state --state RELATED,ESTABLISHED -j %s\n"
	        	  "-A INPUT -i lo -m state --state NEW -j %s\n"
		          "-A INPUT -i %s -m state --state NEW -j %s\n"
			,logdrop, logaccept, "ACCEPT", lan_if, "ACCEPT");
		}

        /* Pass multicast */
        if (nvram_match("mr_enable_x", "1")) {
                fprintf(fp, "-A INPUT -p 2 -d 224.0.0.0/4 -j %s\n", logaccept);
                fprintf(fp, "-A INPUT -p udp -d 224.0.0.0/4 -j %s\n", logaccept);
        }
	/* enable incoming packets from broken dhcp servers, which are sending replies
	 * from addresses other than used for query, this could lead to lower level
	 * of security, but it does not work otherwise (conntrack does not work) :-( 
	 */
		if (nvram_match("wan_proto", "dhcp"))
			fprintf(fp, "-A INPUT -p udp --sport 67 --dport 68 -j %s\n", logaccept);
 
		// Firewall between WAN and Local
		if (nvram_match("misc_http_x", "1"))
		{
			fprintf(fp, "-A INPUT -p tcp -m tcp -d %s --dport 80 -j %s\n", nvram_safe_get("lan_ipaddr"), logaccept);
		}

		if (nvram_match("usb_webenable_x", "2"))
		{
			fprintf(fp, "-A INPUT -p tcp -m tcp --dport %s -j %s\n", nvram_safe_get("usb_webhttpport_x"), logaccept); // oleg patch
			fprintf(fp, "-A INPUT -p tcp -m tcp --dport %s -j %s\n", nvram_safe_get("usb_webactivex_x"), logaccept);  // oleg patch
		}

		if (nvram_invmatch("usb_ftpenable_x", "0"))
		{	
			fprintf(fp, "-A INPUT -p tcp -m tcp --dport %s -j %s\n", nvram_safe_get("usb_ftpport_x"), logaccept);	// oleg patch
		}

		if (nvram_invmatch("misc_ping_x", "0"))
			fprintf(fp, "-A INPUT -p icmp -j %s\n", logaccept);

		if (nvram_invmatch("misc_lpr_x", "0"))
		{
   	             	fprintf(fp, "-A INPUT -p tcp -m tcp --dport %d -j %s\n", 515, logaccept);	// oleg patch
        	        fprintf(fp, "-A INPUT -p tcp -m tcp --dport %d -j %s\n", 9100, logaccept);	// oleg patch
                	fprintf(fp, "-A INPUT -p tcp -m tcp --dport %d -j %s\n", 3838, logaccept);	// oleg patch
		}

		fprintf(fp, "-A INPUT -j %s\n", logdrop);	// always accept for wan-telnet(other app)-dut case
	}

/* apps_dm DHT patch */
	if (nvram_match("apps_dl_share", "1"))
	{
		fprintf(fp, "-I INPUT -p udp --dport 6881 -j ACCEPT\n");	// DHT port
		// port range
		fprintf(fp, "-I INPUT -p udp --dport %s:%s -j ACCEPT\n", nvram_safe_get("apps_dl_share_port_from"), nvram_safe_get("apps_dl_share_port_to"));
		fprintf(fp, "-I INPUT -p tcp --dport %s:%s -j ACCEPT\n", nvram_safe_get("apps_dl_share_port_from"), nvram_safe_get("apps_dl_share_port_to"));
	}

        /* Pass multicast */
        if (nvram_match("mr_enable_x", "1"))
        {
                fprintf(fp, "-A FORWARD -p udp -d 224.0.0.0/4 -j ACCEPT\n");
                if (strlen(macaccept)>0)
                        fprintf(fp, "-A MACS -p udp -d 224.0.0.0/4 -j ACCEPT\n");
        }

        /* Clamp TCP MSS to PMTU of WAN interface before accepting RELATED packets */
        if (nvram_match("wan_proto", "pptp") || nvram_match("wan_proto", "pppoe") || nvram_match("wan_proto", "l2tp"))
        {
                fprintf(fp, "-A FORWARD -p tcp --syn -j TCPMSS --clamp-mss-to-pmtu\n");
                if (strlen(macaccept)>0)
                        fprintf(fp, "-A MACS -p tcp --syn -j TCPMSS --clamp-mss-to-pmtu\n");
        }
	fprintf(fp, "-A FORWARD -m state --state ESTABLISHED,RELATED -j %s\n", logaccept);
	if (strlen(macaccept)>0)
		fprintf(fp, "-A MACS -m state --state ESTABLISHED,RELATED -j %s\n", logaccept);
        /* Filter out invalid WAN->WAN connections */
        fprintf(fp, "-A FORWARD -o %s ! -i %s -j %s\n", wan_if, lan_if, logdrop); 
	if (nvram_invmatch("wan_ifname", wan_if))
                fprintf(fp, "-A FORWARD -o %s ! -i %s -j %s\n", nvram_get("wan_ifname"), lan_if, logdrop);
	/* Drop the wrong state, INVALID, packets */
	fprintf(fp, "-A FORWARD -m state --state INVALID -j %s\n", logdrop);
	if (strlen(macaccept)>0)
		fprintf(fp, "-A MACS -m state --state INVALID -j %s\n", logdrop);

	/* Accept the redirect, might be seen as INVALID, packets */
	fprintf(fp, "-A FORWARD -i %s -o %s -j %s\n", lan_if, lan_if, logaccept);	
	if (strlen(macaccept)>0)
	{
		fprintf(fp, "-A MACS -i %s -o %s -j %s\n", lan_if, lan_if, logaccept);
	}

	if (nvram_match("fw_enable_x", "1") && nvram_match("misc_ping_x", "0"))
		fprintf(fp, "-A FORWARD -i %s -p icmp -j DROP\n", wan_if);

	// DoS attacks
	if (nvram_match("fw_enable_x", "1") && nvram_match("fw_dos_x", "1")) {
		// sync-flood protection
		fprintf(fp, "-A FORWARD -i %s -p tcp --syn -m limit --limit 1/s -j %s\n", wan_if, logaccept);
		// furtive port scanner
		fprintf(fp, "-A FORWARD -i %s -p tcp --tcp-flags SYN,ACK,FIN,RST RST -m limit --limit 1/s -j %s\n", wan_if, logaccept);
		// ping of death
		fprintf(fp, "-A FORWARD -i %s -p icmp --icmp-type echo-request -m limit --limit 1/s -j %s\n", wan_if, logaccept);
	}

	// FILTER from LAN to WAN
        // Rules for MAC Filter and LAN to WAN Filter
        // Drop rules always before Accept
	if (nvram_match("fw_lw_enable_x", "1"))
		lanwanfilter_setting(fp, wan_if, lan_if, logaccept, logdrop);
	else if (nvram_match("macfilter_enable_x", "1"))
	{
         	fprintf(fp, "-A FORWARD -i %s -o %s -j %s\n", lan_if, wan_if, logdrop);
         	fprintf(fp, "-A MACS -i %s -o %s -j %s\n", lan_if, wan_if, logaccept);
	}

	// Filter from WAN to LAN
	if (nvram_match("fw_wl_enable_x", "1"))
		wanlanfilter_setting(fp, wan_if, lan_if, logaccept, logdrop);

	/* Write forward chain rules of NAT */
	// Port Trigger settings
	porttrigger_setting(fp, wan_if, 0);

	if (nvram_match("wan_nat_x", "1") && nvram_invmatch("sp_battle_ips", "0"))
		fprintf(fp, "-A FORWARD -p udp --dport %d -j %s\n", BASEPORT, logaccept);

	if(nvram_match("fw_wl_enable_x", "1")) // Thanks for Oleg
	{   		
		// Default
		fprintf(fp, "-A FORWARD -i %s -o %s -j %s\n", wan_if, lan_if, 
			nvram_match("filter_wl_default_x", "DROP") ? logdrop : logaccept);
	}
	// logaccept chain
	fprintf(fp, "-A logaccept -m state --state NEW -j LOG --log-prefix \"ACCEPT \" "
		  "--log-tcp-sequence --log-tcp-options --log-ip-options\n"
		  "-A logaccept -j ACCEPT\n");

	// logdrop chain
	fprintf(fp,"-A logdrop -m state --state NEW -j LOG --log-prefix \"DROP\" "
		  "--log-tcp-sequence --log-tcp-options --log-ip-options\n"
		  "-A logdrop -j DROP\n");

#ifdef WEBSTRFILTER
	if (nvram_match("url_enable_x", "1"))
		urlfilter_setting(fp);

	if (nvram_match("keyword_enable_x", "1"))
		keywordfilter_setting(fp);
#endif

	fprintf(fp, "COMMIT\n\n");
       	fclose(fp);

	//eval("iptables", "-F");
	eval("iptables-restore", "/tmp/filter_rules");
}

void portforwarding_setting(FILE *fp)
{
	char *nv, *nvp, *b;
	char *proto, *protono, *port, *lport, *dstip, *desc;
	char dstips[64];

	if (nvram_match("wan_nat_x", "0") || nvram_match("vts_enable_x", "0"))
		return;

	nvp = nv = strdup(nvram_safe_get("vts_rulelist"));
	while (nv && (b = strsep(&nvp, "<")) != NULL) {
		char *portv, *portp, *c;

		if ((vstrsep(b, ">", &desc, &port, &dstip, &lport, &proto) != 5))
			continue;

		// Handle port1,port2,port3 format
		portp = portv = strdup(port);
		while (portv && (c = strsep(&portp, ",")) != NULL) {
			if (lport && *lport)
				snprintf(dstips, sizeof(dstips), "--to-destination %s:%s", dstip, lport);
			else
				snprintf(dstips, sizeof(dstips), "--to %s", dstip);

			if (strcmp(proto, "TCP") == 0 || strcmp(proto, "BOTH") == 0) {
				fprintf(fp, "-A VSERVER -p tcp -m tcp --dport %s -j DNAT %s\n", c, dstips);

				if (!strcmp(c, "21") 
						&& atoi(nvram_safe_get("vts_ftpport")) != 0 
						&& atoi(nvram_safe_get("vts_ftpport")) != 21)
					fprintf(fp, "-A VSERVER -p tcp -m tcp --dport %s -j DNAT --to-destination %s:21\n", nvram_safe_get("vts_ftpport"), nvram_safe_get("lan_ipaddr"));
			}
			if (strcmp(proto, "UDP") == 0 || strcmp(proto, "BOTH") == 0)
				fprintf(fp, "-A VSERVER -p udp -m udp --dport %s -j DNAT %s\n", c, dstips);
			// Handle raw protocol in port field, no val1:val2 allowed
			if (strcmp(proto, "OTHER") == 0) {
				protono = strsep(&c, ":");
				fprintf(fp, "-A VSERVER -p %s -j DNAT --to %s\n", protono, dstip);
			}
		}
		free(portv);
	}
	free(nv);
}

void porttrigger_setting(FILE *fp, char *wan_if, int is_nat)
{
	char *nv, *nvp, *b;
	char *out_proto, *in_proto, *out_port, *in_port, *desc;
	char out_protoptr[16], in_protoptr[16];
	int first = 1;

	if (nvram_match("wan_nat_x", "0") || nvram_match("autofw_enable_x", "0"))
		return;

	if (is_nat) {
		fprintf(fp, "-A VSERVER -j TRIGGER --trigger-type dnat\n");
		return;
	}

	nvp = nv = strdup(nvram_safe_get("autofw_rulelist"));
	while (nv && (b = strsep(&nvp, "<")) != NULL) {
		if ((vstrsep(b, ">", &desc, &out_port, &out_proto, &in_port, &in_proto) != 5))
			continue;

		if (first) {
			fprintf(fp, ":triggers - [0:0]\n");
			fprintf(fp, "-A FORWARD -o %s -j triggers\n", wan_if);
			fprintf(fp, "-A FORWARD -i %s -j TRIGGER --trigger-type in\n", wan_if);
			first = 0;
		}
		(void)proto_conv(in_proto, in_protoptr);
		(void)proto_conv(out_proto, out_protoptr);

		fprintf(fp, "-A FORWARD -p %s -m %s --dport %s "
			"-j TRIGGER --trigger-type out --trigger-proto %s --trigger-match %s --trigger-relate %s\n",
			out_protoptr, out_protoptr, out_port,
			in_protoptr, out_port, in_port);
	}
	free(nv);
}

int start_firewall(char *wan_if, char *lan_if)
{
	DIR *dir;
	struct dirent *file;
	FILE *fp;
	char name[NAME_MAX];
        char logaccept[32], logdrop[32];
        char *mcast_ifname = nvram_get("wan_ifname");
       
	if (nvram_match("wan_route_x", "0"))
		return -1;

	fprintf(stderr, "%s: %s/%s\n", __FUNCTION__, wan_if, lan_if);

        /* mcast needs rp filter to be turned off only for non default iface */
        if (!nvram_match("mr_enable_x", "1") || strcmp(wan_if, mcast_ifname) == 0) 
                mcast_ifname = NULL;

	/* Block obviously spoofed IP addresses */
	if (!(dir = opendir("/proc/sys/net/ipv4/conf")))
		perror("/proc/sys/net/ipv4/conf");
	while (dir && (file = readdir(dir))) {
		if (strncmp(file->d_name, ".", NAME_MAX) != 0 &&
		    strncmp(file->d_name, "..", NAME_MAX) != 0) {
			sprintf(name, "/proc/sys/net/ipv4/conf/%s/rp_filter", file->d_name);
			if (!(fp = fopen(name, "r+"))) {
				perror(name);
				break;
                        }
                        fputc(mcast_ifname && strncmp(file->d_name, 	// oleg patch
                                mcast_ifname, NAME_MAX) == 0 ? '0' : '1', fp);
			fclose(fp);	// oleg patch
		}
	}
	closedir(dir);

	/* Determine the log type */
	if (nvram_match("fw_log_x", "accept") || nvram_match("fw_log_x", "both"))
		strcpy(logaccept, "logaccept");
	else strcpy(logaccept, "ACCEPT");

	if (nvram_match("fw_log_x", "drop") || nvram_match("fw_log_x", "both"))
		strcpy(logdrop, "logdrop");
	else strcpy(logdrop, "DROP");

	/* nat setting */
	nat_setting(wan_if, lan_if, logaccept, logdrop);

	/* Filter setting */
	filter_setting(wan_if, lan_if, logaccept, logdrop);

	/* Add max conntrack as 4096, thanks for Oleg */
	if ((fp=fopen("/proc/sys/net/ipv4/netfilter/ip_conntrack_max", "w+")))
	{
		fputs("1024", fp);
		fclose(fp);
	}

	if( (fp=fopen("/proc/sys/net/ipv4/ip_forward", "r+")) ){
		fputc('1', fp);
		fclose(fp);
	} else
		perror("/proc/sys/net/ipv4/ip_forward");

	/* Tweak NAT performance... */
	if ((fp=fopen("/proc/sys/net/ipv4/tcp_fin_timeout", "w+")))
	{
		fputs("30", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/tcp_keepalive_intvl", "w+")))
	{
		fputs("24", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/tcp_keepalive_probes", "w+")))
	{
		fputs("3", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/tcp_keepalive_time", "w+")))
	{
		fputs("1800", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/tcp_retries2", "w+")))
	{
		fputs("5", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/tcp_syn_retries", "w+")))
	{
		fputs("3", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/tcp_synack_retries", "w+")))
	{
		fputs("3", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/tcp_tw_recycle", "w+")))
	{
		fputs("1", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/tcp_tw_reuse", "w+")))
	{
		fputs("1", fp);
		fclose(fp);
	}

	/* Tweak DoS-related... */

	if ((fp=fopen("/proc/sys/net/ipv4/icmp_ignore_bogus_error_responses", "w+")))
	{
		fputs("1", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/tcp_abort_on_overflow", "w+")))
	{
		fputs("1", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/tcp_rfc1337", "w+")))
	{
		fputs("1", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/tcp_syncookies", "w+")))
	{
		fputs("1", fp);
		fclose(fp);
	}

	return 0;
}
