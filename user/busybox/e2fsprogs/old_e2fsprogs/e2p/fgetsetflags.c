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
 * fgetflags.c		- Get a file flags on an ext2 file system
 * fsetflags.c		- Set a file flags on an ext2 file system
 *
 * Copyright (C) 1993, 1994  Remy Card <card@masi.ibp.fr>
 *                           Laboratoire MASI, Institut Blaise Pascal
 *                           Universite Pierre et Marie Curie (Paris VI)
 *
 * This file can be redistributed under the terms of the GNU Library General
 * Public License
 */

/*
 * History:
 * 93/10/30	- Creation
 */

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#ifdef HAVE_EXT2_IOCTLS
#include <fcntl.h>
#include <sys/ioctl.h>
#endif

#include "e2p.h"

#ifdef O_LARGEFILE
#define OPEN_FLAGS (O_RDONLY|O_NONBLOCK|O_LARGEFILE)
#else
#define OPEN_FLAGS (O_RDONLY|O_NONBLOCK)
#endif

int fgetsetflags (const char * name, unsigned long * get_flags, unsigned long set_flags)
{
#ifdef HAVE_EXT2_IOCTLS
	struct stat buf;
	int fd, r, f, save_errno = 0;

	if (!stat(name, &buf) &&
	    !S_ISREG(buf.st_mode) && !S_ISDIR(buf.st_mode)) {
		goto notsupp;
	}
	fd = open (name, OPEN_FLAGS);
	if (fd == -1)
		return -1;
	if (!get_flags) {
		f = (int) set_flags;
		r = ioctl (fd, EXT2_IOC_SETFLAGS, &f);
	} else {
		r = ioctl (fd, EXT2_IOC_GETFLAGS, &f);
		*get_flags = f;
	}
	if (r == -1)
		save_errno = errno;
	close (fd);
	if (save_errno)
		errno = save_errno;
	return r;
notsupp:
#endif /* HAVE_EXT2_IOCTLS */
	errno = EOPNOTSUPP;
	return -1;
}
