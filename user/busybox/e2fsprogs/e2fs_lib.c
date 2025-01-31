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
 * See README for additional information
 *
 * This file can be redistributed under the terms of the GNU Library General
 * Public License
 */

#include "libbb.h"
#include "e2fs_lib.h"

#define HAVE_EXT2_IOCTLS 1

#if INT_MAX == LONG_MAX
#define IF_LONG_IS_SAME(...) __VA_ARGS__
#define IF_LONG_IS_WIDER(...)
#else
#define IF_LONG_IS_SAME(...)
#define IF_LONG_IS_WIDER(...) __VA_ARGS__
#endif

static void close_silently(int fd)
{
	int e = errno;
	close(fd);
	errno = e;
}


/* Iterate a function on each entry of a directory */
int iterate_on_dir(const char *dir_name,
		int (*func)(const char *, struct dirent *, void *),
		void * private)
{
	DIR *dir;
	struct dirent *de, *dep;
	int max_len, len;

	max_len = PATH_MAX + sizeof(struct dirent);
	de = xmalloc(max_len+1);
	memset(de, 0, max_len+1);

	dir = opendir(dir_name);
	if (dir == NULL) {
		free(de);
		return -1;
	}
	while ((dep = readdir(dir))) {
		len = sizeof(struct dirent);
		if (len < dep->d_reclen)
			len = dep->d_reclen;
		if (len > max_len)
			len = max_len;
		memcpy(de, dep, len);
		func(dir_name, de, private);
	}
	closedir(dir);
	free(de);
	return 0;
}


/* Get/set a file version on an ext2 file system */
int fgetsetversion(const char *name, unsigned long *get_version, unsigned long set_version)
{
#if HAVE_EXT2_IOCTLS
	int fd, r;
	IF_LONG_IS_WIDER(int ver;)

	fd = open(name, O_NONBLOCK);
	if (fd == -1)
		return -1;
	if (!get_version) {
		IF_LONG_IS_WIDER(
			ver = (int) set_version;
			r = ioctl(fd, EXT2_IOC_SETVERSION, &ver);
		)
		IF_LONG_IS_SAME(
			r = ioctl(fd, EXT2_IOC_SETVERSION, (void*)&set_version);
		)
	} else {
		IF_LONG_IS_WIDER(
			r = ioctl(fd, EXT2_IOC_GETVERSION, &ver);
			*get_version = ver;
		)
		IF_LONG_IS_SAME(
			r = ioctl(fd, EXT2_IOC_GETVERSION, (void*)get_version);
		)
	}
	close_silently(fd);
	return r;
#else /* ! HAVE_EXT2_IOCTLS */
	errno = EOPNOTSUPP;
	return -1;
#endif /* ! HAVE_EXT2_IOCTLS */
}


/* Get/set a file flags on an ext2 file system */
int fgetsetflags(const char *name, unsigned long *get_flags, unsigned long set_flags)
{
#if HAVE_EXT2_IOCTLS
	struct stat buf;
	int fd, r;
	IF_LONG_IS_WIDER(int f;)

	if (stat(name, &buf) == 0 /* stat is ok */
	 && !S_ISREG(buf.st_mode) && !S_ISDIR(buf.st_mode)
	) {
		goto notsupp;
	}
	fd = open(name, O_NONBLOCK); /* neither read nor write asked for */
	if (fd == -1)
		return -1;

	if (!get_flags) {
		IF_LONG_IS_WIDER(
			f = (int) set_flags;
			r = ioctl(fd, EXT2_IOC_SETFLAGS, &f);
		)
		IF_LONG_IS_SAME(
			r = ioctl(fd, EXT2_IOC_SETFLAGS, (void*)&set_flags);
		)
	} else {
		IF_LONG_IS_WIDER(
			r = ioctl(fd, EXT2_IOC_GETFLAGS, &f);
			*get_flags = f;
		)
		IF_LONG_IS_SAME(
			r = ioctl(fd, EXT2_IOC_GETFLAGS, (void*)get_flags);
		)
	}

	close_silently(fd);
	return r;
 notsupp:
#endif /* HAVE_EXT2_IOCTLS */
	errno = EOPNOTSUPP;
	return -1;
}


/* Print file attributes on an ext2 file system */
const uint32_t e2attr_flags_value[] = {
#ifdef ENABLE_COMPRESSION
	EXT2_COMPRBLK_FL,
	EXT2_DIRTY_FL,
	EXT2_NOCOMPR_FL,
	EXT2_ECOMPR_FL,
#endif
	EXT2_INDEX_FL,
	EXT2_SECRM_FL,
	EXT2_UNRM_FL,
	EXT2_SYNC_FL,
	EXT2_DIRSYNC_FL,
	EXT2_IMMUTABLE_FL,
	EXT2_APPEND_FL,
	EXT2_NODUMP_FL,
	EXT2_NOATIME_FL,
	EXT2_COMPR_FL,
	EXT3_JOURNAL_DATA_FL,
	EXT2_NOTAIL_FL,
	EXT2_TOPDIR_FL
};

const char e2attr_flags_sname[] =
#ifdef ENABLE_COMPRESSION
	"BZXE"
#endif
	"I"
	"suSDiadAcjtT";

static const char e2attr_flags_lname[] =
#ifdef ENABLE_COMPRESSION
	"Compressed_File" "\0"
	"Compressed_Dirty_File" "\0"
	"Compression_Raw_Access" "\0"
	"Compression_Error" "\0"
#endif
	"Indexed_directory" "\0"
	"Secure_Deletion" "\0"
	"Undelete" "\0"
	"Synchronous_Updates" "\0"
	"Synchronous_Directory_Updates" "\0"
	"Immutable" "\0"
	"Append_Only" "\0"
	"No_Dump" "\0"
	"No_Atime" "\0"
	"Compression_Requested" "\0"
	"Journaled_Data" "\0"
	"No_Tailmerging" "\0"
	"Top_of_Directory_Hierarchies" "\0"
	/* Another trailing NUL is added by compiler */;

void print_e2flags(FILE *f, unsigned long flags, unsigned options)
{
	const uint32_t *fv;
	const char *fn;

	fv = e2attr_flags_value;
	if (options & PFOPT_LONG) {
		int first = 1;
		fn = e2attr_flags_lname;
		do {
			if (flags & *fv) {
				if (!first)
					fputs(", ", f);
				fputs(fn, f);
				first = 0;
			}
			fv++;
			fn += strlen(fn) + 1;
		} while (*fn);
		if (first)
			fputs("---", f);
	} else {
		fn = e2attr_flags_sname;
		do  {
			char c = '-';
			if (flags & *fv)
				c = *fn;
			fputc(c, f);
			fv++;
			fn++;
		} while (*fn);
	}
}
