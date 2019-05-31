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
 * milli_httpd - pretty small HTTP server
 *
 * Copyright (C) 2013 ASUSTek Inc.
 *
 */

#ifndef _httpd_h_
#define _httpd_h_

#if defined(DEBUG) && defined(DMALLOC)
#include <dmalloc.h>
#endif

/* Basic authorization userid and passwd limit */
#define AUTH_MAX 64
#define DEFAULT_LOGIN_MAX_NUM 5
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

/* Generic MIME type handler */
struct mime_handler {
	char *pattern;
	char *mime_type;
	char *extra_header;
	void (*input)(char *path, FILE *stream, int len, char *boundary);
	void (*output)(char *path, FILE *stream);
	void (*auth)(char *userid, char *passwd, char *realm);
};
extern struct mime_handler mime_handlers[];

#define CHECK_REFERER	1

#define SERVER_NAME "httpd/2.0"
#define SERVER_PORT 80
#define PROTOCOL "HTTP/1.0"
#define RFC1123FMT "%a, %d %b %Y %H:%M:%S GMT"

//asus token status for APP
#define NOTOKEN		1
#define AUTHFAIL	2
#define ACCOUNTFAIL	3
#define NOREFERER	4
#define WEB_NOREFERER	5
#define REFERERFAIL	6
#define LOGINLOCK	7
#define ISLOGOUT	8
#define NOLOGIN		9

/* MIME referer */
struct mime_referer {
	char *pattern;
	int flag;
};

extern struct mime_referer mime_referers[];

typedef struct asus_token_table asus_token_t;
struct asus_token_table {
	char token[32];
	char ipaddr[16];
	char login_timestampstr[32];
	char host[64];
	asus_token_t *next;
};

extern asus_token_t *head;
extern asus_token_t *curr;

/* CGI helper functions */
extern void init_cgi(char *query);
extern char * get_cgi(char *name);
#ifdef TRANSLATE_ON_FLY
struct language_table{
	char *Lang;
	char *Target_Lang;
};

static struct language_table language_tables[] = {
        {"en-us", "EN"},
        {"en", "EN"},
        {"fi", "FI"},
        {"fi-FI", "FI"},
        {"fr", "FR"},
        {"fr-fr", "FR"},
        {"cs-cz", "CZ"},
        {"cs", "CZ"},
        {"zh-tw", "TW"},
        {"zh", "TW"},   
        {"ru-ru", "RU"},
        {"ru", "RU"},
        {"de-at", "DE"},
        {"de-li", "DE"},
        {"de-lu", "DE"},
        {"de-de", "DE"},
        {"de-ch", "DE"},
        {"de", "DE"},
        {"pl-pl", "PL"},
        {"pl", "PL"},
        {"zh-hk", "CN"},
        {"zh-cn", "CN"},
        {"ms", "MS"},
        {"th", "TH"},
        {"tr", "TR"},
        {"tr-tr", "TR"},
	{"ja", "JP"},
	{"ja-JP", "JP"},

        {NULL, NULL}
};

typedef struct kw_s     {
        int len, tlen;                                          // actually / total
        unsigned char **idx;
        unsigned char *buf;
} kw_t, *pkw_t;

#define INC_ITEM        128
#define REALLOC_VECTOR(p, len, size, item_size) {                               \
        assert ((len) >= 0 && (len) <= (size));                                         \
        if (len == size)        {                                                                               \
                int new_size;                                                                                   \
                void *np;                                                                                               \
                /* out of vector, reallocate */                                                 \
                new_size = size + INC_ITEM;                                                             \
                np = malloc (new_size * (item_size));                                   \
                assert (np != NULL);                                                                    \
                bzero (np, new_size * (item_size));                                             \
                memcpy (np, p, len * (item_size));                                              \
                free (p);                                                                                               \
                p = np;                                                                                                 \
                size = new_size;                                                                                \
        }    \
}
#endif  // defined TRANSLATE_ON_FLY

/* GoAhead 2.1 compatibility */
typedef FILE * webs_t;
typedef char char_t;
#define T(s) (s)
#define __TMPVAR(x) tmpvar ## x
#define _TMPVAR(x) __TMPVAR(x)
#define TMPVAR _TMPVAR(__LINE__)
#define websWrite(wp, fmt, args...) ({ int TMPVAR = fprintf(wp, fmt, ## args); fflush(wp); TMPVAR; })
#define websError(wp, code, msg, args...) fprintf(wp, msg, ## args)
#define websHeader(wp) fputs("<html lang=\"en\">", wp)
#define websFooter(wp) fputs("</html>", wp)
#define websDone(wp, code) fflush(wp)
#define websGetVar(wp, var, default) (get_cgi(var) ? : default)
#define websDefaultHandler(wp, urlPrefix, webDir, arg, url, path, query) ({ do_ej(path, wp); fflush(wp); 1; })
#define websWriteData(wp, buf, nChars) ({ int TMPVAR = fwrite(buf, 1, nChars, wp); fflush(wp); TMPVAR; })
#define websWriteDataNonBlock websWriteData

extern int ejArgs(int argc, char_t **argv, char_t *fmt, ...);

/* GoAhead 2.1 Embedded JavaScript compatibility */
extern void do_ej(char *path, FILE *stream);
struct ej_handler {
	char *pattern;
	int (*output)(int eid, webs_t wp, int argc, char_t **argv);
};
extern struct ej_handler ej_handlers[];

#ifdef TRANSLATE_ON_FLY
extern int load_dictionary (char *lang, pkw_t pkw);
extern char* search_desc (pkw_t pkw, char *name);
//extern char Accept_Language[16];
#endif //defined TRANSLATE_ON_FLY

extern int _x_Setting;
extern char dut_domain_name[32];
extern char Last_Req_Host[64];

#endif /* _httpd_h_ */
