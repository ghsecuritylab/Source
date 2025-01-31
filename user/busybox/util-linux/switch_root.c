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
/* Copyright 2005 Rob Landley <rob@landley.net>
 *
 * Switch from rootfs to another filesystem as the root of the mount tree.
 *
 * Licensed under GPL version 2, see file LICENSE in this tarball for details.
 */

#include "libbb.h"
#include <sys/vfs.h>

// Make up for header deficiencies.
#ifndef RAMFS_MAGIC
#define RAMFS_MAGIC ((unsigned)0x858458f6)
#endif

#ifndef TMPFS_MAGIC
#define TMPFS_MAGIC ((unsigned)0x01021994)
#endif

#ifndef MS_MOVE
#define MS_MOVE     8192
#endif

// Recursively delete contents of rootfs.
static void delete_contents(const char *directory, dev_t rootdev)
{
	DIR *dir;
	struct dirent *d;
	struct stat st;

	// Don't descend into other filesystems
	if (lstat(directory, &st) || st.st_dev != rootdev)
		return;

	// Recursively delete the contents of directories.
	if (S_ISDIR(st.st_mode)) {
		dir = opendir(directory);
		if (dir) {
			while ((d = readdir(dir))) {
				char *newdir = d->d_name;

				// Skip . and ..
				if (DOT_OR_DOTDOT(newdir))
					continue;

				// Recurse to delete contents
				newdir = concat_path_file(directory, newdir);
				delete_contents(newdir, rootdev);
				free(newdir);
			}
			closedir(dir);

			// Directory should now be empty.  Zap it.
			rmdir(directory);
		}

	// It wasn't a directory.  Zap it.
	} else unlink(directory);
}

int switch_root_main(int argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
int switch_root_main(int argc UNUSED_PARAM, char **argv)
{
	char *newroot, *console = NULL;
	struct stat st1, st2;
	struct statfs stfs;
	dev_t rootdev;

	// Parse args (-c console)
	opt_complementary = "-2"; // minimum 2 params
	getopt32(argv, "+c:", &console); // '+': stop parsing at first non-option
	argv += optind;

	// Change to new root directory and verify it's a different fs.
	newroot = *argv++;

	xchdir(newroot);
	if (lstat(".", &st1) || lstat("/", &st2) || st1.st_dev == st2.st_dev) {
		bb_error_msg_and_die("bad newroot %s", newroot);
	}
	rootdev = st2.st_dev;

	// Additional sanity checks: we're about to rm -rf /,  so be REALLY SURE
	// we mean it.  (I could make this a CONFIG option, but I would get email
	// from all the people who WILL eat their filesystems.)
	if (lstat("/init", &st1) || !S_ISREG(st1.st_mode) || statfs("/", &stfs)
	 || (((unsigned)stfs.f_type != RAMFS_MAGIC) && ((unsigned)stfs.f_type != TMPFS_MAGIC))
	 || (getpid() != 1)
	) {
		bb_error_msg_and_die("not rootfs");
	}

	// Zap everything out of rootdev
	delete_contents("/", rootdev);

	// Overmount / with newdir and chroot into it.  The chdir is needed to
	// recalculate "." and ".." links.
	if (mount(".", "/", NULL, MS_MOVE, NULL))
		bb_error_msg_and_die("error moving root");
	xchroot(".");
	xchdir("/");

	// If a new console specified, redirect stdin/stdout/stderr to that.
	if (console) {
		close(0);
		xopen(console, O_RDWR);
		xdup2(0, 1);
		xdup2(0, 2);
	}

	// Exec real init.  (This is why we must be pid 1.)
	execv(argv[0], argv);
	bb_perror_msg_and_die("bad init %s", argv[0]);
}
