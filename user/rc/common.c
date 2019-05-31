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

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>

#include <nvram/bcmnvram.h>
#include <asm/unistd.h>
#include <sys/syscall.h>

#include <shared.h>

#include "rc.h"
#include "version.h"
#include "rc_event.h"

_syscall2( int, track_flag, int *, flag, void *, data);

void track_set(int flag, char *data)
{
	void* tmp;
	ulong addr;
	char buf[64];

	if (flag == ASUS_HIJ_SET_TARGET_IP) {
		addr = inet_addr(data);
		tmp = &addr;
	}
	else if (flag == ASUS_HIJ_SET_TARGET_HOST 
			|| flag == ASUS_HIJ_SET_DUT_MAC) {
		tmp = (void *)data;
	}

	if (!track_flag(&flag, tmp))
		printf("[%s] ok: %d\n", __FUNCTION__, flag);
	else
		printf("[%s] fail: %d\n", __FUNCTION__, flag);
}

#ifdef EEPROM_BACKWARD_COMPATIBILITY
/* 
 * convert country code into reg domain
 */
static char *CountryCode2RegDomain(int n, const char *cc)
{
	char reg_domain[MAX_REGDOMAIN_LEN+1];

	memset(reg_domain, 0, sizeof(reg_domain));

	if (!n) {
		if (!strcasecmp(cc, "CA") 
				|| !strcasecmp(cc, "CO") 
				|| !strcasecmp(cc, "DO") 
				|| !strcasecmp(cc, "GT") 
				|| !strcasecmp(cc, "MX") 
				|| !strcasecmp(cc, "NO") 
				|| !strcasecmp(cc, "PA") 
				|| !strcasecmp(cc, "PR") 
				|| !strcasecmp(cc, "TW") 
				|| !strcasecmp(cc, "US") 
				|| !strcasecmp(cc, "UZ"))
			strcpy(reg_domain, "2G_CH11");
		else if (!strcasecmp(cc, "DB") 
				|| !strcasecmp(cc, ""))
			strcpy(reg_domain, "2G_CH14");
		else
			strcpy(reg_domain, "2G_CH13");
	}
	else {
		if (!strcasecmp(cc, "AL") 
				|| !strcasecmp(cc, "DZ") 
				|| !strcasecmp(cc, "AU") 
				|| !strcasecmp(cc, "BH") 
				|| !strcasecmp(cc, "BY") 
				|| !strcasecmp(cc, "CA") 
				|| !strcasecmp(cc, "CL") 
				|| !strcasecmp(cc, "CO") 
				|| !strcasecmp(cc, "CR") 
				|| !strcasecmp(cc, "DO") 
				|| !strcasecmp(cc, "EC") 
				|| !strcasecmp(cc, "SV") 
				|| !strcasecmp(cc, "GT") 
				|| !strcasecmp(cc, "HN") 
				|| !strcasecmp(cc, "HK") 
				|| !strcasecmp(cc, "IN") 
				|| !strcasecmp(cc, "IL") 
				|| !strcasecmp(cc, "JO") 
				|| !strcasecmp(cc, "KZ") 
				|| !strcasecmp(cc, "KW") 
				|| !strcasecmp(cc, "LB") 
				|| !strcasecmp(cc, "MO") 
				|| !strcasecmp(cc, "MK") 
				|| !strcasecmp(cc, "MY") 
				|| !strcasecmp(cc, "MX") 
				|| !strcasecmp(cc, "MA") 
				|| !strcasecmp(cc, "NZ") 
				|| !strcasecmp(cc, "NO") 
				|| !strcasecmp(cc, "OM") 
				|| !strcasecmp(cc, "PK") 
				|| !strcasecmp(cc, "PA") 
				|| !strcasecmp(cc, "PR") 
				|| !strcasecmp(cc, "QA") 
				|| !strcasecmp(cc, "RO") 
				|| !strcasecmp(cc, "SA") 
				|| !strcasecmp(cc, "SG") 
				|| !strcasecmp(cc, "SY") 
				|| !strcasecmp(cc, "TH") 
				|| !strcasecmp(cc, "UA") 
				|| !strcasecmp(cc, "AE") 
				|| !strcasecmp(cc, "VN") 
				|| !strcasecmp(cc, "YE") 
				|| !strcasecmp(cc, "ZW"))
			strcpy(reg_domain, "5G_BAND124");
		else if (!strcasecmp(cc, "AT") 
				|| !strcasecmp(cc, "BE") 
				|| !strcasecmp(cc, "BR") 
				|| !strcasecmp(cc, "BG") 
				|| !strcasecmp(cc, "CY") 
				|| !strcasecmp(cc, "DK") 
				|| !strcasecmp(cc, "EE") 
				|| !strcasecmp(cc, "FI") 
				|| !strcasecmp(cc, "DE") 
#ifdef IEEE802_11H
				|| !strcasecmp(cc, "GB") 
#endif
				|| !strcasecmp(cc, "GR") 
				|| !strcasecmp(cc, "HU") 
				|| !strcasecmp(cc, "IS") 
				|| !strcasecmp(cc, "IE") 
				|| !strcasecmp(cc, "IT") 
				|| !strcasecmp(cc, "LV") 
				|| !strcasecmp(cc, "LI") 
				|| !strcasecmp(cc, "LT") 
				|| !strcasecmp(cc, "LU") 
				|| !strcasecmp(cc, "NL") 
				|| !strcasecmp(cc, "PL") 
				|| !strcasecmp(cc, "PT") 
				|| !strcasecmp(cc, "SK") 
				|| !strcasecmp(cc, "SI") 
				|| !strcasecmp(cc, "ZA") 
				|| !strcasecmp(cc, "ES") 
				|| !strcasecmp(cc, "SE") 
				|| !strcasecmp(cc, "CH") 
				|| !strcasecmp(cc, "UZ"))
			strcpy(reg_domain, "5G_BAND123");
		else if (!strcasecmp(cc, "AM") 
				|| !strcasecmp(cc, "AZ") 
				|| !strcasecmp(cc, "HR") 
				|| !strcasecmp(cc, "CZ") 
				|| !strcasecmp(cc, "EG") 
				|| !strcasecmp(cc, "FR") 
				|| !strcasecmp(cc, "GE") 
				|| !strcasecmp(cc, "MC") 
				|| !strcasecmp(cc, "TT") 
				|| !strcasecmp(cc, "TN") 
				|| !strcasecmp(cc, "TR"))
			strcpy(reg_domain, "5G_BAND12");
		else if (!strcasecmp(cc, "AR"))
			strcpy(reg_domain, "5G_BAND24_");
		else if (!strcasecmp(cc, "BZ") 
				|| !strcasecmp(cc, "BO") 
				|| !strcasecmp(cc, "BN") 
				|| !strcasecmp(cc, "CN") 
				|| !strcasecmp(cc, "ID") 
				|| !strcasecmp(cc, "IR") 
				|| !strcasecmp(cc, "TW") 
				|| !strcasecmp(cc, "PE") 
				|| !strcasecmp(cc, "PH"))
			strcpy(reg_domain, "5G_BAND4");
		else if (!strcasecmp(cc, "KP") 
				|| !strcasecmp(cc, "KR") 
				|| !strcasecmp(cc, "UY") 
				|| !strcasecmp(cc, "VE"))
			strcpy(reg_domain, "5G_BAND4_");
#ifndef IEEE802_11H
		else if (!strcasecmp(cc, "GB"))
			strcpy(reg_domain, "5G_BAND1");
#endif
		else if (!strcasecmp(cc, "JP"))
			strcpy(reg_domain, "5G_ALL_");
		else if (!strcasecmp(cc, "RU") 
				|| !strcasecmp(cc, "US"))
			strcpy(reg_domain, "5G_BAND14");
		else//!strcasecmp(cc, "DB") || !strcasecmp(cc, "")
			strcpy(reg_domain, "5G_ALL");
	}

	return reg_domain;
}

const char *getDfsStandard(const char *cc)
{
	if (cc == NULL)
		;
	else if (!strcmp(cc, "US"))
		return "FCC";
	else if (!strcmp(cc, "JP"))
		return "JAP";
	// default
	return "CE";
}

/*
 * get/set CountryCode from/to factory partition
 */
int getCountryCode(void)
{
	char buf[3];
	int i;

	memset(buf, 0, sizeof(buf));
	FRead(buf, COUNTRY_EEPROM_ADDR, 2);
	for (i = 0; i < 2; ++i)
		printf("%c", buf[i]);
	printf("\n");
	return 0;
}

int setCountryCode(const char *cc)
{
	char CC[3];

	if (strlen(cc) != 2)
		return -1;
	/* Please refer to ISO3166 code list for other countries and can be found at
	 * http://www.iso.org/iso/en/prods-services/iso3166ma/02iso-3166-code-lists/list-en1.html#sz
	 */
	else if (!strcasecmp(cc, "DB")) ;
	else if (!strcasecmp(cc, "AL")) ;
	else if (!strcasecmp(cc, "DZ")) ;
	else if (!strcasecmp(cc, "AR")) ;
	else if (!strcasecmp(cc, "AM")) ;
	else if (!strcasecmp(cc, "AU")) ;
	else if (!strcasecmp(cc, "AT")) ;
	else if (!strcasecmp(cc, "AZ")) ;
	else if (!strcasecmp(cc, "BH")) ;
	else if (!strcasecmp(cc, "BY")) ;
	else if (!strcasecmp(cc, "BE")) ;
	else if (!strcasecmp(cc, "BZ")) ;
	else if (!strcasecmp(cc, "BO")) ;
	else if (!strcasecmp(cc, "BR")) ;
	else if (!strcasecmp(cc, "BN")) ;
	else if (!strcasecmp(cc, "BG")) ;
	else if (!strcasecmp(cc, "CA")) ;
	else if (!strcasecmp(cc, "CL")) ;
	else if (!strcasecmp(cc, "CN")) ;
	else if (!strcasecmp(cc, "CO")) ;
	else if (!strcasecmp(cc, "CR")) ;
	else if (!strcasecmp(cc, "HR")) ;
	else if (!strcasecmp(cc, "CY")) ;
	else if (!strcasecmp(cc, "CZ")) ;
	else if (!strcasecmp(cc, "DK")) ;
	else if (!strcasecmp(cc, "DO")) ;
	else if (!strcasecmp(cc, "EC")) ;
	else if (!strcasecmp(cc, "EG")) ;
	else if (!strcasecmp(cc, "SV")) ;
	else if (!strcasecmp(cc, "EE")) ;
	else if (!strcasecmp(cc, "FI")) ;
	else if (!strcasecmp(cc, "FR")) ;
	else if (!strcasecmp(cc, "GE")) ;
	else if (!strcasecmp(cc, "DE")) ;
	else if (!strcasecmp(cc, "GR")) ;
	else if (!strcasecmp(cc, "GT")) ;
	else if (!strcasecmp(cc, "HN")) ;
	else if (!strcasecmp(cc, "HK")) ;
	else if (!strcasecmp(cc, "HU")) ;
	else if (!strcasecmp(cc, "IS")) ;
	else if (!strcasecmp(cc, "IN")) ;
	else if (!strcasecmp(cc, "ID")) ;
	else if (!strcasecmp(cc, "IR")) ;
	else if (!strcasecmp(cc, "IE")) ;
	else if (!strcasecmp(cc, "IL")) ;
	else if (!strcasecmp(cc, "IT")) ;
	else if (!strcasecmp(cc, "JP")) ;
	else if (!strcasecmp(cc, "JO")) ;
	else if (!strcasecmp(cc, "KZ")) ;
	else if (!strcasecmp(cc, "KP")) ;
	else if (!strcasecmp(cc, "KR")) ;
	else if (!strcasecmp(cc, "KW")) ;
	else if (!strcasecmp(cc, "LV")) ;
	else if (!strcasecmp(cc, "LB")) ;
	else if (!strcasecmp(cc, "LI")) ;
	else if (!strcasecmp(cc, "LT")) ;
	else if (!strcasecmp(cc, "LU")) ;
	else if (!strcasecmp(cc, "MO")) ;
	else if (!strcasecmp(cc, "MK")) ;
	else if (!strcasecmp(cc, "MY")) ;
	else if (!strcasecmp(cc, "MX")) ;
	else if (!strcasecmp(cc, "MC")) ;
	else if (!strcasecmp(cc, "MA")) ;
	else if (!strcasecmp(cc, "NL")) ;
	else if (!strcasecmp(cc, "NZ")) ;
	else if (!strcasecmp(cc, "NO")) ;
	else if (!strcasecmp(cc, "OM")) ;
	else if (!strcasecmp(cc, "PK")) ;
	else if (!strcasecmp(cc, "PA")) ;
	else if (!strcasecmp(cc, "PE")) ;
	else if (!strcasecmp(cc, "PH")) ;
	else if (!strcasecmp(cc, "PL")) ;
	else if (!strcasecmp(cc, "PT")) ;
	else if (!strcasecmp(cc, "PR")) ;
	else if (!strcasecmp(cc, "QA")) ;
	else if (!strcasecmp(cc, "RO")) ;
	else if (!strcasecmp(cc, "RU")) ;
	else if (!strcasecmp(cc, "SA")) ;
	else if (!strcasecmp(cc, "SG")) ;
	else if (!strcasecmp(cc, "SK")) ;
	else if (!strcasecmp(cc, "SI")) ;
	else if (!strcasecmp(cc, "ZA")) ;
	else if (!strcasecmp(cc, "ES")) ;
	else if (!strcasecmp(cc, "SE")) ;
	else if (!strcasecmp(cc, "CH")) ;
	else if (!strcasecmp(cc, "SY")) ;
	else if (!strcasecmp(cc, "TW")) ;
	else if (!strcasecmp(cc, "TH")) ;
	else if (!strcasecmp(cc, "TT")) ;
	else if (!strcasecmp(cc, "TN")) ;
	else if (!strcasecmp(cc, "TR")) ;
	else if (!strcasecmp(cc, "UA")) ;
	else if (!strcasecmp(cc, "AE")) ;
	else if (!strcasecmp(cc, "GB")) ;
	else if (!strcasecmp(cc, "US")) ;
	else if (!strcasecmp(cc, "UY")) ;
	else if (!strcasecmp(cc, "UZ")) ;
	else if (!strcasecmp(cc, "VE")) ;
	else if (!strcasecmp(cc, "VN")) ;
	else if (!strcasecmp(cc, "YE")) ;
	else if (!strcasecmp(cc, "ZW")) ;
	else
		return -1;

	memset(&CC[0], toupper(cc[0]), 1);
	memset(&CC[1], toupper(cc[1]), 1);
	memset(&CC[2], 0, 1);
	FWrite(CC, COUNTRY_EEPROM_ADDR, 2);
	return 0;
}
#else
/*
 * get/set RegSpec from/to factory partition
 */
int getRegSpec(void)
{
	char buf[MAX_REGSPEC_LEN+1];
	int i;

	memset(buf, 0, sizeof(buf));
	FRead(buf, REGSPEC_ADDR, MAX_REGSPEC_LEN);

	for (i = 0; i < MAX_REGSPEC_LEN; ++i) {
		if ((buf[i] == (char)0xFF) || (buf[i] == '\0'))
			break;
		printf("%c", buf[i]);
	}
	printf("\n");

	return 0;
}

int setRegSpec(const char *cc)
{
	char REGSPEC[MAX_REGSPEC_LEN+1];
	int i;

	if (!strcasecmp(cc, "CE")) ;
	else if (!strcasecmp(cc, "DB")) ;
	else if (!strcasecmp(cc, "FCC")) ;
	else if (!strcasecmp(cc, "APAC")) ;
	else if (!strcasecmp(cc, "AU")) ;
	else if (!strcasecmp(cc, "JP")) ;
	else if (!strcasecmp(cc, "NCC")) ;
	else if (!strcasecmp(cc, "UA")) ;
	else if (!strcasecmp(cc, "IC")) ;
	else
		return -1;

	memset(REGSPEC, 0x0, sizeof(REGSPEC));
	for (i = 0; cc[i] != '\0'; i++)
		REGSPEC[i] = (char)toupper(cc[i]);

	FWrite(REGSPEC, REGSPEC_ADDR, MAX_REGSPEC_LEN);

	return 0;
}

/*
 * get/set RegDomain from/to factory partition
 */
int getRegDomain(int n)
{
	char buf[MAX_REGDOMAIN_LEN+1];
	int i;
	int rd_eeprom_addr[2] = {REG2G_EEPROM_ADDR, REG5G_EEPROM_ADDR};

	memset(buf, 0, sizeof(buf));
	FRead(buf, rd_eeprom_addr[n], MAX_REGDOMAIN_LEN);

	for (i = 0; i < MAX_REGDOMAIN_LEN; ++i) {
		if ((buf[i] == (char)0xFF) || (buf[i] == '\0'))
			break;
		printf("%c", buf[i]);
	}
	printf("\n");

	return 0;
}

int setRegDomain(int n, const char *cc)
{
	char CC[MAX_REGDOMAIN_LEN+1];
	int i;
	int rd_eeprom_addr[2] = {REG2G_EEPROM_ADDR, REG5G_EEPROM_ADDR};

	if (!n) {
		if (!strcasecmp(cc, "2G_CH11")) ;
		else if (!strcasecmp(cc, "2G_CH13")) ;
		else if (!strcasecmp(cc, "2G_CH14")) ;
		else
			return -1;
	}
	else {
		if (!strcasecmp(cc, "5G_ALL")) ;
		else if (!strcasecmp(cc, "5G_BAND14")) ;
		else if (!strcasecmp(cc, "5G_BAND24")) ;
		else if (!strcasecmp(cc, "5G_BAND1")) ;
		else if (!strcasecmp(cc, "5G_BAND4")) ;
		else if (!strcasecmp(cc, "5G_BAND123")) ;
		else
			return -1;
	}

	memset(CC, 0x0, sizeof(CC));
	strcpy(CC, cc);
	for (i = 0; CC[i] != '\0'; i++)
		CC[i] = (char)toupper(CC[i]);

	FWrite(CC, rd_eeprom_addr[n], MAX_REGDOMAIN_LEN);

	return 0;
}
#endif /* EEPROM_BACKWARD_COMPATIBILITY */

/*
 * get/set MAC from/to factory partition
 */
int getMAC(int n)
{
	char buf[8];
	int mac_eeprom_addr[2] = {MAC_EEPROM_ADDR, MAC_EEPROM_ADDR_5G};

	memset(buf, 0, sizeof(buf));
	FRead(buf, mac_eeprom_addr[n], 6);

	printf("%02x:%02x:%02x:%02x:%02x:%02x\n",
			(unsigned char)buf[0],
			(unsigned char)buf[1],
			(unsigned char)buf[2],
			(unsigned char)buf[3],
			(unsigned char)buf[4],
			(unsigned char)buf[5]);

	return 0;
}

int setMAC(int n, const char *mac)
{
	char ea[ETHER_ADDR_LEN];
	int mac_eeprom_addr[2] = {MAC_EEPROM_ADDR, MAC_EEPROM_ADDR_5G};

	if (mac == NULL || !isValidMacAddr(mac))
		return -1;

	if (ether_atoe(mac, ea))
		FWrite(ea, mac_eeprom_addr[n], ETHER_ADDR_LEN);

	return 0;
}

#ifdef ASUS_ATE
/*
 * get/set serial number from/to factory partition
 */
int getSN(void)
{
	char buf[SERIAL_NUMBER_LENGTH+1];
	int i;

	if (FRead(buf, SERIAL_NUMBE_ADDR, SERIAL_NUMBER_LENGTH) < 0)
		fprintf(stderr, "READ Serial Number: Out of scope\n");
	else {
		for(i = 0; i < SERIAL_NUMBER_LENGTH; ++i)
			printf("%c", buf[i]);
		printf("\n");
	}
	return 1;
}

int setSN(const char *SN)
{
	if (SN == NULL || !isValidSN(SN))
		return 0;

	if (FWrite(SN, SERIAL_NUMBE_ADDR, SERIAL_NUMBER_LENGTH) < 0)
		return 0;

	getSN();
	return 1;
}

/*
 * get/set PIN code from/to factory partition
 */
int getPIN(void)
{
	char buf[9];
	int i;

	memset(buf, 0, sizeof(buf));
	FRead(buf, PINCODE_EEPROM_ADDR, 8);

        for(i = 0; i < 8; ++i)
                printf("%c", buf[i]);
        printf("\n");
	return 0;
}

int setPIN(const char *pin)
{
	if (pincheck(pin))
		FWrite(pin, PINCODE_EEPROM_ADDR, 8);
	else
		return -1;
	return 0;
}

/*
 * get/set bootloader version from/to factory partition
 */
int getBootVer(void)
{
	char buf[5];

	memset(buf, 0, sizeof(buf));
#ifdef EEPROM_BACKWARD_COMPATIBILITY
	FRead(buf, BOOTVER_EEPROM_ADDR, 2);

	printf("%s-", MODEL_NAME);
	printf("%c%c\n", 
			buf[0], 
			buf[1]);
#else
	FRead(buf, BOOTVER_EEPROM_ADDR, 4);

	printf("%s-", MODEL_NAME);
	printf("%c.%c.%c.%c\n", 
			buf[0], 
			buf[1], 
			buf[2], 
			buf[3]);
#endif

	return 0;
}

#ifdef EEPROM_BACKWARD_COMPATIBILITY
int setBootVer(const char *bootv)
{
	char btv[2];
	memcpy(btv, bootv, 2);
	FWrite(btv, BOOTVER_EEPROM_ADDR, 2);
	return 0;
}
#endif

/*
 * get/set model name from/to factory partition
 */
int getModelName(void)
{
	unsigned char buf[13];
	int i;

	buf[12] = '\0';
	if (FRead(buf, MODEL_NAME_ADDR, 12) < 0)
		return -1;
	for (i = 11; i >= 0; i--) {
		if (buf[i] == 0xff)
			buf[i] = '\0';
		else
			break;
	}
	printf("%s\n", buf);

	return 0;
}

void setModelName(char *name)
{
	unsigned char buf[13];
	int i;

	strncpy(buf, name, 13);
	buf[12] = '\0';
	for (i = strlen(buf)+1; i < 12; i++)
		buf[i] = 0xff;

	FWrite(buf, MODEL_NAME_ADDR, 12);
}
#endif /* ASUS_ATE */

/*
 * convert IP address format into binary data
 */
in_addr_t inet_addr_(const char *cp)
{
	struct in_addr a;

	if (!inet_aton(cp, &a))
		return INADDR_ANY;
	else
		return a.s_addr;
}

/*
 * convert mac address format from XXXXXXXXXXXX to XX:XX:XX:XX:XX:XX
*/
char *mac_conv(char *mac_name, int idx, char *buf)
{
	char *mac, name[32];
	int i, j = 0;

	if (idx != -1)
		sprintf(name, "%s%d", mac_name, idx);
	else
		sprintf(name, "%s", mac_name);

	mac = nvram_safe_get(name);

	if (strlen(mac) == 0)
		buf[0] = 0;
	else {
		for(i = 0; i < 12; i++) {
			if (i != 0 && i%2 == 0)
				buf[j++] = ':';
			buf[j++] = mac[i];
		}
		buf[j] = 0;
	}

	dprintf("mac: %s\n", buf);
	return (buf);
}

static int valid_subver(char subfs)
{
	if (((subfs >= 'a') && (subfs <= 'z' )) 
			|| ((subfs >= 'A') && (subfs <= 'Z' )))
		return 1;
	else
		return 0;
}

/*
 * get system parameter from factory partition
 */
void getsyspara(void)
{
	int i, n;
	char wif[8], *next;
	char tmp[128], prefix[] = "wlXXXXXXXXXX_";
	unsigned char buf[32];

#ifdef EEPROM_BACKWARD_COMPATIBILITY
	// country code
	char country_code[3];

	memset(country_code, 0, sizeof(country_code));

	country_code[2] = '\0';
	if (FRead(country_code, COUNTRY_EEPROM_ADDR, 2) < 0) {
		fprintf(stderr, "READ ASUS country code: Out of scope\n");
		nvram_set("wl_country_code", "");
	}
	else {
		if ((unsigned char)country_code[0] != 0xff && country_code[0] != 0)
			nvram_set("wl_country_code", country_code);
		else
			nvram_set("wl_country_code", "DB");
	}
#endif

	// MAC
	char macaddr[]="00:11:22:33:44:50";
	int mac_eeprom_addr[2] = {MAC_EEPROM_ADDR, MAC_EEPROM_ADDR_5G};
	//reg domain
	char reg_domain[MAX_REGDOMAIN_LEN+1];
#ifndef EEPROM_BACKWARD_COMPATIBILITY
	int rd_eeprom_addr[2] = {REG2G_EEPROM_ADDR, REG5G_EEPROM_ADDR};
#endif

	memset(reg_domain, 0, sizeof(reg_domain));

	for1each(n, wif, nvram_safe_get("wl_ifnames"), next) {
		memset(buf, 0, sizeof(buf));

		// MAC
		FRead(buf, mac_eeprom_addr[n], 6);
		ether_etoa(buf, macaddr);
		snprintf(prefix, sizeof(prefix), "wl%d_", n);
		nvram_set(strcat_r(prefix, "macaddr", tmp), macaddr);

#ifdef SWMODE_REPEATER_V2_SUPPORT
		// SSID
		sprintf(buf, "ASUS %s.%c%c%c%c", MODEL_NAME, macaddr[12], macaddr[13], macaddr[15], macaddr[16]);
		nvram_set(strcat_r(prefix, "ssid", tmp), buf);
#endif

		// reg domain
#ifdef EEPROM_BACKWARD_COMPATIBILITY
#ifdef DUAL_BAND_NONCONCURRENT
		if ((unsigned char)country_code[0] != 0xff && country_code[0] != 0) {
			strcpy(reg_domain, CountryCode2RegDomain(0, country_code));
			fprintf(stderr, "** CountryCode[%s] convert to RegDomain[%s] **\n", country_code, reg_domain);
			nvram_set("wl0_reg", reg_domain);
			strcpy(reg_domain, CountryCode2RegDomain(1, country_code));
			fprintf(stderr, "** CountryCode[%s] convert to RegDomain[%s] **\n", country_code, reg_domain);
			nvram_set("wl1_reg", reg_domain);
		}
		else {
			nvram_set("wl0_reg", "2G_CH14");
			nvram_set("wl1_reg", "5G_ALL");
		}

#else
		if (strcmp(country_code, "")) {
			strcpy(reg_domain, CountryCode2RegDomain(n, country_code));
			fprintf(stderr, "** CountryCode[%s] convert to RegDomain[%s] **\n", country_code, reg_domain);
			nvram_set(strcat_r(prefix, "reg", tmp), reg_domain);
		}
		else {
			if (n)
				nvram_set(strcat_r(prefix, "reg", tmp), "5G_ALL");
			else
				nvram_set(strcat_r(prefix, "reg", tmp), "2G_CH14");
		}
#endif
#else
		reg_domain[MAX_REGDOMAIN_LEN] = '\0';
		if (FRead(reg_domain, rd_eeprom_addr[n], MAX_REGDOMAIN_LEN) < 0) {
			fprintf(stderr, "READ ASUS reg domain: Out of scope\n");
			if (n)
				nvram_set(strcat_r(prefix, "reg", tmp), "5G_ALL");	// DEFAULT
			else
				nvram_set(strcat_r(prefix, "reg", tmp), "2G_CH14");	// DEFAULT
		}
		else {
			for (i=(MAX_REGDOMAIN_LEN-1) ; i>=0 ; i--) {
				if ((reg_domain[i] == (char)0xff) || (reg_domain[i] == '\0'))
				reg_domain[i] = '\0';
			}
			if ((unsigned char)reg_domain[0] != 0x00)
				nvram_set(strcat_r(prefix, "reg", tmp), reg_domain);
			else {
				if (n)
					nvram_set(strcat_r(prefix, "reg", tmp), "5G_ALL");	// DEFAULT
				else
					nvram_set(strcat_r(prefix, "reg", tmp), "2G_CH14");	// DEFAULT
			}
		}
#endif
	}

#ifndef EEPROM_BACKWARD_COMPATIBILITY
	// reg_spec
	char reg_spec[MAX_REGSPEC_LEN+1];

	memset(reg_spec, 0, sizeof(reg_spec));

	reg_spec[MAX_REGSPEC_LEN] = '\0';
	if (FRead(reg_spec, REGSPEC_ADDR, MAX_REGSPEC_LEN) < 0)
		nvram_set("reg_spec", "CE");	// DEFAULT
	else {
		for (i=(MAX_REGSPEC_LEN-1) ; i>=0 ; i--) {
			if ((reg_spec[i] == (char)0xff) || (reg_spec[i] == '\0'))
				reg_spec[i] = '\0';
		}
		if ((unsigned char)reg_spec[0] != 0x00) {
#if defined(CONFIG_DEFAULTS_ASUS_RPAC52)
			if (strcmp(reg_spec, "APAC")==0) { // APAC use the same power SPEC of CE 
				strcpy(reg_spec,"CE");
			}
#endif
			nvram_set("reg_spec", reg_spec);
		}
		else
			nvram_set("reg_spec", "CE");	// DEFAULT
	}
#endif

	// model name
	unsigned char fac_model_name[13];

	memset(fac_model_name, 0, sizeof(fac_model_name));

	if (FRead(fac_model_name, MODEL_NAME_ADDR, 12) < 0) {
		fprintf(stderr, "READ Factory Model Name: Out of scope\n");
		fac_model_name[0] = 0xff;
	}
	else {
		for (i = 11 ; i >= 1 ; i--) {
			if ((unsigned char)fac_model_name[i] == 0xff)
				fac_model_name[i] = '\0';
			else
				break;
		}
	}

	// PIN code
	char pin[9];

	memset(pin, 0, sizeof(pin));

	if (FRead(pin, PINCODE_EEPROM_ADDR, 8) < 0) {
		fprintf(stderr, "READ ASUS pin code: Out of scope\n");
		nvram_set("secret_code", "12345670");
	}
	else {
		if ((unsigned char)pin[0] != 0xff)
			nvram_set("secret_code", pin);
		else
			nvram_set("secret_code", "12345670");
	}

	// firmware version
	char productid[13];
	char fwver_sub[16];

	memset(buf, 0, sizeof(buf));
	memset(productid, 0, sizeof(productid));
	memset(fwver_sub, 0, sizeof(fwver_sub));

	if (FRead(buf, 0x50020, sizeof(buf)) < 0) {	/* /dev/mtd3, firmware, starts from 0x50000 */
		fprintf(stderr, "READ firmware header: Out of scope\n");
		nvram_set("productid", "unknown");
		nvram_set("firmver", "unknown");
	}
	else {
		//fprintf(stderr, "*** firmware header ***\n%s\n***********************\n", buf);
		if ((unsigned char)buf[0] != 0xff)
			strncpy(productid, buf + 4, 12);
		productid[12] = 0;
		nvram_set("productid", trim_r(productid));

		if (valid_subver(buf[27]))
			sprintf(fwver_sub, "%d.%d.%d.%d%c", buf[0], buf[1], buf[2], buf[3], buf[27]);
		else
			sprintf(fwver_sub, "%d.%d.%d.%d",buf[0], buf[1], buf[2], buf[3]);
		nvram_set("firmver", trim_r(fwver_sub));
        }

	if (((unsigned char)fac_model_name[0] == 0xff) || (fac_model_name[0] == '\0'))
		strcpy(fac_model_name, nvram_get("productid"));
	nvram_set("fac_model_name", fac_model_name);
	nvram_set("machine_name", fac_model_name);
}


#if (defined(AUDIO_SUPPORT) && defined(USB_TO_CM6510))
/*
 * get CM6510 firmware version
 */
static void get_cm6510_firmver(void)
{
	FILE *fp;
	char tmp[64], *ptr;
	int len;

	fp = popen("cat /proc/bus/usb/devices | grep Vendor=0b05", "r");
	if (fp) {
		len = fread(tmp, 1, sizeof(tmp), fp);
		pclose(fp);
		if (len > 0) {
			tmp[len-1] = '\0';
			ptr = strstr(tmp, "Rev= ");
			if (ptr) {
				ptr += strlen("Rev= ");
				fprintf(stderr, "CM6510 firmware version: %s\n", ptr);
				nvram_set("cm6510_firmver", ptr);
			}
		}
	}
}
#endif

static int gettzoffset(char *tzstr, char *tzstr1, int size1)
{
	char offstr[32];
	char *tzptr = tzstr;
	char *offptr = offstr;
	int ret = 0;
	int dst = 0;

	memset(offstr, 0, sizeof(offstr));
	for ( ; *tzptr; tzptr++) {
		if (*tzptr == '-' || *tzptr == '+' || *tzptr == ':' || isdigit(*tzptr)) {
			*offptr++ = *tzptr;
			ret = 1;
		}
		else if (ret) {
			dst = isalpha(*tzptr);
			break;
		}
	}

	if (ret)
		snprintf(tzstr1, size1, "GMT%s%s", offstr, dst ? "DST" : "");

	return ret;
}

void time_zone_x_mapping(void)
{
	char buf[64];
	char *ptr;

	/* NTP server */
	memset(buf, 0, sizeof(buf));
	if (nvram_invmatch("ntp_server0", ""))
		sprintf(buf, "%s%s ", buf, nvram_safe_get("ntp_server0"));
	if (nvram_invmatch("ntp_server1", ""))
		sprintf(buf, "%s%s ", buf, nvram_safe_get("ntp_server1"));
	nvram_set("ntp_servers", buf);

	/* time zone */
	memset(buf, 0, sizeof(buf));
	strcpy(buf, nvram_safe_get("time_zone"));
	/* replace . with : */
	if ((ptr=strchr(buf, '.'))!=NULL) *ptr = ':';
	/* remove *_? */
	if ((ptr=strchr(buf, '_'))!=NULL) *ptr = 0x0;

	if (nvram_match("time_zone_dst", "1"))
		sprintf(buf, "%s,%s", buf, nvram_safe_get("time_zone_dstoff"));
	else
		gettzoffset(buf, buf, sizeof(buf));
	nvram_set("time_zone_x", buf);
}

/*
 * QIS delay bridge mechanism
*/
unsigned long get_uptime(void);
void enable_QIS_delay_bridge(void)
{
	if (nvram_match("qis_apply", "1")) {
		unsigned long uptime=get_uptime();
		char buf[30];
		sprintf(buf, "%lu", uptime);
		nvram_set("dnsqmode_1st", buf);

		nvram_set("qis_state", "0");
	}
}

/*
 * initialize WAN relative nvram
 */
void init_wan_nvram(void)
{
	char *wan_proto = nvram_safe_get("wan_proto");

	if (!strcmp(wan_proto, "dhcp") 
			|| ((!strcmp(wan_proto, "pppoe") 
					|| !strcmp(wan_proto, "pptp") 
					|| !strcmp(wan_proto, "l2tp")) 
				&& nvram_match("wan_dhcpenable_x", "1"))) {
		nvram_set("wan_ipaddr", "");
		nvram_set("wan_netmask", "");
		nvram_set("wan_gateway", "");
	}

	if (!strcmp(wan_proto, "pppoe") 
			|| !strcmp(wan_proto, "pptp") 
			|| !strcmp(wan_proto, "l2tp"))
		nvram_set("upnp_wan_proto", "pppoe");
	else
		nvram_unset("upnp_wan_proto");

	nvram_set("wan_pppoe_ipaddr", "");
	nvram_set("wan_pppoe_netmask", "");
	nvram_set("wan_pppoe_gateway", "");
}

/*
 * initialize common nvram during the boot
 */
void init_common_nvram(void)
{
	/* build information for ATE */
	nvram_set("buildinfo", RT_BUILD_INFO);

#if (defined(AUDIO_SUPPORT) && defined(USB_TO_CM6510))
	/* get CM6510 firmware version */
	get_cm6510_firmver();
#endif

	/* DHCP server */
	nvram_set("dnsqmode_1st", "0");
	if (nvram_match("dhcp_enable_x", "1") || nvram_match("x_Setting", "0"))
		nvram_set("dnsqmode", "2");
	else
		nvram_set("dnsqmode", "1");

	/* LAN Section */
	set_unknown_domain_ip(nvram_safe_get("lan_ipaddr"), nvram_safe_get("lan_netmask"));

	if (nvram_match("lan_hostname", ""))
		nvram_set("lan_hostname", nvram_safe_get("fac_model_name"));

	enable_QIS_delay_bridge();

	/* time zone */
	time_zone_x_mapping();

	/* WAN Section */
	init_wan_nvram();

	convert_routes();

	if (nvram_match("wsc_config_state", "1"))
		nvram_set("x_Setting", "1");

	nvram_unset("ateCommand_flag");
}

/*
 * initialize operation mode relative nvram during the boot
 */
void init_nvram_by_mode(int _sw_mode)
{
	if (_sw_mode == 1) {			// Router mode
		nvram_set("wan_route_x", "1");
		nvram_set("wan_ifname", "vlan2");
	}
	else if (_sw_mode == 2) {		// Repeater mode
		nvram_set("wan_route_x", "0");
		nvram_set("wan_ifname", "");

		/* force turn radio on */
		nvram_set("wl0_radio_x", "1");
		nvram_set("wl1_radio_x", "1");

#ifdef SWMODE_REPEATER_EXPRESSWAY_SUPPORT
		nvram_set("wl_unit", nvram_match("re_expressway", "2")?"1":"0");
#endif
	}
#ifdef SWMODE_ADAPTER_SUPPORT
	else if (_sw_mode == 4) {		// Adapter mode
		nvram_set("wan_route_x", "0");
		nvram_set("wan_ifname", "");
	}
#endif
#ifdef SWMODE_HOTSPOT_SUPPORT
	else if (_sw_mode == 5) {		// Hotspot mode
		nvram_set("wan_route_x", "1");
		nvram_set("wan_ifname", "");

		//nvram_set("wan_proto", "dhcp");
	}
#endif
#ifdef USB_SUPPORT
	else if (_sw_mode == 6) {		// 3G mode
		ram_set("wan_route_x", "1");
		nvram_set("wan_ifname", "ppp0");

		nvram_set("wan_proto", "3g");
	}
#endif
	else {					// AP mode
		nvram_set("wan_route_x", "0");
		nvram_set("wan_ifname", "");
	}
}

/*
 * update LAN nvram: lan_gateway, lan_subnet_t
 */
void update_lan_status(void)
{
	fprintf(stderr, "%s\n", __FUNCTION__);

	if (nvram_match("dhcp_enable_x", "1")) {
		if (nvram_invmatch("dhcp_gateway_x", ""))
			nvram_set("lan_gateway", nvram_safe_get("dhcp_gateway_x"));
		else
			nvram_set("lan_gateway", nvram_safe_get("lan_ipaddr"));
	}
	else
		nvram_set("lan_gateway", nvram_safe_get("lan_ipaddr"));

	/* find out LAN subnet */
	char lan_ipaddr[16], lan_netmask[16], lan_subnet[11];

	memset(lan_ipaddr, 0, 16);
	strcpy(lan_ipaddr, nvram_safe_get("lan_ipaddr"));
	memset(lan_netmask, 0, 16);
	strcpy(lan_netmask, nvram_safe_get("lan_netmask"));
	memset(lan_subnet, 0, 11);
	sprintf(lan_subnet, "0x%x", inet_network(lan_ipaddr)&inet_network(lan_netmask));
	nvram_set("lan_subnet_t", lan_subnet);
}

/*
 * update WAN nvram: wan_subnet_t, wan_dns_t
 */
void update_wan_status(int isup)
{
	fprintf(stderr, "%s\n", __FUNCTION__);

	if (!isup) {
		nvram_set("wan_dns_t", "");
	}
	else {
		/* find out WAN subnet */
		char wan_gateway[16], wan_ipaddr[16], wan_netmask[16], wan_subnet[11];
		char dns_str[36];

		memset(dns_str, 0, sizeof(dns_str));
		memset(wan_gateway, 0, 16);
		strcpy(wan_gateway, nvram_safe_get("wan_gateway"));
		memset(wan_ipaddr, 0, 16);
		strcpy(wan_ipaddr, nvram_safe_get("wan_ipaddr"));
		memset(wan_netmask, 0, 16);
		strcpy(wan_netmask, nvram_safe_get("wan_netmask"));
		memset(wan_subnet, 0, 11);
		sprintf(wan_subnet, "0x%x", inet_network(wan_ipaddr)&inet_network(wan_netmask));
		nvram_set("wan_subnet_t", wan_subnet);

		if (nvram_invmatch("wan_dnsenable_x", "1")) {
			if (nvram_invmatch("wan_dns1_x",""))
				sprintf(dns_str, "%s", nvram_safe_get("wan_dns1_x"));
			if (nvram_invmatch("wan_dns2_x",""))
				sprintf(dns_str, " %s", nvram_safe_get("wan_dns2_x"));
			nvram_set("wan_dns_t", dns_str);
		}
		else
			nvram_set("wan_dns_t", nvram_safe_get("wan_dns"));
	}
}

#if (defined(AUDIO_SUPPORT) && defined(USB_TO_CM6510))
void init_audio_setting(void)
{
	/* volume: 0~100% convert to 0~62, and round off */
	int volume = (atoi(nvram_safe_get("audio_volume")) * 62 + 50) / 100;

	doSystem("amixer -q cset numid=4 %d", volume);
	doSystem("i2ctrl e %s", nvram_safe_get("eq_cur_idx"));
}
#endif
