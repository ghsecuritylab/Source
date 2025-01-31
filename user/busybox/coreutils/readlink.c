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
 * Mini readlink implementation for busybox
 *
 * Copyright (C) 2000,2001 Matt Kraai <kraai@alumni.carnegiemellon.edu>
 *
 * Licensed under GPL v2 or later, see file LICENSE in this tarball for details.
 */

#include "libbb.h"

int readlink_main(int argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
int readlink_main(int argc UNUSED_PARAM, char **argv)
{
	char *buf;
	char *fname;
	char pathbuf[PATH_MAX];

	USE_FEATURE_READLINK_FOLLOW(
		unsigned opt;
		/* We need exactly one non-option argument.  */
		opt_complementary = "=1";
		opt = getopt32(argv, "f");
		fname = argv[optind];
	)
	SKIP_FEATURE_READLINK_FOLLOW(
		const unsigned opt = 0;
		if (argc != 2) bb_show_usage();
		fname = argv[1];
	)

	/* compat: coreutils readlink reports errors silently via exit code */
	logmode = LOGMODE_NONE;

	if (opt) {
		buf = realpath(fname, pathbuf);
	} else {
		buf = xmalloc_readlink_or_warn(fname);
	}

	if (!buf)
		return EXIT_FAILURE;
	puts(buf);

	if (ENABLE_FEATURE_CLEAN_UP && !opt)
		free(buf);

	fflush_stdout_and_exit(EXIT_SUCCESS);
}
