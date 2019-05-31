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
 * Copyright 2013, ASUSTek Inc.
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of ASUSTek Inc.;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of ASUSTek Inc..
 */

#include <stdio.h>
#include <signal.h>
#include <nvram/bcmnvram.h>
#include "process.h"

typedef unsigned char bool;

/* @return:
 *  0: success
 * -1: invalid parameter
 *  1: wait pending rc_service timeout
 */
static int notify_rc_internal(const char *event_name, bool do_wait, const int wait)
{
	int i;
	int _pid = getpid();
	char _proc[64];

	if (!event_name)
		return -1;

	strcpy(_proc, find_proc_name_by_pid(_pid));
	fprintf(stderr, "[rc_service] %s(%d) notify_rc(%s)\n", _proc, _pid, event_name);

	i = wait;
	while ((!nvram_match("rc_service", "")) && (i-- > 0)) {
		fprintf(stderr, "[rc_service] wait for previous notify_rc(%s) ... %d/%d\n", nvram_safe_get("rc_service"), i, wait);
		sleep(1);
	}

	if (i <= 0) {
		fprintf(stderr, "[rc_service] skip the event: %s!\n", event_name);
		return 1;
	}

	nvram_set("rc_service", event_name);
	kill(1, SIGUSR1);

	if (do_wait) {
		i = wait;
		while ((nvram_match("rc_service", (char *)event_name)) && (i-- > 0)) {
			fprintf(stderr, "[rc_service] wait for currnet notify_rc(%s) ... %d/%d\n", event_name, i, wait);
			sleep(1);
		}
	}

	return 0;
}

int notify_rc(const char *event_name)
{
	return notify_rc_internal(event_name, FALSE, 15);
}

int notify_rc_and_wait(const char *event_name)
{
	return notify_rc_internal(event_name, TRUE, 15);
}
