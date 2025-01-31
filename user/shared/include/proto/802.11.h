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
 * Copyright 2001-2003, ASUSTek Inc.   
 * All Rights Reserved.   
 *    
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY   
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM   
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS   
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.   
 *
 * Fundamental types and constants relating to 802.11 
 *
 * $Id: 802.11.h,v 1.1 2007/06/08 10:20:42 arthur Exp $
 */

#ifndef _802_11_H_
#define _802_11_H_

#ifndef _TYPEDEFS_H_
#include <nvram/typedefs.h>
#endif

#ifndef _NET_ETHERNET_H_
#include <net/ethernet.h>
#endif

/* enable structure packing */
#if !defined(__GNUC__)
#pragma pack(1)
#endif

/* some platforms require stronger medicine */
#if defined(__GNUC__)
#define	PACKED	__attribute__((packed))
#else
#define	PACKED
#endif


#define DOT11_TU_TO_US			1024	/* 802.11 Time Unit is 1024 microseconds */

/* Generic 802.11 frame constants */
#define DOT11_A3_HDR_LEN		24
#define DOT11_A4_HDR_LEN		30
#define DOT11_MAC_HDR_LEN		DOT11_A3_HDR_LEN
#define DOT11_FCS_LEN			4
#define DOT11_ICV_LEN			4
#define DOT11_ICV_AES_LEN		8


#define DOT11_KEY_INDEX_SHIFT		6
#define DOT11_IV_LEN			4
#define DOT11_IV_TKIP_LEN		8
#define DOT11_IV_AES_OCB_LEN		4
#define DOT11_IV_AES_CCM_LEN		8

#define DOT11_MAX_MPDU_BODY_LEN		2312
#define DOT11_MAX_MPDU_LEN		2346	/* body len + A4 hdr + FCS */
#define DOT11_MAX_SSID_LEN		32

/* dot11RTSThreshold */
#define DOT11_DEFAULT_RTS_LEN		2347
#define DOT11_MAX_RTS_LEN		2347

/* dot11FragmentationThreshold */
#define DOT11_MIN_FRAG_LEN		256
#define DOT11_MAX_FRAG_LEN		2346	/* Max frag is also limited by aMPDUMaxLength of the attached PHY */
#define DOT11_DEFAULT_FRAG_LEN		2346

/* dot11BeaconPeriod */
#define DOT11_MIN_BEACON_PERIOD		1
#define DOT11_MAX_BEACON_PERIOD		0xFFFF

/* dot11DTIMPeriod */
#define DOT11_MIN_DTIM_PERIOD		1
#define DOT11_MAX_DTIM_PERIOD		0xFF

/* 802.2 LLC/SNAP header used by 802.11 per 802.1H */
#define DOT11_LLC_SNAP_HDR_LEN	8
#define DOT11_OUI_LEN			3
struct dot11_llc_snap_header {
	uint8	dsap;				/* always 0xAA */
	uint8	ssap;				/* always 0xAA */
	uint8	ctl;				/* always 0x03 */
	uint8	oui[DOT11_OUI_LEN];		/* RFC1042: 0x00 0x00 0x00
						   Bridge-Tunnel: 0x00 0x00 0xF8 */
	uint16	type;				/* ethertype */
} PACKED;

/* RFC1042 header used by 802.11 per 802.1H */
#define RFC1042_HDR_LEN			(ETHER_HDR_LEN + DOT11_LLC_SNAP_HDR_LEN)

/* Generic 802.11 MAC header */
/*
 * N.B.: This struct reflects the full 4 address 802.11 MAC header.
 *		 The fields are defined such that the shorter 1, 2, and 3
 *		 address headers just use the first k fields.
 */
struct dot11_header {
	uint16			fc;		/* frame control */
	uint16			durid;		/* duration/ID */
	struct ether_addr	a1;		/* address 1 */
	struct ether_addr	a2;		/* address 2 */
	struct ether_addr	a3;		/* address 3 */
	uint16			seq;		/* sequence control */
	struct ether_addr	a4;		/* address 4 */
} PACKED;

/* Control frames */

struct dot11_rts_frame {
	uint16			fc;		/* frame control */
	uint16			durid;		/* duration/ID */
	struct ether_addr	ra;		/* receiver address */
	struct ether_addr	ta;		/* transmitter address */
} PACKED;
#define	DOT11_RTS_LEN		16

struct dot11_cts_frame {
	uint16			fc;		/* frame control */
	uint16			durid;		/* duration/ID */
	struct ether_addr	ra;		/* receiver address */
} PACKED;
#define	DOT11_CTS_LEN		10

struct dot11_ack_frame {
	uint16			fc;		/* frame control */
	uint16			durid;		/* duration/ID */
	struct ether_addr	ra;		/* receiver address */
} PACKED;
#define	DOT11_ACK_LEN		10

struct dot11_ps_poll_frame {
	uint16			fc;		/* frame control */
	uint16			durid;		/* AID */
	struct ether_addr	bssid;		/* receiver address, STA in AP */
	struct ether_addr	ta;		/* transmitter address */
} PACKED;
#define	DOT11_PS_POLL_LEN	16

struct dot11_cf_end_frame {
	uint16			fc;		/* frame control */
	uint16			durid;		/* duration/ID */
	struct ether_addr	ra;		/* receiver address */
	struct ether_addr	bssid;		/* transmitter address, STA in AP */
} PACKED;
#define	DOT11_CS_END_LEN	16

/* Management frame header */
struct dot11_management_header {
	uint16			fc;		/* frame control */
	uint16			durid;		/* duration/ID */
	struct ether_addr	da;		/* receiver address */
	struct ether_addr	sa;		/* transmitter address */
	struct ether_addr	bssid;		/* BSS ID */
	uint16			seq;		/* sequence control */
} PACKED;
#define	DOT11_MGMT_HDR_LEN	24

/* Management frame payloads */

struct dot11_bcn_prb {
	uint32			timestamp[2];
	uint16			beacon_interval;
	uint16			capability;
} PACKED;
#define	DOT11_BCN_PRB_LEN	12

struct dot11_auth {
	uint16			alg;		/* algorithm */
	uint16			seq;		/* sequence control */
	uint16			status;		/* status code */
} PACKED;
#define DOT11_AUTH_FIXED_LEN	6		/* length of auth frame without challenge info elt */

struct dot11_assoc_req {
	uint16			capability;	/* capability information */
	uint16			listen;		/* listen interval */
} PACKED;

struct dot11_assoc_resp {
	uint16			capability;	/* capability information */
	uint16			status;		/* status code */
	uint16			aid;		/* association ID */
} PACKED;

struct dot11_action_measure {
	uint8	category;
	uint8	action;
	uint8	token;
	uint8	data[1];
} PACKED;
#define DOT11_ACTION_MEASURE_LEN	3

/**************
  802.11h related definitions.
**************/
typedef struct {
	uint8 id;
	uint8 len;
	uint8 power;
} dot11_power_cnst_t;

typedef struct {
	uint8 min;
	uint8 max;
} dot11_power_cap_t;

typedef struct {
	uint8 id;
	uint8 len;
	uint8 tx_pwr;
	uint8 margin;
} dot11_tpc_rep_t;
#define DOT11_MNG_IE_TPC_REPORT_LEN	2	/* length of IE data, not including 2 byte header */

typedef struct {
	uint8 id;
	uint8 len;
	uint8 first_channel;
	uint8 num_channels;
} dot11_supp_channels_t;

struct dot11_channel_switch {
	uint8 id;
	uint8 len;
	uint8 mode;
	uint8 channel;
	uint8 count;
}  PACKED;
typedef struct dot11_channel_switch dot11_channel_switch_t;

/* 802.11h Measurement Request/Report IEs */
/* Measurement Type field */
#define DOT11_MEASURE_TYPE_BASIC 	0
#define DOT11_MEASURE_TYPE_CCA 		1
#define DOT11_MEASURE_TYPE_RPI	 	2

/* Measurement Mode field */

/* Measurement Request Modes */
#define DOT11_MEASURE_MODE_ENABLE 	(1<<1)
#define DOT11_MEASURE_MODE_REQUEST	(1<<2)
#define DOT11_MEASURE_MODE_REPORT 	(1<<3)
/* Measurement Report Modes */
#define DOT11_MEASURE_MODE_LATE 	(1<<0)
#define DOT11_MEASURE_MODE_INCAPABLE	(1<<1)
#define DOT11_MEASURE_MODE_REFUSED	(1<<2)
/* Basic Measurement Map bits */
#define DOT11_MEASURE_BASIC_MAP_BSS	((uint8)(1<<0))
#define DOT11_MEASURE_BASIC_MAP_OFDM	((uint8)(1<<1))
#define DOT11_MEASURE_BASIC_MAP_UKNOWN	((uint8)(1<<2))
#define DOT11_MEASURE_BASIC_MAP_RADAR	((uint8)(1<<3))
#define DOT11_MEASURE_BASIC_MAP_UNMEAS	((uint8)(1<<4))

typedef struct {
	uint8 id;
	uint8 len;
	uint8 token;
	uint8 mode;
	uint8 type;
	uint8 channel;
	uint8 start_time[8];
	uint16 duration;
} dot11_meas_req_t;
#define DOT11_MNG_IE_MREQ_LEN 14
/* length of Measure Request IE data not including variable len */
#define DOT11_MNG_IE_MREQ_FIXED_LEN 3

struct dot11_meas_rep {
	uint8 id;
	uint8 len;
	uint8 token;
	uint8 mode;
	uint8 type;
	union 
	{
		struct {
			uint8 channel;
			uint8 start_time[8];
			uint16 duration;
			uint8 map;
		} PACKED basic;
		uint8 data[1];
	} PACKED rep;
} PACKED;
typedef struct dot11_meas_rep dot11_meas_rep_t;

/* length of Measure Report IE data not including variable len */
#define DOT11_MNG_IE_MREP_FIXED_LEN	3

struct dot11_meas_rep_basic {
	uint8 channel;
	uint8 start_time[8];
	uint16 duration;
	uint8 map;
} PACKED;
typedef struct dot11_meas_rep_basic dot11_meas_rep_basic_t;
#define DOT11_MEASURE_BASIC_REP_LEN	12

struct dot11_quiet {
	uint8 id;
	uint8 len;
	uint8 count;	/* TBTTs until beacon interval in quiet starts */
	uint8 period;	/* Beacon intervals between periodic quiet periods ? */
	uint16 duration;/* Length of quiet period, in TU's */
	uint16 offset;	/* TU's offset from TBTT in Count field */
} PACKED;
typedef struct dot11_quiet dot11_quiet_t;

typedef struct {
	uint8 channel;
	uint8 map;
} chan_map_tuple_t;

typedef struct {
	uint8 id;
	uint8 len;
	uint8 eaddr[ETHER_ADDR_LEN];
	uint8 interval;
	chan_map_tuple_t map[1];
} dot11_ibss_dfs_t;


/* Macro to take a pointer to a beacon or probe response
 * header and return the char* pointer to the SSID info element
 */
#define BCN_PRB_SSID(hdr) ((char*)(hdr) + DOT11_MGMT_HDR_LEN + DOT11_BCN_PRB_LEN)

/* Authentication frame payload constants */
#define DOT11_OPEN_SYSTEM	0
#define DOT11_SHARED_KEY	1
#define DOT11_CHALLENGE_LEN	128

/* Frame control macros */
#define FC_PVER_MASK		0x3
#define FC_PVER_SHIFT		0
#define FC_TYPE_MASK		0xC
#define FC_TYPE_SHIFT		2
#define FC_SUBTYPE_MASK		0xF0
#define FC_SUBTYPE_SHIFT	4
#define FC_TODS			0x100
#define FC_TODS_SHIFT		8
#define FC_FROMDS		0x200
#define FC_FROMDS_SHIFT		9
#define FC_MOREFRAG		0x400
#define FC_MOREFRAG_SHIFT	10
#define FC_RETRY		0x800
#define FC_RETRY_SHIFT		11
#define FC_PM			0x1000
#define FC_PM_SHIFT		12
#define FC_MOREDATA		0x2000
#define FC_MOREDATA_SHIFT	13
#define FC_WEP			0x4000
#define FC_WEP_SHIFT		14
#define FC_ORDER		0x8000
#define FC_ORDER_SHIFT		15

/* sequence control macros */
#define SEQNUM_SHIFT		4
#define FRAGNUM_MASK		0xF

/* Frame Control type/subtype defs */

/* FC Types */
#define FC_TYPE_MNG		0
#define FC_TYPE_CTL		1
#define FC_TYPE_DATA		2

/* Management Subtypes */
#define FC_SUBTYPE_ASSOC_REQ		0
#define FC_SUBTYPE_ASSOC_RESP		1
#define FC_SUBTYPE_REASSOC_REQ		2
#define FC_SUBTYPE_REASSOC_RESP		3
#define FC_SUBTYPE_PROBE_REQ		4
#define FC_SUBTYPE_PROBE_RESP		5
#define FC_SUBTYPE_BEACON		8
#define FC_SUBTYPE_ATIM			9
#define FC_SUBTYPE_DISASSOC		10
#define FC_SUBTYPE_AUTH			11
#define FC_SUBTYPE_DEAUTH		12
#define FC_SUBTYPE_ACTION		13

/* Control Subtypes */
#define FC_SUBTYPE_PS_POLL		10
#define FC_SUBTYPE_RTS			11
#define FC_SUBTYPE_CTS			12
#define FC_SUBTYPE_ACK			13
#define FC_SUBTYPE_CF_END		14
#define FC_SUBTYPE_CF_END_ACK		15

/* Data Subtypes */
#define FC_SUBTYPE_DATA			0
#define FC_SUBTYPE_DATA_CF_ACK		1
#define FC_SUBTYPE_DATA_CF_POLL		2
#define FC_SUBTYPE_DATA_CF_ACK_POLL	3
#define FC_SUBTYPE_NULL			4
#define FC_SUBTYPE_CF_ACK		5
#define FC_SUBTYPE_CF_POLL		6
#define FC_SUBTYPE_CF_ACK_POLL		7

/* type-subtype combos */
#define FC_KIND_MASK		(FC_TYPE_MASK | FC_SUBTYPE_MASK)

#define FC_KIND(t, s) (((t) << FC_TYPE_SHIFT) | ((s) << FC_SUBTYPE_SHIFT))

#define FC_ASSOC_REQ	FC_KIND(FC_TYPE_MNG, FC_SUBTYPE_ASSOC_REQ)
#define FC_ASSOC_RESP	FC_KIND(FC_TYPE_MNG, FC_SUBTYPE_ASSOC_RESP)
#define FC_REASSOC_REQ	FC_KIND(FC_TYPE_MNG, FC_SUBTYPE_REASSOC_REQ)
#define FC_REASSOC_RESP	FC_KIND(FC_TYPE_MNG, FC_SUBTYPE_REASSOC_RESP)
#define FC_PROBE_REQ	FC_KIND(FC_TYPE_MNG, FC_SUBTYPE_PROBE_REQ)
#define FC_PROBE_RESP	FC_KIND(FC_TYPE_MNG, FC_SUBTYPE_PROBE_RESP)
#define FC_BEACON	FC_KIND(FC_TYPE_MNG, FC_SUBTYPE_BEACON)
#define FC_DISASSOC	FC_KIND(FC_TYPE_MNG, FC_SUBTYPE_DISASSOC)
#define FC_AUTH		FC_KIND(FC_TYPE_MNG, FC_SUBTYPE_AUTH)
#define FC_DEAUTH	FC_KIND(FC_TYPE_MNG, FC_SUBTYPE_DEAUTH)
#define FC_ACTION	FC_KIND(FC_TYPE_MNG, FC_SUBTYPE_ACTION)

#define FC_PS_POLL	FC_KIND(FC_TYPE_CTL, FC_SUBTYPE_PS_POLL)
#define FC_RTS		FC_KIND(FC_TYPE_CTL, FC_SUBTYPE_RTS)
#define FC_CTS		FC_KIND(FC_TYPE_CTL, FC_SUBTYPE_CTS)
#define FC_ACK		FC_KIND(FC_TYPE_CTL, FC_SUBTYPE_ACK)
#define FC_CF_END	FC_KIND(FC_TYPE_CTL, FC_SUBTYPE_CF_END)
#define FC_CF_END_ACK	FC_KIND(FC_TYPE_CTL, FC_SUBTYPE_CF_END_ACK)

#define FC_DATA		FC_KIND(FC_TYPE_DATA, FC_SUBTYPE_DATA)
#define FC_NULL_DATA	FC_KIND(FC_TYPE_DATA, FC_SUBTYPE_NULL)
#define FC_DATA_CF_ACK	FC_KIND(FC_TYPE_DATA, FC_SUBTYPE_DATA_CF_ACK)

/* Management Frames */

/* Management Frame Constants */

/* Fixed fields */
#define DOT11_MNG_AUTH_ALGO_LEN		2
#define DOT11_MNG_AUTH_SEQ_LEN		2
#define DOT11_MNG_BEACON_INT_LEN	2
#define DOT11_MNG_CAP_LEN		2
#define DOT11_MNG_AP_ADDR_LEN		6
#define DOT11_MNG_LISTEN_INT_LEN	2
#define DOT11_MNG_REASON_LEN		2
#define DOT11_MNG_AID_LEN		2
#define DOT11_MNG_STATUS_LEN		2
#define DOT11_MNG_TIMESTAMP_LEN		8

/* DUR/ID field in assoc resp is 0xc000 | AID */
#define DOT11_AID_MASK			0x3fff

/* Reason Codes */
#define DOT11_RC_RESERVED			0
#define DOT11_RC_UNSPECIFIED			1	/* Unspecified reason */
#define DOT11_RC_AUTH_INVAL			2	/* Previous authentication no longer valid */
#define DOT11_RC_DEAUTH_LEAVING			3	/* Deauthenticated because sending station is
							   leaving (or has left) IBSS or ESS */
#define DOT11_RC_INACTIVITY			4	/* Disassociated due to inactivity */
#define DOT11_RC_BUSY				5	/* Disassociated because AP is unable to handle
							   all currently associated stations */
#define DOT11_RC_INVAL_CLASS_2			6	/* Class 2 frame received from
							   nonauthenticated station */
#define DOT11_RC_INVAL_CLASS_3			7	/* Class 3 frame received from
							   nonassociated station */
#define DOT11_RC_DISASSOC_LEAVING		8	/* Disassociated because sending station is
							   leaving (or has left) BSS */
#define DOT11_RC_NOT_AUTH			9	/* Station requesting (re)association is
							   not authenticated with responding station */
#define DOT11_RC_MAX				23	/* Reason codes > 23 are reserved */

/* Status Codes */
#define DOT11_STATUS_SUCCESS			0	/* Successful */
#define DOT11_STATUS_FAILURE			1	/* Unspecified failure */
#define DOT11_STATUS_CAP_MISMATCH		10	/* Cannot support all requested capabilities
							   in the Capability Information field */
#define DOT11_STATUS_REASSOC_FAIL		11	/* Reassociation denied due to inability to
							   confirm that association exists */
#define DOT11_STATUS_ASSOC_FAIL			12	/* Association denied due to reason outside
							   the scope of this standard */
#define DOT11_STATUS_AUTH_MISMATCH		13	/* Responding station does not support the
							   specified authentication algorithm */
#define DOT11_STATUS_AUTH_SEQ			14	/* Received an Authentication frame with
							   authentication transaction sequence number
							   out of expected sequence */
#define DOT11_STATUS_AUTH_CHALLENGE_FAIL	15	/* Authentication rejected because of challenge failure */
#define DOT11_STATUS_AUTH_TIMEOUT		16	/* Authentication rejected due to timeout waiting
							   for next frame in sequence */
#define DOT11_STATUS_ASSOC_BUSY_FAIL		17	/* Association denied because AP is unable to
							   handle additional associated stations */
#define DOT11_STATUS_ASSOC_RATE_MISMATCH	18	/* Association denied due to requesting station
							   not supporting all of the data rates in the
							   BSSBasicRateSet parameter */
#define DOT11_STATUS_ASSOC_SHORT_REQUIRED	19	/* Association denied due to requesting station
							   not supporting the Short Preamble option */
#define DOT11_STATUS_ASSOC_PBCC_REQUIRED	20	/* Association denied due to requesting station
							   not supporting the PBCC Modulation option */
#define DOT11_STATUS_ASSOC_AGILITY_REQUIRED	21	/* Association denied due to requesting station
							   not supporting the Channel Agility option */
#define DOT11_STATUS_ASSOC_SPECTRUM_REQUIRED	22	/* Association denied because Spectrum Management 
							   capability is required. */
#define DOT11_STATUS_ASSOC_BAD_POWER_CAP	23	/* Association denied because the info in the 
							   Power Cap element is unacceptable. */
#define DOT11_STATUS_ASSOC_BAD_SUP_CHANNELS	24	/* Association denied because the info in the 
							   Supported Channel element is unacceptable */
#define DOT11_STATUS_ASSOC_SHORTSLOT_REQUIRED	25	/* Association denied due to requesting station
							   not supporting the Short Slot Time option */
#define DOT11_STATUS_ASSOC_ERPBCC_REQUIRED	26	/* Association denied due to requesting station
							   not supporting the ER-PBCC Modulation option */
#define DOT11_STATUS_ASSOC_DSSOFDM_REQUIRED	27	/* Association denied due to requesting station
							   not supporting the DSS-OFDM option */

/* Info Elts, length of INFORMATION portion of Info Elts */
#define DOT11_MNG_DS_PARAM_LEN			1
#define DOT11_MNG_IBSS_PARAM_LEN		2

/* TIM Info element has 3 bytes fixed info in INFORMATION field,
 * followed by 1 to 251 bytes of Partial Virtual Bitmap */
#define DOT11_MNG_TIM_FIXED_LEN			3
#define DOT11_MNG_TIM_DTIM_COUNT		0
#define DOT11_MNG_TIM_DTIM_PERIOD		1
#define DOT11_MNG_TIM_BITMAP_CTL		2
#define DOT11_MNG_TIM_PVB			3

/* TLV defines */
#define TLV_TAG_OFF		0
#define TLV_LEN_OFF		1
#define TLV_HDR_LEN		2
#define TLV_BODY_OFF		2

/* Management Frame Information Element IDs */
#define DOT11_MNG_SSID_ID			0
#define DOT11_MNG_RATES_ID			1
#define DOT11_MNG_FH_PARMS_ID			2
#define DOT11_MNG_DS_PARMS_ID			3
#define DOT11_MNG_CF_PARMS_ID			4
#define DOT11_MNG_TIM_ID			5
#define DOT11_MNG_IBSS_PARMS_ID			6
#define DOT11_MNG_COUNTRY_ID			7
#define DOT11_MNG_HOPPING_PARMS_ID		8
#define DOT11_MNG_HOPPING_TABLE_ID		9
#define DOT11_MNG_REQUEST_ID			10
#define DOT11_MNG_CHALLENGE_ID			16
#define DOT11_MNG_PWR_CONSTRAINT_ID		32    /* 11H PowerConstraint	*/
#define DOT11_MNG_PWR_CAP_ID			33    /* 11H PowerCapability	*/
#define DOT11_MNG_TPC_REQUEST_ID 		34    /* 11H TPC Request	*/
#define DOT11_MNG_TPC_REPORT_ID			35    /* 11H TPC Report		*/
#define DOT11_MNG_SUPP_CHANNELS_ID		36    /* 11H Supported Channels	*/
#define DOT11_MNG_CHANNEL_SWITCH_ID		37    /* 11H ChannelSwitch Announcement*/
#define DOT11_MNG_MEASURE_REQUEST_ID		38    /* 11H MeasurementRequest	*/
#define DOT11_MNG_MEASURE_REPORT_ID		39    /* 11H MeasurementReport	*/
#define DOT11_MNG_QUIET_ID			40    /* 11H Quiet		*/
#define DOT11_MNG_IBSS_DFS_ID			41    /* 11H IBSS_DFS 		*/
#define DOT11_MNG_ERP_ID			42
#define DOT11_MNG_NONERP_ID			47
#define DOT11_MNG_EXT_RATES_ID			50
#define DOT11_MNG_WPA_ID			221
#define DOT11_MNG_PROPR_ID			221

/* ERP info element bit values */
#define DOT11_MNG_ERP_LEN			1	/* ERP is currently 1 byte long */
#define DOT11_MNG_NONERP_PRESENT		0x01	/* NonERP (802.11b) STAs are present in the BSS */
#define DOT11_MNG_USE_PROTECTION		0x02	/* Use protection mechanisms for ERP-OFDM frames */
#define DOT11_MNG_BARKER_PREAMBLE		0x04	/* Short Preambles: 0 == allowed, 1 == not allowed */

/* Capability Information Field */
#define DOT11_CAP_ESS				0x0001
#define DOT11_CAP_IBSS				0x0002
#define DOT11_CAP_POLLABLE			0x0004
#define DOT11_CAP_POLL_RQ			0x0008
#define DOT11_CAP_PRIVACY			0x0010
#define DOT11_CAP_SHORT				0x0020
#define DOT11_CAP_PBCC				0x0040
#define DOT11_CAP_AGILITY			0x0080
#define DOT11_CAP_SPECTRUM			0x0100
#define DOT11_CAP_SHORTSLOT			0x0400
#define DOT11_CAP_CCK_OFDM			0x2000

/* Action Frame Constants */
#define DOT11_ACTION_CAT_ERR_MASK	0x10
#define DOT11_ACTION_CAT_SPECT_MNG	0x00

#define DOT11_ACTION_ID_M_REQ		0
#define DOT11_ACTION_ID_M_REP		1
#define DOT11_ACTION_ID_TPC_REQ		2
#define DOT11_ACTION_ID_TPC_REP		3
#define DOT11_ACTION_ID_CHANNEL_SWITCH	4

/* MLME Enumerations */
#define DOT11_BSSTYPE_INFRASTRUCTURE		0
#define DOT11_BSSTYPE_INDEPENDENT		1
#define DOT11_BSSTYPE_ANY			2
#define DOT11_SCANTYPE_ACTIVE			0
#define DOT11_SCANTYPE_PASSIVE			1

/* 802.11 A PHY constants */
#define APHY_SLOT_TIME		9
#define APHY_SIFS_TIME		16
#define APHY_DIFS_TIME		(APHY_SIFS_TIME + (2 * APHY_SLOT_TIME))
#define APHY_PREAMBLE_TIME	16
#define APHY_SIGNAL_TIME	4
#define APHY_SYMBOL_TIME	4
#define APHY_SERVICE_NBITS	16
#define APHY_TAIL_NBITS		6
#define	APHY_CWMIN		15

/* 802.11 B PHY constants */
#define BPHY_SLOT_TIME		20
#define BPHY_SIFS_TIME		10
#define BPHY_DIFS_TIME		50
#define BPHY_PLCP_TIME		192
#define BPHY_PLCP_SHORT_TIME	96
#define	BPHY_CWMIN		31

/* 802.11 G constants */
#define DOT11_OFDM_SIGNAL_EXTENSION	6

#define PHY_CWMAX		1023

#define	DOT11_MAXNUMFRAGS	16	/* max # fragments per MSDU */

/* dot11Counters Table - 802.11 spec., Annex D */
typedef struct d11cnt {
	uint32		txfrag;		/* dot11TransmittedFragmentCount */
	uint32		txmulti;	/* dot11MulticastTransmittedFrameCount */
	uint32		txfail;		/* dot11FailedCount */
	uint32		txretry;	/* dot11RetryCount */
	uint32		txretrie;	/* dot11MultipleRetryCount */
	uint32		rxdup;		/* dot11FrameduplicateCount */
	uint32		txrts;		/* dot11RTSSuccessCount */
	uint32		txnocts;	/* dot11RTSFailureCount */
	uint32		txnoack;	/* dot11ACKFailureCount */
	uint32		rxfrag;		/* dot11ReceivedFragmentCount */
	uint32		rxmulti;	/* dot11MulticastReceivedFrameCount */
	uint32		rxcrc;		/* dot11FCSErrorCount */
	uint32		txfrmsnt;	/* dot11TransmittedFrameCount */
	uint32		rxundec;	/* dot11WEPUndecryptableCount */
} d11cnt_t;

/* BRCM OUI */
#define BRCM_OUI		"\x00\x10\x18"

/* BRCM info element */
struct brcm_ie {
	uchar	id;		
	uchar	len;   
	uchar 	oui[3];
	uchar	ver;
	uchar	assoc;		/*  # of assoc STAs */
	uchar	flags;		/* misc flags */
} PACKED;
#define BRCM_IE_LEN		8
typedef	struct brcm_ie brcm_ie_t;
#define BRCM_IE_VER		1

/* brcm_ie flags */
#define	BRF_ABCAP		0x1	/* afterburner capable */
#define	BRF_ABRQRD		0x2	/* afterburner requested */

/* WPA definitions */
#define WPA_VERSION		1
#define WPA_OUI			"\x00\x50\xF2"

#define WPA_OUI_LEN	3

/* WPA authentication modes */
#define WPA_AUTH_NONE		0	/* None */
#define WPA_AUTH_UNSPECIFIED	1	/* Unspecified authentication over 802.1X: default for WPA */
#define WPA_AUTH_PSK		2	/* Pre-shared Key over 802.1X */
#define WPA_AUTH_DISABLED	255	/* Legacy (i.e., non-WPA) */
				 
#define IS_WPA_AUTH(auth)	((auth) == WPA_AUTH_NONE || \
				 (auth) == WPA_AUTH_UNSPECIFIED || \
				 (auth) == WPA_AUTH_PSK)


/* Key related defines */
#define DOT11_MAX_KEY_SIZE	32	/* max size of any key */
#define DOT11_MAX_IV_SIZE	16	/* max size of any IV */
#define DOT11_EXT_IV_FLAG	(1<<5)	/* flag to indicate IV is > 4 bytes */

#define WEP1_KEY_SIZE		5	/* max size of any WEP key */
#define WEP1_KEY_HEX_SIZE	10	/* size of WEP key in hex. */
#define WEP128_KEY_SIZE		13	/* max size of any WEP key */
#define WEP128_KEY_HEX_SIZE	26	/* size of WEP key in hex. */
#define TKIP_MIC_SIZE		8	/* size of TKIP MIC */
#define TKIP_EOM_SIZE		7	/* max size of TKIP EOM */
#define TKIP_EOM_FLAG		0x5a	/* TKIP EOM flag byte */
#define TKIP_KEY_SIZE		32	/* size of any TKIP key */
#define TKIP_MIC_AUTH_TX	16	/* offset to Authenticator MIC TX key */
#define TKIP_MIC_AUTH_RX	24	/* offset to Authenticator MIC RX key */
#define TKIP_MIC_SUP_RX		16	/* offset to Supplicant MIC RX key */
#define TKIP_MIC_SUP_TX		24	/* offset to Supplicant MIC TX key */
#define AES_KEY_SIZE		16	/* size of AES key */

#undef PACKED
#if !defined(__GNUC__)
#pragma pack()
#endif

#endif /* _802_11_H_ */
