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
 * Mini run-parts implementation for busybox
 *
 * Copyright (C) 2007 Bernhard Fischer
 *
 * Based on a older version that was in busybox which was 1k big..
 *   Copyright (C) 2001 by Emanuele Aina <emanuele.aina@tiscali.it>
 *
 * Based on the Debian run-parts program, version 1.15
 *   Copyright (C) 1996 Jeff Noxon <jeff@router.patch.net>,
 *   Copyright (C) 1996-1999 Guy Maor <maor@debian.org>
 *
 *
 * Licensed under GPL v2 or later, see file LICENSE in this tarball for details.
 */

/* This is my first attempt to write a program in C (well, this is my first
 * attempt to write a program! :-) . */

/* This piece of code is heavily based on the original version of run-parts,
 * taken from debian-utils. I've only removed the long options and a the
 * report mode. As the original run-parts support only long options, I've
 * broken compatibility because the BusyBox policy doesn't allow them.
 * The supported options are:
 * -t           test. Print the name of the files to be executed, without
 *              execute them.
 * -a ARG       argument. Pass ARG as an argument the program executed. It can
 *              be repeated to pass multiple arguments.
 * -u MASK      umask. Set the umask of the program executed to MASK.
 */

#include "libbb.h"

struct globals {
	char **names;
	int    cur;
	char  *cmd[1];
};
#define G (*(struct globals*)&bb_common_bufsiz1)
#define names (G.names)
#define cur   (G.cur  )
#define cmd   (G.cmd  )

enum { NUM_CMD = (COMMON_BUFSIZE - sizeof(G)) / sizeof(cmd[0]) - 1 };

enum {
	OPT_r = (1 << 0),
	OPT_a = (1 << 1),
	OPT_u = (1 << 2),
	OPT_t = (1 << 3),
	OPT_l = (1 << 4) * ENABLE_FEATURE_RUN_PARTS_FANCY,
};

#if ENABLE_FEATURE_RUN_PARTS_FANCY
#define list_mode (option_mask32 & OPT_l)
#else
#define list_mode 0
#endif

/* Is this a valid filename (upper/lower alpha, digits,
 * underscores, and hyphens only?)
 */
static bool invalid_name(const char *c)
{
	c = bb_basename(c);

	while (*c && (isalnum(*c) || *c == '_' || *c == '-'))
		c++;

	return *c; /* TRUE (!0) if terminating NUL is not reached */
}

static int bb_alphasort(const void *p1, const void *p2)
{
	int r = strcmp(*(char **) p1, *(char **) p2);
	return (option_mask32 & OPT_r) ? -r : r;
}

static int FAST_FUNC act(const char *file, struct stat *statbuf, void *args UNUSED_PARAM, int depth)
{
	if (depth == 1)
		return TRUE;

	if (depth == 2
	 && (  !(statbuf->st_mode & (S_IFREG | S_IFLNK))
	    || invalid_name(file)
	    || (!list_mode && access(file, X_OK) != 0))
	) {
		return SKIP;
	}

	names = xrealloc_vector(names, 4, cur);
	names[cur++] = xstrdup(file);
	/*names[cur] = NULL; - xrealloc_vector did it */

	return TRUE;
}

#if ENABLE_FEATURE_RUN_PARTS_LONG_OPTIONS
static const char runparts_longopts[] ALIGN1 =
	"arg\0"     Required_argument "a"
	"umask\0"   Required_argument "u"
	"test\0"    No_argument       "t"
#if ENABLE_FEATURE_RUN_PARTS_FANCY
	"list\0"    No_argument       "l"
	"reverse\0" No_argument       "r"
//TODO: "verbose\0" No_argument       "v"
#endif
	;
#endif

int run_parts_main(int argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
int run_parts_main(int argc UNUSED_PARAM, char **argv)
{
	const char *umask_p = "22";
	llist_t *arg_list = NULL;
	unsigned n;
	int ret;

#if ENABLE_FEATURE_RUN_PARTS_LONG_OPTIONS
	applet_long_options = runparts_longopts;
#endif
	/* We require exactly one argument: the directory name */
	/* We require exactly one argument: the directory name */
	opt_complementary = "=1:a::";
	getopt32(argv, "ra:u:t"USE_FEATURE_RUN_PARTS_FANCY("l"), &arg_list, &umask_p);

	umask(xstrtou_range(umask_p, 8, 0, 07777));

	n = 1;
	while (arg_list && n < NUM_CMD) {
		cmd[n++] = llist_pop(&arg_list);
	}
	/* cmd[n] = NULL; - is already zeroed out */

	/* run-parts has to sort executables by name before running them */

	recursive_action(argv[optind],
			ACTION_RECURSE|ACTION_FOLLOWLINKS,
			act,            /* file action */
			act,            /* dir action */
			NULL,           /* user data */
			1               /* depth */
		);

	if (!names)
		return 0;

	qsort(names, cur, sizeof(char *), bb_alphasort);

	n = 0;
	while (1) {
		char *name = *names++;
		if (!name)
			break;
		if (option_mask32 & (OPT_t | OPT_l)) {
			puts(name);
			continue;
		}
		cmd[0] = name;
		ret = wait4pid(spawn(cmd));
		if (ret == 0)
			continue;
		n = 1;
		if (ret < 0)
			bb_perror_msg("can't exec %s", name);
		else /* ret > 0 */
			bb_error_msg("%s exited with code %d", name, ret);
	}

	return n;
}
