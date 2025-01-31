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
 * WPA Supplicant / main() function for UNIX like OSes and MinGW
 * Copyright (c) 2003-2006, Jouni Malinen <jkmaline@cc.hut.fi>
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
#ifdef __linux__
#include <fcntl.h>
#endif /* __linux__ */

#include "common.h"
#include "wpa_supplicant_i.h"


extern const char *wpa_supplicant_version;
extern const char *wpa_supplicant_license;
#ifndef CONFIG_NO_STDOUT_DEBUG
extern const char *wpa_supplicant_full_license1;
extern const char *wpa_supplicant_full_license2;
extern const char *wpa_supplicant_full_license3;
extern const char *wpa_supplicant_full_license4;
extern const char *wpa_supplicant_full_license5;
#endif /* CONFIG_NO_STDOUT_DEBUG */

extern struct wpa_driver_ops *wpa_supplicant_drivers[];


static void usage(void)
{
	int i;
	printf("%s\n\n%s\n"
	       "usage:\n"
	       "  wpa_supplicant [-BddehLqquvwW] [-P<pid file>] "
	       "[-g<global ctrl>] \\\n"
	       "        -i<ifname> -c<config file> [-C<ctrl>] [-D<driver>] "
	       "[-p<driver_param>] \\\n"
	       "        [-b<br_ifname> [-N -i<ifname> -c<conf> [-C<ctrl>] "
	       "[-D<driver>] \\\n"
	       "        [-p<driver_param>] [-b<br_ifname>] ...]\n"
	       "\n"
	       "drivers:\n",
	       wpa_supplicant_version, wpa_supplicant_license);

	for (i = 0; wpa_supplicant_drivers[i]; i++) {
		printf("  %s = %s\n",
		       wpa_supplicant_drivers[i]->name,
		       wpa_supplicant_drivers[i]->desc);
	}

#ifndef CONFIG_NO_STDOUT_DEBUG
	printf("options:\n"
	       "  -b = optional bridge interface name\n"
	       "  -B = run daemon in the background\n"
	       "  -c = Configuration file\n"
	       "  -C = ctrl_interface parameter (only used if -c is not)\n"
	       "  -i = interface name\n"
	       "  -d = increase debugging verbosity (-dd even more)\n"
	       "  -D = driver name\n"
	       "  -g = global ctrl_interface\n"
	       "  -K = include keys (passwords, etc.) in debug output\n"
	       "  -t = include timestamp in debug messages\n"
	       "  -h = show this help text\n"
	       "  -L = show license (GPL and BSD)\n");
	printf("  -p = driver parameters\n"
	       "  -P = PID file\n"
	       "  -q = decrease debugging verbosity (-qq even less)\n"
#ifdef CONFIG_CTRL_IFACE_DBUS
	       "  -u = enable DBus control interface\n"
#endif /* CONFIG_CTRL_IFACE_DBUS */
	       "  -v = show version\n"
	       "  -w = wait for interface to be added, if needed\n"
	       "  -W = wait for a control interface monitor before starting\n"
	       "  -N = start describing new interface\n");

	printf("example:\n"
	       "  wpa_supplicant -Dwext -iwlan0 -c/etc/wpa_supplicant.conf\n");
#endif /* CONFIG_NO_STDOUT_DEBUG */
}


static void license(void)
{
#ifndef CONFIG_NO_STDOUT_DEBUG
	printf("%s\n\n%s%s%s%s%s\n",
	       wpa_supplicant_version,
	       wpa_supplicant_full_license1,
	       wpa_supplicant_full_license2,
	       wpa_supplicant_full_license3,
	       wpa_supplicant_full_license4,
	       wpa_supplicant_full_license5);
#endif /* CONFIG_NO_STDOUT_DEBUG */
}


static void wpa_supplicant_fd_workaround(void)
{
#ifdef __linux__
	int s, i;
	/* When started from pcmcia-cs scripts, wpa_supplicant might start with
	 * fd 0, 1, and 2 closed. This will cause some issues because many
	 * places in wpa_supplicant are still printing out to stdout. As a
	 * workaround, make sure that fd's 0, 1, and 2 are not used for other
	 * sockets. */
	for (i = 0; i < 3; i++) {
		s = open("/dev/null", O_RDWR);
		if (s > 2) {
			close(s);
			break;
		}
	}
#endif /* __linux__ */
}


int main(int argc, char *argv[])
{
	int c, i;
	struct wpa_interface *ifaces, *iface;
	int iface_count, exitcode = -1;
	struct wpa_params params;
	struct wpa_global *global;

	if (os_program_init())
		return -1;

	os_memset(&params, 0, sizeof(params));
	params.wpa_debug_level = MSG_INFO;

	iface = ifaces = os_zalloc(sizeof(struct wpa_interface));
	if (ifaces == NULL)
		return -1;
	iface_count = 1;

	wpa_supplicant_fd_workaround();

	for (;;) {
		c = getopt(argc, argv, "b:Bc:C:D:dg:hi:KLNp:P:qtuvwW");
		if (c < 0)
			break;
		switch (c) {
		case 'b':
			iface->bridge_ifname = optarg;
			break;
		case 'B':
			params.daemonize++;
			break;
		case 'c':
			iface->confname = optarg;
			break;
		case 'C':
			iface->ctrl_interface = optarg;
			break;
		case 'D':
			iface->driver = optarg;
			break;
		case 'd':
#ifdef CONFIG_NO_STDOUT_DEBUG
			printf("Debugging disabled with "
			       "CONFIG_NO_STDOUT_DEBUG=y build time "
			       "option.\n");
			goto out;
#else /* CONFIG_NO_STDOUT_DEBUG */
			params.wpa_debug_level--;
			break;
#endif /* CONFIG_NO_STDOUT_DEBUG */
		case 'g':
			params.ctrl_interface = optarg;
			break;
		case 'h':
			usage();
			goto out;
		case 'i':
			iface->ifname = optarg;
			break;
		case 'K':
			params.wpa_debug_show_keys++;
			break;
		case 'L':
			license();
			goto out;
		case 'p':
			iface->driver_param = optarg;
			break;
		case 'P':
			os_free(params.pid_file);
			params.pid_file = os_rel2abs_path(optarg);
			break;
		case 'q':
			params.wpa_debug_level++;
			break;
		case 't':
			params.wpa_debug_timestamp++;
			break;
#ifdef CONFIG_CTRL_IFACE_DBUS
		case 'u':
			params.dbus_ctrl_interface = 1;
			break;
#endif /* CONFIG_CTRL_IFACE_DBUS */
		case 'v':
			printf("%s\n", wpa_supplicant_version);
			goto out;
		case 'w':
			params.wait_for_interface++;
			break;
		case 'W':
			params.wait_for_monitor++;
			break;
		case 'N':
			iface_count++;
			iface = os_realloc(ifaces, iface_count *
					   sizeof(struct wpa_interface));
			if (iface == NULL)
				goto out;
			ifaces = iface;
			iface = &ifaces[iface_count - 1]; 
			os_memset(iface, 0, sizeof(*iface));
			break;
		default:
			usage();
			goto out;
		}
	}

	exitcode = 0;
	global = wpa_supplicant_init(&params);
	if (global == NULL) {
		printf("Failed to initialize wpa_supplicant\n");
		exitcode = -1;
		goto out;
	}

	for (i = 0; exitcode == 0 && i < iface_count; i++) {
		if ((ifaces[i].confname == NULL &&
		     ifaces[i].ctrl_interface == NULL) ||
		    ifaces[i].ifname == NULL) {
			if (iface_count == 1 && (params.ctrl_interface ||
						 params.dbus_ctrl_interface))
				break;
			usage();
			exitcode = -1;
			break;
		}
		if (wpa_supplicant_add_iface(global, &ifaces[i]) == NULL)
			exitcode = -1;
	}

	if (exitcode == 0)
		exitcode = wpa_supplicant_run(global);

	wpa_supplicant_deinit(global);

out:
	os_free(ifaces);
	os_free(params.pid_file);

	os_program_deinit();

	return exitcode;
}
