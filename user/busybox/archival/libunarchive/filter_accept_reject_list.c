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
 * Copyright (C) 2002 by Glenn McGrath
 *
 * Licensed under GPLv2 or later, see file LICENSE in this tarball for details.
 */

#include "libbb.h"
#include "unarchive.h"

/*
 * Accept names that are in the accept list and not in the reject list
 */
char FAST_FUNC filter_accept_reject_list(archive_handle_t *archive_handle)
{
	const char *key;
	const llist_t *reject_entry;
	const llist_t *accept_entry;

	key = archive_handle->file_header->name;

	/* If the key is in a reject list fail */
	reject_entry = find_list_entry2(archive_handle->reject, key);
	if (reject_entry) {
		return EXIT_FAILURE;
	}
	accept_entry = find_list_entry2(archive_handle->accept, key);

	/* Fail if an accept list was specified and the key wasnt in there */
	if ((accept_entry == NULL) && archive_handle->accept) {
		return EXIT_FAILURE;
	}

	/* Accepted */
	return EXIT_SUCCESS;
}
