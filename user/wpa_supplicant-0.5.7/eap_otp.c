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
 * EAP peer method: EAP-OTP (RFC 3748)
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

#include "common.h"
#include "eap_i.h"
#include "config_ssid.h"


static void * eap_otp_init(struct eap_sm *sm)
{
	/* No need for private data. However, must return non-NULL to indicate
	 * success. */
	return (void *) 1;
}


static void eap_otp_deinit(struct eap_sm *sm, void *priv)
{
}


static u8 * eap_otp_process(struct eap_sm *sm, void *priv,
			    struct eap_method_ret *ret,
			    const u8 *reqData, size_t reqDataLen,
			    size_t *respDataLen)
{
	const struct eap_hdr *req;
	struct eap_hdr *resp;
	const u8 *pos, *password;
	u8 *rpos;
	size_t password_len, len;
	int otp;

	pos = eap_hdr_validate(EAP_VENDOR_IETF, EAP_TYPE_OTP,
			       reqData, reqDataLen, &len);
	if (pos == NULL) {
		ret->ignore = TRUE;
		return NULL;
	}
	req = (const struct eap_hdr *) reqData;
	wpa_hexdump_ascii(MSG_MSGDUMP, "EAP-OTP: Request message",
			  pos, len);

	password = eap_get_config_otp(sm, &password_len);
	if (password)
		otp = 1;
	else {
		password = eap_get_config_password(sm, &password_len);
		otp = 0;
	}

	if (password == NULL) {
		wpa_printf(MSG_INFO, "EAP-OTP: Password not configured");
		eap_sm_request_otp(sm, (const char *) pos, len);
		ret->ignore = TRUE;
		return NULL;
	}

	ret->ignore = FALSE;

	ret->methodState = METHOD_DONE;
	ret->decision = DECISION_COND_SUCC;
	ret->allowNotifications = FALSE;

	resp = eap_msg_alloc(EAP_VENDOR_IETF, EAP_TYPE_OTP, respDataLen,
			     password_len, EAP_CODE_RESPONSE, req->identifier,
			     &rpos);
	if (resp == NULL)
		return NULL;
	os_memcpy(rpos, password, password_len);
	wpa_hexdump_ascii_key(MSG_MSGDUMP, "EAP-OTP: Response",
			      password, password_len);

	if (otp) {
		wpa_printf(MSG_DEBUG, "EAP-OTP: Forgetting used password");
		eap_clear_config_otp(sm);
	}

	return (u8 *) resp;
}


int eap_peer_otp_register(void)
{
	struct eap_method *eap;
	int ret;

	eap = eap_peer_method_alloc(EAP_PEER_METHOD_INTERFACE_VERSION,
				    EAP_VENDOR_IETF, EAP_TYPE_OTP, "OTP");
	if (eap == NULL)
		return -1;

	eap->init = eap_otp_init;
	eap->deinit = eap_otp_deinit;
	eap->process = eap_otp_process;

	ret = eap_peer_method_register(eap);
	if (ret)
		eap_peer_method_free(eap);
	return ret;
}
