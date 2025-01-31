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
 * taskset - retrieve or set a processes' CPU affinity
 * Copyright (c) 2006 Bernhard Fischer
 *
 * Licensed under GPLv2 or later, see file LICENSE in this tarball for details.
 */

#include <sched.h>
#include "libbb.h"

#if ENABLE_FEATURE_TASKSET_FANCY
#define TASKSET_PRINTF_MASK "%s"
/* craft a string from the mask */
static char *from_cpuset(cpu_set_t *mask)
{
	int i;
	char *ret = NULL;
	char *str = xzalloc((CPU_SETSIZE / 4) + 1); /* we will leak it */

	for (i = CPU_SETSIZE - 4; i >= 0; i -= 4) {
		int val = 0;
		int off;
		for (off = 0; off <= 3; ++off)
			if (CPU_ISSET(i + off, mask))
				val |= 1 << off;
		if (!ret && val)
			ret = str;
		*str++ = bb_hexdigits_upcase[val] | 0x20;
	}
	return ret;
}
#else
#define TASKSET_PRINTF_MASK "%llx"
static unsigned long long from_cpuset(cpu_set_t *mask)
{
	struct BUG_CPU_SETSIZE_is_too_small {
		char BUG_CPU_SETSIZE_is_too_small[
			CPU_SETSIZE < sizeof(int) ? -1 : 1];
	};
	char *p = (void*)mask;

	/* Take the least significant bits. Careful!
	 * Consider both CPU_SETSIZE=4 and CPU_SETSIZE=1024 cases
	 */
#if BB_BIG_ENDIAN
	/* For big endian, it means LAST bits */
	if (CPU_SETSIZE < sizeof(long))
		p += CPU_SETSIZE - sizeof(int);
	else if (CPU_SETSIZE < sizeof(long long))
		p += CPU_SETSIZE - sizeof(long);
	else
		p += CPU_SETSIZE - sizeof(long long);
#endif
	if (CPU_SETSIZE < sizeof(long))
		return *(unsigned*)p;
	if (CPU_SETSIZE < sizeof(long long))
		return *(unsigned long*)p;
	return *(unsigned long long*)p;
}
#endif


int taskset_main(int argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
int taskset_main(int argc UNUSED_PARAM, char **argv)
{
	cpu_set_t mask;
	pid_t pid = 0;
	unsigned opt_p;
	const char *current_new;
	char *pid_str;
	char *aff = aff; /* for compiler */

	/* NB: we mimic util-linux's taskset: -p does not take
	 * an argument, i.e., "-pN" is NOT valid, only "-p N"!
	 * Indeed, util-linux-2.13-pre7 uses:
	 * getopt_long(argc, argv, "+pchV", ...), not "...p:..." */

	opt_complementary = "-1"; /* at least 1 arg */
	opt_p = getopt32(argv, "+p");
	argv += optind;

	if (opt_p) {
		pid_str = *argv++;
		if (*argv) { /* "-p <aff> <pid> ...rest.is.ignored..." */
			aff = pid_str;
			pid_str = *argv; /* NB: *argv != NULL in this case */
		}
		/* else it was just "-p <pid>", and *argv == NULL */
		pid = xatoul_range(pid_str, 1, ((unsigned)(pid_t)ULONG_MAX) >> 1);
	} else {
		aff = *argv++; /* <aff> <cmd...> */
		if (!*argv)
			bb_show_usage();
	}

	current_new = "current\0new";
	if (opt_p) {
 print_aff:
		if (sched_getaffinity(pid, sizeof(mask), &mask) < 0)
			bb_perror_msg_and_die("can't %cet pid %d's affinity", 'g', pid);
		printf("pid %d's %s affinity mask: "TASKSET_PRINTF_MASK"\n",
				pid, current_new, from_cpuset(&mask));
		if (!*argv) {
			/* Either it was just "-p <pid>",
			 * or it was "-p <aff> <pid>" and we came here
			 * for the second time (see goto below) */
			return EXIT_SUCCESS;
		}
		*argv = NULL;
		current_new += 8; /* "new" */
	}

	{ /* Affinity was specified, translate it into cpu_set_t */
		unsigned i;
		/* Do not allow zero mask: */
		unsigned long long m = xstrtoull_range(aff, 0, 1, ULLONG_MAX);
		enum { CNT_BIT = CPU_SETSIZE < sizeof(m)*8 ? CPU_SETSIZE : sizeof(m)*8 };

		CPU_ZERO(&mask);
		for (i = 0; i < CNT_BIT; i++) {
			unsigned long long bit = (1ULL << i);
			if (bit & m)
				CPU_SET(i, &mask);
		}
	}

	/* Set pid's or our own (pid==0) affinity */
	if (sched_setaffinity(pid, sizeof(mask), &mask))
		bb_perror_msg_and_die("can't %cet pid %d's affinity", 's', pid);

	if (!*argv) /* "-p <aff> <pid> [...ignored...]" */
		goto print_aff; /* print new affinity and exit */

	BB_EXECVP(*argv, argv);
	bb_simple_perror_msg_and_die(*argv);
}
