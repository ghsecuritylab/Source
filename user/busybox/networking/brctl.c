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
 * Small implementation of brctl for busybox.
 *
 * Copyright (C) 2008 by Bernhard Fischer
 *
 * Some helper functions from bridge-utils are
 * Copyright (C) 2000 Lennert Buytenhek
 *
 * Licensed under GPLv2 or later, see file LICENSE in this tarball for details.
 */
/* This applet currently uses only the ioctl interface and no sysfs at all.
 * At the time of this writing this was considered a feature.
 */
#include "libbb.h"
#include <linux/sockios.h>
#include <net/if.h>

/* Maximum number of ports supported per bridge interface.  */
#ifndef MAX_PORTS
#define MAX_PORTS 32
#endif

/* Use internal number parsing and not the "exact" conversion.  */
/* #define BRCTL_USE_INTERNAL 0 */ /* use exact conversion */
#define BRCTL_USE_INTERNAL 1

#if ENABLE_FEATURE_BRCTL_FANCY
#include <linux/if_bridge.h>

/* FIXME: These 4 funcs are not really clean and could be improved */
static ALWAYS_INLINE void strtotimeval(struct timeval *tv,
		const char *time_str)
{
	double secs;
#if BRCTL_USE_INTERNAL
	secs = /*bb_*/strtod(time_str, NULL);
	if (!secs)
#else
	if (sscanf(time_str, "%lf", &secs) != 1)
#endif
		bb_error_msg_and_die (bb_msg_invalid_arg, time_str, "timespec");
	tv->tv_sec = secs;
	tv->tv_usec = 1000000 * (secs - tv->tv_sec);
}

static ALWAYS_INLINE unsigned long __tv_to_jiffies(const struct timeval *tv)
{
	unsigned long long jif;

	jif = 1000000ULL * tv->tv_sec + tv->tv_usec;

	return jif/10000;
}
# if 0
static void __jiffies_to_tv(struct timeval *tv, unsigned long jiffies)
{
	unsigned long long tvusec;

	tvusec = 10000ULL*jiffies;
	tv->tv_sec = tvusec/1000000;
	tv->tv_usec = tvusec - 1000000 * tv->tv_sec;
}
# endif
static unsigned long str_to_jiffies(const char *time_str)
{
	struct timeval tv;
	strtotimeval(&tv, time_str);
	return __tv_to_jiffies(&tv);
}

static void arm_ioctl(unsigned long *args,
		unsigned long arg0, unsigned long arg1, unsigned long arg2)
{
	args[0] = arg0;
	args[1] = arg1;
	args[2] = arg2;
	args[3] = 0;
}
#endif


int brctl_main(int argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
int brctl_main(int argc UNUSED_PARAM, char **argv)
{
	static const char keywords[] ALIGN1 =
		"addbr\0" "delbr\0" "addif\0" "delif\0"
	USE_FEATURE_BRCTL_FANCY(
		"stp\0"
		"setageing\0" "setfd\0" "sethello\0" "setmaxage\0"
		"setpathcost\0" "setportprio\0" "setbridgeprio\0"
	)
	USE_FEATURE_BRCTL_SHOW("showmacs\0" "show\0");

	enum { ARG_addbr = 0, ARG_delbr, ARG_addif, ARG_delif
		USE_FEATURE_BRCTL_FANCY(,
		   ARG_stp,
		   ARG_setageing, ARG_setfd, ARG_sethello, ARG_setmaxage,
		   ARG_setpathcost, ARG_setportprio, ARG_setbridgeprio
		)
		USE_FEATURE_BRCTL_SHOW(, ARG_showmacs, ARG_show)
	};

	int fd;
	smallint key;
	struct ifreq ifr;
	char *br, *brif;

	argv++;
	while (*argv) {
#if ENABLE_FEATURE_BRCTL_FANCY
		int ifidx[MAX_PORTS];
		unsigned long args[4];
		ifr.ifr_data = (char *) &args;
#endif

		key = index_in_strings(keywords, *argv);
		if (key == -1) /* no match found in keywords array, bail out. */
			bb_error_msg_and_die(bb_msg_invalid_arg, *argv, applet_name);
		argv++;
		fd = xsocket(AF_INET, SOCK_STREAM, 0);

#if ENABLE_FEATURE_BRCTL_SHOW
		if (key == ARG_show) { /* show */
			char brname[IFNAMSIZ];
			int bridx[MAX_PORTS];
			int i, num;
			arm_ioctl(args, BRCTL_GET_BRIDGES,
						(unsigned long) bridx, MAX_PORTS);
			num = xioctl(fd, SIOCGIFBR, args);
			printf("bridge name\tbridge id\t\tSTP enabled\tinterfaces\n");
			for (i = 0; i < num; i++) {
				char ifname[IFNAMSIZ];
				int j, tabs;
				struct __bridge_info bi;
				unsigned char *x;

				if (!if_indextoname(bridx[i], brname))
					bb_perror_msg_and_die("can't get bridge name for index %d", i);
				strncpy(ifr.ifr_name, brname, IFNAMSIZ);

				arm_ioctl(args, BRCTL_GET_BRIDGE_INFO,
							(unsigned long) &bi, 0);
				xioctl(fd, SIOCDEVPRIVATE, &ifr);
				printf("%s\t\t", brname);

				/* print bridge id */
				x = (unsigned char *) &bi.bridge_id;
				for (j = 0; j < 8; j++) {
					printf("%.2x", x[j]);
					if (j == 1)
						bb_putchar('.');
				}
				printf(bi.stp_enabled ? "\tyes" : "\tno");

				/* print interface list */
				arm_ioctl(args, BRCTL_GET_PORT_LIST,
							(unsigned long) ifidx, MAX_PORTS);
				xioctl(fd, SIOCDEVPRIVATE, &ifr);
				tabs = 0;
				for (j = 0; j < MAX_PORTS; j++) {
					if (!ifidx[j])
						continue;
					if (!if_indextoname(ifidx[j], ifname))
						bb_perror_msg_and_die("can't get interface name for index %d", j);
					if (tabs)
						printf("\t\t\t\t\t");
					else
						tabs = 1;
					printf("\t\t%s\n", ifname);
				}
				if (!tabs)	/* bridge has no interfaces */
					bb_putchar('\n');
			}
			goto done;
		}
#endif

		if (!*argv) /* all but 'show' need at least one argument */
			bb_show_usage();

		br = *argv++;

		if (key == ARG_addbr || key == ARG_delbr) { /* addbr or delbr */
			ioctl_or_perror_and_die(fd,
					key == ARG_addbr ? SIOCBRADDBR : SIOCBRDELBR,
					br, "bridge %s", br);
			goto done;
		}

		if (!*argv) /* all but 'addif/delif' need at least two arguments */
			bb_show_usage();

		strncpy(ifr.ifr_name, br, IFNAMSIZ);
		if (key == ARG_addif || key == ARG_delif) { /* addif or delif */
			brif = *argv;
			ifr.ifr_ifindex = if_nametoindex(brif);
			if (!ifr.ifr_ifindex) {
				bb_perror_msg_and_die("iface %s", brif);
			}
			ioctl_or_perror_and_die(fd,
					key == ARG_addif ? SIOCBRADDIF : SIOCBRDELIF,
					&ifr, "bridge %s", br);
			goto done_next_argv;
		}
#if ENABLE_FEATURE_BRCTL_FANCY
		if (key == ARG_stp) { /* stp */
			/* FIXME: parsing yes/y/on/1 versus no/n/off/0 is too involved */
			arm_ioctl(args, BRCTL_SET_BRIDGE_STP_STATE,
					  (unsigned)(**argv - '0'), 0);
			goto fire;
		}
		if ((unsigned)(key - ARG_setageing) < 4) { /* time related ops */
			static const uint8_t ops[] ALIGN1 = {
				BRCTL_SET_AGEING_TIME,          /* ARG_setageing */
				BRCTL_SET_BRIDGE_FORWARD_DELAY, /* ARG_setfd     */
				BRCTL_SET_BRIDGE_HELLO_TIME,    /* ARG_sethello  */
				BRCTL_SET_BRIDGE_MAX_AGE        /* ARG_setmaxage */
			};
			arm_ioctl(args, ops[key - ARG_setageing], str_to_jiffies(*argv), 0);
			goto fire;
		}
		if (key == ARG_setpathcost
		 || key == ARG_setportprio
		 || key == ARG_setbridgeprio
		) {
			static const uint8_t ops[] ALIGN1 = {
				BRCTL_SET_PATH_COST,      /* ARG_setpathcost   */
				BRCTL_SET_PORT_PRIORITY,  /* ARG_setportprio   */
				BRCTL_SET_BRIDGE_PRIORITY /* ARG_setbridgeprio */
			};
			int port = -1;
			unsigned arg1, arg2;

			if (key != ARG_setbridgeprio) {
				/* get portnum */
				unsigned i;

				port = if_nametoindex(*argv++);
				if (!port)
					bb_error_msg_and_die(bb_msg_invalid_arg, *argv, "port");
				memset(ifidx, 0, sizeof ifidx);
				arm_ioctl(args, BRCTL_GET_PORT_LIST, (unsigned long)ifidx,
						  MAX_PORTS);
				xioctl(fd, SIOCDEVPRIVATE, &ifr);
				for (i = 0; i < MAX_PORTS; i++) {
					if (ifidx[i] == port) {
						port = i;
						break;
					}
				}
			}
			arg1 = port;
			arg2 = xatoi_u(*argv);
			if (key == ARG_setbridgeprio) {
				arg1 = arg2;
				arg2 = 0;
			}
			arm_ioctl(args, ops[key - ARG_setpathcost], arg1, arg2);
		}
 fire:
		/* Execute the previously set command */
		xioctl(fd, SIOCDEVPRIVATE, &ifr);
#endif
 done_next_argv:
		argv++;
 done:
		close(fd);
	}

	return EXIT_SUCCESS;
}
