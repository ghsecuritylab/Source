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
 * ASUS Home Gateway Reference Design
 * Web Page Configuration Support Routines
 *
 * Copyright 2013, ASUSTek Inc.
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 */

#include <stdio.h>

#include <net/ethernet.h>
#include <nvram/bcmnvram.h>

#include <semaphore_mfp.h>
#include <shared.h>
#include <ralink.h>
#include <iwlib.h>
#include <stapriv.h>

#include "httpd.h"
#ifdef APP_SUPPORT
#include "web-app.h"
#endif

/*
 * GUI: show wireless status
 */
int ej_wl_status(int eid, webs_t wp, int argc, char_t **argv)
{
	int n;
	char wif[8], *next;
	int ret = 0;
	int channel;
	struct iw_range	range;
	double freq;
	struct iwreq wrq0;
	struct iwreq wrq1;
	struct iwreq wrq2;
	struct iwreq wrq3;
	unsigned long phy_mode;
	char radio_buf[32];
#ifdef SWMODE_REPEATER_EXPRESSWAY_SUPPORT
	int re_expressway = atoi(nvram_safe_get("re_expressway"));
#endif

	for1each(n, wif, nvram_safe_get("wl_ifnames"), next) {
#ifdef SWMODE_REPEATER_EXPRESSWAY_SUPPORT
		if (IsExpressWayCli())
			;
		else if (IsExpressWayApcli())
			continue;
		else {
#endif
#ifndef DUAL_BAND_NONCONCURRENT
			if (n) {
				websWrite(wp, "\n========================================\n");
				websWrite(wp, "5G\n");
			}
			else
				websWrite(wp, "2.4G\n");
#endif
#ifdef SWMODE_REPEATER_EXPRESSWAY_SUPPORT
		}
#endif

		sprintf(radio_buf,"sw%d_radio", n);
		if (nvram_match(radio_buf, "0")) {
			ret += websWrite(wp, "Radio is disabled\n");
			continue;
		}

		if (wl_ioctl(wif, SIOCGIWAP, &wrq0) < 0) {
			ret += websWrite(wp, "SYSTEM ERROR!\n");
			continue;
		}

		wrq0.u.ap_addr.sa_family = ARPHRD_ETHER;
		ret += websWrite(wp, "MAC address	: %02X:%02X:%02X:%02X:%02X:%02X\n",
			(unsigned char)wrq0.u.ap_addr.sa_data[0],
			(unsigned char)wrq0.u.ap_addr.sa_data[1],
			(unsigned char)wrq0.u.ap_addr.sa_data[2],
			(unsigned char)wrq0.u.ap_addr.sa_data[3],
			(unsigned char)wrq0.u.ap_addr.sa_data[4],
			(unsigned char)wrq0.u.ap_addr.sa_data[5]);
		if (wl_ioctl(wif, SIOCGIWFREQ, &wrq1) < 0)
			return ret;

		char buffer[sizeof(iwrange) * 2];
		bzero(buffer, sizeof(buffer));
		wrq2.u.data.pointer = (caddr_t) buffer;
		wrq2.u.data.length = sizeof(buffer);
		wrq2.u.data.flags = 0;
		if (wl_ioctl(wif, SIOCGIWRANGE, &wrq2) < 0)
			return ret;

		if (ralink_get_range_info(wif, &range, buffer, wrq2.u.data.length) < 0)
			return ret;

		bzero(buffer, sizeof(unsigned long));
		wrq2.u.data.length = sizeof(unsigned long);
		wrq2.u.data.pointer = (caddr_t) buffer;
		wrq2.u.data.flags = RT_OID_GET_PHY_MODE;
		if (wl_ioctl(wif, RT_PRIV_IOCTL, &wrq2) < 0)
			return ret;
		else
			phy_mode = *((unsigned int *)buffer);
		freq = iw_freq2float(&(wrq1.u.freq));
		if (freq < KILO)
			channel = (int)freq;
		else {
			channel = iw_freq_to_channel(freq, &range);
			if (channel < 0)
				return ret;
		}

#ifdef IEEE802_11AC
		if (n) {
			char tmp[16];
			char *p = tmp;
			if (phy_mode & WMODE_A)
				p += sprintf(p, "/a");
			if (phy_mode & WMODE_B)
				p += sprintf(p, "/b");
			if (phy_mode & WMODE_G)
				p += sprintf(p, "/g");
			if (phy_mode & WMODE_GN)
				p += sprintf(p, "/n");	//N in 2G
			if (phy_mode & WMODE_AN)
				p += sprintf(p, "/n");	//N in 5G
			if (phy_mode & WMODE_AC)
				p += sprintf(p, "/ac");
			if (p != tmp)
				ret += websWrite(wp, "Phy Mode	: 11%s\n", tmp+1);	// skip first '/'
		}
		else
#endif
		{
			if (phy_mode == PHY_11BG_MIXED)
				ret += websWrite(wp, "Phy Mode	: 11b/g\n");
			//else if (phy_mode == PHY_11B)
			//	ret += websWrite(wp, "Phy Mode	: 11b\n");
			else if (phy_mode == PHY_11A)
				ret += websWrite(wp, "Phy Mode	: 11a\n");
			//else if (phy_mode == PHY_11ABG_MIXED)
			//	ret += websWrite(wp, "Phy Mode	: 11a/b/g\n");
			//else if (phy_mode == PHY_11G)
			//	ret += websWrite(wp, "Phy Mode	: 11g\n");
			//else if (phy_mode == PHY_11ABGN_MIXED)
			//	ret += websWrite(wp, "Phy Mode	: 11a/b/g/n\n");
			else if (phy_mode == PHY_11N_2G)
				ret += websWrite(wp, "Phy Mode	: 11n\n");
			//else if (phy_mode == PHY_11GN_MIXED)
			//	ret += websWrite(wp, "Phy Mode	: 11g/n\n");
			else if (phy_mode == PHY_11AN_MIXED)
				ret += websWrite(wp, "Phy Mode	: 11a/n\n");
			else if (phy_mode == PHY_11BGN_MIXED)
				ret += websWrite(wp, "Phy Mode	: 11b/g/n\n");
			//else if (phy_mode == PHY_11AGN_MIXED)
			//	ret += websWrite(wp, "Phy Mode	: 11a/g/n\n");
			else if (phy_mode == PHY_11N_5G)
				ret += websWrite(wp, "Phy Mode	: 11n\n");
#if 0
#ifdef IEEE802_11AC
			//else if (phy_mode == PHY_11VHT_N_ABG_MIXED)
			//	ret += websWrite(wp, "Phy Mode	: 11a/b/g/n/ac\n");
			//else if (phy_mode == PHY_11VHT_N_AG_MIXED)
			//	ret += websWrite(wp, "Phy Mode	: 11a/g/n/ac\n");
			else if (phy_mode == PHY_11VHT_N_A_MIXED)
				ret += websWrite(wp, "Phy Mode	: 11a/n/ac\n");
			else if (phy_mode == PHY_11VHT_N_MIXED)
				ret += websWrite(wp, "Phy Mode	: 11n/ac\n");
#endif
#endif
		}

		ret += websWrite(wp, "Channel		: %d\n", channel);

		char data[8192];
		memset(data, 0, sizeof(data));
		wrq3.u.data.pointer = data;
		wrq3.u.data.length = sizeof(data);
		wrq3.u.data.flags = 0;
		if (wl_ioctl(wif, RTPRIV_IOCTL_GET_MAC_TABLE, &wrq3) < 0)
			return ret;

#define SHOW_STA_INFO(_p, _i, _st)											\
	{														\
		_st *Entry = ((_st *)(_p)) + _i;									\
		ret += websWrite(wp, "%02X:%02X:%02X:%02X:%02X:%02X\n",							\
				Entry->Addr[0], Entry->Addr[1],								\
				Entry->Addr[2], Entry->Addr[3],								\
				Entry->Addr[4], Entry->Addr[5]								\
		);													\
	}

		RT_802_11_MAC_TABLE* mp = (RT_802_11_MAC_TABLE*)wrq3.u.data.pointer;
		int i;

		ret += websWrite(wp, "\n");
		ret += websWrite(wp, "Stations List                           \n");
		ret += websWrite(wp, "----------------------------------------\n");
		for (i=0 ; i<mp->Num ; i++) {
#ifdef IEEE802_11AC
			if (n) {
				SHOW_STA_INFO(mp->Entry, i, RT_802_11_MAC_ENTRY_11AC);
			}
			else
#endif
			{
				SHOW_STA_INFO(mp->Entry, i, RT_802_11_MAC_ENTRY);
			}
		}
	}

	return ret;
}


/* SSID only support ASCII 0x20 ~ 0x7e */
static int validate_ssid(char *str)
{
	int i;

	for (i = 0; i < strlen(str); i++) {
		if ((str[i] >= 32) && (str[i] <= 126))
			continue;
		else
			return 1;
	}

	return 0;
}

/* remove space in the end of string */
static char *trim_r(char *str)
{
	int i;

	i = strlen(str);

	while (i >= 1) {
		if (*(str+i-1) == ' ' || *(str+i-1) == 0x0a || *(str+i-1) == 0x0d)
			*(str+i-1)=0x0;
		else
			break;
		i--;
	}
	return str;
}

static char *toupperstr(char *str)
{
	int i;

	for (i = 0; i < strlen(str); i++)
		str[i] = toupper(str[i]);
	return str;
}

static void get_apcli_address(char* output_address, char* aif)
{
	int skfd;
	int rc;
	struct wireless_info info;
	char buffer[128];

	if (!output_address)
		return;

	/* Create a channel to the NET kernel. */
	if ((skfd = iw_sockets_open()) < 0) {
		perror("socket");
		return;
	}

	rc = get_info(skfd, aif, &info);

	/* Close the socket. */
  	close(skfd);

	if (!rc && info.b.has_essid && info.b.essid_on && info.has_ap_addr) {
		strcpy(output_address, iw_sawap_ntop(&info.ap_addr, buffer));
		if (!strcmp(output_address, "Not-Associated") 
				|| !strcmp(output_address, "Invalid") 
				|| !strcmp(output_address, "None"))
			strcpy(output_address, "");
	}
	else
		strcpy(output_address, "");
}

/*
 * GUI: site survey
 */
int ej_SiteSurvey(int eid, webs_t wp, int argc, char_t **argv)
{
	int org_wmode[2] = {0, 0};
#ifdef DUAL_BAND_NONCONCURRENT
	char org_ch[4];
	int _sw_mode = atoi(nvram_safe_get("sw_mode"));
	int _cur_freq = atoi(nvram_safe_get("cur_freq"));
	int try_another_band = 1;
#endif

	int ret = 0, i = 0, apCount = 0, n = 0, comma_need = 0;
	char wif[8], *wif_next, aif[8], *aif_next;
	char tmp[128], prefix[] = "staXXXXXXXXXX_";
	char data[8192];
	char ssid_str[256];
	struct iwreq wrq;
	SSA *ssap;
	char apcli_mac[18];
	int connStatus = 0;

	char commch[4], cench[4];
	int commonchannel, centralchannel;
	int ht_extcha;

	if (nvram_match("wps_running", "1")) {	// WPS is proceeding
		ret += websWrite(wp, "[");
		ret += websWrite(wp, "];");
		goto WPSing;
	}

	ret += websWrite(wp, "[");

TryAnotherBand:
	for2each(n, wif, nvram_safe_get("wl_ifnames"), wif_next,
			aif, nvram_safe_get("sta_ifnames"), aif_next) {
#ifdef DUAL_BAND_NONCONCURRENT
		if (try_another_band) {
			if (_sw_mode == 2 
#ifdef SWMODE_HOTSPOT_SUPPORT
					|| _sw_mode == 5
#endif
			) {
				strcpy(org_ch, get_Channel(aif));		//for EA-N66 save origin channel
				eval("killall", "-SIGTSTP", "apcli_monitor");	//pause apcli_monitor
			}
#endif
			snprintf(prefix, sizeof(prefix), "wl%d_", n);
			org_wmode[n] = wl_nmode2wmode(n, atoi(nvram_safe_get(strcat_r(prefix, "nmode_x", tmp))));
#ifdef IEEE802_11AC
			if (org_wmode[n] != 9 && org_wmode[n] != 14)
#else
			if (org_wmode[n] != 9 && org_wmode[n] != 8)
#endif
#ifdef DUAL_BAND_NONCONCURRENT
				change_WirelessMode(_cur_freq, wif);
#else
				change_WirelessMode(n, wif);
#endif
#ifdef DUAL_BAND_NONCONCURRENT
		}
		else {
			change_to_another_band(_cur_freq, wif);
			ap_set(wif, "RadioOn=0");
			usleep(20000);
			ap_set(wif, "RadioOn=1");
		}
#endif

		memset(data, 0x00, 255);
		strcpy(data, "SiteSurvey=1");
		wrq.u.data.length = strlen(data)+1;
		wrq.u.data.pointer = data;
		wrq.u.data.flags = 0;

		spinlock_lock(SPINLOCK_SiteSurvey);
		if (wl_ioctl(wif, RTPRIV_IOCTL_SET, &wrq) < 0) {
			spinlock_unlock(0);
			fprintf(stderr, "[httpd] site survey fails...\n");
			goto SiteSurveyFail;
		}
		spinlock_unlock(SPINLOCK_SiteSurvey);
	}

	nvram_set("ap_selecting", "1");
	fprintf(stderr, "Please wait (web hook) ");
	sleep(1);
	fprintf(stderr, ".");
	sleep(1);
	fprintf(stderr, ".");
	sleep(1);
	fprintf(stderr, ".");
	sleep(1);
	fprintf(stderr, ".\n\n");
	nvram_set("ap_selecting", "0");

	for2each(n, wif, nvram_safe_get("wl_ifnames"), wif_next,
			aif, nvram_safe_get("sta_ifnames"), aif_next) {
		i = 0;
		apCount = 0;
		snprintf(prefix, sizeof(prefix), "sta%d_", n);

		memset(data, 0, 8192);
		strcpy(data, "");
		wrq.u.data.length = 8192;
		wrq.u.data.pointer = data;
		wrq.u.data.flags = 0;

		if (wl_ioctl(wif, RTPRIV_IOCTL_GSITESURVEY, &wrq) < 0) {
			fprintf(stderr, "[httpd] get site survey result fails...\n");
			goto SiteSurveyFail;
		}

		memset(apcli_mac, 0, sizeof(apcli_mac));
		get_apcli_address(apcli_mac, aif);
		get_apcli_connStatus(aif, &connStatus);

		if (n == 0)
			fprintf(stderr, "\n%-4s%-33s%-18s%-9s%-16s%-9s\n%-8s%-3s%-4s%-5s%-5s%-3s\n", 
						"Ch", 
						"SSID", 
						"BSSID", 
						"Enc", 
						"Auth", 
						"Siganl(%)", 
						"W-Mode", 
						"NT", 
						"CC", 
						" WPS", 
						" DPID", 
						" EC");

		if (wrq.u.data.length > 0) {
			ssap = (SSA *)(wrq.u.data.pointer + sizeof(SITE_SURVEY));
			int len = strlen(wrq.u.data.pointer + sizeof(SITE_SURVEY));
			char *sp, *op;
			op = sp = wrq.u.data.pointer + sizeof(SITE_SURVEY);

			while (*sp && ((len - (sp-op)) >= 0)) {
				ssap->SiteSurvey[i].channel[3] = '\0';
				ssap->SiteSurvey[i].ssid[32] = '\0';
				ssap->SiteSurvey[i].bssid[17] = '\0';
				ssap->SiteSurvey[i].encryption[8] = '\0';
				ssap->SiteSurvey[i].authmode[15] = '\0';
				ssap->SiteSurvey[i].signal[8] = '\0';
				ssap->SiteSurvey[i].wmode[7] = '\0';
				ssap->SiteSurvey[i].bsstype[2] = '\0';
				ssap->SiteSurvey[i].centralchannel[3] = '\0';
				ssap->SiteSurvey[i].wps[4] = '\0';
				ssap->SiteSurvey[i].dpid[4] = '\0';

				sp += sizeof(SITE_SURVEY);
				apCount = ++i;
			}

			for (i = 0; i < apCount; i++) {
				memset(commch,0,sizeof(commch));
				memcpy(commch,ssap->SiteSurvey[i].channel,3);
				commonchannel=atoi(commch);
				memset(cench,0,sizeof(cench));
				memcpy(cench,ssap->SiteSurvey[i].centralchannel,3);
				centralchannel = atoi(cench);

				if (strstr(ssap->SiteSurvey[i].bsstype, "n") 
						&& (commonchannel != centralchannel)) {
					if (commonchannel <= 4)
						ht_extcha = 1;
					else if (commonchannel > 4 && commonchannel < 8) {
						if (centralchannel < commonchannel)
							ht_extcha = 0;
						else
							ht_extcha = 1;
					}
					else if (commonchannel >= 8) {
						char *value;
						int channellistnum;

						value = nvram_safe_get("wl0_reg");
						if (strcmp(value,"2G_CH11")==0)
							channellistnum = 11;
						else if (strcmp(value,"2G_CH14")==0)
							channellistnum = 14;
						else // 2G_CH13
							channellistnum = 13;

						if ((channellistnum - commonchannel) < 4)
							ht_extcha = 0;
						else {
							if (centralchannel < commonchannel)
								ht_extcha = 0;
							else
								ht_extcha = 1;
						}
					}
				}
				else
					ht_extcha = -1;

				fprintf(stderr, "%-4s%-33s%-18s%-9s%-16s%-9s\n%-8s%-3s%-4s%-5s%-5s%d\n",
						ssap->SiteSurvey[i].channel,
						(char*)ssap->SiteSurvey[i].ssid,
						ssap->SiteSurvey[i].bssid,
						ssap->SiteSurvey[i].encryption,
						ssap->SiteSurvey[i].authmode,
						ssap->SiteSurvey[i].signal,
						ssap->SiteSurvey[i].wmode,
						ssap->SiteSurvey[i].bsstype,
						ssap->SiteSurvey[i].centralchannel,
						ssap->SiteSurvey[i].wps,
						ssap->SiteSurvey[i].dpid,
						ht_extcha);
			}
			fprintf(stderr, "\n");
		}

		for (i = 0; i < apCount; i++) {
			if (validate_ssid(trim_r(ssap->SiteSurvey[i].ssid)))
				continue;
			else {
				if (comma_need)
					ret += websWrite(wp, ",\n");
			}
			comma_need = 1;

			ret += websWrite(wp, "[");

			//SSID
			if (strlen(ssap->SiteSurvey[i].ssid)==0)
				ret += websWrite(wp, "\"\", ");
			else {
				memset(ssid_str, 0, sizeof(ssid_str));
				char_to_ascii(ssid_str, trim_r(ssap->SiteSurvey[i].ssid));
				ret += websWrite(wp, "\"%s\", ", ssid_str);
			}
			//Channel
			memset(cench,0,sizeof(cench));
			memset(commch,0,sizeof(commch));
			memcpy(commch,ssap->SiteSurvey[i].channel,3);
			sprintf(cench,"%d",atoi(commch));
			ret += websWrite(wp, "\"%s\", ", cench);
			//Auth
			if (!strncmp(ssap->SiteSurvey[i].authmode, "OPEN", 4) 
					&& !strncmp(ssap->SiteSurvey[i].encryption, "NONE", 4))
				ret += websWrite(wp, "\"%s\", ", "Open System");
			else if (!strncmp(ssap->SiteSurvey[i].authmode, "WPAPSK", 6))
				ret += websWrite(wp, "\"%s\", ", "WPA-PSK");
			else if (!strncmp(ssap->SiteSurvey[i].authmode, "WPA2PSK", 7))
				ret += websWrite(wp, "\"%s\", ", "WPA2-PSK");
			else
				ret += websWrite(wp, "\"%s\", ", trim_r(ssap->SiteSurvey[i].authmode));
			//Encryp
			ret += websWrite(wp, "\"%s\", ", trim_r(ssap->SiteSurvey[i].encryption));
			//Signal
			ret += websWrite(wp, "\"%s\", ", trim_r(ssap->SiteSurvey[i].signal));
			//BSSID
			ret += websWrite(wp, "\"%s\", ", trim_r(toupperstr(ssap->SiteSurvey[i].bssid)));
			ret += websWrite(wp, "\"%s\", ", trim_r(ssap->SiteSurvey[i].bsstype));
			//Wireless mode
			if (!strcmp(trim_r(ssap->SiteSurvey[i].wmode), "11b"))
				ret += websWrite(wp, "\"%s\", ", "b");
			else if (!strcmp(trim_r(ssap->SiteSurvey[i].wmode), "11a"))
				ret += websWrite(wp, "\"%s\", ", "a");
			else if (!strcmp(trim_r(ssap->SiteSurvey[i].wmode), "11a/n"))
				ret += websWrite(wp, "\"%s\", ", "an");
			else if (!strcmp(trim_r(ssap->SiteSurvey[i].wmode), "11b/g"))
				ret += websWrite(wp, "\"%s\", ", "bg");
			else if (!strcmp(trim_r(ssap->SiteSurvey[i].wmode), "11b/g/n"))
				ret += websWrite(wp, "\"%s\", ", "bgn");
			else
				ret += websWrite(wp, "\"%s\", ", "");
			//Connect status: 0=unknown, 1=connecting, 2=connected
			if (strcmp(apcli_mac, trim_r(toupperstr(ssap->SiteSurvey[i].bssid))) == 0) {
				if (connStatus)
					ret += websWrite(wp, "\"%s\"", "2");
				else
					ret += websWrite(wp, "\"%s\"", "1");
			}
			else
				ret += websWrite(wp, "\"%s\"", "0");

			ret += websWrite(wp, "]");
		}
	}

#ifdef DUAL_BAND_NONCONCURRENT
	if (try_another_band) {
		try_another_band = 0;
		goto TryAnotherBand;
	}
#endif

SiteSurveyFail:
	ret += websWrite(wp, "];");

	for1each(n, wif, nvram_safe_get("wl_ifnames"), wif_next) {
#ifndef DUAL_BAND_NONCONCURRENT
#ifdef IEEE802_11AC
		if (org_wmode[n] != 9 && org_wmode[n] != 14)
#else
		if (org_wmode[n] != 9 && org_wmode[n] != 8)
#endif
#endif
		{
			sprintf(data, "WirelessMode=%d", org_wmode[n]);
			ap_set(wif, data);
		}

#ifdef DUAL_BAND_NONCONCURRENT
		ap_set(wif, "RadioOn=0");
		usleep(20000);
		ap_set(wif, "RadioOn=1");
		if (_sw_mode == 2 
#ifdef SWMODE_HOTSPOT_SUPPORT
				|| _sw_mode == 5
#endif
		) {
			doSystem("iwpriv %s set Channel=%s", wif, org_ch);	//restore origin channel
			eval("killall", "-SIGCONT", "apcli_monitor");		//resume apcli_monitor
		}
#endif
	}

WPSing:
	return ret;
}


static void get_auth_type(char *auth_mode, char *wpa_mode, char *crypto, char *result_char)
{
	if (!strcmp(auth_mode, "open"))
		*result_char = '0';
	else if (!strcmp(auth_mode, "shared"))
		*result_char = '1';
	else if (!strcmp(auth_mode, "psk")) {
		if (!strcmp(wpa_mode, "1"))
			*result_char = '2'; //WPAPSK
		else if (!strcmp(wpa_mode, "2"))
			*result_char = '3'; //WPA2PSK
		else
			*result_char = '2'; //WPAPSK
	}
#ifdef WPA_ENTERPRISE_SUPPORT
	else if (!strcmp(auth_mode, "wpa")) {
		if (!strcmp(wpa_mode, "4")) {
			if (!strcmp(crypto, "tkip"))
				*result_char = '4'; //WPA-enterprise
			else if (!strcmp(crypto, "aes"))
				*result_char = '5'; //WPA2-enterprise
			else
				*result_char = '4'; //WPA-enterprise
		} else
			*result_char = '4'; //WPA-enterprise
	}
	else if (!strcmp(auth_mode, "wpa2"))
		*result_char = '5'; //WPA2-enterprise
#endif
	else
		*result_char = '0';
}

static void get_wep_bits(char *wep_x, char *wep_key_type, char *wep_key, char *result_char)
{
	if (wep_x[0] == '0')
		*result_char = '0';
	else {
		if (!strcmp(wep_key_type, "0")) { // hex
			if (strlen(wep_key) > 10)
				*result_char = '2'; // 128
			else
				*result_char = '1'; // 64
		}
		else {
			if (strlen(wep_key) > 5)
				*result_char = '2'; // 128
			else
				*result_char = '1'; // 64
		}
	}
}

int ej_pap_status(int eid, webs_t wp, int argc, char_t **argv)
{
	char tmp[512];
	FILE *fp;
	int rlen;
	char *pt1,*pt2,*pt3;
	char SSID[65], Freq[4], Channel[30], AUTH, MAC[30], WEP;
	char non_pap_Channel[4];
	char nvram_tmp[128], prefix[] = "staXXXXXXXXXX_";
	char wif[8], aif[8];
	char wep_key_buf[10];
	int pap_link;
	char command[50];

	pap_link = 0;
	SSID[0] = Freq[4] = Channel[0] = MAC[0] = non_pap_Channel[0] = '\0';
	AUTH = WEP = '0';

#ifdef SWMODE_ADAPTER_SUPPORT
	if (nvram_match("sw_mode", "4")) {
snprintf(prefix, sizeof(prefix), "sta0_");//Barton tmp add
		if (nvram_match(strcat_r(prefix, "auth_mode", nvram_tmp), "open") 
				&& (nvram_match(strcat_r(prefix, "wep_x", nvram_tmp), "1") 
					|| nvram_match(strcat_r(prefix, "wep_x", nvram_tmp), "2")) 
				&& nvram_match("open_wep_err", "1")) {
			//printf("pap_status : open-wep disconnected!!!!!\n");
		}
		else {
			fp = popen("iwpriv ra0 connStatus", "r");
			if (fp) {
				rlen = fread(tmp, 1, sizeof(tmp), fp);
				pclose(fp);
				if (rlen > 1) {
					tmp[rlen-1] = '\0';
					pt1 = strstr(tmp, "Connected(AP: ");
					if (pt1) {
						pap_link = 1;
						pt1 += strlen("Connected(AP: ");
						pt3 = pt1;
						pt2 = NULL;
						while (1) {
							pt1 = strstr(pt1, "[");
							if (pt1) {
								pt2 = pt1;
								pt1++;
							} else
								break;
						}
						if (pt2) {
							*pt2++ = '\0';
							strcpy(SSID, pt3);
							pt3 = pt2;
							pt1 = strstr(pt2, "]");
							if (pt1) {
								*pt1 = '\0';
								strcpy(MAC, pt3);
							}
						}

#ifdef DUAL_BAND_NONCONCURRENT
						strcpy(Freq, nvram_match("cur_freq", "1")?"5":"2.4");
#else
						strcpy(Freq, nvram_safe_get("sta_freq"));
#endif
						get_auth_type(nvram_safe_get(strcat_r(prefix, "auth_mode", nvram_tmp)), 
								nvram_safe_get(strcat_r(prefix, "wpa_mode", nvram_tmp)), 
								nvram_safe_get(strcat_r(prefix, "crypto", nvram_tmp)), 
								&AUTH);
						sprintf(wep_key_buf,"sta%s_key%s", 
								nvram_match("sta_freq", "5")?"1":"0", 
								nvram_safe_get(strcat_r(prefix, "key", nvram_tmp)));
						get_wep_bits(nvram_safe_get(strcat_r(prefix, "wep_x", nvram_tmp)), 
								nvram_safe_get(strcat_r(prefix, "key_type", nvram_tmp)),
								nvram_safe_get(wep_key_buf), 
								&WEP);
						fp = popen("iwpriv ra0 show Channel", "r");
						if (fp) {
							rlen = fread(tmp, 1, sizeof(tmp), fp);
							pclose(fp);
							if (rlen > 1) {
								tmp[rlen-1] = '\0';
								pt1 = strstr(tmp, "show:");
								if (pt1) {
									strcpy(Channel, pt1+6);
								}
							}
						}
					}
				}
			}
		}
	} else 
#endif
	if (nvram_match("sw_mode", "2")) {
		int n;
		char _wif[8], *_wif_next, _aif[8], *_aif_next;

		/* find out wireless interface */
		for2each(n, _wif, nvram_safe_get("wl_ifnames"), _wif_next,
				_aif, nvram_safe_get("sta_ifnames"), _aif_next) {
			if ((n == 0 && nvram_match("sta_freq", "2.4")) 
					|| (n == 1 && nvram_match("sta_freq", "5"))) {
				snprintf(prefix, sizeof(prefix), "sta%d_", n);
				strcpy(aif, _aif);
			}
			else
				strcpy(wif, _wif);	// non-parent-AP interface
		}

		if (nvram_match(strcat_r(prefix, "connStatus", nvram_tmp), "1")) {
			pap_link = 1;
			sprintf(command,"iwconfig %s", aif);
			fp = popen(command, "r");
			if (fp) {
				rlen = fread(tmp, 1, sizeof(tmp), fp);
				pclose(fp);
				if (rlen > 1) {
					tmp[rlen-1] = '\0';
					pt1 = strstr(tmp, "ESSID:\"");
					if (pt1) {
						pt2 = pt1 + 7;
						pt1 = strstr(pt2, "\"  ");
						if (pt1) {
							*pt1 = '\0';
							strcpy(SSID, pt2);
							pt1 += 3;
							pt1 = strstr(pt1, "Channel=");
							if (pt1) {
								pt2 = pt1 + strlen("Channel=");
								pt1 = strstr(pt2, " ");
								if (pt1) {
									*pt1++ = '\0';
									strcpy(Channel, pt2);
									pt1 = strstr(pt1, "Access Point: ");
									if (pt1) {
										pt2 = pt1 + strlen("Access Point: ");
										pt1 = strstr(pt2, "\n");
										if (pt1) {
											*pt1 = '\0';
											strcpy(MAC, pt2);
										}
									}
								}
							}
						}
					}
				}
			}
#ifdef DUAL_BAND_NONCONCURRENT
			strcpy(Freq, nvram_match("cur_freq", "1") ? "5" : "2.4");
#else
			strcpy(Freq, nvram_safe_get("sta_freq"));
#endif
			get_auth_type(nvram_safe_get(strcat_r(prefix, "auth_mode", nvram_tmp)), 
					nvram_safe_get(strcat_r(prefix, "wpa_mode", nvram_tmp)), 
					nvram_safe_get(strcat_r(prefix, "crypto", nvram_tmp)), 
					&AUTH);
			sprintf(wep_key_buf, "sta%s_key%s", 
					nvram_match("sta_freq", "5") ? "1" : "0", 
					nvram_safe_get(strcat_r(prefix, "key", nvram_tmp)));
			get_wep_bits(nvram_safe_get(strcat_r(prefix, "wep_x", nvram_tmp)), 
					nvram_safe_get(strcat_r(prefix, "key_type", nvram_tmp)), 
					nvram_safe_get(wep_key_buf), 
					&WEP);

			// for dual band
			if (strstr(nvram_safe_get("www_support"), "5G") != NULL)
				strcpy(non_pap_Channel, get_Channel(wif));
		}
	}

	char tmpSSID[256];
	memset(tmpSSID, 0x0, sizeof(tmpSSID));
	char_to_ascii(tmpSSID, SSID);
	websWrite(wp, "[ \"%s\", \"%c\", \"%s\", \"%s\", \"%s\", \"%d\", \"%c\", \"%s\" ]", 
				tmpSSID, 
				AUTH, 
				Channel, 
				Freq, 
				MAC, 
				pap_link, 
				WEP, 
				non_pap_Channel);

	return 0;
}


/* 
 * Wireless Client List
 */
int ej_wl_auth_list(int eid, webs_t wp, int argc, char_t **argv)
{
	int n;
	char wif[8], *next;
	int ret = 0;
	struct iwreq wrq;
	int first_sta = 1;

#ifdef SWMODE_ADAPTER_SUPPORT
	if (nvram_match("sw_mode", "4"))
		goto exit;
#endif

	for1each(n, wif, nvram_safe_get("wl_ifnames"), next) {
		/* query wl for authenticated sta list */
		char data[8192];
		memset(data, 0, sizeof(data));
		wrq.u.data.pointer = data;
		wrq.u.data.length = sizeof(data);
		wrq.u.data.flags = 0;
		if (wl_ioctl(wif, RTPRIV_IOCTL_GET_MAC_TABLE, &wrq) < 0)
			goto exit;

		/* build wireless sta list */
		RT_802_11_MAC_TABLE *mp = (RT_802_11_MAC_TABLE *)wrq.u.data.pointer;
		int i;
		for (i=0; i<mp->Num; i++) {
			if (first_sta == 1)
				first_sta = 0;
			else
				ret += websWrite(wp, ", ");
			ret += websWrite(wp, "[");

			ret += websWrite(wp, "\"%02x:%02x:%02x:%02x:%02x:%02x\"",
					mp->Entry[i].Addr[0], mp->Entry[i].Addr[1],
					mp->Entry[i].Addr[2], mp->Entry[i].Addr[3],
					mp->Entry[i].Addr[4], mp->Entry[i].Addr[5]);

			/* reserved */
			// associated
			ret += websWrite(wp, ", \"%s\"", "YES");
			// authorized
			ret += websWrite(wp, ", \"%s\"", "");

			ret += websWrite(wp, "]");
		}
	}

exit:
	return ret;
}


#ifdef RUNTIME_CHECK_PAP_PASSWORD
static int runtime_set_apcli_para(webs_t wp, int chk, char *wif, char *aif, int n)
{
	int i, flag_wep = 0;
	char tmp[128], prefix[] = "staXXXXXXXXXX_";
	char buf[65];

	snprintf(prefix, sizeof(prefix), "sta%d_", n);

	char *sta_ssid = chk==1?websGetVar(wp, "sta_ssid", ""):nvram_safe_get(strcat_r(prefix, "ssid", tmp));
	char *sta_bssid = chk==1?websGetVar(wp, "sta_bssid", ""):"";
	char *sta_wpa_psk = chk==1?websGetVar(wp, "sta_wpa_psk", ""):nvram_safe_get(strcat_r(prefix, "wpa_psk", tmp));
	char *sta_auth_mode = chk==1?websGetVar(wp, "sta_auth_mode", ""):nvram_safe_get(strcat_r(prefix, "auth_mode", tmp));
	char *sta_crypto = chk==1?websGetVar(wp, "sta_crypto", ""):nvram_safe_get(strcat_r(prefix, "crypto", tmp));
	char *sta_wpa_mode = chk==1?websGetVar(wp, "sta_wpa_mode", ""):nvram_safe_get(strcat_r(prefix, "wpa_mode", tmp));
	char *sta_wep_x = chk==1?websGetVar(wp, "sta_wep_x", ""):nvram_safe_get(strcat_r(prefix, "wep_x", tmp));
	int sta_key = chk==1?atoi(websGetVar(wp, "sta_key", "")):atoi(nvram_safe_get(strcat_r(prefix, "key", tmp)));
	//char *sta_key_type = chk==1?websGetVar(wp, "sta_key_type", ""):nvram_safe_get(strcat_r(prefix, "key_type", tmp));
	char *sta_key_str[5] = {"", 
				chk==1?websGetVar(wp, "sta_key1", ""):nvram_safe_get(strcat_r(prefix, "key1", tmp)), 
				chk==1?websGetVar(wp, "sta_key2", ""):nvram_safe_get(strcat_r(prefix, "key2", tmp)), 
				chk==1?websGetVar(wp, "sta_key3", ""):nvram_safe_get(strcat_r(prefix, "key3", tmp)), 
				chk==1?websGetVar(wp, "sta_key4", ""):nvram_safe_get(strcat_r(prefix, "key4", tmp))};

	if (!strcmp(sta_ssid, "")) return 0;

	//replace SSID each char to "\char"
	memset(buf, 0x0, sizeof(buf));
	for (i = 0; i < strlen(sta_ssid); i++)
		sprintf(buf, "%s\\%c", buf, sta_ssid[i]);
	buf[2*i+1] = '\0';
	doSystem("iwpriv %s set ApCliSsid=%s", aif, buf);
	doSystem("iwpriv %s set ApCliBssid=%s", aif, sta_bssid);
	if (!strcmp(sta_auth_mode, "open")) {
		ap_set(aif, "ApCliAuthMode=OPEN");

		if (!strcmp(sta_wep_x, "0"))
			ap_set(aif, "ApCliEncrypType=NONE");
		else {
			flag_wep = 1;
			ap_set(aif, "ApCliEncrypType=WEP");
		}
	}
	else if (!strcmp(sta_auth_mode, "shared")) {
		flag_wep = 1;
		ap_set(aif, "ApCliAuthMode=SHARED");
		ap_set(aif, "ApCliEncrypType=WEP");
	}
	else if (!strcmp(sta_auth_mode, "psk")) {
		if (!strcmp(sta_wpa_mode, "1"))
			ap_set(aif, "ApCliAuthMode=WPAPSK");
		else if (!strcmp(sta_wpa_mode, "2"))
			ap_set(aif, "ApCliAuthMode=WPA2PSK");
		if (!strcmp(sta_crypto, "tkip"))
			ap_set(aif, "ApCliEncrypType=TKIP");
		else if (!strcmp(sta_crypto, "aes"))
			ap_set(aif, "ApCliEncrypType=AES");
		//replace PSK key each char to "\char"
		memset(buf, 0x0, sizeof(buf));
		for (i = 0; i < strlen(sta_wpa_psk); i++)
			sprintf(buf, "%s\\%c", buf, sta_wpa_psk[i]);
		buf[2*i+1] = '\0';
		doSystem("iwpriv %s set ApCliWPAPSK=%s", aif, buf);
	}
	if (flag_wep) {
		doSystem("iwpriv %s set ApCliDefaultKeyID=%d", aif, sta_key);
		doSystem("iwpriv %s set ApCliKey%d=%s", aif, sta_key, sta_key_str[sta_key]);
	}
	ap_set(aif, "ApCliEnable=1");

	if (chk == 0) return 0;

	//check apcli connect status
	int connStatus;
	for (i = 0; i < 10; i++) {
		get_apcli_connStatus(aif, &connStatus);
		if (connStatus)
			return 2;
		sleep(1);
	}
	if (!strcmp(sta_auth_mode, "psk"))
		return 3;
	else
		return 1;

}

#ifdef SWMODE_ADAPTER_SUPPORT
static int runtime_set_sta_para(webs_t wp, int chk, char *wif, int n)
{
	int flag_wep = 0;
	char tmp[128], prefix[] = "staXXXXXXXXXX_";

	snprintf(prefix, sizeof(prefix), "sta%d_", n);

	char *sta_ssid = chk==1?websGetVar(wp, "sta_ssid", ""):nvram_safe_get(strcat_r(prefix, "ssid", tmp));
	char *sta_wpa_psk = chk==1?websGetVar(wp, "sta_wpa_psk", ""):nvram_safe_get(strcat_r(prefix, "wpa_psk", tmp));
	char *sta_auth_mode = chk==1?websGetVar(wp, "sta_auth_mode", ""):nvram_safe_get(strcat_r(prefix, "auth_mode", tmp));
	char *sta_crypto = chk==1?websGetVar(wp, "sta_crypto", ""):nvram_safe_get(strcat_r(prefix, "crypto", tmp));
	char *sta_wpa_mode = chk==1?websGetVar(wp, "sta_wpa_mode", ""):nvram_safe_get(strcat_r(prefix, "wpa_mode", tmp));
	char *sta_wep_x = chk==1?websGetVar(wp, "sta_wep_x", ""):nvram_safe_get(strcat_r(prefix, "wep_x", tmp));
	int sta_key = chk==1?atoi(websGetVar(wp, "sta_key", "")):atoi(nvram_safe_get(strcat_r(prefix, "key", tmp)));
	//char *sta_key_type = chk==1?websGetVar(wp, "sta_key_type", ""):nvram_safe_get(strcat_r(prefix, "key_type", tmp));
	char *sta_key_str[5] = {"", 
				chk==1?websGetVar(wp, "sta_key1", ""):nvram_safe_get(strcat_r(prefix, "key1", tmp)), 
				chk==1?websGetVar(wp, "sta_key2", ""):nvram_safe_get(strcat_r(prefix, "key2", tmp)), 
				chk==1?websGetVar(wp, "sta_key3", ""):nvram_safe_get(strcat_r(prefix, "key3", tmp)), 
				chk==1?websGetVar(wp, "sta_key4", ""):nvram_safe_get(strcat_r(prefix, "key4", tmp))};

	if (!strcmp(sta_ssid, "")) return 0;

	//replace SSID each char to "\char"
	char buf[65];
	int i;
	memset(buf, 0x0, sizeof(buf));
	for (i=0 ; i<strlen(sta_ssid) ; i++)
		sprintf(buf, "%s\\%c", buf, sta_ssid[i]);
	buf[2*i+1] = '\0';

	doSystem("iwpriv %s set SSID=%s", wif, buf);
	if (!strcmp(sta_auth_mode, "open")) {
		ap_set(wif, "AuthMode=OPEN");

		if (!strcmp(sta_wep_x, "0"))
			ap_set(wif, "EncrypType=NONE");
		else {
			flag_wep = 1;
			ap_set(wif, "EncrypType=WEP");
		}
	}
	else if (!strcmp(sta_auth_mode, "shared")) {
		flag_wep = 1;
		ap_set(wif, "AuthMode=SHARED");
		ap_set(wif, "EncrypType=WEP");
	}
	else if (!strcmp(sta_auth_mode, "psk")) {
		if (!strcmp(sta_wpa_mode, "1"))
			ap_set(wif, "AuthMode=WPAPSK");
		else if (!strcmp(sta_wpa_mode, "2"))
			ap_set(wif, "AuthMode=WPA2PSK");
		if (!strcmp(sta_crypto, "tkip"))
			ap_set(wif, "EncrypType=TKIP");
		else if (!strcmp(sta_crypto, "aes"))
			ap_set(wif, "EncrypType=AES");
		doSystem("iwpriv %s set WPAPSK=%s", wif, sta_wpa_psk);
	}
	if (flag_wep) {
		doSystem("iwpriv %s set DefaultKeyID=%d", wif, sta_key);
		doSystem("iwpriv %s set Key%d=%s", wif, sta_key, sta_key_str[sta_key]);
	}

	if (chk == 0) return 0;

	sleep(10);

	//check sta connect status
	int connStatus;
	if (ea_connStatus())
		connStatus = 2;
	else if (flag_wep)
		connStatus = 1;
	else
		connStatus = 3;

	return connStatus;
}
#endif

int ej_check_pap_password(int eid, webs_t wp, int argc, char_t **argv)
{
	int _sw_mode = atoi(nvram_safe_get("sw_mode"));
	int n, ret =0;
	char wif[8], *wif_next, aif[8], *aif_next;
	char org_ch[4];
#ifdef DUAL_BAND_NONCONCURRENT
	int _cur_freq = atoi(nvram_safe_get("cur_freq"));
#else
	int sta_band = atoi(websGetVar(wp, "sta_band", ""));
#endif
	int sta_ch = atoi(websGetVar(wp, "sta_ch", ""));
#ifdef SWMODE_REPEATER_EXPRESSWAY_SUPPORT
	int re_expressway = atoi(nvram_safe_get("re_expressway"));
#endif
	int connStatus;

	for2each(n, wif, nvram_safe_get("wl_ifnames"), wif_next,
			aif, nvram_safe_get("sta_ifnames"), aif_next) {
#ifndef DUAL_BAND_NONCONCURRENT
		if (n == sta_band)
#endif
			break;
	}

	switch (_sw_mode) {
		case 1:
		case 2:
		case 3:
#ifdef SWMODE_HOTSPOT_SUPPORT
		case 5:
#endif
#ifdef USB_SUPPORT
		case 6:
#endif
			if (_sw_mode == 2 
#ifdef SWMODE_HOTSPOT_SUPPORT
					|| _sw_mode == 5
#endif
			) {
#ifdef SWMODE_REPEATER_EXPRESSWAY_SUPPORT
				if (IsExpressWayCli())
					eval("ifconfig", aif, "up");
#endif
				eval("killall", "-SIGTSTP", "apcli_monitor");	//pause apcli_monitor
			}
			else
				eval("ifconfig", aif, "up");

#ifdef DUAL_BAND_NONCONCURRENT
			strcpy(org_ch, get_Channel(aif));			//for EA-N66 save origin channel
			if ((_cur_freq && sta_ch <= 14) || (!_cur_freq && sta_ch > 14))
				change_to_another_band(_cur_freq, wif);
#else
			strcpy(org_ch, get_Channel(wif));			//save origin channel
#endif
			doSystem("iwpriv %s set Channel=%d", wif, sta_ch);
			connStatus = runtime_set_apcli_para(wp, 1, wif, aif, n);
			ret += websWrite(wp, "%d", connStatus);
#ifdef DUAL_BAND_NONCONCURRENT
			if ((_cur_freq && sta_ch <= 14) || (!_cur_freq && sta_ch > 14))
				go_back_original_band(_cur_freq, wif);
#endif
			doSystem("iwpriv %s set Channel=%s", wif, org_ch);	//restore origin channel
			runtime_set_apcli_para(wp, 0, wif, aif, n);

			if (_sw_mode == 2 
#ifdef SWMODE_HOTSPOT_SUPPORT
					|| _sw_mode == 5
#endif
			) {
#ifdef SWMODE_REPEATER_EXPRESSWAY_SUPPORT
				if (IsExpressWayCli())
					eval("ifconfig", aif, "down");
#endif
				eval("killall", "-SIGCONT", "apcli_monitor");   //resume apcli_monitor
			}
			else
				eval("ifconfig", aif, "down");
			break;
#ifdef SWMODE_ADAPTER_SUPPORT
		case 4:
#ifdef DUAL_BAND_NONCONCURRENT
			if ((_cur_freq && sta_ch <= 14) || (!_cur_freq && sta_ch > 14))
				change_to_another_band(_cur_freq, wif);
#endif
			connStatus = runtime_set_sta_para(wp, 1, wif, n);
			ret += websWrite(wp, "%d", connStatus);
#ifdef DUAL_BAND_NONCONCURRENT
			if ((_cur_freq && sta_ch <= 14) || (!_cur_freq && sta_ch > 14))
				go_back_original_band(_cur_freq, wif);
#endif
			runtime_set_sta_para(wp, 0, wif, n);
			break;
#endif
		default:
			fprintf(stderr, "### %s: wrong sw_mode! ###\n", __FUNCTION__);
	}

#ifdef APP_SUPPORT
	FILE *fp;

	if ((fp = fopen(CHECK_PAP_PASSWORD_RESULT, "w")) != NULL) {
		fprintf(fp, "%d", connStatus);
		fclose(fp);
	}
#endif

	return ret;
}
#endif /* RUNTIME_CHECK_PAP_PASSWORD */

#ifdef WPS_SUPPORT
typedef struct PACKED _WSC_CONFIGURED_VALUE {
	unsigned short WscConfigured;	// 1 un-configured; 2 configured
	unsigned char WscSsid[32 + 1];
	unsigned short WscAuthMode;	// mandatory, 0x01: open, 0x02: wpa-psk, 0x04: shared, 0x08:wpa, 0x10: wpa2, 0x
	unsigned short WscEncrypType;	// 0x01: none, 0x02: wep, 0x04: tkip, 0x08: aes
	unsigned char DefaultKeyIdx;
	unsigned char WscWPAKey[64 + 1];
} WSC_CONFIGURED_VALUE;

/*
 * these definitions are from rt2860v2 driver include/wsc.h 
 */
static char *getWscStatusStr(int status)
{
	switch (status) {
	case 0:
		return "Not used";
	case 1:
		return "Idle";
	case 2:
		//return "WPS Fail(Ignore this if Intel/Marvell registrar used)";
		return "Idle";
	case 3:
		return "Start WPS Process";
	case 4:
		return "Received EAPOL-Start";
	case 5:
		return "Sending EAP-Req(ID)";
	case 6:
		return "Receive EAP-Rsp(ID)";
	case 7:
		return "Receive EAP-Req with wrong WPS SMI Vendor Id";
	case 8:
		return "Receive EAP-Req with wrong WPS Vendor Type";
	case 9:
		return "Sending EAP-Req(WPS_START)";
	case 10:
		return "Send M1";
	case 11:
		return "Received M1";
	case 12:
		return "Send M2";
	case 13:
		return "Received M2";
	case 14:
		return "Received M2D";
	case 15:
		return "Send M3";
	case 16:
		return "Received M3";
	case 17:
		return "Send M4";
	case 18:
		return "Received M4";
	case 19:
		return "Send M5";
	case 20:
		return "Received M5";
	case 21:
		return "Send M6";
	case 22:
		return "Received M6";
	case 23:
		return "Send M7";
	case 24:
		return "Received M7";
	case 25:
		return "Send M8";
	case 26:
		return "Received M8";
	case 27:
		return "Processing EAP Response (ACK)";
	case 28:
		return "Processing EAP Request (Done)";
	case 29:
		return "Processing EAP Response (Done)";
	case 30:
		return "Sending EAP-Fail";
	case 31:
		return "WPS_ERROR_HASH_FAIL";
	case 32:
		return "WPS_ERROR_HMAC_FAIL";
	case 33:
		return "WPS_ERROR_DEV_PWD_AUTH_FAIL";
	case 34:
		//return "Configured";
		return "Success";
	case 35:
		return "SCAN AP";
	case 36:
		return "EAPOL START SENT";
	case 37:
		return "WPS_EAP_RSP_DONE_SENT";
	case 38:
		return "WAIT PINCODE";
	case 39:
		return "WSC_START_ASSOC";
	case 0x101:
		return "PBC:TOO MANY AP";
	case 0x102:
		return "PBC:NO AP";
	case 0x103:
		return "EAP_FAIL_RECEIVED";
	case 0x104:
		return "EAP_NONCE_MISMATCH";
	case 0x105:
		return "EAP_INVALID_DATA";
	case 0x106:
		return "PASSWORD_MISMATCH";
	case 0x107:
		return "EAP_REQ_WRONG_SMI";
	case 0x108:
		return "EAP_REQ_WRONG_VENDOR_TYPE";
	case 0x109:
		return "PBC_SESSION_OVERLAP";
	default:
		return "Unknown";
	}
}

static int getWscStatus(char *wif)
{
	int data = 0;
	struct iwreq wrq;

	wrq.u.data.length = sizeof(data);
	wrq.u.data.pointer = (caddr_t) &data;
	wrq.u.data.flags = RT_OID_WSC_QUERY_STATUS;

	if (wl_ioctl(wif, RT_PRIV_IOCTL, &wrq) < 0)
		fprintf(stderr, "errors in getting WSC status\n");

	return data;
}

static unsigned int getAPPIN(char *wif)
{
	unsigned int data = 0;
	struct iwreq wrq;

	wrq.u.data.length = sizeof(data);
	wrq.u.data.pointer = (caddr_t) &data;
	wrq.u.data.flags = RT_OID_WSC_PIN_CODE;

	if (wl_ioctl(wif, RT_PRIV_IOCTL, &wrq) < 0)
		fprintf(stderr, "errors in getting AP PIN\n");

	return data;
}

int ej_wps_info(int eid, webs_t wp, int argc, char_t **argv)
{
	int n;
	char wif[8], *next;
	WSC_CONFIGURED_VALUE result;
	struct iwreq wrq;
	char tmp[128], prefix[] = "wlXXXXXXXXXX_";
	int ret = 0;

	ret += websWrite(wp, "<wps>\n");
	for1each(n, wif, nvram_safe_get("wl_ifnames"), next) {
		ret += websWrite(wp, "<wps%d>\n", n);

		wrq.u.data.length = sizeof(WSC_CONFIGURED_VALUE);
		wrq.u.data.pointer = (caddr_t) &result;
		wrq.u.data.flags = 0;
		strcpy((char *)&result, "get_wsc_profile");

		if (wl_ioctl(wif, RTPRIV_IOCTL_WSC_PROFILE, &wrq) < 0) {
			fprintf(stderr, "errors in getting WSC profile\n");
			return 0;
		}

		//0. WSC Status
		ret += websWrite(wp, "<wps_info>%s</wps_info>\n", getWscStatusStr(getWscStatus(wif)));
		//1. WPSConfigured
		if (result.WscConfigured==2)
			ret += websWrite(wp, "<wps_info>%s</wps_info>\n", "Yes");
		else
			ret += websWrite(wp, "<wps_info>%s</wps_info>\n", "No");
		//2. AP PIN Code
		ret += websWrite(wp, "<wps_info>%08d</wps_info>\n", getAPPIN(wif));
		//3. auth mode
		snprintf(prefix, sizeof(prefix), "wl%d_", n);
		ret += websWrite(wp, "<wps_info>%s</wps_info>\n", nvram_safe_get(strcat_r(prefix, "auth_mode", tmp)));
		//4. HideSSID
		ret += websWrite(wp, "<wps_info>%s</wps_info>\n", nvram_safe_get(strcat_r(prefix, "closed", tmp)));
		//5. radio on/off
		snprintf(prefix, sizeof(prefix), "sw%d_", n);
		ret += websWrite(wp, "<wps_info>%s</wps_info>\n", nvram_safe_get(strcat_r(prefix, "radio", tmp)));

		ret += websWrite(wp, "</wps%d>\n", n);
	}
	ret += websWrite(wp, "</wps>");

	return ret;
}
#endif /* WPS_SUPPORT */
