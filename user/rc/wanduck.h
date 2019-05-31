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
#ifndef _wanduck_h_
#define _wanduck_h_

#include <stdio.h>
#include <signal.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>

#include <nvram/bcmnvram.h>

#include <shared.h>
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

#define RFC1123FMT		"%a, %d %b %Y %H:%M:%S GMT"

#define MAXLINE			2048
#define PATHLEN			256
#define DFL_HTTP_SERV_PORT	"18017"
#define DFL_DNS_SERV_PORT	"18018"
#define MAX_USER		100

#define T_HTTP			'H'
#define T_DNS			'D'

#define DISCONN			0
#define CONNED			1
#define C2D			3
#define D2C			4

#define CASE_DISWAN		1
#define CASE_PPPFAIL		2
#define CASE_DHCPFAIL		3
#define CASE_MISROUTE		4
#define CASE_OTHERS		5
#define CASE_THESAMESUBNET	6
#define CASE_DISPARENTAP	8

#pragma pack(1)
typedef struct Flags_Pack {
        uint16_t reply_1:4,     // bit:0-3
        non_auth_ok:1,          // bit:4
        answer_auth:1,          // bit:5
        reserved:1,             // bit:6
        recur_avail:1,          // bit:7
        recur_desired:1,        // bit:8
        truncated:1,            // bit:9
        authori:1,              // bit:10
        opcode:4,               // bit:11-14
        response:1;             // bit:15
} flag_pack;

typedef struct DNS_HEADER {
        uint16_t tid;
        union {
                uint16_t flag_num;
                flag_pack flags;
        } flag_set;
        uint16_t questions;
        uint16_t answer_rrs;
        uint16_t auth_rrs;
	uint16_t additional_rss;
} dns_header;

typedef struct QUERIES {
	char name[PATHLEN];
	uint16_t type;
	uint16_t ip_class;
} dns_queries;

typedef struct ANSWER {
	uint16_t name;
	uint16_t type;
	uint16_t ip_class;
	uint32_t ttl;	// time to live
	uint16_t data_len;
	uint32_t addr;
} dns_answer;

typedef struct DNS_REQUEST {
	dns_header header;
	dns_queries queries;
} dns_query_packet;

typedef struct DNS_RESPONSE {
	dns_header header;
	char *q_name;
	uint16_t q_type;
	uint16_t q_class;
	dns_answer answers;
} dns_response_packet;

typedef struct REQCLIENT {
	int sfd;
	char type;
} clients;
#pragma pack()

static int http_sock, dns_sock, maxfd;
static clients client[MAX_USER];
static fd_set rset, allset;
static int fd_i;
static int cur_sockfd, connfd;

static int disconn_case;
static int err_state;
static int isFirstUse;
static int _sw_mode = 0;
static int query_udhcpc = 0;
static int wait_udhcpc = 0;

static char wan_ifname[8];
static char wan_proto[16];
static char wan_subnet_t[11];
static char lan_subnet_t[11];
static char default_domain_name[32];

#define Change_To_NAT_rules()		eval("iptables-restore", "/tmp/nat_rules")
#define Change_To_Redirect_rules()	eval("iptables-restore", "/tmp/redirect_rules")
#define Change_To_HIJ_rules()		eval("iptables-restore", "/tmp/hij_rules")

#ifdef SWMODE_ADAPTER_SUPPORT
int u_during_scan(void);
#endif
#endif
