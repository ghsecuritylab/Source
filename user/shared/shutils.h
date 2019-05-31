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
 * Shell-like utility functions
 *
 * Copyright 2003, ASUSTek Inc.
 * All Rights Reserved.                
 *                                     
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of ASUSTek Inc.;   
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior      
 * written permission of ASUSTek Inc..                            
 */

#ifndef _shutils_h_
#define _shutils_h_

extern char * fd2str(int fd);
extern char * file2str(const char *path);

extern int waitfor(int fd, int timeout);
extern int _eval(char *const argv[], char *path, int timeout, pid_t *ppid);
extern char * _backtick(char *const argv[]);

extern int kill_pidfile(char *pidfile);
extern int kill_pidfile_s(char *pidfile, int sig);

extern int safe_fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
extern int safe_fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);

#define ETHER_ADDR_LEN 6

extern int ether_atoe(const char *a, unsigned char *e);
extern char * ether_etoa(const unsigned char *e, char *a);

extern void logmessage(char *logheader, char *fmt, ...);
extern int check_if_file_exist(char *filename);

/*
 * Concatenate two strings together into a caller supplied buffer
 * @param	s1	first string
 * @param	s2	second string
 * @param	buf	buffer large enough to hold both strings
 * @return	buf
 */
static inline char * strcat_r(const char *s1, const char *s2, char *buf)
{
	strcpy(buf, s1);
	strcat(buf, s2);
	return buf;
}	

/* Check for a blank character; that is, a space or a tab */
#define isblank(c) ((c) == ' ' || (c) == '\t')

/* Strip trailing CR/NL from string <s> */
#define chomp(s) ({ \
	char *c = (s) + strlen((s)) - 1; \
	while ((c > (s)) && (*c == '\n' || *c == '\r')) \
		*c-- = '\0'; \
	s; \
})

/* Simple version of _backtick() */
#define backtick(cmd, args...) ({ \
	char *argv[] = { cmd, ## args, NULL }; \
	_backtick(argv); \
})

/* Simple version of _eval() (no timeout and wait for child termination) */
#define eval(cmd, args...) ({ \
	char *argv[] = { cmd, ## args, NULL }; \
	_eval(argv, NULL, 0, NULL); \
})

#define eval_dumb(cmd, args...) ({ \
	char *argv[] = {cmd, ## args, NULL}; \
	_eval(argv, ">/dev/null", 0, NULL); \
})

/* Copy each token in wordlist delimited by space into word */
#define foreach(word, wordlist, next) \
	for (next = &wordlist[strspn(wordlist, " ")], \
	     strncpy(word, next, sizeof(word)), \
	     word[strcspn(word, " ")] = '\0', \
	     word[sizeof(word) - 1] = '\0', \
	     next = strchr(next, ' '); \
	     strlen(word); \
	     next = next ? &next[strspn(next, " ")] : "", \
	     strncpy(word, next, sizeof(word)), \
	     word[strcspn(word, " ")] = '\0', \
	     word[sizeof(word) - 1] = '\0', \
	     next = strchr(next, ' '))

#define for1each(n, word, wordlist, next) \
	for (n = 0, \
	     next = &wordlist[strspn(wordlist, " ")], \
	     strncpy(word, next, sizeof(word)), \
	     word[strcspn(word, " ")] = '\0', \
	     word[sizeof(word) - 1] = '\0', \
	     next = strchr(next, ' '); \
	     strlen(word); \
	     next = next ? &next[strspn(next, " ")] : "", \
	     strncpy(word, next, sizeof(word)), \
	     word[strcspn(word, " ")] = '\0', \
	     word[sizeof(word) - 1] = '\0', \
	     next = strchr(next, ' '), \
	     n++)

/* Copy each token in word1list delimited by space into word1
   Copy each token in word2list delimited by space into word2 */
#define for2each(n, word1, word1list, next1, word2, word2list, next2) \
	for (n = 0, \
	     next1 = &word1list[strspn(word1list, " ")], \
	     next2 = &word2list[strspn(word2list, " ")], \
	     strncpy(word1, next1, sizeof(word1)), \
	     strncpy(word2, next2, sizeof(word2)), \
	     word1[strcspn(word1, " ")] = '\0', \
	     word2[strcspn(word2, " ")] = '\0', \
	     word1[sizeof(word1) - 1] = '\0', \
	     word2[sizeof(word2) - 1] = '\0', \
	     next1 = strchr(next1, ' '), \
	     next2 = strchr(next2, ' '); \
	     strlen(word1), \
	     strlen(word2); \
	     next1 = next1 ? &next1[strspn(next1, " ")] : "", \
	     next2 = next2 ? &next2[strspn(next2, " ")] : "", \
	     strncpy(word1, next1, sizeof(word1)), \
	     strncpy(word2, next2, sizeof(word2)), \
	     word1[strcspn(word1, " ")] = '\0', \
	     word2[strcspn(word2, " ")] = '\0', \
	     word1[sizeof(word1) - 1] = '\0', \
	     word2[sizeof(word2) - 1] = '\0', \
	     next1 = strchr(next1, ' '), \
	     next2 = strchr(next2, ' '), \
	     n++)

/* Return NUL instead of NULL if undefined */
#define safe_getenv(s) (getenv(s) ? : "")

#define vstrsep(buf, sep, args...) _vstrsep(buf, sep, args, NULL)
extern int _vstrsep(char *buf, const char *sep, ...);

/* Print directly to the console */
#define dbg(fmt, args...) do { FILE *fp = fopen("/dev/console", "w"); if (fp) { fprintf(fp, fmt, ## args); fclose(fp); } } while (0)
#define cprintf(fmt, args...) //do { FILE *fp = fopen("/dev/console", "w"); if (fp) { fprintf(fp, fmt, ## args); fclose(fp); } } while (0)

/* Debug print */
#ifdef DEBUG
#define dprintf(fmt, args...) //cprintf("%s: " fmt, __FUNCTION__, ## args)
#else
#define dprintf(fmt, args...) //cprintf(fmt, ## args);
#endif

#endif /* _shutils_h_ */
