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
 * tee implementation for busybox
 *
 * Copyright (C) 2003  Manuel Novoa III  <mjn3@codepoet.org>
 *
 * Licensed under GPLv2 or later, see file LICENSE in this tarball for details.
 */

/* BB_AUDIT SUSv3 compliant */
/* http://www.opengroup.org/onlinepubs/007904975/utilities/tee.html */

#include "libbb.h"
#include <signal.h>

int tee_main(int argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
int tee_main(int argc, char **argv)
{
	const char *mode = "w\0a";
	FILE **files;
	FILE **fp;
	char **names;
	char **np;
	char retval;
//TODO: make unconditional
#if ENABLE_FEATURE_TEE_USE_BLOCK_IO
	ssize_t c;
# define buf bb_common_bufsiz1
#else
	int c;
#endif
	retval = getopt32(argv, "ia");	/* 'a' must be 2nd */
	argc -= optind;
	argv += optind;

	mode += (retval & 2);	/* Since 'a' is the 2nd option... */

	if (retval & 1) {
		signal(SIGINT, SIG_IGN); /* TODO - switch to sigaction. (why?) */
	}
	retval = EXIT_SUCCESS;
	/* gnu tee ignores SIGPIPE in case one of the output files is a pipe
	 * that doesn't consume all its input.  Good idea... */
	signal(SIGPIPE, SIG_IGN);

	/* Allocate an array of FILE *'s, with one extra for a sentinal. */
	fp = files = xzalloc(sizeof(FILE *) * (argc + 2));
	np = names = argv - 1;

	files[0] = stdout;
	goto GOT_NEW_FILE;
	do {
		*fp = stdout;
		if (NOT_LONE_DASH(*argv)) {
			*fp = fopen_or_warn(*argv, mode);
			if (*fp == NULL) {
				retval = EXIT_FAILURE;
				argv++;
				continue;
			}
		}
		*np = *argv++;
 GOT_NEW_FILE:
		setbuf(*fp, NULL);	/* tee must not buffer output. */
		fp++;
		np++;
	} while (*argv);
	/* names[0] will be filled later */

#if ENABLE_FEATURE_TEE_USE_BLOCK_IO
	while ((c = safe_read(STDIN_FILENO, buf, sizeof(buf))) > 0) {
		fp = files;
		do
			fwrite(buf, 1, c, *fp++);
		while (*fp);
	}
	if (c < 0) {		/* Make sure read errors are signaled. */
		retval = EXIT_FAILURE;
	}
#else
	setvbuf(stdout, NULL, _IONBF, 0);
	while ((c = getchar()) != EOF) {
		fp = files;
		do
			putc(c, *fp++);
		while (*fp);
	}
#endif

	/* Now we need to check for i/o errors on stdin and the various
	 * output files.  Since we know that the first entry in the output
	 * file table is stdout, we can save one "if ferror" test by
	 * setting the first entry to stdin and checking stdout error
	 * status with fflush_stdout_and_exit()... although fflush()ing
	 * is unnecessary here. */
	np = names;
	fp = files;
	names[0] = (char *) bb_msg_standard_input;
	files[0] = stdin;
	do {	/* Now check for input and output errors. */
		/* Checking ferror should be sufficient, but we may want to fclose.
		 * If we do, remember not to close stdin! */
		die_if_ferror(*fp++, *np++);
	} while (*fp);

	fflush_stdout_and_exit(retval);
}
