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

/* This is the definition of the events that drive the State Machines */

#ifndef	SMEVENT_H
#define SMEVENT_H

static const char * const smEvent_names[] =
{
    "PacketRcvd",
    "DiscoverRcvd",
    "EmitRcvd",
    "ResetRcvd",
    "BlockTimeout",
    "ChargeTimeout",
    "EmitTimeout",
    "HelloDelayTimeout",
    "InactivityTimeout",
    "InvalidPacket"
};

typedef enum {
    evtPacketRcvd,
    evtDiscoverRcvd,
    evtEmitRcvd,
    evtResetRcvd,
    evtBlockTimeout,
    evtChargeTimeout,
    evtEmitTimeout,
    evtHelloDelayTimeout,
    evtInactivityTimeout,
    evtInvalidPacket		// marks the first of the invalid events
} smEvent_t;

typedef struct {
    smEvent_t	evtType;
    session_t  *ssn;		// NULL if unknown or not applicable
    /* Knowledge of the packet, hoisted here to preclude multiple re-interpretations */
    bool_t	   isNewSession;
    bool_t	   isAckingMe;
    bool_t	   isInternalEvt; // faked, as differing from a real packet or timeout
    uint	   numDescrs;     // For an Emit packet, the count of its descriptors
} protocol_event_t;

#endif /*** SMEVENT_H ***/
