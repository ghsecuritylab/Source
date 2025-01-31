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
 * WPA Supplicant / Example program entrypoint
 * Copyright (c) 2003-2005, Jouni Malinen <jkmaline@cc.hut.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 */

#include "includes.h"

#include "common.h"
#include "wpa_supplicant_i.h"

int main(int argc, char *argv[])
{
	struct wpa_interface iface;
	int exitcode = 0;
	struct wpa_params params;
	struct wpa_global *global;

	memset(&params, 0, sizeof(params));
	params.wpa_debug_level = MSG_INFO;

	global = wpa_supplicant_init(&params);
	if (global == NULL)
		return -1;

	memset(&iface, 0, sizeof(iface));
	/* TODO: set interface parameters */

	if (wpa_supplicant_add_iface(global, &iface) == NULL)
		exitcode = -1;

	if (exitcode == 0)
		exitcode = wpa_supplicant_run(global);

	wpa_supplicant_deinit(global);

	return exitcode;
}
