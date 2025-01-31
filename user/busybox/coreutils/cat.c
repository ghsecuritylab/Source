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
 * cat implementation for busybox
 *
 * Copyright (C) 2003  Manuel Novoa III  <mjn3@codepoet.org>
 *
 * Licensed under GPLv2, see file License in this tarball for details.
 */

/* BB_AUDIT SUSv3 compliant */
/* http://www.opengroup.org/onlinepubs/007904975/utilities/cat.html */

#include "libbb.h"

/* This is a NOFORK applet. Be very careful! */


int bb_cat(char **argv)
{
	int fd;
	int retval = EXIT_SUCCESS;

	if (!*argv)
		argv = (char**) &bb_argv_dash;

	do {
		fd = open_or_warn_stdin(*argv);
		if (fd >= 0) {
			/* This is not a xfunc - never exits */
			off_t r = bb_copyfd_eof(fd, STDOUT_FILENO);
			if (fd != STDIN_FILENO)
				close(fd);
			if (r >= 0)
				continue;
		}
		retval = EXIT_FAILURE;
	} while (*++argv);

	return retval;
}

int cat_main(int argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
int cat_main(int argc UNUSED_PARAM, char **argv)
{
	getopt32(argv, "u");
	argv += optind;
	return bb_cat(argv);
}
