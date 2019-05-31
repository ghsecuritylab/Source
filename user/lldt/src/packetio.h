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
 * LICENSE NOTICE.
 *
 * Use of the Microsoft Windows Rally Development Kit is covered under
 * the Microsoft Windows Rally Development Kit License Agreement,
 * which is provided within the Microsoft Windows Rally Development
 * Kit or at http://www.microsoft.com/whdc/rally/rallykit.mspx. If you
 * want a license from Microsoft to use the software in the Microsoft
 * Windows Rally Development Kit, you must (1) complete the designated
 * "licensee" information in the Windows Rally Development Kit License
 * Agreement, and (2) sign and return the Agreement AS IS to Microsoft
 * at the address provided in the Agreement.
 */

/*
 * Copyright (c) Microsoft Corporation 2005.  All rights reserved.
 * This software is provided with NO WARRANTY.
 */

#ifndef PACKETIO_H
#define PACKETIO_H

extern void packetio_recv_handler(int fd, void *state);

extern void packetio_tx_hello(void);
extern void packetio_tx_queryresp(void);
extern void packetio_tx_flat(void);
extern void packetio_tx_emitee(topo_emitee_desc_t *ed);
extern void packetio_tx_ack(uint16_t thisSeqNum);
extern void packetio_tx_qltlvResp(uint16_t thisSeqNum, tlv_desc_t *tlvDescr, size_t LtlvOffset);

extern void packetio_invalidate_retxbuf(void);

// ------ for QOS ------ //
extern void  tx_write(uint8_t *buf, size_t nbytes);
extern void* fmt_base(uint8_t *buf, const etheraddr_t *srchw, const etheraddr_t *dsthw, lld2_tos_t tos,
	              topo_opcode_t g_opcode, uint16_t seqnum, bool_t use_broadcast);

#endif /* PACKETIO_H */
