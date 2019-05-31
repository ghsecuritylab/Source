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
 *
 * Copyright 2013, ASUSTek Inc.
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND ASUS GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 */

#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <nvram/bcmnvram.h>

#include <semaphore_mfp.h>
#include <shared.h>
#include <flash_mtd.h>

#include "rc.h"
#include "rc_event.h"

extern struct nvram_tuple router_defaults[];
extern struct nvram_tuple boot_defaults[];

static int noconsole = 0;
static const char *const environment[] = {
	"HOME=/",
	"PATH=/sbin:/usr/sbin:/bin:/usr/bin",
	"SHELL=/bin/sh",
	"USER=root",
	NULL
};

void wl_defaults(void)
{
	int unit;
	char tmp[128], prefix[] = "wlXXXXXXXXXX_";
	struct nvram_tuple *t;

	for (unit=0; unit<2; unit++) {
		snprintf(prefix, sizeof(prefix), "wl%d_", unit);

		for (t = router_defaults; t->name; t++) {
			if (strncmp(t->name, "wl_", 3) != 0 
					|| !strncmp(t->name, "wl_ifnames", 10))
				continue;

			if (!nvram_get(strcat_r(prefix, &t->name[3], tmp))) {
				nvram_set(tmp, t->value);
			}
		}
	}
}

static void restore_defaults(void)
{
	int restore_defaults;
	struct nvram_tuple *t;

	restore_defaults = !nvram_match("restore_defaults", "0");
	if (restore_defaults)
		fprintf(stderr, "\n## Restoring defaults... ##\n");

	for (t = router_defaults; t->name; t++) {
		if (restore_defaults 				/* force restore defaults */
				|| !nvram_get(t->name)) {	/* only restore null nvram */
			nvram_set(t->name, t->value);
		}
	}

	wl_defaults();

	if (restore_defaults) {
#if defined(CONFIG_DEFAULTS_ASUS_RPN53)
		int i;
		unsigned char buf[13];

		memset(buf, 0, sizeof(buf));

		if (FRead(buf, MODEL_NAME_ADDR, 12) < 0) {
			fprintf(stderr, "READ Factory Model Name: Out of scope\n");
			buf[0] = 0xff;
		}
		else {
			for (i = 11 ; i >= 1 ; i--) {
				if ((unsigned char)buf[i] == 0xff)
					buf[i] = '\0';
				else
					break;
			}
		}

		if (!strcmp(buf, "RP-N54")) {
			nvram_set("wl0_ssid", "ASUS_RPN54");
			nvram_set("wl1_ssid", "ASUS_RPN54_5G");
		}
#endif

		nvram_commit();
		fprintf(stderr, "done\n");
	}

	klogctl(8, NULL, atoi(nvram_safe_get("console_loglevel")));

	for (t = boot_defaults; t->name; t++)
		nvram_set(t->name, t->value);
}

void set_unknown_domain_ip(char *ip, char *netmask)
{
	int i;
	char tmp[32];

	if(!chk_same_subnet(nvram_safe_get("reply_fake_ip"), ip, netmask))
		return;

	if (!strncmp(netmask, "255.255.255.0", 13)) {		// class C
		for (i = 255; i >= 0; i--) {
			sprintf(tmp, "192.168.%d.1", i);
			if (!chk_same_subnet(tmp, ip, netmask))
				break;
		}
	}
	else if (!strncmp(netmask, "255.255.0.0", 11)) {	// class B
		for (i = 31; i >= 16; i--) {
			sprintf(tmp, "172.%d.0.1", i);
			if (!chk_same_subnet(tmp, ip, netmask))
				break;
		}
	}
	else if (!strncmp(netmask, "255.0.0.0", 9)) {		// class A
		for (i = 255; i >= 0; i--) {
			sprintf(tmp, "10.%d.0.1", i);
			if (!chk_same_subnet(tmp, ip, "255.255.0.0"))
				break;
		}
	}
	else
		sprintf(tmp, "10.0.0.1");

	fprintf(stderr, "choose unknown domain IP as %s!\n", tmp);
	nvram_set("reply_fake_ip", tmp);
}

static void sysinit(void)
{
	time_t tm = 1293840000;	// Jan 1 00:00:00 GMT 2011
	int i;
	char tmp[16];
	FILE *fp;
	/* Set a sane date */
	stime(&tm);

	/* mount filesystem */
	eval("mount", "-a");
	mount("mdev", "/dev", "ramfs", MS_MGC_VAL, NULL);
	mkdir("/dev/pts", 0755);
	mount("devpts", "/dev/pts", "devpts", MS_MGC_VAL, NULL);
	eval("mdev", "-s");

	/* create device note */
	mknod("/dev/null",    S_IFCHR | 0660, makedev(1, 3));
	mknod("/dev/random",  S_IFCHR | 0660, makedev(1, 8));
	mknod("/dev/urandom", S_IFCHR | 0660, makedev(1, 9));
	mknod("/dev/console", S_IFCHR | 0660, makedev(5, 1));

	for (i = 0; i < NUM_INFO; i++) {
		sprintf(tmp, "/dev/mtdblock%d", i);
		mknod(tmp, S_IFBLK | 0660, makedev(31, i));
		sprintf(tmp, "/dev/mtd%d", i);
		mknod(tmp, S_IFCHR | 0660, makedev(90, i*2));
		sprintf(tmp, "/dev/mtd%dro", i);
		mknod(tmp, S_IFCHR | 0620, makedev(90, i*2+1));
	}

	mknod("/dev/video0", S_IFCHR | 0644, makedev(81, 0));
	mknod("/dev/flash0", S_IFCHR | 0644, makedev(200, 0));
	mknod("/dev/swnat0", S_IFCHR | 0644, makedev(210, 0));
	mknod("/dev/spiS0",  S_IFCHR | 0644, makedev(217, 0));
	mknod("/dev/i2cM0",  S_IFCHR | 0644, makedev(218, 0));
	mknod("/dev/hwnat0", S_IFCHR | 0644, makedev(220, 0));
	mknod("/dev/nvram",  S_IFCHR | 0644, makedev(228, 0));
	mknod("/dev/acl0",   S_IFCHR | 0644, makedev(230, 0));
	mknod("/dev/pcm0",   S_IFCHR | 0644, makedev(233, 0));
	mknod("/dev/i2s0",   S_IFCHR | 0644, makedev(234, 0));
	mknod("/dev/cls0",   S_IFCHR | 0644, makedev(235, 0));
#if (defined(AUDIO_SUPPORT) && defined(USB_TO_CM6510))
	mknod("/dev/i2cif",  S_IFCHR | 0644, makedev(238, 0));
#endif
	mknod("/dev/ac0",    S_IFCHR | 0644, makedev(240, 0));
	mknod("/dev/mtr0",   S_IFCHR | 0644, makedev(250, 0));
	mknod("/dev/gpio",   S_IFCHR | 0644, makedev(252, 0));
	mknod("/dev/rdm0",   S_IFCHR | 0644, makedev(253, 0));

	/* create /etc/mdev.conf */
	if ((fp = fopen("/etc/mdev.conf", "w"))) {
		fprintf(fp, "# <device regex> <uid>:<gid> <octal permissions> [<@|$|*> <command>]\n");
		fprintf(fp, "# The special characters have the meaning:\n");
		fprintf(fp, "# @ Run after creating the device.\n");
		fprintf(fp, "# $ Run before removing the device.\n");
		fprintf(fp, "# * Run both after creating and before removing the device.\n");
		fprintf(fp, "sd[a-z][1-9] 0:0 0660 */sbin/automount.sh $MDEV\n");
		fclose(fp);
	}
	/* enable usb hot-plug feature */
	if ((fp = fopen("/proc/sys/kernel/hotplug", "w+"))) {
		fputs("/sbin/mdev", fp);
		fclose(fp);
	}

	/* create directory */
	mkdir("/tmp/var", 0777);
	mkdir("/var/lock", 0777);
	mkdir("/var/log", 0777);
	mkdir("/var/run", 0777);
	mkdir("/var/tmp", 0777);

	/* Setup console */
	if (console_init())
		noconsole = 1;

	chdir("/");
	setsid();
	const char *const *e;
	/* Make sure environs is set to something sane */
	for (e = environment; *e; e++)
		putenv((char *) *e);

	/* extra settings */
	symlink("/tmp", "/shares");
	if ((fp = fopen("/proc/sys/net/ipv4/netfilter/ip_conntrack_udp_timeout", "w+"))) {
		fputs("90", fp);
		fclose(fp);
	}
	if ((fp = fopen("/proc/sys/net/nf_conntrack_max", "w+"))) {
#if (defined(CONFIG_DEFSETTING_4M16M) || defined(CONFIG_DEFSETTING_4M32M))
		fputs("1024", fp);
#elif (defined(CONFIG_DEFSETTING_8M64M))
		fputs("8192", fp);
#else
#error Invalid Flash/RAM size!!
#endif
		fclose(fp);
	}
	if ((fp = fopen("/proc/sys/net/ipv4/netfilter/ip_conntrack_tcp_timeout_established", "w+"))) {
		fputs("600", fp);
		fclose(fp);
	}
	if ((fp = fopen("/proc/sys/kernel/panic", "w+"))) {
		fputs("10", fp);
		fclose(fp);
	}
}

#ifdef RA_SINGLE_SKU
static void create_SingleSKU(const char *path, const char *pAppend, const char *reg_spec)
{
	char src[128];
	char dest[128];

	sprintf(src , "/ra_SKU/SingleSKU%s_%s.dat", pAppend, reg_spec);
	sprintf(dest, "%s/SingleSKU%s.dat", path, pAppend);
	symlink(src, dest);
}

static void destroy_SingleSKU(const char *path, const char *pAppend)
{
	char dest[128];

	sprintf(dest, "%s/SingleSKU%s.dat", path, pAppend);
	unlink(dest);
}
#endif

static void insmod_ko(int _sw_mode)
{
#ifdef LED_DRV_SUPPORT
	eval("insmod", "-q", "/usr/lib/led-drv.ko");
#endif
#ifdef AUDIO_SUPPORT
#ifdef I2C_TO_ALC5627
	eval("insmod", "-q", "/usr/lib/snd-drv.ko");
#endif
#ifdef USB_TO_CM6510
	eval("insmod", "-q", "/usr/lib/i2cif.ko");
#endif
#endif

	int n, is_iNIC;
	char wif[8], *next;

	for1each(n, wif, nvram_safe_get("wl_ifnames"), next) {
		if (!strncmp(wif, "rai", 3))
			is_iNIC = 1;
		else
			is_iNIC = 0;
		gen_ralink_config(n, is_iNIC);
	}
	gen_multiple_card_info();

#ifdef SWMODE_ADAPTER_SUPPORT
	if (_sw_mode == 4)
		eval("insmod", "-q", "/usr/lib/rt2860v2_sta.ko");
	else
#endif
	{
#ifdef RA_SINGLE_SKU
		char *reg_spec;

		reg_spec = nvram_safe_get("reg_spec");
		create_SingleSKU("/etc/Wireless/RT2860", "", reg_spec);
		create_SingleSKU("/etc/Wireless/iNIC", "_5G", reg_spec);
#endif
		insmod_wifi_ko();
	}
}

static void rmmod_ko(int _sw_mode)
{
#ifdef LED_DRV_SUPPORT
	eval("rmmod", "led-drv");
#endif
#ifdef AUDIO_SUPPORT
#ifdef I2C_TO_ALC5627
	eval("rmmod", "snd-drv");
#endif
#ifdef USB_TO_CM6510
	eval("rmmod", "i2cif");
#endif
#endif

#ifdef SWMODE_ADAPTER_SUPPORT
	if (_sw_mode == 4)
		eval("rmmod", "rt2860v2_sta");
	else
#endif
	{
		rmmod_wifi_ko();

#ifdef RA_SINGLE_SKU
		destroy_SingleSKU("/etc/Wireless/RT2860", "");
		destroy_SingleSKU("/etc/Wireless/iNIC", "_5G");
#endif
	}
}

static int state = START;
static int signalled = -1;

/* Signal handling */
void rc_signal(int sig)
{
	if (state == IDLE) {
		if (sig == SIGHUP)
			signalled = RESTART;
		else if (sig == SIGUSR2)
			signalled = START;
		else if (sig == SIGINT)
			signalled = STOP;
		else if (sig == SIGALRM)
			signalled = TIMER;
		else if (sig == SIGUSR1)
			signalled = SERVICE;
		else if (sig == SIGTTOU)
			signalled = MTDWRITE;
	}
}

/* Timer procedure */
int do_timer(void)
{
	time_t now;
	struct tm gm, local;
	struct timezone tz;
	char *pt;

	/* Update kernel timezone */
	pt = getenv("TZ");
	if (!pt || nvram_invmatch("time_zone_x", pt))
		setenv("TZ", nvram_safe_get("time_zone_x"), 1);
	time(&now);
	gmtime_r(&now, &gm);
	localtime_r(&now, &local);
	tz.tz_minuteswest = (mktime(&gm) - mktime(&local)) / 60;
	settimeofday(NULL, &tz);

	return 0;
}

void init_spinlock()
{
	spinlock_init(SPINLOCK_SiteSurvey);
}

//int sw_mode_change = 0;

/* Main loop */
static void main_loop(void)
{
	sigset_t sigset;
	pid_t shell_pid = 0;
	int _sw_mode;

#ifndef EXTERNAL_PHY_SUPPORT
	/* link down esw port */
	eval("mii_mgr", "-s", "-p", "0", "-r", "0", "-v", "0x3900");//port 0
	eval("mii_mgr", "-s", "-p", "1", "-r", "0", "-v", "0x3900");//port 1
	eval("mii_mgr", "-s", "-p", "2", "-r", "0", "-v", "0x3900");//port 2
	eval("mii_mgr", "-s", "-p", "3", "-r", "0", "-v", "0x3900");//port 3
	eval("mii_mgr", "-s", "-p", "4", "-r", "0", "-v", "0x3900");//port 4
#endif

	/* initialize GPIO mode and direction */
	gpio_init();

	/* Basic initialization */
	sysinit();

	/* Setup signal handlers */
	signal_init();
	signal(SIGHUP, rc_signal);
	signal(SIGUSR1, rc_signal);
	signal(SIGUSR2, rc_signal);
	signal(SIGINT, rc_signal);
	signal(SIGALRM, rc_signal);
	signal(SIGTTOU, rc_signal);
	sigemptyset(&sigset);

	eval("insmod", "nvram_linux.o");

	/* Loop forever */
	for (;;) {
		fprintf(stderr, "[rc] main_loop: state= %d\n", state);
		switch (state) {
		case MTDWRITE:
		      	state = IDLE;
#ifdef USE_SQUASHFS
			eval("mtd_write", "-r", "write", "/tmp/linux.trx", "Kernel_RootFS");
#else
			eval("mtd_write", "-r", "write", "/tmp/linux.trx", "Kernel");
#endif
		  	break;	 
		case SERVICE:
			state = handle_notifications();
			break;
		case RESTART:
			/* Fall through */
		case STOP:
#ifndef EXTERNAL_PHY_SUPPORT
			eval("mii_mgr", "-s", "-p", STR(ESW_WAN_PORT), "-r", "0", "-v", "0x3900");
#endif

			stop_services();
			stop_wan();
#ifdef WPS_SUPPORT
			stop_wsc();
#endif
			stop_lan(_sw_mode);
			stop_wlan(_sw_mode);
			destroy_ifname(_sw_mode);
			deconfig_loopback();
			rmmod_ko(_sw_mode);

			if (state == STOP) {
#ifndef EXTERNAL_PHY_SUPPORT
				eval("mii_mgr", "-s", "-p", STR(ESW_WAN_PORT), "-r", "0", "-v", "0x3300");
#endif
				state = IDLE;
				break;
			}
			/* Fall through */
		case START:
			restore_defaults();
			getsyspara();

			/* check mode switch */
			_sw_mode = atoi(nvram_safe_get("sw_mode"));
			if (_sw_mode != atoi(nvram_safe_get("pre_sw_mode"))) {
				fprintf(stderr, "\n---change mode from %s to %d---\n\n", nvram_safe_get("pre_sw_mode"), _sw_mode);
				//sw_mode_change = 1;	// reserved, unused
			}

			init_common_nvram();
			init_nvram_by_mode(_sw_mode);

#if defined(CONFIG_DEFAULTS_RALINK_MT7620)
			eval("reg", "s", "0xb0000000");
			eval("reg", "w", "0x34", "0x100000");
			usleep(20000);
			eval("reg", "w", "0x34", "0x0");
#endif

			insmod_ko(_sw_mode);
			config_loopback();
			create_ifname(_sw_mode);
			start_wlan(_sw_mode);
			start_lan(_sw_mode);
			sleep(1);
#ifdef SWMODE_HOTSPOT_SUPPORT
			if (_sw_mode != 5)
#endif
				start_wan(_sw_mode);
			init_spinlock();

#ifdef WEB_REDIRECT
			if (state == START) {
				redirect_setting();
				start_wanduck();
				sleep(1);
			}
			else
				restart_wanduck();
#endif

			start_services();
#if (defined(AUDIO_SUPPORT) && defined(USB_TO_CM6510))
			init_audio_setting();
#endif
			logmessage("rc", "System startup!");
			nvram_set("success_start_service", "1");	// For judging if the system is ready.
			nvram_set("pre_sw_mode", nvram_safe_get("sw_mode"));

#ifndef EXTERNAL_PHY_SUPPORT
			eval("mii_mgr", "-s", "-p", STR(ESW_WAN_PORT), "-r", "0", "-v", "0x3300");
#endif
			/* Fall through */
		case TIMER:
			do_timer();
			/* Fall through */
		case IDLE:
			state = IDLE;
			/* Wait for user input or state change */
			while (signalled == -1) {
				if (!noconsole && (!shell_pid || kill(shell_pid, 0) != 0))
					shell_pid = run_shell(0, 1);
				else
					sigsuspend(&sigset);
			}
			state = signalled;
			signalled = -1;
			break;
		default:
			return;
		}
	}
}

int main(int argc, char **argv)
{
	char *base = strrchr(argv[0], '/');

	base = base ? base + 1 : argv[0];

	if (!strcmp(base, "init")) {
		main_loop();
		return 0;
	}
	else if (!strcmp(base, "nvram_oob")) {
		nvram_unset("restore_defaults");
		restore_defaults();
		return 0;
	}
	else if (!strcmp(base, "watchdog"))
		return watchdog_main();
	else if (!strcmp(base, "wanduck"))
		return wanduck_main(argc, argv);
	else if (!strcmp(base, "apcli_monitor"))
		return apcli_monitor_main();
	else if (!strcmp(base, "detectWAN_arp"))
		return detectWAN_arp_main(argc, argv);
	/* udhcpc [ deconfig bound renew ] */
	else if (!strcmp(base, "udhcpc"))
		return udhcpc_main(argc, argv);
	/* ddns update ok */
	else if (!strcmp(base, "ddns_updated")) 
		return ddns_updated_main();
	else if (!strcmp(base, "ntp"))
		return ntp_main();
	else if (!strcmp(base, "gen_ralink_config")) {
		int n, is_iNIC;
		char wif[8], *next;

		for1each(n, wif, nvram_safe_get("wl_ifnames"), next) {
			if (!strncmp(wif, "rai", 3))
				is_iNIC = 1;
			else
				is_iNIC = 0;
			gen_ralink_config(n, is_iNIC);
		}
		return 0;
	}
#ifdef WPS_SUPPORT
	else if (!strcmp(base, "wps_start")) {
		start_wsc();
		return 0;
	}
	else if (!strcmp(base, "wps_oob")) {
		wps_oob();
		return 0;
	}
	else if (!strcmp(base, "wps_stop")) {
		stop_wsc();
		return 0;
	}
#endif
	/* ppp */
	else if (!strcmp(base, "ip-up"))
		return ipup_main();
	else if (!strcmp(base, "ip-down"))
		return ipdown_main();
	else if (!strcmp(base, "wan-up"))
		return ipup_main();
	else if (!strcmp(base, "wan-down"))
		return ipdown_main();
	else if (!strcmp(base, "restart_dns")) {
		restart_dns();
		return 0;
	}
	else if (!strcmp(base, "start_telnetd")) {
		start_telnetd(0);
		return 0;
	}
	else if (!strcmp(base, "run_telnetd")) {
		start_telnetd(1);
		return 0;
	}
	else if (!strcmp(base, "restart_dhcpd")) {
		if (argc >= 2)
			restart_dnsmasq(atoi(argv[1]));
		else
			restart_dnsmasq(0);
		return 0;
	}
	else if (!strcmp(base, "link_down")) {
		link_down();
		return 0;
	}
	else if (!strcmp(base, "link_up")) {
		link_up();
		return 0;
	}
	else if (!strcmp(base, "link_status")) {
		GetPhyStatus(ESW_WAN_PORT, 0);
		return 0;
	}
	else if (!strcmp(base, "track_set")) {
		if (argc == 2 || argc == 3)
			track_set(atoi(argv[1]), argv[2]);
		else
			fprintf(stderr, "Usage: track_set flag [data]\n");
		return 0;
	}
	else if (!strcmp(base, "logmessage")) {
		if (argc == 3)
			logmessage(argv[1], argv[2]);
		else
			fprintf(stderr, "Usage: logmessage [HEADWORD] [STRING]");
		return 0;
	}
#ifdef ASUS_ATE
	else if (!strcmp(base, "ATE")) {
		if (argc == 2 || argc == 3 || argc == 4) {
			asus_ate_command(argv[1], argv[2], argv[3]);
		}
		else
			fprintf(stderr, "ATE_ERROR\n");
		return 0;
	}
#endif /* ASUS_ATE */

	return EINVAL;
}
