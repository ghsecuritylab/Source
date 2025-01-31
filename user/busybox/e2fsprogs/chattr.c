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
 * chattr.c		- Change file attributes on an ext2 file system
 *
 * Copyright (C) 1993, 1994  Remy Card <card@masi.ibp.fr>
 *                           Laboratoire MASI, Institut Blaise Pascal
 *                           Universite Pierre et Marie Curie (Paris VI)
 *
 * This file can be redistributed under the terms of the GNU General
 * Public License
 */

/*
 * History:
 * 93/10/30	- Creation
 * 93/11/13	- Replace stat() calls by lstat() to avoid loops
 * 94/02/27	- Integrated in Ted's distribution
 * 98/12/29	- Ignore symlinks when working recursively (G M Sipe)
 * 98/12/29	- Display version info only when -V specified (G M Sipe)
 */

#include "libbb.h"
#include "e2fs_lib.h"

#define OPT_ADD 1
#define OPT_REM 2
#define OPT_SET 4
#define OPT_SET_VER 8

struct globals {
	unsigned long version;
	unsigned long af;
	unsigned long rf;
	smallint flags;
	smallint recursive;
};

static unsigned long get_flag(char c)
{
	const char *fp = strchr(e2attr_flags_sname_chattr, c);
	if (fp)
		return e2attr_flags_value_chattr[fp - e2attr_flags_sname_chattr];
	bb_show_usage();
}

static int decode_arg(const char *arg, struct globals *gp)
{
	unsigned long *fl;
	char opt = *arg++;

	fl = &gp->af;
	if (opt == '-') {
		gp->flags |= OPT_REM;
		fl = &gp->rf;
	} else if (opt == '+') {
		gp->flags |= OPT_ADD;
	} else if (opt == '=') {
		gp->flags |= OPT_SET;
	} else
		return 0;

	while (*arg)
		*fl |= get_flag(*arg++);

	return 1;
}

static void change_attributes(const char *name, struct globals *gp);

static int chattr_dir_proc(const char *dir_name, struct dirent *de, void *gp)
{
	char *path = concat_subpath_file(dir_name, de->d_name);
	/* path is NULL if de->d_name is "." or "..", else... */
	if (path) {
		change_attributes(path, gp);
		free(path);
	}
	return 0;
}

static void change_attributes(const char *name, struct globals *gp)
{
	unsigned long fsflags;
	struct stat st;

	if (lstat(name, &st) != 0) {
		bb_perror_msg("stat %s", name);
		return;
	}
	if (S_ISLNK(st.st_mode) && gp->recursive)
		return;

	/* Don't try to open device files, fifos etc.  We probably
	 * ought to display an error if the file was explicitly given
	 * on the command line (whether or not recursive was
	 * requested).  */
	if (!S_ISREG(st.st_mode) && !S_ISLNK(st.st_mode) && !S_ISDIR(st.st_mode))
		return;

	if (gp->flags & OPT_SET_VER)
		if (fsetversion(name, gp->version) != 0)
			bb_perror_msg("setting version on %s", name);

	if (gp->flags & OPT_SET) {
		fsflags = gp->af;
	} else {
		if (fgetflags(name, &fsflags) != 0) {
			bb_perror_msg("reading flags on %s", name);
			goto skip_setflags;
		}
		/*if (gp->flags & OPT_REM) - not needed, rf is zero otherwise */
			fsflags &= ~gp->rf;
		/*if (gp->flags & OPT_ADD) - not needed, af is zero otherwise */
			fsflags |= gp->af;
		/* What is this? And why it's not done for SET case? */
		if (!S_ISDIR(st.st_mode))
			fsflags &= ~EXT2_DIRSYNC_FL;
	}
	if (fsetflags(name, fsflags) != 0)
		bb_perror_msg("setting flags on %s", name);

 skip_setflags:
	if (gp->recursive && S_ISDIR(st.st_mode))
		iterate_on_dir(name, chattr_dir_proc, gp);
}

int chattr_main(int argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
int chattr_main(int argc UNUSED_PARAM, char **argv)
{
	struct globals g;
	char *arg;

	memset(&g, 0, sizeof(g));

	/* parse the args */
	while ((arg = *++argv)) {
		/* take care of -R and -v <version> */
		if (arg[0] == '-'
		 && (arg[1] == 'R' || arg[1] == 'v')
		 && !arg[2]
		) {
			if (arg[1] == 'R') {
				g.recursive = 1;
				continue;
			}
			/* arg[1] == 'v' */
			if (!*++argv)
				bb_show_usage();
			g.version = xatoul(*argv);
			g.flags |= OPT_SET_VER;
			continue;
		}

		if (!decode_arg(arg, &g))
			break;
	}

	/* run sanity checks on all the arguments given us */
	if (!*argv)
		bb_show_usage();
	if ((g.flags & OPT_SET) && (g.flags & (OPT_ADD|OPT_REM)))
		bb_error_msg_and_die("= is incompatible with - and +");
	if (g.rf & g.af)
		bb_error_msg_and_die("can't set and unset a flag");
	if (!g.flags)
		bb_error_msg_and_die("must use '-v', =, - or +");

	/* now run chattr on all the files passed to us */
	do change_attributes(*argv, &g); while (*++argv);

	return EXIT_SUCCESS;
}
