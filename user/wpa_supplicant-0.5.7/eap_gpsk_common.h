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
 * EAP server/peer: EAP-GPSK shared routines
 * Copyright (c) 2006, Jouni Malinen <jkmaline@cc.hut.fi>
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

#ifndef EAP_GPSK_COMMON_H
#define EAP_GPSK_COMMON_H

#define EAP_GPSK_OPCODE_GPSK_1 1
#define EAP_GPSK_OPCODE_GPSK_2 2
#define EAP_GPSK_OPCODE_GPSK_3 3
#define EAP_GPSK_OPCODE_GPSK_4 4

#define EAP_GPSK_RAND_LEN 32
#define EAP_GPSK_MAX_SK_LEN 32
#define EAP_GPSK_MAX_PK_LEN 32
#define EAP_GPSK_MAX_MIC_LEN 32

#define EAP_GPSK_VENDOR_IETF		0x000000
#define EAP_GPSK_CIPHER_RESERVED	0x000000
#define EAP_GPSK_CIPHER_AES		0x000001
#define EAP_GPSK_CIPHER_SHA256		0x000002


#ifdef _MSC_VER
#pragma pack(push, 1)
#endif /* _MSC_VER */

struct eap_gpsk_csuite {
	u8 vendor[3];
	u8 specifier[3];
} STRUCT_PACKED;

#ifdef _MSC_VER
#pragma pack(pop)
#endif /* _MSC_VER */

int eap_gpsk_supported_ciphersuite(int vendor, int specifier);
int eap_gpsk_derive_keys(const u8 *psk, size_t psk_len, int vendor,
			 int specifier,
			 const u8 *rand_client, const u8 *rand_server,
			 const u8 *id_client, size_t id_client_len,
			 const u8 *id_server, size_t id_server_len,
			 u8 *msk, u8 *emsk, u8 *sk, size_t *sk_len,
			 u8 *pk, size_t *pk_len);
size_t eap_gpsk_mic_len(int vendor, int specifier);
int eap_gpsk_compute_mic(const u8 *sk, size_t sk_len, int vendor,
			 int specifier, const u8 *data, size_t len, u8 *mic);

#endif /* EAP_GPSK_COMMON_H */
