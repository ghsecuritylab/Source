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
/* milli_httpd - pretty small HTTP server
** A combination of
** micro_httpd - really small HTTP server
** and
** mini_httpd - small HTTP server
**
** Copyright 1999,2000 by Jef Poskanzer <jef@acme.com>.
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
** FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
** DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
** OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
** HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
** LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
** OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
** SUCH DAMAGE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <assert.h>
#include <sys/ioctl.h>
#include <sys/sysinfo.h>
#include <ctype.h>
#include <httpd.h>
#include <arpa/inet.h>  //2008.08 magic
#include <error.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <shared.h>
#include <nvram/bcmnvram.h>
#include <netinet/in.h>

#define ETCPHYRD        14
#define SIOCGETCPHYRD   0x89FE

#ifdef RTCONFIG_HTTPS
#include <syslog.h>
#include <mssl.h>
#include <shutils.h>
#define SERVER_PORT_SSL 443
#endif

/* A multi-family sockaddr. */
typedef union {
	struct sockaddr sa;
	struct sockaddr_in sa_in;
	} usockaddr;

#include "queue.h"
#define MAX_CONN_ACCEPT 64
#define MAX_CONN_TIMEOUT 60

typedef struct conn_item {
	TAILQ_ENTRY(conn_item) entry;
	int fd;
	usockaddr usa;
} conn_item_t;

typedef struct conn_list {
       TAILQ_HEAD(, conn_item) head;
       int count;
} conn_list_t;

/* Globals. */
static FILE *conn_fp;
static char auth_userid[AUTH_MAX];
static char auth_passwd[AUTH_MAX];
static char auth_realm[AUTH_MAX];
char referer_host[64];
char gen_token[32]={0};

asus_token_t *head;
asus_token_t *curr;

#ifdef TRANSLATE_ON_FLY
char Accept_Language[16];
#endif //TRANSLATE_ON_FLY

int _x_Setting = 0;
char dut_domain_name[32];
char Last_Req_Host[64];

int change_account = 0;
int reget_account = 0;
char url[128];
int http_port = SERVER_PORT;

/* Added by Joey for handle one people at the same time */
unsigned int login_ip = 0;	// the logined ip
time_t login_timestamp = 0;	// the timestamp of the logined ip
time_t login_timestamp_tmp = 0;	// the timestamp of the current session.
time_t last_login_timestamp = 0;// the timestamp of the current session.
unsigned int login_ip_tmp = 0;	// the ip of the current session.
unsigned int login_try = 0;
unsigned int last_login_ip = 0;	// the last logined ip 2008.08 magic
/* limit login IP addr; 2015.03 Gabriel */
unsigned int access_ip[4];
const int int_1 = 1;
unsigned int MAX_login;
int lock_flag = 0;

long uptime(void)
{
	struct sysinfo info;
	sysinfo(&info);

	return info.uptime;
}

static void http_login_cache(usockaddr *u)
{
	login_ip_tmp = (unsigned int)(u->sa_in.sin_addr.s_addr);
}

static void http_login(unsigned int ip)
{
	char tmp[32];

	if (http_port != SERVER_PORT || ip == 0x100007f)
		return;

	login_ip = ip;
	last_login_ip = 0;
	login_timestamp = time((time_t *)0);

	memset(tmp, 0, 32);
	sprintf(tmp, "%u", login_ip);
	nvram_set("login_ip", tmp);

	memset(tmp, 0, 32);
	sprintf(tmp, "%lu", login_timestamp);
	nvram_set("login_timestamp", tmp);
}

static void http_logout(unsigned int ip, char *cookies)
{
	if (ip == login_ip) {
		last_login_ip = login_ip;
		login_ip = 0;
		login_timestamp = 0;

		nvram_set("login_ip", "");
		nvram_set("login_timestamp", "");
		memset(referer_host, 0, sizeof(referer_host));
		delete_logout_from_list(cookies);

		if (change_account == 1) {
			change_account = 0;
			reget_account = 1;
		}
	}
}

// 0: can not login, 1: can login, 2: loginer, 3: not loginer.
static int http_login_check(void)
{
	if (http_port != SERVER_PORT || login_ip_tmp == 0x100007f)
		return 0;
	else if (login_ip == 0)
		return 1;
	else if (login_ip == login_ip_tmp)
		return 2;
	return 3;
}

static void http_login_timeout(unsigned int ip, char *cookies)
{
	time_t now;

	time(&now);
	if (login_ip != 0 
			&& login_ip != ip 
			&& (unsigned long)(now-login_timestamp) > 60) //one minitues
		http_logout(login_ip, cookies);
}


static int initialize_listen_socket(usockaddr* usaP)
{
	int listen_fd;
	int i = 1;

	memset(usaP, 0, sizeof(usockaddr));
	usaP->sa.sa_family = AF_INET;
	usaP->sa_in.sin_addr.s_addr = htonl(INADDR_ANY);
	usaP->sa_in.sin_port = htons(http_port);

	listen_fd = socket(usaP->sa.sa_family, SOCK_STREAM, 0);
	if (listen_fd < 0) {
		perror("socket");
		return -1;
	}
	fcntl(listen_fd, F_SETFD, 1);
	if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, (char*) &i, sizeof(i)) < 0) {
		close(listen_fd);
		perror("setsockopt");
		return -1;
	}
	if (bind(listen_fd, &usaP->sa, sizeof(struct sockaddr_in)) < 0) {
		close(listen_fd);
		perror("bind");
		return -1;
	}
	if (listen(listen_fd, 1024) < 0) {
		close(listen_fd);
		perror("listen");
		return -1;
	}
	return listen_fd;
}

static void send_headers(int status, char* title, char* extra_header, char* mime_type)
{
	time_t now;
	char timebuf[100];

	fprintf(conn_fp, "%s %d %s\r\n", PROTOCOL, status, title);
	fprintf(conn_fp, "Server: %s\r\n", SERVER_NAME);
	now = time((time_t *)0);
	strftime(timebuf, sizeof(timebuf), RFC1123FMT, gmtime(&now));
	fprintf(conn_fp, "Date: %s\r\n", timebuf);
	if (extra_header != (char *)0)
		fprintf(conn_fp, "%s\r\n", extra_header);
	if (mime_type != (char *)0)
		fprintf(conn_fp, "Content-Type: %s\r\n", mime_type);
	fprintf(conn_fp, "Connection: close\r\n");
	fprintf(conn_fp, "\r\n");
}

static void send_page(int status, char *title, char *extra_header, char *text)
{
	send_headers(status, title, extra_header, "text/html");
	fprintf(conn_fp, "<HTML><HEAD>");
	fprintf(conn_fp, "%s\n", text);
	fprintf(conn_fp, "</HEAD></HTML>\n");
	fflush(conn_fp);
}

void send_login_page(int error_status, char *url, char *file, int lock_time)
{
	char inviteCode[256] = {0};
	char url_tmp[64] = {0};

	if (url == NULL)
		strncpy(url_tmp, "index.asp", sizeof(url_tmp));
	else
		strncpy(url_tmp, url, sizeof(url_tmp));

	snprintf(inviteCode, sizeof(inviteCode), "<script>top.location.href='/Main_Login.asp?error_status=%d&page=%s&lock_time=%d';</script>", error_status, url_tmp, lock_time);
	send_page( 200, "OK", (char*) 0, inviteCode);
}

void __send_login_page(int error_status, char *url, int lock_time)
{
	login_try++;
	last_login_timestamp = login_timestamp_tmp;
	send_login_page(error_status, url, NULL, lock_time);
}

static int referer_check(char *referer)
{
	char *auth_referer = NULL;
	char *cp1 = NULL, *cp2 = NULL, *location_cp = NULL, *location_cp1 = NULL;

	if (!referer) {
		send_login_page(NOREFERER, NULL, NULL, 0);
		return NOREFERER;
	}
	else {
		if (strstr(referer,"\r") != (char*) 0)
			location_cp1 = strtok(referer, "\r");
		else
			location_cp1 = referer;

		location_cp = strstr(location_cp1, "//");
		if (location_cp != (char*) 0) {
			cp1 = &location_cp[2];
			if (strstr(cp1, "/") != (char*) 0) {
				cp2 = strtok(cp1, "/");
				auth_referer = cp2;
			}
			else
				auth_referer = cp1;
		}
		else
			auth_referer = location_cp1;
	}
	if (referer_host[0] == 0) {
		send_login_page(WEB_NOREFERER, NULL, NULL, 0);
		return WEB_NOREFERER;
	}
	if (strncmp(dut_domain_name, auth_referer, strlen(dut_domain_name)) == 0)
		strcpy(auth_referer, nvram_safe_get("lan_ipaddr"));
	/* form based referer info? */
	if ((strlen(auth_referer) == strlen(referer_host)) && strncmp(auth_referer, referer_host, strlen(referer_host)) == 0) {
		//fprintf(stderr, "asus token referer_check: the right user and password\n");
		return 0;
	}
	else {
		//fprintf(stderr, "asus token referer_check: the wrong user and password\n");
		send_login_page(REFERERFAIL, NULL, NULL, 0);
		return REFERERFAIL;
	}
}

static void send_error(int status, char *title, char *extra_header, char *text)
{
	send_headers(status, title, extra_header, "text/html");
	fprintf(conn_fp, "<HTML><HEAD><TITLE>%d %s</TITLE></HEAD>\n<BODY BGCOLOR=\"#cc9999\"><H4>%d %s</H4>\n", status, title, status, title);
	fprintf(conn_fp, "%s\n", text);
	fprintf(conn_fp, "</BODY></HTML>\n");
	fflush(conn_fp);
}

asus_token_t* search_token_in_list(char *token, asus_token_t **prev)
{
	asus_token_t *ptr = head;
	asus_token_t *tmp = NULL;
	int found = 0;
	char *cp = NULL;

	while (ptr != NULL) {
		if (!strncmp(token, ptr->token, 32)) {
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

int delete_logout_from_list(char *cookies)
{
	asus_token_t *prev = NULL;
	asus_token_t *del = NULL;
	char asustoken[32];
	char *cp = NULL, *location_cp;

	memset(asustoken, 0, sizeof(asustoken));

	if (!cookies || !_x_Setting)
		return 0;
	else {
		if (strncmp(cookies, "cgi_logout", 10) == 0)
			strncpy(asustoken, cookies, sizeof(asustoken));
		else {
			location_cp = strstr(cookies, "asus_token");
			if (location_cp != NULL) {
				cp = &location_cp[11];
				cp += strspn(cp, " \t");
				snprintf(asustoken, sizeof(asustoken), "%s", cp);
			}
			else
				return 0;
		}
	}

	del = search_token_in_list(asustoken,&prev);
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

#define	HEAD_HTTP_LOGIN	"HTTP login"	// copy from push_log/push_log.h

static int auth_check(char *dirname, char *authorization, char *url, char *file, char *cookies)
{
	struct in_addr temp_ip_addr;
	char *temp_ip_str;
	time_t dt;
	char asustoken[32];
	char *cp = NULL, *location_cp;

	memset(asustoken,0,sizeof(asustoken));

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
			logmessage(HEAD_HTTP_LOGIN, "Detect abnormal logins at %d times. The newest one was from %s.", login_try, temp_ip_str);

		send_login_page(LOGINLOCK, url, NULL, dt);
		return LOGINLOCK;
	}

	/* Is this directory unprotected? */
	if (!strlen(auth_passwd)) {
		/* Yes, let the request go through. */
		return 0;
	}

	if (!cookies) {
		send_login_page(NOTOKEN, url, file, 0);
		return NOTOKEN;
	}
	else {
		location_cp = strstr(cookies, "asus_token");
		if (location_cp != NULL) {
			cp = &location_cp[11];
			cp += strspn(cp, " \t");
			snprintf(asustoken, sizeof(asustoken), "%s", cp);
		}
		else {
			send_login_page(NOTOKEN, url, file, 0);
			return NOTOKEN;
		}
	}
	/* form based authorization info? */

	if (search_token_in_list(asustoken, NULL) != NULL) {
		//fprintf(stderr, "asus token auth_check: the right user and password\n");
		login_try = 0;
		last_login_timestamp = 0;
		lock_flag = 0;
		return 0;
	}
	else {
		//fprintf(stderr, "asus token auth_check: the wrong user and password\n");
		send_login_page(AUTHFAIL, url, file, 0);
		return AUTHFAIL;
	}

	send_login_page(AUTHFAIL, url, file, 0);
	return AUTHFAIL;
}


/* Simple shell-style filename matcher.  Only does ? * and **, and multiple
 ** patterns separated by |.  Returns 1 or 0.
 */
static int match_one(const char *pattern, int patternlen, const char *string)
{
	const char *p;

	for (p = pattern; p - pattern < patternlen; ++p, ++string) {
		if (*p == '?' && *string != '\0')
			continue;
		if (*p == '*') {
			int i, pl;

			++p;
			if (*p == '*') {
				/* Double-wildcard matches anything. */
				++p;
				i = strlen(string);
			}
			else
				/* Single-wildcard matches anything but slash. */
				i = strcspn(string, "/");
			pl = patternlen - (p - pattern);
			for (; i >= 0; --i)
				if (match_one(p, pl, &(string[i])))
					return 1;
			return 0;
		}
		if (*p != *string)
			return 0;
	}
	if (*string == '\0')
		return 1;
	return 0;
}

static int match(const char *pattern, const char *string)
{
	const char *or;

	for (;;) {
		or = strchr(pattern, '|');
		if (or == (char *)0)
			return match_one(pattern, strlen(pattern), string);
		if (match_one(pattern, or - pattern, string))
			return 1;
		pattern = or + 1;
	}
}

char *generate_token(void)
{
	int a = 0, b = 0, c = 0, d = 0;
	//char create_token[32] = {0};

	memset(gen_token, 0, sizeof(gen_token));
	srand(time(NULL));
	a = rand();
	b = rand();
	c = rand();
	d = rand();
	snprintf(gen_token, sizeof(gen_token), "%d%d%d%d", a, b, c, d);

	return gen_token;
}

/*static void send_token_headers(int status, char *title, char *extra_header, char *mime_type)
{
	time_t now;
	char timebuf[100];
	char asus_token[32] = {0};

	memset(asus_token, 0, sizeof(asus_token));

	if (!_x_Setting && strcmp( gen_token, "") != 0)
		strncpy(asus_token, gen_token, sizeof(asus_token));
	else {
		strncpy(asus_token, generate_token(), sizeof(asus_token));
		add_asus_token(asus_token);
	}

	fprintf(conn_fp, "%s %d %s\r\n", PROTOCOL, status, title);
	fprintf(conn_fp, "Server: %s\r\n", SERVER_NAME);
	now = time((time_t*) 0);
	strftime(timebuf, sizeof(timebuf), RFC1123FMT, gmtime(&now));
	fprintf(conn_fp, "Date: %s\r\n", timebuf);
	if (extra_header != (char*) 0)
		fprintf(conn_fp, "%s\r\n", extra_header);
	if (mime_type != (char*) 0)
		fprintf(conn_fp, "Content-Type: %s\r\n", mime_type);

	fprintf(conn_fp, "Set-Cookie: asus_token=%s; HttpOnly;\r\n",asus_token);
	fprintf(conn_fp, "Connection: close\r\n");
	fprintf(conn_fp, "\r\n");
}*/

static void handle_request(void)
{
	char line[10000], *cur;
	char *method, *path, *protocol, *authorization, *boundary, *alang, *cookies, *referer;
	char *cp;
	char *file;
	int len;
	struct mime_handler *handler;
	struct mime_referer *doreferer;
	int do_referer;
	int cl = 0, flags;
	int auth_result = 1;
	int referer_result = 1;

	/* Initialize the request variables. */
	authorization = boundary = cookies = referer = NULL;
	bzero(line, sizeof(line));

	/* Parse the first line of the request. */
	if (fgets(line, sizeof(line), conn_fp) == (char *)0) {
		send_error(400, "Bad Request", (char*) 0, "No request found.");
		return;
	}

	method = path = line;
	strsep(&path, " ");
	while (path && *path == ' ') path++;
	protocol = path;
	strsep(&protocol, " ");
	while (protocol && *protocol == ' ') protocol++;
	cp = protocol;
	strsep(&cp, " ");
	if (!method || !path || !protocol) {
		send_error(400, "Bad Request", (char*) 0, "Can't parse request.");
		return;
	}
	cur = protocol + strlen(protocol) + 1;

#ifdef TRANSLATE_ON_FLY
	memset(Accept_Language, 0, sizeof(Accept_Language));
#endif

	Last_Req_Host[0] = '\0';

	/* Parse the rest of the request headers. */
	while (fgets(cur, line + sizeof(line) - cur, conn_fp) != (char *)0) {
		if (strcmp(cur, "\n") == 0 || strcmp(cur, "\r\n") == 0)
			break;
#ifdef TRANSLATE_ON_FLY
		else if (strncasecmp(cur, "Accept-Language:", 16) == 0) {
			char *p;
			struct language_table *pLang;
			char lang_buf[256];

			memset(lang_buf, 0, sizeof(lang_buf));
			alang = &cur[16];
			strcpy(lang_buf, alang);
			p = lang_buf;
			while (p != NULL) {
				p = strtok (p, "\r\n ,;");
				if (p == NULL)
					break;
				int i, len = strlen(p);

				for (i = 0; i < len; ++i) {
					if (isupper(p[i]))
						p[i] = tolower(p[i]);
				}

				for (pLang = language_tables; pLang->Lang != NULL; ++pLang) {
					if (strcasecmp(p, pLang->Lang) == 0) {
						snprintf(Accept_Language, sizeof(Accept_Language), "%s", pLang->Target_Lang);
						if (_x_Setting == 0)
							nvram_set("preferred_lang", Accept_Language);
						break;
					}
				}

				if (Accept_Language[0] != 0)
					break;
				p += strlen(p) + 1;
			}

			if (Accept_Language[0] == 0) {
				// If all language setting of user's browser are not supported, use English.
				strcpy(Accept_Language, "EN");

				if(_x_Setting == 0)
					nvram_set("preferred_lang", "EN");
			}
		}
#endif
		else if (strncasecmp(cur, "Cookie:", 7) == 0) {
			cp = &cur[7];
			cp += strspn(cp, " \t");
			cookies = cp;
			cur = cp + strlen(cp) + 1;
		}
		else if (strncasecmp(cur, "Referer:", 8) == 0) {
			cp = &cur[8];
			cp += strspn(cp, " \t");
			referer = cp;
			cur = cp + strlen(cp) + 1;
		}
		else if (strncasecmp(cur, "Host:", 5) == 0) {
			int i;

			cp = &cur[5];
			cp += strspn(cp, " \t");
			strncpy(Last_Req_Host, cp, sizeof(Last_Req_Host));
			for (i = strlen(Last_Req_Host)-1; i >= 0; i--) {
				if ((Last_Req_Host[i] == '\r') || (Last_Req_Host[i] == '\n'))
					Last_Req_Host[i] = '\0';
				else
					break;
			}
			cur = cp + strlen(cp) + 1;
		}
		else if (strncasecmp(cur, "Authorization:", 14) == 0) {
			cp = &cur[14];
			cp += strspn(cp, " \t");
			authorization = cp;
			cur = cp + strlen(cp) + 1;
		}
		else if (strncasecmp(cur, "Content-Length:", 15) == 0) {
			cp = &cur[15];
			cp += strspn(cp, " \t");
			cl = strtoul(cp, NULL, 0);
		}
		else if ((cp = strstr(cur, "boundary="))) {
			boundary = &cp[9];
			for (cp = cp + 9; *cp && *cp != '\r' && *cp != '\n'; cp++)
				;
			*cp = '\0';
			cur = ++cp;
		}
	}

	if (strcasecmp(method, "get") != 0 && strcasecmp(method, "post") != 0) {
		send_error(501, "Not Implemented", (char *)0, "That method is not implemented.");
		return;
	}
	if (path[0] != '/') {
		send_error(400, "Bad Request", (char *)0, "Bad filename.");
		return;
	}
	file = &(path[1]);
	len = strlen( file );
	if (file[0] == '/' 
			|| strcmp(file, "..") == 0 
			|| strncmp(file, "../", 3) == 0 
			|| strstr(file, "/../") != (char *)0 
			|| strcmp(&(file[len-3]), "/..") == 0) {
		send_error(400, "Bad Request", (char *)0, "Illegal filename.");
		return;
	}

	if (file[0] == '\0' || file[len-1] == '/') {
		if (_x_Setting)
			file = "index.asp";
		else
			file = "QIS_wizard.htm";
	}

	char *query;
	int file_len;

	memset(url, 0, 128);
	if ((query = index(file, '?')) != NULL) {
		file_len = strlen(file) - strlen(query);
		strncpy(url, file, file_len);
	}
	else
		strcpy(url, file);

	http_login_timeout(login_ip_tmp, cookies);

	if (http_port == SERVER_PORT 
			&& http_login_check() == 3 
			&& (strstr(url, ".asp") != NULL 
				|| strstr(url, "QIS_") != NULL)) {
		file = "Nologin.asp";
		memset(url, 0, 128);
		strcpy(url, file);
	}

	do_referer = 0;

	// check doreferer first
	for (doreferer = &mime_referers[0]; doreferer->pattern; doreferer++) {
		if (match(doreferer->pattern, url)) {
			do_referer = doreferer->flag;
			break;
		}
	}

	for (handler = &mime_handlers[0]; handler->pattern; handler++) {
		if (match(handler->pattern, url)) {
			//fprintf(stderr, "%s: URL[%s] match pattern[%s]\n", __FUNCTION__, url, handler->pattern);

			if (strcmp(url, "ajax_detect_dhcp_ip.asp") == 0)
				nvram_set("qis_state", "1"); // check device get new IP
			else if (strcmp(url, "ajax_detect_internet.asp") == 0)
				nvram_set("qis_state", "2"); // check device get gateway
			else if (strcmp(url, "ajax_report_error.asp") == 0)
				nvram_set("qis_state", "3"); // apcli connection fail
			else if (strcmp(url, "qis/QIS_survey.htm") == 0)
				nvram_set("qis_apply", "0"); // force clear

			if (handler->auth) {
				if ((strstr(url, "QIS_") || strcmp(url, "start_apply.htm") == 0) && !_x_Setting)
					;
				else {
					if (do_referer & CHECK_REFERER) {
						referer_result = referer_check(referer);
						if (referer_result != 0) {
							if (strcasecmp(method, "post") == 0) {
								if (handler->input)
									handler->input(file, conn_fp, cl, boundary);
								send_login_page(referer_result, NULL, NULL, 0);
							}
							//http_logout(login_ip_tmp, cookies);
							return;
						}
					}
					handler->auth(auth_userid, auth_passwd, auth_realm);
					auth_result = auth_check(auth_realm, authorization, url, file, cookies);
					if (auth_result != 0) {
						if (strcasecmp(method, "post") == 0) {
							if (handler->input)
								handler->input(file, conn_fp, cl, boundary);
							send_login_page(auth_result, NULL, NULL, 0);
						}
						//http_logout(login_ip_tmp, cookies);
						return;
					}
				}
				http_login(login_ip_tmp);
			}

			if (!strcmp(file, "Logout.asp")) {
				http_logout(login_ip_tmp, cookies);
				send_login_page(ISLOGOUT, NULL, NULL, 0);
				return;
			}
			if (strcasecmp(method, "post") == 0 && !handler->input) {
				send_error(501, "Not Implemented", NULL, "That method is not implemented.");
				return;
			}

			if (handler->input) {
				handler->input(file, conn_fp, cl, boundary);
#if defined(linux)
				if ((flags = fcntl(fileno(conn_fp), F_GETFL)) != -1 
						&& fcntl(fileno(conn_fp), F_SETFL, flags | O_NONBLOCK) != -1) {
					/* Read up to two more characters */
					if (fgetc(conn_fp) != EOF)
						fgetc(conn_fp);

					fcntl(fileno(conn_fp), F_SETFL, flags);
				}
#endif
			}

			if (strncmp(url, "login.cgi", strlen(url)) != 0)
				send_headers( 200, "Ok", handler->extra_header, handler->mime_type);

			if (handler->output)
				handler->output(file, conn_fp);

			break;
		}
	}

	if (!handler->pattern)
		send_error(404, "Not Found", (char *)0, "File not found.");
}

#ifdef TRANSLATE_ON_FLY
static void release_dictionary(pkw_t pkw)
{
	if (pkw == NULL)
		return;

	pkw->len = pkw->tlen = 0;

	if (pkw->idx != NULL) {
		free(pkw->idx);
		pkw->idx = NULL;
	}

	if (pkw->buf != NULL) {
		free(pkw->buf);
		pkw->buf = NULL;
	}
}

int load_dictionary(char *lang, pkw_t pkw)
{
	char dfn[16];
	char *buf_head, *buf_head2;
	char *p, *q, *q2;
	char *pid, *model;
	int f_len, b_len, pid_len, model_len, last_one;
	FILE *dfp = NULL;
	int dict_size = 0;
	struct timeval tv1, tv2;
	const char *eng_dict = "EN.dict";
	static char loaded_dict[12] = {'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'};

	gettimeofday(&tv1, NULL);
	if (lang == NULL || (lang != NULL && strlen (lang) == 0))
		// if "lang" is invalid, use English as default
		snprintf(dfn, sizeof(dfn), eng_dict);
	else
		snprintf(dfn, sizeof(dfn), "%s.dict", lang);

	if (strcmp(dfn, loaded_dict) == 0)
		return 1;
	release_dictionary(pkw);

	do {
		dfp = fopen(dfn, "r");
		if (dfp != NULL) {
			snprintf(loaded_dict, sizeof(loaded_dict), "%s", dfn);
			break;
		}

		if (dfp == NULL && strcmp(dfn, eng_dict) == 0)
			return 0;
		else
			// If we can't open specified language file, use English as default
			snprintf(dfn, sizeof(dfn), eng_dict);
	} while (1);

	memset(pkw, 0, sizeof(kw_t));
	fseek(dfp, 0L, SEEK_END);
	f_len = ftell(dfp);
	dict_size = f_len + 128;
	REALLOC_VECTOR(pkw->idx, pkw->len, pkw->tlen, sizeof(unsigned char *));
	buf_head = q = malloc(dict_size);
	fseek(dfp, 0L, SEEK_SET);
	b_len = fread(q, 1, f_len, dfp);
	if (b_len != f_len) {
		printf("ERROR!!, read file [%s] fail!!\n", dfn);
		q[0] = '\0';
	}
	q[f_len] = '\0';
	fclose(dfp);
	///// replace productid to fac_model_name
	pid = nvram_get("productid");
	model = nvram_get("fac_model_name");
	pid_len = strlen(pid);
	model_len = strlen(model);
	if (strcmp(pid, model)) {
		buf_head2 = q2 = malloc(f_len * 2 + 128);
		q = buf_head;
		while (p = strstr(q, pid)) {
			b_len = p - q;
			memcpy(q2, q, b_len);
			q2 += b_len;
			memcpy(q2, model, model_len);
			q2 += model_len;
			q = p + pid_len;
		}
		b_len = f_len - (q - buf_head);
		memcpy(q2, q, b_len + 1);
		free(buf_head);
		q2 += b_len;
		dict_size = q2-buf_head2 + 128;
		buf_head2 = (char *)realloc(buf_head2, dict_size);
		pkw->buf = q = buf_head2;
	} else
		pkw->buf = q = buf_head;
	/////
	last_one = 0;
	while ((!last_one) && (*q != '\0')) {
		// if pkw->idx is not enough, add 32 item to pkw->idx
		REALLOC_VECTOR(pkw->idx, pkw->len, pkw->tlen, sizeof(unsigned char *));
		q2 = strchr(q, '\n');
		if (q2)
			*q2 = '\0';
		else
			last_one = 1;
		if ((p = strchr(q, '=')) != NULL) {
			pkw->idx[pkw->len] = q;
			pkw->len++;
			q = p + strlen(p);
			*q = '\0';
			q++;
		}
		if (!last_one)
			q = q2 + 1;
	}

	gettimeofday(&tv2, NULL);
	//printf("Load %d keywords spent %ldms\n", pkw->len, ((tv2.tv_sec * 1000000 + tv2.tv_usec) - (tv1.tv_sec * 1000000 + tv1.tv_usec)) / 1000);

	return 1;
}

char *search_desc(pkw_t pkw, char *name)
{
	int i;
	char *p, *ret = NULL;

	if (pkw == NULL || (pkw != NULL && pkw->len <= 0))
		return NULL;
	for (i = 0; i < pkw->len; ++i)  {
		p = pkw->idx[i];
		if (strncmp(name, p, strlen(name)) == 0) {
			ret = p + strlen(name);
			break;
		}
	}

	return ret;
}
#endif //TRANSLATE_ON_FLY

void reapchild()
{
	signal(SIGCHLD, reapchild);
	wait(NULL);
}

void http_get_access_ip(void) {
        struct in_addr tmp_access_addr;
        char tmp_access_ip[32];
        char *nv, *nvp, *b;
        int i=0, p=0;

        for(; i<ARRAY_SIZE(access_ip); i++)
                access_ip[i]=0;

        nv = nvp = strdup(nvram_safe_get("http_clientlist"));

        if (nv) {
                while ((b = strsep(&nvp, "<")) != NULL) {
                        if (strlen(b)==0) continue;
                        sprintf(tmp_access_ip, "%s", b);
                        inet_aton(tmp_access_ip, &tmp_access_addr);

                       /* if (p >= ARRAY_SIZE(access_ip)) {
                                _dprintf("%s: access_ip out of range (p %d addr %x)!\n",
                                        __func__, p, (unsigned int)tmp_access_addr.s_addr);
                                break;
                        }
			*/
                        access_ip[p] = (unsigned int)tmp_access_addr.s_addr;
                        p++;
                }
                free(nv);
        }
}

int http_client_ip_check(void) {

	int i = 0;
	if(nvram_match("http_client", "1")) {
		while(i<ARRAY_SIZE(access_ip)) {
			if(access_ip[i]!=0) {
				if(login_ip_tmp==access_ip[i])	return 1;
			}
			i++;
		}
		return 0;
	}		
	return 1;
}

int main(int argc, char **argv)
{
	FILE *pid_fp;
	usockaddr usa;
	int listen_fd;
	socklen_t sz = sizeof(usa);
	char pidfile[32];
	fd_set active_rfds;
	conn_list_t pool;

	// usage: httpd [port]
	if (argc > 1) http_port = atoi(argv[1]);

	nvram_unset("login_timestamp");
	nvram_unset("login_ip");
	http_get_access_ip();


	//Ignore broken pipes 
	signal(SIGPIPE, SIG_IGN);
	signal(SIGCHLD, reapchild);

	// Initialize listen socket 
	if ((listen_fd = initialize_listen_socket(&usa)) < 2) {
		fprintf(stderr, "can't bind to any address\n");
		exit(errno);
	}

	// Daemonize and log PID 
	if (http_port == SERVER_PORT)
		strcpy(pidfile, "/var/run/httpd.pid");
	else
		sprintf(pidfile, "/var/run/httpd-%d.pid", http_port);

	if (!(pid_fp = fopen(pidfile, "w"))) {
		perror(pidfile);
		return errno;
	}
	fprintf(pid_fp, "%d", getpid());
	fclose(pid_fp);

	_x_Setting = atoi(nvram_safe_get("x_Setting"));
	memset(dut_domain_name, 0, sizeof(dut_domain_name));
	strcpy(dut_domain_name, nvram_safe_get("hijdomain"));

	// Loop forever handling requests 


	/* Init connection pool */
	FD_ZERO(&active_rfds);
	TAILQ_INIT(&pool.head);
	pool.count = 0;

	/* Loop forever handling requests */
	for (;;) {
		int max_fd, count;
		struct timeval tv;
		fd_set rfds;
		conn_item_t *item, *next;

		memcpy(&rfds, &active_rfds, sizeof(rfds));
		if (pool.count < MAX_CONN_ACCEPT) {
			FD_SET(listen_fd, &rfds);
			max_fd = listen_fd;
		} else  max_fd = -1;
		
		TAILQ_FOREACH(item, &pool.head, entry)
		max_fd = (item->fd > max_fd) ? item->fd : max_fd;

		/* Wait for new connection or incoming request */
		tv.tv_sec = MAX_CONN_TIMEOUT;
		tv.tv_usec = 0;
		while ((count = select(max_fd + 1, &rfds, NULL, NULL, &tv)) < 0 && errno == EINTR)
			continue;
		if (count < 0) {
			perror("select");
			return errno;
		}

		/* Check and accept new connection */
		if (count && FD_ISSET(listen_fd, &rfds)) {
			item = malloc(sizeof(*item));
			if (item == NULL) {
				perror("malloc");
				return errno;
			}
			while ((item->fd = accept(listen_fd, &item->usa.sa, &sz)) < 0 && errno == EINTR)
				continue;
			if (item->fd < 0) {
				perror("accept");
				free(item);
				continue;
			}

			/* Set the KEEPALIVE option to cull dead connections */
			setsockopt(item->fd, SOL_SOCKET, SO_KEEPALIVE, &int_1, sizeof(int_1));

			/* Add to active connections */
			FD_SET(item->fd, &active_rfds);
			TAILQ_INSERT_TAIL(&pool.head, item, entry);
			pool.count++;
			/* Continue waiting over again */ 
			continue;
		}

		/* Check and process pending or expired requests */
		TAILQ_FOREACH_SAFE(item, &pool.head, entry, next) {
			if (count && !FD_ISSET(item->fd, &rfds))
				continue;

			/* Delete from active connections */
			FD_CLR(item->fd, &active_rfds);
			TAILQ_REMOVE(&pool.head, item, entry);
			pool.count--;

			/* Process request if any */
			if (count) {
				if (!(conn_fp = fdopen(item->fd, "r+"))) {
					perror("fdopen");
					goto skip;
				}

				http_login_cache(&item->usa);
			       
				if (http_client_ip_check())  handle_request();
			
				fflush(conn_fp);
				shutdown(item->fd, 2), item->fd = -1;
				fclose(conn_fp);

			skip:
				/* Skip the rest of */
				if (--count == 0)
					next = NULL;

			}

			/* Close timed out and/or still alive */
			if (item->fd >= 0) {
				shutdown(item->fd, 2);
				close(item->fd);
			}
			free(item);
		}
	}

	shutdown(listen_fd, 2);
	close(listen_fd);

	return 0;
}

