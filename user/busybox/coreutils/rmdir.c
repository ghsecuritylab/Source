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
 * rmdir implementation for busybox
 *
 * Copyright (C) 2003  Manuel Novoa III  <mjn3@codepoet.org>
 *
 * Licensed under GPLv2 or later, see file LICENSE in this tarball for details.
 */

/* BB_AUDIT SUSv3 compliant */
/* http://www.opengroup.org/onlinepubs/007904975/utilities/rmdir.html */

#include "libbb.h"

/* This is a NOFORK applet. Be very careful! */


#define PARENTS 0x01
#define IGNORE_NON_EMPTY 0x02

int rmdir_main(int argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
int rmdir_main(int argc UNUSED_PARAM, char **argv)
{
	int status = EXIT_SUCCESS;
	int flags;
	char *path;

#if ENABLE_FEATURE_RMDIR_LONG_OPTIONS
	static const char rmdir_longopts[] ALIGN1 =
		"parents\0"                  No_argument "p"
		/* Debian etch: many packages fail to be purged or installed
		 * because they desperately want this option: */
		"ignore-fail-on-non-empty\0" No_argument "\xff"
		;
	applet_long_options = rmdir_longopts;
#endif
	flags = getopt32(argv, "p");
	argv += optind;

	if (!*argv) {
		bb_show_usage();
	}

	do {
		path = *argv;

		while (1) {
			if (rmdir(path) < 0) {
#if ENABLE_FEATURE_RMDIR_LONG_OPTIONS
				if ((flags & IGNORE_NON_EMPTY) && errno == ENOTEMPTY)
					break;
#endif
				bb_perror_msg("'%s'", path);	/* Match gnu rmdir msg. */
				status = EXIT_FAILURE;
			} else if (flags & PARENTS) {
				/* Note: path was not "" since rmdir succeeded. */
				path = dirname(path);
				/* Path is now just the parent component.  Dirname
				 * returns "." if there are no parents.
				 */
				if (NOT_LONE_CHAR(path, '.')) {
					continue;
				}
			}
			break;
		}
	} while (*++argv);

	return status;
}
