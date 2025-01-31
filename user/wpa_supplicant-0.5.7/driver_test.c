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
 * WPA Supplicant - testing driver interface
 * Copyright (c) 2004-2006, Jouni Malinen <jkmaline@cc.hut.fi>
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
#include <sys/un.h>
#include <dirent.h>
#include <sys/stat.h>

#include "common.h"
#include "driver.h"
#include "wpa_supplicant.h"
#include "l2_packet.h"
#include "eloop.h"
#include "sha1.h"
#include "wpa.h"
#include "mlme.h"


struct wpa_driver_test_data {
	void *ctx;
	u8 own_addr[ETH_ALEN];
	int test_socket;
	struct sockaddr_un hostapd_addr;
	int hostapd_addr_set;
	char *own_socket_path;
	char *test_dir;
	u8 bssid[ETH_ALEN];
	u8 ssid[32];
	size_t ssid_len;
#define MAX_SCAN_RESULTS 30
	struct wpa_scan_result scanres[MAX_SCAN_RESULTS];
	size_t num_scanres;
	int use_associnfo;
	u8 assoc_wpa_ie[80];
	size_t assoc_wpa_ie_len;
	int use_mlme;
};


static int wpa_driver_test_set_wpa(void *priv, int enabled)
{
	wpa_printf(MSG_DEBUG, "%s: enabled=%d", __func__, enabled);
	return 0;
}


static void wpa_driver_test_scan_timeout(void *eloop_ctx, void *timeout_ctx)
{
	wpa_printf(MSG_DEBUG, "Scan timeout - try to get results");
	wpa_supplicant_event(timeout_ctx, EVENT_SCAN_RESULTS, NULL);
}


static void wpa_driver_scan_dir(struct wpa_driver_test_data *drv,
				const char *path)
{
	struct dirent *dent;
	DIR *dir;
	struct sockaddr_un addr;

	dir = opendir(path);
	if (dir == NULL)
		return;

	while ((dent = readdir(dir))) {
		if (os_strncmp(dent->d_name, "AP-", 3) != 0)
			continue;
		wpa_printf(MSG_DEBUG, "%s: SCAN %s", __func__, dent->d_name);

		os_memset(&addr, 0, sizeof(addr));
		addr.sun_family = AF_UNIX;
		os_snprintf(addr.sun_path, sizeof(addr.sun_path), "%s/%s",
			    path, dent->d_name);

		if (sendto(drv->test_socket, "SCAN", 4, 0,
			   (struct sockaddr *) &addr, sizeof(addr)) < 0) {
			perror("sendto(test_socket)");
		}
	}
	closedir(dir);
}


static int wpa_driver_test_scan(void *priv, const u8 *ssid, size_t ssid_len)
{
	struct wpa_driver_test_data *drv = priv;
	wpa_printf(MSG_DEBUG, "%s: priv=%p", __func__, priv);

	drv->num_scanres = 0;

	if (drv->test_socket >= 0 && drv->test_dir)
		wpa_driver_scan_dir(drv, drv->test_dir);

	if (drv->test_socket >= 0 && drv->hostapd_addr_set &&
	    sendto(drv->test_socket, "SCAN", 4, 0,
		   (struct sockaddr *) &drv->hostapd_addr,
		   sizeof(drv->hostapd_addr)) < 0) {
		perror("sendto(test_socket)");
	}

	eloop_register_timeout(1, 0, wpa_driver_test_scan_timeout, drv,
			       drv->ctx);
	return 0;
}


static int wpa_driver_test_get_scan_results(void *priv,
					    struct wpa_scan_result *results,
					    size_t max_size)
{
	struct wpa_driver_test_data *drv = priv;
	size_t num = drv->num_scanres;
	if (num > max_size)
		num = max_size;
	os_memcpy(results, &drv->scanres,
		  num * sizeof(struct wpa_scan_result));
	return num;
}


static int wpa_driver_test_set_key(void *priv, wpa_alg alg, const u8 *addr,
				   int key_idx, int set_tx,
				   const u8 *seq, size_t seq_len,
				   const u8 *key, size_t key_len)
{
	wpa_printf(MSG_DEBUG, "%s: priv=%p alg=%d key_idx=%d set_tx=%d",
		   __func__, priv, alg, key_idx, set_tx);
	if (addr) {
		wpa_printf(MSG_DEBUG, "   addr=" MACSTR, MAC2STR(addr));
	}
	if (seq) {
		wpa_hexdump(MSG_DEBUG, "   seq", seq, seq_len);
	}
	if (key) {
		wpa_hexdump(MSG_DEBUG, "   key", key, key_len);
	}
	return 0;
}


static int wpa_driver_test_associate(
	void *priv, struct wpa_driver_associate_params *params)
{
	struct wpa_driver_test_data *drv = priv;
	wpa_printf(MSG_DEBUG, "%s: priv=%p freq=%d pairwise_suite=%d "
		   "group_suite=%d key_mgmt_suite=%d auth_alg=%d mode=%d",
		   __func__, priv, params->freq, params->pairwise_suite,
		   params->group_suite, params->key_mgmt_suite,
		   params->auth_alg, params->mode);
	if (params->bssid) {
		wpa_printf(MSG_DEBUG, "   bssid=" MACSTR,
			   MAC2STR(params->bssid));
	}
	if (params->ssid) {
		wpa_hexdump_ascii(MSG_DEBUG, "   ssid",
				  params->ssid, params->ssid_len);
	}
	if (params->wpa_ie) {
		wpa_hexdump(MSG_DEBUG, "   wpa_ie",
			    params->wpa_ie, params->wpa_ie_len);
		drv->assoc_wpa_ie_len = params->wpa_ie_len;
		if (drv->assoc_wpa_ie_len > sizeof(drv->assoc_wpa_ie))
			drv->assoc_wpa_ie_len = sizeof(drv->assoc_wpa_ie);
		os_memcpy(drv->assoc_wpa_ie, params->wpa_ie,
			  drv->assoc_wpa_ie_len);
	} else
		drv->assoc_wpa_ie_len = 0;

	if (drv->test_dir && params->bssid) {
		os_memset(&drv->hostapd_addr, 0, sizeof(drv->hostapd_addr));
		drv->hostapd_addr.sun_family = AF_UNIX;
		os_snprintf(drv->hostapd_addr.sun_path,
			    sizeof(drv->hostapd_addr.sun_path),
			    "%s/AP-" MACSTR,
			    drv->test_dir, MAC2STR(params->bssid));
		drv->hostapd_addr_set = 1;
	}

	if (drv->test_socket >= 0 && drv->hostapd_addr_set) {
		char cmd[200], *pos, *end;
		int ret;
		end = cmd + sizeof(cmd);
		pos = cmd;
		ret = os_snprintf(pos, end - pos, "ASSOC " MACSTR " ",
				  MAC2STR(drv->own_addr));
		if (ret >= 0 && ret < end - pos)
			pos += ret;
		pos += wpa_snprintf_hex(pos, end - pos, params->ssid,
					params->ssid_len);
		ret = os_snprintf(pos, end - pos, " ");
		if (ret >= 0 && ret < end - pos)
			pos += ret;
		pos += wpa_snprintf_hex(pos, end - pos, params->wpa_ie,
					params->wpa_ie_len);
		end[-1] = '\0';
		if (sendto(drv->test_socket, cmd, os_strlen(cmd), 0,
			   (struct sockaddr *) &drv->hostapd_addr,
			   sizeof(drv->hostapd_addr)) < 0) {
			perror("sendto(test_socket)");
			return -1;
		}

		os_memcpy(drv->ssid, params->ssid, params->ssid_len);
		drv->ssid_len = params->ssid_len;
	} else
		wpa_supplicant_event(drv->ctx, EVENT_ASSOC, NULL);

	return 0;
}


static int wpa_driver_test_get_bssid(void *priv, u8 *bssid)
{
	struct wpa_driver_test_data *drv = priv;
	os_memcpy(bssid, drv->bssid, ETH_ALEN);
	return 0;
}


static int wpa_driver_test_get_ssid(void *priv, u8 *ssid)
{
	struct wpa_driver_test_data *drv = priv;
	os_memcpy(ssid, drv->ssid, 32);
	return drv->ssid_len;
}


static int wpa_driver_test_send_disassoc(struct wpa_driver_test_data *drv)
{
	if (drv->test_socket >= 0 &&
	    sendto(drv->test_socket, "DISASSOC", 8, 0,
		   (struct sockaddr *) &drv->hostapd_addr,
		   sizeof(drv->hostapd_addr)) < 0) {
		perror("sendto(test_socket)");
		return -1;
	}
	return 0;
}


static int wpa_driver_test_deauthenticate(void *priv, const u8 *addr,
					  int reason_code)
{
	struct wpa_driver_test_data *drv = priv;
	wpa_printf(MSG_DEBUG, "%s addr=" MACSTR " reason_code=%d",
		   __func__, MAC2STR(addr), reason_code);
	os_memset(drv->bssid, 0, ETH_ALEN);
	wpa_supplicant_event(drv->ctx, EVENT_DISASSOC, NULL);
	return wpa_driver_test_send_disassoc(drv);
}


static int wpa_driver_test_disassociate(void *priv, const u8 *addr,
					int reason_code)
{
	struct wpa_driver_test_data *drv = priv;
	wpa_printf(MSG_DEBUG, "%s addr=" MACSTR " reason_code=%d",
		   __func__, MAC2STR(addr), reason_code);
	os_memset(drv->bssid, 0, ETH_ALEN);
	wpa_supplicant_event(drv->ctx, EVENT_DISASSOC, NULL);
	return wpa_driver_test_send_disassoc(drv);
}


static void wpa_driver_test_scanresp(struct wpa_driver_test_data *drv,
				     struct sockaddr_un *from,
				     socklen_t fromlen,
				     const char *data)
{
	struct wpa_scan_result *res;
	const char *pos, *pos2;
	size_t len;
	u8 ie[200], *ipos, *end;

	wpa_printf(MSG_DEBUG, "test_driver: SCANRESP %s", data);
	if (drv->num_scanres >= MAX_SCAN_RESULTS) {
		wpa_printf(MSG_DEBUG, "test_driver: No room for the new scan "
			   "result");
		return;
	}

	/* SCANRESP BSSID SSID IEs */
	res = &drv->scanres[drv->num_scanres];

	os_memset(res, 0, sizeof(*res));
	if (hwaddr_aton(data, res->bssid)) {
		wpa_printf(MSG_DEBUG, "test_driver: invalid BSSID in scanres");
		return;
	}

	pos = data + 17;
	while (*pos == ' ')
		pos++;
	pos2 = os_strchr(pos, ' ');
	if (pos2 == NULL) {
		wpa_printf(MSG_DEBUG, "test_driver: invalid SSID termination "
			   "in scanres");
		return;
	}
	len = (pos2 - pos) / 2;
	if (len > sizeof(res->ssid))
		len = sizeof(res->ssid);
	if (hexstr2bin(pos, res->ssid, len) < 0) {
		wpa_printf(MSG_DEBUG, "test_driver: invalid SSID in scanres");
		return;
	}
	res->ssid_len = len;

	pos = pos2 + 1;
	pos2 = os_strchr(pos, ' ');
	if (pos2 == NULL)
		len = os_strlen(pos) / 2;
	else
		len = (pos2 - pos) / 2;
	if (len > sizeof(ie))
		len = sizeof(ie);
	if (hexstr2bin(pos, ie, len) < 0) {
		wpa_printf(MSG_DEBUG, "test_driver: invalid IEs in scanres");
		return;
	}

	ipos = ie;
	end = ipos + len;
	while (ipos + 1 < end && ipos + 2 + ipos[1] <= end) {
		len = 2 + ipos[1];
		if (len > SSID_MAX_WPA_IE_LEN)
			len = SSID_MAX_WPA_IE_LEN;
		if (ipos[0] == RSN_INFO_ELEM) {
			os_memcpy(res->rsn_ie, ipos, len);
			res->rsn_ie_len = len;
		} else if (ipos[0] == GENERIC_INFO_ELEM) {
			os_memcpy(res->wpa_ie, ipos, len);
			res->wpa_ie_len = len;
		}

		ipos += 2 + ipos[1];
	}

	if (pos2) {
		pos = pos2 + 1;
		while (*pos == ' ')
			pos++;
		if (os_strncmp(pos, "PRIVACY", 7) == 0)
			res->caps |= IEEE80211_CAP_PRIVACY;
	}

	drv->num_scanres++;
}


static void wpa_driver_test_assocresp(struct wpa_driver_test_data *drv,
				      struct sockaddr_un *from,
				      socklen_t fromlen,
				      const char *data)
{
	/* ASSOCRESP BSSID <res> */
	if (hwaddr_aton(data, drv->bssid)) {
		wpa_printf(MSG_DEBUG, "test_driver: invalid BSSID in "
			   "assocresp");
	}
	if (drv->use_associnfo) {
		union wpa_event_data event;
		os_memset(&event, 0, sizeof(event));
		event.assoc_info.req_ies = drv->assoc_wpa_ie;
		event.assoc_info.req_ies_len = drv->assoc_wpa_ie_len;
		wpa_supplicant_event(drv->ctx, EVENT_ASSOCINFO, &event);
	}
	wpa_supplicant_event(drv->ctx, EVENT_ASSOC, NULL);
}


static void wpa_driver_test_disassoc(struct wpa_driver_test_data *drv,
				     struct sockaddr_un *from,
				     socklen_t fromlen)
{
	wpa_supplicant_event(drv->ctx, EVENT_DISASSOC, NULL);
}


static void wpa_driver_test_eapol(struct wpa_driver_test_data *drv,
				  struct sockaddr_un *from,
				  socklen_t fromlen,
				  const u8 *data, size_t data_len)
{
	const u8 *src = drv->bssid;

	if (data_len > 14) {
		/* Skip Ethernet header */
		src = data + ETH_ALEN;
		data += 14;
		data_len -= 14;
	}
	wpa_supplicant_rx_eapol(drv->ctx, src, data, data_len);
}


static void wpa_driver_test_mlme(struct wpa_driver_test_data *drv,
				 struct sockaddr_un *from,
				 socklen_t fromlen,
				 const u8 *data, size_t data_len)
{
	struct ieee80211_rx_status rx_status;
	os_memset(&rx_status, 0, sizeof(rx_status));
	ieee80211_sta_rx(drv->ctx, data, data_len, &rx_status);
}


static void wpa_driver_test_receive_unix(int sock, void *eloop_ctx,
					 void *sock_ctx)
{
	struct wpa_driver_test_data *drv = eloop_ctx;
	char *buf;
	int res;
	struct sockaddr_un from;
	socklen_t fromlen = sizeof(from);
	const size_t buflen = 2000;

	buf = os_malloc(buflen);
	if (buf == NULL)
		return;
	res = recvfrom(sock, buf, buflen - 1, 0,
		       (struct sockaddr *) &from, &fromlen);
	if (res < 0) {
		perror("recvfrom(test_socket)");
		os_free(buf);
		return;
	}
	buf[res] = '\0';

	wpa_printf(MSG_DEBUG, "test_driver: received %u bytes", res);

	if (os_strncmp(buf, "SCANRESP ", 9) == 0) {
		wpa_driver_test_scanresp(drv, &from, fromlen, buf + 9);
	} else if (os_strncmp(buf, "ASSOCRESP ", 10) == 0) {
		wpa_driver_test_assocresp(drv, &from, fromlen, buf + 10);
	} else if (os_strcmp(buf, "DISASSOC") == 0) {
		wpa_driver_test_disassoc(drv, &from, fromlen);
	} else if (os_strcmp(buf, "DEAUTH") == 0) {
		wpa_driver_test_disassoc(drv, &from, fromlen);
	} else if (os_strncmp(buf, "EAPOL ", 6) == 0) {
		wpa_driver_test_eapol(drv, &from, fromlen,
				      (const u8 *) buf + 6, res - 6);
	} else if (os_strncmp(buf, "MLME ", 5) == 0) {
		wpa_driver_test_mlme(drv, &from, fromlen,
				     (const u8 *) buf + 5, res - 5);
	} else {
		wpa_hexdump_ascii(MSG_DEBUG, "Unknown test_socket command",
				  (u8 *) buf, res);
	}
	os_free(buf);
}


static void * wpa_driver_test_init(void *ctx, const char *ifname)
{
	struct wpa_driver_test_data *drv;

	drv = os_zalloc(sizeof(*drv));
	if (drv == NULL)
		return NULL;
	drv->ctx = ctx;
	drv->test_socket = -1;

	/* Set dummy BSSID and SSID for testing. */
	drv->bssid[0] = 0x02;
	drv->bssid[1] = 0x00;
	drv->bssid[2] = 0x00;
	drv->bssid[3] = 0x00;
	drv->bssid[4] = 0x00;
	drv->bssid[5] = 0x01;
	os_memcpy(drv->ssid, "test", 5);
	drv->ssid_len = 4;

	/* Generate a MAC address to help testing with multiple STAs */
	drv->own_addr[0] = 0x02; /* locally administered */
	sha1_prf((const u8 *) ifname, os_strlen(ifname),
		 "wpa_supplicant test mac addr generation",
		 NULL, 0, drv->own_addr + 1, ETH_ALEN - 1);

	return drv;
}


static void wpa_driver_test_close_test_socket(struct wpa_driver_test_data *drv)
{
	if (drv->test_socket >= 0) {
		eloop_unregister_read_sock(drv->test_socket);
		close(drv->test_socket);
		drv->test_socket = -1;
	}

	if (drv->own_socket_path) {
		unlink(drv->own_socket_path);
		os_free(drv->own_socket_path);
		drv->own_socket_path = NULL;
	}
}


static void wpa_driver_test_deinit(void *priv)
{
	struct wpa_driver_test_data *drv = priv;
	wpa_driver_test_close_test_socket(drv);
	eloop_cancel_timeout(wpa_driver_test_scan_timeout, drv, drv->ctx);
	os_free(drv->test_dir);
	os_free(drv);
}


static int wpa_driver_test_attach(struct wpa_driver_test_data *drv,
				  const char *dir)
{
	static unsigned int counter = 0;
	struct sockaddr_un addr;
	size_t len;

	os_free(drv->own_socket_path);
	if (dir) {
		len = os_strlen(dir) + 30;
		drv->own_socket_path = os_malloc(len);
		if (drv->own_socket_path == NULL)
			return -1;
		os_snprintf(drv->own_socket_path, len, "%s/STA-" MACSTR,
			    dir, MAC2STR(drv->own_addr));
	} else {
		drv->own_socket_path = os_malloc(100);
		if (drv->own_socket_path == NULL)
			return -1;
		os_snprintf(drv->own_socket_path, 100,
			    "/tmp/wpa_supplicant_test-%d-%d",
			    getpid(), counter++);
	}

	drv->test_socket = socket(PF_UNIX, SOCK_DGRAM, 0);
	if (drv->test_socket < 0) {
		perror("socket(PF_UNIX)");
		os_free(drv->own_socket_path);
		drv->own_socket_path = NULL;
		return -1;
	}

	os_memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	os_strncpy(addr.sun_path, drv->own_socket_path, sizeof(addr.sun_path));
	if (bind(drv->test_socket, (struct sockaddr *) &addr,
		 sizeof(addr)) < 0) {
		perror("bind(PF_UNIX)");
		close(drv->test_socket);
		unlink(drv->own_socket_path);
		os_free(drv->own_socket_path);
		drv->own_socket_path = NULL;
		return -1;
	}

	eloop_register_read_sock(drv->test_socket,
				 wpa_driver_test_receive_unix, drv, NULL);

	return 0;
}


static int wpa_driver_test_set_param(void *priv, const char *param)
{
	struct wpa_driver_test_data *drv = priv;
	const char *pos, *pos2;
	size_t len;

	wpa_printf(MSG_DEBUG, "%s: param='%s'", __func__, param);
	if (param == NULL)
		return 0;

	wpa_driver_test_close_test_socket(drv);
	pos = os_strstr(param, "test_socket=");
	if (pos) {
		pos += 12;
		pos2 = os_strchr(pos, ' ');
		if (pos2)
			len = pos2 - pos;
		else
			len = os_strlen(pos);
		if (len > sizeof(drv->hostapd_addr.sun_path))
			return -1;
		os_memset(&drv->hostapd_addr, 0, sizeof(drv->hostapd_addr));
		drv->hostapd_addr.sun_family = AF_UNIX;
		os_memcpy(drv->hostapd_addr.sun_path, pos, len);
		drv->hostapd_addr_set = 1;
	}

	pos = os_strstr(param, "test_dir=");
	if (pos) {
		char *end;
		os_free(drv->test_dir);
		drv->test_dir = os_strdup(pos + 9);
		if (drv->test_dir == NULL)
			return -1;
		end = os_strchr(drv->test_dir, ' ');
		if (end)
			*end = '\0';
		wpa_driver_test_attach(drv, drv->test_dir);
	} else
		wpa_driver_test_attach(drv, NULL);

	if (os_strstr(param, "use_associnfo=1")) {
		wpa_printf(MSG_DEBUG, "test_driver: Use AssocInfo events");
		drv->use_associnfo = 1;
	}

#ifdef CONFIG_CLIENT_MLME
	if (os_strstr(param, "use_mlme=1")) {
		wpa_printf(MSG_DEBUG, "test_driver: Use internal MLME");
		drv->use_mlme = 1;
	}
#endif /* CONFIG_CLIENT_MLME */

	return 0;
}


static const u8 * wpa_driver_test_get_mac_addr(void *priv)
{
	struct wpa_driver_test_data *drv = priv;
	wpa_printf(MSG_DEBUG, "%s", __func__);
	return drv->own_addr;
}


static int wpa_driver_test_send_eapol(void *priv, const u8 *dest, u16 proto,
				      const u8 *data, size_t data_len)
{
	struct wpa_driver_test_data *drv = priv;
	struct msghdr msg;
	struct iovec io[3];
	struct l2_ethhdr eth;
	struct sockaddr_un addr;

	wpa_hexdump(MSG_MSGDUMP, "test_send_eapol TX frame", data, data_len);

	os_memset(&eth, 0, sizeof(eth));
	os_memcpy(eth.h_dest, dest, ETH_ALEN);
	os_memcpy(eth.h_source, drv->own_addr, ETH_ALEN);
	eth.h_proto = host_to_be16(proto);

	io[0].iov_base = "EAPOL ";
	io[0].iov_len = 6;
	io[1].iov_base = (u8 *) &eth;
	io[1].iov_len = sizeof(eth);
	io[2].iov_base = (u8 *) data;
	io[2].iov_len = data_len;

	os_memset(&msg, 0, sizeof(msg));
	msg.msg_iov = io;
	msg.msg_iovlen = 3;
	if (os_memcmp(dest, drv->bssid, ETH_ALEN) == 0 ||
	    drv->test_dir == NULL) {
		msg.msg_name = &drv->hostapd_addr;
		msg.msg_namelen = sizeof(drv->hostapd_addr);
	} else {
		struct stat st;
		os_memset(&addr, 0, sizeof(addr));
		addr.sun_family = AF_UNIX;
		os_snprintf(addr.sun_path, sizeof(addr.sun_path),
			    "%s/STA-" MACSTR, drv->test_dir, MAC2STR(dest));
		if (stat(addr.sun_path, &st) < 0) {
			os_snprintf(addr.sun_path, sizeof(addr.sun_path),
				    "%s/AP-" MACSTR,
				    drv->test_dir, MAC2STR(dest));
		}
		msg.msg_name = &addr;
		msg.msg_namelen = sizeof(addr);
	}

	if (sendmsg(drv->test_socket, &msg, 0) < 0) {
		perror("sendmsg(test_socket)");
		return -1;
	}

	return 0;
}


static int wpa_driver_test_get_capa(void *priv, struct wpa_driver_capa *capa)
{
	struct wpa_driver_test_data *drv = priv;
	os_memset(capa, 0, sizeof(*capa));
	capa->key_mgmt = WPA_DRIVER_CAPA_KEY_MGMT_WPA |
		WPA_DRIVER_CAPA_KEY_MGMT_WPA2 |
		WPA_DRIVER_CAPA_KEY_MGMT_WPA_PSK |
		WPA_DRIVER_CAPA_KEY_MGMT_WPA2_PSK |
		WPA_DRIVER_CAPA_KEY_MGMT_WPA_NONE;
	capa->enc = WPA_DRIVER_CAPA_ENC_WEP40 |
		WPA_DRIVER_CAPA_ENC_WEP104 |
		WPA_DRIVER_CAPA_ENC_TKIP |
		WPA_DRIVER_CAPA_ENC_CCMP;
	capa->auth = WPA_DRIVER_AUTH_OPEN |
		WPA_DRIVER_AUTH_SHARED |
		WPA_DRIVER_AUTH_LEAP;
	if (drv->use_mlme)
		capa->flags |= WPA_DRIVER_FLAGS_USER_SPACE_MLME;

	return 0;
}


static int wpa_driver_test_mlme_setprotection(void *priv, const u8 *addr,
					      int protect_type,
					      int key_type)
{
	wpa_printf(MSG_DEBUG, "%s: protect_type=%d key_type=%d",
		   __func__, protect_type, key_type);

	if (addr) {
		wpa_printf(MSG_DEBUG, "%s: addr=" MACSTR,
			   __func__, MAC2STR(addr));
	}

	return 0;
}


#ifdef CONFIG_CLIENT_MLME
static struct wpa_hw_modes *
wpa_driver_test_get_hw_feature_data(void *priv, u16 *num_modes, u16 *flags)
{
	struct wpa_hw_modes *modes;

	*num_modes = 1;
	*flags = 0;
	modes = os_zalloc(*num_modes * sizeof(struct wpa_hw_modes));
	if (modes == NULL)
		return NULL;
	modes[0].mode = WPA_MODE_IEEE80211G;
	modes[0].num_channels = 1;
	modes[0].num_rates = 1;
	modes[0].channels = os_zalloc(sizeof(struct wpa_channel_data));
	modes[0].rates = os_zalloc(sizeof(struct wpa_rate_data));
	if (modes[0].channels == NULL || modes[0].rates == NULL) {
		ieee80211_sta_free_hw_features(modes, *num_modes);
		return NULL;
	}
	modes[0].channels[0].chan = 1;
	modes[0].channels[0].freq = 2412;
	modes[0].channels[0].flag = WPA_CHAN_W_SCAN | WPA_CHAN_W_ACTIVE_SCAN;
	modes[0].rates[0].rate = 10;
	modes[0].rates[0].flags = WPA_RATE_BASIC | WPA_RATE_SUPPORTED |
		WPA_RATE_CCK | WPA_RATE_MANDATORY;

	return modes;
}


int wpa_driver_test_set_channel(void *priv, wpa_hw_mode phymode, int chan,
				int freq)
{
	wpa_printf(MSG_DEBUG, "%s: phymode=%d chan=%d freq=%d",
		   __func__, phymode, chan, freq);
	return 0;
}


static int wpa_driver_test_send_mlme(void *priv, const u8 *data,
				     size_t data_len)
{
	struct wpa_driver_test_data *drv = priv;
	struct msghdr msg;
	struct iovec io[2];
	struct sockaddr_un addr;
	const u8 *dest;
	struct dirent *dent;
	DIR *dir;

	wpa_hexdump(MSG_MSGDUMP, "test_send_mlme", data, data_len);
	if (data_len < 10)
		return -1;
	dest = data + 4;

	io[0].iov_base = "MLME ";
	io[0].iov_len = 5;
	io[1].iov_base = (u8 *) data;
	io[1].iov_len = data_len;

	os_memset(&msg, 0, sizeof(msg));
	msg.msg_iov = io;
	msg.msg_iovlen = 2;
	if (os_memcmp(dest, drv->bssid, ETH_ALEN) == 0 ||
	    drv->test_dir == NULL) {
		msg.msg_name = &drv->hostapd_addr;
		msg.msg_namelen = sizeof(drv->hostapd_addr);
	} else if (os_memcmp(dest, "\xff\xff\xff\xff\xff\xff", ETH_ALEN) == 0)
	{
		dir = opendir(drv->test_dir);
		if (dir == NULL)
			return -1;
		while ((dent = readdir(dir))) {
#ifdef _DIRENT_HAVE_D_TYPE
			/* Skip the file if it is not a socket.
			 * Also accept DT_UNKNOWN (0) in case
			 * the C library or underlying file
			 * system does not support d_type. */
			if (dent->d_type != DT_SOCK &&
			    dent->d_type != DT_UNKNOWN)
				continue;
#endif /* _DIRENT_HAVE_D_TYPE */
			if (os_strcmp(dent->d_name, ".") == 0 ||
			    os_strcmp(dent->d_name, "..") == 0)
				continue;
			wpa_printf(MSG_DEBUG, "%s: Send broadcast MLME to %s",
				   __func__, dent->d_name);
			os_memset(&addr, 0, sizeof(addr));
			addr.sun_family = AF_UNIX;
			os_snprintf(addr.sun_path, sizeof(addr.sun_path),
				    "%s/%s", drv->test_dir, dent->d_name);

			msg.msg_name = &addr;
			msg.msg_namelen = sizeof(addr);

			if (sendmsg(drv->test_socket, &msg, 0) < 0)
				perror("sendmsg(test_socket)");
		}
		closedir(dir);
		return 0;
	} else {
		struct stat st;
		os_memset(&addr, 0, sizeof(addr));
		addr.sun_family = AF_UNIX;
		os_snprintf(addr.sun_path, sizeof(addr.sun_path),
			    "%s/AP-" MACSTR, drv->test_dir, MAC2STR(dest));
		if (stat(addr.sun_path, &st) < 0) {
			os_snprintf(addr.sun_path, sizeof(addr.sun_path),
				    "%s/STA-" MACSTR,
				    drv->test_dir, MAC2STR(dest));
		}
		msg.msg_name = &addr;
		msg.msg_namelen = sizeof(addr);
	}

	if (sendmsg(drv->test_socket, &msg, 0) < 0) {
		perror("sendmsg(test_socket)");
		return -1;
	}

	return 0;
}


static int wpa_driver_test_mlme_add_sta(void *priv, const u8 *addr,
					const u8 *supp_rates,
					size_t supp_rates_len)
{
	wpa_printf(MSG_DEBUG, "%s: addr=" MACSTR, __func__, MAC2STR(addr));
	return 0;
}


static int wpa_driver_test_mlme_remove_sta(void *priv, const u8 *addr)
{
	wpa_printf(MSG_DEBUG, "%s: addr=" MACSTR, __func__, MAC2STR(addr));
	return 0;
}


int wpa_driver_test_set_ssid(void *priv, const u8 *ssid, size_t ssid_len)
{
	wpa_printf(MSG_DEBUG, "%s", __func__);
	return 0;
}


int wpa_driver_test_set_bssid(void *priv, const u8 *bssid)
{
	wpa_printf(MSG_DEBUG, "%s: bssid=" MACSTR, __func__, MAC2STR(bssid));
	return 0;
}
#endif /* CONFIG_CLIENT_MLME */


const struct wpa_driver_ops wpa_driver_test_ops = {
	"test",
	"wpa_supplicant test driver",
	wpa_driver_test_get_bssid,
	wpa_driver_test_get_ssid,
	wpa_driver_test_set_wpa,
	wpa_driver_test_set_key,
	wpa_driver_test_init,
	wpa_driver_test_deinit,
	wpa_driver_test_set_param,
	NULL /* set_countermeasures */,
	NULL /* set_drop_unencrypted */,
	wpa_driver_test_scan,
	wpa_driver_test_get_scan_results,
	wpa_driver_test_deauthenticate,
	wpa_driver_test_disassociate,
	wpa_driver_test_associate,
	NULL /* set_auth_alg */,
	NULL /* add_pmkid */,
	NULL /* remove_pmkid */,
	NULL /* flush_pmkid */,
	wpa_driver_test_get_capa,
	NULL /* poll */,
	NULL /* get_ifname */,
	wpa_driver_test_get_mac_addr,
	wpa_driver_test_send_eapol,
	NULL /* set_operstate */,
	wpa_driver_test_mlme_setprotection,
#ifdef CONFIG_CLIENT_MLME
	wpa_driver_test_get_hw_feature_data,
	wpa_driver_test_set_channel,
	wpa_driver_test_set_ssid,
	wpa_driver_test_set_bssid,
	wpa_driver_test_send_mlme,
	wpa_driver_test_mlme_add_sta,
	wpa_driver_test_mlme_remove_sta
#else /* CONFIG_CLIENT_MLME */
	NULL /* get_hw_feature_data */,
	NULL /* set_channel */,
	NULL /* set_ssid */,
	NULL /* set_bssid */,
	NULL /* send_mlme */,
	NULL /* mlme_add_sta */,
	NULL /* mlme_remove_sta */
#endif /* CONFIG_CLIENT_MLME */
};
