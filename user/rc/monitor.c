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

#include <stdio.h>
#include <signal.h>
#include <pthread.h>
#include <sys/wait.h>
#include <sys/times.h>

#include <nvram/bcmnvram.h>

#include <shared.h>
#include <iwlib.h>

#include "rc.h"
#include "rc_event.h"

#define APCLI_CONN_PERIOD	20	// sec
#define APCLI_DISCONN_PERIOD	5	// sec

int ap_set(char *wif, const char *pv_pair);
unsigned long get_uptime(void);

static int pause_monitor = 0;
static int need_manually_bridge = 1;
static int main_sleep = 1;
static int delay_band;
static int qis_state = 0, qis_state_old = 0;
static int apcli_connStatus[2] = {0, 0};
static int pap_HT_EXT[2] = {-1, -1};
static int pap_channel[2] = {-1, -1};
static int _sw_mode;
#ifdef REPEATER_5G_WORKAROUND
static int disconnected_cnt = -1; /* -1:disable, 0:enable */
#endif

#define no_connected_PAP()	((nvram_match("sta_freq", "2.4") && n == 0 && !apcli_connStatus[1]) \
				|| (nvram_match("sta_freq", "5") && n == 1 && !apcli_connStatus[0]))

#define have_connected_PAP()	((nvram_match("sta_freq", "2.4") && !apcli_connStatus[0] && apcli_connStatus[1]) \
				|| (nvram_match("sta_freq", "5") && !apcli_connStatus[1] && apcli_connStatus[0]))

static int chld_handle(int sig)
{
	while (waitpid(-1,NULL,WNOHANG)>0);
}

static void force_client_renew_ip(void)
{
	restart_dnsmasq(1);
}

/*
 * apcli setup stage3: bridge apcli interface (for Repeater mode) & DHCP IP
 */
static void bridge_apcli(int band)
{
	int n;
	char wif[8], *wif_next, aif[8], *aif_next;

	nvram_set("sta_freq", band==1?"5":"2.4");
	fprintf(stderr, "choose %s parent-AP\n", band==1?"5G":"2.4G");

#ifdef SWMODE_HOTSPOT_SUPPORT
	if (_sw_mode == 2) {
#endif
		char *lan_ifname = nvram_safe_get("lan_ifname");
		char buf[32];
		char *pt;
		unsigned char val;
		unsigned char ea[ETHER_ADDR_LEN];

		for2each(n, wif, nvram_safe_get("wl_ifnames"), wif_next, 
				aif, nvram_safe_get("sta_ifnames"), aif_next) {
			if (band) {
				if (n) {
					eval("brctl", "addif", lan_ifname, aif);
				}
				else {
					eval("brctl", "delif", lan_ifname, aif);
					strncpy(buf, nvram_safe_get("wl1_macaddr"), sizeof(buf));
				}
			}
			else {
				if (n) {
					eval("brctl", "delif", lan_ifname, aif);
				}
				else {
					eval("brctl", "addif", lan_ifname, aif);
					strncpy(buf, nvram_safe_get("wl0_macaddr"), sizeof(buf));
				}
			}
		}

		pt = buf + 15;
		val = strtoul(pt, NULL, 16);
		val++;
		sprintf(pt, "%02X", val);
		eval("ifconfig", lan_ifname, "hw", "ether", buf);
		if (ether_atoe(buf, ea))
			track_set(ASUS_HIJ_SET_DUT_MAC, ea);

		if (nvram_match("lan_proto_x", "1"))
			if (nvram_match("qis_apply", "1"))
				eval("killall", "-SIGTTOU", "udhcpc");
			else {
				stop_udhcpc();
				start_udhcpc(lan_ifname, NULL, nvram_safe_get("lan_hostname"));
			}
		else {
			lan_up(lan_ifname);
			nvram_set("dnsqmode", "1");
			force_client_renew_ip();
		}

#ifdef SWMODE_REPEATER_V2_SUPPORT
		notify_rc("stop_wif");
#endif
#ifdef SWMODE_HOTSPOT_SUPPORT
	}
	else {
		for1each(n, aif, nvram_safe_get("sta_ifnames"), aif_next) {
			if (n == band) {
				nvram_set("wan_ifname", aif);
				start_wan(_sw_mode);
			}
			else
				eval("ifconfig", aif, "0.0.0.0");
		}
	}
#endif
}

/*
 * apcli setup stage2: advance known DHCP IP for QIS
 */
static void advance_known_dhcp_ip(void)
{
	if (nvram_match("lan_proto_x", "1")) {
		int n;
		char aif[8], *next;

		for1each(n, aif, nvram_safe_get("sta_ifnames"), next) {
			if (n == delay_band) {
				stop_udhcpc();
				start_udhcpc(aif, "br0", nvram_safe_get("lan_hostname"));
				break;
			}
		}
	}
	else
		nvram_set("lan_ipaddr_new", nvram_safe_get("lan_ipaddr"));
}

/*
 * apcli setup stage1: choose 2.4/5GHz by link quality
 */
static int simple_lock = 0;
static void apcli_monitor_setup(void)
{
	int band = 0;

	if (simple_lock) {
		fprintf(stderr, "Re-entry apcli_monitor_setup!!\n");
		return;
	}

	simple_lock = 1;
	need_manually_bridge = 0;
	band = choose_apcli_by_link_quality();
	if (band == -1) {
		fprintf(stderr, "this SIGUSR1 should be ignored!\n");
		simple_lock = 0;
		return;
	}

	if (nvram_match("qis_apply", "1") && get_uptime() < (atol(nvram_safe_get("dnsqmode_1st"))+240)) {
		fprintf(stderr, "*********** Delay bridge stage 1!\n");
		main_sleep = 0;
		delay_band = band;
	}
	else
		bridge_apcli(band);

	simple_lock = 0;
}

static void update_apcli_status(void)
{
	int n;
	char aif[8], *next;
	char tmp[128], prefix[] = "staXXXXXXXXXX_";

	for1each(n, aif, nvram_safe_get("sta_ifnames"), next) {
		int connStatus = 0;

		snprintf(prefix, sizeof(prefix), "sta%d_", n);
		if (nvram_match(strcat_r(prefix, "ssid", tmp), ""))
			continue;

#ifndef DUAL_BAND_NONCONCURRENT
		fprintf(stderr, "%sG ", n==1?"5":"2.4");
#endif
			get_apcli_connStatus(aif, &connStatus);

		if (connStatus) {
			int skfd;/* generic raw socket desc. */
			struct wireless_info info;
			char buffer[128];

			/* Create a channel to the NET kernel. */
			if ((skfd = iw_sockets_open()) < 0) {
				perror("socket");
				return 0;
			}
			get_info(skfd, aif, &info);
			/* Close the socket. */
			close(skfd);

			fprintf(stderr, "Connected with: \"%s\" (%s)\n", 
					info.b.essid, iw_sawap_ntop(&info.ap_addr, buffer));
#ifdef REPEATER_5G_WORKAROUND
			if (n == 1)
				disconnected_cnt = 0;
#endif
			nvram_set(strcat_r(prefix, "connStatus", tmp), "1");
			apcli_connStatus[n] = 1;
		}
		else {
			fprintf(stderr, "Disconnected...\n");
			if (_sw_mode == 2 
					&& !nvram_match("dnsqmode", "2") 
					&& (nvram_match("dhcp_enable_x", "1") 
						|| nvram_match("x_Setting", "0")) 
					&& nvram_match("sta_freq", "") 
					&& !nvram_match("qis_apply", "1")) {
				nvram_set("dnsqmode", "2");
				force_client_renew_ip();
#ifdef SWMODE_REPEATER_V2_SUPPORT
				notify_rc("start_wif");
#endif
			}

			if (no_connected_PAP()) {
				need_manually_bridge = 1;
				nvram_set("sta_freq", "");
			}

#ifdef REPEATER_5G_WORKAROUND
			if (n == 1 && disconnected_cnt >= 0) {
				if (++disconnected_cnt > 3) {
					disconnected_cnt = 0;
					restart_apcliX(n);
					init_apcli_monitor_para(n, aif);
				}
			}
#endif
			nvram_set(strcat_r(prefix, "connStatus", tmp), "0");
			apcli_connStatus[n] = 0;
		}
	}
}

/* pthread arguments structure */
struct pthread_arg{
	int n;
	char wif[8];
	int HT_EXT;
	int channel;
};

/* pthread for 2.4/5G site survey */
void site_survey_pthread(void *arg)
{
	struct pthread_arg *a = (struct pthread_arg *)arg;

	a->channel = site_survey_for_channel(a->n ,a->wif, &a->HT_EXT);
}

static void apcli_connect(void)
{
	char tmp[128], prefix[] = "staXXXXXXXXXX_";
	int n;
	char wif[8], *next;
	int cur_channel[2];
	pthread_t id[2];
	struct pthread_arg a[2];
	int site_survey_times = 0;

	if (pause_monitor > 0)
		pause_monitor--;

	update_apcli_status();

	for1each(n, wif, nvram_safe_get("wl_ifnames"), next) {
		cur_channel[n] = get_channel(wif);
		snprintf(prefix, sizeof(prefix), "sta%d_", n);

		if ((apcli_connStatus[n] && pap_channel[n] == cur_channel[n]) 
				|| nvram_match(strcat_r(prefix, "ssid", tmp), ""))
			continue;

		a[n].n = n;
		strcpy(a[n].wif, wif);

		if (pthread_create(&id[n], NULL, (void *) site_survey_pthread, &a[n]) != 0) { 
			fprintf(stderr, "Create site_survey pthread error!\n");
			return;
		}
	}

	for1each(n, wif, nvram_safe_get("wl_ifnames"), next) {
		snprintf(prefix, sizeof(prefix), "sta%d_", n);

		if ((apcli_connStatus[n] && pap_channel[n] == cur_channel[n]) 
				|| nvram_match(strcat_r(prefix, "ssid", tmp), ""))
			continue;

		pthread_join(id[n], NULL);
		pap_HT_EXT[n] = a[n].HT_EXT;
		pap_channel[n] = a[n].channel;
		site_survey_times = APCLI_DISCONN_PERIOD;

		if ((pap_channel[n] != -1 && pap_channel[n] != cur_channel[n]) 
				|| (pap_HT_EXT[n] != -1 && pap_HT_EXT[n] != get_extcha(wif))) {
			char buf[32];

			fprintf(stderr, "Change to channel: %d\n", pap_channel[n]);

			if (n == 1) {
				/* 20MHz */
				if (pap_channel[n] == 140 || pap_channel[n] == 165) {
					ap_set(wif, "HtBw=0");
#ifdef IEEE802_11AC
					ap_set(wif, "VhtBw=0");
					ap_set(wif, "VhtDisallowNonVHT=0");
#endif
				}
				/* 20/40/80MHz */
				else {
					ap_set(wif, "HtBw=1");
#ifdef IEEE802_11AC
					ap_set(wif, "VhtBw=1");
					ap_set(wif, "VhtDisallowNonVHT=0");
#endif
				}
			}

			if (pap_HT_EXT[n] != -1) {
				sprintf(buf, "HtExtcha=%d", pap_HT_EXT[n]);
				ap_set(wif, buf);
			}

			sprintf(buf, "Channel=%d", pap_channel[n]);
			ap_set(wif, buf);
		}
		else {
			if (have_connected_PAP())
				apcli_monitor_setup();
		}
	}

	if (!apcli_connStatus[0] && !apcli_connStatus[1])
		alarm(APCLI_DISCONN_PERIOD);
	else
		alarm(APCLI_CONN_PERIOD - site_survey_times);
}

/* return 0 or 1, -1 is none */
static int choose_apcli_by_link_quality(void)
{
	// assume only connected to one frequency
	update_apcli_status();
	if (!apcli_connStatus[0] && !apcli_connStatus[1])
		return -1;
	else if (!apcli_connStatus[1])
		return 0;
	else if (!apcli_connStatus[0])
		return 1;

	// connected to 2.4/5G parent-AP
	// check link quality for choose parent-AP
	int n;
	char aif[8], *next;
	int apcli_Quality[2];

	fprintf(stderr, "apcli link quality: ");
	for1each(n, aif, nvram_safe_get("sta_ifnames"), next) {
		get_apcli_quality(aif, &apcli_Quality[n]);
		fprintf(stderr, "[%s=%d] \n", aif, apcli_Quality[n]);
	}
	fprintf(stderr, "\n");

	if (apcli_Quality[1] >= apcli_Quality[0])
		return 1;
	else
		return 0;
}

static void sig_handle(int sig)
{
	if (sig == SIGUSR1) {
		fprintf(stderr, "receive SIGUSR1 from wireless driver!\n");
		if (nvram_invmatch("sta_freq", "") 
				|| pause_monitor > 0
		) {
			fprintf(stderr, "Ignore this SIGUSR1! already connected to other frequency\n");
		}
		else
			apcli_monitor_setup();
	}
	else if (sig == SIGTSTP) {
		fprintf(stderr, "Pause apcli_monitor!\n");
		pause_monitor = 1;
		alarm(0);
	}
	else if (sig == SIGCONT) {
		int n;
		char wif[8], *next;

		for1each(n, wif, nvram_safe_get("wl_ifnames"), next) {
			if ((pap_channel[n] != -1 && pap_channel[n] != get_channel(wif)) 
					|| (pap_HT_EXT[n] != -1 && pap_HT_EXT[n] != get_extcha(wif))) {
				char buf[32];

				if (pap_HT_EXT[n] != -1) {
					sprintf(buf, "HtExtcha=%d", pap_HT_EXT[n]);
					ap_set(wif, buf);
				}

				sprintf(buf, "Channel=%d", pap_channel[n]);
				ap_set(wif, buf);
			}
		}

		fprintf(stderr, "Resume apcli_monitor!\n");
		if (nvram_invmatch("sta_freq", ""))
			alarm(APCLI_CONN_PERIOD);
		else
			alarm(APCLI_DISCONN_PERIOD);
	}
	else if (sig == SIGALRM) {
		/* prevent wireless driver does not send signal, 
		   or apcli_monitor lost signal from wireless driver */
		if ((need_manually_bridge == 1) 
#ifdef SWMODE_HOTSPOT_SUPPORT
				&& _sw_mode == 2 
#endif
				&& (apcli_connStatus[0] || apcli_connStatus[1]))
			apcli_monitor_setup();

		apcli_connect();
	}
}

int apcli_monitor_main(void)
{
	FILE *fp;
	int n;
	char aif[8], *next;

	/* write pid */
	if ((fp = fopen(APCLI_MONITOR_PIDFILE, "w")) != NULL) {
		fprintf(fp, "%d", getpid());
		fclose(fp);
	}

	/* get current mode */
	_sw_mode = atoi(nvram_safe_get("sw_mode"));

	nvram_set("sta_freq", "");
	nvram_set("lan_ipaddr_new", "");
	nvram_set("lan_gateway_old", "");
	for1each(n, aif, nvram_safe_get("sta_ifnames"), next)
		init_apcli_monitor_para(n, aif);

	signal(SIGCHLD, chld_handle);
	signal(SIGUSR1, sig_handle);
	signal(SIGTSTP, sig_handle);
	signal(SIGCONT, sig_handle);
	signal(SIGALRM, sig_handle);

	alarm(1);

	while (1) {
		if (main_sleep)
			pause();
		else {
			sleep(1);

			qis_state = atoi(nvram_safe_get("qis_state"));
			if (get_uptime() >= (atol(nvram_safe_get("dnsqmode_1st"))+240)) {
				qis_state = 2;
				nvram_set("qis_apply", "0"); // force clear
			}

			if (qis_state != qis_state_old) {
				switch (qis_state) {
				case 1:
					fprintf(stderr, "*********** Delay bridge stage 2!\n");
					advance_known_dhcp_ip();
					break;
				case 2:
					fprintf(stderr, "########### start bridge after delay...\n");
					main_sleep = 1;
					bridge_apcli(delay_band);
					break;
				case 3:
					fprintf(stderr, "########### QIS fail...\n");
					main_sleep = 1;
					break;
				default:
					;
				}
				qis_state_old = qis_state;
			}

			if (main_sleep) {
				nvram_unset("qis_state");
				nvram_set("qis_apply", "0"); // force clear
				nvram_commit();
			}
		}
	}

	return 0;
}
