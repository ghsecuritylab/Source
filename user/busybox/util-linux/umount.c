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
 * Mini umount implementation for busybox
 *
 * Copyright (C) 1999-2004 by Erik Andersen <andersen@codepoet.org>
 * Copyright (C) 2005 by Rob Landley <rob@landley.net>
 *
 * Licensed under GPL version 2, see file LICENSE in this tarball for details.
 */

#include <mntent.h>
#include "libbb.h"

#if defined(__dietlibc__)
/* 16.12.2006, Sampo Kellomaki (sampo@iki.fi)
 * dietlibc-0.30 does not have implementation of getmntent_r() */
static struct mntent *getmntent_r(FILE* stream, struct mntent* result,
		char* buffer UNUSED_PARAM, int bufsize UNUSED_PARAM)
{
	struct mntent* ment = getmntent(stream);
	return memcpy(result, ment, sizeof(*ment));
}
#endif

/* ignored: -v -d -t -i */
#define OPTION_STRING           "fldnra" "vdt:i"
#define OPT_FORCE               (1 << 0)
#define OPT_LAZY                (1 << 1)
#define OPT_FREELOOP            (1 << 2)
#define OPT_NO_MTAB             (1 << 3)
#define OPT_REMOUNT             (1 << 4)
#define OPT_ALL                 (ENABLE_FEATURE_UMOUNT_ALL ? (1 << 5) : 0)

// These constants from linux/fs.h must match OPT_FORCE and OPT_LAZY,
// otherwise "doForce" trick below won't work!
//#define MNT_FORCE  0x00000001 /* Attempt to forcibly umount */
//#define MNT_DETACH 0x00000002 /* Just detach from the tree */

int umount_main(int argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
int umount_main(int argc UNUSED_PARAM, char **argv)
{
	int doForce;
	char *const path = xmalloc(PATH_MAX + 2); /* to save stack */
	struct mntent me;
	FILE *fp;
	char *fstype = NULL;
	int status = EXIT_SUCCESS;
	unsigned opt;
	struct mtab_list {
		char *dir;
		char *device;
		struct mtab_list *next;
	} *mtl, *m;

	opt = getopt32(argv, OPTION_STRING, &fstype);
	//argc -= optind;
	argv += optind;
	doForce = MAX((opt & OPT_FORCE), (opt & OPT_LAZY));

	/* Get a list of mount points from mtab.  We read them all in now mostly
	 * for umount -a (so we don't have to worry about the list changing while
	 * we iterate over it, or about getting stuck in a loop on the same failing
	 * entry.  Notice that this also naturally reverses the list so that -a
	 * umounts the most recent entries first. */
	m = mtl = NULL;

	// If we're umounting all, then m points to the start of the list and
	// the argument list should be empty (which will match all).
	fp = setmntent(bb_path_mtab_file, "r");
	if (!fp) {
		if (opt & OPT_ALL)
			bb_error_msg_and_die("can't open %s", bb_path_mtab_file);
	} else {
		while (getmntent_r(fp, &me, path, PATH_MAX)) {
			/* Match fstype if passed */
			if (fstype && match_fstype(&me, fstype))
				continue;
			m = xmalloc(sizeof(struct mtab_list));
			m->next = mtl;
			m->device = xstrdup(me.mnt_fsname);
			m->dir = xstrdup(me.mnt_dir);
			mtl = m;
		}
		endmntent(fp);
	}

	// If we're not umounting all, we need at least one argument.
	if (!(opt & OPT_ALL) && !fstype) {
		if (!argv[0])
			bb_show_usage();
		m = NULL;
	}

	// Loop through everything we're supposed to umount, and do so.
	for (;;) {
		int curstat;
		char *zapit = *argv;

		// Do we already know what to umount this time through the loop?
		if (m)
			safe_strncpy(path, m->dir, PATH_MAX);
		// For umount -a, end of mtab means time to exit.
		else if (opt & OPT_ALL)
			break;
		// Use command line argument (and look it up in mtab list)
		else {
			if (!zapit)
				break;
			argv++;
			realpath(zapit, path);
			for (m = mtl; m; m = m->next)
				if (!strcmp(path, m->dir) || !strcmp(path, m->device))
					break;
		}
		// If we couldn't find this sucker in /etc/mtab, punt by passing our
		// command line argument straight to the umount syscall.  Otherwise,
		// umount the directory even if we were given the block device.
		if (m) zapit = m->dir;

		// Let's ask the thing nicely to unmount.
		curstat = umount(zapit);

		// Force the unmount, if necessary.
		if (curstat && doForce)
			curstat = umount2(zapit, doForce);

		// If still can't umount, maybe remount read-only?
		if (curstat) {
			if ((opt & OPT_REMOUNT) && errno == EBUSY && m) {
				// Note! Even if we succeed here, later we should not
				// free loop device or erase mtab entry!
				const char *msg = "%s busy - remounted read-only";
				curstat = mount(m->device, zapit, NULL, MS_REMOUNT|MS_RDONLY, NULL);
				if (curstat) {
					msg = "can't remount %s read-only";
					status = EXIT_FAILURE;
				}
				bb_error_msg(msg, m->device);
			} else {
				status = EXIT_FAILURE;
				bb_perror_msg("can't %sumount %s", (doForce ? "forcibly " : ""), zapit);
			}
		} else {
			// De-allocate the loop device.  This ioctl should be ignored on
			// any non-loop block devices.
			if (ENABLE_FEATURE_MOUNT_LOOP && (opt & OPT_FREELOOP) && m)
				del_loop(m->device);
			if (ENABLE_FEATURE_MTAB_SUPPORT && !(opt & OPT_NO_MTAB) && m)
				erase_mtab(m->dir);
		}

		// Find next matching mtab entry for -a or umount /dev
		// Note this means that "umount /dev/blah" will unmount all instances
		// of /dev/blah, not just the most recent.
		if (m) while ((m = m->next) != NULL)
			if ((opt & OPT_ALL) || !strcmp(path, m->device))
				break;
	}

	// Free mtab list if necessary
	if (ENABLE_FEATURE_CLEAN_UP) {
		while (mtl) {
			m = mtl->next;
			free(mtl->device);
			free(mtl->dir);
			free(mtl);
			mtl = m;
		}
		free(path);
	}

	return status;
}
