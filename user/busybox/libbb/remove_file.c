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
 * Mini remove_file implementation for busybox
 *
 * Copyright (C) 2001 Matt Kraai <kraai@alumni.carnegiemellon.edu>
 *
 * Licensed under GPLv2 or later, see file LICENSE in this tarball for details.
 */

#include "libbb.h"

/* Used from NOFORK applets. Must not allocate anything */

int FAST_FUNC remove_file(const char *path, int flags)
{
	struct stat path_stat;

	if (lstat(path, &path_stat) < 0) {
		if (errno != ENOENT) {
			bb_perror_msg("cannot stat '%s'", path);
			return -1;
		}
		if (!(flags & FILEUTILS_FORCE)) {
			bb_perror_msg("cannot remove '%s'", path);
			return -1;
		}
		return 0;
	}

	if (S_ISDIR(path_stat.st_mode)) {
		DIR *dp;
		struct dirent *d;
		int status = 0;

		if (!(flags & FILEUTILS_RECUR)) {
			bb_error_msg("%s: is a directory", path);
			return -1;
		}

		if ((!(flags & FILEUTILS_FORCE) && access(path, W_OK) < 0 && isatty(0))
		 || (flags & FILEUTILS_INTERACTIVE)
		) {
			fprintf(stderr, "%s: descend into directory '%s'? ", applet_name,
					path);
			if (!bb_ask_confirmation())
				return 0;
		}

		dp = opendir(path);
		if (dp == NULL) {
			return -1;
		}

		while ((d = readdir(dp)) != NULL) {
			char *new_path;

			new_path = concat_subpath_file(path, d->d_name);
			if (new_path == NULL)
				continue;
			if (remove_file(new_path, flags) < 0)
				status = -1;
			free(new_path);
		}

		if (closedir(dp) < 0) {
			bb_perror_msg("cannot close '%s'", path);
			return -1;
		}

		if (flags & FILEUTILS_INTERACTIVE) {
			fprintf(stderr, "%s: remove directory '%s'? ", applet_name, path);
			if (!bb_ask_confirmation())
				return status;
		}

		if (rmdir(path) < 0) {
			bb_perror_msg("cannot remove '%s'", path);
			return -1;
		}

		return status;
	}

	/* !ISDIR */
	if ((!(flags & FILEUTILS_FORCE)
	     && access(path, W_OK) < 0
	     && !S_ISLNK(path_stat.st_mode)
	     && isatty(0))
	 || (flags & FILEUTILS_INTERACTIVE)
	) {
		fprintf(stderr, "%s: remove '%s'? ", applet_name, path);
		if (!bb_ask_confirmation())
			return 0;
	}

	if (unlink(path) < 0) {
		bb_perror_msg("cannot remove '%s'", path);
		return -1;
	}

	return 0;
}
