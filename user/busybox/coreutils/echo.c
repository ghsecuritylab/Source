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
 * echo implementation for busybox
 *
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Licensed under GPLv2 or later, see file LICENSE in this tarball for details.
 *
 * Original copyright notice is retained at the end of this file.
 */

/* BB_AUDIT SUSv3 compliant -- unless configured as fancy echo. */
/* http://www.opengroup.org/onlinepubs/007904975/utilities/echo.html */

/* Mar 16, 2003      Manuel Novoa III   (mjn3@codepoet.org)
 *
 * Because of behavioral differences, implemented configurable SUSv3
 * or 'fancy' gnu-ish behaviors.  Also, reduced size and fixed bugs.
 * 1) In handling '\c' escape, the previous version only suppressed the
 *     trailing newline.  SUSv3 specifies _no_ output after '\c'.
 * 2) SUSv3 specifies that octal escapes are of the form \0{#{#{#}}}.
 *    The previous version did not allow 4-digit octals.
 */

#include "libbb.h"

/* This is a NOFORK applet. Be very careful! */

/* NB: can be used by shell even if not enabled as applet */

int echo_main(int argc UNUSED_PARAM, char **argv)
{
	const char *arg;
#if !ENABLE_FEATURE_FANCY_ECHO
	enum {
		eflag = '\\',
		nflag = 1,  /* 1 -- print '\n' */
	};

	/* We must check that stdout is not closed.
	 * The reason for this is highly non-obvious.
	 * echo_main is used from shell. Shell must correctly handle "echo foo"
	 * if stdout is closed. With stdio, output gets shoveled into
	 * stdout buffer, and even fflush cannot clear it out. It seems that
	 * even if libc receives EBADF on write attempts, it feels determined
	 * to output data no matter what. So it will try later,
	 * and possibly will clobber future output. Not good. */
// TODO: check fcntl() & O_ACCMODE == O_WRONLY or O_RDWR?
	if (fcntl(1, F_GETFL) == -1)
		return 1; /* match coreutils 6.10 (sans error msg to stderr) */
	//if (dup2(1, 1) != 1) - old way
	//	return 1;

	arg = *++argv;
	if (!arg)
		goto newline_ret;
#else
	const char *p;
	char nflag = 1;
	char eflag = 0;

	/* We must check that stdout is not closed. */
	if (fcntl(1, F_GETFL) == -1)
		return 1;

	while (1) {
		arg = *++argv;
		if (!arg)
			goto newline_ret;
		if (*arg != '-')
			break;

		/* If it appears that we are handling options, then make sure
		 * that all of the options specified are actually valid.
		 * Otherwise, the string should just be echoed.
		 */
		p = arg + 1;
		if (!*p)	/* A single '-', so echo it. */
			goto just_echo;

		do {
			if (!strrchr("neE", *p))
				goto just_echo;
		} while (*++p);

		/* All of the options in this arg are valid, so handle them. */
		p = arg + 1;
		do {
			if (*p == 'n')
				nflag = 0;
			if (*p == 'e')
				eflag = '\\';
		} while (*++p);
	}
 just_echo:
#endif
	while (1) {
		/* arg is already == *argv and isn't NULL */
		int c;

		if (!eflag) {
			/* optimization for very common case */
			fputs(arg, stdout);
		} else while ((c = *arg++)) {
			if (c == eflag) {	/* Check for escape seq. */
				if (*arg == 'c') {
					/* '\c' means cancel newline and
					 * ignore all subsequent chars. */
					goto ret;
				}
#if !ENABLE_FEATURE_FANCY_ECHO
				/* SUSv3 specifies that octal escapes must begin with '0'. */
				if ( ((int)(unsigned char)(*arg) - '0') >= 8) /* '8' or bigger */
#endif
				{
					/* Since SUSv3 mandates a first digit of 0, 4-digit octals
					* of the form \0### are accepted. */
					if (*arg == '0') {
						/* NB: don't turn "...\0" into "...\" */
						if (arg[1] && ((unsigned char)(arg[1]) - '0') < 8) {
							arg++;
						}
					}
					/* bb_process_escape_sequence handles NUL correctly
					 * ("...\" case). */
					c = bb_process_escape_sequence(&arg);
				}
			}
			bb_putchar(c);
		}

		arg = *++argv;
		if (!arg)
			break;
		bb_putchar(' ');
	}

 newline_ret:
	if (nflag) {
		bb_putchar('\n');
	}
 ret:
	return fflush(stdout);
}

/*-
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Kenneth Almquist.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. <BSD Advertising Clause omitted per the July 22, 1999 licensing change
 *		ftp://ftp.cs.berkeley.edu/pub/4bsd/README.Impt.License.Change>
 *
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)echo.c	8.1 (Berkeley) 5/31/93
 */

#ifdef VERSION_WITH_WRITEV
/* We can't use stdio.
 * The reason for this is highly non-obvious.
 * echo_main is used from shell. Shell must correctly handle "echo foo"
 * if stdout is closed. With stdio, output gets shoveled into
 * stdout buffer, and even fflush cannot clear it out. It seems that
 * even if libc receives EBADF on write attempts, it feels determined
 * to output data no matter what. So it will try later,
 * and possibly will clobber future output. Not good.
 *
 * Using writev instead, with 'direct' conversion of argv vector.
 */

int echo_main(int argc, char **argv)
{
	struct iovec io[argc];
	struct iovec *cur_io = io;
	char *arg;
	char *p;
#if !ENABLE_FEATURE_FANCY_ECHO
	enum {
		eflag = '\\',
		nflag = 1,  /* 1 -- print '\n' */
	};
	arg = *++argv;
	if (!arg)
		goto newline_ret;
#else
	char nflag = 1;
	char eflag = 0;

	while (1) {
		arg = *++argv;
		if (!arg)
			goto newline_ret;
		if (*arg != '-')
			break;

		/* If it appears that we are handling options, then make sure
		 * that all of the options specified are actually valid.
		 * Otherwise, the string should just be echoed.
		 */
		p = arg + 1;
		if (!*p)	/* A single '-', so echo it. */
			goto just_echo;

		do {
			if (!strrchr("neE", *p))
				goto just_echo;
		} while (*++p);

		/* All of the options in this arg are valid, so handle them. */
		p = arg + 1;
		do {
			if (*p == 'n')
				nflag = 0;
			if (*p == 'e')
				eflag = '\\';
		} while (*++p);
	}
 just_echo:
#endif

	while (1) {
		/* arg is already == *argv and isn't NULL */
		int c;

		cur_io->iov_base = p = arg;

		if (!eflag) {
			/* optimization for very common case */
			p += strlen(arg);
		} else while ((c = *arg++)) {
			if (c == eflag) {	/* Check for escape seq. */
				if (*arg == 'c') {
					/* '\c' means cancel newline and
					 * ignore all subsequent chars. */
					cur_io->iov_len = p - (char*)cur_io->iov_base;
					cur_io++;
					goto ret;
				}
#if !ENABLE_FEATURE_FANCY_ECHO
				/* SUSv3 specifies that octal escapes must begin with '0'. */
				if ( (((unsigned char)*arg) - '1') >= 7)
#endif
				{
					/* Since SUSv3 mandates a first digit of 0, 4-digit octals
					* of the form \0### are accepted. */
					if (*arg == '0' && ((unsigned char)(arg[1]) - '0') < 8) {
						arg++;
					}
					/* bb_process_escape_sequence can handle nul correctly */
					c = bb_process_escape_sequence( (void*) &arg);
				}
			}
			*p++ = c;
		}

		arg = *++argv;
		if (arg)
			*p++ = ' ';
		cur_io->iov_len = p - (char*)cur_io->iov_base;
		cur_io++;
		if (!arg)
			break;
	}

 newline_ret:
	if (nflag) {
		cur_io->iov_base = (char*)"\n";
		cur_io->iov_len = 1;
		cur_io++;
	}
 ret:
	/* TODO: implement and use full_writev? */
	return writev(1, io, (cur_io - io)) >= 0;
}
#endif
