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
 * head implementation for busybox
 *
 * Copyright (C) 2003  Manuel Novoa III  <mjn3@codepoet.org>
 *
 * Licensed under GPLv2 or later, see file LICENSE in this tarball for details.
 */

/* BB_AUDIT SUSv3 compliant */
/* BB_AUDIT GNU compatible -c, -q, and -v options in 'fancy' configuration. */
/* http://www.opengroup.org/onlinepubs/007904975/utilities/head.html */

#include "libbb.h"

static const char head_opts[] ALIGN1 =
	"n:"
#if ENABLE_FEATURE_FANCY_HEAD
	"c:qv"
#endif
	;

#if ENABLE_FEATURE_FANCY_HEAD
static const struct suffix_mult head_suffixes[] = {
	{ "b", 512 },
	{ "k", 1024 },
	{ "m", 1024*1024 },
	{ }
};
#endif

static const char header_fmt_str[] ALIGN1 = "\n==> %s <==\n";

int head_main(int argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
int head_main(int argc, char **argv)
{
	unsigned long count = 10;
	unsigned long i;
#if ENABLE_FEATURE_FANCY_HEAD
	int count_bytes = 0;
	int header_threshhold = 1;
#endif

	FILE *fp;
	const char *fmt;
	char *p;
	int opt;
	int c;
	int retval = EXIT_SUCCESS;

#if ENABLE_INCLUDE_SUSv2 || ENABLE_FEATURE_FANCY_HEAD
	/* Allow legacy syntax of an initial numeric option without -n. */
	if (argc > 1 && argv[1][0] == '-'
	 && isdigit(argv[1][1])
	) {
		--argc;
		++argv;
		p = (*argv) + 1;
		goto GET_COUNT;
	}
#endif

	/* No size benefit in converting this to getopt32 */
	while ((opt = getopt(argc, argv, head_opts)) > 0) {
		switch (opt) {
#if ENABLE_FEATURE_FANCY_HEAD
		case 'q':
			header_threshhold = INT_MAX;
			break;
		case 'v':
			header_threshhold = -1;
			break;
		case 'c':
			count_bytes = 1;
			/* fall through */
#endif
		case 'n':
			p = optarg;
#if ENABLE_INCLUDE_SUSv2 || ENABLE_FEATURE_FANCY_HEAD
 GET_COUNT:
#endif

#if !ENABLE_FEATURE_FANCY_HEAD
			count = xatoul(p);
#else
			count = xatoul_sfx(p, head_suffixes);
#endif
			break;
		default:
			bb_show_usage();
		}
	}

	argc -= optind;
	argv += optind;
	if (!*argv)
		*--argv = (char*)"-";

	fmt = header_fmt_str + 1;
#if ENABLE_FEATURE_FANCY_HEAD
	if (argc <= header_threshhold) {
		header_threshhold = 0;
	}
#else
	if (argc <= 1) {
		fmt += 11; /* "" */
	}
	/* Now define some things here to avoid #ifdefs in the code below.
	 * These should optimize out of the if conditions below. */
#define header_threshhold   1
#define count_bytes         0
#endif

	do {
		fp = fopen_or_warn_stdin(*argv);
		if (fp) {
			if (fp == stdin) {
				*argv = (char *) bb_msg_standard_input;
			}
			if (header_threshhold) {
				printf(fmt, *argv);
			}
			i = count;
			while (i && ((c = getc(fp)) != EOF)) {
				if (count_bytes || (c == '\n')) {
					--i;
				}
				putchar(c);
			}
			if (fclose_if_not_stdin(fp)) {
				bb_simple_perror_msg(*argv);	/* Avoid multibyte problems. */
				retval = EXIT_FAILURE;
			}
			die_if_ferror_stdout();
		}
		fmt = header_fmt_str;
	} while (*++argv);

	fflush_stdout_and_exit(retval);
}
