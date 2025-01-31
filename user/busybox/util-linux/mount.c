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
 * Mini mount implementation for busybox
 *
 * Copyright (C) 1995, 1996 by Bruce Perens <bruce@pixar.com>.
 * Copyright (C) 1999-2004 by Erik Andersen <andersen@codepoet.org>
 * Copyright (C) 2005-2006 by Rob Landley <rob@landley.net>
 *
 * Licensed under GPLv2 or later, see file LICENSE in this tarball for details.
 */

/* Design notes: There is no spec for mount.  Remind me to write one.

   mount_main() calls singlemount() which calls mount_it_now().

   mount_main() can loop through /etc/fstab for mount -a
   singlemount() can loop through /etc/filesystems for fstype detection.
   mount_it_now() does the actual mount.
*/

#include <mntent.h>
#include <syslog.h>
#include "libbb.h"

#if ENABLE_FEATURE_MOUNT_LABEL
#include "volume_id.h"
#endif

/* Needed for nfs support only */
#include <sys/utsname.h>
#undef TRUE
#undef FALSE
#include <rpc/rpc.h>
#include <rpc/pmap_prot.h>
#include <rpc/pmap_clnt.h>

#ifndef MS_SILENT
#define MS_SILENT	(1 << 15)
#endif
/* Grab more as needed from util-linux's mount/mount_constants.h */
#ifndef MS_DIRSYNC
#define MS_DIRSYNC      128     /* Directory modifications are synchronous */
#endif


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


// Not real flags, but we want to be able to check for this.
enum {
	MOUNT_USERS  = (1 << 28) * ENABLE_DESKTOP,
	MOUNT_NOAUTO = (1 << 29),
	MOUNT_SWAP   = (1 << 30),
};


#define OPTION_STR "o:t:rwanfvsi"
enum {
	OPT_o = (1 << 0),
	OPT_t = (1 << 1),
	OPT_r = (1 << 2),
	OPT_w = (1 << 3),
	OPT_a = (1 << 4),
	OPT_n = (1 << 5),
	OPT_f = (1 << 6),
	OPT_v = (1 << 7),
	OPT_s = (1 << 8),
	OPT_i = (1 << 9),
};

#if ENABLE_FEATURE_MTAB_SUPPORT
#define useMtab (!(option_mask32 & OPT_n))
#else
#define useMtab 0
#endif

#if ENABLE_FEATURE_MOUNT_FAKE
#define fakeIt (option_mask32 & OPT_f)
#else
#define fakeIt 0
#endif


// TODO: more "user" flag compatibility.
// "user" option (from mount manpage):
// Only the user that mounted a filesystem can unmount it again.
// If any user should be able to unmount, then use users instead of user
// in the fstab line.  The owner option is similar to the user option,
// with the restriction that the user must be the owner of the special file.
// This may be useful e.g. for /dev/fd if a login script makes
// the console user owner of this device.

/* Standard mount options (from -o options or --options), with corresponding
 * flags */

static const int32_t mount_options[] = {
	// MS_FLAGS set a bit.  ~MS_FLAGS disable that bit.  0 flags are NOPs.

	USE_FEATURE_MOUNT_LOOP(
		/* "loop" */ 0,
	)

	USE_FEATURE_MOUNT_FSTAB(
		/* "defaults" */ 0,
		/* "quiet" 0 - do not filter out, vfat wants to see it */
		/* "noauto" */ MOUNT_NOAUTO,
		/* "sw"     */ MOUNT_SWAP,
		/* "swap"   */ MOUNT_SWAP,
		USE_DESKTOP(/* "user"  */ MOUNT_USERS,)
		USE_DESKTOP(/* "users" */ MOUNT_USERS,)
		/* "_netdev" */ 0,
	)

	USE_FEATURE_MOUNT_FLAGS(
		// vfs flags
		/* "nosuid"      */ MS_NOSUID,
		/* "suid"        */ ~MS_NOSUID,
		/* "dev"         */ ~MS_NODEV,
		/* "nodev"       */ MS_NODEV,
		/* "exec"        */ ~MS_NOEXEC,
		/* "noexec"      */ MS_NOEXEC,
		/* "sync"        */ MS_SYNCHRONOUS,
		/* "dirsync"     */ MS_DIRSYNC,
		/* "async"       */ ~MS_SYNCHRONOUS,
		/* "atime"       */ ~MS_NOATIME,
		/* "noatime"     */ MS_NOATIME,
		/* "diratime"    */ ~MS_NODIRATIME,
		/* "nodiratime"  */ MS_NODIRATIME,
		/* "mand"        */ MS_MANDLOCK,
		/* "nomand"      */ ~MS_MANDLOCK,
		/* "relatime"    */ MS_RELATIME,
		/* "norelatime"  */ ~MS_RELATIME,
		/* "loud"        */ ~MS_SILENT,

		// action flags
		/* "bind"        */ MS_BIND,
		/* "move"        */ MS_MOVE,
		/* "shared"      */ MS_SHARED,
		/* "slave"       */ MS_SLAVE,
		/* "private"     */ MS_PRIVATE,
		/* "unbindable"  */ MS_UNBINDABLE,
		/* "rshared"     */ MS_SHARED|MS_RECURSIVE,
		/* "rslave"      */ MS_SLAVE|MS_RECURSIVE,
		/* "rprivate"    */ MS_SLAVE|MS_RECURSIVE,
		/* "runbindable" */ MS_UNBINDABLE|MS_RECURSIVE,
	)

	// Always understood.
	/* "ro"      */ MS_RDONLY,  // vfs flag
	/* "rw"      */ ~MS_RDONLY, // vfs flag
	/* "remount" */ MS_REMOUNT  // action flag
};

static const char mount_option_str[] =
	USE_FEATURE_MOUNT_LOOP(
		"loop" "\0"
	)
	USE_FEATURE_MOUNT_FSTAB(
		"defaults" "\0"
		/* "quiet" "\0" - do not filter out, vfat wants to see it */
		"noauto" "\0"
		"sw" "\0"
		"swap" "\0"
		USE_DESKTOP("user" "\0")
		USE_DESKTOP("users" "\0")
		"_netdev" "\0"
	)
	USE_FEATURE_MOUNT_FLAGS(
		// vfs flags
		"nosuid" "\0"
		"suid" "\0"
		"dev" "\0"
		"nodev" "\0"
		"exec" "\0"
		"noexec" "\0"
		"sync" "\0"
		"dirsync" "\0"
		"async" "\0"
		"atime" "\0"
		"noatime" "\0"
		"diratime" "\0"
		"nodiratime" "\0"
		"mand" "\0"
		"nomand" "\0"
		"relatime" "\0"
		"norelatime" "\0"
		"loud" "\0"

		// action flags
		"bind" "\0"
		"move" "\0"
		"shared" "\0"
		"slave" "\0"
		"private" "\0"
		"unbindable" "\0"
		"rshared" "\0"
		"rslave" "\0"
		"rprivate" "\0"
		"runbindable" "\0"
	)

	// Always understood.
	"ro" "\0"        // vfs flag
	"rw" "\0"        // vfs flag
	"remount" "\0"   // action flag
;


struct globals {
#if ENABLE_FEATURE_MOUNT_NFS
	smalluint nfs_mount_version;
#endif
#if ENABLE_FEATURE_MOUNT_VERBOSE
	unsigned verbose;
#endif
	llist_t *fslist;
	char getmntent_buf[1];

};
enum { GETMNTENT_BUFSIZE = COMMON_BUFSIZE - offsetof(struct globals, getmntent_buf) };
#define G (*(struct globals*)&bb_common_bufsiz1)
#define nfs_mount_version (G.nfs_mount_version)
#if ENABLE_FEATURE_MOUNT_VERBOSE
#define verbose           (G.verbose          )
#else
#define verbose           0
#endif
#define fslist            (G.fslist           )
#define getmntent_buf     (G.getmntent_buf    )


#if ENABLE_FEATURE_MOUNT_VERBOSE
static int verbose_mount(const char *source, const char *target,
		const char *filesystemtype,
		unsigned long mountflags, const void *data)
{
	int rc;

	errno = 0;
	rc = mount(source, target, filesystemtype, mountflags, data);
	if (verbose >= 2)
		bb_perror_msg("mount('%s','%s','%s',0x%08lx,'%s'):%d",
			source, target, filesystemtype,
			mountflags, (char*)data, rc);
	return rc;
}
#else
#define verbose_mount(...) mount(__VA_ARGS__)
#endif

static int resolve_mount_spec(char **fsname)
{
	char *tmp = NULL;

#if ENABLE_FEATURE_MOUNT_LABEL
	if (!strncmp(*fsname, "UUID=", 5))
		tmp = get_devname_from_uuid(*fsname + 5);
	else if (!strncmp(*fsname, "LABEL=", 6))
		tmp = get_devname_from_label(*fsname + 6);
#endif

	if (tmp) {
		*fsname = tmp;
		return 1;
	}
	return 0;
}

/* Append mount options to string */
static void append_mount_options(char **oldopts, const char *newopts)
{
	if (*oldopts && **oldopts) {
		/* do not insert options which are already there */
		while (newopts[0]) {
			char *p;
			int len = strlen(newopts);
			p = strchr(newopts, ',');
			if (p) len = p - newopts;
			p = *oldopts;
			while (1) {
				if (!strncmp(p, newopts, len)
				 && (p[len] == ',' || p[len] == '\0'))
					goto skip;
				p = strchr(p,',');
				if (!p) break;
				p++;
			}
			p = xasprintf("%s,%.*s", *oldopts, len, newopts);
			free(*oldopts);
			*oldopts = p;
 skip:
			newopts += len;
			while (newopts[0] == ',') newopts++;
		}
	} else {
		if (ENABLE_FEATURE_CLEAN_UP) free(*oldopts);
		*oldopts = xstrdup(newopts);
	}
}

/* Use the mount_options list to parse options into flags.
 * Also return list of unrecognized options if unrecognized!=NULL */
static long parse_mount_options(char *options, char **unrecognized)
{
	long flags = MS_SILENT;

	// Loop through options
	for (;;) {
		unsigned i;
		char *comma = strchr(options, ',');
		const char *option_str = mount_option_str;

		if (comma) *comma = '\0';

/* FIXME: use hasmntopt() */
		// Find this option in mount_options
		for (i = 0; i < ARRAY_SIZE(mount_options); i++) {
			if (!strcasecmp(option_str, options)) {
				long fl = mount_options[i];
				if (fl < 0) flags &= fl;
				else flags |= fl;
				break;
			}
			option_str += strlen(option_str) + 1;
		}
		// If unrecognized not NULL, append unrecognized mount options */
		if (unrecognized && i == ARRAY_SIZE(mount_options)) {
			// Add it to strflags, to pass on to kernel
			i = *unrecognized ? strlen(*unrecognized) : 0;
			*unrecognized = xrealloc(*unrecognized, i + strlen(options) + 2);

			// Comma separated if it's not the first one
			if (i) (*unrecognized)[i++] = ',';
			strcpy((*unrecognized)+i, options);
		}

		if (!comma)
			break;
		// Advance to next option
		*comma = ',';
		options = ++comma;
	}

	return flags;
}

// Return a list of all block device backed filesystems

static llist_t *get_block_backed_filesystems(void)
{
	static const char filesystems[2][sizeof("/proc/filesystems")] = {
		"/etc/filesystems",
		"/proc/filesystems",
	};
	char *fs, *buf;
	llist_t *list = 0;
	int i;
	FILE *f;

	for (i = 0; i < 2; i++) {
		f = fopen_for_read(filesystems[i]);
		if (!f) continue;

		while ((buf = xmalloc_fgetline(f)) != NULL) {
			if (!strncmp(buf, "nodev", 5) && isspace(buf[5]))
				continue;
			fs = skip_whitespace(buf);
			if (*fs=='#' || *fs=='*' || !*fs) continue;

			llist_add_to_end(&list, xstrdup(fs));
			free(buf);
		}
		if (ENABLE_FEATURE_CLEAN_UP) fclose(f);
	}

	return list;
}

#if ENABLE_FEATURE_CLEAN_UP
static void delete_block_backed_filesystems(void)
{
	llist_free(fslist, free);
}
#else
void delete_block_backed_filesystems(void);
#endif

// Perform actual mount of specific filesystem at specific location.
// NB: mp->xxx fields may be trashed on exit
static int mount_it_now(struct mntent *mp, long vfsflags, char *filteropts)
{
	int rc = 0;

	if (fakeIt) {
		if (verbose >= 2)
			bb_error_msg("would do mount('%s','%s','%s',0x%08lx,'%s')",
				mp->mnt_fsname, mp->mnt_dir, mp->mnt_type,
				vfsflags, filteropts);
		goto mtab;
	}

	// Mount, with fallback to read-only if necessary.
	for (;;) {
		errno = 0;
		rc = verbose_mount(mp->mnt_fsname, mp->mnt_dir, mp->mnt_type,
				vfsflags, filteropts);

		// If mount failed, try
		// helper program mount.<mnt_type>
		if (ENABLE_FEATURE_MOUNT_HELPERS && rc) {
			char *args[6];
			int errno_save = errno;
			args[0] = xasprintf("mount.%s", mp->mnt_type);
			rc = 1;
			if (filteropts) {
				args[rc++] = (char *)"-o";
				args[rc++] = filteropts;
			}
			args[rc++] = mp->mnt_fsname;
			args[rc++] = mp->mnt_dir;
			args[rc] = NULL;
			rc = wait4pid(spawn(args));
			free(args[0]);
			if (!rc)
				break;
			errno = errno_save;
		}

		if (!rc || (vfsflags & MS_RDONLY) || (errno != EACCES && errno != EROFS))
			break;
		if (!(vfsflags & MS_SILENT))
			bb_error_msg("%s is write-protected, mounting read-only",
						mp->mnt_fsname);
		vfsflags |= MS_RDONLY;
	}

	// Abort entirely if permission denied.

	if (rc && errno == EPERM)
		bb_error_msg_and_die(bb_msg_perm_denied_are_you_root);

	/* If the mount was successful, and we're maintaining an old-style
	 * mtab file by hand, add the new entry to it now. */
 mtab:
	if (useMtab && !rc && !(vfsflags & MS_REMOUNT)) {
		char *fsname;
		FILE *mountTable = setmntent(bb_path_mtab_file, "a+");
		const char *option_str = mount_option_str;
		int i;

		if (!mountTable) {
			bb_error_msg("no %s", bb_path_mtab_file);
			goto ret;
		}

		// Add vfs string flags

		for (i = 0; mount_options[i] != MS_REMOUNT; i++) {
			if (mount_options[i] > 0 && (mount_options[i] & vfsflags))
				append_mount_options(&(mp->mnt_opts), option_str);
			option_str += strlen(option_str) + 1;
		}

		// Remove trailing / (if any) from directory we mounted on

		i = strlen(mp->mnt_dir) - 1;
		if (i > 0 && mp->mnt_dir[i] == '/') mp->mnt_dir[i] = '\0';

		// Convert to canonical pathnames as needed

		mp->mnt_dir = bb_simplify_path(mp->mnt_dir);
		fsname = 0;
		if (!mp->mnt_type || !*mp->mnt_type) { /* bind mount */
			mp->mnt_fsname = fsname = bb_simplify_path(mp->mnt_fsname);
			mp->mnt_type = (char*)"bind";
		}
		mp->mnt_freq = mp->mnt_passno = 0;

		// Write and close.

		addmntent(mountTable, mp);
		endmntent(mountTable);
		if (ENABLE_FEATURE_CLEAN_UP) {
			free(mp->mnt_dir);
			free(fsname);
		}
	}
 ret:
	return rc;
}

#if ENABLE_FEATURE_MOUNT_NFS

/*
 * Linux NFS mount
 * Copyright (C) 1993 Rick Sladkey <jrs@world.std.com>
 *
 * Licensed under GPLv2, see file LICENSE in this tarball for details.
 *
 * Wed Feb  8 12:51:48 1995, biro@yggdrasil.com (Ross Biro): allow all port
 * numbers to be specified on the command line.
 *
 * Fri, 8 Mar 1996 18:01:39, Swen Thuemmler <swen@uni-paderborn.de>:
 * Omit the call to connect() for Linux version 1.3.11 or later.
 *
 * Wed Oct  1 23:55:28 1997: Dick Streefland <dick_streefland@tasking.com>
 * Implemented the "bg", "fg" and "retry" mount options for NFS.
 *
 * 1999-02-22 Arkadiusz Mickiewicz <misiek@misiek.eu.org>
 * - added Native Language Support
 *
 * Modified by Olaf Kirch and Trond Myklebust for new NFS code,
 * plus NFSv3 stuff.
 */

/* This is just a warning of a common mistake.  Possibly this should be a
 * uclibc faq entry rather than in busybox... */
#if defined(__UCLIBC__) && ! defined(__UCLIBC_HAS_RPC__)
#error "You need to build uClibc with UCLIBC_HAS_RPC for NFS support."
#endif

#define MOUNTPORT 635
#define MNTPATHLEN 1024
#define MNTNAMLEN 255
#define FHSIZE 32
#define FHSIZE3 64

typedef char fhandle[FHSIZE];

typedef struct {
	unsigned int fhandle3_len;
	char *fhandle3_val;
} fhandle3;

enum mountstat3 {
	MNT_OK = 0,
	MNT3ERR_PERM = 1,
	MNT3ERR_NOENT = 2,
	MNT3ERR_IO = 5,
	MNT3ERR_ACCES = 13,
	MNT3ERR_NOTDIR = 20,
	MNT3ERR_INVAL = 22,
	MNT3ERR_NAMETOOLONG = 63,
	MNT3ERR_NOTSUPP = 10004,
	MNT3ERR_SERVERFAULT = 10006,
};
typedef enum mountstat3 mountstat3;

struct fhstatus {
	unsigned int fhs_status;
	union {
		fhandle fhs_fhandle;
	} fhstatus_u;
};
typedef struct fhstatus fhstatus;

struct mountres3_ok {
	fhandle3 fhandle;
	struct {
		unsigned int auth_flavours_len;
		char *auth_flavours_val;
	} auth_flavours;
};
typedef struct mountres3_ok mountres3_ok;

struct mountres3 {
	mountstat3 fhs_status;
	union {
		mountres3_ok mountinfo;
	} mountres3_u;
};
typedef struct mountres3 mountres3;

typedef char *dirpath;

typedef char *name;

typedef struct mountbody *mountlist;

struct mountbody {
	name ml_hostname;
	dirpath ml_directory;
	mountlist ml_next;
};
typedef struct mountbody mountbody;

typedef struct groupnode *groups;

struct groupnode {
	name gr_name;
	groups gr_next;
};
typedef struct groupnode groupnode;

typedef struct exportnode *exports;

struct exportnode {
	dirpath ex_dir;
	groups ex_groups;
	exports ex_next;
};
typedef struct exportnode exportnode;

struct ppathcnf {
	int pc_link_max;
	short pc_max_canon;
	short pc_max_input;
	short pc_name_max;
	short pc_path_max;
	short pc_pipe_buf;
	uint8_t pc_vdisable;
	char pc_xxx;
	short pc_mask[2];
};
typedef struct ppathcnf ppathcnf;

#define MOUNTPROG 100005
#define MOUNTVERS 1

#define MOUNTPROC_NULL 0
#define MOUNTPROC_MNT 1
#define MOUNTPROC_DUMP 2
#define MOUNTPROC_UMNT 3
#define MOUNTPROC_UMNTALL 4
#define MOUNTPROC_EXPORT 5
#define MOUNTPROC_EXPORTALL 6

#define MOUNTVERS_POSIX 2

#define MOUNTPROC_PATHCONF 7

#define MOUNT_V3 3

#define MOUNTPROC3_NULL 0
#define MOUNTPROC3_MNT 1
#define MOUNTPROC3_DUMP 2
#define MOUNTPROC3_UMNT 3
#define MOUNTPROC3_UMNTALL 4
#define MOUNTPROC3_EXPORT 5

enum {
#ifndef NFS_FHSIZE
	NFS_FHSIZE = 32,
#endif
#ifndef NFS_PORT
	NFS_PORT = 2049
#endif
};

/*
 * We want to be able to compile mount on old kernels in such a way
 * that the binary will work well on more recent kernels.
 * Thus, if necessary we teach nfsmount.c the structure of new fields
 * that will come later.
 *
 * Moreover, the new kernel includes conflict with glibc includes
 * so it is easiest to ignore the kernel altogether (at compile time).
 */

struct nfs2_fh {
	char                    data[32];
};
struct nfs3_fh {
	unsigned short          size;
	unsigned char           data[64];
};

struct nfs_mount_data {
	int		version;		/* 1 */
	int		fd;			/* 1 */
	struct nfs2_fh	old_root;		/* 1 */
	int		flags;			/* 1 */
	int		rsize;			/* 1 */
	int		wsize;			/* 1 */
	int		timeo;			/* 1 */
	int		retrans;		/* 1 */
	int		acregmin;		/* 1 */
	int		acregmax;		/* 1 */
	int		acdirmin;		/* 1 */
	int		acdirmax;		/* 1 */
	struct sockaddr_in addr;		/* 1 */
	char		hostname[256];		/* 1 */
	int		namlen;			/* 2 */
	unsigned int	bsize;			/* 3 */
	struct nfs3_fh	root;			/* 4 */
};

/* bits in the flags field */
enum {
	NFS_MOUNT_SOFT = 0x0001,	/* 1 */
	NFS_MOUNT_INTR = 0x0002,	/* 1 */
	NFS_MOUNT_SECURE = 0x0004,	/* 1 */
	NFS_MOUNT_POSIX = 0x0008,	/* 1 */
	NFS_MOUNT_NOCTO = 0x0010,	/* 1 */
	NFS_MOUNT_NOAC = 0x0020,	/* 1 */
	NFS_MOUNT_TCP = 0x0040,		/* 2 */
	NFS_MOUNT_VER3 = 0x0080,	/* 3 */
	NFS_MOUNT_KERBEROS = 0x0100,	/* 3 */
	NFS_MOUNT_NONLM = 0x0200,	/* 3 */
	NFS_MOUNT_NORDIRPLUS = 0x4000
};


/*
 * We need to translate between nfs status return values and
 * the local errno values which may not be the same.
 *
 * Andreas Schwab <schwab@LS5.informatik.uni-dortmund.de>: change errno:
 * "after #include <errno.h> the symbol errno is reserved for any use,
 *  it cannot even be used as a struct tag or field name".
 */

#ifndef EDQUOT
#define EDQUOT	ENOSPC
#endif

// Convert each NFSERR_BLAH into EBLAH

static const struct {
	short stat;
	short errnum;
} nfs_errtbl[] = {
	{0,0}, {1,EPERM}, {2,ENOENT}, {5,EIO}, {6,ENXIO}, {13,EACCES}, {17,EEXIST},
	{19,ENODEV}, {20,ENOTDIR}, {21,EISDIR}, {22,EINVAL}, {27,EFBIG},
	{28,ENOSPC}, {30,EROFS}, {63,ENAMETOOLONG}, {66,ENOTEMPTY}, {69,EDQUOT},
	{70,ESTALE}, {71,EREMOTE}, {-1,EIO}
};

static char *nfs_strerror(int status)
{
	int i;

	for (i = 0; nfs_errtbl[i].stat != -1; i++) {
		if (nfs_errtbl[i].stat == status)
			return strerror(nfs_errtbl[i].errnum);
	}
	return xasprintf("unknown nfs status return value: %d", status);
}

static bool_t xdr_fhandle(XDR *xdrs, fhandle objp)
{
	if (!xdr_opaque(xdrs, objp, FHSIZE))
		 return FALSE;
	return TRUE;
}

static bool_t xdr_fhstatus(XDR *xdrs, fhstatus *objp)
{
	if (!xdr_u_int(xdrs, &objp->fhs_status))
		 return FALSE;
	switch (objp->fhs_status) {
	case 0:
		if (!xdr_fhandle(xdrs, objp->fhstatus_u.fhs_fhandle))
			 return FALSE;
		break;
	default:
		break;
	}
	return TRUE;
}

static bool_t xdr_dirpath(XDR *xdrs, dirpath *objp)
{
	if (!xdr_string(xdrs, objp, MNTPATHLEN))
		 return FALSE;
	return TRUE;
}

static bool_t xdr_fhandle3(XDR *xdrs, fhandle3 *objp)
{
	if (!xdr_bytes(xdrs, (char **)&objp->fhandle3_val, (unsigned int *) &objp->fhandle3_len, FHSIZE3))
		 return FALSE;
	return TRUE;
}

static bool_t xdr_mountres3_ok(XDR *xdrs, mountres3_ok *objp)
{
	if (!xdr_fhandle3(xdrs, &objp->fhandle))
		return FALSE;
	if (!xdr_array(xdrs, &(objp->auth_flavours.auth_flavours_val), &(objp->auth_flavours.auth_flavours_len), ~0,
				sizeof (int), (xdrproc_t) xdr_int))
		return FALSE;
	return TRUE;
}

static bool_t xdr_mountstat3(XDR *xdrs, mountstat3 *objp)
{
	if (!xdr_enum(xdrs, (enum_t *) objp))
		 return FALSE;
	return TRUE;
}

static bool_t xdr_mountres3(XDR *xdrs, mountres3 *objp)
{
	if (!xdr_mountstat3(xdrs, &objp->fhs_status))
		return FALSE;
	switch (objp->fhs_status) {
	case MNT_OK:
		if (!xdr_mountres3_ok(xdrs, &objp->mountres3_u.mountinfo))
			 return FALSE;
		break;
	default:
		break;
	}
	return TRUE;
}

#define MAX_NFSPROT ((nfs_mount_version >= 4) ? 3 : 2)

/*
 * Unfortunately, the kernel prints annoying console messages
 * in case of an unexpected nfs mount version (instead of
 * just returning some error).  Therefore we'll have to try
 * and figure out what version the kernel expects.
 *
 * Variables:
 *	KERNEL_NFS_MOUNT_VERSION: kernel sources at compile time
 *	NFS_MOUNT_VERSION: these nfsmount sources at compile time
 *	nfs_mount_version: version this source and running kernel can handle
 */
static void
find_kernel_nfs_mount_version(void)
{
	int kernel_version;

	if (nfs_mount_version)
		return;

	nfs_mount_version = 4; /* default */

	kernel_version = get_linux_version_code();
	if (kernel_version) {
		if (kernel_version < KERNEL_VERSION(2,1,32))
			nfs_mount_version = 1;
		else if (kernel_version < KERNEL_VERSION(2,2,18) ||
				(kernel_version >= KERNEL_VERSION(2,3,0) &&
				 kernel_version < KERNEL_VERSION(2,3,99)))
			nfs_mount_version = 3;
		/* else v4 since 2.3.99pre4 */
	}
}

static void
get_mountport(struct pmap *pm_mnt,
	struct sockaddr_in *server_addr,
	long unsigned prog,
	long unsigned version,
	long unsigned proto,
	long unsigned port)
{
	struct pmaplist *pmap;

	server_addr->sin_port = PMAPPORT;
/* glibc 2.4 (still) has pmap_getmaps(struct sockaddr_in *).
 * I understand it like "IPv6 for this is not 100% ready" */
	pmap = pmap_getmaps(server_addr);

	if (version > MAX_NFSPROT)
		version = MAX_NFSPROT;
	if (!prog)
		prog = MOUNTPROG;
	pm_mnt->pm_prog = prog;
	pm_mnt->pm_vers = version;
	pm_mnt->pm_prot = proto;
	pm_mnt->pm_port = port;

	while (pmap) {
		if (pmap->pml_map.pm_prog != prog)
			goto next;
		if (!version && pm_mnt->pm_vers > pmap->pml_map.pm_vers)
			goto next;
		if (version > 2 && pmap->pml_map.pm_vers != version)
			goto next;
		if (version && version <= 2 && pmap->pml_map.pm_vers > 2)
			goto next;
		if (pmap->pml_map.pm_vers > MAX_NFSPROT ||
		    (proto && pm_mnt->pm_prot && pmap->pml_map.pm_prot != proto) ||
		    (port && pmap->pml_map.pm_port != port))
			goto next;
		memcpy(pm_mnt, &pmap->pml_map, sizeof(*pm_mnt));
 next:
		pmap = pmap->pml_next;
	}
	if (!pm_mnt->pm_vers)
		pm_mnt->pm_vers = MOUNTVERS;
	if (!pm_mnt->pm_port)
		pm_mnt->pm_port = MOUNTPORT;
	if (!pm_mnt->pm_prot)
		pm_mnt->pm_prot = IPPROTO_TCP;
}

#if BB_MMU
static int daemonize(void)
{
	int pid = fork();
	if (pid < 0) /* error */
		return -errno;
	if (pid > 0) /* parent */
		return 0;
	/* child */
	close(0);
	xopen(bb_dev_null, O_RDWR);
	xdup2(0, 1);
	xdup2(0, 2);
	setsid();
	openlog(applet_name, LOG_PID, LOG_DAEMON);
	logmode = LOGMODE_SYSLOG;
	return 1;
}
#else
static inline int daemonize(void) { return -ENOSYS; }
#endif

// TODO
static inline int we_saw_this_host_before(const char *hostname UNUSED_PARAM)
{
	return 0;
}

/* RPC strerror analogs are terminally idiotic:
 * *mandatory* prefix and \n at end.
 * This hopefully helps. Usage:
 * error_msg_rpc(clnt_*error*(" ")) */
static void error_msg_rpc(const char *msg)
{
	int len;
	while (msg[0] == ' ' || msg[0] == ':') msg++;
	len = strlen(msg);
	while (len && msg[len-1] == '\n') len--;
	bb_error_msg("%.*s", len, msg);
}

// NB: mp->xxx fields may be trashed on exit
static int nfsmount(struct mntent *mp, long vfsflags, char *filteropts)
{
	CLIENT *mclient;
	char *hostname;
	char *pathname;
	char *mounthost;
	struct nfs_mount_data data;
	char *opt;
	struct hostent *hp;
	struct sockaddr_in server_addr;
	struct sockaddr_in mount_server_addr;
	int msock, fsock;
	union {
		struct fhstatus nfsv2;
		struct mountres3 nfsv3;
	} status;
	int daemonized;
	char *s;
	int port;
	int mountport;
	int proto;
#if BB_MMU
	smallint bg = 0;
#else
	enum { bg = 0 };
#endif
	int retry;
	int mountprog;
	int mountvers;
	int nfsprog;
	int nfsvers;
	int retval;
	/* these all are one-bit really. 4.3.1 likes this combination: */
	smallint tcp;
	smallint soft;
	int intr;
	int posix;
	int nocto;
	int noac;
	int nordirplus;
	int nolock;

	find_kernel_nfs_mount_version();

	daemonized = 0;
	mounthost = NULL;
	retval = ETIMEDOUT;
	msock = fsock = -1;
	mclient = NULL;

	/* NB: hostname, mounthost, filteropts must be free()d prior to return */

	filteropts = xstrdup(filteropts); /* going to trash it later... */

	hostname = xstrdup(mp->mnt_fsname);
	/* mount_main() guarantees that ':' is there */
	s = strchr(hostname, ':');
	pathname = s + 1;
	*s = '\0';
	/* Ignore all but first hostname in replicated mounts
	   until they can be fully supported. (mack@sgi.com) */
	s = strchr(hostname, ',');
	if (s) {
		*s = '\0';
		bb_error_msg("warning: multiple hostnames not supported");
	}

	server_addr.sin_family = AF_INET;
	if (!inet_aton(hostname, &server_addr.sin_addr)) {
		hp = gethostbyname(hostname);
		if (hp == NULL) {
			bb_herror_msg("%s", hostname);
			goto fail;
		}
		if ((size_t)hp->h_length > sizeof(struct in_addr)) {
			bb_error_msg("got bad hp->h_length");
			hp->h_length = sizeof(struct in_addr);
		}
		memcpy(&server_addr.sin_addr,
				hp->h_addr, hp->h_length);
	}

	memcpy(&mount_server_addr, &server_addr, sizeof(mount_server_addr));

	/* add IP address to mtab options for use when unmounting */

	if (!mp->mnt_opts) { /* TODO: actually mp->mnt_opts is never NULL */
		mp->mnt_opts = xasprintf("addr=%s", inet_ntoa(server_addr.sin_addr));
	} else {
		char *tmp = xasprintf("%s%saddr=%s", mp->mnt_opts,
					mp->mnt_opts[0] ? "," : "",
					inet_ntoa(server_addr.sin_addr));
		free(mp->mnt_opts);
		mp->mnt_opts = tmp;
	}

	/* Set default options.
	 * rsize/wsize (and bsize, for ver >= 3) are left 0 in order to
	 * let the kernel decide.
	 * timeo is filled in after we know whether it'll be TCP or UDP. */
	memset(&data, 0, sizeof(data));
	data.retrans  = 3;
	data.acregmin = 3;
	data.acregmax = 60;
	data.acdirmin = 30;
	data.acdirmax = 60;
	data.namlen   = NAME_MAX;

	soft = 0;
	intr = 0;
	posix = 0;
	nocto = 0;
	nolock = 0;
	noac = 0;
	nordirplus = 0;
	retry = 10000;		/* 10000 minutes ~ 1 week */
	tcp = 0;

	mountprog = MOUNTPROG;
	mountvers = 0;
	port = 0;
	mountport = 0;
	nfsprog = 100003;
	nfsvers = 0;

	/* parse options */
	if (filteropts)	for (opt = strtok(filteropts, ","); opt; opt = strtok(NULL, ",")) {
		char *opteq = strchr(opt, '=');
		if (opteq) {
			int val, idx;
			static const char options[] ALIGN1 =
				/* 0 */ "rsize\0"
				/* 1 */ "wsize\0"
				/* 2 */ "timeo\0"
				/* 3 */ "retrans\0"
				/* 4 */ "acregmin\0"
				/* 5 */ "acregmax\0"
				/* 6 */ "acdirmin\0"
				/* 7 */ "acdirmax\0"
				/* 8 */ "actimeo\0"
				/* 9 */ "retry\0"
				/* 10 */ "port\0"
				/* 11 */ "mountport\0"
				/* 12 */ "mounthost\0"
				/* 13 */ "mountprog\0"
				/* 14 */ "mountvers\0"
				/* 15 */ "nfsprog\0"
				/* 16 */ "nfsvers\0"
				/* 17 */ "vers\0"
				/* 18 */ "proto\0"
				/* 19 */ "namlen\0"
				/* 20 */ "addr\0";

			*opteq++ = '\0';
			idx = index_in_strings(options, opt);
			switch (idx) {
			case 12: // "mounthost"
				mounthost = xstrndup(opteq,
						strcspn(opteq, " \t\n\r,"));
				continue;
			case 18: // "proto"
				if (!strncmp(opteq, "tcp", 3))
					tcp = 1;
				else if (!strncmp(opteq, "udp", 3))
					tcp = 0;
				else
					bb_error_msg("warning: unrecognized proto= option");
				continue;
			case 20: // "addr" - ignore
				continue;
			}

			val = xatoi_u(opteq);
			switch (idx) {
			case 0: // "rsize"
				data.rsize = val;
				continue;
			case 1: // "wsize"
				data.wsize = val;
				continue;
			case 2: // "timeo"
				data.timeo = val;
				continue;
			case 3: // "retrans"
				data.retrans = val;
				continue;
			case 4: // "acregmin"
				data.acregmin = val;
				continue;
			case 5: // "acregmax"
				data.acregmax = val;
				continue;
			case 6: // "acdirmin"
				data.acdirmin = val;
				continue;
			case 7: // "acdirmax"
				data.acdirmax = val;
				continue;
			case 8: // "actimeo"
				data.acregmin = val;
				data.acregmax = val;
				data.acdirmin = val;
				data.acdirmax = val;
				continue;
			case 9: // "retry"
				retry = val;
				continue;
			case 10: // "port"
				port = val;
				continue;
			case 11: // "mountport"
				mountport = val;
				continue;
			case 13: // "mountprog"
				mountprog = val;
				continue;
			case 14: // "mountvers"
				mountvers = val;
				continue;
			case 15: // "nfsprog"
				nfsprog = val;
				continue;
			case 16: // "nfsvers"
			case 17: // "vers"
				nfsvers = val;
				continue;
			case 19: // "namlen"
				//if (nfs_mount_version >= 2)
					data.namlen = val;
				//else
				//	bb_error_msg("warning: option namlen is not supported\n");
				continue;
			default:
				bb_error_msg("unknown nfs mount parameter: %s=%d", opt, val);
				goto fail;
			}
		}
		else { /* not of the form opt=val */
			static const char options[] ALIGN1 =
				"bg\0"
				"fg\0"
				"soft\0"
				"hard\0"
				"intr\0"
				"posix\0"
				"cto\0"
				"ac\0"
				"tcp\0"
				"udp\0"
				"lock\0"
				"rdirplus\0";
			int val = 1;
			if (!strncmp(opt, "no", 2)) {
				val = 0;
				opt += 2;
			}
			switch (index_in_strings(options, opt)) {
			case 0: // "bg"
#if BB_MMU
				bg = val;
#endif
				break;
			case 1: // "fg"
#if BB_MMU
				bg = !val;
#endif
				break;
			case 2: // "soft"
				soft = val;
				break;
			case 3: // "hard"
				soft = !val;
				break;
			case 4: // "intr"
				intr = val;
				break;
			case 5: // "posix"
				posix = val;
				break;
			case 6: // "cto"
				nocto = !val;
				break;
			case 7: // "ac"
				noac = !val;
				break;
			case 8: // "tcp"
				tcp = val;
				break;
			case 9: // "udp"
				tcp = !val;
				break;
			case 10: // "lock"
				if (nfs_mount_version >= 3)
					nolock = !val;
				else
					bb_error_msg("warning: option nolock is not supported");
				break;
			case 11: //rdirplus
				nordirplus = !val;
				break;
			default:
				bb_error_msg("unknown nfs mount option: %s%s", val ? "" : "no", opt);
				goto fail;
			}
		}
	}
	proto = (tcp) ? IPPROTO_TCP : IPPROTO_UDP;

	data.flags = (soft ? NFS_MOUNT_SOFT : 0)
		| (intr ? NFS_MOUNT_INTR : 0)
		| (posix ? NFS_MOUNT_POSIX : 0)
		| (nocto ? NFS_MOUNT_NOCTO : 0)
		| (noac ? NFS_MOUNT_NOAC : 0)
		| (nordirplus ? NFS_MOUNT_NORDIRPLUS : 0);
	if (nfs_mount_version >= 2)
		data.flags |= (tcp ? NFS_MOUNT_TCP : 0);
	if (nfs_mount_version >= 3)
		data.flags |= (nolock ? NFS_MOUNT_NONLM : 0);
	if (nfsvers > MAX_NFSPROT || mountvers > MAX_NFSPROT) {
		bb_error_msg("NFSv%d not supported", nfsvers);
		goto fail;
	}
	if (nfsvers && !mountvers)
		mountvers = (nfsvers < 3) ? 1 : nfsvers;
	if (nfsvers && nfsvers < mountvers) {
		mountvers = nfsvers;
	}

	/* Adjust options if none specified */
	if (!data.timeo)
		data.timeo = tcp ? 70 : 7;

	data.version = nfs_mount_version;

	if (vfsflags & MS_REMOUNT)
		goto do_mount;

	/*
	 * If the previous mount operation on the same host was
	 * backgrounded, and the "bg" for this mount is also set,
	 * give up immediately, to avoid the initial timeout.
	 */
	if (bg && we_saw_this_host_before(hostname)) {
		daemonized = daemonize();
		if (daemonized <= 0) { /* parent or error */
			retval = -daemonized;
			goto ret;
		}
	}

	/* create mount daemon client */
	/* See if the nfs host = mount host. */
	if (mounthost) {
		if (mounthost[0] >= '0' && mounthost[0] <= '9') {
			mount_server_addr.sin_family = AF_INET;
			mount_server_addr.sin_addr.s_addr = inet_addr(hostname);
		} else {
			hp = gethostbyname(mounthost);
			if (hp == NULL) {
				bb_herror_msg("%s", mounthost);
				goto fail;
			}
			if ((size_t)hp->h_length > sizeof(struct in_addr)) {
				bb_error_msg("got bad hp->h_length");
				hp->h_length = sizeof(struct in_addr);
			}
			mount_server_addr.sin_family = AF_INET;
			memcpy(&mount_server_addr.sin_addr,
						hp->h_addr, hp->h_length);
		}
	}

	/*
	 * The following loop implements the mount retries. When the mount
	 * times out, and the "bg" option is set, we background ourself
	 * and continue trying.
	 *
	 * The case where the mount point is not present and the "bg"
	 * option is set, is treated as a timeout. This is done to
	 * support nested mounts.
	 *
	 * The "retry" count specified by the user is the number of
	 * minutes to retry before giving up.
	 */
	{
		struct timeval total_timeout;
		struct timeval retry_timeout;
		struct pmap pm_mnt;
		time_t t;
		time_t prevt;
		time_t timeout;

		retry_timeout.tv_sec = 3;
		retry_timeout.tv_usec = 0;
		total_timeout.tv_sec = 20;
		total_timeout.tv_usec = 0;
//FIXME: use monotonic()?
		timeout = time(NULL) + 60 * retry;
		prevt = 0;
		t = 30;
 retry:
		/* be careful not to use too many CPU cycles */
		if (t - prevt < 30)
			sleep(30);

		get_mountport(&pm_mnt, &mount_server_addr,
				mountprog,
				mountvers,
				proto,
				mountport);
		nfsvers = (pm_mnt.pm_vers < 2) ? 2 : pm_mnt.pm_vers;

		/* contact the mount daemon via TCP */
		mount_server_addr.sin_port = htons(pm_mnt.pm_port);
		msock = RPC_ANYSOCK;

		switch (pm_mnt.pm_prot) {
		case IPPROTO_UDP:
			mclient = clntudp_create(&mount_server_addr,
						 pm_mnt.pm_prog,
						 pm_mnt.pm_vers,
						 retry_timeout,
						 &msock);
			if (mclient)
				break;
			mount_server_addr.sin_port = htons(pm_mnt.pm_port);
			msock = RPC_ANYSOCK;
		case IPPROTO_TCP:
			mclient = clnttcp_create(&mount_server_addr,
						 pm_mnt.pm_prog,
						 pm_mnt.pm_vers,
						 &msock, 0, 0);
			break;
		default:
			mclient = NULL;
		}
		if (!mclient) {
			if (!daemonized && prevt == 0)
				error_msg_rpc(clnt_spcreateerror(" "));
		} else {
			enum clnt_stat clnt_stat;
			/* try to mount hostname:pathname */
			mclient->cl_auth = authunix_create_default();

			/* make pointers in xdr_mountres3 NULL so
			 * that xdr_array allocates memory for us
			 */
			memset(&status, 0, sizeof(status));

			if (pm_mnt.pm_vers == 3)
				clnt_stat = clnt_call(mclient, MOUNTPROC3_MNT,
					      (xdrproc_t) xdr_dirpath,
					      (caddr_t) &pathname,
					      (xdrproc_t) xdr_mountres3,
					      (caddr_t) &status,
					      total_timeout);
			else
				clnt_stat = clnt_call(mclient, MOUNTPROC_MNT,
					      (xdrproc_t) xdr_dirpath,
					      (caddr_t) &pathname,
					      (xdrproc_t) xdr_fhstatus,
					      (caddr_t) &status,
					      total_timeout);

			if (clnt_stat == RPC_SUCCESS)
				goto prepare_kernel_data; /* we're done */
			if (errno != ECONNREFUSED) {
				error_msg_rpc(clnt_sperror(mclient, " "));
				goto fail;	/* don't retry */
			}
			/* Connection refused */
			if (!daemonized && prevt == 0) /* print just once */
				error_msg_rpc(clnt_sperror(mclient, " "));
			auth_destroy(mclient->cl_auth);
			clnt_destroy(mclient);
			mclient = NULL;
			close(msock);
			msock = -1;
		}

		/* Timeout. We are going to retry... maybe */

		if (!bg)
			goto fail;
		if (!daemonized) {
			daemonized = daemonize();
			if (daemonized <= 0) { /* parent or error */
				retval = -daemonized;
				goto ret;
			}
		}
		prevt = t;
		t = time(NULL);
		if (t >= timeout)
			/* TODO error message */
			goto fail;

		goto retry;
	}

 prepare_kernel_data:

	if (nfsvers == 2) {
		if (status.nfsv2.fhs_status != 0) {
			bb_error_msg("%s:%s failed, reason given by server: %s",
				hostname, pathname,
				nfs_strerror(status.nfsv2.fhs_status));
			goto fail;
		}
		memcpy(data.root.data,
				(char *) status.nfsv2.fhstatus_u.fhs_fhandle,
				NFS_FHSIZE);
		data.root.size = NFS_FHSIZE;
		memcpy(data.old_root.data,
				(char *) status.nfsv2.fhstatus_u.fhs_fhandle,
				NFS_FHSIZE);
	} else {
		fhandle3 *my_fhandle;
		if (status.nfsv3.fhs_status != 0) {
			bb_error_msg("%s:%s failed, reason given by server: %s",
				hostname, pathname,
				nfs_strerror(status.nfsv3.fhs_status));
			goto fail;
		}
		my_fhandle = &status.nfsv3.mountres3_u.mountinfo.fhandle;
		memset(data.old_root.data, 0, NFS_FHSIZE);
		memset(&data.root, 0, sizeof(data.root));
		data.root.size = my_fhandle->fhandle3_len;
		memcpy(data.root.data,
				(char *) my_fhandle->fhandle3_val,
				my_fhandle->fhandle3_len);

		data.flags |= NFS_MOUNT_VER3;
	}

	/* create nfs socket for kernel */

	if (tcp) {
		if (nfs_mount_version < 3) {
			bb_error_msg("NFS over TCP is not supported");
			goto fail;
		}
		fsock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	} else
		fsock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (fsock < 0) {
		bb_perror_msg("nfs socket");
		goto fail;
	}
	if (bindresvport(fsock, 0) < 0) {
		bb_perror_msg("nfs bindresvport");
		goto fail;
	}
	if (port == 0) {
		server_addr.sin_port = PMAPPORT;
		port = pmap_getport(&server_addr, nfsprog, nfsvers,
					tcp ? IPPROTO_TCP : IPPROTO_UDP);
		if (port == 0)
			port = NFS_PORT;
	}
	server_addr.sin_port = htons(port);

	/* prepare data structure for kernel */

	data.fd = fsock;
	memcpy((char *) &data.addr, (char *) &server_addr, sizeof(data.addr));
	strncpy(data.hostname, hostname, sizeof(data.hostname));

	/* clean up */

	auth_destroy(mclient->cl_auth);
	clnt_destroy(mclient);
	close(msock);
	msock = -1;

	if (bg) {
		/* We must wait until mount directory is available */
		struct stat statbuf;
		int delay = 1;
		while (stat(mp->mnt_dir, &statbuf) == -1) {
			if (!daemonized) {
				daemonized = daemonize();
				if (daemonized <= 0) { /* parent or error */
// FIXME: parent doesn't close fsock - ??!
					retval = -daemonized;
					goto ret;
				}
			}
			sleep(delay);	/* 1, 2, 4, 8, 16, 30, ... */
			delay *= 2;
			if (delay > 30)
				delay = 30;
		}
	}

 do_mount: /* perform actual mount */

	mp->mnt_type = (char*)"nfs";
	retval = mount_it_now(mp, vfsflags, (char*)&data);
	goto ret;

 fail:	/* abort */

	if (msock >= 0) {
		if (mclient) {
			auth_destroy(mclient->cl_auth);
			clnt_destroy(mclient);
		}
		close(msock);
	}
	if (fsock >= 0)
		close(fsock);

 ret:
	free(hostname);
	free(mounthost);
	free(filteropts);
	return retval;
}

#else /* !ENABLE_FEATURE_MOUNT_NFS */

/* Never called. Call should be optimized out. */
int nfsmount(struct mntent *mp, long vfsflags, char *filteropts);

#endif /* !ENABLE_FEATURE_MOUNT_NFS */

// Mount one directory.  Handles CIFS, NFS, loopback, autobind, and filesystem
// type detection.  Returns 0 for success, nonzero for failure.
// NB: mp->xxx fields may be trashed on exit
static int singlemount(struct mntent *mp, int ignore_busy)
{
	int rc = -1;
	long vfsflags;
	char *loopFile = 0, *filteropts = 0;
	llist_t *fl = 0;
	struct stat st;

	vfsflags = parse_mount_options(mp->mnt_opts, &filteropts);

	// Treat fstype "auto" as unspecified.

	if (mp->mnt_type && strcmp(mp->mnt_type, "auto") == 0)
		mp->mnt_type = NULL;

	// Might this be a virtual filesystem?

	if (ENABLE_FEATURE_MOUNT_HELPERS
	 && (strchr(mp->mnt_fsname, '#'))
	) {
		char *s, *p, *args[35];
		int n = 0;
// FIXME: does it allow execution of arbitrary commands?!
// What args[0] can end up with?
		for (s = p = mp->mnt_fsname; *s && n < 35-3; ++s) {
			if (s[0] == '#' && s[1] != '#') {
				*s = '\0';
				args[n++] = p;
				p = s + 1;
			}
		}
		args[n++] = p;
		args[n++] = mp->mnt_dir;
		args[n] = NULL;
		rc = wait4pid(xspawn(args));
		goto report_error;
	}

	// Might this be an CIFS filesystem?

	if (ENABLE_FEATURE_MOUNT_CIFS
	 && (!mp->mnt_type || strcmp(mp->mnt_type, "cifs") == 0)
	 && (mp->mnt_fsname[0] == '/' || mp->mnt_fsname[0] == '\\')
	 && mp->mnt_fsname[0] == mp->mnt_fsname[1]
	) {
		len_and_sockaddr *lsa;
		char *ip, *dotted;
		char *s;

		rc = 1;
		// Replace '/' with '\' and verify that unc points to "//server/share".

		for (s = mp->mnt_fsname; *s; ++s)
			if (*s == '/') *s = '\\';

		// get server IP

		s = strrchr(mp->mnt_fsname, '\\');
		if (s <= mp->mnt_fsname+1) goto report_error;
		*s = '\0';
		lsa = host2sockaddr(mp->mnt_fsname+2, 0);
		*s = '\\';
		if (!lsa) goto report_error;

		// insert ip=... option into string flags.

		dotted = xmalloc_sockaddr2dotted_noport(&lsa->u.sa);
		ip = xasprintf("ip=%s", dotted);
		parse_mount_options(ip, &filteropts);

		// compose new unc '\\server-ip\share'
		// (s => slash after hostname)

		mp->mnt_fsname = xasprintf("\\\\%s%s", dotted, s);

		// lock is required
		vfsflags |= MS_MANDLOCK;

		mp->mnt_type = (char*)"cifs";
		rc = mount_it_now(mp, vfsflags, filteropts);
		if (ENABLE_FEATURE_CLEAN_UP) {
			free(mp->mnt_fsname);
			free(ip);
			free(dotted);
			free(lsa);
		}
		goto report_error;
	}

	// Might this be an NFS filesystem?

	if (ENABLE_FEATURE_MOUNT_NFS
	 && (!mp->mnt_type || !strcmp(mp->mnt_type, "nfs"))
	 && strchr(mp->mnt_fsname, ':') != NULL
	) {
		rc = nfsmount(mp, vfsflags, filteropts);
		goto report_error;
	}

	// Look at the file.  (Not found isn't a failure for remount, or for
	// a synthetic filesystem like proc or sysfs.)
	// (We use stat, not lstat, in order to allow
	// mount symlink_to_file_or_blkdev dir)

	if (!stat(mp->mnt_fsname, &st)
	 && !(vfsflags & (MS_REMOUNT | MS_BIND | MS_MOVE))
	) {
		// Do we need to allocate a loopback device for it?

		if (ENABLE_FEATURE_MOUNT_LOOP && S_ISREG(st.st_mode)) {
			loopFile = bb_simplify_path(mp->mnt_fsname);
			mp->mnt_fsname = NULL; /* will receive malloced loop dev name */
			if (set_loop(&(mp->mnt_fsname), loopFile, 0) < 0) {
				if (errno == EPERM || errno == EACCES)
					bb_error_msg(bb_msg_perm_denied_are_you_root);
				else
					bb_perror_msg("cannot setup loop device");
				return errno;
			}

		// Autodetect bind mounts

		} else if (S_ISDIR(st.st_mode) && !mp->mnt_type)
			vfsflags |= MS_BIND;
	}

	/* If we know the fstype (or don't need to), jump straight
	 * to the actual mount. */

	if (mp->mnt_type || (vfsflags & (MS_REMOUNT | MS_BIND | MS_MOVE)))
		rc = mount_it_now(mp, vfsflags, filteropts);
	else {
		// Loop through filesystem types until mount succeeds
		// or we run out

		/* Initialize list of block backed filesystems.  This has to be
		 * done here so that during "mount -a", mounts after /proc shows up
		 * can autodetect. */

		if (!fslist) {
			fslist = get_block_backed_filesystems();
			if (ENABLE_FEATURE_CLEAN_UP && fslist)
				atexit(delete_block_backed_filesystems);
		}

		for (fl = fslist; fl; fl = fl->link) {
			mp->mnt_type = fl->data;
			rc = mount_it_now(mp, vfsflags, filteropts);
			if (!rc) break;
		}
	}

	// If mount failed, clean up loop file (if any).

	if (ENABLE_FEATURE_MOUNT_LOOP && rc && loopFile) {
		del_loop(mp->mnt_fsname);
		if (ENABLE_FEATURE_CLEAN_UP) {
			free(loopFile);
			free(mp->mnt_fsname);
		}
	}

 report_error:
	if (ENABLE_FEATURE_CLEAN_UP)
		free(filteropts);

	if (errno == EBUSY && ignore_busy)
		return 0;
	if (rc < 0)
		bb_perror_msg("mounting %s on %s failed", mp->mnt_fsname, mp->mnt_dir);
	return rc;
}

// Parse options, if necessary parse fstab/mtab, and call singlemount for
// each directory to be mounted.

static const char must_be_root[] ALIGN1 = "you must be root";

int mount_main(int argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
int mount_main(int argc UNUSED_PARAM, char **argv)
{
	char *cmdopts = xstrdup("");
	char *fstype = NULL;
	char *storage_path;
	char *opt_o;
	const char *fstabname;
	FILE *fstab;
	int i, j, rc = 0;
	unsigned opt;
	struct mntent mtpair[2], *mtcur = mtpair;
	SKIP_DESKTOP(const int nonroot = 0;)

	USE_DESKTOP(int nonroot = ) sanitize_env_if_suid();

	// Parse long options, like --bind and --move.  Note that -o option
	// and --option are synonymous.  Yes, this means --remount,rw works.
	for (i = j = 1; argv[i]; i++) {
		if (argv[i][0] == '-' && argv[i][1] == '-')
			append_mount_options(&cmdopts, argv[i] + 2);
		else
			argv[j++] = argv[i];
	}
	argv[j] = NULL;

	// Parse remaining options
	// Max 2 params; -v is a counter
	opt_complementary = "?2" USE_FEATURE_MOUNT_VERBOSE(":vv");
	opt = getopt32(argv, OPTION_STR, &opt_o, &fstype
			USE_FEATURE_MOUNT_VERBOSE(, &verbose));
	if (opt & OPT_o) append_mount_options(&cmdopts, opt_o); // -o
	if (opt & OPT_r) append_mount_options(&cmdopts, "ro"); // -r
	if (opt & OPT_w) append_mount_options(&cmdopts, "rw"); // -w
	argv += optind;

	// If we have no arguments, show currently mounted filesystems
	if (!argv[0]) {
		if (!(opt & OPT_a)) {
			FILE *mountTable = setmntent(bb_path_mtab_file, "r");

			if (!mountTable)
				bb_error_msg_and_die("no %s", bb_path_mtab_file);

			while (getmntent_r(mountTable, &mtpair[0], getmntent_buf,
								GETMNTENT_BUFSIZE))
			{
				// Don't show rootfs. FIXME: why??
				// util-linux 2.12a happily shows rootfs...
				//if (!strcmp(mtpair->mnt_fsname, "rootfs")) continue;

				if (!fstype || !strcmp(mtpair->mnt_type, fstype))
					printf("%s on %s type %s (%s)\n", mtpair->mnt_fsname,
							mtpair->mnt_dir, mtpair->mnt_type,
							mtpair->mnt_opts);
			}
			if (ENABLE_FEATURE_CLEAN_UP)
				endmntent(mountTable);
			return EXIT_SUCCESS;
		}
		storage_path = NULL;
	} else {
		// When we have two arguments, the second is the directory and we can
		// skip looking at fstab entirely.  We can always abspath() the directory
		// argument when we get it.
		if (argv[1]) {
			if (nonroot)
				bb_error_msg_and_die(must_be_root);
			mtpair->mnt_fsname = argv[0];
			mtpair->mnt_dir = argv[1];
			mtpair->mnt_type = fstype;
			mtpair->mnt_opts = cmdopts;
			if (ENABLE_FEATURE_MOUNT_LABEL) {
				resolve_mount_spec(&mtpair->mnt_fsname);
			}
			rc = singlemount(mtpair, 0);
			return rc;
		}
		storage_path = bb_simplify_path(argv[0]); // malloced
	}

	// Past this point, we are handling either "mount -a [opts]"
	// or "mount [opts] single_param"

	i = parse_mount_options(cmdopts, 0); // FIXME: should be "long", not "int"
	if (nonroot && (i & ~MS_SILENT)) // Non-root users cannot specify flags
		bb_error_msg_and_die(must_be_root);

	// If we have a shared subtree flag, don't worry about fstab or mtab.
	if (ENABLE_FEATURE_MOUNT_FLAGS
	 && (i & (MS_SHARED | MS_PRIVATE | MS_SLAVE | MS_UNBINDABLE))
	) {
		rc = verbose_mount(/*source:*/ "", /*target:*/ argv[0],
				/*type:*/ "", /*flags:*/ i, /*data:*/ "");
		if (rc)
			bb_simple_perror_msg_and_die(argv[0]);
		return rc;
	}

	// Open either fstab or mtab
	fstabname = "/etc/fstab";
	if (i & MS_REMOUNT) {
		// WARNING. I am not sure this matches util-linux's
		// behavior. It's possible util-linux does not
		// take -o opts from mtab (takes only mount source).
		fstabname = bb_path_mtab_file;
	}
	fstab = setmntent(fstabname, "r");
	if (!fstab)
		bb_perror_msg_and_die("cannot read %s", fstabname);

	// Loop through entries until we find what we're looking for
	memset(mtpair, 0, sizeof(mtpair));
	for (;;) {
		struct mntent *mtother = (mtcur==mtpair ? mtpair+1 : mtpair);

		// Get next fstab entry
		if (!getmntent_r(fstab, mtcur, getmntent_buf
					+ (mtcur==mtpair ? GETMNTENT_BUFSIZE/2 : 0),
				GETMNTENT_BUFSIZE/2)
		) { // End of fstab/mtab is reached
			mtcur = mtother; // the thing we found last time
			break;
		}

		// If we're trying to mount something specific and this isn't it,
		// skip it.  Note we must match the exact text in fstab (ala
		// "proc") or a full path from root
		if (argv[0]) {

			// Is this what we're looking for?
			if (strcmp(argv[0], mtcur->mnt_fsname) &&
			   strcmp(storage_path, mtcur->mnt_fsname) &&
			   strcmp(argv[0], mtcur->mnt_dir) &&
			   strcmp(storage_path, mtcur->mnt_dir)) continue;

			// Remember this entry.  Something later may have
			// overmounted it, and we want the _last_ match.
			mtcur = mtother;

		// If we're mounting all
		} else {
			// Do we need to match a filesystem type?
			if (fstype && match_fstype(mtcur, fstype))
				continue;

			// Skip noauto and swap anyway.
			if (parse_mount_options(mtcur->mnt_opts, 0) & (MOUNT_NOAUTO | MOUNT_SWAP))
				continue;

			// No, mount -a won't mount anything,
			// even user mounts, for mere humans
			if (nonroot)
				bb_error_msg_and_die(must_be_root);

			// Mount this thing
			if (ENABLE_FEATURE_MOUNT_LABEL)
				resolve_mount_spec(&mtpair->mnt_fsname);

			// NFS mounts want this to be xrealloc-able
			mtcur->mnt_opts = xstrdup(mtcur->mnt_opts);
			if (singlemount(mtcur, 1)) {
				// Count number of failed mounts
				rc++;
			}
			free(mtcur->mnt_opts);
		}
	}

	// End of fstab/mtab is reached.
	// Were we looking for something specific?
	if (argv[0]) {
		// If we didn't find anything, complain
		if (!mtcur->mnt_fsname)
			bb_error_msg_and_die("can't find %s in %s",
				argv[0], fstabname);
		if (nonroot) {
			// fstab must have "users" or "user"
			if (!(parse_mount_options(mtcur->mnt_opts, 0) & MOUNT_USERS))
				bb_error_msg_and_die(must_be_root);
		}

		// Mount the last thing we found
		mtcur->mnt_opts = xstrdup(mtcur->mnt_opts);
		append_mount_options(&(mtcur->mnt_opts), cmdopts);
		if (ENABLE_FEATURE_MOUNT_LABEL) {
			resolve_mount_spec(&mtpair->mnt_fsname);
		}
		rc = singlemount(mtcur, 0);
		if (ENABLE_FEATURE_CLEAN_UP)
			free(mtcur->mnt_opts);
	}

	if (ENABLE_FEATURE_CLEAN_UP)
		endmntent(fstab);
	if (ENABLE_FEATURE_CLEAN_UP) {
		free(storage_path);
		free(cmdopts);
	}
	return rc;
}
