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
 * Mini chown implementation for busybox
 *
 * Copyright (C) 1999-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * Licensed under GPLv2 or later, see file LICENSE in this tarball for details.
 */

/* BB_AUDIT SUSv3 defects - none? */
/* BB_AUDIT GNU defects - unsupported long options. */
/* http://www.opengroup.org/onlinepubs/007904975/utilities/chown.html */

#include "libbb.h"

/* This is a NOEXEC applet. Be very careful! */


#define OPT_STR     ("Rh" USE_DESKTOP("vcfLHP"))
#define BIT_RECURSE 1
#define OPT_RECURSE (opt & 1)
#define OPT_NODEREF (opt & 2)
#define OPT_VERBOSE (USE_DESKTOP(opt & 0x04) SKIP_DESKTOP(0))
#define OPT_CHANGED (USE_DESKTOP(opt & 0x08) SKIP_DESKTOP(0))
#define OPT_QUIET   (USE_DESKTOP(opt & 0x10) SKIP_DESKTOP(0))
/* POSIX options
 * -L traverse every symbolic link to a directory encountered
 * -H if a command line argument is a symbolic link to a directory, traverse it
 * -P do not traverse any symbolic links (default)
 * We do not conform to the following:
 * "Specifying more than one of -H, -L, and -P is not an error.
 * The last option specified shall determine the behavior of the utility." */
/* -L */
#define BIT_TRAVERSE 0x20
#define OPT_TRAVERSE (USE_DESKTOP(opt & BIT_TRAVERSE) SKIP_DESKTOP(0))
/* -H or -L */
#define BIT_TRAVERSE_TOP (0x20|0x40)
#define OPT_TRAVERSE_TOP (USE_DESKTOP(opt & BIT_TRAVERSE_TOP) SKIP_DESKTOP(0))

typedef int (*chown_fptr)(const char *, uid_t, gid_t);

struct param_t {
	struct bb_uidgid_t ugid;
	chown_fptr chown_func;
};

static int FAST_FUNC fileAction(const char *fileName, struct stat *statbuf,
		void *vparam, int depth UNUSED_PARAM)
{
#define param  (*(struct param_t*)vparam)
#define opt option_mask32
	uid_t u = (param.ugid.uid == (uid_t)-1) ? statbuf->st_uid : param.ugid.uid;
	gid_t g = (param.ugid.gid == (gid_t)-1) ? statbuf->st_gid : param.ugid.gid;

	if (param.chown_func(fileName, u, g) == 0) {
		if (OPT_VERBOSE
		 || (OPT_CHANGED && (statbuf->st_uid != u || statbuf->st_gid != g))
		) {
			printf("changed ownership of '%s' to %u:%u\n",
					fileName, (unsigned)u, (unsigned)g);
		}
		return TRUE;
	}
	if (!OPT_QUIET)
		bb_simple_perror_msg(fileName);	/* A filename can have % in it... */
	return FALSE;
#undef opt
#undef param
}

int chown_main(int argc UNUSED_PARAM, char **argv)
{
	int retval = EXIT_SUCCESS;
	int opt, flags;
	struct param_t param;

	param.ugid.uid = -1;
	param.ugid.gid = -1;
	param.chown_func = chown;

	opt_complementary = "-2";
	opt = getopt32(argv, OPT_STR);
	argv += optind;

	/* This matches coreutils behavior (almost - see below) */
	if (OPT_NODEREF
	    /* || (OPT_RECURSE && !OPT_TRAVERSE_TOP): */
	    USE_DESKTOP( || (opt & (BIT_RECURSE|BIT_TRAVERSE_TOP)) == BIT_RECURSE)
	) {
		param.chown_func = lchown;
	}

	flags = ACTION_DEPTHFIRST; /* match coreutils order */
	if (OPT_RECURSE)
		flags |= ACTION_RECURSE;
	if (OPT_TRAVERSE_TOP)
		flags |= ACTION_FOLLOWLINKS_L0; /* -H/-L: follow links on depth 0 */
	if (OPT_TRAVERSE)
		flags |= ACTION_FOLLOWLINKS; /* follow links if -L */

	parse_chown_usergroup_or_die(&param.ugid, argv[0]);

	/* Ok, ready to do the deed now */
	argv++;
	do {
		if (!recursive_action(*argv,
				flags,          /* flags */
				fileAction,     /* file action */
				fileAction,     /* dir action */
				&param,         /* user data */
				0)              /* depth */
		) {
			retval = EXIT_FAILURE;
		}
	} while (*++argv);

	return retval;
}

/*
Testcase. Run in empty directory.

#!/bin/sh
t1="/tmp/busybox chown"
t2="/usr/bin/chown"
create() {
    rm -rf $1; mkdir $1
    (
    cd $1 || exit 1
    mkdir dir dir2
    >up
    >file
    >dir/file
    >dir2/file
    ln -s dir linkdir
    ln -s file linkfile
    ln -s ../up dir/linkup
    ln -s ../dir2 dir/linkupdir2
    )
    chown -R 0:0 $1
}
tst() {
    create test1
    create test2
    echo "[$1]" >>test1.out
    echo "[$1]" >>test2.out
    (cd test1; $t1 $1) >>test1.out 2>&1
    (cd test2; $t2 $1) >>test2.out 2>&1
    (cd test1; ls -lnR) >out1
    (cd test2; ls -lnR) >out2
    echo "chown $1" >out.diff
    if ! diff -u out1 out2 >>out.diff; then exit 1; fi
    rm out.diff
}
tst_for_each() {
    tst "$1 1:1 file"
    tst "$1 1:1 dir"
    tst "$1 1:1 linkdir"
    tst "$1 1:1 linkfile"
}
echo "If script produced 'out.diff' file, then at least one testcase failed"
>test1.out
>test2.out
# These match coreutils 6.8:
tst_for_each "-v"
tst_for_each "-vR"
tst_for_each "-vRP"
tst_for_each "-vRL"
tst_for_each "-vRH"
tst_for_each "-vh"
tst_for_each "-vhR"
tst_for_each "-vhRP"
tst_for_each "-vhRL"
tst_for_each "-vhRH"
# Fix `name' in coreutils output
sed 's/`/'"'"'/g' -i test2.out
# Compare us with coreutils output
diff -u test1.out test2.out

*/
