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
 * mknod implementation for busybox
 *
 * Copyright (C) 2003  Manuel Novoa III  <mjn3@codepoet.org>
 *
 * Licensed under GPLv2 or later, see file LICENSE in this tarball for details.
 */

/* BB_AUDIT SUSv3 N/A -- Matches GNU behavior. */

#include <sys/sysmacros.h>  // For makedev

#include "libbb.h"
#include "libcoreutils/coreutils.h"

static const char modes_chars[] ALIGN1 = { 'p', 'c', 'u', 'b', 0, 1, 1, 2 };
static const mode_t modes_cubp[] = { S_IFIFO, S_IFCHR, S_IFBLK };

int mknod_main(int argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
int mknod_main(int argc, char **argv)
{
	mode_t mode;
	dev_t dev;
	const char *name;

	mode = getopt_mk_fifo_nod(argv);
	argv += optind;
	argc -= optind;

	if (argc >= 2) {
		name = strchr(modes_chars, argv[1][0]);
		if (name != NULL) {
			mode |= modes_cubp[(int)(name[4])];

			dev = 0;
			if (*name != 'p') {
				argc -= 2;
				if (argc == 2) {
					/* Autodetect what the system supports; these macros should
					 * optimize out to two constants. */
					dev = makedev(xatoul_range(argv[2], 0, major(UINT_MAX)),
					              xatoul_range(argv[3], 0, minor(UINT_MAX)));
				}
			}

			if (argc == 2) {
				name = *argv;
				if (mknod(name, mode, dev) == 0) {
					return EXIT_SUCCESS;
				}
				bb_simple_perror_msg_and_die(name);
			}
		}
	}
	bb_show_usage();
}
