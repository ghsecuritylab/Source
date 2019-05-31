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

#ifndef _ralink_h_
#define _ralink_h_

#define RTMP_RBUS_SUPPORT 1
#define MAX_NUMBER_OF_MAC	32

typedef union _MACHTTRANSMIT_SETTING {
	struct	{
		unsigned short MCS:7;		// MCS
		unsigned short BW:1;		// channel bandwidth 20MHz or 40 MHz
		unsigned short ShortGI:1;
		unsigned short STBC:2;		// SPACE
		unsigned short rsv:3;
		unsigned short MODE:2;		// Use definition MODE_xxx.
	}	field;
	unsigned short word;
} MACHTTRANSMIT_SETTING, *PMACHTTRANSMIT_SETTING;

typedef struct _RT_802_11_MAC_ENTRY {
	unsigned char		ApIdx;
	unsigned char		Addr[ETHER_ADDR_LEN];
	unsigned char		Aid;
	unsigned char		Psm;		// 0:PWR_ACTIVE, 1:PWR_SAVE
	unsigned char		MimoPs;		// 0:MMPS_STATIC, 1:MMPS_DYNAMIC, 3:MMPS_Enabled
	char			AvgRssi0;
	char			AvgRssi1;
	char			AvgRssi2;
	unsigned int		ConnectedTime;
	MACHTTRANSMIT_SETTING	TxRate;
#ifdef RTMP_RBUS_SUPPORT
	unsigned int		LastRxRate;
	short			StreamSnr[3];		// BF SNR from RXWI. Units=0.25 dB. 22 dB offset removed
	short			SoundingRespSnr[3];	// SNR from Sounding Response. Units=0.25 dB. 22 dB offset removed
	//SHORT			TxPER;			// TX PER over the last second. Percent
	//SHORT			reserved;
#endif /* RTMP_RBUS_SUPPORT */
} RT_802_11_MAC_ENTRY, *PRT_802_11_MAC_ENTRY;

typedef struct _RT_802_11_MAC_TABLE {
	unsigned long		Num;
	RT_802_11_MAC_ENTRY	Entry[MAX_NUMBER_OF_MAC];
} RT_802_11_MAC_TABLE, *PRT_802_11_MAC_TABLE;

#ifdef IEEE802_11AC
typedef union  _MACHTTRANSMIT_SETTING_11AC {
	struct  {
		unsigned short	MCS:7;	// MCS
		unsigned short	BW:2;	//channel bandwidth 20MHz or 40 MHz
		unsigned short	ShortGI:1;
		unsigned short	STBC:1;	//SPACE
		unsigned short	eTxBF:1;
		unsigned short	iTxBF:1;
		unsigned short	MODE:3;	// Use definition MODE_xxx.
	} field;
	unsigned short	word;
} MACHTTRANSMIT_SETTING_11AC, *PMACHTTRANSMIT_SETTING_11AC;

typedef struct _RT_802_11_MAC_ENTRY_11AC {
	unsigned char		ApIdx;
	unsigned char		Addr[ETHER_ADDR_LEN];
	unsigned char		Aid;
	unsigned char		Psm;	/* 0:PWR_ACTIVE, 1:PWR_SAVE */
	unsigned char		MimoPs;	/* 0:MMPS_STATIC, 1:MMPS_DYNAMIC, 3:MMPS_Enabled */
	char			AvgRssi0;
	char			AvgRssi1;
	char			AvgRssi2;
	unsigned int		ConnectedTime;
	MACHTTRANSMIT_SETTING_11AC	TxRate;
	unsigned int		LastRxRate;
	short			StreamSnr[3];	/* BF SNR from RXWI. Units=0.25 dB. 22 dB offset removed */
	short			SoundingRespSnr[3];
} RT_802_11_MAC_ENTRY_11AC, *PRT_802_11_MAC_ENTRY_11AC;
#endif

typedef struct _SITE_SURVEY
{
	char channel[4];
	unsigned char ssid[33];
	char bssid[18];
	char encryption[9];
	char authmode[16];
	char signal[9];
	char wmode[8];
	char bsstype[3];
	char centralchannel[4];
	char wps[5];
	char dpid[5];
	char newline;
} SITE_SURVEY;

typedef struct _SITE_SURVEY_ARRAY
{
	SITE_SURVEY SiteSurvey[64];
} SSA;

#define SITE_SURVEY_APS_MAX (16*1024)

#define RT_PRIV_IOCTL			(SIOCIWFIRSTPRIV + 0x01)
#define RTPRIV_IOCTL_SET		(SIOCIWFIRSTPRIV + 0x02)
#define RTPRIV_IOCTL_GSITESURVEY	(SIOCIWFIRSTPRIV + 0x0D)
#define RTPRIV_IOCTL_GET_MAC_TABLE	(SIOCIWFIRSTPRIV + 0x0F)
#define RTPRIV_IOCTL_SHOW		(SIOCIWFIRSTPRIV + 0x11)
#define RTPRIV_IOCTL_WSC_PROFILE	(SIOCIWFIRSTPRIV + 0x12)
#define RTPRIV_IOCTL_ASUSCMD		(SIOCIWFIRSTPRIV + 0x1E)

enum ASUS_IOCTL_SUBCMD {
	ASUS_SUBCMD_UNKNOWN = 0,
	ASUS_SUBCMD_GRSSI,
	ASUS_SUBCMD_CLIQ,
	ASUS_SUBCMD_CHLIST,
	ASUS_SUBCMD_GROAM,
	ASUS_SUBCMD_CONN_STATUS,
	ASUS_SUBCMD_GEXTCHA,
	ASUS_SUBCMD_MAX
};

#define OID_802_11_DISASSOCIATE		0x0114
#define OID_802_11_BSSID_LIST_SCAN	0x0508
#define OID_802_11_SSID			0x0509
#define OID_802_11_BSSID		0x050A
#define RT_OID_802_11_RADIO		0x050B
#define OID_802_11_BSSID_LIST		0x0609
#define OID_802_3_CURRENT_ADDRESS	0x060A
#define OID_GEN_MEDIA_CONNECT_STATUS	0x060B
#define RT_OID_GET_PHY_MODE		0x0761
#define OID_GET_SET_TOGGLE		0x8000
#define RT_OID_SYNC_RT61		0x0D010750
#define RT_OID_WSC_QUERY_STATUS		((RT_OID_SYNC_RT61 + 0x01) & 0xffff)
#define RT_OID_WSC_PIN_CODE		((RT_OID_SYNC_RT61 + 0x02) & 0xffff)

#ifdef DUAL_BAND_NONCONCURRENT
#define change_to_another_band(n, wif)	change_WirelessMode(!n, wif)
#define go_back_original_band(n, wif)	change_WirelessMode(n, wif)
#endif

#endif /* _ralink_h_ */
