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
 * Mini copy_file implementation for busybox
 *
 * Copyright (C) 2001 by Matt Kraai <kraai@alumni.carnegiemellon.edu>
 * SELinux support by Yuichi Nakamura <ynakam@hitachisoft.jp>
 *
 * Licensed under GPLv2 or later, see file LICENSE in this tarball for details.
 *
 */

#include "libbb.h"

// POSIX: if exists and -i, ask (w/o -i assume yes).
// Then open w/o EXCL (yes, not unlink!).
// If open still fails and -f, try unlink, then try open again.
// Result: a mess:
// If dest is a softlink, we overwrite softlink's destination!
// (or fail, if it points to dir/nonexistent location/etc).
// This is strange, but POSIX-correct.
// coreutils cp has --remove-destination to override this...
//
// NB: we have special code which still allows for "cp file /dev/node"
// to work POSIX-ly (the only realistic case where it makes sense)

#define DO_POSIX_CP 0  /* 1 - POSIX behavior, 0 - safe behavior */

// errno must be set to relevant value ("why we cannot create dest?")
// for POSIX mode to give reasonable error message
static int ask_and_unlink(const char *dest, int flags)
{
	int e = errno;
#if DO_POSIX_CP
	if (!(flags & (FILEUTILS_FORCE|FILEUTILS_INTERACTIVE))) {
		// Either it exists, or the *path* doesnt exist
		bb_perror_msg("cannot create '%s'", dest);
		return -1;
	}
#endif
	// If !DO_POSIX_CP, act as if -f is always in effect - we don't want
	// "cannot create" msg, we want unlink to be done (silently unless -i).

	// TODO: maybe we should do it only if ctty is present?
	if (flags & FILEUTILS_INTERACTIVE) {
		// We would not do POSIX insanity. -i asks,
		// then _unlinks_ the offender. Presto.
		// (No "opening without O_EXCL", no "unlink only if -f")
		// Or else we will end up having 3 open()s!
		fprintf(stderr, "%s: overwrite '%s'? ", applet_name, dest);
		if (!bb_ask_confirmation())
			return 0; // not allowed to overwrite
	}
	if (unlink(dest) < 0) {
#if ENABLE_FEATURE_VERBOSE_CP_MESSAGE
		if (e == errno && e == ENOENT) {
			/* e == ENOTDIR is similar: path has non-dir component,
			 * but in this case we don't even reach copy_file() */
			bb_error_msg("cannot create '%s': Path does not exist", dest);
			return -1; // error
		}
#endif
		errno = e;
		bb_perror_msg("cannot create '%s'", dest);
		return -1; // error
	}
	return 1; // ok (to try again)
}

/* Return:
 * -1 error, copy not made
 *  0 copy is made or user answered "no" in interactive mode
 *    (failures to preserve mode/owner/times are not reported in exit code)
 */
int FAST_FUNC copy_file(const char *source, const char *dest, int flags)
{
	/* This is a recursive function, try to minimize stack usage */
	/* NB: each struct stat is ~100 bytes */
	struct stat source_stat;
	struct stat dest_stat;
	signed char retval = 0;
	signed char dest_exists = 0;
	signed char ovr;

/* Inverse of cp -d ("cp without -d") */
#define FLAGS_DEREF (flags & FILEUTILS_DEREFERENCE)

	if ((FLAGS_DEREF ? stat : lstat)(source, &source_stat) < 0) {
		// This may be a dangling symlink.
		// Making [sym]links to dangling symlinks works, so...
		if (flags & (FILEUTILS_MAKE_SOFTLINK|FILEUTILS_MAKE_HARDLINK))
			goto make_links;
		bb_perror_msg("cannot stat '%s'", source);
		return -1;
	}

	if (lstat(dest, &dest_stat) < 0) {
		if (errno != ENOENT) {
			bb_perror_msg("cannot stat '%s'", dest);
			return -1;
		}
	} else {
		if (source_stat.st_dev == dest_stat.st_dev
		 && source_stat.st_ino == dest_stat.st_ino
		) {
			bb_error_msg("'%s' and '%s' are the same file", source, dest);
			return -1;
		}
		dest_exists = 1;
	}

#if ENABLE_SELINUX
	if ((flags & FILEUTILS_PRESERVE_SECURITY_CONTEXT) && is_selinux_enabled() > 0) {
		security_context_t con;
		if (lgetfilecon(source, &con) >= 0) {
			if (setfscreatecon(con) < 0) {
				bb_perror_msg("cannot set setfscreatecon %s", con);
				freecon(con);
				return -1;
			}
		} else if (errno == ENOTSUP || errno == ENODATA) {
			setfscreatecon_or_die(NULL);
		} else {
			bb_perror_msg("cannot lgetfilecon %s", source);
			return -1;
		}
	}
#endif

	if (S_ISDIR(source_stat.st_mode)) {
		DIR *dp;
		const char *tp;
		struct dirent *d;
		mode_t saved_umask = 0;

		if (!(flags & FILEUTILS_RECUR)) {
			bb_error_msg("omitting directory '%s'", source);
			return -1;
		}

		/* Did we ever create source ourself before? */
		tp = is_in_ino_dev_hashtable(&source_stat);
		if (tp) {
			/* We did! it's a recursion! man the lifeboats... */
			bb_error_msg("recursion detected, omitting directory '%s'",
					source);
			return -1;
		}

		/* Create DEST */
		if (dest_exists) {
			if (!S_ISDIR(dest_stat.st_mode)) {
				bb_error_msg("target '%s' is not a directory", dest);
				return -1;
			}
			/* race here: user can substitute a symlink between
			 * this check and actual creation of files inside dest */
		} else {
			mode_t mode;
			saved_umask = umask(0);

			mode = source_stat.st_mode;
			if (!(flags & FILEUTILS_PRESERVE_STATUS))
				mode = source_stat.st_mode & ~saved_umask;
			/* Allow owner to access new dir (at least for now) */
			mode |= S_IRWXU;
			if (mkdir(dest, mode) < 0) {
				umask(saved_umask);
				bb_perror_msg("cannot create directory '%s'", dest);
				return -1;
			}
			umask(saved_umask);
			/* need stat info for add_to_ino_dev_hashtable */
			if (lstat(dest, &dest_stat) < 0) {
				bb_perror_msg("cannot stat '%s'", dest);
				return -1;
			}
		}
		/* remember (dev,inode) of each created dir.
		 * NULL: name is not remembered */
		add_to_ino_dev_hashtable(&dest_stat, NULL);

		/* Recursively copy files in SOURCE */
		dp = opendir(source);
		if (dp == NULL) {
			retval = -1;
			goto preserve_mode_ugid_time;
		}

		while ((d = readdir(dp)) != NULL) {
			char *new_source, *new_dest;

			new_source = concat_subpath_file(source, d->d_name);
			if (new_source == NULL)
				continue;
			new_dest = concat_path_file(dest, d->d_name);
			if (copy_file(new_source, new_dest, flags) < 0)
				retval = -1;
			free(new_source);
			free(new_dest);
		}
		closedir(dp);

		if (!dest_exists
		 && chmod(dest, source_stat.st_mode & ~saved_umask) < 0
		) {
			bb_perror_msg("cannot preserve %s of '%s'", "permissions", dest);
			/* retval = -1; - WRONG! copy *WAS* made */
		}
		goto preserve_mode_ugid_time;
	}

	if (flags & (FILEUTILS_MAKE_SOFTLINK|FILEUTILS_MAKE_HARDLINK)) {
		int (*lf)(const char *oldpath, const char *newpath);
 make_links:
		// Hmm... maybe
		// if (DEREF && MAKE_SOFTLINK) source = realpath(source) ?
		// (but realpath returns NULL on dangling symlinks...)
		lf = (flags & FILEUTILS_MAKE_SOFTLINK) ? symlink : link;
		if (lf(source, dest) < 0) {
			ovr = ask_and_unlink(dest, flags);
			if (ovr <= 0)
				return ovr;
			if (lf(source, dest) < 0) {
				bb_perror_msg("cannot create link '%s'", dest);
				return -1;
			}
		}
		/* _Not_ jumping to preserve_mode_ugid_time:
		 * hard/softlinks don't have those */
		return 0;
	}

	if (/* "cp thing1 thing2" without -R: just open and read() from thing1 */
	    !(flags & FILEUTILS_RECUR)
	    /* "cp [-opts] regular_file thing2" */
	 || S_ISREG(source_stat.st_mode)
	 /* DEREF uses stat, which never returns S_ISLNK() == true.
	  * So the below is never true: */
	 /* || (FLAGS_DEREF && S_ISLNK(source_stat.st_mode)) */
	) {
		int src_fd;
		int dst_fd;
		mode_t new_mode;

		if (!FLAGS_DEREF && S_ISLNK(source_stat.st_mode)) {
			/* "cp -d symlink dst": create a link */
			goto dont_cat;
		}

		if (ENABLE_FEATURE_PRESERVE_HARDLINKS && !FLAGS_DEREF) {
			const char *link_target;
			link_target = is_in_ino_dev_hashtable(&source_stat);
			if (link_target) {
				if (link(link_target, dest) < 0) {
					ovr = ask_and_unlink(dest, flags);
					if (ovr <= 0)
						return ovr;
					if (link(link_target, dest) < 0) {
						bb_perror_msg("cannot create link '%s'", dest);
						return -1;
					}
				}
				return 0;
			}
			add_to_ino_dev_hashtable(&source_stat, dest);
		}

		src_fd = open_or_warn(source, O_RDONLY);
		if (src_fd < 0)
			return -1;

		/* Do not try to open with weird mode fields */
		new_mode = source_stat.st_mode;
		if (!S_ISREG(source_stat.st_mode))
			new_mode = 0666;

		/* POSIX way is a security problem versus symlink attacks,
		 * we do it only for non-symlinks, and only for non-recursive,
		 * non-interactive cp. NB: it is still racy
		 * for "cp file /home/bad_user/file" case
		 * (user can rm file and create a link to /etc/passwd) */
		if (DO_POSIX_CP
		 || (dest_exists
		     && !(flags & (FILEUTILS_RECUR|FILEUTILS_INTERACTIVE))
		     && !S_ISLNK(dest_stat.st_mode))
		) {
			dst_fd = open(dest, O_WRONLY|O_CREAT|O_TRUNC, new_mode);
		} else  /* safe way: */
			dst_fd = open(dest, O_WRONLY|O_CREAT|O_EXCL, new_mode);
		if (dst_fd == -1) {
			ovr = ask_and_unlink(dest, flags);
			if (ovr <= 0) {
				close(src_fd);
				return ovr;
			}
			/* It shouldn't exist. If it exists, do not open (symlink attack?) */
			dst_fd = open3_or_warn(dest, O_WRONLY|O_CREAT|O_EXCL, new_mode);
			if (dst_fd < 0) {
				close(src_fd);
				return -1;
			}
		}

#if ENABLE_SELINUX
		if ((flags & (FILEUTILS_PRESERVE_SECURITY_CONTEXT|FILEUTILS_SET_SECURITY_CONTEXT))
		 && is_selinux_enabled() > 0
		) {
			security_context_t con;
			if (getfscreatecon(&con) == -1) {
				bb_perror_msg("getfscreatecon");
				return -1;
			}
			if (con) {
				if (setfilecon(dest, con) == -1) {
					bb_perror_msg("setfilecon:%s,%s", dest, con);
					freecon(con);
					return -1;
				}
				freecon(con);
			}
		}
#endif
		if (bb_copyfd_eof(src_fd, dst_fd) == -1)
			retval = -1;
		/* Ok, writing side I can understand... */
		if (close(dst_fd) < 0) {
			bb_perror_msg("cannot close '%s'", dest);
			retval = -1;
		}
		/* ...but read size is already checked by bb_copyfd_eof */
		close(src_fd);
		/* "cp /dev/something new_file" should not
		 * copy mode of /dev/something */
		if (!S_ISREG(source_stat.st_mode))
			return retval;
		goto preserve_mode_ugid_time;
	}
 dont_cat:

	/* Source is a symlink or a special file */
	/* We are lazy here, a bit lax with races... */
	if (dest_exists) {
		errno = EEXIST;
		ovr = ask_and_unlink(dest, flags);
		if (ovr <= 0)
			return ovr;
	}
	if (S_ISLNK(source_stat.st_mode)) {
		char *lpath = xmalloc_readlink_or_warn(source);
		if (lpath) {
			int r = symlink(lpath, dest);
			free(lpath);
			if (r < 0) {
				bb_perror_msg("cannot create symlink '%s'", dest);
				return -1;
			}
			if (flags & FILEUTILS_PRESERVE_STATUS)
				if (lchown(dest, source_stat.st_uid, source_stat.st_gid) < 0)
					bb_perror_msg("cannot preserve %s of '%s'", "ownership", dest);
		}
		/* _Not_ jumping to preserve_mode_ugid_time:
		 * symlinks don't have those */
		return 0;
	}
	if (S_ISBLK(source_stat.st_mode) || S_ISCHR(source_stat.st_mode)
	 || S_ISSOCK(source_stat.st_mode) || S_ISFIFO(source_stat.st_mode)
	) {
		if (mknod(dest, source_stat.st_mode, source_stat.st_rdev) < 0) {
			bb_perror_msg("cannot create '%s'", dest);
			return -1;
		}
	} else {
		bb_error_msg("unrecognized file '%s' with mode %x", source, source_stat.st_mode);
		return -1;
	}

 preserve_mode_ugid_time:

	if (flags & FILEUTILS_PRESERVE_STATUS
	/* Cannot happen: */
	/* && !(flags & (FILEUTILS_MAKE_SOFTLINK|FILEUTILS_MAKE_HARDLINK)) */
	) {
		struct utimbuf times;

		times.actime = source_stat.st_atime;
		times.modtime = source_stat.st_mtime;
		/* BTW, utimes sets usec-precision time - just FYI */
		if (utime(dest, &times) < 0)
			bb_perror_msg("cannot preserve %s of '%s'", "times", dest);
		if (chown(dest, source_stat.st_uid, source_stat.st_gid) < 0) {
			source_stat.st_mode &= ~(S_ISUID | S_ISGID);
			bb_perror_msg("cannot preserve %s of '%s'", "ownership", dest);
		}
		if (chmod(dest, source_stat.st_mode) < 0)
			bb_perror_msg("cannot preserve %s of '%s'", "permissions", dest);
	}

	return retval;
}
