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
 * e2image.h --- header file describing the ext2 image format
 *
 * Copyright (C) 2000 Theodore Ts'o.
 *
 * Note: this uses the POSIX IO interfaces, unlike most of the other
 * functions in this library.  So sue me.
 *
 * %Begin-Header%
 * This file may be redistributed under the terms of the GNU Public
 * License.
 * %End-Header%
 */


struct ext2_image_hdr {
	__u32	magic_number;	/* This must be EXT2_ET_MAGIC_E2IMAGE */
	char	magic_descriptor[16]; /* "Ext2 Image 1.0", w/ null padding */
	char	fs_hostname[64];/* Hostname of machine of image */
	char	fs_netaddr[32];	/* Network address */
	__u32	fs_netaddr_type;/* 0 = IPV4, 1 = IPV6, etc. */
	__u32	fs_device;	/* Device number of image */
	char	fs_device_name[64]; /* Device name */
	char	fs_uuid[16];	/* UUID of filesystem */
	__u32	fs_blocksize;	/* Block size of the filesystem */
	__u32	fs_reserved[8];

	__u32	image_device;	/* Device number of image file */
	__u32	image_inode;	/* Inode number of image file */
	__u32	image_time;	/* Time of image creation */
	__u32	image_reserved[8];

	__u32	offset_super;	/* Byte offset of the sb and descriptors */
	__u32	offset_inode;	/* Byte offset of the inode table  */
	__u32	offset_inodemap; /* Byte offset of the inode bitmaps */
	__u32	offset_blockmap; /* Byte offset of the inode bitmaps */
	__u32	offset_reserved[8];
};













