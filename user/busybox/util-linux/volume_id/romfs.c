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
 * volume_id - reads filesystem label and uuid
 *
 * Copyright (C) 2004 Kay Sievers <kay.sievers@vrfy.org>
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation; either
 *	version 2.1 of the License, or (at your option) any later version.
 *
 *	This library is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *	Lesser General Public License for more details.
 *
 *	You should have received a copy of the GNU Lesser General Public
 *	License along with this library; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "volume_id_internal.h"

struct romfs_super {
	uint8_t magic[8];
	uint32_t size;
	uint32_t checksum;
	uint8_t name[0];
} __attribute__((__packed__));

int volume_id_probe_romfs(struct volume_id *id, uint64_t off)
{
	struct romfs_super *rfs;

	dbg("probing at offset 0x%llx", (unsigned long long) off);

	rfs = volume_id_get_buffer(id, off, 0x200);
	if (rfs == NULL)
		return -1;

	if (memcmp(rfs->magic, "-rom1fs-", 4) == 0) {
		size_t len = strlen((char *)rfs->name);

		if (len) {
//			volume_id_set_label_raw(id, rfs->name, len);
			volume_id_set_label_string(id, rfs->name, len);
		}

//		volume_id_set_usage(id, VOLUME_ID_FILESYSTEM);
//		id->type = "romfs";
		return 0;
	}

	return -1;
}
