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

#include "wanduck.h"

#ifdef SWMODE_ADAPTER_SUPPORT
static int chk_ea_connStatus(void)
{
	if (nvram_match("sta0_auth_mode", "open") 
			&& (nvram_match("sta0_wep_x", "1") || nvram_match("sta0_wep_x", "2")) 
			&& nvram_match("open_wep_err","1"))
		;
	else if (ea_connStatus())
		return 1;
	return 0;
}

static void chg_dhcp_mode(int enable_dhcpd)
{
	if (nvram_match("dhcp_enable_x", "0") && !isFirstUse)
		return;

	int is_during_user_scan = u_during_scan();
	fprintf(stderr, "### change dhcp mode:%d ###\n", enable_dhcpd);
	nvram_set("dnsqmode", enable_dhcpd?"2":"1");
	restart_dnsmasq(!is_during_user_scan);
}
#endif

static void redirect_func(int enable)
{
	if (!enable) {
		switch (_sw_mode) {
			case 1:
			case 5:
			case 6:
				Change_To_NAT_rules();
				break;
			case 2:
			case 3:
				Change_To_HIJ_rules();
				break;
#ifdef SWMODE_ADAPTER_SUPPORT
			case 4:
				chg_dhcp_mode(0);
				break;
#endif
			default:
				;
		}
	}
	else {
		switch (_sw_mode) {
			case 1:
			case 2:
			case 3:
			case 5:
			case 6:
				Change_To_Redirect_rules();
				break;
#ifdef SWMODE_ADAPTER_SUPPORT
			case 4:
				chg_dhcp_mode(1);
				break;
#endif
			default:
				;
		}

	}
}

static void bridge_wan(void)
{
	if (_sw_mode == 1 && query_udhcpc) {
		fprintf(stderr, "### %s ###\n", __FUNCTION__);
		eval("brctl", "addif", "br0", "vlan2");
        }
}

static void unbridge_wan(void)
{
	if (_sw_mode == 1 && query_udhcpc) {
		fprintf(stderr, "### %s ###\n", __FUNCTION__);
		eval("brctl", "delif", "br0", "vlan2");
		wait_udhcpc = 0;
        }
	wait_udhcpc = 0;
}

static void chk_ppp_status(void)
{
	FILE *fp;
	char line[128], buf[128], *p;

	if ((fp = fopen("/tmp/wanstatus.log", "r")) && fgets(line, sizeof(line), fp)) {
		p = strstr(line, ",");
		strcpy(buf, p+1);
		fclose(fp);
	}

	if (strstr(buf, "Failed to authenticate ourselves to peer"))
		nvram_set("wan_state", "4");
}

static int wan_got_ip(void)
{
	int s, ret = 0;
	struct ifreq ifr;

	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		return ret;

	strncpy(ifr.ifr_name, wan_ifname, IFNAMSIZ);
	if (!ioctl(s, SIOCGIFADDR, &ifr))
		ret = 1;

	close(s);
	return ret;
}

static int chk_wan_ipaddr(void)
{
	if (nvram_match("wan_ipaddr", "")) {
		if (wait_udhcpc < 12) {
			if ((wait_udhcpc == 1) || (wait_udhcpc == 6))
				eval("killall", "-SIGUSR1", "udhcpc");
			wait_udhcpc++;

			if (wait_udhcpc == 12)
				bridge_wan();
		}

		return 0;
	}

	return 1;
}

static int chk_ip_conflict(void)
{
	memset(wan_subnet_t, 0, sizeof(wan_subnet_t));
	strcpy(wan_subnet_t, nvram_safe_get("wan_subnet_t"));

	if (!strcmp(lan_subnet_t, wan_subnet_t)) {
		fprintf(stderr, "### IP conflict ###\n");

		if (wan_got_ip()) {
			Change_To_Redirect_rules();
			eval("ifconfig", wan_ifname, "0.0.0.0");
		}
		nvram_set("wan_state", "2");

		return 0;
	}

	return 1;
}

static int chk_wan_proto(void)
{
	if (!strcmp(wan_proto, "pptp") 
			|| !strcmp(wan_proto, "l2tp") 
			|| !strcmp(wan_proto, "pppoe")) {
		DIR *_dir;
		struct dirent *entry;
		int got_ppp_link = 0;

		if ((_dir = opendir("/tmp/ppp")) == NULL)
			disconn_case = CASE_PPPFAIL;

		while ((entry = readdir(_dir)) != NULL) {
			if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
				continue;
			if (strstr(entry->d_name, "link") != NULL) {
				got_ppp_link = 1;
				break;
			}
		}
		closedir(_dir);

		if (got_ppp_link == 0) {
			chk_ppp_status();
			disconn_case = CASE_PPPFAIL;
			return 0;
		}
	}
	else {
		if (nvram_match("wan_enable", "0")) {
			disconn_case = CASE_OTHERS;
			return 0;
		}
		else if ((query_udhcpc && !chk_wan_ipaddr()) 
				|| nvram_match("wan_state", "3")) {
			disconn_case = CASE_DHCPFAIL;
			return 0;
		}
		else if (!chk_ip_conflict()) {
			disconn_case = CASE_THESAMESUBNET;
			return 0;
		}
		else {
			/* Open socket to kernel */
			int s;
			if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
				disconn_case = CASE_OTHERS;
				return 0;
			}
			close(s);
		}
	}

	return 1;
}

static int check_wan_status(void)
{
	if (isFirstUse)
		return DISCONN;

	disconn_case = 0;
	switch (_sw_mode) {
		case 1:
			if (!GetLinkStatus(ESW_WAN_PORT))
				disconn_case = CASE_DISWAN;
			else if (!chk_wan_proto())
				;

			break;
		case 2:
			if (nvram_match("dnsqmode", "2"))
				disconn_case = CASE_DISPARENTAP;

			break;
		case 3:
			if (!GetLinkStatus(ESW_WAN_PORT))
				disconn_case = CASE_DISWAN;
			else if (nvram_match("dnsqmode", "2"))
				disconn_case = CASE_DHCPFAIL;

			break;
#ifdef SWMODE_ADAPTER_SUPPORT
		case 4:
			if (!chk_ea_connStatus())
				disconn_case = CASE_DISPARENTAP;

			break;
#endif
#ifdef SWMODE_HOTSPOT_SUPPORT
		case 5:
			if (nvram_match("sta_freq", ""))
				disconn_case = CASE_DISPARENTAP;
			else if (!chk_wan_proto())
				;

			break;
#endif
#ifdef USB_SUPPORT
		case 6:
//TBD.

			break;
#endif
		default:
			fprintf(stderr, "### %s: wrong sw_mode! ###\n", __FUNCTION__);
	}

	if (disconn_case != 0)
		return DISCONN;
	return CONNED;
}

static void record_conn_status(void)
{
	if (err_state == C2D) {
		if (disconn_case == CASE_DISWAN) {
			logmessage("WAN Connection", "The cable for Ethernet was not plugged in.");
		}
		else if (disconn_case == CASE_PPPFAIL) {
			FILE *fp = fopen("/tmp/wanstatus.log", "r");
			char log_info[64];

			if (fp == NULL) {
				logmessage("WAN Connection", "WAN was exceptionally disconnected.");
				return;
			}

			memset(log_info, 0, 64);
			fgets(log_info, 64, fp);
			fclose(fp);

			if (strstr(log_info, "Failed to authenticate ourselves to peer") != NULL)
				logmessage("WAN Connection", "PPPoE or PPTP authentification failed.");
			else
				logmessage("WAN Connection", "No response from the remote server.");
		}
		else if (disconn_case == CASE_DHCPFAIL) {
			if (!strcmp(wan_proto, "dhcp"))
				logmessage("WAN Connection", "ISP's DHCP did not function properly.");
			else
				logmessage("WAN Connection", "Detected that the WAN Connection Type was PPPoE. But the PPPoE Setting was not complete.");
		}
		else if (disconn_case == CASE_MISROUTE) {
			logmessage("WAN Connection", "The router's ip was the same as gateway's ip. It led to your packages couldn't dispatch to internet correctly.");
		}
		else if (disconn_case == CASE_THESAMESUBNET) {
			logmessage("WAN Connection", "The LAN's subnet may be the same with the WAN's subnet.");
		}
		else {	// disconn_case == CASE_OTHERS
			logmessage("WAN Connection", "WAN was exceptionally disconnected.");
		}
	}
	else if (err_state == D2C) {
		if (_sw_mode == 1 || _sw_mode == 3)
			logmessage("WAN Connection", "WAN was restored.");
	}
}

static void close_socket(int sockfd, char type)
{
	close(sockfd);
	FD_CLR(sockfd, &allset);
	client[fd_i].sfd = -1;
	client[fd_i].type = 0;
}

/********************************************************************************
 * run_http_serv
 */
static void send_page(int sfd, char *url)
{
	char buf[2*MAXLINE];
	time_t now;
	char timebuf[128];

	//fprintf(stderr, "%s: url=%s\n", __FUNCTION__, url);

	memset(buf, 0, sizeof(buf));
	now = time((time_t *)0);
	strftime(timebuf, sizeof(timebuf), RFC1123FMT, gmtime(&now));

	sprintf(buf, "HTTP/1.0 302 Moved Temporarily\r\nServer: wanduck\r\nDate: %s\r\n", timebuf);

	if (isFirstUse) /* QIS */
		sprintf(buf, "%sConnection: close\r\nLocation:http://%s/QIS_wizard.htm\r\nContent-Type: text/plain\r\n\r\n<html></html>\r\n", buf, default_domain_name);
	else if (err_state == C2D || err_state == DISCONN) /* error page */
		sprintf(buf, "%sConnection: close\r\nLocation:http://%s/error_page.htm?flag=%d\r\nContent-Type: text/plain\r\n\r\n<html></html>\r\n", buf, default_domain_name, disconn_case);

	write(sfd, buf, strlen(buf));
}

/* parse Connection of HTTP */
static void parse_connection(char *HTTP, char *conn)
{
	int i = 0, j = 0;
	char *pt;

	if ((pt = strstr(HTTP, "Connection: ")) != NULL) {
		pt += 12;
		for (i = 0; i < strlen(pt); i++) {
			if (pt[i] == '\r' || pt[i] == '\n') {
				conn[j] = '\0';
				break;
			}
			conn[j++] = pt[i];
		}
	}
	//fprintf(stderr, "%s: Connection=%s\n", __FUNCTION__, conn);
}

/* parse destination URL of HTTP */
static void parse_dest_url(char *HTTP, char *url)
{
	int i = 0, j = 0;
	char dest[256], host[64];
	char *pt;

	memset(dest, 0, sizeof(dest));
	memset(host, 0, sizeof(host));

	for (i = 0; i < strlen(HTTP); i++) {
		if (i >= sizeof(dest))
			break;

		if (HTTP[i] == ' ' || HTTP[i] == '?') {
			dest[j] = '\0';
			break;
		}

		dest[j++] = HTTP[i];
	}
	//fprintf(stderr, "%s: dest=%s\n", __FUNCTION__, dest);

	if ((pt = strstr(HTTP, "Host: ")) != NULL) {
		pt += 6;
		j = 0;
		for (i = 0; i < strlen(pt); i++) {
			if (i >= sizeof(host))
				break;

			if (pt[i] == '\r' || pt[i] == '\n') {
				host[j] = '\0';
				break;
			}

			host[j++] = pt[i];
		}
	}
	//fprintf(stderr, "%s: host=%s\n", __FUNCTION__, host);

	sprintf(url, "%s/%s", host, dest);
}

static void handle_http_req(int sfd, char *line)
{
	char conn[16], url[256];

	//fprintf(stderr, "%s: line=%s\n", __FUNCTION__, line);

	if (!strncmp(line, "GET /", 5)) {
		memset(conn, 0, sizeof(conn));
		parse_connection(line + 5, conn);
		if (strcasecmp(conn, "close")) {						/* ignore HTTP request of close connection */
			memset(url, 0, sizeof(url));
			parse_dest_url(line + 5, url);
			if (!strcmp(url, "www.apple.com/library/test/success.html")		/* ignore iOS detect internet */
					|| !strcmp(url, "www.msftncsi.com/ncsi.txt")		/* ignore Win8 detect internet */
					|| !strcmp(url, "clients3.google.com/generate_204")	/* ignore Android detect internet */
					|| !strcmp(url, "www.youtube.com/robots.txt")		/* ignore Zenfone detect internet */
			)
				;
			else
				send_page(sfd, url);
		}
	}
	close_socket(sfd, T_HTTP);
}

static void run_http_serv(int sockfd)
{
	ssize_t n;
	char line[MAXLINE];

	memset(line, 0, sizeof(line));

	if ((n = read(sockfd, line, MAXLINE)) == 0) {
		close_socket(sockfd, T_HTTP);
		return;
	}
	else if (n < 0) {
		perror("readline");
		return;
	}
	else {
		if (client[fd_i].type == T_HTTP)
			handle_http_req(sockfd, line);
		else
			close_socket(sockfd, T_HTTP);
	}
}
/********************************************************************************/

/********************************************************************************
 * run_dns_serv
 */
static void ques_str(char *buf)
{
	char *p;
	int i, ni;

	for (p = buf, i = 0, ni = 0; p[i]; p[i]=='.' ? p[i]=1, p+=i, i=0 : i++)
		if (p[i] == '.')
			p[0] = i - 1;
	p[0] = i - 1;       // in case xxx (no dot)
}

static void parse_req_queries(char *content, char *lp, int len, int *reply_size)
{
	int i, rn;

	rn = *(reply_size);
	for (i = 0; i < len; ++i) {
		content[rn+i] = lp[i];
		if (lp[i] == 0) {
			++i;
			break;
		}
	}

	if (i >= len)
		return;

	content[rn+i] = lp[i];
	content[rn+i+1] = lp[i+1];
	content[rn+i+2] = lp[i+2];
	content[rn+i+3] = lp[i+3];
	i += 4;

	*reply_size += i;
}

const char domain_name_str1[]="wwwasusnetworknet";
const char domain_name_str2[]="wwwasusroutercom";
const char domain_name_str3[]="repeatersasuscom";

static void lowerAndStripSym(char *pout, char *pin)
{
	int i, j;
	char inc;
	int maxlen = strlen(pin)<32 ? strlen(pin) : 31;

	for (i = 0, j = 0; i < maxlen; ++i) {
		inc = pin[i];
		if (isalnum(inc))
			pout[j++] = isupper(inc) ? tolower(inc) : inc;
	}
}

static int valid_domain_name(char *in_name)
{
	char buf[32];

	memset(buf, 0, sizeof(buf));
	lowerAndStripSym(buf, in_name);

	return (!strcmp(domain_name_str1, buf) || !strcmp(domain_name_str2, buf) || !strcmp(domain_name_str3, buf));
}

static void handle_dns_req(int sfd, char *line, int maxlen, struct sockaddr *pcliaddr, int clen)
{
	dns_query_packet d_req;
	dns_response_packet d_reply;
	int reply_size;
	char reply_content[MAXLINE];
	char qbuf[PATHLEN];

	reply_size = 0;
	memset(reply_content, 0, MAXLINE);
	memset(&d_req, 0, sizeof(d_req));
	memcpy(&d_req.header, line, sizeof(d_req.header));

	/* case of our own domain queries */
	memcpy(&d_req.queries, line+sizeof(d_req.header), sizeof(d_req.queries));
	qbuf[0] = 1;
	strcpy(qbuf+1, nvram_safe_get("hijdomain"));
	ques_str(qbuf);

	// header
	memcpy(&d_reply.header, &d_req.header, sizeof(dns_header));
	d_reply.header.flag_set.flag_num = htons(0x8580);
	//d_reply.header.flag_set.flag_num = htons(0x8180);
	d_reply.header.answer_rrs = htons(0x0001);
	memcpy(reply_content, &d_reply.header, sizeof(d_reply.header));
	reply_size += sizeof(d_reply.header);

	// Force to send answer response
	reply_content[5] = 1;	// Questions
	reply_content[7] = 1;	// Answer RRS
	reply_content[9] = 0;	// Authority RRS
	reply_content[11] = 0;	// Additional RRS

	// queries
	parse_req_queries(reply_content, line+sizeof(dns_header), maxlen-sizeof(dns_header), &reply_size);

	// answers
	d_reply.answers.name = htons(0xc00c);
	d_reply.answers.type = htons(0x0001);
	d_reply.answers.ip_class = htons(0x0001);
	//d_reply.answers.ttl = htonl(0x00000001);
	d_reply.answers.ttl = htonl(0x00000000);
	d_reply.answers.data_len = htons(0x0004);
	if ((!strcmp(d_req.queries.name, qbuf)) || valid_domain_name(d_req.queries.name)) {
		fprintf(stderr, "### query own domain ###\n");
		d_reply.answers.addr = htonl(ntohl(inet_addr(nvram_safe_get("lan_ipaddr"))));
	}
	else
		d_reply.answers.addr = htonl(ntohl(inet_addr(nvram_safe_get("reply_fake_ip"))));

	memcpy(reply_content+reply_size, &d_reply.answers, sizeof(d_reply.answers));
	reply_size += sizeof(d_reply.answers);

	sendto(sfd, reply_content, reply_size, 0, pcliaddr, clen);
}

static void run_dns_serv(int sockfd)
{
	int n;
	char line[MAXLINE];
	struct sockaddr_in cliaddr;
	int clilen = sizeof(cliaddr);

	memset(line, 0, MAXLINE);
	memset(&cliaddr, 0, clilen);

	if ((n = recvfrom(sockfd, line, MAXLINE, 0, (struct sockaddr *)&cliaddr, &clilen)) == 0)
		return;
	else if (n < 0) {
		perror("readline");
		return;
	}
	else
		handle_dns_req(sockfd, line, n, (struct sockaddr *)&cliaddr, clilen);
}
/********************************************************************************/

static int passivesock(char *service, int protocol_num, int qlen)
{
	struct sockaddr_in sin;
	int s, type, bReuseaddr = 1;

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;

	// map service name to port number
	if ((sin.sin_port = htons((u_short)atoi(service))) == 0) {
		perror("cannot get service entry");
		return -1;
	}

	if (protocol_num == IPPROTO_UDP)
		type = SOCK_DGRAM;
	else
		type = SOCK_STREAM;

	if ((s = socket(PF_INET, type, protocol_num)) < 0) {
		perror("cannot create socket");
		return -1;
	}

	if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&bReuseaddr, sizeof(bReuseaddr)) < 0) {
		perror("cannot set socket's option: SO_REUSEADDR");
		close(s);
		return -1;
	}

	if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
		perror("cannot bind port");
		close(s);
		return -1;
	}

	if (type == SOCK_STREAM && listen(s, qlen) < 0) {
		perror("cannot listen to port");
		close(s);
		return -1;
	}

	return s;
}

static int build_socket(char *http_port, char *dns_port, int *hd, int *dd)
{
	if ((*hd = passivesock(http_port, IPPROTO_TCP, 10)) < 0) {
		fprintf(stderr, "Fail to socket for httpd port: %s.\n", http_port);
		return -1;
	}

	if ((*dd = passivesock(dns_port, IPPROTO_UDP, 10)) < 0) {
		fprintf(stderr, "Fail to socket for DNS port: %s.\n", dns_port);
		return -1;
	}

	return 0;
}

static void sig_handle(int sig)
{
	if (sig == SIGUSR2) {
		if (err_state == CONNED)
			redirect_func(0);
		else if (err_state == DISCONN)
			redirect_func(1);
	}
	else if (sig == SIGUSR1) {
		memset(wan_ifname, 0, sizeof(wan_ifname));
		strcpy(wan_ifname, nvram_safe_get("wan_ifname"));
		memset(wan_proto, 0, sizeof(wan_proto));
		strcpy(wan_proto, nvram_safe_get("wan_proto"));
		memset(default_domain_name, 0, sizeof(default_domain_name));
		strcpy(default_domain_name, nvram_safe_get("hijdomain"));
		memset(lan_subnet_t, 0, sizeof(lan_subnet_t));
		strcpy(lan_subnet_t, nvram_safe_get("lan_subnet_t"));

		_sw_mode = atoi(nvram_safe_get("sw_mode"));
		if (((_sw_mode == 1) && !strcmp(wan_proto, "dhcp")) 
				|| _sw_mode == 5)
			query_udhcpc = 1;

		if (nvram_match("x_Setting", "1"))
			isFirstUse = 0;
		else
			isFirstUse = 1;
	}
	else if (sig == SIGTERM) {
		int i;

		FD_ZERO(&allset);
		close(http_sock);
		close(dns_sock);
		for (i = 0; i < maxfd; ++i)
			close(i);

		sleep(1);

		if (err_state == DISCONN) {
			fprintf(stderr, "%s: ### Disable direct rule ###\n", __FUNCTION__);
			redirect_func(0);
		}
		eval("rm", "-f", "/var/run/wanduck.pid");
		fprintf(stderr, "%s: ### wanduck safe exit ###\n", __FUNCTION__);

		exit(0);
	}
}

int wanduck_main(int argc, char **argv)
{
	char *http_servport, *dns_servport;
	socklen_t clilen;
	struct sockaddr_in cliaddr;
	struct timeval tval;
	int nready, maxi, sockfd, conn_state;

	signal(SIGHUP, SIG_IGN);
	signal(SIGTERM, sig_handle);
	signal(SIGUSR1, sig_handle);
	signal(SIGUSR2, sig_handle);

	if (argc < 3) {
		http_servport = DFL_HTTP_SERV_PORT;
		dns_servport = DFL_DNS_SERV_PORT;
	}
	else {
		if (atoi(argv[1]) <= 0)
			http_servport = DFL_HTTP_SERV_PORT;
		else
			http_servport = argv[1];

		if (atoi(argv[2]) <= 0)
			dns_servport = DFL_DNS_SERV_PORT;
		else
			dns_servport = argv[2];
	}

	if (build_socket(http_servport, dns_servport, &http_sock, &dns_sock) < 0) {
		fprintf(stderr, "*** Fail to build socket! ***\n");
		exit(0);
	}

	FILE *fp = fopen("/var/run/wanduck.pid", "w");
	if (fp != NULL) {
		fprintf(fp, "%d", getpid());
		fclose(fp);
	}

	maxfd = (http_sock > dns_sock) ? http_sock : dns_sock;
	maxi = -1;

	FD_ZERO(&allset);
	FD_SET(http_sock, &allset);
	FD_SET(dns_sock, &allset);

	for (fd_i = 0; fd_i < MAX_USER; ++fd_i) {
		client[fd_i].sfd = -1;
		client[fd_i].type = 0;
	}

	disconn_case = 0;
	err_state = CONNED;//fake
	clilen = sizeof(cliaddr);

	sig_handle(SIGUSR1);

	for (;;) {
		rset = allset;
		tval.tv_sec = 1;
		tval.tv_usec = 0;

		conn_state = check_wan_status();
		if (conn_state == CONNED) {
			if (err_state == DISCONN)
				err_state = D2C;
		}
		else if (conn_state == DISCONN) {
			if (err_state == CONNED)
				err_state = C2D;
		}
		record_conn_status();

		if (err_state == C2D) {
			fprintf(stderr, "### Enable direct rule (C2D) ###\n");

			err_state = DISCONN;
			redirect_func(1);
			unbridge_wan();

			if (nvram_match("wan_state", "1"))
				nvram_set("wan_state", "0");
		}
		else if (err_state == D2C) {
			fprintf(stderr, "### Disable direct rule (D2C) ###\n");

			err_state = CONNED;
			redirect_func(0);

			nvram_set("wan_state", "1");
		}

		if ((nready = select(maxfd+1, &rset, NULL, NULL, &tval)) <= 0)
			continue;

		if (FD_ISSET(dns_sock, &rset)) {
			//printf("# run fake dns service\n");
			run_dns_serv(dns_sock);
			if (--nready <= 0)
				continue;
		}
		else if (FD_ISSET(http_sock, &rset)) {
			//printf("# run fake httpd service\n");
			if ((connfd = accept(http_sock, (struct sockaddr *)&cliaddr, &clilen)) <= 0) {
				perror("http accept");
				continue;
			}
			cur_sockfd = connfd;

			for (fd_i = 0; fd_i < MAX_USER; ++fd_i) {
				if (client[fd_i].sfd < 0) {
					client[fd_i].sfd = cur_sockfd;
					client[fd_i].type = T_HTTP;
					break;
				}
			}

			if (fd_i == MAX_USER) {
				fprintf(stderr, "wanduck servs full\n");
				close(cur_sockfd);
				continue;
			}

			FD_SET(cur_sockfd, &allset);
			if (cur_sockfd > maxfd)
				maxfd = cur_sockfd;
			if (fd_i > maxi)
				maxi = fd_i;

			if (--nready <= 0)
				continue;	// no more readable descriptors
		}

		// polling
		for (fd_i = 0; fd_i <= maxi; ++fd_i) {
			if ((sockfd = client[fd_i].sfd) < 0)
				continue;

			if (FD_ISSET(sockfd, &rset)) {
				int nread;

				ioctl(sockfd, FIONREAD, &nread);
				if (nread == 0) {
					close_socket(sockfd, T_HTTP);
					continue;
				}

				cur_sockfd = sockfd;
				run_http_serv(sockfd);

				if (--nready <= 0)
					break;
			}
		}
	}

	fprintf(stderr, "wanduck exit error\n");
	exit(1);
}
