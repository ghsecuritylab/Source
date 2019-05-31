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
 * Router default NVRAM values
 *
 * Copyright 2013, ASUSTek Inc.
 * All Rights Reserved.
 *
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 */

#include <nvram/bcmnvram.h>
#include "../../autoconf.h"

/* 
 * factory default
 */
struct nvram_tuple router_defaults[] = {
	{ "restore_defaults", "0", 0 },		/* Set to 0 to not restore defaults on boot */
	{ "NetworkType", "Infra", 0 },		// Infra or Adhoc
	{ "BssidNum", "1", 0 },
	{ "wl_TxPower", "100", 0 },
	{ "wl_DisableOLBC", "0", 0 },
	{ "wl_TxPreamble", "0", 0 },
	{ "wl_TxBurst", "1", 0 },
	{ "wl_PktAggregate", "1", 0 },
	{ "AccessPolicy1", "0", 0 },
	{ "AccessControlList1", "", 0 },
	{ "AccessPolicy2", "0", 0 },
	{ "AccessControlList2", "", 0 },
	{ "AccessPolicy3", "0", 0 },
	{ "AccessControlList3", "", 0 },
	{ "wl_HT_AutoBA", "1", 0 },
	{ "wl_HT_HTC", "1", 0 },
	{ "wl_HT_RDG", "1", 0 },
	{ "wl_HT_LinkAdapt", "0", 0 },
	{ "wl_HT_BW", "1", 0 },
	{ "wl_nctrlsb", "lower", 0 },		/* N-CTRL SB */
	{ "wl_HT_OpMode", "0", 0 },
	{ "wl_HT_MpduDensity", "5", 0 },
	{ "wl_HT_AMSDU", "0", 0 },
	{ "wl_HT_GI", "1", 0 },
	{ "wl_HT_BAWinSize", "64", 0 },
	{ "wl_HT_MCS", "33", 0 },
	{ "wl_HT_BADecline", "0", 0 },
	{ "wl_HT_STBC", "1", 0 },
	{ "wl_HT_PROTECT", "1", 0 },
	{ "wl_HT_MIMOPSMode", "3", 0 },
	{ "wl_HT_DisallowTKIP", "1", 0 },
	{ "APAifsn", "3;7;1;1", 0 },
	{ "APCwmin", "4;4;3;2", 0 },
	{ "APCwmax", "6;10;4;3", 0 },
	{ "APTxop", "0;0;94;47", 0 },
	{ "APACM", "0;0;0;0", 0 },
	{ "BSSAifsn", "3;7;2;2", 0 },
	{ "BSSCwmin", "4;4;3;2", 0 },
	{ "BSSCwmax", "10;10;4;3", 0 },
	{ "BSSTxop", "0;0;94;47", 0 },
	{ "BSSACM", "0;0;0;0", 0 },
	{ "wl_APSDCapable", "0", 0 },
	{ "wl_DLSCapable", "0", 0 },
	{ "CSPeriod", "6", 0 },
	{ "RDRegion", "0", 0 },
	{ "RekeyMethod", "TIME", 0 },
	{ "PMKCachePeriod", "10", 0 },
	{ "PreAuth", "0", 0 },
	{ "RADIUS_Acct_Server", "", 0 },
	{ "RADIUS_Acct_Port", "1813", 0 },
	{ "RADIUS_Acct_Key", "", 0 },
	{ "EAPifname", "br0", 0 },
	{ "PreAuthifname", "br0", 0 },
	{ "session_timeout_interval", "0", 0 },
	{ "idle_timeout_interval", "0", 0 },
	/* Miscellaneous parameters */
	{ "timer_interval", "3600", 0 },	/* Timer interval in seconds */
	{ "log_level", "0", 0 },		/* Bitmask 0:off 1:denied 2:accepted */
	{ "upnp_enable", "1", 0 },		/* Start UPnP */
	{ "console_loglevel", "7", 0 },		/* Kernel panics only */
	{ "log_ipaddr", "", 0 },		/* syslog recipient */
	/* LAN H/W parameters */
	{ "lan_ifname", "br0", 0 },		/* LAN interface name */
	{ "lan_hwaddr", "", 0 },		/* LAN interface MAC address */
	/* LAN TCP/IP parameters */
	{ "lan_dhcp", "0", 0 },			/* DHCP client [static|dhcp] */
	{ "lan_ipaddr", "192.168.1.1", 0 },	/* LAN IP address */
	{ "lan_netmask", "255.255.255.0", 0 },	/* LAN netmask */
	{ "lan_gateway", "", 0 },		/* LAN gateway */
	{ "lan_wins", "", 0 },			/* x.x.x.x x.x.x.x ... */
	{ "lan_domain", "", 0 },		/* LAN domain name */
	{ "lan_lease", "86400", 0 },		/* LAN lease time in seconds */
	{ "lan_stp", "1", 0 },			/* LAN spanning tree protocol */
	{ "lan_route", "", 0 },			/* Static routes (ipaddr:netmask:gateway:metric:ifname ...) */
	/* WAN H/W parameters */
	{ "wan_ifname", "", 0 },		/* WAN interface name */
	{ "wan_hwaddr", "", 0 },		/* WAN interface MAC address */
	/* WAN TCP/IP parameters */
	{ "wan_proto", "dhcp", 0 },		/* [static|dhcp|pppoe|disabled] */
	{ "wan_ipaddr", "", 0 },		/* WAN IP address */
	{ "wan_netmask", "", 0 },		/* WAN netmask */
	{ "wan_gateway", "", 0 },		/* WAN gateway */
	{ "wan_dns", "", 0 },			/* x.x.x.x x.x.x.x ... */
	{ "wan_wins", "", 0 },			/* x.x.x.x x.x.x.x ... */
	{ "wan_hostname", "", 0 },		/* WAN hostname */
	{ "wan_domain", "", 0 },		/* WAN domain name */
	{ "wan_lease", "86400", 0 },		/* WAN lease time in seconds */
	/* PPPoE parameters */
	{ "wan_pppoe_ifname", "ppp0", 0 },	/* PPPoE enslaved interface */
	{ "wan_pppoe_username", "", 0 },	/* PPP username */
	{ "wan_pppoe_passwd", "", 0 },		/* PPP password */
	{ "wan_pppoe_ipaddr", "", 0 },		/* PPP IP address */
	{ "wan_pppoe_netmask", "", 0 },		/* PPP netmask */
	{ "wan_pppoe_gateway", "", 0 },		/* PPP gateway */
	{ "wan_pppoe_idletime", "0", 0 },	// oleg patch
	{ "wan_pppoe_keepalive", "0", 0 },	/* Restore link automatically */
	{ "wan_pppoe_demand", "0", 0 },		/* Dial on demand */
	{ "wan_pppoe_mru", "1492", 0 },		/* Negotiate MRU to this value */
	{ "wan_pppoe_mtu", "1492", 0 },		/* Negotiate MTU to the smaller of this value or the peer MRU */
	{ "wan_pppoe_service", "", 0 },		/* PPPoE service name */
	{ "wan_pppoe_ac", "", 0 },		/* PPPoE access concentrator name */
	/* Misc WAN parameters */
	{ "wan_enable", "1", 0 },
	{ "wan_nat_x", "1", 0 },
	{ "wan_upnp_enable", "1", 0 },
	{ "wan_desc", "", 0 },			/* WAN connection description */
	{ "wan_route", "", 0 },			/* Static routes (ipaddr:netmask:gateway:metric:ifname ...) */
	{ "wan_primary", "0", 0 },		/* Primary wan connection */
	{ "wan_unit", "0", 0 },			/* Last configured connection */
	/* Filters */
	{ "filter_maclist", "", 0 },		/* xx:xx:xx:xx:xx:xx ... */
	{ "filter_macmode", "deny", 0 },	/* "allow" only, "deny" only, or "disabled" (allow all) */
	{ "filter_client0", "", 0 },		/* [lan_ipaddr0-lan_ipaddr1|*]:lan_port0-lan_port1,
						 * proto,enable,day_start-day_end,sec_start-sec_end,desc */
	/* Port forwards */
	{ "forward_port0", "", 0 },		/* wan_port0-wan_port1>lan_ipaddr: lan_port0-lan_port1[:,]proto[:,]enable[:,]desc */
	/* DHCP server parameters */
	{ "dhcp_enable_x", "1", 0 },
	{ "dhcp_start", "192.168.1.2", 0 },	/* First assignable DHCP address */
	{ "dhcp_end", "192.168.1.254", 0 },	/* Last assignable DHCP address */
	{ "dhcp_lease", "86400", 0 },
	{ "dhcp_gateway_x", "", 0 },
	{ "dhcp_dns1_x", "", 0 },
	{ "dhcp_wins_x", "", 0 },
	{ "dhcp_static_x", "0", 0 },
	{ "dhcp_staticlist", "", 0 },
	/* Web server parameters */
	{ "http_username", "admin", 0 },	/* Username */
	{ "http_passwd", "admin", 0 },		/* Password */
	{ "http_wanport", "", 0 },		/* WAN port to listen on */
	{ "http_lanport", "80", 0 },		/* LAN port to listen on */
	/* Wireless H/W parameters */
	{ "wl0_macaddr", "", 0 },		/* 2.4G MAC address: XX:XX:XX:XX:XX:XX */
	{ "wl1_macaddr", "", 0 },		/* 5G MAC address: XX:XX:XX:XX:XX:XX */
	/* Wireless parameters */
	{ "wl_ssid", "", 0 },
	{ "wl_auth_mode", "open", 0 },		/* Network authentication mode Open System */
	{ "wl_nmode_x", "0", 0 },		/* Auto */
	{ "wl_channel", "0", 0 },		/* Channel number */
	{ "wl_rateset", "default", 0 },		/* "default" or "all" or "12" */
	{ "wl_bcn", "100", 0 },			/* Beacon interval */
	{ "wl_dtim", "1", 0 },			/* DTIM period */
	{ "wl_gmode_protection", "auto", 0 },	/* 802.11g RTS/CTS protection (off|auto) */
	{ "wl_rts", "2347", 0 },		/* RTS threshold */
	{ "wl_frag", "2346", 0 },		/* Fragmentation threshold */
	{ "wl_ap_isolate", "0", 0 },            /* AP isolate mode */
	{ "wl_closed", "0", 0 },		/* Closed (hidden) network */
	{ "wl_wme", "1", 0 },			/* WME mode (off|on) */
	{ "wl_wme_no_ack", "off", 0 },		/* WME No-Acknowledgment mode */
	{ "wl_mrate", "13", 0 },		/* Mcast Rate (bps, 0 for auto) */
	{ "wl_user_rssi", "0", 0 },		/* Roaming */
	/* WEP parameters */
	{ "wl_wep_x", "0", 0 },
	{ "wl_key", "1", 0 },			/* Current WEP key */
	{ "wl_key1", "", 0 },			/* 5/13 char ASCII or 10/26 char hex */
	{ "wl_key2", "", 0 },			/* 5/13 char ASCII or 10/26 char hex */
	{ "wl_key3", "", 0 },			/* 5/13 char ASCII or 10/26 char hex */
	{ "wl_key4", "", 0 },			/* 5/13 char ASCII or 10/26 char hex */
	{ "wl_key_type", "0", 0 },		/* WEP key format (HEX/ASCII)*/
	/* WPA parameters */
	{ "wl_wpa_mode", "2", 0 },
	{ "wl_crypto", "aes", 0 },		/* WPA data encryption */
	{ "wl_wpa_psk", "12345678", 0 },	/* WPA pre-shared key */
	{ "wl_wpa_gtk_rekey", "0", 0 },		/* GTK rotation interval */
	{ "wl_radius_ipaddr", "", 0 },		/* RADIUS server IP address */
	{ "wl_radius_port", "1812", 0 },	/* RADIUS server UDP port */
	{ "wl_radius_key", "", 0 },		/* RADIUS shared secret */
	{ "wl_lazywds", "0", 0 },		/* Enable "lazy" WDS mode (0|1) */
	{ "ap_selecting", "0", 0 },
	{ "pppd_way", "2", 0 },
	{ "fw_dos_x", "0", 0 },			// oleg patch
	{ "dr_enable_x", "1", 0 },		// oleg patch
	{ "mr_enable_x", "0", 0 },		// oleg patch
	// Wireless
	// RBRList
	{ "wl_wdslist_x", "", 0 },
	// ACL
	{ "wl_macmode", "disabled", 0 },	/* "allow" only, "deny" only, or "disabled"(allow all) */
	{ "wl_maclist_x", "", 0 },		/* xx:xx:xx:xx:xx:xx ... */
	// LAN
	// GWStatic
	{ "sr_enable_x", "0", 0 },
	{ "sr_rulelist", "", 0 },
	// WAN
	// x_USRRuleList
	{ "qos_service_name_x", "", 0 },
	{ "qos_ip_x", "", 0 },
	{ "qos_port_x", "", 0 },
	{ "qos_prio_x", "", 0 },
	// TriggerList
	{ "autofw_enable_x", "0", 0 },
	{ "autofw_rulelist", "", 0 },
	// VSList
	{ "vts_enable_x", "0", 0 },
	{ "vts_rulelist", "", 0 },
	{ "vts_ftpport", "2021", 0 },
	// USB Application
	// Storage_UserList
	{ "acc_username", "", 0 },
	{ "acc_password", "", 0 },
	// Firewall
	// UrlList
	{ "url_enable_x", "0", 0 },
	{ "url_date_x", "1111111", 0 },
	{ "url_time_x", "00002359", 0 },
	{ "url_rulelist", "", 0 },
	// KeywordList
	{ "keyword_enable_x", "0", 0 },
	{ "keyword_date_x", "1111111", 0 },
	{ "keyword_time_x", "00002359", 0 },
	{ "keyword_rulelist", "", 0 },
	// MFList
	{ "macfilter_list_x", "", 0 },
	{ "macfilter_enable_x", "0", 0 },
	{ "macfilter_num_x", "0", 0 },
	// LWFilterList
	{ "fw_lw_enable_x", "0", 0 },
	{ "filter_lw_date_x", "1111111", 0 },
	{ "filter_lw_time_x", "00002359", 0 },
	{ "filter_lw_time2_x", "00002359", 0 },
	{ "filter_lw_default_x", "ACCEPT", 0 },
	{ "filter_lw_icmp_x", "", 0 },
	{ "filter_lwlist", "", 0 },
	// WLFilterList
	{ "fw_wl_enable_x", "0", 0 },
	{ "filter_wl_date_x", "1111111", 0 },
	{ "filter_wl_time_x", "00002359", 0 },
	{ "filter_wl_time2_x", "00002359", 0 },
	{ "filter_wl_default_x", "ACCEPT", 0 },
	{ "filter_wl_icmp_x", "", 0 },
	{ "filter_wllist", "", 0 },
	{ "wan_dhcpenable_x", "1", 0 },
	{ "wan_dnsenable_x", "1", 0 },
	{ "wan_dns1_x", "", 0 },
	{ "wan_dns2_x", "", 0 },
	{ "dmz_ip", "", 0 },
	{ "sp_battle_ips", "1", 0 },
	{ "wan_pppoe_txonly_x", "0", 0 },
	{ "wan_pppoe_relay_x", "0", 0 },
	{ "wan_pppoe_options_x", "", 0 },
	{ "wan_pptp_options_x", "", 0 },
	{ "wan_heartbeat_x", "", 0 },
	{ "fw_enable_x", "1", 0 },
	{ "fw_log_x", "none", 0 },
	{ "misc_natlog_x", "0", 0 },
	{ "misc_http_x", "0", 0 },
	{ "misc_httpport_x", "8080", 0 },
	{ "misc_lpr_x", "0", 0 },
	{ "misc_ping_x", "0", 0 },
	{ "misc_conntrack_x", "4096", 0 },
	{ "dr_static_rip_x", "0", 0 },
	{ "dr_static_matric_x", "1", 0 },
	{ "dr_default_x", "1", 0 },
	{ "dr_staticnum_x", "0", 0 },
	{ "dr_staticipaddr_x", "", 0 },
	{ "dr_staticnetmask_x", "0", 0 },
	{ "dr_staticgateway_x", "", 0 },
	{ "lan_proto_x", "1", 0 },
	{ "lan_hostname", "", 0 },
	{ "log_ipaddr", "", 0 },
	{ "time_zone", "GMT0DST_1", 0 },
	{ "time_zone_dst", "0", 0 },
	{ "time_zone_dstoff", "", 0 },
	{ "time_interval", "20", 0 },
	{ "ntp_server1", "time.nist.gov", 0 },
	{ "ntp_server0", "pool.ntp.org", 0 },
	// DDNS
	{ "ddns_enable_x", "0", 0 },
	{ "ddns_server_x", "", 0 },
	{ "ddns_username_x", "", 0 },
	{ "ddns_passwd_x", "", 0 },
	{ "ddns_hostname_x", "", 0 },
	{ "ddns_wildcard_x", "0", 0 },
	{ "wl_mode_x", "0", 0 },
	{ "wl_wdsapply_x", "0", 0 },
	{ "wl_wdsnum_x", "0", 0 },
	{ "wl_phrase_x", "", 0 },
	{ "wl_wpa_gtk_rekey", "0", 0 },
	{ "wl_afterburner", "off", 0 },
	{ "wl_rate", "0", 0 },
	{ "wl_frameburst", "off", 0 },
	{ "wl_radio_x", "1", 0 },
	{ "wl_radio_date_x", "1111111", 0 },
	{ "wl_radio_time_x", "00002359", 0 },
	{ "wl_radio_time2_x", "00002359", 0 },
	{ "wl_radio_power_x", "17", 0 },
	{ "wl_preauth", "disabled", 0 },
	{ "wl_net_reauth", "36000", 0 },
	{ "wl_macapply_x", "Both", 0 },
#ifdef DLM
	{ "acc_num", "0", 0 },
	{ "st_samba_mode", "1", 0 },
	{ "st_ftp_mode", "1", 0 },
	{ "enable_ftp", "1", 0 },
	{ "enable_samba", "1", 0 },
	{ "st_max_user", "5", 0 },
	{ "st_samba_workgroup", "WORKGROUP", 0 },
	{ "st_samba_workgroupb", "WORKGROUP", 0 },
	{ "apps_dl", "1", 0 },
	{ "apps_dl_share", "0", 0 },
	{ "apps_dl_seed", "24", 0 },
	{ "apps_dms", "1", 0 },
	{ "apps_caps", "0", 0 },
	{ "apps_comp", "0", 0 },
	{ "apps_pool", "harddisk/part0", 0 },
	{ "apps_share", "share", 0 },
	{ "apps_ver", "", 0 },
	{ "apps_seed", "0", 0 },
	{ "apps_upload_max", "0", 0 },
	{ "machine_name", MODEL_NAME, 0 },
	{ "apps_dms_usb_port", "1", 0 },
	{ "apps_dl_share_port_from", "6880", 0 },
	{ "apps_dl_share_port_to", "6999", 0 },
	{ "sh_num", "0", 0 },
#endif
	{ "temp_lang", "", 0 },
	{ "preferred_lang", "EN", 0 },
	{ "qos_enable","0", 0 },
	{ "qos_global_enable","0", 0 },
	{ "qos_userdef_enable","0", 0 },
	{ "qos_shortpkt_prio","0", 0 },		//Internet
	{ "qos_pshack_prio","0", 0 },		//Game
	{ "qos_tos_prio","0", 0 },
	{ "qos_userspec_app","0", 0 },
	{ "qos_service_enable","0", 0 },
	{ "qos_service_ubw","0", 0 },
	{ "qos_manual_ubw","0", 0 },
	{ "qos_dfragment_enable","0", 0 },
	{ "qos_dfragment_size","0", 0 },
	{ "ftp_lang", "", 0 },
#ifdef WPS_SUPPORT
	/* WSC parameters */
	{ "wps_mode", "2", 0 },
	{ "rep_wps_be_ap", "0", 0 },
#endif
	{ "x_Setting", "0", 0 },
	/* APCLI/STA parameters */
	{ "sta_clonemac", "ff:ff:ff:ff:ff:ff", 0 },
	{ "sta0_ssid", "", 0 },
	{ "sta0_bssid", "", 0 },
	{ "sta0_auth_mode", "open", 0 },
	{ "sta0_wep_x", "0", 0 },
	{ "sta0_wpa_mode", "2", 0 },
	{ "sta0_crypto", "aes", 0 },
	{ "sta0_wpa_psk", "", 0 },
	{ "sta0_key", "1", 0 },
	{ "sta0_key_type", "0", 0 },
	{ "sta0_key1", "", 0 },
	{ "sta0_key2", "", 0 },
	{ "sta0_key3", "", 0 },
	{ "sta0_key4", "", 0 },
	{ "sta1_ssid", "", 0 },
	{ "sta1_bssid", "", 0 },
	{ "sta1_auth_mode", "open", 0 },
	{ "sta1_wep_x", "0", 0 },
	{ "sta1_wpa_mode", "2", 0 },
	{ "sta1_crypto", "aes", 0 },
	{ "sta1_wpa_psk", "", 0 },
	{ "sta1_key", "1", 0 },
	{ "sta1_key_type", "0", 0 },
	{ "sta1_key1", "", 0 },
	{ "sta1_key2", "", 0 },
	{ "sta1_key3", "", 0 },
	{ "sta1_key4", "", 0 },
#ifdef WPA_SUPPLICANT_SUPPORT
	/* WPA/WPA2-Enterprise */
	{ "security_infra", "", 0 },
	{ "cert_auth_type_from_wpa", "", 0 },
	{ "cert_tunnel_auth_peap", "", 0 },
	{ "cert_id", "", 0 },
	{ "cert_password", "", 0 },
	{ "cert_tunnel_auth_ttls", "", 0 },
#endif
	{ "telnetd", "0", 0 },
	{ "EVDO_on", "0", 0 },
	{ "wl0_AutoChannelSkipList", "", 0 },
	{ "wl1_AutoChannelSkipList", "165", 0 },
	{ "reply_fake_ip", "192.168.255.1", 0 },
	{ "hijdomain", "repeater.asus.com", 0 },
	{ "dnsqmode", "2", 0 },			// 0:disabled, 1:dns only, 2:dns+dhcpd
	{ "qis_apply", "0", 0 },		// check nvrma apply from QIS. If 1, reboot. Reboot make sure RE/EA mode can connect another AP.
	/* audio */
	{ "audio_volume", "50", 0 },		// 0~100%
	{ "eq_cur_idx", "0", 0 },		// Equalizer: 0~9, reference ~/I2C_DRV/mt7620/wmp-n12-i2c.c
	{ "audio_led_type", "0", 0 },		// LED indicate type during the audio playing
	/* internet radio */
	{ "radio_table", "0", 0 },		// 0/1
	{ "usr_url0","",0 },
	{ "usr_url1","",0 },
	{ "usr_url2","",0 },
	{ "usr_url3","",0 },
	{ "usr_url4","",0 },
	{ "usr_url5","",0 },
	{ "usr_url6","",0 },
	{ "usr_url7","",0 },
	{ "usr_url8","",0 },
	{ "usr_url9","",0 },
	/* Touch */
	{ "touch_setting", "00100", 0 },	// Backlight only
	{ "touch_ctl_x", "0", 0 },		// 0:normal operation mode, 1:radio specific control mode
	{ "touch_ctl1_item_x", "0", 0 },	// 0:sequentially play, 1:custom play
	{ "touch_ctl1_radio_no", "1", 0 },	// internet radio no.
	{ "productid", MODEL_NAME, 0 },		// for original productid
	{ "fac_model_name", MODEL_NAME, 0 },	// new for UI, WSC ...

	/* product dependency */
	{ "sw_mode", "2", 0 },			// repeater mode
	{ "pre_sw_mode", "2", 0 },		// repeater mode
	{ "re_expressway", "0", 0 },		// express way: 0:none, 1:apclii0/ra0, 2:apcli0/rai0
	{ "re_mediabridge", "0", 0 },		// media bridge mode: 0:disable, 1:enable
	{ "re_wifipxy", "0", 0 },		// repeater mode wifi proxy feature
	{ "wl0_ssid", OOB_SSID_2G, 0 },
	{ "wl1_ssid", OOB_SSID_5G, 0 },
#if defined(CONFIG_DEFAULTS_ASUS_EAN66)
	/* Tx/Rx Stream */
	{ "wl0_HT_TxStream", "3", 0 },
	{ "wl0_HT_RxStream", "3", 0 },
	{ "wl1_HT_TxStream", "", 0 },
	{ "wl1_HT_RxStream", "", 0 },
	{ "cur_freq", "0", 0 },			// 0:2.4G, 1:5G
	{ "led_bright", "high", 0 },		// LED brightness: high/dim/off
	{ "led_ind", "flash", 0 },		// LED indication: stable/flash
#elif defined(CONFIG_DEFAULTS_ASUS_RPN12)
	/* Tx/Rx Stream */
	{ "wl0_HT_TxStream", "2", 0 },
	{ "wl0_HT_RxStream", "2", 0 },
	{ "wl1_HT_TxStream", "", 0 },
	{ "wl1_HT_RxStream", "", 0 },
#elif defined(CONFIG_DEFAULTS_ASUS_RPN14)
	/* Tx/Rx Stream */
	{ "wl0_HT_TxStream", "2", 0 },
	{ "wl0_HT_RxStream", "2", 0 },
	{ "wl1_HT_TxStream", "", 0 },
	{ "wl1_HT_RxStream", "", 0 },
#elif defined(CONFIG_DEFAULTS_ASUS_RPN53)
	/* Tx/Rx Stream */
	{ "wl0_HT_TxStream", "2", 0 },
	{ "wl0_HT_RxStream", "2", 0 },
	{ "wl1_HT_TxStream", "2", 0 },
	{ "wl1_HT_RxStream", "2", 0 },
#elif defined(CONFIG_DEFAULTS_ASUS_RPAC52)
	/* Tx/Rx Stream */
	{ "wl0_HT_TxStream", "2", 0 },
	{ "wl0_HT_RxStream", "2", 0 },
	{ "wl1_HT_TxStream", "1", 0 },
	{ "wl1_HT_RxStream", "1", 0 },
#elif defined(CONFIG_DEFAULTS_ASUS_RPAC56)
	/* Tx/Rx Stream */
	{ "wl0_HT_TxStream", "2", 0 },
	{ "wl0_HT_RxStream", "2", 0 },
	{ "wl1_HT_TxStream", "2", 0 },
	{ "wl1_HT_RxStream", "2", 0 },
#elif defined(CONFIG_DEFAULTS_ASUS_WMPN12)
	/* Tx/Rx Stream */
	{ "wl0_HT_TxStream", "2", 0 },
	{ "wl0_HT_RxStream", "2", 0 },
	{ "wl1_HT_TxStream", "", 0 },
	{ "wl1_HT_RxStream", "", 0 },
#else
#error Invalid Product!!
#endif
	{ 0, 0, 0 }
};

/* 
 * booting default
 */
struct nvram_tuple boot_defaults[] = {
	{ "success_start_service", "0", 0 },
#ifdef WPS_SUPPORT
	{ "wps_running", "0", 0 },
	{ "wps_enable", "0", 0 },
	{ "wps_start_flag", "0", 0 },
#endif
	{ "rc_service", "", 0 },
#ifdef ASUS_ATE
	{ "asus_mfg", "0", 0 },
	{ "btn_rst", "0", 0 },
	{ "btn_ez", "0", 0 },
#ifdef TOUCH_SUPPORT
	{ "btn_touch", "0", 0 },
#endif
#ifdef AUDIO_JACK_SUPPORT
	{ "btn_jack", "0", 0 },
#endif
#endif
	{ "wl_unit", "0", 0 },
	{ "ntp_sync", "0", 0 },
	{ "webs_state_upgrade", "2", 0 },	/* auto firmware upgrade */
	{ "en_networkmap", "0", 0 },		/* enable/disable networkmap */
	{ "networkmap_fullscan", "0", 0 },
	/* audio */
	{ "radio_total", "0", 0 },
	{ "radio_preparing", "0", 0 },		/* fix if the player die & crash in preparing stage */
	{ "audio_mute", "0", 0 },		/* turn on audio as default when boot up, bcz we have many audio source to play... */
	{ "audio_playing", "0", 0 },		/* 0/1: audio stop/play */
	{ "now_play", "999", 0 },		/* reset to initial value when boot up */
	{ "nvram_kbps", "0", 0 },		/* kbps status */
	{ "invalid_url", "0", 0 },		/* radio url is invalid */
	{ "shair_vol", "0", 0 },		/* shairport volume status */
	{ "wan_dns_t", "", 0 },
	{ "wan_state", "0", 0 },		/* 0:disconnect
						 * 1:connect
						 * 2:IP conflict
						 * 3:lost gateway (static IP)
						 * 4:ppp failed to authenticate
						 */
	{ "wan_subnet_t", "", 0 },
	{ "lan_subnet_t", "", 0 },
	{ "wl0_reload_svc_wl", "0", 0 },
	{ "wl1_reload_svc_wl", "0", 0 },
	{ "webs_state_info", "", 0 },
#ifdef LED_DRV_SUPPORT
	{ "led_drv_stage", "0", 0 },		/* reference NonGPL/drivers/LED_DRV/include/led-drv.h */
	{ "led_drv_ate", "0", 0 },		/* reference NonGPL/drivers/LED_DRV/include/led-drv.h */
#endif
#if defined(CONFIG_DEFAULTS_ASUS_EAN66)
	/* Wireless interface */
	{ "wl_ifnames", "ra0", 0 },		// 1st:2.4G
	{ "sta_ifnames", "apcli0", 0 },		// 1st:2.4G
	/* GUI support */
	{ "www_support", "AP EA QISAD QISOP NONCC LEDCTL", 0 },
#elif defined(CONFIG_DEFAULTS_ASUS_RPN12)
	/* Wireless interface */
	{ "wl_ifnames", "ra0", 0 },		// 1st:2.4G
	{ "sta_ifnames", "apcli0", 0 },		// 1st:2.4G
	/* GUI support */
	{ "www_support", "REmb AP CPW WEBSITE WPXY", 0 },
#elif defined(CONFIG_DEFAULTS_ASUS_RPN14)
	/* Wireless interface */
	{ "wl_ifnames", "ra0", 0 },		// 1st:2.4G
	{ "sta_ifnames", "apcli0", 0 },		// 1st:2.4G
	/* GUI support */
	{ "www_support", "REmb AP QISAD QISOP CPW TOUCH IR WPXY", 0 },
#elif defined(CONFIG_DEFAULTS_ASUS_RPN53)
	/* Wireless interface */
	{ "wl_ifnames", "ra0 rai0", 0 },	// 1st:2.4G, 2nd:5G
	{ "sta_ifnames", "apcli0 apclii0", 0 },	// 1st:2.4G, 2nd:5G
	/* GUI support */
	{ "www_support", "5G QISAD QISOP REew REmb AP CPW TOUCH IR WPXY", 0 },
#elif defined(CONFIG_DEFAULTS_ASUS_RPAC52)
	/* Wireless interface */
	{ "wl_ifnames", "ra0 rai0", 0 },	// 1st:2.4G, 2nd:5G
	{ "sta_ifnames", "apcli0 apclii0", 0 },	// 1st:2.4G, 2nd:5G
	/* GUI support */
	{ "www_support", "5G QISAD QISOP 11AC REew REmb AP CPW TOUCH IR WPXY", 0 },
#elif defined(CONFIG_DEFAULTS_ASUS_RPAC56)
	/* Wireless interface */
	{ "wl_ifnames", "ra0 rai0", 0 },	// 1st:2.4G, 2nd:5G
	{ "sta_ifnames", "apcli0 apclii0", 0 },	// 1st:2.4G, 2nd:5G
	/* GUI support */
	{ "www_support", "5G QISAD QISOP 11AC REew REmb AP CPW WPXY", 0 },
#elif defined(CONFIG_DEFAULTS_ASUS_WMPN12)
	/* Wireless interface */
	{ "wl_ifnames", "ra0", 0 },		// 1st:2.4G
	{ "sta_ifnames", "apcli0", 0 },		// 1st:2.4G
	/* GUI support */
	{ "www_support", "REv2 CPW IR EQ", 0 },
#else
#error Invalid Product!!
#endif

	{ 0, 0, 0 }
};
