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
/*
 * This is the original minix inode layout on disk.
 * Note the 8-bit gid and atime and ctime.
 */
struct minix1_inode {
	uint16_t i_mode;
	uint16_t i_uid;
	uint32_t i_size;
	uint32_t i_time;
	uint8_t  i_gid;
	uint8_t  i_nlinks;
	uint16_t i_zone[9];
};

/*
 * The new minix inode has all the time entries, as well as
 * long block numbers and a third indirect block (7+1+1+1
 * instead of 7+1+1). Also, some previously 8-bit values are
 * now 16-bit. The inode is now 64 bytes instead of 32.
 */
struct minix2_inode {
	uint16_t i_mode;
	uint16_t i_nlinks;
	uint16_t i_uid;
	uint16_t i_gid;
	uint32_t i_size;
	uint32_t i_atime;
	uint32_t i_mtime;
	uint32_t i_ctime;
	uint32_t i_zone[10];
};

/*
 * minix superblock data on disk
 */
struct minix_superblock {
	uint16_t s_ninodes;
	uint16_t s_nzones;
	uint16_t s_imap_blocks;
	uint16_t s_zmap_blocks;
	uint16_t s_firstdatazone;
	uint16_t s_log_zone_size;
	uint32_t s_max_size;
	uint16_t s_magic;
	uint16_t s_state;
	uint32_t s_zones;
};

struct minix_dir_entry {
	uint16_t inode;
	char name[0];
};

/* Believe it or not, but mount.h has this one #defined */
#undef BLOCK_SIZE

enum {
	BLOCK_SIZE              = 1024,
	BITS_PER_BLOCK          = BLOCK_SIZE << 3,

	MINIX_ROOT_INO          = 1,
	MINIX_BAD_INO           = 2,

	MINIX1_SUPER_MAGIC      = 0x137F,       /* original minix fs */
	MINIX1_SUPER_MAGIC2     = 0x138F,       /* minix fs, 30 char names */
	MINIX2_SUPER_MAGIC      = 0x2468,       /* minix V2 fs */
	MINIX2_SUPER_MAGIC2     = 0x2478,       /* minix V2 fs, 30 char names */
	MINIX_VALID_FS          = 0x0001,       /* clean fs */
	MINIX_ERROR_FS          = 0x0002,       /* fs has errors */

	INODE_SIZE1             = sizeof(struct minix1_inode),
	INODE_SIZE2             = sizeof(struct minix2_inode),
	MINIX1_INODES_PER_BLOCK = BLOCK_SIZE / sizeof(struct minix1_inode),
	MINIX2_INODES_PER_BLOCK = BLOCK_SIZE / sizeof(struct minix2_inode),
};

/*
Basic test script for regressions in mkfs/fsck.
Copies current dir into image (typically bbox build tree).

#!/bin/sh
tmpdir=/tmp/minixtest-$$
tmpimg=/tmp/minix-img-$$

mkdir $tmpdir
dd if=/dev/zero of=$tmpimg bs=1M count=20 || exit
./busybox mkfs.minix $tmpimg || exit
mount -o loop $tmpimg $tmpdir || exit
cp -a "$PWD" $tmpdir
umount $tmpdir || exit
./busybox fsck.minix -vfm $tmpimg || exit
echo "Continue?"
read junk
./busybox fsck.minix -vfml $tmpimg || exit
rmdir $tmpdir
rm $tmpimg

*/
