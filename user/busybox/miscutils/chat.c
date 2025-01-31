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
/* vi: set sw=4 ts=4: */
/*
 * bare bones chat utility
 * inspired by ppp's chat
 *
 * Copyright (C) 2008 by Vladimir Dronnikov <dronnikov@gmail.com>
 *
 * Licensed under GPLv2, see file LICENSE in this tarball for details.
 */
#include "libbb.h"

/*
#define ENABLE_FEATURE_CHAT_NOFAIL              1 // +126 bytes
#define ENABLE_FEATURE_CHAT_TTY_HIFI            0 // + 70 bytes
#define ENABLE_FEATURE_CHAT_IMPLICIT_CR         1 // + 44 bytes
#define ENABLE_FEATURE_CHAT_SEND_ESCAPES        0 // +103 bytes
#define ENABLE_FEATURE_CHAT_VAR_ABORT_LEN       0 // + 70 bytes
#define ENABLE_FEATURE_CHAT_CLR_ABORT           0 // +113 bytes
#define ENABLE_FEATURE_CHAT_SWALLOW_OPTS        0 // + 23 bytes
*/

// default timeout: 45 sec
#define	DEFAULT_CHAT_TIMEOUT 45*1000
// max length of "abort string",
// i.e. device reply which causes termination
#define MAX_ABORT_LEN 50

// possible exit codes
enum {
	ERR_OK = 0,     // all's well
	ERR_MEM,        // read too much while expecting
	ERR_IO,         // signalled or I/O error
	ERR_TIMEOUT,    // timed out while expecting
	ERR_ABORT,      // first abort condition was met
//	ERR_ABORT2,     // second abort condition was met
//	...
};

// exit code
// N.B> 10 bytes for volatile. Why all these signals?!
static /*volatile*/ smallint exitcode;

// trap for critical signals
static void signal_handler(UNUSED_PARAM int signo)
{
	// report I/O error condition
	exitcode = ERR_IO;
}

#if !ENABLE_FEATURE_CHAT_IMPLICIT_CR
#define unescape(s, nocr) unescape(s)
#endif
static size_t unescape(char *s, int *nocr)
{
	char *start = s;
	char *p = s;

	while (*s) {
		char c = *s;
		// do we need special processing?
		// standard escapes + \s for space and \N for \0
		// \c inhibits terminating \r for commands and is noop for expects
		if ('\\' == c) {
			c = *++s;
			if (c) {
#if ENABLE_FEATURE_CHAT_IMPLICIT_CR
				if ('c' == c) {
					*nocr = 1;
					goto next;
				}
#endif
				if ('N' == c) {
					c = '\0';
				} else if ('s' == c) {
					c = ' ';
#if ENABLE_FEATURE_CHAT_NOFAIL
				// unescape leading dash only
				// TODO: and only for expect, not command string
				} else if ('-' == c && (start + 1 == s)) {
					//c = '-';
#endif
				} else {
					c = bb_process_escape_sequence((const char **)&s);
					s--;
				}
			}
		// ^A becomes \001, ^B -- \002 and so on...
		} else if ('^' == c) {
			c = *++s-'@';
		}
		// put unescaped char
		*p++ = c;
#if ENABLE_FEATURE_CHAT_IMPLICIT_CR
 next:
#endif
		// next char
		s++;
	}
	*p = '\0';

	return p - start;
}


int chat_main(int argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
int chat_main(int argc UNUSED_PARAM, char **argv)
{
// should we dump device output? to what fd? by default no.
// this can be controlled later via ECHO {ON|OFF} chat directive
//	int echo_fd;
	bool echo = 0;
	// collection of device replies which cause unconditional termination
	llist_t *aborts = NULL;
	// inactivity period
	int timeout = DEFAULT_CHAT_TIMEOUT;
	// maximum length of abort string
#if ENABLE_FEATURE_CHAT_VAR_ABORT_LEN
	size_t max_abort_len = 0;
#else
#define max_abort_len MAX_ABORT_LEN
#endif
#if ENABLE_FEATURE_CHAT_TTY_HIFI
	struct termios tio0, tio;
#endif
	// directive names
	enum {
		DIR_HANGUP = 0,
		DIR_ABORT,
#if ENABLE_FEATURE_CHAT_CLR_ABORT
		DIR_CLR_ABORT,
#endif
		DIR_TIMEOUT,
		DIR_ECHO,
		DIR_SAY,
	};

	// make x* functions fail with correct exitcode
	xfunc_error_retval = ERR_IO;

	// trap vanilla signals to prevent process from being killed suddenly
	bb_signals(0
		+ (1 << SIGHUP)
		+ (1 << SIGINT)
		+ (1 << SIGTERM)
		+ (1 << SIGPIPE)
		, signal_handler);

#if ENABLE_FEATURE_CHAT_TTY_HIFI
	tcgetattr(STDIN_FILENO, &tio);
	tio0 = tio;
	cfmakeraw(&tio);
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &tio);
#endif

#if ENABLE_FEATURE_CHAT_SWALLOW_OPTS
	getopt32(argv, "vVsSE");
	argv += optind;
#else
	argv++; // goto first arg
#endif
	// handle chat expect-send pairs
	while (*argv) {
		// directive given? process it
		int key = index_in_strings(
			"HANGUP\0" "ABORT\0"
#if ENABLE_FEATURE_CHAT_CLR_ABORT
			"CLR_ABORT\0"
#endif
			"TIMEOUT\0" "ECHO\0" "SAY\0"
			, *argv
		);
		if (key >= 0) {
			// cache directive value
			char *arg = *++argv;
			// ON -> 1, anything else -> 0
			bool onoff = !strcmp("ON", arg);
			// process directive
			if (DIR_HANGUP == key) {
				// turn SIGHUP on/off
				signal(SIGHUP, onoff ? signal_handler : SIG_IGN);
			} else if (DIR_ABORT == key) {
				// append the string to abort conditions
#if ENABLE_FEATURE_CHAT_VAR_ABORT_LEN
				size_t len = strlen(arg);
				if (len > max_abort_len)
					max_abort_len = len;
#endif
				llist_add_to_end(&aborts, arg);
#if ENABLE_FEATURE_CHAT_CLR_ABORT
			} else if (DIR_CLR_ABORT == key) {
				// remove the string from abort conditions
				// N.B. gotta refresh maximum length too...
#if ENABLE_FEATURE_CHAT_VAR_ABORT_LEN
				max_abort_len = 0;
#endif
				for (llist_t *l = aborts; l; l = l->link) {
#if ENABLE_FEATURE_CHAT_VAR_ABORT_LEN
					size_t len = strlen(l->data);
#endif
					if (!strcmp(arg, l->data)) {
						llist_unlink(&aborts, l);
						continue;
					}
#if ENABLE_FEATURE_CHAT_VAR_ABORT_LEN
					if (len > max_abort_len)
						max_abort_len = len;
#endif
				}
#endif
			} else if (DIR_TIMEOUT == key) {
				// set new timeout
				// -1 means OFF
				timeout = atoi(arg) * 1000;
				// 0 means default
				// >0 means value in msecs
				if (!timeout)
					timeout = DEFAULT_CHAT_TIMEOUT;
			} else if (DIR_ECHO == key) {
				// turn echo on/off
				// N.B. echo means dumping output
				// from stdin (device) to stderr
				echo = onoff;
//TODO?				echo_fd = onoff * STDERR_FILENO;
//TODO?				echo_fd = xopen(arg, O_WRONLY|O_CREAT|O_TRUNC);
			} else if (DIR_SAY == key) {
				// just print argument verbatim
				fprintf(stderr, arg);
			}
			// next, please!
			argv++;
		// ordinary expect-send pair!
		} else {
			//-----------------------
			// do expect
			//-----------------------
			int expect_len;
			size_t buf_len = 0;
			size_t max_len = max_abort_len;

			struct pollfd pfd;
#if ENABLE_FEATURE_CHAT_NOFAIL
			int nofail = 0;
#endif
			char *expect = *argv++;

			// sanity check: shall we really expect something?
			if (!expect)
				goto expect_done;

#if ENABLE_FEATURE_CHAT_NOFAIL
			// if expect starts with -
			if ('-' == *expect) {
				// swallow -
				expect++;
				// and enter nofail mode
				nofail++;
			}
#endif

#ifdef ___TEST___BUF___ // test behaviour with a small buffer
#	undef COMMON_BUFSIZE
#	define COMMON_BUFSIZE 6
#endif
			// expand escape sequences in expect
			expect_len = unescape(expect, &expect_len /*dummy*/);
			if (expect_len > max_len)
				max_len = expect_len;
			// sanity check:
			// we should expect more than nothing but not more than input buffer
			// TODO: later we'll get rid of fixed-size buffer
			if (!expect_len)
				goto expect_done;
			if (max_len >= COMMON_BUFSIZE) {
				exitcode = ERR_MEM;
				goto expect_done;
			}

			// get reply
			pfd.fd = STDIN_FILENO;
			pfd.events = POLLIN;
			while (!exitcode
			    && poll(&pfd, 1, timeout) > 0
			    && (pfd.revents & POLLIN)
			) {
#define buf bb_common_bufsiz1
				llist_t *l;
				ssize_t delta;

				// read next char from device
				if (safe_read(STDIN_FILENO, buf+buf_len, 1) > 0) {
					// dump device output if ECHO ON or RECORD fname
//TODO?					if (echo_fd > 0) {
//TODO?						full_write(echo_fd, buf+buf_len, 1);
//TODO?					}
					if (echo > 0)
						full_write(STDERR_FILENO, buf+buf_len, 1);
					buf_len++;
					// move input frame if we've reached higher bound
					if (buf_len > COMMON_BUFSIZE) {
						memmove(buf, buf+buf_len-max_len, max_len);
						buf_len = max_len;
					}
				}
				// N.B. rule of thumb: values being looked for can
				// be found only at the end of input buffer
				// this allows to get rid of strstr() and memmem()

				// TODO: make expect and abort strings processed uniformly
				// abort condition is met? -> bail out
				for (l = aborts, exitcode = ERR_ABORT; l; l = l->link, ++exitcode) {
					size_t len = strlen(l->data);
					delta = buf_len-len;
					if (delta >= 0 && !memcmp(buf+delta, l->data, len))
						goto expect_done;
				}
				exitcode = ERR_OK;

				// expected reply received? -> goto next command
				delta = buf_len - expect_len;
				if (delta >= 0 && !memcmp(buf+delta, expect, expect_len))
					goto expect_done;
#undef buf
			}

			// device timed out or unexpected reply received
			exitcode = ERR_TIMEOUT;
 expect_done:
#if ENABLE_FEATURE_CHAT_NOFAIL
			// on success and when in nofail mode
			// we should skip following subsend-subexpect pairs
			if (nofail) {
				if (!exitcode) {
					// find last send before non-dashed expect
					while (*argv && argv[1] && '-' == argv[1][0])
						argv += 2;
					// skip the pair
					// N.B. do we really need this?!
					if (!*argv++ || !*argv++)
						break;
				}
				// nofail mode also clears all but IO errors (or signals)
				if (ERR_IO != exitcode)
					exitcode = ERR_OK;
			}
#endif
			// bail out unless we expected successfully
			if (exitcode)
				break;

			//-----------------------
			// do send
			//-----------------------
			if (*argv) {
#if ENABLE_FEATURE_CHAT_IMPLICIT_CR
				int nocr = 0; // inhibit terminating command with \r
#endif
				char *loaded = NULL; // loaded command
				size_t len;
				char *buf = *argv++;

				// if command starts with @
				// load "real" command from file named after @
				if ('@' == *buf) {
					// skip the @ and any following white-space
					trim(++buf);
					buf = loaded = xmalloc_xopen_read_close(buf, NULL);
				}

				// expand escape sequences in command
				len = unescape(buf, &nocr);

				// send command
#if ENABLE_FEATURE_CHAT_SEND_ESCAPES
				pfd.fd = STDOUT_FILENO;
				pfd.events = POLLOUT;
				while (len && !exitcode
				    && poll(&pfd, 1, timeout) > 0
				    && (pfd.revents & POLLOUT)
				) {
					// ugly! ugly! ugly!
					// gotta send char by char to achieve this!
					// Brrr...
					// "\\d" means 1 sec delay, "\\p" means 0.01 sec delay
					// "\\K" means send BREAK
					char c = *buf;
					if ('\\' == c) {
						c = *++buf;
						if ('d' == c) {
							sleep(1);
							len--;
							continue;
						} else if ('p' == c) {
							usleep(10000);
							len--;
							continue;
						} else if ('K' == c) {
							tcsendbreak(STDOUT_FILENO, 0);
							len--;
							continue;
						} else {
							buf--;
						}
					}
					if (safe_write(STDOUT_FILENO, buf, 1) > 0) {
						len--;
						buf++;
					} else
						break;
				}
#else
//				if (len) {
					alarm(timeout);
					len -= full_write(STDOUT_FILENO, buf, len);
					alarm(0);
//				}
#endif

				// report I/O error if there still exists at least one non-sent char
				if (len)
					exitcode = ERR_IO;

				// free loaded command (if any)
				if (loaded)
					free(loaded);
#if ENABLE_FEATURE_CHAT_IMPLICIT_CR
				// or terminate command with \r (if not inhibited)
				else if (!nocr)
					xwrite(STDOUT_FILENO, "\r", 1);
#endif

				// bail out unless we sent command successfully
				if (exitcode)
					break;

			}
		}
	}

#if ENABLE_FEATURE_CHAT_TTY_HIFI
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &tio0);
#endif

	return exitcode;
}
