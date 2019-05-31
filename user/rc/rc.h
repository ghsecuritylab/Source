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
 * Router rc control script
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

#ifndef _rc_h_
#define _rc_h_

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "../../autoconf.h"

#if defined(CONFIG_DEFAULTS_RALINK_MT7620)
#include <mt7620.h>
#elif defined(CONFIG_DEFAULTS_RALINK_MT7621)
#include <mt7621.h>
#elif defined(CONFIG_DEFAULTS_RALINK_MT7628)
#include <mt7628.h>
#elif defined(CONFIG_DEFAULTS_RALINK_RT3883)
#include <rt3883.h>
#else
#error Invalid Product!!
#endif

#define IFUP (IFF_UP | IFF_RUNNING | IFF_BROADCAST | IFF_MULTICAST)

#define sin_addr(s) (((struct sockaddr_in *)(s))->sin_addr)

/* udhcpc.c */
extern int udhcpc_main(int argc, char **argv);

/* ppp.c */
extern int ipup_main(void);
extern int ipdown_main(void);

/* init.c */
extern int console_init(void);
extern pid_t run_shell(int timeout, int nowait);
extern void signal_init(void);
extern void fatal_signal(int sig);

/* common.c */
extern in_addr_t inet_addr_(const char *cp);

/* network.c */
extern int ifconfig(char *ifname, int flags, char *addr, char *netmask);
extern int route_add(char *name, int metric, char *dst, char *gateway, char *genmask);
extern int route_del(char *name, int metric, char *dst, char *gateway, char *genmask);
extern void config_loopback(void);
extern void start_lan(int _sw_mode);
extern void stop_lan(int _sw_mode);
extern void lan_up(char *lan_ifname);
extern void lan_down(char *lan_ifname);
extern void start_wan(int _sw_mode);
extern void stop_wan(void);
extern void wan_up(char *ifname);
extern void wan_down(char *ifname);
extern int preset_wan_routes(char *ifname);

/* services.c */
extern int start_udhcpc(char *ifname1, char *ifname2, char *hostname);
extern int stop_udhcpc(void);
extern int start_dnsmasq(void);
extern int stop_dnsmasq(void);
extern int start_ntpc(void);
extern int stop_ntpc(void);
extern int start_services(void);
extern int stop_services(void);

/* firewall.c */
#ifdef WEB_REDIRECT
extern void rewrite_redirect(void);
extern void rewrite_redirect_hij(void);
extern void redirect_setting(void);
#endif

// for log message title
#define LOGNAME MODEL_NAME

/* pid of daemon */
#define APCLI_MONITOR_PIDFILE	"/var/run/apcli_monitor.pid"

/* factory parameter address */
#define MAC_EEPROM_ADDR		0x40004 //6 bytes
#define MAC_EEPROM_ADDR_5G	0x48004	//6 bytes
#ifdef EEPROM_BACKWARD_COMPATIBILITY
#define COUNTRY_EEPROM_ADDR	0x40200 //2 bytes
#define BOOTVER_EEPROM_ADDR	0x40202 //2 bytes
#define ATE_EEPROM_ADDR		0x40204 //1 bytes
#endif
#define PINCODE_EEPROM_ADDR	0x40205 //8 byte
#define MODEL_NAME_ADDR		0x4020d //12 byte
#define SERIAL_NUMBER_LENGTH	12
#define SERIAL_NUMBE_ADDR	0x40219	//12 byte
#ifndef EEPROM_BACKWARD_COMPATIBILITY
#define BOOTVER_EEPROM_ADDR	0x40230 //4 bytes
#endif
#define MAX_REGDOMAIN_LEN	10
#define MAX_REGSPEC_LEN		4
#define REG2G_EEPROM_ADDR	0x40234 //10 bytes
#define REG5G_EEPROM_ADDR	0x4023E //10 bytes
#define REGSPEC_ADDR		0x40248 // 4 bytes
//#define NEXT			0x4024c

#endif /* _rc_h_ */
