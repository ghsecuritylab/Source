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
/* Copyright 2002 Laurence Anderson
 *
 * Licensed under GPLv2 or later, see file LICENSE in this tarball for details.
 */

#include "libbb.h"
#include "unarchive.h"

typedef struct hardlinks_t {
	struct hardlinks_t *next;
	int inode; /* TODO: must match maj/min too! */
	int mode ;
	int mtime; /* These three are useful only in corner case */
	int uid  ; /* of hardlinks with zero size body */
	int gid  ;
	char name[1];
} hardlinks_t;

char FAST_FUNC get_header_cpio(archive_handle_t *archive_handle)
{
	file_header_t *file_header = archive_handle->file_header;
	char cpio_header[110];
	int namesize;
	int major, minor, nlink, mode, inode;
	unsigned size, uid, gid, mtime;

#define hardlinks_to_create (*(hardlinks_t **)(&archive_handle->ah_priv[0]))
#define created_hardlinks   (*(hardlinks_t **)(&archive_handle->ah_priv[1]))
//	if (!archive_handle->ah_priv_inited) {
//		archive_handle->ah_priv_inited = 1;
//		hardlinks_to_create = NULL;
//		created_hardlinks = NULL;
//	}

	/* There can be padding before archive header */
	data_align(archive_handle, 4);

	size = full_read(archive_handle->src_fd, cpio_header, 110);
	if (size == 0) {
		goto create_hardlinks;
	}
	if (size != 110) {
		bb_error_msg_and_die("short read");
	}
	archive_handle->offset += 110;

	if (strncmp(&cpio_header[0], "07070", 5) != 0
	 || (cpio_header[5] != '1' && cpio_header[5] != '2')
	) {
		bb_error_msg_and_die("unsupported cpio format, use newc or crc");
	}

	if (sscanf(cpio_header + 6,
			"%8x" "%8x" "%8x" "%8x"
			"%8x" "%8x" "%8x" /*maj,min:*/ "%*16c"
			/*rmaj,rmin:*/"%8x" "%8x" "%8x" /*chksum: "%*8c"*/,
			&inode, &mode, &uid, &gid,
			&nlink, &mtime, &size,
			&major, &minor, &namesize) != 10)
		bb_error_msg_and_die("damaged cpio file");
	file_header->mode = mode;
	file_header->uid = uid;
	file_header->gid = gid;
	file_header->mtime = mtime;
	file_header->size = size;

	namesize &= 0x1fff; /* paranoia: limit names to 8k chars */
	file_header->name = xzalloc(namesize + 1);
	/* Read in filename */
	xread(archive_handle->src_fd, file_header->name, namesize);
	archive_handle->offset += namesize;

	/* Update offset amount and skip padding before file contents */
	data_align(archive_handle, 4);

	if (strcmp(file_header->name, "TRAILER!!!") == 0) {
		/* Always round up. ">> 9" divides by 512 */
		printf("%"OFF_FMT"u blocks\n", (archive_handle->offset + 511) >> 9);
		goto create_hardlinks;
	}

	file_header->link_target = NULL;
	if (S_ISLNK(file_header->mode)) {
		file_header->size &= 0x1fff; /* paranoia: limit names to 8k chars */
		file_header->link_target = xzalloc(file_header->size + 1);
		xread(archive_handle->src_fd, file_header->link_target, file_header->size);
		archive_handle->offset += file_header->size;
		file_header->size = 0; /* Stop possible seeks in future */
	}

// TODO: data_extract_all can't deal with hardlinks to non-files...
// when fixed, change S_ISREG to !S_ISDIR here

	if (nlink > 1 && S_ISREG(file_header->mode)) {
		hardlinks_t *new = xmalloc(sizeof(*new) + namesize);
		new->inode = inode;
		new->mode  = mode ;
		new->mtime = mtime;
		new->uid   = uid  ;
		new->gid   = gid  ;
		strcpy(new->name, file_header->name);
		/* Put file on a linked list for later */
		if (size == 0) {
			new->next = hardlinks_to_create;
			hardlinks_to_create = new;
			return EXIT_SUCCESS; /* Skip this one */
			/* TODO: this breaks cpio -t (it does not show hardlinks) */
		}
		new->next = created_hardlinks;
		created_hardlinks = new;
	}
	file_header->device = makedev(major, minor);

	if (archive_handle->filter(archive_handle) == EXIT_SUCCESS) {
		archive_handle->action_data(archive_handle);
		archive_handle->action_header(file_header);
	} else {
		data_skip(archive_handle);
	}

	archive_handle->offset += file_header->size;

	free(file_header->link_target);
	free(file_header->name);
	file_header->link_target = NULL;
	file_header->name = NULL;

	return EXIT_SUCCESS;

 create_hardlinks:
	free(file_header->link_target);
	free(file_header->name);

	while (hardlinks_to_create) {
		hardlinks_t *cur;
		hardlinks_t *make_me = hardlinks_to_create;

		hardlinks_to_create = make_me->next;

		memset(file_header, 0, sizeof(*file_header));
		file_header->mtime = make_me->mtime;
		file_header->name = make_me->name;
		file_header->mode = make_me->mode;
		file_header->uid = make_me->uid;
		file_header->gid = make_me->gid;
		/*file_header->size = 0;*/
		/*file_header->link_target = NULL;*/

		/* Try to find a file we are hardlinked to */
		cur = created_hardlinks;
		while (cur) {
			/* TODO: must match maj/min too! */
			if (cur->inode == make_me->inode) {
				file_header->link_target = cur->name;
				 /* link_target != NULL, size = 0: "I am a hardlink" */
				if (archive_handle->filter(archive_handle) == EXIT_SUCCESS)
					archive_handle->action_data(archive_handle);
				free(make_me);
				goto next_link;
			}
			cur = cur->next;
		}
		/* Oops... no file with such inode was created... do it now
		 * (happens when hardlinked files are empty (zero length)) */
		if (archive_handle->filter(archive_handle) == EXIT_SUCCESS)
			archive_handle->action_data(archive_handle);
		/* Move to the list of created hardlinked files */
		make_me->next = created_hardlinks;
		created_hardlinks = make_me;
 next_link: ;
	}

	while (created_hardlinks) {
		hardlinks_t *p = created_hardlinks;
		created_hardlinks = p->next;
		free(p);
	}

	return EXIT_FAILURE; /* "No more files to process" */
}
