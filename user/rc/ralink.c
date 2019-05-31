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

#include <nvram/bcmnvram.h>
#include <linux/autoconf.h>
#include <ralink_gpio.h>

#include <semaphore_mfp.h>
#include <shared.h>
#include <ralink.h>
typedef unsigned char bool;
#include <wlioctl.h>
#include <iwlib.h>

#include "rc.h"
#include "rc_event.h"
#include "watchdog.h"

#ifdef ASUS_ATE
#define MAX_FRW 64 //for FWRITE

static unsigned char nibble_hex(char *c)
{
	int val;
	char tmpstr[3];

	tmpstr[2] = '\0';
	memcpy(tmpstr, c, 2);
	val = strtoul(tmpstr, NULL, 16);

	return (unsigned char)val;
}

static int atoh(const char *a, unsigned char *e)
{
	char *c = (char *) a;
	int i = 0;

	memset(e, 0, MAX_FRW);
	for (;;) {
		if ((*c == '\0') || (*(c+1) == '\0'))
			break;
		e[i++] = (unsigned char) nibble_hex(c);
		if (i == MAX_FRW)
			break;
		c += 2;
	}
	return i;
}

int FWRITE(char *da, char* str_hex)
{
	unsigned char ee[MAX_FRW];
	unsigned int addr_da;
	int len;

	addr_da = strtoul(da, NULL, 16);
	if (addr_da && (len = atoh(str_hex, ee)))
		FWrite(ee, addr_da, len);

	return 0;
}

int set40M_Channel(int band, char *channel)
{
	char buf[16];
	int n;
	char wif[8], *next;

	if (channel == NULL || !isValidChannel(band, channel))
		return 0;

	/* find out wireless interface */
	for1each(n, wif, nvram_safe_get("wl_ifnames"), next) {
		if (n == band)
			break;
	}

	sprintf(buf,"Channel=%s", channel);
	ap_set(wif, buf);
	ap_set(wif, "HtBw=1");
	puts("1");

	return 1;
}

int getChannelList(int band)
{
	char ch_list[128];
	int n;
	char wif[8], *next;

	/* find out wireless interface */
	for1each(n, wif, nvram_safe_get("wl_ifnames"), next) {
#ifdef DUAL_BAND_NONCONCURRENT
		change_WirelessMode(band, wif);
#else
		if (n == band)
#endif
			break;
	}

	if (get_channel_list_via_driver(wif, ch_list) > 0)
		puts(ch_list);

	return 0;
}

int getrssi(int band)
{
	char data[32];
	int n;
	char wif[8], *next;

	/* find out wireless interface */
	for1each(n, wif, nvram_safe_get("wl_ifnames"), next) {
		if (n == band)
			break;
	}

	memset(data, 0x00, sizeof(data));

	if (asuscmd_wl_ioctl(wif, ASUS_SUBCMD_GRSSI, data) < 0) {
		dbg("errors in getting RSSI result\n");
		return 0;
	}

	if (strlen(data) > 0)
		puts(data);

	return 0;
}

int asuscfe(int band, const char *PwqV)
{
	int n;
	char wif[8], *next;

	/* find out wireless interface */
	for1each(n, wif, nvram_safe_get("wl_ifnames"), next) {
		if (n == band)
			break;
	}

	if (strcmp(PwqV, "stat") == 0) {
		eval("iwpriv", wif, "stat");
	}
	else if (strstr(PwqV, "=") && strstr(PwqV, "=") != PwqV) {
		eval("iwpriv", wif, "set", PwqV);
		puts("1");
	}

	return 0;
}

int eeprom_upgrade(const char *path, const char *band, int is_bk)
{
	#define FLASH_OFFSET 0x40000
	FILE *fp = NULL;
	char buf_org[512], buf_new[512];
	int ret = 0;
	int len, offset;
	int band_offset;

	if ((!path) || (!band)) goto quit_out;
	if (band[0] == '5')
		band_offset = 0x8000;
	else
		band_offset = 0;

	fp = fopen(path, "r");
	if (!fp) goto quit_out;

	len = fread(buf_new, 1, 512, fp);
	if (len != 512) goto quit_out;

	if (is_bk) {
		FRead(buf_org, FLASH_OFFSET+band_offset, 512);
		offset = MAC_EEPROM_ADDR - FLASH_OFFSET;
		memcpy(buf_new+offset, buf_org+offset, 6);
	}
	FWrite(buf_new, FLASH_OFFSET+band_offset, 512);
	ret = 1;

quit_out:
	if (fp) fclose(fp);
	if (ret)
		fprintf(stderr, "success!\n");
	else
		fprintf(stderr, "fail!\n");
	return ret;
}
#endif /* ASUS_ATE */

int checkReg2G(char *str)
{
	if (strcmp(str,"2G_CH11")==0)
		return 0;
	else if (strcmp(str,"2G_CH14")==0)
		return 5;
	else
		return 1;
}

int checkReg5G(char *str)
{
	// reference MTK driver include/rtmp_def.h, REGION_0_A_BAND - REGION_16_A_BAND
	if (strcmp(str,"5G_BAND14")==0)
		return 10;
	else if (strcmp(str,"5G_BAND24")==0)
		return 16;
	else if (strcmp(str,"5G_BAND1")==0)
		return 6;
	else if (strcmp(str,"5G_BAND4")==0)
		return 4;
	else if (strcmp(str,"5G_BAND123")==0) {
#ifdef IEEE802_11H
		if (nvram_match("reg_spec", "UA"))
			return 18;
		else
			return 1;
#else
		return 6;
#endif
	}
#ifdef EEPROM_BACKWARD_COMPATIBILITY
	else if (strcmp(str,"5G_BAND124")==0)
		return 0;
	else if (strcmp(str,"5G_BAND12")==0)
		return 2;
	else if (strcmp(str,"5G_BAND24_")==0)
		return 3;
	else if (strcmp(str,"5G_BAND4_")==0)
		return 5;
	else if (strcmp(str,"5G_ALL_")==0)
		return 9;
#endif
	else if (strcmp(str,"5G_ALL")==0)
		return 7;
	else
		return 7;
}

static int getFirstChannel(int band, char *reg)
{
	if (band == 0)
		return 1;
	else {
		if (strncmp(reg, "5G_BAND1", 8) == 0)
			return 36;
		else if (strncmp(reg, "5G_BAND2", 8) == 0)
			return 56;
		//else if (strncmp(reg, "5G_BAND3", 8) == 0)
		//	return ;
		else if (strncmp(reg, "5G_BAND4", 8) == 0)
			return 149;
		else
			return 36;
	}
}

#ifdef SWMODE_ADAPTER_SUPPORT
int gen_sta_config(int band, int is_iNIC)
{
	FILE *fp;
	char tmp[128], prefix[] = "wlXXXXXXX_", sta_prefix[] = "staXXXXXXX_";
	char *str;
	int warning = 0;

	printf("--- start to gen ralink STA config. ---\n");
	mkdir("/etc/Wireless", 0755);
	if (!is_iNIC) {
		mkdir("/etc/Wireless/RT2860", 0755);
		if(!(fp=fopen("/etc/Wireless/RT2860/RT2860.dat", "w+")))
			return 0;
	} else {
		mkdir("/etc/Wireless/iNIC", 0755);
		if (!(fp=fopen("/etc/Wireless/iNIC/iNIC_ap.dat", "w+")))
			return 0;
	}

	fprintf(fp, "Default\n");

	snprintf(prefix, sizeof(prefix), "wl%d_", band);
	snprintf(sta_prefix, sizeof(sta_prefix), "sta%d_", band);

#ifdef WPA_SUPPLICANT_SUPPORT
	fprintf(fp, "EthConvertMode=dongle\n");
#else	
	fprintf(fp, "EthConvertMode=hybrid\n");
#endif
	char mac[18];
	memset(mac, 0, 18);
	memcpy(mac, nvram_safe_get("sta_clonemac"), 17);
	if(strlen(mac) == 17)
		fprintf(fp, "EthCloneMac=%s\n", mac);
	else
		fprintf(fp, "EthCloneMac=ff:ff:ff:ff:ff:ff\n");

	//CountryRegion
	str = nvram_safe_get("wl0_reg");
	if (str)
		fprintf(fp, "CountryRegion=%d\n", checkReg2G(str));
	else
		fprintf(fp, "CountryRegion=5\n");

	//CountryRegion for A band
	str = nvram_safe_get("wl1_reg");
	if (str)
		fprintf(fp, "CountryRegionABand=%d\n", checkReg5G(str));
	else
		fprintf(fp, "CountryRegionABand=7\n");

	//CountryCode
	fprintf(fp, "CountryCode=\n");

	fprintf(fp, "SSID=%s\n", nvram_safe_get(strcat_r(sta_prefix, "ssid", tmp)));
	fprintf(fp, "NetworkType=%s\n", nvram_safe_get("NetworkType"));

	//WirelessMode
	int wmode;
	wmode = wl_nmode2wmode(band, atoi(nvram_safe_get(strcat_r(prefix, "nmode_x", tmp))));
	fprintf(fp, "WirelessMode=%d\n", wmode);

	//Channel
	str = nvram_safe_get(strcat_r(prefix, "channel", tmp));
	if (str)
		fprintf(fp, "Channel=%s\n", str);
	else
		fprintf(fp, "Channel=0\n");

	//BGProtection
	if (band == 1)
		fprintf(fp, "BGProtection=2\n");
	else {
		str = nvram_safe_get(strcat_r(prefix, "gmode_protection", tmp));
		if (str) {
			if (!strcmp(str, "auto"))
				fprintf(fp, "BGProtection=0\n");  // sync to ASUS-WRT
			else
				fprintf(fp, "BGProtection=2\n");
		}
		else {
			warning = 1;
			fprintf(fp, "BGProtection=0\n");
		}
	}

	//TxPreamble
	str = nvram_safe_get(strcat_r(prefix, "TxPreamble", tmp));
	if (str)
		fprintf(fp, "TxPreamble=%s\n", str);
	else {
		warning = 2;
		fprintf(fp, "TxPreamble=0\n");
	}

	//RTSThreshold  Default=2347
	str = nvram_safe_get(strcat_r(prefix, "rts", tmp));
	if (str)
		fprintf(fp, "RTSThreshold=%s\n", str);
	else {
		warning = 3;
		fprintf(fp, "RTSThreshold=2347\n");
	}

	//FragThreshold  Default=2346
	str = nvram_safe_get(strcat_r(prefix, "frag", tmp));
	if (str)
		fprintf(fp, "FragThreshold=%s\n", str);
	else {
		warning = 4;
		fprintf(fp, "FragThreshold=2346\n");
	}

	//TxBurst
	str = nvram_safe_get(strcat_r(prefix, "TxBurst", tmp));
	if (str)
		fprintf(fp, "TxBurst=%s\n", str);
	else {
		warning = 5;
		fprintf(fp, "TxBurst=0\n");
	}

	//PktAggregate
	str = nvram_safe_get(strcat_r(prefix, "PktAggregate", tmp));
	if (str)
		fprintf(fp, "PktAggregate=%s\n", str);
	else {
		warning = 6;
		fprintf(fp, "PktAggregate=1\n");
	}

	//WmmCapable
	char wmm_enable[8];
	memset(wmm_enable, 0x0, sizeof(wmm_enable));

	str = nvram_safe_get(strcat_r(prefix, "nmode_x", tmp));
	if (str && atoi(str) == 1)	// always enable WMM in N only mode
		sprintf(wmm_enable+strlen(wmm_enable), "%d", 1);
	else
		sprintf(wmm_enable+strlen(wmm_enable), "%d", atoi(nvram_safe_get(strcat_r(prefix, "wme", tmp))));

	fprintf(fp, "WmmCapable=%s\n", wmm_enable);

	//AuthMode
	char *staX_auth_mode, *staX_crypto, *staX_wep_x, *staX_wpa_mode;
	staX_auth_mode = nvram_safe_get(strcat_r(sta_prefix, "auth_mode", tmp));
	staX_crypto = nvram_safe_get(strcat_r(sta_prefix, "crypto", tmp));
	staX_wep_x = nvram_safe_get(strcat_r(sta_prefix, "wep_x", tmp));
	staX_wpa_mode = nvram_safe_get(strcat_r(sta_prefix, "wpa_mode", tmp));
	if (staX_auth_mode) {
		if (!strcmp(staX_auth_mode, "open") && !strcmp(staX_wep_x, "0"))
			fprintf(fp, "AuthMode=OPEN\n");
		else if (!strcmp(staX_auth_mode, "open") || !strcmp(staX_auth_mode, "shared"))
			fprintf(fp, "AuthMode=WEPAUTO\n");
		else if (!strcmp(staX_auth_mode, "psk")) {
			if (!strcmp(staX_wpa_mode, "1"))
				fprintf(fp, "AuthMode=WPAPSK\n");
			else if (!strcmp(staX_wpa_mode, "2"))
				fprintf(fp, "AuthMode=WPA2PSK\n");
		}
#ifdef WPA_ENTERPRISE_SUPPORT
		else if (!strcmp(staX_auth_mode, "wpa")) {
			if (!strcmp(staX_wpa_mode, "4")) {
				if (!strcmp(staX_crypto, "tkip"))
					fprintf(fp, "AuthMode=WPA\n");
				else if (!strcmp(staX_crypto, "aes"))
					fprintf(fp, "AuthMode=WPA2\n");
				else
					fprintf(fp, "AuthMode=WPA\n");
			}
			else
				fprintf(fp, "AuthMode=WPA\n");
		}
		else if (!strcmp(staX_auth_mode, "wpa2"))
			fprintf(fp, "AuthMode=WPA2\n");
#endif
		else
			fprintf(fp, "AuthMode=OPEN\n");
	}
	else {
		warning = 7;
		fprintf(fp, "AuthMode=OPEN\n");
	}

	//EncrypType
	if (!strcmp(staX_auth_mode, "open") && !strcmp(staX_wep_x, "0"))
		fprintf(fp, "EncrypType=NONE\n");
	else if ((!strcmp(staX_auth_mode, "open") && strcmp(staX_wep_x, "0") != 0)
			|| !strcmp(staX_auth_mode, "shared")
			|| !strcmp(staX_auth_mode, "radius"))
		fprintf(fp, "EncrypType=WEP\n");
	else if (!strcmp(staX_crypto, "tkip"))
		fprintf(fp, "EncrypType=TKIP\n");
	else if (!strcmp(staX_crypto, "aes"))
		fprintf(fp, "EncrypType=AES\n");
	else if (!strcmp(staX_crypto, "tkip+aes"))
		fprintf(fp, "EncrypType=TKIP\n");
	else {
		warning = 8;
		fprintf(fp, "EncrypType=NONE\n");
	}

	//DefaultKeyID
	fprintf(fp, "DefaultKeyID=%s\n", nvram_safe_get(strcat_r(sta_prefix, "key", tmp)));

	str = nvram_safe_get(strcat_r(sta_prefix, "key_type", tmp));
	//Key1Type(0 -> Hex, 1->Ascii)
	fprintf(fp, "Key1Type=%s\n", str);
	//Key1Str
	fprintf(fp, "Key1Str=%s\n", nvram_safe_get(strcat_r(sta_prefix, "key1", tmp)));

	//Key2Type(0 -> Hex, 1->Ascii)
	fprintf(fp, "Key2Type=%s\n", str);
	//Key2Str
	fprintf(fp, "Key2Str=%s\n", nvram_safe_get(strcat_r(sta_prefix, "key2", tmp)));

	//Key3Type(0 -> Hex, 1->Ascii)
	fprintf(fp, "Key3Type=%s\n", str);
	//Key3Str
	fprintf(fp, "Key3Str=%s\n", nvram_safe_get(strcat_r(sta_prefix, "key3", tmp)));

	//Key4Type(0 -> Hex, 1->Ascii)
	fprintf(fp, "Key4Type=%s\n", str);
	//Key4Str
	fprintf(fp, "Key4Str=%s\n", nvram_safe_get(strcat_r(sta_prefix, "key4", tmp)));

	//WPAPSK
	fprintf(fp, "WPAPSK=%s\n", nvram_safe_get(strcat_r(sta_prefix, "wpa_psk", tmp)));

	fprintf(fp, "PSMode=CAM\n");
	fprintf(fp, "AutoRoaming=0\n");
	fprintf(fp, "RoamThreshold=70\n");

	//HT_RDG
	str = nvram_safe_get(strcat_r(prefix, "HT_RDG", tmp));
	if (str)
		fprintf(fp, "HT_RDG=%s\n", str);
	else {
		warning = 9;
		fprintf(fp, "HT_RDG=1\n");
	}

//Barton...need fix
	//HT_EXTCHA
	str = nvram_safe_get("HT_EXTCHA");
	if(strlen(str) > 0)
		fprintf(fp, "HT_EXTCHA=%s\n", str);
	else
		fprintf(fp, "HT_EXTCHA=1\n");

	//HT_OpMode
	str = nvram_safe_get(strcat_r(prefix, "HT_OpMode", tmp));
	if (str)
		fprintf(fp, "HT_OpMode=%s\n", str);
	else {
		warning = 10;
		fprintf(fp, "HT_OpMode=0\n");
	}

	//HT_MpduDensity
	str = nvram_safe_get(strcat_r(prefix, "HT_MpduDensity", tmp));
	if (str)
		fprintf(fp, "HT_MpduDensity=%s\n", str);
	else {
		warning = 11;
		fprintf(fp, "HT_MpduDensity=5\n");
	}

//Barton...need fix
	//HT_BW
	str = nvram_safe_get("HT_BW");
	if(strlen(str) > 0)
		fprintf(fp, "HT_BW=%s\n", str);
	else
		fprintf(fp, "HT_BW=1\n");

	//HT_AutoBA
	str = nvram_safe_get(strcat_r(prefix, "HT_AutoBA", tmp));
	if (str)
		fprintf(fp, "HT_AutoBA=%s\n", str);
	else {
		warning = 12;
		fprintf(fp, "HT_AutoBA=1\n");
	}

	//HT_AMSDU
	str = nvram_safe_get(strcat_r(prefix, "HT_AMSDU", tmp));
	if (str)
		fprintf(fp, "HT_AMSDU=%s\n", str);
	else {
		warning = 13;
		fprintf(fp, "HT_AMSDU=0\n");
	}

	//HT_BAWinSize
	str = nvram_safe_get(strcat_r(prefix, "HT_BAWinSize", tmp));
	if (str)
		fprintf(fp, "HT_BAWinSize=%s\n", str);
	else {
		warning = 14;
		fprintf(fp, "HT_BAWinSize=64\n");
	}

	//HT_GI
	str = nvram_safe_get(strcat_r(prefix, "HT_GI", tmp));
	if (str)
		fprintf(fp, "HT_GI=%s\n", str);
	else {
		warning = 15;
		fprintf(fp, "HT_GI=1\n");
	}

	//HT_MCS
	str = nvram_safe_get(strcat_r(prefix, "HT_MCS", tmp));
	if (str)
		fprintf(fp, "HT_MCS=%s\n", str);
	else {
		warning = 16;
		fprintf(fp, "HT_MCS=33\n");
	}

	fprintf(fp, "HT_MIMOPSMode=3\n");

	//HT_BADecline
	str = nvram_safe_get(strcat_r(prefix, "HT_BADecline", tmp));
	if (str)
		fprintf(fp, "HT_BADecline=%s\n", str);
	else {
		warning = 17;
		fprintf(fp, "HT_BADecline=0\n");
	}

#ifdef IEEE802_11H
	int flag_80211h = 0;
#ifdef DUAL_BAND_NONCONCURRENT
	if (nvram_match("cur_freq", "1") && nvram_match("wl1_reg", "5G_BAND123"))
#else
	if (band == 1 
			&& nvram_match("wl1_reg", "5G_BAND123") 
			&& (nvram_match("reg_spec", "CE") 
				|| nvram_match("reg_spec", "JP") 
				|| nvram_match("reg_spec", "UA")))
#endif
		flag_80211h = 1;

	if (flag_80211h)
		fprintf(fp, "IEEE80211H=1\n");
	else
#endif
		fprintf(fp, "IEEE80211H=0\n");

#ifdef IEEE802_11H
	if (flag_80211h) {
#ifdef EEPROM_BACKWARD_COMPATIBILITY
		fprintf(fp, "RDRegion=%s\n", getDfsStandard(nvram_safe_get("wl_country_code")));
#else
//TBD
#endif
	}
#endif

	//TGnWifiTest
	fprintf(fp, "TGnWifiTest=0\n");

	fprintf(fp, "WirelessEvent=0\n");
	fprintf(fp, "CarrierDetect=0\n");

#ifdef WPS_SUPPORT
	fprintf(fp, "WscManufacturer=ASUSTeK Computer Inc.\n");
	fprintf(fp, "WscModelName=Wireless Station\n");
	fprintf(fp, "WscDeviceName=ASUS Wireless Station\n");
	fprintf(fp, "WscModelNumber=%s\n", nvram_safe_get("fac_model_name"));
	fprintf(fp, "WscSerialNumber=00000000\n");
#endif

	if (warning) {
		printf("%d!!!!\n", warning);
		printf("Miss some configuration, please check!!!!\n");
	}

	fclose(fp);
	return 0;
}
#endif

int gen_ralink_config(int band, int is_iNIC)
{
	FILE *fp;
	char tmp[128], prefix[] = "wlXXXXXXX_", sta_prefix[] = "staXXXXXXX_";
	char *str = NULL;
	int _sw_mode = atoi(nvram_safe_get("sw_mode"));
	int warning = 0;
	int  i;

#ifdef SWMODE_ADAPTER_SUPPORT
	if (_sw_mode == 4) {
		gen_sta_config(band, is_iNIC);
		return 0;
	}
#endif
	printf("--- start to gen ralink AP config. ---\n");
	mkdir("/etc/Wireless", 0755);
	if (!is_iNIC) {
		mkdir("/etc/Wireless/RT2860", 0755);
		if(!(fp=fopen("/etc/Wireless/RT2860/RT2860.dat", "w+")))
			return 0;
	} else {
		mkdir("/etc/Wireless/iNIC", 0755);
		if (!(fp=fopen("/etc/Wireless/iNIC/iNIC_ap.dat", "w+")))
			return 0;
	}

	fprintf(fp, "#The word of \"Default\" must not be removed\n");
	fprintf(fp, "Default\n");

	snprintf(prefix, sizeof(prefix), "wl%d_", band);
	snprintf(sta_prefix, sizeof(sta_prefix), "sta%d_", band);

	//CountryRegion
	str = nvram_safe_get("wl0_reg");
	if (str)
		fprintf(fp, "CountryRegion=%d\n", checkReg2G(str));
	else
		fprintf(fp, "CountryRegion=5\n");

	//CountryRegion for A band
	str = nvram_safe_get("wl1_reg");
	if (str)
		fprintf(fp, "CountryRegionABand=%d\n", checkReg5G(str));
	else {
		warning = 1;
		fprintf(fp, "CountryRegionABand=7\n");
	}

	//CountryCode
	char *reg_spec = nvram_safe_get("reg_spec");
#ifdef CE_ADAPTIVITY
	if (!strcmp(reg_spec, "CE") || !strcmp(reg_spec, "UA")) {
		fprintf(fp, "CountryCode=FR\n");
#if (defined(CONFIG_DEFAULTS_ASUS_RPN14) 		\
	|| defined(CONFIG_DEFAULTS_ASUS_RPAC52) 	\
	|| defined(CONFIG_DEFAULTS_ASUS_WMPN12))
		if (!band)
			fprintf(fp, "ED_MODE=1\n");
		else
#endif
		{
		fprintf(fp, "EDCCA_AP_STA_TH=30\n");
		fprintf(fp, "EDCCA_AP_AP_TH=30\n");
		fprintf(fp, "EDCCA_FALSE_CCA_TH=3000\n");
		fprintf(fp, "EDCCA_ED_TH=90\n");
		fprintf(fp, "EDCCA_BLOCK_CHECK_TH=2\n");
		fprintf(fp, "EDCCA_AP_RSSI_TH=-80\n");
		}
	}
	else
#endif
	fprintf(fp, "CountryCode=\n");

	fprintf(fp, "ChannelGeography=1\n");

	//SSID Num. [MSSID Only]
	int ssid_num = 1;
	str = nvram_safe_get("BssidNum");
	if (str) {
		fprintf(fp, "BssidNum=%s\n", str);
		ssid_num = atoi(str);
	}
	else {
		warning = 2;
		fprintf(fp, "BssidNum=1\n");
		ssid_num = 1;
	}

	//SSID
	str = nvram_safe_get(strcat_r(prefix, "ssid", tmp));
	if (str)
		fprintf(fp, "SSID1=%s\n", str);
	else {
		warning = 3;
		fprintf(fp, "SSID1=default\n");
	}

	fprintf(fp, "SSID2=\n");
	fprintf(fp, "SSID3=\n");
	fprintf(fp, "SSID4=\n");
	fprintf(fp, "SSID5=\n");
	fprintf(fp, "SSID6=\n");
	fprintf(fp, "SSID7=\n");
	fprintf(fp, "SSID8=\n");

	//WirelessMode
	int wmode;
	wmode = wl_nmode2wmode(band, atoi(nvram_safe_get(strcat_r(prefix, "nmode_x", tmp))));
	fprintf(fp, "WirelessMode=%d\n", wmode);

	fprintf(fp, "TxRate=0\n");

	//Channel
	int wlX_channel = atoi(nvram_safe_get(strcat_r(prefix, "channel", tmp)));
	char *staX_ssid = nvram_safe_get(strcat_r(sta_prefix, "ssid", tmp));
	if ((_sw_mode == 2 
#ifdef SWMODE_HOTSPOT_SUPPORT
				|| _sw_mode == 5 
#endif
			) && strcmp(staX_ssid, "") != 0)
		fprintf(fp, "Channel=%d\n", getFirstChannel(band, nvram_safe_get(strcat_r(prefix, "reg", tmp))));
	else if (wlX_channel != 0)
		fprintf(fp, "Channel=%d\n", wlX_channel);

	//BasicRate
	str = nvram_safe_get(strcat_r(prefix, "rateset", tmp));
	if (str) {
		if (!strcmp(str, "default"))	// 1, 2, 5.5, 11
			fprintf(fp, "BasicRate=15\n");
		else if (!strcmp(str, "all"))	// 1, 2, 5.5, 6, 11, 12, 24
			fprintf(fp, "BasicRate=351\n");
		else if (!strcmp(str, "12"))	// 1, 2
			fprintf(fp, "BasicRate=3\n");
		else
			fprintf(fp, "BasicRate=15\n");
	}
	else {
		warning = 5;
		fprintf(fp, "BasicRate=15\n");
	}

	//BeaconPeriod
	str = nvram_safe_get(strcat_r(prefix, "bcn", tmp));
	if (str)
		fprintf(fp, "BeaconPeriod=%s\n", str);
	else {
		warning = 6;
		fprintf(fp, "BeaconPeriod=100\n");
	}

	//DTIM Period
	str = nvram_safe_get(strcat_r(prefix, "dtim", tmp));
	if (str)
		fprintf(fp, "DtimPeriod=%s\n", str);
	else {
		warning = 7;
		fprintf(fp, "DtimPeriod=1\n");
	}

	//TxPower
	str = nvram_safe_get(strcat_r(prefix, "TxPower", tmp));
	if (str)
		fprintf(fp, "TxPower=%s\n", str);
	else {
		warning = 8;
		fprintf(fp, "TxPower=100\n");
	}

	//DisableOLBC
	str = nvram_safe_get(strcat_r(prefix, "DisableOLBC", tmp));
	if (str)
		fprintf(fp, "DisableOLBC=%s\n", str);
	else {
		warning = 9;
		fprintf(fp, "DisableOLBC=0\n");
	}

	//BGProtection
	if (band == 1)
		fprintf(fp, "BGProtection=2\n");
	else {
		str = nvram_safe_get(strcat_r(prefix, "gmode_protection", tmp));
		if (str) {
			if (!strcmp(str, "auto"))
				fprintf(fp, "BGProtection=0\n");  // sync to ASUS-WRT
			else
				fprintf(fp, "BGProtection=2\n");
		}
		else {
			warning = 10;
			fprintf(fp, "BGProtection=0\n");
		}
	}

	//TxAntenna
	fprintf(fp, "TxAntenna=\n");

	//RxAntenna
	fprintf(fp, "RxAntenna=\n");

	//TxPreamble
	str = nvram_safe_get(strcat_r(prefix, "TxPreamble", tmp));
	if (str)
		fprintf(fp, "TxPreamble=%s\n", str);
	else {
		warning = 11;
		fprintf(fp, "TxPreamble=0\n");
	}

	//RTSThreshold  Default=2347
	str = nvram_safe_get(strcat_r(prefix, "rts", tmp));
	if (str)
		fprintf(fp, "RTSThreshold=%s\n", str);
	else {
		warning = 12;
		fprintf(fp, "RTSThreshold=2347\n");
	}

	//FragThreshold  Default=2346
	str = nvram_safe_get(strcat_r(prefix, "frag", tmp));
	if (str)
		fprintf(fp, "FragThreshold=%s\n", str);
	else {
		warning = 13;
		fprintf(fp, "FragThreshold=2346\n");
	}

	//TxBurst
	str = nvram_safe_get(strcat_r(prefix, "TxBurst", tmp));
	if (str)
		fprintf(fp, "TxBurst=%s\n", str);
	else {
		warning = 14;
		fprintf(fp, "TxBurst=0\n");
	}

	//PktAggregate
	str = nvram_safe_get(strcat_r(prefix, "PktAggregate", tmp));
	if (str)
		fprintf(fp, "PktAggregate=%s\n", str);
	else {
		warning = 15;
		fprintf(fp, "PktAggregate=1\n");
	}

	fprintf(fp, "TurboRate=0\n");

	//WmmCapable
	char wmm_enable[8];
	memset(wmm_enable, 0x0, sizeof(wmm_enable));

	str = nvram_safe_get(strcat_r(prefix, "nmode_x", tmp));
	if (str && atoi(str) == 1)	// always enable WMM in N only mode
		sprintf(wmm_enable+strlen(wmm_enable), "%d", 1);
	else
		sprintf(wmm_enable+strlen(wmm_enable), "%d", atoi(nvram_safe_get(strcat_r(prefix, "wme", tmp))));

	fprintf(fp, "WmmCapable=%s\n", wmm_enable);

	//APAifsn
	fprintf(fp, "APAifsn=%s\n", nvram_safe_get("APAifsn"));
	//APCwmin
	fprintf(fp, "APCwmin=%s\n", nvram_safe_get("APCwmin"));
	//APCwmax
	fprintf(fp, "APCwmax=%s\n", nvram_safe_get("APCwmax"));
	//APTxop
	fprintf(fp, "APTxop=%s\n", nvram_safe_get("APTxop"));
	//APACM
	fprintf(fp, "APACM=%s\n", nvram_safe_get("APACM"));
	//BSSAifsn
	fprintf(fp, "BSSAifsn=%s\n", nvram_safe_get("BSSAifsn"));
	//BSSCwmin
	fprintf(fp, "BSSCwmin=%s\n", nvram_safe_get("BSSCwmin"));
	//BSSCwmax
	fprintf(fp, "BSSCwmax=%s\n", nvram_safe_get("BSSCwmax"));
	//BSSTxop
	fprintf(fp, "BSSTxop=%s\n", nvram_safe_get("BSSTxop"));
	//BSSACM
	fprintf(fp, "BSSACM=%s\n", nvram_safe_get("BSSACM"));
	//AckPolicy
	char wmm_noack[8];
	bzero(wmm_noack, sizeof(char)*8);
	for (i = 0; i < 4; i++) {
		sprintf(wmm_noack+strlen(wmm_noack), "%d", strcmp(nvram_safe_get(strcat_r(prefix, "wme_no_ack", tmp)), "on")? 0 : 1);
		sprintf(wmm_noack+strlen(wmm_noack), "%c", ';');
	}
	wmm_noack[strlen(wmm_noack) - 1] = '\0';
	fprintf(fp, "AckPolicy=%s\n", wmm_noack);

	//APSDCapable
	str = nvram_safe_get(strcat_r(prefix, "APSDCapable", tmp));
	if (str)
		fprintf(fp, "APSDCapable=%s\n", str);
	else {
		warning = 16;
		fprintf(fp, "APSDCapable=0\n");
	}

	//DLSDCapable
	str = nvram_safe_get(strcat_r(prefix, "DLSCapable", tmp));
	if (str)
		fprintf(fp, "DLSCapable=%s\n", str);
	else {
		warning = 17;
		fprintf(fp, "DLSCapable=0\n");
	}

	//NoForwarding pre SSID & NoForwardingBTNBSSID
	str = nvram_safe_get(strcat_r(prefix, "ap_isolate", tmp));
	if (str) {
		fprintf(fp, "NoForwarding=%s\n", str);
		fprintf(fp, "NoForwardingBTNBSSID=%s\n", str);
	}
	else {
		warning = 18;
		fprintf(fp, "NoForwarding=0\n");
		fprintf(fp, "NoForwardingBTNBSSID=0\n");
	}

	//HideSSID
	fprintf(fp, "HideSSID=%s\n", nvram_safe_get(strcat_r(prefix, "closed", tmp)));

	//ShortSlot
	fprintf(fp, "ShortSlot=1\n");

	//AutoChannelSelect
	if (((_sw_mode == 2 
#ifdef SWMODE_HOTSPOT_SUPPORT
				|| _sw_mode == 5 
#endif
			) && strcmp(staX_ssid, "") != 0) 
			|| wlX_channel != 0)
		fprintf(fp, "AutoChannelSelect=0\n");
	else
		fprintf(fp, "AutoChannelSelect=1\n");

	//AutoChannelSkipList
	fprintf(fp, "AutoChannelSkipList=%s\n", nvram_safe_get(strcat_r(prefix, "AutoChannelSkipList", tmp)));

	//AuthMode
	char *wlX_auth_mode, *wlX_crypto;
	int flag_8021x = 0, flag_80211h = 0;
	wlX_auth_mode = nvram_safe_get(strcat_r(prefix, "auth_mode", tmp));
	wlX_crypto = nvram_safe_get(strcat_r(prefix, "crypto", tmp));

#ifdef IEEE802_11H
#ifdef DUAL_BAND_NONCONCURRENT
	if (nvram_match("cur_freq", "1") && nvram_match("wl1_reg", "5G_BAND123"))
#else
	if (band == 1 
			&& nvram_match("wl1_reg", "5G_BAND123") 
			&& (!strcmp(reg_spec, "CE") 
				|| !strcmp(reg_spec, "JP") 
				|| !strcmp(reg_spec, "UA")))
#endif
		flag_80211h = 1;
#endif

	if (wlX_auth_mode) {
		if (!strcmp(wlX_auth_mode, "open")) {
			//IEEE8021X
			fprintf(fp, "IEEE8021X=0\n");
			fprintf(fp, "IEEE80211H=%d\n", flag_80211h);
			fprintf(fp, "CSPeriod=%s\n", nvram_safe_get("CSPeriod"));
			fprintf(fp, "PreAuth=%s\n", nvram_safe_get("PreAuth"));

			fprintf(fp, "AuthMode=OPEN\n");
		}
		else if (!strcmp(wlX_auth_mode, "shared")) {
			//IEEE8021X
			fprintf(fp, "IEEE8021X=0\n");
			fprintf(fp, "IEEE80211H=%d\n", flag_80211h);
			fprintf(fp, "CSPeriod=%s\n", nvram_safe_get("CSPeriod"));
			fprintf(fp, "PreAuth=%s\n", nvram_safe_get("PreAuth"));

			fprintf(fp, "AuthMode=SHARED\n");
		}
		else if (!strcmp(wlX_auth_mode, "psk")) {
			//IEEE8021X
			fprintf(fp, "IEEE8021X=0\n");
			fprintf(fp, "IEEE80211H=%d\n", flag_80211h);
			fprintf(fp, "CSPeriod=%s\n", nvram_safe_get("CSPeriod"));
			fprintf(fp, "PreAuth=%s\n", nvram_safe_get("PreAuth"));

			if(nvram_match(strcat_r(prefix, "wpa_mode", tmp), "0"))
				fprintf(fp, "AuthMode=WPAPSKWPA2PSK\n");
			else if(nvram_match(strcat_r(prefix, "wpa_mode", tmp), "1"))
				fprintf(fp, "AuthMode=WPAPSK\n");
			else if(nvram_match(strcat_r(prefix, "wpa_mode", tmp), "2"))
				fprintf(fp, "AuthMode=WPA2PSK\n");
		}
#ifdef WPA_ENTERPRISE_SUPPORT
		else if (!strcmp(wlX_auth_mode, "wpa")) {
			//IEEE8021X
			flag_8021x=1;
			fprintf(fp, "IEEE8021X=0\n");
			fprintf(fp, "IEEE80211H=%d\n", flag_80211h);
			fprintf(fp, "CSPeriod=%s\n", nvram_safe_get("CSPeriod"));
			fprintf(fp, "PreAuth=%s\n", nvram_safe_get("PreAuth"));

			if (nvram_match(strcat_r(prefix, "wpa_mode", tmp), "4")) {
				if (!strcmp(wlX_crypto, "tkip"))
					fprintf(fp, "AuthMode=WPA\n");
				else if (!strcmp(wlX_crypto, "aes"))
					fprintf(fp, "AuthMode=WPA2\n");
				else
					fprintf(fp, "AuthMode=WPA1WPA2\n");
			}
			else if(nvram_match(strcat_r(prefix, "wpa_mode", tmp), "3"))
				fprintf(fp, "AuthMode=WPA\n");
		}
		else if (!strcmp(wlX_auth_mode, "wpa2")) {
			//IEEE8021X
			flag_8021x=1;
			fprintf(fp, "IEEE8021X=0\n");
			fprintf(fp, "IEEE80211H=%d\n", flag_80211h);
			fprintf(fp, "CSPeriod=%s\n", nvram_safe_get("CSPeriod"));
			fprintf(fp, "PreAuth=%s\n", nvram_safe_get("PreAuth"));

			fprintf(fp, "AuthMode=WPA2\n");
		}
		else if (!strcmp(wlX_auth_mode, "radius")) {
			//IEEE8021X
			flag_8021x=1;
			fprintf(fp, "IEEE8021X=1\n");
			fprintf(fp, "IEEE80211H=%d\n", flag_80211h);
			fprintf(fp, "CSPeriod=%s\n", nvram_safe_get("CSPeriod"));
			fprintf(fp, "PreAuth=%s\n", nvram_safe_get("PreAuth"));

			fprintf(fp, "AuthMode=OPEN\n");
		}
#endif
		else {
			//IEEE8021X
			fprintf(fp, "IEEE8021X=0\n");
			fprintf(fp, "IEEE80211H=%d\n", flag_80211h);
			fprintf(fp, "CSPeriod=%s\n", nvram_safe_get("CSPeriod"));
			fprintf(fp, "PreAuth=%s\n", nvram_safe_get("PreAuth"));

			fprintf(fp, "AuthMode=OPEN\n");
		}
	}
	else {
		warning = 20;
		fprintf(fp, "AuthMode=OPEN\n");
	}

	//EncrypType
	if ((!strcmp(wlX_auth_mode, "open") && nvram_match(strcat_r(prefix, "wep_x", tmp), "0")))
		fprintf(fp, "EncrypType=NONE\n");
	else if ((!strcmp(wlX_auth_mode, "open") && nvram_invmatch(strcat_r(prefix, "wep_x", tmp), "0")) 
			|| !strcmp(wlX_auth_mode, "shared") 
			|| !strcmp(wlX_auth_mode, "radius"))
		fprintf(fp, "EncrypType=WEP\n");
	else if (!strcmp(wlX_crypto, "tkip"))
		fprintf(fp, "EncrypType=TKIP\n");
	else if (!strcmp(wlX_crypto, "aes"))
		fprintf(fp, "EncrypType=AES\n");
	else if (!strcmp(wlX_crypto, "tkip+aes"))
		fprintf(fp, "EncrypType=TKIPAES\n");
	else {
		warning = 21;
		fprintf(fp, "EncrypType=NONE\n");
	}

	//RekeyInterval
	str = nvram_safe_get(strcat_r(prefix, "wpa_gtk_rekey", tmp));
	if (str) {
		fprintf(fp, "RekeyInterval=%s\n", str);
		if (atoi(str)==0)
			fprintf(fp, "RekeyMethod=DISABLE\n");
		else
			fprintf(fp, "RekeyMethod=%s\n", nvram_safe_get("RekeyMethod"));
	}
	else {
		warning = 22;
		fprintf(fp, "RekeyInterval=0\n");
		fprintf(fp, "RekeyMethod=DISABLE\n");
	}

	//PMKCachePeriod
	str = nvram_safe_get("PMKCachePeriod");
	if (str)
		fprintf(fp, "PMKCachePeriod=%s\n", str);
	else {
		warning = 23;
		fprintf(fp, "PMKCachePeriod=10\n");
	}

	//WPAPSK
	fprintf(fp, "WPAPSK1=%s\n", nvram_safe_get(strcat_r(prefix, "wpa_psk", tmp)));

	fprintf(fp, "WPAPSK2=\n");
	fprintf(fp, "WPAPSK3=\n");
	fprintf(fp, "WPAPSK4=\n");
	fprintf(fp, "WPAPSK5=\n");
	fprintf(fp, "WPAPSK6=\n");
	fprintf(fp, "WPAPSK7=\n");
	fprintf(fp, "WPAPSK8=\n");

	//DefaultKeyID
	fprintf(fp, "DefaultKeyID=%s\n", nvram_safe_get(strcat_r(prefix, "key", tmp)));

	//Key1Type(0 -> Hex, 1->Ascii)
	fprintf(fp, "Key1Type=%s\n", nvram_safe_get(strcat_r(prefix, "key_type", tmp)));
	//Key1Str
	fprintf(fp, "Key1Str1=%s\n", nvram_safe_get(strcat_r(prefix, "key1", tmp)));
	fprintf(fp, "Key1Str2=\n");
	fprintf(fp, "Key1Str3=\n");
	fprintf(fp, "Key1Str4=\n");
	fprintf(fp, "Key1Str5=\n");
	fprintf(fp, "Key1Str6=\n");
	fprintf(fp, "Key1Str7=\n");
	fprintf(fp, "Key1Str8=\n");

	//Key2Type
	fprintf(fp, "Key2Type=%s\n", nvram_safe_get(strcat_r(prefix, "key_type", tmp)));
	//Key2Str
	fprintf(fp, "Key2Str1=%s\n", nvram_safe_get(strcat_r(prefix, "key2", tmp)));
	fprintf(fp, "Key2Str2=\n");
	fprintf(fp, "Key2Str3=\n");
	fprintf(fp, "Key2Str4=\n");
	fprintf(fp, "Key2Str5=\n");
	fprintf(fp, "Key2Str6=\n");
	fprintf(fp, "Key2Str7=\n");
	fprintf(fp, "Key2Str8=\n");

	//Key3Type
	fprintf(fp, "Key3Type=%s\n", nvram_safe_get(strcat_r(prefix, "key_type", tmp)));
	//Key3Str
	fprintf(fp, "Key3Str1=%s\n", nvram_safe_get(strcat_r(prefix, "key3", tmp)));
	fprintf(fp, "Key3Str2=\n");
	fprintf(fp, "Key3Str3=\n");
	fprintf(fp, "Key3Str4=\n");
	fprintf(fp, "Key3Str5=\n");
	fprintf(fp, "Key3Str6=\n");
	fprintf(fp, "Key3Str7=\n");
	fprintf(fp, "Key3Str8=\n");

	//Key4Type
	fprintf(fp, "Key4Type=%s\n", nvram_safe_get(strcat_r(prefix, "key_type", tmp)));
	//Key4Str
	fprintf(fp, "Key4Str1=%s\n", nvram_safe_get(strcat_r(prefix, "key4", tmp)));
	fprintf(fp, "Key4Str2=\n");
	fprintf(fp, "Key4Str3=\n");
	fprintf(fp, "Key4Str4=\n");
	fprintf(fp, "Key4Str5=\n");
	fprintf(fp, "Key4Str6=\n");
	fprintf(fp, "Key4Str7=\n");
	fprintf(fp, "Key4Str8=\n");

	fprintf(fp, "HSCounter=0\n");

#ifdef IEEE802_11H
	if (flag_80211h) {
#ifdef EEPROM_BACKWARD_COMPATIBILITY
		fprintf(fp, "RDRegion=%s\n", getDfsStandard(nvram_safe_get("wl_country_code")));
#else
		if (!strcmp(reg_spec, "CE") || !strcmp(reg_spec, "UA"))
			fprintf(fp, "RDRegion=CE\n");
		else if (!strcmp(reg_spec, "JP"))
			fprintf(fp, "RDRegion=JAP\n");
		else
			fprintf(fp, "RDRegion=\n");
#endif
	}
	else
#endif
		fprintf(fp, "RDRegion=\n");

	//HT_HTC
	str = nvram_safe_get(strcat_r(prefix, "HT_HTC", tmp));
	if (str)
		fprintf(fp, "HT_HTC=%s\n", str);
	else {
		warning = 24;
		fprintf(fp, "HT_HTC=1\n");
	}

	//HT_RDG
	str = nvram_safe_get(strcat_r(prefix, "HT_RDG", tmp));
	if (str)
		fprintf(fp, "HT_RDG=%s\n", str);
	else {
		warning = 25;
		fprintf(fp, "HT_RDG=1\n");
	}

	//HT_LinkAdapt
	str = nvram_safe_get(strcat_r(prefix, "HT_LinkAdapt", tmp));
	if (str)
		fprintf(fp, "HT_LinkAdapt=%s\n", str);
	else {
		warning = 26;
		fprintf(fp, "HT_LinkAdapt=0\n");
	}

	//HT_OpMode
	str = nvram_safe_get(strcat_r(prefix, "HT_OpMode", tmp));
	if (str)
		fprintf(fp, "HT_OpMode=%s\n", str);
	else {
		warning = 27;
		fprintf(fp, "HT_OpMode=0\n");
	}

	//HT_MpduDensity
	str = nvram_safe_get(strcat_r(prefix, "HT_MpduDensity", tmp));
	if (str)
		fprintf(fp, "HT_MpduDensity=%s\n", str);
	else {
		warning = 28;
		fprintf(fp, "HT_MpduDensity=5\n");
	}

	//HT_EXTCHA
	int HTBW_MAX = 1;
	if (_sw_mode != 2 
#ifdef SWMODE_HOTSPOT_SUPPORT
			&& _sw_mode != 5 
#endif
			&& wlX_channel != 0) {
		if (!band) {
			int EXTCHA = 0;
			int EXTCHA_MAX = 0;
			int Channel_MAX = 11;

			if (wlX_channel < 8)
				EXTCHA_MAX = 1;
			else {
				char *value = nvram_safe_get("wl0_reg");

				if (!strcmp(value, "2G_CH11"))
					Channel_MAX = 11;
				else if (!strcmp(value, "2G_CH14"))
					Channel_MAX = 14;
				else	// 2G_CH13
					Channel_MAX = 13;

				if ((Channel_MAX - wlX_channel) < 4)
					EXTCHA_MAX = 0;
				else
					EXTCHA_MAX = 1;
			}

			str = nvram_safe_get(strcat_r(prefix, "nctrlsb", tmp));
			int extcha = !strcmp(str, "lower") ? 1 : 0;
			if (str && strlen(str)) {
				if (wlX_channel <= 4)
					fprintf(fp, "HT_EXTCHA=1\n");
				else if (extcha <= EXTCHA_MAX)
					fprintf(fp, "HT_EXTCHA=%d\n", extcha);
				else
					fprintf(fp, "HT_EXTCHA=0\n");
			}
		}
		else {
			if ((wlX_channel == 36) 
					|| (wlX_channel == 44) 
					|| (wlX_channel == 52) 
					|| (wlX_channel == 60) 
					|| (wlX_channel == 100) 
					|| (wlX_channel == 108) 
					|| (wlX_channel == 116) 
					|| (wlX_channel == 124) 
					|| (wlX_channel == 132) 
					|| (wlX_channel == 149) 
					|| (wlX_channel == 157))
				fprintf(fp, "HT_EXTCHA=1\n");
			else if ((wlX_channel == 40) 
					|| (wlX_channel == 48) 
					|| (wlX_channel == 56) 
					|| (wlX_channel == 64) 
					|| (wlX_channel == 104) 
					|| (wlX_channel == 112) 
					|| (wlX_channel == 120) 
					|| (wlX_channel == 128) 
					|| (wlX_channel == 136) 
					|| (wlX_channel == 153) 
					|| (wlX_channel == 161))
				fprintf(fp, "HT_EXTCHA=0\n");
			else
				HTBW_MAX = 0;
		}
	}

	//HT_BW
	str = nvram_safe_get(strcat_r(prefix, "HT_BW", tmp));

	if (atoi(str) > 0 && HTBW_MAX == 1)
		fprintf(fp, "HT_BW=1\n");
	else
		fprintf(fp, "HT_BW=0\n");

	//HT_BSSCoexistence
	if (str && !strcmp(str, "1"))
		fprintf(fp, "HT_BSSCoexistence=1\n");	// 20 Only and 20/40 coexistence support
	else
		fprintf(fp, "HT_BSSCoexistence=0\n");	// 40 Only

	//HT_AutoBA
	str = nvram_safe_get(strcat_r(prefix, "HT_AutoBA", tmp));
	if (str)
		fprintf(fp, "HT_AutoBA=%s\n", str);
	else {
		warning = 29;
		fprintf(fp, "HT_AutoBA=1\n");
	}

	//HT_AMSDU
	str = nvram_safe_get(strcat_r(prefix, "HT_AMSDU", tmp));
	if (str)
		fprintf(fp, "HT_AMSDU=%s\n", str);
	else {
		warning = 30;
		fprintf(fp, "HT_AMSDU=0\n");
	}

	//HT_BAWinSize
	str = nvram_safe_get(strcat_r(prefix, "HT_BAWinSize", tmp));
	if (str)
		fprintf(fp, "HT_BAWinSize=%s\n", str);
	else {
		warning = 31;
		fprintf(fp, "HT_BAWinSize=64\n");
	}

	//HT_GI
	str = nvram_safe_get(strcat_r(prefix, "HT_GI", tmp));
	if (str)
		fprintf(fp, "HT_GI=%s\n", str);
	else {
		warning = 32;
		fprintf(fp, "HT_GI=1\n");
	}

	//HT_STBC
	str = nvram_safe_get(strcat_r(prefix, "HT_STBC", tmp));
	if (str)
		fprintf(fp, "HT_STBC=%s\n", str);
	else {
		warning = 33;
		fprintf(fp, "HT_STBC=1\n");
	}

	//HT_MCS
	str = nvram_safe_get(strcat_r(prefix, "HT_MCS", tmp));
	if (str)
		fprintf(fp, "HT_MCS=%s\n", str);
	else {
		warning = 34;
		fprintf(fp, "HT_MCS=33\n");
	}

	//HT_TxStream
	str = nvram_safe_get(strcat_r(prefix, "HT_TxStream", tmp));
	if (str)
		fprintf(fp, "HT_TxStream=%s\n", str);
	else {
		warning = 35;
		fprintf(fp, "HT_TxStream=2\n");
	}

	//HT_RxStream
	str = nvram_safe_get(strcat_r(prefix, "HT_RxStream", tmp));
	if (str)
		fprintf(fp, "HT_RxStream=%s\n", str);
	else {
		warning = 36;
		fprintf(fp, "HT_RxStream=2\n");
	}

	//HT_BADecline
	str = nvram_safe_get(strcat_r(prefix, "HT_BADecline", tmp));
	if (str)
		fprintf(fp, "HT_BADecline=%s\n", str);
	else {
		warning = 37;
		fprintf(fp, "HT_BADecline=0\n");
	}

	//HT_PROTECT
	str = nvram_safe_get(strcat_r(prefix, "HT_PROTECT", tmp));
	if (str)
		fprintf(fp, "HT_PROTECT=%s\n", str);
	else {
		warning = 38;
		fprintf(fp, "HT_PROTECT=1\n");
	}

	//HT_DisallowTKIP
	str = nvram_safe_get(strcat_r(prefix, "HT_DisallowTKIP", tmp));
	if (str) {
		if (atoi(str) == 1)
			fprintf(fp, "HT_DisallowTKIP=1\n");
		else
			fprintf(fp, "HT_DisallowTKIP=0\n");
	}
	else {
		warning = 39;
		fprintf(fp, "HT_DisallowTKIP=0\n");
	}

#ifdef IEEE802_11AC
	//VHT_BW, VHT_DisallowNonVHT
	int VHTBW_MAX = 0;
	if (wmode == 14 || wmode == 15)
		VHTBW_MAX = 1;

	str = nvram_safe_get(strcat_r(prefix, "HT_BW", tmp));
	if (band != 1) {
	}
	else if (str && (strcmp(str, "1") == 0) && (HTBW_MAX == 1) && (VHTBW_MAX == 1)) {	// Auto
		fprintf(fp, "VHT_BW=1\n");
		fprintf(fp, "VHT_DisallowNonVHT=0\n");
	}
	else if (str && (strcmp(str, "3") == 0) && (HTBW_MAX == 1) && (VHTBW_MAX == 1)) {	// 80 MHz Only
		fprintf(fp, "VHT_BW=1\n");
		fprintf(fp, "VHT_DisallowNonVHT=1\n");
	}
	else {
		fprintf(fp, "VHT_BW=0\n");
		fprintf(fp, "VHT_DisallowNonVHT=0\n");
	}

	fprintf(fp,"VHT_STBC=0\n");
	fprintf(fp,"VHT_SGI=1\n");
	fprintf(fp,"VHT_BW_SIGNAL=0\n");
#endif

	//GreenAP
	fprintf(fp, "GreenAP=0\n");

	//WscConfStatus
	int wsc_configure = 0;
	str = nvram_safe_get(strcat_r(prefix, "wsc_config_state", tmp));
	if (str)
		wsc_configure = atoi(str);
	else {
		warning = 40;
		wsc_configure = 0;
	}
	
	if ((wsc_configure == 0) && (nvram_match("x_Setting", "0"))) {
		fprintf(fp, "WscConfMode=0\n");
		fprintf(fp, "WscConfStatus=1\n");
	}
	else {
		fprintf(fp, "WscConfMode=0\n");
		fprintf(fp, "WscConfStatus=2\n");
	}

	
	fprintf(fp, "WscVendorPinCode=%s\n", nvram_safe_get("secret_code"));

	//AccessPolicy0
	char list[2048];
	str = nvram_safe_get(strcat_r(prefix, "macmode", tmp));
	if (str) {
		if (!strcmp(str, "disabled")) {
			fprintf(fp, "AccessPolicy0=0\n");
			fprintf(fp, "AccessPolicy1=0\n");
			fprintf(fp, "AccessPolicy2=0\n");
			fprintf(fp, "AccessPolicy3=0\n");
		}
		else if (!strcmp(str, "allow")) {
			fprintf(fp, "AccessPolicy0=1\n");
			fprintf(fp, "AccessPolicy1=1\n");
			fprintf(fp, "AccessPolicy2=1\n");
			fprintf(fp, "AccessPolicy3=1\n");
		}
		else if (!strcmp(str, "deny")) {
			fprintf(fp, "AccessPolicy0=2\n");
			fprintf(fp, "AccessPolicy1=2\n");
			fprintf(fp, "AccessPolicy2=2\n");
			fprintf(fp, "AccessPolicy3=2\n");
		}
		else {
			fprintf(fp, "AccessPolicy0=0\n");
			fprintf(fp, "AccessPolicy1=0\n");
			fprintf(fp, "AccessPolicy2=0\n");
			fprintf(fp, "AccessPolicy3=0\n");
		}
	}
	else {
		warning = 41;
		fprintf(fp, "AccessPolicy0=0\n");
		fprintf(fp, "AccessPolicy1=0\n");
		fprintf(fp, "AccessPolicy2=0\n");
		fprintf(fp, "AccessPolicy3=0\n");
	}

	list[0]=0;
	list[1]=0;
	if (nvram_invmatch(strcat_r(prefix, "macmode", tmp), "disabled")) {
		strcpy(list, nvram_safe_get(strcat_r(prefix, "maclist_x", tmp)));
		if (list[0]=='<') {
			char *pt=&list[1];
			while (pt=strchr(pt,'<'))
				*pt++=';';
		}
	}
	// AccessControlLis0~3
	fprintf(fp, "AccessControlList0=%s\n", list+1);
	fprintf(fp, "AccessControlList1=%s\n", list+1);
	fprintf(fp, "AccessControlList2=%s\n", list+1);
	fprintf(fp, "AccessControlList3=%s\n", list+1);

	fprintf(fp, "AccessPolicy4=0\n");
	fprintf(fp, "AccessPolicy5=0\n");
	fprintf(fp, "AccessPolicy6=0\n");
	fprintf(fp, "AccessPolicy7=0\n");

	fprintf(fp, "AccessControlList4=\n");	
	fprintf(fp, "AccessControlList5=\n");
	fprintf(fp, "AccessControlList6=\n");
	fprintf(fp, "AccessControlList7=\n");

	if (_sw_mode != 2 
#ifdef SWMODE_HOTSPOT_SUPPORT
			&& _sw_mode != 5 
#endif
			&& !nvram_match(strcat_r(prefix, "mode_x", tmp), "0")) {
		//WdsEnable
		str = nvram_safe_get(strcat_r(prefix, "mode_x", tmp));
		if (str) {
			if (!strcmp(wlX_auth_mode, "open") 
					|| (!strcmp(wlX_auth_mode, "psk") 
						&& nvram_match(strcat_r(prefix, "wpa_mode", tmp), "2") 
						&& !strcmp(wlX_crypto, "aes"))) {
				if (atoi(str) == 0)
					fprintf(fp, "WdsEnable=0\n");
				else if (atoi(str) == 1)
					fprintf(fp, "WdsEnable=2\n");
				else if (atoi(str) == 2) {
					if (nvram_match(strcat_r(prefix, "wdsapply_x", tmp), "0"))
						fprintf(fp, "WdsEnable=4\n");
					else
						fprintf(fp, "WdsEnable=3\n");
				}
			}
			else
				fprintf(fp, "WdsEnable=0\n");
		}
		else {
			warning = 42;
			fprintf(fp, "WdsEnable=0\n");
		}

		//WdsEncrypType
		if (!strcmp(wlX_auth_mode, "open") 
				&& nvram_match(strcat_r(prefix, "wep_x", tmp), "0"))
			fprintf(fp, "WdsEncrypType=NONE;NONE;NONE;NONE\n");
		else if (!strcmp(wlX_auth_mode, "open") 
				&& nvram_invmatch(strcat_r(prefix, "wep_x", tmp), "0"))
			fprintf(fp, "WdsEncrypType=WEP;WEP;WEP;WEP\n");
		else if (!strcmp(wlX_auth_mode, "psk") 
				&& nvram_match(strcat_r(prefix, "wpa_mode", tmp), "2") 
				&& !strcmp(wlX_crypto, "aes"))
			fprintf(fp, "WdsEncrypType=AES;AES;AES;AES\n");
		else
			fprintf(fp, "WdsEncrypType=NONE;NONE;NONE;NONE\n");

		//WdsPhyMode
		str = nvram_safe_get(strcat_r(prefix, "mode_x", tmp));
		if (str) {
			if ((atoi(nvram_safe_get(strcat_r(prefix, "nmode_x", tmp))) == 0))
				fprintf(fp, "WdsPhyMode=HTMIX\n");
			else if ((atoi(nvram_safe_get(strcat_r(prefix, "nmode_x", tmp))) == 1))
				fprintf(fp, "WdsPhyMode=GREENFIELD\n");
			else
				fprintf(fp, "WdsPhyMode=\n");
		}

		//WdsList
		char macbuf[36];
		int num;
		list[0] = 0;
		list[1] = 0;
		if ((nvram_match(strcat_r(prefix, "mode_x", tmp), "1") 
					|| (nvram_match(strcat_r(prefix, "mode_x", tmp), "2") 
						&& nvram_match(strcat_r(prefix, "wdsapply_x", tmp), "1"))) 
				&& (!strcmp(wlX_auth_mode, "open") 
					|| (!strcmp(wlX_auth_mode, "psk") 
						&& nvram_match(strcat_r(prefix, "wpa_mode", tmp), "2") 
						&& !strcmp(wlX_crypto, "aes")))) {
			num = atoi(nvram_safe_get(strcat_r(prefix, "wdsnum_x", tmp)));
			for (i=0 ; i<num ; i++)
				sprintf(list, "%s;%s", list, mac_conv(strcat_r(prefix, "wdslist_x", tmp), i, macbuf));
		}

		fprintf(fp, "WdsList=%s\n", list+1);

		//WdsKey
		if (!strcmp(wlX_auth_mode, "open") 
				&& nvram_match(strcat_r(prefix, "wep_x", tmp), "0")) {
			fprintf(fp, "WdsDefaultKeyID=\n");
			fprintf(fp, "Wds0Key=\n");
			fprintf(fp, "Wds1Key=\n");
			fprintf(fp, "Wds2Key=\n");
			fprintf(fp, "Wds3Key=\n");
		}
		else if (!strcmp(wlX_auth_mode, "open") 
				&& nvram_invmatch(strcat_r(prefix, "wep_x", tmp), "0")) {
			str = nvram_safe_get(strcat_r(prefix, "key", tmp));
			fprintf(fp, "WdsDefaultKeyID=%s;%s;%s;%s\n", str, str, str, str);
			sprintf(list, "wl_key%s", str);
			fprintf(fp, "Wds0Key=%s\n", nvram_safe_get(list));
			fprintf(fp, "Wds1Key=%s\n", nvram_safe_get(list));
			fprintf(fp, "Wds2Key=%s\n", nvram_safe_get(list));
			fprintf(fp, "Wds3Key=%s\n", nvram_safe_get(list));
		}
		else if (!strcmp(wlX_auth_mode, "psk") 
				&& nvram_match(strcat_r(prefix, "wpa_mode", tmp), "2") 
				&& !strcmp(wlX_crypto, "aes")) {
			str = nvram_safe_get(strcat_r(prefix, "wpa_psk", tmp));
			fprintf(fp, "WdsKey=%s\n", str);
			fprintf(fp, "Wds0Key=%s\n",str);
			fprintf(fp, "Wds1Key=%s\n",str);
			fprintf(fp, "Wds2Key=%s\n",str);
			fprintf(fp, "Wds3Key=%s\n",str);
		}
	}

	//RADIUS_Server
	if (!strcmp(nvram_safe_get(strcat_r(prefix, "radius_ipaddr", tmp)), ""))
		fprintf(fp, "RADIUS_Server=0;0;0;0;0;0;0;0\n");
	else
		fprintf(fp, "RADIUS_Server=%s;0;0;0;0;0;0;0\n", nvram_safe_get(strcat_r(prefix, "radius_ipaddr", tmp)));

	//RADIUS_Port
	str = nvram_safe_get(strcat_r(prefix, "radius_port", tmp));
	if (str)
		fprintf(fp, "RADIUS_Port=%s;%s;%s;%s;%s;%s;%s;%s\n", str, str, str, str, str, str, str, str);
	else {
		warning = 43;
		fprintf(fp, "RADIUS_Port=1812;1812;1812;1812;1812;1812;1812;1812\n");
	}

	//RADIUS_Key
	str = nvram_safe_get(strcat_r(prefix, "radius_key", tmp));
	fprintf(fp, "RADIUS_Key=%s;%s;%s;%s;%s;%s;%s;%s\n", str, str, str, str, str, str, str, str);

	//RADIUS_Acct_Server
	fprintf(fp, "RADIUS_Acct_Server=%s\n", nvram_safe_get("RADIUS_Acct_Server"));

	//RADIUS_Acct_Port
	str = nvram_safe_get("RADIUS_Acct_Port");
	if (str)
		fprintf(fp, "RADIUS_Acct_Port=%s\n", str);
	else {
		warning = 44;
		fprintf(fp, "RADIUS_Acct_Port=1813\n");
	}

	//RADIUS_Acct_Key
	fprintf(fp, "RADIUS_Acct_Key=%s\n", nvram_safe_get("RADIUS_Acct_Key"));

	//own_ip_addr
	if (flag_8021x==1)
		fprintf(fp, "own_ip_addr=%s\n", nvram_safe_get("lan_ipaddr"));
	else
		fprintf(fp, "own_ip_addr=\n");

	//Ethifname
	fprintf(fp, "Ethifname=\n");

	//EAPifname
	if (flag_8021x==1)
		fprintf(fp, "EAPifname=%s\n", nvram_safe_get("EAPifname"));
	else
		fprintf(fp, "EAPifname=\n");

	//PreAuthifname
	if (flag_8021x==1)
		fprintf(fp, "PreAuthifname=%s\n", nvram_safe_get("PreAuthifname"));
	else
		fprintf(fp, "PreAuthifname=\n");

	//session_timeout_interval
	str = nvram_safe_get("session_timeout_interval");
	if (str)
		fprintf(fp, "session_timeout_interval=%s\n", str);
	else {
		warning = 45;
		fprintf(fp, "session_timeout_interval=0\n");
	}

	//idle_timeout_interval
	str = nvram_safe_get("idle_timeout_interval");
	if (str)
		fprintf(fp, "idle_timeout_interval=%s\n", str);
	else {
		warning = 46;
		fprintf(fp, "idle_timeout_interval=0\n");
	}

	fprintf(fp, "SSID=\n");
	fprintf(fp, "WPAPSK=\n");
	fprintf(fp, "Key1Str=\n");
	fprintf(fp, "Key2Str=\n");
	fprintf(fp, "Key3Str=\n");
	fprintf(fp, "Key4Str=\n");

	if ((_sw_mode == 2 
#ifdef SWMODE_HOTSPOT_SUPPORT
				|| _sw_mode == 5 
#endif
			) && strcmp(staX_ssid, "") != 0) {
		int flag_wep;

		fprintf(fp, "ApCliEnable=0\n");
#if (defined(CONFIG_DEFAULTS_RALINK_MT7620) 		\
	|| defined(CONFIG_DEFAULTS_RALINK_MT7621) 	\
	|| defined(CONFIG_DEFAULTS_RALINK_MT7628))
#if (defined(CONFIG_DEFAULTS_ASUS_RPN12) 		\
	|| defined(CONFIG_DEFAULTS_ASUS_RPN14) 		\
	|| defined(CONFIG_DEFAULTS_ASUS_RPAC52) 	\
	|| defined(CONFIG_DEFAULTS_ASUS_RPAC56) 	\
	|| defined(CONFIG_DEFAULTS_ASUS_WMPN12))
		if (_sw_mode == 2 && nvram_match("re_wifipxy", "1"))
#elif defined(CONFIG_DEFAULTS_ASUS_RPN53)
		if (_sw_mode == 2 && nvram_match("re_wifipxy", "1") && band==0)
#else
#error Invalid Model
#endif
			fprintf(fp, "MACRepeaterEn=1\n");
		else
			fprintf(fp, "MACRepeaterEn=0\n");
#endif
		fprintf(fp, "ApCliSsid=%s\n", staX_ssid);
		fprintf(fp, "ApCliBssid=\n");

		str = nvram_safe_get(strcat_r(sta_prefix, "auth_mode", tmp));
		if (str) {
			if (!strcmp(str, "open")) {
				if (_sw_mode == 2) {
					fprintf(fp, "ApCliAuthMode=OPEN\n");

					if (nvram_match(strcat_r(sta_prefix, "wep_x", tmp), "0"))
						fprintf(fp, "ApCliEncrypType=NONE\n");
					else {
						flag_wep = 1;
						fprintf(fp, "ApCliEncrypType=WEP\n");
					}
				}
				else {
					if (nvram_match(strcat_r(sta_prefix, "wep_x", tmp), "0")) {
						fprintf(fp, "ApCliAuthMode=OPEN\n");
						fprintf(fp, "ApCliEncrypType=NONE\n");
					}
					else {
						flag_wep = 1;
						fprintf(fp, "ApCliAuthMode=WEPAUTO\n");
						fprintf(fp, "ApCliEncrypType=WEP\n");
					}
				}
			}
			else if (!strcmp(str, "shared")) {
				flag_wep = 1;
				if (_sw_mode == 2) {
					fprintf(fp, "ApCliAuthMode=SHARED\n");
					fprintf(fp, "ApCliEncrypType=WEP\n");
				}
				else {
					fprintf(fp, "ApCliAuthMode=WEPAUTO\n");
					fprintf(fp, "ApCliEncrypType=WEP\n");
				}
			}
			else if (!strcmp(str, "psk")) {
				if (nvram_match(strcat_r(sta_prefix, "wpa_mode", tmp), "1"))
					fprintf(fp, "ApCliAuthMode=WPAPSK\n");
				else if (nvram_match(strcat_r(sta_prefix, "wpa_mode", tmp), "2"))
					fprintf(fp, "ApCliAuthMode=WPA2PSK\n");

				//EncrypType
				if (nvram_match(strcat_r(sta_prefix, "crypto", tmp), "tkip"))
					fprintf(fp, "ApCliEncrypType=TKIP\n");
				else if (nvram_match(strcat_r(sta_prefix, "crypto", tmp), "aes"))
					fprintf(fp, "ApCliEncrypType=AES\n");

				//WPAPSK
				fprintf(fp, "ApCliWPAPSK=%s\n", nvram_safe_get(strcat_r(sta_prefix, "wpa_psk", tmp)));
			}
			else {
				fprintf(fp, "ApCliAuthMode=OPEN\n");
				fprintf(fp, "ApCliEncrypType=NONE\n");
			}
		}
		else {
			fprintf(fp, "ApCliAuthMode=OPEN\n");
			fprintf(fp, "ApCliEncrypType=NONE\n");
		}

		//EncrypType
		if (flag_wep) {
			//DefaultKeyID
			fprintf(fp, "ApCliDefaultKeyID=%s\n", nvram_safe_get(strcat_r(sta_prefix, "key", tmp)));

			//KeyType (0 -> Hex, 1->Ascii)
			fprintf(fp, "ApCliKey1Type=%s\n", nvram_safe_get(strcat_r(sta_prefix, "key_type", tmp)));
			fprintf(fp, "ApCliKey2Type=%s\n", nvram_safe_get(strcat_r(sta_prefix, "key_type", tmp)));
			fprintf(fp, "ApCliKey3Type=%s\n", nvram_safe_get(strcat_r(sta_prefix, "key_type", tmp)));
			fprintf(fp, "ApCliKey4Type=%s\n", nvram_safe_get(strcat_r(sta_prefix, "key_type", tmp)));

			//KeyStr
			fprintf(fp, "ApCliKey1Str=%s\n", nvram_safe_get(strcat_r(sta_prefix, "key1", tmp)));
			fprintf(fp, "ApCliKey2Str=%s\n", nvram_safe_get(strcat_r(sta_prefix, "key2", tmp)));
			fprintf(fp, "ApCliKey3Str=%s\n", nvram_safe_get(strcat_r(sta_prefix, "key3", tmp)));
			fprintf(fp, "ApCliKey4Str=%s\n", nvram_safe_get(strcat_r(sta_prefix, "key4", tmp)));
		}
		else {
			fprintf(fp, "ApCliDefaultKeyID=0\n");
			fprintf(fp, "ApCliKey1Type=0\n");
			fprintf(fp, "ApCliKey1Str=\n");
			fprintf(fp, "ApCliKey2Type=0\n");
			fprintf(fp, "ApCliKey2Str=\n");
			fprintf(fp, "ApCliKey3Type=0\n");
			fprintf(fp, "ApCliKey3Str=\n");
			fprintf(fp, "ApCliKey4Type=0\n");
			fprintf(fp, "ApCliKey4Str=\n");
		}
	}
	else {
		fprintf(fp, "ApCliEnable=0\n");
		fprintf(fp, "MACRepeaterEn=0\n");
		fprintf(fp, "ApCliSsid=\n");
		fprintf(fp, "ApCliBssid=\n");
		fprintf(fp, "ApCliAuthMode=\n");
		fprintf(fp, "ApCliEncrypType=\n");
		fprintf(fp, "ApCliWPAPSK=\n");
		fprintf(fp, "ApCliDefaultKeyID=0\n");
		fprintf(fp, "ApCliKey1Type=0\n");
		fprintf(fp, "ApCliKey1Str=\n");
		fprintf(fp, "ApCliKey2Type=0\n");
		fprintf(fp, "ApCliKey2Str=\n");
		fprintf(fp, "ApCliKey3Type=0\n");
		fprintf(fp, "ApCliKey3Str=\n");
		fprintf(fp, "ApCliKey4Type=0\n");
		fprintf(fp, "ApCliKey4Str=\n");
	}

	//CarrierDetect
	if (!strcmp(reg_spec, "JP"))
		fprintf(fp, "CarrierDetect=1\n");
	else
		fprintf(fp, "CarrierDetect=0\n");

	//IgmpSnEnable
	//McastPhyMode, PHY mode for Multicast frames
	//McastMcs, MCS for Multicast frames, ranges from 0 to 7
	/*	MODE=1, MCS=0: Legacy CCK 1Mbps
	 *	MODE=1, MCS=1: Legacy CCK 2Mbps
	 *	MODE=1, MCS=2: Legacy CCK 5.5Mbps
	 *	MODE=1, MCS=3: Legacy CCK 11Mbps
	 *	MODE=2, MCS=0: Legacy OFDM 6Mbps
	 *	MODE=2, MCS=1: Legacy OFDM 9Mbps
	 *	MODE=2, MCS=2: Legacy OFDM 12Mbps
	 *	MODE=2, MCS=3: Legacy OFDM 18Mbps
	 *	MODE=2, MCS=4: Legacy OFDM 24Mbps
	 * 	MODE=2, MCS=5: Legacy OFDM 36Mbps
	 *	MODE=2, MCS=6: Legacy OFDM 48Mbps
	 *	MODE=2, MCS=7: Legacy OFDM 54Mbps
	 *	MODE=3, MCS=15: HTMIX 130/144Mbps
	 **/
	str = nvram_safe_get(strcat_r(prefix, "mrate", tmp));
	if (str) {
		if (atoi(str)==0)	// Disable => Auto
			fprintf(fp, "IgmpSnEnable=0\n");
		else {			// Disable => Auto
			fprintf(fp, "IgmpSnEnable=1\n");
			fprintf(fp, "McastPhyMode=3\n");
			fprintf(fp, "McastMcs=15\n");
		}
	}
	else {
		warning = 47;
	}

	fprintf(fp, "BeaconLostTime=4\n");

#ifdef WPS_SUPPORT
	fprintf(fp, "WscManufacturer=ASUSTeK Computer Inc.\n");
	fprintf(fp, "WscModelName=Wireless Range Extender\n");
	fprintf(fp, "WscDeviceName=ASUS Wireless Range Extender\n");
	fprintf(fp, "WscModelNumber=%s\n", nvram_safe_get("fac_model_name"));
	fprintf(fp, "WscSerialNumber=%s\n", nvram_safe_get(strcat_r(prefix, "macaddr", tmp)));
#endif

	if (warning) {
		printf("%d!!!!\n", warning);
		printf("Miss some configuration, please check!!!!\n");
	}

	fclose(fp);

	return 0;
}

int gen_multiple_card_info()
{
	FILE *fp;

	if(!(fp=fopen("/etc/Wireless/RT2860APCard.dat", "w+")))
		return 0;

	fprintf(fp, "#The word of \"Default\" must not be removed, maximum 32 cards, 00 ~ 31\n");
	fprintf(fp, "Default\n");
	fprintf(fp, "\n");
	fprintf(fp, "SELECT=CARDID\n");
	fprintf(fp, "\n");
	fprintf(fp, "00CARDID=/etc/Wireless/RT2860/RT2860.dat\n");
	fprintf(fp, "01CARDID=/etc/Wireless/iNIC/iNIC_ap.dat\n");
	fclose(fp);

	return 0;
}

/*
 * wireless radio on/off
 */
int wireless_radio(int n, char *wif, int ctrl)
{
	doSystem("nvram set sw%d_radio=%d", n, ctrl);
	if (!ctrl) {
		ap_set(wif, "RadioOn=0");
	}
	else {
		ap_set(wif, "RadioOn=1");
	}
}

#ifdef WPA_SUPPLICANT_SUPPORT
int OidSetInformation(unsigned long OidQueryCode, int socket_id, char *DeviceName, void *ptr, unsigned long PtrLength)
{
	struct iwreq wrq;

	strcpy(wrq.ifr_name, DeviceName);
	wrq.u.data.length = PtrLength;
	wrq.u.data.pointer = (caddr_t) ptr;
	wrq.u.data.flags = OidQueryCode | OID_GET_SET_TOGGLE;

	return (ioctl(socket_id, RT_PRIV_IOCTL, &wrq));
}
#endif

/*
 * get current wireless channel
 */
int get_channel(char *wif)
{
	int channel;
	struct iw_range	range;
	double freq;
	struct iwreq wrq1;
	struct iwreq wrq2;

	if (wl_ioctl(wif, SIOCGIWFREQ, &wrq1) < 0)
		return 0;

	char buffer[sizeof(iwrange) * 2];
	bzero(buffer, sizeof(buffer));
	wrq2.u.data.pointer = (caddr_t) buffer;
	wrq2.u.data.length = sizeof(buffer);
	wrq2.u.data.flags = 0;

	if (wl_ioctl(wif, SIOCGIWRANGE, &wrq2) < 0)
		return 0;

	if (ralink_get_range_info(wif, &range, buffer, wrq2.u.data.length) < 0)
		return 0;

	freq = iw_freq2float(&(wrq1.u.freq));
	if (freq < KILO)
		channel = (int) freq;
	else {
		channel = iw_freq_to_channel(freq, &range);
		if (channel < 0)
			return 0;
	}

	return channel;
}

/*
 * site survey for apcli_monitor
 */
int site_survey_for_channel(int n, const char *wif, int *HT_EXT)
{
	char tmp[128], prefix[] = "staXXXXXXXXXX_";
	snprintf(prefix, sizeof(prefix), "sta%d_", n);

	int i = 0, apCount = 0;
	char data[8192];
	struct iwreq wrq;
	SSA *ssap;
	char *ssid = nvram_safe_get(strcat_r(prefix, "ssid", tmp));
	int channellistnum, commonchannel, centralchannel, ht_extcha = 1;
	int wep = 0;

	if (nvram_invmatch(strcat_r(prefix, "wep_x", tmp), "0") 
			|| nvram_match(strcat_r(prefix, "auth_mode", tmp), "psk"))
		wep = 1;

	*HT_EXT = -1;

	memset(data, 0x00, 255);
	strcpy(data, "SiteSurvey=1");
	wrq.u.data.length = strlen(data) + 1;
	wrq.u.data.pointer = data;
	wrq.u.data.flags = 0;

	spinlock_lock(SPINLOCK_SiteSurvey);
	if (wl_ioctl(wif, RTPRIV_IOCTL_SET, &wrq) < 0) {
		spinlock_unlock(0);

		fprintf(stderr, "Site Survey fails\n");
		return -1;
	}
	spinlock_unlock(SPINLOCK_SiteSurvey);

	fprintf(stderr, "Look for SSID: %s\n", ssid);
	fprintf(stderr, "Please wait");
	sleep(1);
	fprintf(stderr, ".");
	sleep(1);
	fprintf(stderr, ".");
	sleep(1);
	fprintf(stderr, ".");
	sleep(1);
	fprintf(stderr, ".\n");

	memset(data, 0x0, 8192);
	strcpy(data, "");
	wrq.u.data.length = 8192;
	wrq.u.data.pointer = data;
	wrq.u.data.flags = 0;

	if (wl_ioctl(wif, RTPRIV_IOCTL_GSITESURVEY, &wrq) < 0) {
		fprintf(stderr, "errors in getting site survey result\n");
		return -1;
	}

	fprintf(stderr, "\n%-4s%-33s%-18s%-9s%-16s%-9s\n%-8s%-3s%-4s%-5s%-5s%-3s\n", "Ch", "SSID", "BSSID", "Enc", "Auth", "Siganl(%)", "W-Mode", "NT", "CC", " WPS", " DPID", " EC");

	if (wrq.u.data.length > 0) {
		char commch[4];
		char cench[4];
		int signal_max = -1, signal_tmp = -1, ht_extcha_max, idx = -1;
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

		if (apCount) {
			for (i = 0; i < apCount; i++) {
				memset(commch,0,sizeof(commch));
				memcpy(commch,ssap->SiteSurvey[i].channel,3);
				commonchannel=atoi(commch);
				memset(cench,0,sizeof(cench));
				memcpy(cench,ssap->SiteSurvey[i].centralchannel,3);
				centralchannel = atoi(cench);
				if (strstr(ssap->SiteSurvey[i].bsstype, "n") 
						&& (commonchannel != centralchannel)) {
					if (n) {
						if (centralchannel < commonchannel)
							ht_extcha = 0;
						else
							ht_extcha = 1;
					}
					else {
						if (commonchannel <= 4)
							ht_extcha = 1;
						else if (commonchannel > 4 && commonchannel < 8) {
							if (centralchannel < commonchannel)
								ht_extcha = 0;
							else
								ht_extcha = 1;
						}
						else if (commonchannel >= 8) {
							char *value = nvram_safe_get("wl0_reg");

							if (!strcmp(value,"2G_CH11"))
								channellistnum = 11;
							else if (!strcmp(value,"2G_CH14"))
								channellistnum = 14;
							else	// 2G_CH13
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
				}
				else
					ht_extcha = -1;

				fprintf(stderr, "%-4s%-33s%-18s%-9s%-16s%-9s\n%-8s%-3s%-4s%-5s%-5s%d\n",
						ssap->SiteSurvey[i].channel,
						ssap->SiteSurvey[i].ssid,
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

				/* non-hidden AP */
				if (ssid && !strcmp(ssid, trim_r(ssap->SiteSurvey[i].ssid)) 
						&& ((!strncmp(ssap->SiteSurvey[i].encryption, "NONE", 4) && wep == 0) 
							|| (strncmp(ssap->SiteSurvey[i].encryption, "NONE", 4) && wep == 1))) {
					if (!strncmp(ssap->SiteSurvey[i].bssid, nvram_safe_get(strcat_r(prefix, "bssid", tmp)), 17)) {
						*HT_EXT = ht_extcha;
						return commonchannel;
					}
					else if ((signal_tmp = atoi(trim_r(ssap->SiteSurvey[i].signal))) > signal_max) {
						signal_max = signal_tmp;
						ht_extcha_max = ht_extcha;
						idx = commonchannel;
					}
				}
				/* hidden AP */
				else if (!strlen(trim_r(ssap->SiteSurvey[i].ssid)) 
						&& ((!strncmp(ssap->SiteSurvey[i].encryption, "NONE", 4) && wep == 0) 
							|| (strncmp(ssap->SiteSurvey[i].encryption, "NONE", 4) && wep == 1))) {
					if (!strncmp(ssap->SiteSurvey[i].bssid, nvram_safe_get(strcat_r(prefix, "bssid", tmp)), 17)) {
						*HT_EXT = ht_extcha;
						return commonchannel;
					}
				}
			}
			fprintf(stderr, "\n");
		}

		if (idx != -1) {
			*HT_EXT = ht_extcha_max;
			return idx;
		}
	}

	return -1;
}

/*
 * initial apcli_monitor parameter
 */
int init_apcli_monitor_para(int n, char *aif)
{
	char tmp[128], prefix[] = "staXXXXXXXXXX_";
	char buf[32];

	snprintf(prefix, sizeof(prefix), "sta%d_", n);

	nvram_set(strcat_r(prefix, "connStatus", tmp), "0");

	doSystem("echo 1 > /proc/sys/net/ipv4/conf/%s/arp_ignore", aif);
	doSystem("echo 2 > /proc/sys/net/ipv4/conf/%s/arp_announce", aif);

	sprintf(buf, "ApcliMonitorPid=%d", get_process_pid(APCLI_MONITOR_PIDFILE));
	ap_set(aif, buf);
	if (nvram_invmatch(strcat_r(prefix, "ssid", tmp), ""))
		ap_set(aif, "ApCliEnable=1");
}


#ifdef WPS_SUPPORT
void wps_oob(void)
{
	int n;
	char wif[8], *next;
	char tmp[128], prefix[] = "wlXXXXXXX_";

	nvram_set("wsc_config_state", "0");
	for1each(n, wif, nvram_safe_get("wl_ifnames"), next) {
		snprintf(prefix, sizeof(prefix), "wl%d_", n);
#if defined(CONFIG_DEFAULTS_ASUS_RPN53)
		if (nvram_match("fac_model_name", "RP-N54"))
			nvram_set(strcat_r(prefix, "ssid", tmp), n==0?"ASUS_RPN54":"ASUS_RPN54_5G");
		else
#endif
		nvram_set(strcat_r(prefix, "ssid", tmp), n==0?OOB_SSID_2G:OOB_SSID_5G);
		nvram_set(strcat_r(prefix, "auth_mode", tmp), "open");
		nvram_set(strcat_r(prefix, "wep_x", tmp), "0");
		nvram_set(strcat_r(prefix, "wpa_mode", tmp), "2");
		nvram_set(strcat_r(prefix, "crypto", tmp), "aes");
		nvram_set(strcat_r(prefix, "key", tmp), "1");
		nvram_set(strcat_r(prefix, "phrase_x", tmp), "");
		nvram_set(strcat_r(prefix, "key1", tmp), "");
		nvram_set(strcat_r(prefix, "key2", tmp), "");
		nvram_set(strcat_r(prefix, "key3", tmp), "");
		nvram_set(strcat_r(prefix, "key4", tmp), "");
		nvram_set(strcat_r(prefix, "wpa_psk", tmp), "12345678");

		ap_set(wif, "AuthMode=OPEN");
		ap_set(wif, "EncrypType=NONE");
		ap_set(wif, "IEEE8021X=0");
		ap_set(wif, "DefaultKeyID=1");
		doSystem("iwpriv %s set SSID=%s", wif, nvram_safe_get(strcat_r(prefix, "ssid", tmp)));
		ap_set(wif, "WscConfStatus=1");	// AP is unconfigured
	}
	nvram_commit();
}

void ui_prestart_wsc(void)
{
	int n;
	char wif[8], *next;

	fprintf(stderr, "WPS: UI pre-start\n");

	for1each(n, wif, nvram_safe_get("wl_ifnames"), next) {
		ap_set(wif, "WscConfMode=0");	// WPS disabled
		ap_set(wif, "WscConfMode=7");	// Enrollee + Proxy + Registrar
	}
}

#ifdef DUAL_BAND_NONCONCURRENT
extern int wps_freq;
#endif

void start_wsc(void)
{
	int _sw_mode = atoi(nvram_safe_get("sw_mode"));
	int n;
	char wif[8], *wif_next, aif[8], *aif_next;

	fprintf(stderr, "WPS: start\n");
	nvram_set("wps_running", "1");
#ifdef LED_DRV_SUPPORT
	nvram_set("led_drv_stage", "5");
#endif

	if (_sw_mode == 2 && nvram_invmatch("rep_wps_be_ap", "1")) {
#ifdef SWMODE_REPEATER_EXPRESSWAY_SUPPORT
		int re_expressway = atoi(nvram_safe_get("re_expressway"));
#endif
#ifdef SWMODE_REPEATER_MEDIABRIDGE_SUPPORT
		int re_mediabridge = atoi(nvram_safe_get("re_mediabridge"));
#endif
		char buf[32];
		unsigned char ea[ETHER_ADDR_LEN];

		stop_apcli_monitor();

		for2each(n, wif, nvram_safe_get("wl_ifnames"), wif_next,
				aif, nvram_safe_get("sta_ifnames"), aif_next) {
#ifdef SWMODE_REPEATER_EXPRESSWAY_SUPPORT
			if (IsExpressWayCli())
				ifconfig(aif, IFUP, NULL, NULL);
			else if (IsExpressWayApcli())
				ifconfig(wif, IFUP, NULL, NULL);
			else
#endif
#ifdef SWMODE_REPEATER_MEDIABRIDGE_SUPPORT
			if (IsMediaBridge())
				ifconfig(wif, IFUP, NULL, NULL);
			else
#endif
				;

#ifdef DUAL_BAND_NONCONCURRENT
			change_WirelessMode(wps_freq, wif);
			ap_set(wif, "RadioOn=0");
			usleep(20000);
			ap_set(wif, "RadioOn=1");
#else
			change_WirelessMode(n, wif);
#endif
			eval("brctl", "delif", "br0", aif);
			ap_set(aif, "ApCliEnable=0");
			ap_set(wif, "MACRepeaterEn=0");
			doSystem("rm -f /var/run/re_wpsc%d.pid", n);
			doSystem("rm -f /tmp/wsc.%s", aif);
			doSystem("re_wpsc %s %s %d", wif, aif, get_channel(wif));
		}

		strncpy(buf, nvram_safe_get("wl0_macaddr"), sizeof(buf));
		eval("ifconfig", "br0", "hw", "ether", buf);
		if (ether_atoe(buf, ea))
			track_set(ASUS_HIJ_SET_DUT_MAC, ea);
	}
#ifdef SWMODE_ADAPTER_SUPPORT
	else if (_sw_mode == 4) {
		for1each(n, wif, nvram_safe_get("wl_ifnames"), wif_next) {
#ifdef DUAL_BAND_NONCONCURRENT
			change_WirelessMode(wps_freq, wif);
			ap_set(wif, "RadioOn=0");
			usleep(20000);
			ap_set(wif, "RadioOn=1");
#endif
			eval("iwpriv", wif, "wsc_conf_mode", "1");
			eval("iwpriv", wif, "wsc_mode", "2");
			eval("iwpriv", wif, "wsc_start");
		}
	}
#endif
	else {
		for1each(n, wif, nvram_safe_get("wl_ifnames"), wif_next) {
			ap_set(wif, "WscConfMode=7");		// Enrollee + Proxy + Registrar

			if (nvram_match("wps_mode", "1")) {
				char pincode[32];

				if (nvram_match("wps_pin_web", ""))
					sprintf(pincode, "WscPinCode=0");
				else
					sprintf(pincode, "WscPinCode=%s", nvram_safe_get("wps_pin_web"));
				ap_set(wif, pincode);
				ap_set(wif, "WscMode=1");	// PIN method
			}
			else
				ap_set(wif, "WscMode=2");	// PBC method

			ap_set(wif, "WscGetConf=1");		// Trigger WPS AP to do simple config with WPS Client
		}
#ifdef DUAL_BAND_NONCONCURRENT
		wps_freq = 1;
#endif
	}
}

void stop_wsc(void)
{
	int _sw_mode = atoi(nvram_safe_get("sw_mode"));
	int n;
	char wif[8], *next;

	nvram_set("wps_running", "0");

	if (_sw_mode == 2 && nvram_invmatch("rep_wps_be_ap", "1")) {
		eval_dumb("killall", "re_wpsc");
	}
#ifdef SWMODE_ADAPTER_SUPPORT
	else if (_sw_mode == 4) {
		for1each(n, wif, nvram_safe_get("wl_ifnames"), next) {
			eval("iwpriv", wif, "wsc_conf_mode", "0");
			eval("iwpriv", wif, "wsc_stop");
		}
	}
#endif
	else {
		for1each(n, wif, nvram_safe_get("wl_ifnames"), next)
			ap_set(wif, "WscConfMode=0");
	}
}

int getWscStatusCli(char *aif)
{
	char fname[32];
	char *fp;
	int fsize;

	sprintf(fname, "/tmp/wsc.%s", aif);
	fp = readfile(fname, &fsize);
	if (fp) {
		int wsc_code;

		wsc_code = atoi(fp);
		free(fp);
		//printf("[%s]: WscCode is %d\n", aif, wsc_code);

		return wsc_code;
	}

	return 0;
}

int getWscStatus(char *wif)
{
	int socket_id;
	int data = 0;
	struct iwreq wrq;

	wrq.u.data.length = sizeof(data);
	wrq.u.data.pointer = (caddr_t) &data;
	wrq.u.data.flags = RT_OID_WSC_QUERY_STATUS;

	if (wl_ioctl(wif, RT_PRIV_IOCTL, &wrq) < 0)
		fprintf(stderr, "errors in getting WSC status\n");

	return data;
}

#define WPS_PAP_PROFILE_PATH "/tmp/iwpriv.stat"

int RetrieveWscPapProfile(int n, char *wif)
/*
 * format sample:
 * SSID                            = ASUS-Vic1
 * MAC                             = F4:6D:04:DB:4E:CE
 * AuthType                        = OPEN
 * EncrypType                      = WEP
 * KeyIndex                        = 1
 * Key                             = 6162636465
 */
{
	char *fp;
	int fsize, ret = 0;
	char tmp[128], prefix[] = "staXXXXXXXXXX_";
#ifndef SWMODE_REPEATER_V2_SUPPORT
	int wps_first_success = 0;

	if (check_if_file_exist("/var/run/re_wpsc0.pid") 
			|| check_if_file_exist("/var/run/re_wpsc1.pid") 
			|| strstr(nvram_safe_get("www_support"), "5G") == NULL)
		wps_first_success = 1;
#endif

	snprintf(prefix, sizeof(prefix),"sta%d_", n);
	doSystem("iwpriv %s stat > %s", wif, WPS_PAP_PROFILE_PATH);
	sleep(2);

	fp = readfile(WPS_PAP_PROFILE_PATH, &fsize);
	if (fp) {
		char *pt1, *pt2, buf[128];
		int is_nokey = 0;
		int is_psk = 0;
		int key_idx;

		pt1 = strstr(fp, "Profile[0]:");
		if (pt1) {
			//SSID
			pt2 = pt1 + 11;
			pt1 = strstr(pt2, "= ");
			pt1 += 2;
			pt2 = strstr(pt1, "\n");
			*pt2 = '\0';
			nvram_set(strcat_r(prefix, "ssid", tmp), pt1);
#ifndef SWMODE_REPEATER_V2_SUPPORT
			if (wps_first_success) {
				sprintf(buf, "%s_RPT", pt1);
				nvram_set("wl0_ssid", buf);
				sprintf(buf, "%s_RPT5G", pt1);
				nvram_set("wl1_ssid", buf);
			}
#endif

			//AuthType
			pt2++;
			pt1 = strstr(pt2, "AuthType");
			pt2 = pt1 + 8;
			pt1 = strstr(pt2, "= ");
			pt1 += 2;
			pt2 = strstr(pt1, "\n");
			*pt2 = '\0';
			if (!strcmp(pt1, "OPEN")) {
				nvram_set(strcat_r(prefix, "auth_mode", tmp), "open");
#ifndef SWMODE_REPEATER_V2_SUPPORT
				if (wps_first_success) {
					nvram_set("wl0_auth_mode", "open");
					nvram_set("wl1_auth_mode", "open");
				}
#endif
			}
			else if (!strcmp(pt1, "WPAPSK")) {
				nvram_set(strcat_r(prefix, "auth_mode", tmp), "psk");
				nvram_set(strcat_r(prefix, "wpa_mode", tmp), "1");
#ifndef SWMODE_REPEATER_V2_SUPPORT
				if (wps_first_success) {
					nvram_set("wl0_auth_mode", "psk");
					nvram_set("wl0_wpa_mode", "1");
					nvram_set("wl1_auth_mode", "psk");
					nvram_set("wl1_wpa_mode", "1");
				}
#endif
				is_psk = 1;
			}
			else if (!strcmp(pt1, "SHARED")) {
				nvram_set(strcat_r(prefix, "auth_mode", tmp), "shared");
#ifndef SWMODE_REPEATER_V2_SUPPORT
				if (wps_first_success) {
					nvram_set("wl0_auth_mode", "shared");
					nvram_set("wl1_auth_mode", "shared");
				}
#endif
			}
#ifdef WPA_ENTERPRISE_SUPPORT
			else if (!strcmp(pt1, "WPA")) {
				nvram_set(strcat_r(prefix, "auth_mode", tmp), "wpa");
				nvram_set(strcat_r(prefix, "wpa_mode", tmp), "4");
#ifndef SWMODE_REPEATER_V2_SUPPORT
				if (wps_first_success) {
					nvram_set("wl0_auth_mode", "wpa");
					nvram_set("wl0_wpa_mode", "4");
					nvram_set("wl1_auth_mode", "wpa");
					nvram_set("wl1_wpa_mode", "4");
				}
#endif
			}
			else if (!strcmp(pt1, "WPA2")) {
				nvram_set(strcat_r(prefix, "auth_mode", tmp), "wpa2");
				nvram_set(strcat_r(prefix, "wpa_mode", tmp), "4");
#ifndef SWMODE_REPEATER_V2_SUPPORT
				if (wps_first_success) {
					nvram_set("wl0_auth_mode", "wpa2");
					nvram_set("wl0_wpa_mode", "4");
					nvram_set("wl1_auth_mode", "wpa2");
					nvram_set("wl1_wpa_mode", "4");
				}
#endif
			}
#endif
			else if (!strcmp(pt1, "WPAPSKWPA2PSK")) {
				nvram_set(strcat_r(prefix, "auth_mode", tmp), "psk");
				nvram_set(strcat_r(prefix, "wpa_mode", tmp), "0");
#ifndef SWMODE_REPEATER_V2_SUPPORT
				if (wps_first_success) {
					nvram_set("wl0_auth_mode", "psk");
					nvram_set("wl0_wpa_mode", "0");
					nvram_set("wl1_auth_mode", "psk");
					nvram_set("wl1_wpa_mode", "0");
				}
#endif
				is_psk = 1;
			}
			else if (!strcmp(pt1, "WPA2PSK")) {
				nvram_set(strcat_r(prefix, "auth_mode", tmp), "psk");
				nvram_set(strcat_r(prefix, "wpa_mode", tmp), "2");
#ifndef SWMODE_REPEATER_V2_SUPPORT
				if (wps_first_success) {
					nvram_set("wl0_auth_mode", "psk");
					nvram_set("wl0_wpa_mode", "2");
					nvram_set("wl1_auth_mode", "psk");
					nvram_set("wl1_wpa_mode", "2");
				}
#endif
				is_psk = 1;
			}
			else
				fprintf(stderr, "!! Invalid AuthType:%s\n", pt1);

			//EncrypType
			pt2++;
			pt1 = strstr(pt2, "= ");
			pt1 += 2;
			pt2 = strstr(pt1, "\n");
			*pt2 = '\0';
			if (!strcmp(pt1, "NONE")) {
				nvram_set(strcat_r(prefix, "wep_x", tmp), "0");
#ifndef SWMODE_REPEATER_V2_SUPPORT
				if (wps_first_success) {
					nvram_set("wl0_wep_x", "0");
					nvram_set("wl1_wep_x", "0");
				}
#endif
				is_nokey = 1;
			}
			else if (!strcmp(pt1, "WEP")) {
				nvram_set(strcat_r(prefix, "wep_x", tmp), "1");
#ifndef SWMODE_REPEATER_V2_SUPPORT
				if (wps_first_success) {
					nvram_set("wl0_wep_x", "1");
					nvram_set("wl1_wep_x", "1");
				}
#endif
			}
			else if (!strcmp(pt1, "TKIP")) {
				nvram_set(strcat_r(prefix, "crypto", tmp), "tkip");
#ifndef SWMODE_REPEATER_V2_SUPPORT
				if (wps_first_success) {
					nvram_set("wl0_crypto", "tkip");
					nvram_set("wl1_crypto", "tkip");
				}
#endif
			}
			else if (!strcmp(pt1, "AES")) {
				nvram_set(strcat_r(prefix, "crypto", tmp), "aes");
#ifndef SWMODE_REPEATER_V2_SUPPORT
				if (wps_first_success) {
					nvram_set("wl0_crypto", "aes");
					nvram_set("wl1_crypto", "aes");
				}
#endif
			}
			else if (!strcmp(pt1, "TKIPAES")) {
				nvram_set(strcat_r(prefix, "crypto", tmp), "tkip+aes");
#ifndef SWMODE_REPEATER_V2_SUPPORT
				if (wps_first_success) {
					nvram_set("wl0_crypto", "tkip+aes");
					nvram_set("wl1_crypto", "tkip+aes");
				}
#endif
			}
			else
				fprintf(stderr, "!! Invalid EncrypType Type:%s\n", pt1);
			nvram_set(strcat_r(prefix, "key_type", tmp), "1");
			nvram_set(strcat_r(prefix, "key1", tmp), "");
			nvram_set(strcat_r(prefix, "key2", tmp), "");
			nvram_set(strcat_r(prefix, "key3", tmp), "");
			nvram_set(strcat_r(prefix, "key4", tmp), "");
#ifndef SWMODE_REPEATER_V2_SUPPORT
			if (wps_first_success) {
				nvram_set("wl0_key_type", "1");
				nvram_set("wl0_key1", "");
				nvram_set("wl0_key2", "");
				nvram_set("wl0_key3", "");
				nvram_set("wl0_key4", "");
				nvram_set("wl1_key_type", "1");
				nvram_set("wl1_key1", "");
				nvram_set("wl1_key2", "");
				nvram_set("wl1_key3", "");
				nvram_set("wl1_key4", "");
			}
#endif
			if (is_nokey) {
				nvram_set(strcat_r(prefix, "key", tmp), "1");
#ifndef SWMODE_REPEATER_V2_SUPPORT
				if (wps_first_success) {
					nvram_set("wl0_key", "1");
					nvram_set("wl1_key", "1");
				}
#endif
				ret = 1;
			}
			else {
				//KeyIndex
				pt2++;
				pt1 = strstr(pt2, "= ");
				pt1 += 2;
				pt2 = strstr(pt1, "\n");
				*pt2 = '\0';
				nvram_set(strcat_r(prefix, "key", tmp), pt1);
#ifndef SWMODE_REPEATER_V2_SUPPORT
				if (wps_first_success) {
					nvram_set("wl0_key", pt1);
					nvram_set("wl1_key", pt1);
				}
#endif
				key_idx = atoi(pt1);

				//Key
				pt2++;
				pt1 = strstr(pt2, "= ");
				if (pt1) {
					pt1 += 2;
					pt2 = strstr(pt1, "\n");
					*pt2 = '\0';
					sprintf(buf, "%s%d", strcat_r(prefix, "key", tmp), key_idx);
					nvram_set(buf, pt1);
#ifndef SWMODE_REPEATER_V2_SUPPORT
					if (wps_first_success) {
						sprintf(buf, "wl0_key%d", key_idx);
						nvram_set(buf, pt1);
						sprintf(buf, "wl1_key%d", key_idx);
						nvram_set(buf, pt1);
					}
#endif
					if (is_psk) {
						// fix Ralink wireless driver bug
						char _wpa_psk[65];
						int _len = strlen(pt1);
						if (_len > 64)
							_len = 64;
						memset(_wpa_psk, 0x0, sizeof(_wpa_psk));
						strncpy(_wpa_psk, pt1, _len);

						nvram_set(strcat_r(prefix, "wpa_psk", tmp), _wpa_psk);
#ifndef SWMODE_REPEATER_V2_SUPPORT
						if (wps_first_success) {
							nvram_set("wl0_wpa_psk", _wpa_psk);
							nvram_set("wl1_wpa_psk", _wpa_psk);
						}
#endif
					}
				}
				else
					fprintf(stderr, "No key found!!\n");
				ret = 1;
			}
		}
		free(fp);
	}
	unlink(WPS_PAP_PROFILE_PATH);

	return ret;
}

#ifdef DUAL_BAND_AUTOSELECT 
void GuessSSIDProfile(int band_chk, char *ssidptr)
{
/*
 * format sample:
 * SSID                            = ASUS-Vic1
 * AuthType                        = OPEN
 * EncrypType                      = WEP
 * KeyIndex                        = 1
 * Key                             = 6162636465
 */
	char *strptr1;
	char ssidptr1[]="staXXXXXXXXXX_", ssidptr2[]="staXXXXXXXXXX_", tmp[128];

	snprintf(ssidptr1, sizeof(ssidptr1),"sta%d_",  band_chk);
	snprintf(ssidptr2, sizeof(ssidptr2),"sta%d_",  band_chk==0? 1:0 );
	//set SSID
	nvram_set(strcat_r(ssidptr1, "ssid", tmp), ssidptr);

	//AuthType
	strptr1 = nvram_get( strcat_r(ssidptr2, "auth_mode", tmp));
	nvram_set(strcat_r(ssidptr1, "auth_mode", tmp), strptr1);

	strptr1 = nvram_get( strcat_r(ssidptr2, "wpa_mode", tmp));
	nvram_set(strcat_r(ssidptr1, "wpa_mode", tmp), strptr1);

	//EncrypType
	strptr1 = nvram_get( strcat_r(ssidptr2, "wep_x", tmp));
	nvram_set(strcat_r(ssidptr1, "wep_x", tmp), strptr1);		

	strptr1 = nvram_get( strcat_r(ssidptr2, "crypto", tmp));
	nvram_set(strcat_r(ssidptr1, "crypto", tmp), strptr1);

	//Key
	strptr1 = nvram_get( strcat_r(ssidptr2, "key_type", tmp));
	nvram_set(strcat_r(ssidptr1, "key_type", tmp), strptr1);
	strptr1 = nvram_get( strcat_r(ssidptr2, "key", tmp));
	nvram_set(strcat_r(ssidptr1, "key", tmp), strptr1);
	strptr1 = nvram_get( strcat_r(ssidptr2, "key", tmp));
	nvram_set(strcat_r(ssidptr1, "key1", tmp), strptr1);
	strptr1 = nvram_get( strcat_r(ssidptr2, "key", tmp));
	nvram_set(strcat_r(ssidptr1, "key2", tmp), strptr1);
	strptr1 = nvram_get( strcat_r(ssidptr2, "key", tmp));
	nvram_set(strcat_r(ssidptr1, "key3", tmp), strptr1);
	strptr1 = nvram_get( strcat_r(ssidptr2, "key4", tmp));
	nvram_set(strcat_r(ssidptr1, "key4", tmp), strptr1);

	//Wpa_Psk
	strptr1 = nvram_get( strcat_r(ssidptr2, "wpa_psk", tmp));
	nvram_set(strcat_r(ssidptr1, "wpa_psk", tmp), strptr1);	
	
	return ; 
}
#endif

int ClearWscStaOldProfile(int n)
{
	char tmp[128], prefix[] = "staXXXXXXXXXX_";

	snprintf(prefix, sizeof(prefix), "sta%d_", n);
	nvram_set(strcat_r(prefix, "ssid", tmp), "");
	nvram_set(strcat_r(prefix, "bssid", tmp), "");
	nvram_set(strcat_r(prefix, "auth_mode", tmp), "open");
	nvram_set(strcat_r(prefix, "wep_x", tmp), "0");
	nvram_set(strcat_r(prefix, "wpa_mode", tmp), "0");
	nvram_set(strcat_r(prefix, "crypto", tmp), "tkip+aes");
	nvram_set(strcat_r(prefix, "wpa_psk", tmp), "");
	nvram_set(strcat_r(prefix, "key", tmp), "1");
	nvram_set(strcat_r(prefix, "key_type", tmp), "0");
	nvram_set(strcat_r(prefix, "key1", tmp), "");
	nvram_set(strcat_r(prefix, "key2", tmp), "");
	nvram_set(strcat_r(prefix, "key3", tmp), "0");
	nvram_set(strcat_r(prefix, "key4", tmp), "");
}
#endif /* WPS_SUPPORT */


/* 
 * Ralink GPIO setting 
 */
#define GPIO_DEV	"/dev/gpio"

int ra_gpio_set_dir(unsigned long dir)
{
	int fd;

	fd = open(GPIO_DEV, O_RDONLY);
	if (fd < 0) {
		perror(GPIO_DEV);
		return -1;
	}

#if (defined(RALINK_GPIO_HAS_9524) || defined(RALINK_GPIO_HAS_7224))
	fprintf(stderr, "set GPIO 0~23 dir: %x\n", dir);
#elif (defined(RALINK_GPIO_HAS_9532))
	fprintf(stderr, "set GPIO 0~31 dir: %x\n", dir);
#else
	fprintf(stderr, "set GPIO 0~23 dir: %x\n", dir);
#endif
	if (ioctl(fd, RALINK_GPIO_SET_DIR, dir) < 0) {
		perror("ioctl");
		close(fd);
		return -1;
	}
	close(fd);

	return 0;
}

#if (defined(RALINK_GPIO_HAS_9524) || defined(RALINK_GPIO_HAS_7224))
int ra_gpio3924_set_dir(unsigned long dir)
{
	int fd;

	fd = open(GPIO_DEV, O_RDONLY);
	if (fd < 0) {
		perror(GPIO_DEV);
		return -1;
	}

	fprintf(stderr, "set GPIO 24~39 dir: %x\n", dir);
	if (ioctl(fd, RALINK_GPIO3924_SET_DIR, dir) < 0) {
		perror("ioctl");
		close(fd);
		return -1;
	}
	close(fd);

	return 0;
}

int ra_gpio7140_set_dir(unsigned long dir)
{
	int fd;

	fd = open(GPIO_DEV, O_RDONLY);
	if (fd < 0) {
		perror(GPIO_DEV);
		return -1;
	}

	fprintf(stderr, "set GPIO 40~71 dir: %x\n", dir);
	if (ioctl(fd, RALINK_GPIO7140_SET_DIR, dir) < 0) {
		perror("ioctl");
		close(fd);
		return -1;
	}
	close(fd);

	return 0;
}
#elif (defined(RALINK_GPIO_HAS_9532))
int ra_gpio6332_set_dir(unsigned long dir)
{
	int fd;

	fd = open(GPIO_DEV, O_RDONLY);
	if (fd < 0) {
		perror(GPIO_DEV);
		return -1;
	}

	fprintf(stderr, "set GPIO 32~63 dir: %x\n", dir);
	if (ioctl(fd, RALINK_GPIO6332_SET_DIR, dir) < 0) {
		perror("ioctl");
		close(fd);
		return -1;
	}
	close(fd);

	return 0;
}

int ra_gpio9564_set_dir(unsigned long dir)
{
	int fd;

	fd = open(GPIO_DEV, O_RDONLY);
	if (fd < 0) {
		perror(GPIO_DEV);
		return -1;
	}

	fprintf(stderr, "set GPIO 64~95 dir: %x\n", dir);
	if (ioctl(fd, RALINK_GPIO9564_SET_DIR, dir) < 0) {
		perror("ioctl");
		close(fd);
		return -1;
	}
	close(fd);

	return 0;
}
#endif

int ra_gpio_read(int idx)
{
	int fd, req, value;

	value = 0;
	fd = open(GPIO_DEV, O_RDONLY);
	if (fd < 0) {
		perror(GPIO_DEV);
		return -1;
	}
#if (defined(RALINK_GPIO_HAS_9524) || defined(RALINK_GPIO_HAS_7224))
	if(idx <= 23)
		req = RALINK_GPIO_READ;
	else if (idx > 23 && idx <= 39) {
		idx -= 24;
		req = RALINK_GPIO3924_READ;
	}
	else if (idx > 39 && idx <= 71) {
		idx -= 40;
		req = RALINK_GPIO7140_READ;
	}
#elif (defined(RALINK_GPIO_HAS_9532))
	if(idx <= 31)
		req = RALINK_GPIO_READ;
	else if (idx > 31 && idx <= 63) {
		idx -= 32;
		req = RALINK_GPIO6332_READ;
	}
	else if (idx > 63 && idx <= 95) {
		idx -= 64;
		req = RALINK_GPIO9564_READ;
	}
#else
	if(idx <= 23)
		req = RALINK_GPIO_READ;
#endif
	else {
		close(fd);
		printf("%s: GPIO%d out of range\n", __FUNCTION__, idx);
		return -1;
	}

	if (ioctl(fd, req, &value) < 0) {
		perror("ioctl");
		close(fd);
		return -1;
	}

	value = (value >> idx) & 1;
	close(fd);

	return value;
}

int ra_gpio_write_bit(int idx, int value)
{
	int fd, req;

	fd = open(GPIO_DEV, O_RDONLY);
	if (fd < 0) {
		perror(GPIO_DEV);
		return -1;
	}
	value &= 1;
#if (defined(RALINK_GPIO_HAS_9524) || defined(RALINK_GPIO_HAS_7224))
	if (idx <= 23) {
		if (value)
			req = RALINK_GPIO_SET;
		else
			req = RALINK_GPIO_CLEAR;
	}
	else if (idx > 23 && idx <= 39) {
		idx -= 24;
		if (value)
			req = RALINK_GPIO3924_SET;
		else
			req = RALINK_GPIO3924_CLEAR;
	}
	else if (idx > 39 && idx <= 71) {
		idx -= 40;
		if (value)
			req = RALINK_GPIO7140_SET;
		else
			req = RALINK_GPIO7140_CLEAR;
	}
#elif (defined(RALINK_GPIO_HAS_9532))
	if (idx <= 31) {
		if (value)
			req = RALINK_GPIO_SET;
		else
			req = RALINK_GPIO_CLEAR;
	}
	else if (idx > 31 && idx <= 63) {
		idx -= 32;
		if (value)
			req = RALINK_GPIO6332_SET;
		else
			req = RALINK_GPIO6332_CLEAR;
	}
	else if (idx > 63 && idx <= 95) {
		idx -= 64;
		if (value)
			req = RALINK_GPIO9564_SET;
		else
			req = RALINK_GPIO9564_CLEAR;
	}
#else
	if (idx <= 23) {
		if (value)
			req = RALINK_GPIO_SET;
		else
			req = RALINK_GPIO_CLEAR;
	}
#endif
	else {
		close(fd);
		fprintf(stderr, "%s: GPIO%d out of range\n", __FUNCTION__, idx);
		return -1;
	}

	if (ioctl(fd, req, (1 << idx)) < 0) {
		perror("ioctl");
		close(fd);
		return -1;
	}
	close(fd);

	return 0;
}

int ra_gpio_enb_irq(void)
{
	int fd;

	fd = open(GPIO_DEV, O_RDONLY);
	if (fd < 0) {
		perror(GPIO_DEV);
		return -1;
	}
	if (ioctl(fd, RALINK_GPIO_ENABLE_INTP) < 0) {
		perror("ioctl");
		close(fd);
		return -1;
	}
	close(fd);
	return 0;
}

int ra_gpio_dis_irq(void)
{
	int fd;

	fd = open(GPIO_DEV, O_RDONLY);
	if (fd < 0) {
		perror(GPIO_DEV);
		return -1;
	}
	if (ioctl(fd, RALINK_GPIO_DISABLE_INTP) < 0) {
		perror("ioctl");
		close(fd);
		return -1;
	}
	close(fd);
	return 0;
}

int ra_gpio_reg_info(int gpio_num)
{
	int fd;

	ralink_gpio_reg_info info;

	fd = open(GPIO_DEV, O_RDONLY);
	if (fd < 0) {
		perror(GPIO_DEV);
		return -1;
	}
	info.pid = getpid();
	info.irq = gpio_num;
	if (ioctl(fd, RALINK_GPIO_REG_IRQ, &info) < 0) {
		perror("ioctl");
		close(fd);
		return -1;
	}
	close(fd);

	return 0;
}

/* 
 * enable GPIO interrupt
 */
void enable_gpio_irq(void)
{
	/* enable interrupt */
	ra_gpio_enb_irq();
#ifdef TOUCH_SUPPORT
	// enable GPIO#24 interrupt for touch sensor
	ra_gpio_reg_info(TOUCH_SENSOR_GPIO_IRQ);
#endif
#ifdef AUDIO_JACK_SUPPORT
	// enable GPIO#27 interrupt for audio jack detect
	ra_gpio_reg_info(AUDIO_JACK_GPIO_IRQ);
#endif
#ifdef VOL_KNOB_SUPPORT
	// enable GPIO#40~42 interrupt for volume knob
	ra_gpio_reg_info(VOL_KNOB_A_GPIO_IRQ);
	ra_gpio_reg_info(VOL_KNOB_B_GPIO_IRQ);
	ra_gpio_reg_info(VOL_KNOB_M_GPIO_IRQ);
#endif
}

void gpio_init(void)
{
#if (defined(CONFIG_DEFAULTS_ASUS_EAN66))
	/* set GPIO mode */
	eval("reg", "s", "0xb0000000");
	eval("reg", "w", "60", "1C5C");

	/* set GPIO 24, 12, 10, 0 dir out, others in */
	ra_gpio_set_dir(0x1401);
	ra_gpio3924_set_dir(0x0001);
	ra_gpio_write_bit(10, 1);// Atenna diversion
	ra_gpio_write_bit(12, 1);// PHY reset (GPIO#12)
#elif (defined(CONFIG_DEFAULTS_ASUS_RPN12))
	/* set GPIO mode */
	eval("reg", "s", "0xb0000000");
	//eval("reg", "w", "60", "54154444");
	eval("reg", "w", "60", "571544c4");
	eval("reg", "w", "64", "140014");// reserved, valid implement point in drivers/net/raeth/raether.c

	/* set GPIO 43, 5~2 dir out, others in */
	ra_gpio_set_dir(0x03c);
	ra_gpio6332_set_dir(0x0800);
#elif (defined(CONFIG_DEFAULTS_ASUS_RPN14) || defined(CONFIG_DEFAULTS_ASUS_RPN53) || defined(CONFIG_DEFAULTS_ASUS_RPAC52))
	/* set GPIO mode */
	eval("reg", "s", "0xb0000000");
	eval("reg", "w", "60", "1A3798");

	/* set GPIO 35~28 dir out, others in */
	ra_gpio_set_dir(0x0);
	ra_gpio3924_set_dir(0x0ff0);
	ra_gpio7140_set_dir(0x0);
#elif (defined(CONFIG_DEFAULTS_ASUS_RPAC56))
//TBD.
#elif (defined(CONFIG_DEFAULTS_ASUS_WMPN12))
	/* set GPIO mode */
	eval("reg", "s", "0xb0000000");
	eval("reg", "w", "60", "1AB799");

	/* set GPIO 71~60, 43, 35~28, 25, 23~22 dir out, others in */
	ra_gpio_set_dir(0xc00000);
	ra_gpio3924_set_dir(0x00ff2);
	ra_gpio7140_set_dir(0xfff00008);
#else
#error Invalid Product!!
#endif
}


#ifdef ROAMING_SUPPORT
typedef struct _ROAM_INFO
{
	char mac[18];
	char rssi[3][5];
	char newline;
} ROAM_INFO;

typedef struct _ROAM_INFO_ARRAY
{
	ROAM_INFO sta[128];
} RIA;

void rssi_check(int n, char *wif, char *aif)
{
	int _sw_mode = atoi(nvram_safe_get("sw_mode"));
	char tmp[128], prefix[] = "wlXXXXXXXXXX_";
	char data[2048], header[64], pap_bssid[18];
	struct iwreq wrq, wrq2;
	int i = 0,staCount = 0, rssi_th = 0;
	RIA *ria;

	snprintf(prefix, sizeof(prefix), "wl%d_", n);
	if (!(rssi_th = atoi(nvram_safe_get(strcat_r(prefix, "user_rssi", tmp)))))
		return;

	memset(data, 0x0, sizeof(data));
	wrq.u.data.length = sizeof(data);
	wrq.u.data.pointer = (caddr_t) data;
	wrq.u.data.flags = ASUS_SUBCMD_GROAM;
	if (wl_ioctl(wif, RTPRIV_IOCTL_ASUSCMD, &wrq) < 0) {
		fprintf(stderr, "errors in getting GROAM result\n");
		return;
	}

	memset(header, 0x0, sizeof(header));
	sprintf(header, "%-18s%-5s%-5s%-5s\n", "MAC", "RSSI0", "RSSI1", "RSSI2");

	if (wrq.u.data.length > 0) {
		ria = (RIA *)(wrq.u.data.pointer + strlen(header));
		int len = strlen(wrq.u.data.pointer + strlen(header));
		char *sp, *op;
		op = sp = wrq.u.data.pointer + strlen(header);
		while (*sp && ((len - (sp-op)) >= 0)) {
			ria->sta[i].mac[17] = '\0';
			ria->sta[i].rssi[0][4] = '\0';
			ria->sta[i].rssi[1][4] = '\0';
			ria->sta[i].rssi[2][4] = '\0';
			sp += strlen(header);
			staCount = ++i;
		}

		memset(pap_bssid, 0x0, sizeof(pap_bssid));
		if (_sw_mode == 2 
#ifdef SWMODE_HOTSPOT_SUPPORT
				|| _sw_mode == 5
#endif
		) {
			if (wl_ioctl(aif, SIOCGIWAP, &wrq2) >= 0) {
				wrq2.u.ap_addr.sa_family = ARPHRD_ETHER;
				sprintf(pap_bssid, "%02X:%02X:%02X:%02X:%02X:%02X", 
							(unsigned char)wrq2.u.ap_addr.sa_data[0], 
							(unsigned char)wrq2.u.ap_addr.sa_data[1], 
							(unsigned char)wrq2.u.ap_addr.sa_data[2], 
							(unsigned char)wrq2.u.ap_addr.sa_data[3], 
							(unsigned char)wrq2.u.ap_addr.sa_data[4], 
							(unsigned char)wrq2.u.ap_addr.sa_data[5]);
			}
		}

		if (staCount) {
			int j, _rssi[3] = {0};
			int wl_HT_TxStream = atoi(nvram_safe_get(strcat_r(prefix, "HT_TxStream", tmp)));
			for (i = 0; i < staCount; i++) {
				if (!strncmp(pap_bssid, ria->sta[i].mac, sizeof(pap_bssid))) {
					//fprintf(stderr, "[roaming] sta%d: mac=%s <--- parent-AP\n", i, ria->sta[i].mac);
					continue;
				}

				for (j = 0; j < wl_HT_TxStream; j++)
					_rssi[j] = atoi(ria->sta[i].rssi[j]);
				//fprintf(stderr, "[roaming] sta%d: mac=%s, rssi=%d/%d/%d\n", i, ria->sta[i].mac, _rssi[0], _rssi[1], _rssi[2]);

				if ((_rssi[0] == 0 || _rssi[0] < rssi_th) 
						&& (_rssi[1] == 0 || _rssi[1] < rssi_th) 
						&& (_rssi[2] == 0 || _rssi[2] < rssi_th))
					doSystem("iwpriv %s set DisConnectSta=%s", wif, ria->sta[i].mac);
			}
		}
	}
}
#endif /* ROAMING_SUPPORT */
