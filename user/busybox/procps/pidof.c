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
 * pidof implementation for busybox
 *
 * Copyright (C) 1999-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * Licensed under the GPL version 2, see the file LICENSE in this tarball.
 */

#include "libbb.h"

enum {
	USE_FEATURE_PIDOF_SINGLE(OPTBIT_SINGLE,)
	USE_FEATURE_PIDOF_OMIT(  OPTBIT_OMIT  ,)
	OPT_SINGLE = USE_FEATURE_PIDOF_SINGLE((1<<OPTBIT_SINGLE)) + 0,
	OPT_OMIT   = USE_FEATURE_PIDOF_OMIT(  (1<<OPTBIT_OMIT  )) + 0,
};

int pidof_main(int argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
int pidof_main(int argc UNUSED_PARAM, char **argv)
{
	unsigned first = 1;
	unsigned opt;
#if ENABLE_FEATURE_PIDOF_OMIT
	llist_t *omits = NULL; /* list of pids to omit */
	opt_complementary = "o::";
#endif

	/* do unconditional option parsing */
	opt = getopt32(argv, ""
			USE_FEATURE_PIDOF_SINGLE ("s")
			USE_FEATURE_PIDOF_OMIT("o:", &omits));

#if ENABLE_FEATURE_PIDOF_OMIT
	/* fill omit list.  */
	{
		llist_t *omits_p = omits;
		while (omits_p) {
			/* are we asked to exclude the parent's process ID?  */
			if (strcmp(omits_p->data, "%PPID") == 0) {
				omits_p->data = utoa((unsigned)getppid());
			}
			omits_p = omits_p->link;
		}
	}
#endif
	/* Looks like everything is set to go.  */
	argv += optind;
	while (*argv) {
		pid_t *pidList;
		pid_t *pl;

		/* reverse the pidlist like GNU pidof does.  */
		pidList = pidlist_reverse(find_pid_by_name(*argv));
		for (pl = pidList; *pl; pl++) {
#if ENABLE_FEATURE_PIDOF_OMIT
			if (opt & OPT_OMIT) {
				llist_t *omits_p = omits;
				while (omits_p) {
					if (xatoul(omits_p->data) == (unsigned long)(*pl)) {
						goto omitting;
					}
					omits_p = omits_p->link;
				}
			}
#endif
			printf(" %u" + first, (unsigned)*pl);
			first = 0;
			if (ENABLE_FEATURE_PIDOF_SINGLE && (opt & OPT_SINGLE))
				break;
#if ENABLE_FEATURE_PIDOF_OMIT
 omitting: ;
#endif
		}
		free(pidList);
		argv++;
	}
	if (!first)
		bb_putchar('\n');

#if ENABLE_FEATURE_PIDOF_OMIT
	if (ENABLE_FEATURE_CLEAN_UP)
		llist_free(omits, NULL);
#endif
	return first; /* 1 (failure) - no processes found */
}
