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
 * Copyright (C) 2005 Kay Sievers <kay.sievers@vrfy.org>
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

void volume_id_set_unicode16(char *str, size_t len, const uint8_t *buf, enum endian endianess, size_t count)
{
	unsigned i, j;
	unsigned c;

	j = 0;
	for (i = 0; i + 2 <= count; i += 2) {
		if (endianess == LE)
			c = (buf[i+1] << 8) | buf[i];
		else
			c = (buf[i] << 8) | buf[i+1];
		if (c == 0) {
			str[j] = '\0';
			break;
		} else if (c < 0x80) {
			if (j+1 >= len)
				break;
			str[j++] = (uint8_t) c;
		} else if (c < 0x800) {
			if (j+2 >= len)
				break;
			str[j++] = (uint8_t) (0xc0 | (c >> 6));
			str[j++] = (uint8_t) (0x80 | (c & 0x3f));
		} else {
			if (j+3 >= len)
				break;
			str[j++] = (uint8_t) (0xe0 | (c >> 12));
			str[j++] = (uint8_t) (0x80 | ((c >> 6) & 0x3f));
			str[j++] = (uint8_t) (0x80 | (c & 0x3f));
		}
	}
	str[j] = '\0';
}

#ifdef UNUSED
static const char *usage_to_string(enum volume_id_usage usage_id)
{
	switch (usage_id) {
	case VOLUME_ID_FILESYSTEM:
		return "filesystem";
	case VOLUME_ID_PARTITIONTABLE:
		return "partitiontable";
	case VOLUME_ID_OTHER:
		return "other";
	case VOLUME_ID_RAID:
		return "raid";
	case VOLUME_ID_DISKLABEL:
		return "disklabel";
	case VOLUME_ID_CRYPTO:
		return "crypto";
	case VOLUME_ID_UNPROBED:
		return "unprobed";
	case VOLUME_ID_UNUSED:
		return "unused";
	}
	return NULL;
}

void volume_id_set_usage_part(struct volume_id_partition *part, enum volume_id_usage usage_id)
{
	part->usage_id = usage_id;
	part->usage = usage_to_string(usage_id);
}

void volume_id_set_usage(struct volume_id *id, enum volume_id_usage usage_id)
{
	id->usage_id = usage_id;
	id->usage = usage_to_string(usage_id);
}

void volume_id_set_label_raw(struct volume_id *id, const uint8_t *buf, size_t count)
{
	memcpy(id->label_raw, buf, count);
	id->label_raw_len = count;
}
#endif

#ifdef NOT_NEEDED
static size_t strnlen(const char *s, size_t maxlen)
{
	size_t i;
	if (!maxlen) return 0;
	if (!s) return 0;
	for (i = 0; *s && i < maxlen; ++s) ++i;
	return i;
}
#endif

void volume_id_set_label_string(struct volume_id *id, const uint8_t *buf, size_t count)
{
	unsigned i;

	memcpy(id->label, buf, count);

	/* remove trailing whitespace */
	i = strnlen(id->label, count);
	while (i--) {
		if (!isspace(id->label[i]))
			break;
	}
	id->label[i+1] = '\0';
}

void volume_id_set_label_unicode16(struct volume_id *id, const uint8_t *buf, enum endian endianess, size_t count)
{
	 volume_id_set_unicode16(id->label, sizeof(id->label), buf, endianess, count);
}

void volume_id_set_uuid(struct volume_id *id, const uint8_t *buf, enum uuid_format format)
{
	unsigned i;
	unsigned count = 0;

	switch (format) {
	case UUID_DOS:
		count = 4;
		break;
	case UUID_NTFS:
	case UUID_HFS:
		count = 8;
		break;
	case UUID_DCE:
		count = 16;
		break;
	case UUID_DCE_STRING:
		/* 36 is ok, id->uuid has one extra byte for NUL */
		count = VOLUME_ID_UUID_SIZE;
		break;
	}
//	memcpy(id->uuid_raw, buf, count);
//	id->uuid_raw_len = count;

	/* if set, create string in the same format, the native platform uses */
	for (i = 0; i < count; i++)
		if (buf[i] != 0)
			goto set;
	return; /* all bytes are zero, leave it empty ("") */

set:
	switch (format) {
	case UUID_DOS:
		sprintf(id->uuid, "%02X%02X-%02X%02X",
			buf[3], buf[2], buf[1], buf[0]);
		break;
	case UUID_NTFS:
		sprintf(id->uuid, "%02X%02X%02X%02X%02X%02X%02X%02X",
			buf[7], buf[6], buf[5], buf[4],
			buf[3], buf[2], buf[1], buf[0]);
		break;
	case UUID_HFS:
		sprintf(id->uuid, "%02X%02X%02X%02X%02X%02X%02X%02X",
			buf[0], buf[1], buf[2], buf[3],
			buf[4], buf[5], buf[6], buf[7]);
		break;
	case UUID_DCE:
		sprintf(id->uuid,
			"%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
			buf[0], buf[1], buf[2], buf[3],
			buf[4], buf[5],
			buf[6], buf[7],
			buf[8], buf[9],
			buf[10], buf[11], buf[12], buf[13], buf[14],buf[15]);
		break;
	case UUID_DCE_STRING:
		memcpy(id->uuid, buf, count);
		id->uuid[count] = '\0';
		break;
	}
}

void *volume_id_get_buffer(struct volume_id *id, uint64_t off, size_t len)
{
	ssize_t buf_len;

	dbg("get buffer off 0x%llx(%llu), len 0x%zx", (unsigned long long) off, (unsigned long long) off, len);
	/* check if requested area fits in superblock buffer */
	if (off + len <= SB_BUFFER_SIZE) {
		if (id->sbbuf == NULL) {
			id->sbbuf = xmalloc(SB_BUFFER_SIZE);
		}

		/* check if we need to read */
		if ((off + len) > id->sbbuf_len) {
			dbg("read sbbuf len:0x%llx", (unsigned long long) (off + len));
			xlseek(id->fd, 0, SEEK_SET);
			buf_len = full_read(id->fd, id->sbbuf, off + len);
			if (buf_len < 0) {
				dbg("read failed (%s)", strerror(errno));
				return NULL;
			}
			dbg("got 0x%zx (%zi) bytes", buf_len, buf_len);
			id->sbbuf_len = buf_len;
			if ((uint64_t)buf_len < off + len) {
				dbg("requested 0x%zx bytes, got only 0x%zx bytes", len, buf_len);
				return NULL;
			}
		}

		return &(id->sbbuf[off]);
	}

	if (len > SEEK_BUFFER_SIZE) {
		dbg("seek buffer too small %d", SEEK_BUFFER_SIZE);
		return NULL;
	}

	/* get seek buffer */
	if (id->seekbuf == NULL) {
		id->seekbuf = xmalloc(SEEK_BUFFER_SIZE);
	}

	/* check if we need to read */
	if ((off < id->seekbuf_off) || ((off + len) > (id->seekbuf_off + id->seekbuf_len))) {
		dbg("read seekbuf off:0x%llx len:0x%zx", (unsigned long long) off, len);
		xlseek(id->fd, off, SEEK_SET);
		buf_len = full_read(id->fd, id->seekbuf, len);
		if (buf_len < 0) {
			dbg("read failed (%s)", strerror(errno));
			return NULL;
		}
		dbg("got 0x%zx (%zi) bytes", buf_len, buf_len);
		id->seekbuf_off = off;
		id->seekbuf_len = buf_len;
		if ((size_t)buf_len < len) {
			dbg("requested 0x%zx bytes, got only 0x%zx bytes", len, buf_len);
			return NULL;
		}
	}

	return &(id->seekbuf[off - id->seekbuf_off]);
}

void volume_id_free_buffer(struct volume_id *id)
{
	free(id->sbbuf);
	id->sbbuf = NULL;
	id->sbbuf_len = 0;
	free(id->seekbuf);
	id->seekbuf = NULL;
	id->seekbuf_len = 0;
}
