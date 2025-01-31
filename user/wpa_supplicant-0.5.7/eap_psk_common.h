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
 * EAP server/peer: EAP-PSK shared routines
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

#ifndef EAP_PSK_COMMON_H
#define EAP_PSK_COMMON_H


#define EAP_PSK_RAND_LEN 16
#define EAP_PSK_MAC_LEN 16
#define EAP_PSK_TEK_LEN 16
#define EAP_PSK_PSK_LEN 16
#define EAP_PSK_AK_LEN 16
#define EAP_PSK_KDK_LEN 16

#define EAP_PSK_R_FLAG_CONT 1
#define EAP_PSK_R_FLAG_DONE_SUCCESS 2
#define EAP_PSK_R_FLAG_DONE_FAILURE 3
#define EAP_PSK_E_FLAG 0x20

#define EAP_PSK_FLAGS_GET_T(flags) (((flags) & 0xc0) >> 6)
#define EAP_PSK_FLAGS_SET_T(t) ((u8) (t) << 6)

#ifdef _MSC_VER
#pragma pack(push, 1)
#endif /* _MSC_VER */

/* Shared prefix for all EAP-PSK frames */
struct eap_psk_hdr {
	u8 code;
	u8 identifier;
	u16 length; /* including code, identifier, and length */
	u8 type; /* EAP_TYPE_PSK */
	u8 flags;
} STRUCT_PACKED;

/* EAP-PSK First Message (AS -> Supplicant) */
struct eap_psk_hdr_1 {
	u8 code;
	u8 identifier;
	u16 length; /* including code, identifier, and length */
	u8 type; /* EAP_TYPE_PSK */
	u8 flags;
	u8 rand_s[EAP_PSK_RAND_LEN];
	/* Followed by variable length ID_S */
} STRUCT_PACKED;

/* EAP-PSK Second Message (Supplicant -> AS) */
struct eap_psk_hdr_2 {
	u8 code;
	u8 identifier;
	u16 length; /* including code, identifier, and length */
	u8 type; /* EAP_TYPE_PSK */
	u8 flags;
	u8 rand_s[EAP_PSK_RAND_LEN];
	u8 rand_p[EAP_PSK_RAND_LEN];
	u8 mac_p[EAP_PSK_MAC_LEN];
	/* Followed by variable length ID_P */
} STRUCT_PACKED;

/* EAP-PSK Third Message (AS -> Supplicant) */
struct eap_psk_hdr_3 {
	u8 code;
	u8 identifier;
	u16 length; /* including code, identifier, and length */
	u8 type; /* EAP_TYPE_PSK */
	u8 flags;
	u8 rand_s[EAP_PSK_RAND_LEN];
	u8 mac_s[EAP_PSK_MAC_LEN];
	/* Followed by variable length PCHANNEL */
} STRUCT_PACKED;

/* EAP-PSK Fourth Message (Supplicant -> AS) */
struct eap_psk_hdr_4 {
	u8 code;
	u8 identifier;
	u16 length; /* including code, identifier, and length */
	u8 type; /* EAP_TYPE_PSK */
	u8 flags;
	u8 rand_s[EAP_PSK_RAND_LEN];
	/* Followed by variable length PCHANNEL */
} STRUCT_PACKED;

#ifdef _MSC_VER
#pragma pack(pop)
#endif /* _MSC_VER */


void eap_psk_key_setup(const u8 *psk, u8 *ak, u8 *kdk);
void eap_psk_derive_keys(const u8 *kdk, const u8 *rand_p, u8 *tek, u8 *msk,
			 u8 *emsk);

#endif /* EAP_PSK_COMMON_H */
