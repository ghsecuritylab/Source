#ifdef ASUS_ATE
#include <stdio.h>
#include <errno.h>

#include <nvram/bcmnvram.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>

#include <shared.h>

#include "rc.h"

#define MULTICAST_BIT  0x0001
#define UNIQUE_OUI_BIT 0x0002

int isValidMacAddr(const char* mac)
{
	int sec_byte;
	int i = 0, s = 0;

	if (strlen(mac) != 17 || !strcmp("00:00:00:00:00:00", mac))
		return 0;

	while (*mac && i < 12) {
		if (isxdigit(*mac)) {
			if (i == 1) {
				sec_byte = strtol(mac, NULL, 16);
				if ((sec_byte & MULTICAST_BIT) || (sec_byte & UNIQUE_OUI_BIT))
					break;
			}
			i++;
		}
		else if ( *mac==':') {
			if (i == 0 || i/2-1 != s)
				break;
			++s;
		}
		++mac;
	}
	return (i == 12 && s == 5);
}

#if 0
int isValidRegrev(char *regrev)
{
	char *c = regrev;
	int len, i = 0, ret = 0;

	len = strlen(regrev);

	if (len == 1 || len == 2) {
		while (i < len) { //0~9
			if ((*c > 0x2F && *c < 0x3A)) {
				i++;
				c++;
				ret = 1;
			}
			else {
				ret = 0;
				break;
			}
		}
	}

	return ret;
}
#endif

int isValidChannel(int band, char *channel)
{
	char *c = channel;
	int len, i = 0, ret = 0;

	len = strlen(channel);

	if ((band == 0 && (len == 1 || len == 2)) 
			|| (band == 1 && (len == 2 || len == 3))) {
		while (i < len) { //0~9
			if ((*c > 0x2F && *c < 0x3A)) {
				i++;
				c++;
				ret = 1;
			}
			else {
				ret = 0;
				break;
			}
		}
	}

	return ret;
}

int pincheck(const char *a)
{
	unsigned char *c = (char *) a;
	unsigned long int uiPINtemp = atoi(a);
	unsigned long int uiAccum = 0;
	int i = 0;

	for (;;) {
		if (*c > 0x39 || *c < 0x30)
			break;
		else
			i++;
		if (!*c++ || i == 8)
			break;
	}
	if (i == 8) {
		uiAccum += 3 * ((uiPINtemp / 10000000) % 10);
		uiAccum += 1 * ((uiPINtemp / 1000000) % 10);
		uiAccum += 3 * ((uiPINtemp / 100000) % 10);
		uiAccum += 1 * ((uiPINtemp / 10000) % 10);
		uiAccum += 3 * ((uiPINtemp / 1000) % 10);
		uiAccum += 1 * ((uiPINtemp / 100) % 10);
		uiAccum += 3 * ((uiPINtemp / 10) % 10);
		uiAccum += 1 * ((uiPINtemp / 1) % 10);
		if (0 != (uiAccum % 10)) {
			return 0;
		}
		return 1;
	}
	else
		return 0;
}

int isValidSN(const char *sn)
{
	int i;
	unsigned char *c;

	if (strlen(sn) != SERIAL_NUMBER_LENGTH)
		return 0;

	c = (char *)sn;
	/* [1]year: C~Z (2012=C, 2013=D, ...) */
	if (*c < 0x43 || *c > 0x5A)
		return 0;
	c++;
	/* [2]month: 1~9 & ABC */
	if (!((*c > 0x30 && *c < 0x3A) || *c == 0x41 || *c == 0x42 || *c == 0x43))
		return 0;
	c++;
	/* [3]WLAN & ADSL: I(aye) */
	if (*c != 0x49)
		return 0;
	c++;
	/* [4]Channel: AEJ0(zero) (A:11ch, E:13ch, J:14ch, 0:no ch) */
	if (*c != 0x41 && *c != 0x45 && *c != 0x4A && *c != 0x30)
		return 0;
	c++;
	/* [5]factory: 0~9 & A~Z, except I(aye) & O(oh) */
	if (!((*c > 0x2F && *c < 0x3A) || (*c > 0x40 && *c < 0x5B)) || *c == 0x49 || *c == 0x4F)
		return 0;
	c++;
	/* [6]model: 0~9 & A~Z */
	if (!((*c > 0x2F && *c < 0x3A) || (*c > 0x40 && *c < 0x5B)))
		return 0;
	c++;
	/* [7~12]serial: 0~9 */
	i = 7;
	while (i < 13) {
		if (*c < 0x30 || *c > 0x39)
			return 0;
		c++;
		i++;
	}

	return 1;
}

#if 0
int Get_USB_Port_Info(int port_x)
{
	char output_buf[16];
	char usb_pid[14];
	char usb_vid[14];
	sprintf(usb_pid, "usb_path%d_pid", port_x);
	sprintf(usb_vid, "usb_path%d_vid", port_x);

	if (strcmp(nvram_get(usb_pid),"") && strcmp(nvram_get(usb_vid),"")) {
		sprintf(output_buf, "%s/%s", nvram_get(usb_pid), nvram_get(usb_vid));
		puts(output_buf);
	}
	else
		puts("N/A");

	return 1;
}

int Get_USB_Port_Folder(int port_x)
{
	char usb_folder[19];
	sprintf(usb_folder, "usb_path%d_fs_path0", port_x);
	if (strcmp(nvram_safe_get(usb_folder), ""))
		puts(nvram_safe_get(usb_folder));
	else
		puts("N/A");

	return 1;
}

int Get_SD_Card_Info(void)
{
	char check_cmd[48];
	char sd_info_buf[128];
	int get_sd_card = 1;
	FILE *fp;

	if (nvram_match("usb_path3_fs_path0", "")) {
		puts("0");
		return 1;
	}

	sprintf(check_cmd, "test_disk2 %s &> /var/sd_info.txt", nvram_get("usb_path3_fs_path0"));
	system(check_cmd);

	if (fp = fopen("/var/sd_info.txt", "r")) {
		while (fgets(sd_info_buf, 128, fp) != NULL) {
			if (strstr(sd_info_buf, "No partition") || strstr(sd_info_buf, "No disk"))
				get_sd_card = 0;
		}
		if (get_sd_card)
			puts("1");
		else
			puts("0");
		fclose(fp);
		eval("rm", "-rf", "/var/sd_info.txt");
	}
	else
		puts("ATE_ERROR");

	return 1;
}

int Get_SD_Card_Folder(void)
{
	if (strcmp(nvram_safe_get("usb_path3_fs_path0"), ""))
		puts(nvram_safe_get("usb_path3_fs_path0"));
	else
		puts("N/A");

	return 1;
}

int Ej_device(const char *dev_no)
{
	if(dev_no == NULL || *dev_no < '1' || *dev_no > '9') 
		return 0;
	else {
		eval("ejusb", dev_no);
		sleep(4);
		puts("1");
	}
	return 1;
}
#endif

static void pkt_flood(void)
{
	struct sockaddr_ll dev;
	int fd;
	unsigned char buffer[1514];

	fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	dev.sll_family = AF_PACKET;
	dev.sll_protocol = htons(ETH_P_ALL);
	dev.sll_ifindex = 2;
	bind(fd, (struct sockaddr *) &dev, sizeof(dev));

	memset(buffer, 0xff, 6);
	FRead(buffer+6, MAC_EEPROM_ADDR, 6);
	memset(buffer+12, 0x55, 1502);
	while (1)
		send(fd, buffer, 1514, 0);
}

int asus_ate_command(const char *command, const char *value, const char *value2)
{
	int n;
	char wif[8], *wif_next, aif[8], *aif_next;
	//fprintf(stderr, "===[ATE %s %s]===\n", command, value);

	if (!strcmp(command, "Set_StartATEMode")) {
		nvram_set("asus_mfg", "1");
#ifdef LED_DRV_SUPPORT
		nvram_set("led_drv_stage", "9");
#if defined(CONFIG_DEFAULTS_ASUS_RPN12)
		/* set I2S to GPIO mode */
		eval("reg", "s", "0xb0000000");
		eval("reg", "w", "60", "54154444");
#endif
#endif
		if (nvram_match("asus_mfg", "1")) {
			puts("1");
#ifdef WPS_SUPPORT
			stop_wsc();
#endif
			stop_ntpc();
			stop_lltd();
			stop_udhcpc();
#ifdef WEB_REDIRECT
			stop_wanduck();
#endif
			stop_logger();
#ifdef AUDIO_SUPPORT
			unlink("/tmp/iradio");
			stop_audiod();
#endif
			stop_dnsmasq();
			stop_networkmap();
			stop_apcli_monitor();
			for2each(n, wif, nvram_safe_get("wl_ifnames"), wif_next,
					aif, nvram_safe_get("sta_ifnames"), aif_next) {
				eval("brctl", "delif", "br0", aif);
				ap_set(wif, "AuthMode=OPEN");
				ap_set(wif, "EncrypType=NONE");
				ap_set(wif, "HideSSID=0");
#if defined(CONFIG_DEFAULTS_ASUS_RPN53)
				if (nvram_match("fac_model_name", "RP-N54"))
					doSystem("iwpriv %s set SSID=%s", wif, n==0?"ASUS_RPN54":"ASUS_RPN54_5G");
				else
#endif
				doSystem("iwpriv %s set SSID=%s", wif, n==0?OOB_SSID_2G:OOB_SSID_5G);
				ap_set(aif, "ApCliEnable=0");
				ifconfig(aif, 0, NULL, NULL);
			}
			eval("ifconfig", "br0", "hw", "ether", nvram_safe_get("wl0_macaddr"));
#ifdef AUDIO_SUPPORT
#ifdef I2C_TO_ALC5627
			system("amixer cset numid=1 50% 1> /dev/null");
#endif
#ifdef USB_TO_CM6510
			system("amixer cset numid=4 50% 1> /dev/null");
#endif
#endif
		}
		else
			puts("ATE_ERROR");
		return 0;
	}
	else if (!strcmp(command, "Set_AllLedOn") 
			|| !strcmp(command, "Set_AllBlueLedOn")) {
		puts("1");
#ifdef LED_DRV_SUPPORT
		nvram_set("led_drv_ate", "1");
#endif
		return 0;
	}
	else if (!strcmp(command, "Set_AllWhiteLedOn")) {
		puts("1");
#ifdef LED_DRV_SUPPORT
		nvram_set("led_drv_ate", "4");
#endif
		return 0;
	}
	else if (!strcmp(command, "Set_AllOrangeLedOn")) {
		puts("1");
#ifdef LED_DRV_SUPPORT
		nvram_set("led_drv_ate", "5");
#endif
		return 0;
	}
#if (defined(AUDIO_SUPPORT) && defined(VOL_KNOB_SUPPORT))
	else if (!strcmp(command, "Set_VolumeLed")) {
		int idx, flag;

		if (value && value2 
				&& (idx = atoi(value)) > 0 && idx < 20 
				&& ((flag = atoi(value2)) == 0 || flag == 1)) {
#ifdef LED_DRV_SUPPORT
			nvram_set("led_drv_ate", "0");
#endif
			if (idx <= 7)
				idx += 28;
			else
				idx += 52;
			ra_gpio_write_bit(idx, 1 - flag);
			puts("1");
			return 0;
		}
		else {
			puts("ATE_ERROR_INCORRECT_PARAMETER");
			return EINVAL;
		}
	}
#endif
	else if (!strcmp(command, "Set_AllLedOff")) {
		puts("1");
#ifdef LED_DRV_SUPPORT
		nvram_set("led_drv_ate", "2");
#endif
		return 0;
	}
	else if (!strcmp(command, "Set_AllLedOn_Half")) {
#ifdef LED_BRIGHTNESS_SUPPORT
		puts("1");
#ifdef LED_DRV_SUPPORT
		nvram_set("led_drv_ate", "3");
#endif
		return 0;
#else
		puts("ATE_ERROR");
		return EINVAL;
#endif
	}
#ifdef EEPROM_BACKWARD_COMPATIBILITY
	else if (!strcmp(command, "Set_BootLoaderVersion")) {
		setBootVer(value);
		return getBootVer();
	}
#endif
	else if (!strcmp(command, "Set_MacAddr_2G")) {
		if (setMAC(0, value) == -1) {
			puts("ATE_ERROR_INCORRECT_PARAMETER");
			return EINVAL;
		}
		getMAC(0);
		return 0;
	}
	else if (!strcmp(command, "Set_MacAddr_5G")) {
		if (setMAC(1, value) == -1) {
			puts("ATE_ERROR_INCORRECT_PARAMETER");
			return EINVAL;
		}
		getMAC(1);
		return 0;
	}
	else if (!strcmp(command, "eeprom")) {
		// eeprom band filename
		// eg. eeprom 5 /tmp/5G.bin
		//     eeprom 2 /tmp/2G.bin
		if ( !eeprom_upgrade(value2, value, 1))
			return EINVAL;
		return 0;
	}
	else if (!strcmp(command, "eeover")) {
		if ( !eeprom_upgrade(value2, value, 0))
			return EINVAL;
		return 0;
	}
#ifdef AUDIO_SUPPORT
	else if (!strcmp(command, "Set_Play")) {
		if (nvram_invmatch("asus_mfg", "0")) {
			int ret;
			char cmdbuf[512];
			if (!value||(strlen(value)==0)) {
				puts("ATE_ERROR_INCORRECT_PARAMETER");
				return EINVAL;
			}
			system("killall -9 asplayer 2> /dev/null");
			sprintf(cmdbuf,"asplayer /usr/lib/testmp3/%s 2> /dev/null",value);
			ret=system(cmdbuf);
			if (ret) {
				puts("Play fail!\n");
				return EINVAL;
			}
			else {
				puts("Play OK!\n");
				return 0;
			}
		}
		return 0;
	}
	else if (!strcmp(command, "Set_PlayURL")) {
		if (nvram_invmatch("asus_mfg", "0")) {
			int ret;
			char cmdbuf[512];
			if (!value||(strlen(value)==0)) {
				puts("ATE_ERROR_INCORRECT_PARAMETER");
				return EINVAL;
			}
			system("killall -9 asplayer 2> /dev/null");
			sprintf(cmdbuf,"asplayer %s 2> /dev/null",value);
			ret=system(cmdbuf);
			if (ret) {
				puts("Play fail!\n");
				return EINVAL;
			}
			else {
				puts("Play OK!\n");
				return 0;
			}
		}
		return 0;
	}
#endif
#ifdef EEPROM_BACKWARD_COMPATIBILITY
	else if (!strcmp(command, "Set_RegulationDomain")) {
		if (setCountryCode(value) == -1) {
			puts("ATE_ERROR_INCORRECT_PARAMETER");
			return EINVAL;
		}
		getCountryCode();
		return 0;
	}
#else
	else if (!strcmp(command, "Set_RegulationDomain_2G")) {
		if (setRegDomain(0, value) == -1) {
			puts("ATE_ERROR_INCORRECT_PARAMETER");
			return EINVAL;
		}
		getRegDomain(0);
		return 0;
	}
	else if (!strcmp(command, "Set_RegulationDomain_5G")) {
		if (setRegDomain(1, value) == -1) {
			puts("ATE_ERROR_INCORRECT_PARAMETER");
			return EINVAL;
		}
		getRegDomain(1);
		return 0;
	}
#endif
#if 0
	else if (!strcmp(command, "Set_Regrev_2G")) {
		if (!setRegrev_2G(value)) {
			puts("ATE_ERROR_INCORRECT_PARAMETER");
			return EINVAL;
		}
		return 0;
	}
	else if (!strcmp(command, "Set_Regrev_5G")) {
		if (!setRegrev_5G(value)) {
			puts("ATE_ERROR_INCORRECT_PARAMETER");
			return EINVAL;
		}
		return 0;
	}
	else if (!strcmp(command, "Set_Commit")) {
		setCommit();
		return 0;
	}
#endif
	else if (!strcmp(command, "pkt_flood2")) {
		if (nvram_invmatch("asus_mfg", "0")) {
			system("/usr/lib/testmp3/loop.sh &");
			pkt_flood();
		}
		return 0;
	}
	else if (!strcmp(command, "pkt_flood")) {
		if (nvram_invmatch("asus_mfg", "0")) {
			pkt_flood();
		}
		return 0;
	}
	else if (!strcmp(command, "Set_SerialNumber")) {
		if (!setSN(value)) {
			puts("ATE_ERROR_INCORRECT_PARAMETER");
			return EINVAL;
		}
		return 0;
 	}
#ifndef EEPROM_BACKWARD_COMPATIBILITY
	else if (!strcmp(command, "Set_RegSpec")) {
		if ((value) && (value[0] != '\0') && (strlen(value) <= MAX_REGSPEC_LEN)) {
				if (setRegSpec(value)==0)
					return 0;
			}
		puts("ATE_ERROR_INCORRECT_PARAMETER");
		return EINVAL;
	}
#endif
	else if (!strcmp(command, "Set_ModelName")) {
		if ((value) && (value[0] != '\0') && (strlen(value) <= 12))
			setModelName(value);
		else {
			puts("ATE_ERROR_INCORRECT_PARAMETER");
			return EINVAL;
		}
		getModelName();
		return 0;
	}
	else if (!strcmp(command, "Set_PINCode")) {
		if (setPIN(value) == -1) {
			puts("ATE_ERROR_INCORRECT_PARAMETER");
			return EINVAL;
		}
		getPIN();
		return 0;
	}
	else if (!strcmp(command, "Set_40M_Channel_2G")) {
		if(!set40M_Channel(0, value)) {
			puts("ATE_ERROR_INCORRECT_PARAMETER");
			return EINVAL;
		}
		return 0;
	}
	else if (!strcmp(command, "Set_40M_Channel_5G")) {
		if(!set40M_Channel(1, value)) {
			puts("ATE_ERROR_INCORRECT_PARAMETER");
			return EINVAL;
		}
		return 0;
	}
	else if (!strcmp(command, "Set_RestoreDefault")) {
		system("/bin/mtd_write erase Config > /dev/null 2>&1");
		puts("1");
		return 0;
	}
#if 0
	else if (!strcmp(command, "Set_Eject")) {
		if (!Ej_device(value)) {
			puts("ATE_ERROR_INCORRECT_PARAMETER");
			return EINVAL;
		}
		return 0;
	}
	else if (!strcmp(command, "Set_FanOn")) {
		setFanOn();
		return 0;
	}
	else if (!strcmp(command, "Set_FanOff")) {
		setFanOff();
		return 0;
	}
#endif
	else if (!strcmp(command, "Get_FWVersion")) {
		eval("nvram", "get", "firmver");
		return 0;
	}
	else if (!strcmp(command, "Get_BootLoaderVersion")) {
		getBootVer();
		return 0;
	}
	else if (!strcmp(command, "Get_ResetButtonStatus")) {
		puts(nvram_safe_get("btn_rst"));
		return 0;
	}
	else if (!strcmp(command, "Get_WpsButtonStatus")) {
		puts(nvram_safe_get("btn_ez"));
		return 0;
	}
#ifdef TOUCH_SUPPORT
	else if (!strcmp(command, "Get_TouchStatus")) {
		puts(nvram_safe_get("btn_touch"));
		return 0;
	}
#endif
#ifdef AUDIO_SUPPORT
#ifdef AUDIO_JACK_SUPPORT
	else if (!strcmp(command, "Get_AudioJackStatus")) {
		puts(nvram_safe_get("btn_jack"));
		return 0;
	}
#endif
	else if (!strcmp(command, "Get_VolumeKnobStatus")) {
#ifdef LED_DRV_SUPPORT
		nvram_set("led_drv_ate", "0");
#endif
		puts(nvram_safe_get("audio_volume"));
		return 0;
	}
#endif
	else if (!strcmp(command, "Get_SWMode")) {
		puts(nvram_safe_get("sw_mode"));
		return 0;
	}
	else if (!strcmp(command, "Get_MacAddr_2G")) {
		getMAC(0);
		return 0;
	}
	else if (!strcmp(command, "Get_MacAddr_5G")) {
		getMAC(1);
		return 0;
	}
#if 0
	else if (!strcmp(command, "Get_Usb2p0_Port1_Infor")) {
		Get_USB_Port_Info(1);
		return 0;
	}
	else if (!strcmp(command, "Get_Usb2p0_Port1_Folder")) {
		Get_USB_Port_Folder(1);
		return 0;
	}
	else if (!strcmp(command, "Get_Usb2p0_Port2_Infor")) {
		Get_USB_Port_Info(2);
		return 0;
	}
	else if (!strcmp(command, "Get_Usb2p0_Port2_Folder")) {
		Get_USB_Port_Folder(2);
		return 0;
	}
	else if (!strcmp(command, "Get_SD_Infor")) {
		Get_SD_Card_Info();
		return 0;
	}
	else if (!strcmp(command, "Get_SD_Folder")) {
		Get_SD_Card_Folder();
		return 0;
	}
#endif
#ifdef EEPROM_BACKWARD_COMPATIBILITY
	else if (!strcmp(command, "Get_RegulationDomain")) {
		getCountryCode();
		return 0;
	}
#else
	else if (!strcmp(command, "Get_RegulationDomain_2G")) {
		getRegDomain(0);
		return 0;
	}
	else if (!strcmp(command, "Get_RegulationDomain_5G")) {
		getRegDomain(1);
		return 0;
	}
#endif
#if 0
	else if (!strcmp(command, "Get_Regrev_2G")) {
		getRegrev_2G();
		return 0;
	}
	else if (!strcmp(command, "Get_Regrev_5G")) {
		getRegrev_5G();
		return 0;
	}
#endif
	else if (!strcmp(command, "Get_SerialNumber")) {
		getSN();
		return 0;
	}
	else if (!strcmp(command, "Get_ModelName")) {
		getModelName();
		return 0;
	}
#ifndef EEPROM_BACKWARD_COMPATIBILITY
	else if (!strcmp(command, "Get_RegSpec")) {
		getRegSpec();
		return 0;
	}
#endif
	else if (!strcmp(command, "Get_PINCode")) {
		getPIN();
		return 0;
	}
	else if (!strcmp(command, "Get_WanLanStatus")) {
		if(!GetPhyStatus(ESW_WAN_PORT, 1))
			puts("ATE_ERROR");
		return 0;
	}
	else if (!strcmp(command, "Get_FwReadyStatus")) {
		puts(nvram_safe_get("success_start_service"));
		return 0;
	}
	else if (!strcmp(command, "Get_Build_Info")) {
		puts(nvram_safe_get("buildinfo"));
		return 0;
	}
	else if (!strcmp(command, "Get_RSSI_2G")) {
		getrssi(0);
		return 0;
	}
	else if (!strcmp(command, "Get_RSSI_5G")) {
		getrssi(1);
		return 0;
	}
	else if (!strcmp(command, "Get_ChannelList_2G")) {
		return getChannelList(0);
	}
	else if (!strcmp(command, "Get_ChannelList_5G")) {
		return getChannelList(1);
	}
#if 0
	else if (!strcmp(command, "Get_Usb3p0_Port1_Infor")) {
		puts("ATE_ERROR"); //Need to implement
		return 0;
	}
	else if (!strcmp(command, "Get_Usb3p0_Port2_Infor")) {
		puts("ATE_ERROR"); //Need to implement
		return 0;
	}
	else if (!strcmp(command, "Get_Usb3p0_Port3_Infor")) {
		puts("ATE_ERROR"); //Need to implement
		return 0;
	}
	else if (!strcmp(command, "Get_Usb3p0_Port1_Folder")) {
		puts("ATE_ERROR"); //Need to implement
		return EINVAL;
	}
	else if (!strcmp(command, "Get_Usb3p0_Port2_Folder")) {
		puts("ATE_ERROR"); //Need to implement
		return EINVAL;
	}
 	else if (!strcmp(command, "Get_Usb3p0_Port3_Folder")) {
		puts("ATE_ERROR"); //Need to implement
		return EINVAL;
	}
	else if (!strcmp(command, "Get_Usb3p0_Port1_DataRate")) {
		puts("ATE_ERROR"); //Need to implement
		return EINVAL;
	}
	else if (!strcmp(command, "Get_Usb3p0_Port2_DataRate")) {
		puts("ATE_ERROR"); //Need to implement
		return EINVAL;
	}
	else if (!strcmp(command, "Get_Usb3p0_Port3_DataRate")) {
		puts("ATE_ERROR"); //Need to implement
		return EINVAL;
	}
	else if (!strcmp(command, "Get_fail_ret")) {
		Get_fail_ret();
		return 0;
	}
	else if (!strcmp(command, "Get_fail_reboot_log")) {
		Get_fail_reboot_log();
		return 0;
	}
	else if (!strcmp(command, "Get_fail_dev_log")) {
		Get_fail_dev_log();
		return 0;
	}
#endif
	else if (!strcmp(command, "Ra_FWRITE")) {
		return FWRITE(value, value2);
	}
	else if (!strcmp(command, "Ra_Asuscfe_2G")) {
		return asuscfe(0, value);
	}
	else if (!strcmp(command, "Ra_Asuscfe_5G")) {
		return asuscfe(1, value);
	}
#if 0
	else if (!strcmp(command, "Get_WifiSwStatus")) {
		puts(nvram_safe_get("btn_wifi_sw"));
		return 0;
	}
#endif
	else if (!strcmp(command, "Get_ATEVersion")) {
		puts(nvram_safe_get("Ate_version"));
		return 0;
	}
	else if (!strcmp(command, "Get_XSetting")) {
		puts(nvram_safe_get("x_Setting"));
		return 0;
	}
#if 0
	else if (!strcmp(command, "Set_Eject")) {
		if( !Ej_device(value)) {
			puts("ATE_ERROR_INCORRECT_PARAMETER");
			return EINVAL;
		}
		return 0;
	}
#endif
	else {
		puts("ATE_UNSUPPORT");
		return EINVAL;
	}

	return 0;
}

#if 0
int ate_dev_status(void)
{
	int ret = 1, wl_band = 1;
	char wl_dev_name[12], dev_chk_buf[19], word[256], *next;

	sprintf(wl_dev_name, nvram_safe_get("wl_ifnames"));
	if (switch_exist())
		sprintf(dev_chk_buf, "switch=O");
	else {
		sprintf(dev_chk_buf, "switch=X");
#ifdef CONFIG_BCMWL5	//broadcom platform need to shift the interface name
		sprintf(wl_dev_name, "eth0 eth1");
#endif
		ret = 0;
	}

	foreach (word, wl_dev_name, next) {
		if (wl_exist(word, wl_band)) {
			if (wl_band == 1)
				sprintf(dev_chk_buf, "%s,2G=O", dev_chk_buf);
			else
				sprintf(dev_chk_buf, "%s,5G=O", dev_chk_buf);
		}
		else {
			if (wl_band == 1)
				sprintf(dev_chk_buf, "%s,2G=X", dev_chk_buf);
			else
				sprintf(dev_chk_buf, "%s,5G=X", dev_chk_buf);
			ret = 0;
		}
			wl_band++;
	}
	nvram_set("Ate_dev_status", dev_chk_buf);
	return ret;
}
#endif
#endif /* ASUS_ATE */
