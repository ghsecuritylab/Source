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
 * Mini basename implementation for busybox
 *
 * Copyright (C) 1999-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * Licensed under GPLv2 or later, see file LICENSE in this tarball for details.
 *
 */

/* BB_AUDIT SUSv3 compliant */
/* http://www.opengroup.org/onlinepubs/007904975/utilities/basename.html */


/* Mar 16, 2003      Manuel Novoa III   (mjn3@codepoet.org)
 *
 * Changes:
 * 1) Now checks for too many args.  Need at least one and at most two.
 * 2) Don't check for options, as per SUSv3.
 * 3) Save some space by using strcmp().  Calling strncmp() here was silly.
 */

#include "libbb.h"

/* This is a NOFORK applet. Be very careful! */

int basename_main(int argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
int basename_main(int argc, char **argv)
{
	size_t m, n;
	char *s;

	if (((unsigned int)(argc-2)) >= 2) {
		bb_show_usage();
	}

	/* It should strip slash: /abc/def/ -> def */
	s = bb_get_last_path_component_strip(*++argv);

	m = strlen(s);
	if (*++argv) {
		n = strlen(*argv);
		if ((m > n) && ((strcmp)(s+m-n, *argv) == 0)) {
			m -= n;
			/*s[m] = '\0'; - redundant */
		}
	}

	/* puts(s) will do, but we can do without stdio this way: */
	s[m++] = '\n';
	return full_write(STDOUT_FILENO, s, m) == (ssize_t)m;
}
