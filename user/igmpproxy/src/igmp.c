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
**  igmpproxy - IGMP proxy based multicast router 
**  Copyright (C) 2005 Johnny Egeland <johnny@rlo.org>
**
**  This program is free software; you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation; either version 2 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
**
**----------------------------------------------------------------------------
**
**  This software is derived work from the following software. The original
**  source code has been modified from it's original state by the author
**  of igmpproxy.
**
**  smcroute 0.92 - Copyright (C) 2001 Carsten Schill <carsten@cschill.de>
**  - Licensed under the GNU General Public License, version 2
**  
**  mrouted 3.9-beta3 - COPYRIGHT 1989 by The Board of Trustees of 
**  Leland Stanford Junior University.
**  - Original license can be found in the "doc/mrouted-LINCESE" file.
**
*/
/**
*   igmp.h - Recieves IGMP requests, and handle them 
*            appropriately...
*/

#include "defs.h"
 
// Globals                  
uint32     allhosts_group;          /* All hosts addr in net order */
uint32     allrouters_group;          /* All hosts addr in net order */
              
extern int MRouterFD;

/*
 * Open and initialize the igmp socket, and fill in the non-changing
 * IP header fields in the output packet buffer.
 */
void initIgmp() {
    struct ip *ip;

    recv_buf = malloc(RECV_BUF_SIZE);
    send_buf = malloc(RECV_BUF_SIZE);

    k_hdr_include(TRUE);    /* include IP header when sending */
    k_set_rcvbuf(256*1024,48*1024); /* lots of input buffering        */
    k_set_ttl(1);       /* restrict multicasts to one hop */
    k_set_loop(FALSE);      /* disable multicast loopback     */

    ip         = (struct ip *)send_buf;
    bzero(ip, sizeof(struct ip));
    /*
     * Fields zeroed that aren't filled in later:
     * - IP ID (let the kernel fill it in)
     * - Offset (we don't send fragments)
     * - Checksum (let the kernel fill it in)
     */
    ip->ip_v   = IPVERSION;
    ip->ip_hl  = sizeof(struct ip) >> 2;
    ip->ip_tos = 0xc0;      /* Internet Control */
    ip->ip_ttl = MAXTTL;    /* applies to unicasts only */
    ip->ip_p   = IPPROTO_IGMP;

    allhosts_group   = htonl(INADDR_ALLHOSTS_GROUP);
    allrouters_group = htonl(INADDR_ALLRTRS_GROUP);
}

/**
*   Finds the textual name of the supplied IGMP request.
*/
char *igmpPacketKind(u_int type, u_int code) {
    static char unknown[20];

    switch (type) {
    case IGMP_MEMBERSHIP_QUERY:     return  "Membership query  ";
    case IGMP_V1_MEMBERSHIP_REPORT:  return "V1 member report  ";
    case IGMP_V2_MEMBERSHIP_REPORT:  return "V2 member report  ";
    case IGMP_V2_LEAVE_GROUP:        return "Leave message     ";
    
    default:
        sprintf(unknown, "unk: 0x%02x/0x%02x    ", type, code);
        return unknown;
    }
}


/**
 * Process a newly received IGMP packet that is sitting in the input
 * packet buffer.
 */
void acceptIgmp(int recvlen) {
    register uint32 src, dst, group;
    struct ip *ip;
    struct igmp *igmp;
    int ipdatalen, iphdrlen, igmpdatalen;

    if (recvlen < sizeof(struct ip)) {
        log(LOG_WARNING, 0,
            "received packet too short (%u bytes) for IP header", recvlen);
        return;
    }

    ip        = (struct ip *)recv_buf;
    src       = ip->ip_src.s_addr;
    dst       = ip->ip_dst.s_addr;

    //IF_DEBUG log(LOG_DEBUG, 0, "Got a IGMP request to process...");

    /* 
     * this is most likely a message from the kernel indicating that
     * a new src grp pair message has arrived and so, it would be 
     * necessary to install a route into the kernel for this.
     */
    if (ip->ip_p == 0) {
        if (src == 0 || dst == 0) {
            log(LOG_WARNING, 0, "kernel request not accurate");
        }
        else {
            struct IfDesc *checkVIF;
            
            // Check if the source address matches a valid address on upstream vif.
            checkVIF = getIfByIx( upStreamVif );
            if(checkVIF == 0) {
//              log(LOG_ERR, 0, "Upstream VIF was null.");
                log(LOG_WARNING, 0, "Upstream VIF was null.");
                return;
            } 
            else if(src == checkVIF->InAdr.s_addr) {
                log(LOG_NOTICE, 0, "Route activation request from %s for %s is from myself. Ignoring.",
                    inetFmt(src, s1), inetFmt(dst, s2));
                return;
            }
            else if(!isAdressValidForIf(checkVIF, src)) {
                log(LOG_WARNING, 0, "The source address %s for group %s, is not in any valid net for upstream VIF.",
                    inetFmt(src, s1), inetFmt(dst, s2));
                return;
            }
            
            // Activate the route.
            IF_DEBUG log(LOG_DEBUG, 0, "Route activate request from %s to %s",
                         inetFmt(src,s1), inetFmt(dst,s2));
            activateRoute(dst, src);
            

        }
        return;
    }

    iphdrlen  = ip->ip_hl << 2;
    ipdatalen = ntohs(ip->ip_len) - iphdrlen;

    if (iphdrlen + ipdatalen != recvlen) {
        log(LOG_WARNING, 0,
            "received packet from %s shorter (%u bytes) than hdr+data length (%u+%u)",
            inetFmt(src, s1), recvlen, iphdrlen, ipdatalen);
        return;
    }

    igmp        = (struct igmp *)(recv_buf + iphdrlen);
    group       = igmp->igmp_group.s_addr;
    igmpdatalen = ipdatalen - IGMP_MINLEN;
    if (igmpdatalen < 0) {
        log(LOG_WARNING, 0,
            "received IP data field too short (%u bytes) for IGMP, from %s",
            ipdatalen, inetFmt(src, s1));
        return;
    }

    log(LOG_NOTICE, 0, "RECV %s from %-15s to %s",
        igmpPacketKind(igmp->igmp_type, igmp->igmp_code),
        inetFmt(src, s1), inetFmt(dst, s2) );

    switch (igmp->igmp_type) {
    case IGMP_V1_MEMBERSHIP_REPORT:
    case IGMP_V2_MEMBERSHIP_REPORT:
        acceptGroupReport(src, group, igmp->igmp_type);
        return;
    
    case IGMP_V2_LEAVE_GROUP:
        acceptLeaveMessage(src, group);
        return;
    
        /*
    case IGMP_MEMBERSHIP_QUERY:
        //accept_membership_query(src, dst, group, igmp->igmp_code);
        return;

    */

    default:
        log(LOG_INFO, 0,
            "ignoring unknown IGMP message type %x from %s to %s",
            igmp->igmp_type, inetFmt(src, s1),
            inetFmt(dst, s2));
        return;
    }
}


/*
 * Construct an IGMP message in the output packet buffer.  The caller may
 * have already placed data in that buffer, of length 'datalen'.
 */
void buildIgmp(uint32 src, uint32 dst, int type, int code, uint32 group, int datalen) {
    struct ip *ip;
    struct igmp *igmp;
    extern int curttl;

    ip                      = (struct ip *)send_buf;
    ip->ip_src.s_addr       = src;
    ip->ip_dst.s_addr       = dst;
    ip->ip_len              = MIN_IP_HEADER_LEN + IGMP_MINLEN + datalen;
    ip->ip_len              = htons(ip->ip_len);

    if (IN_MULTICAST(ntohl(dst))) {
        ip->ip_ttl = curttl;
    } else {
        ip->ip_ttl = MAXTTL;
    }

    igmp                    = (struct igmp *)(send_buf + MIN_IP_HEADER_LEN);
    igmp->igmp_type         = type;
    igmp->igmp_code         = code;
    igmp->igmp_group.s_addr = group;
    igmp->igmp_cksum        = 0;
    igmp->igmp_cksum        = inetChksum((u_short *)igmp,
                                         IGMP_MINLEN + datalen);
}

/* 
 * Call build_igmp() to build an IGMP message in the output packet buffer.
 * Then send the message from the interface with IP address 'src' to
 * destination 'dst'.
 */
void sendIgmp(uint32 src, uint32 dst, int type, int code, uint32 group, int datalen) {
    struct sockaddr_in sdst;
    int setloop = 0, setigmpsource = 0;

    buildIgmp(src, dst, type, code, group, datalen);

    if (IN_MULTICAST(ntohl(dst))) {
        k_set_if(src);
        setigmpsource = 1;
        if (type != IGMP_DVMRP || dst == allhosts_group) {
            setloop = 1;
            k_set_loop(TRUE);
        }
    }

    bzero(&sdst, sizeof(sdst));
    sdst.sin_family = AF_INET;
#ifdef HAVE_SA_LEN
    sdst.sin_len = sizeof(sdst);
#endif
    sdst.sin_addr.s_addr = dst;
    if (sendto(MRouterFD, send_buf,
               MIN_IP_HEADER_LEN + IGMP_MINLEN + datalen, 0,
               (struct sockaddr *)&sdst, sizeof(sdst)) < 0) {
        if (errno == ENETDOWN)
            log(LOG_ERR, errno, "Sender VIF was down.");
        else
            log(LOG_INFO, errno,
                "sendto to %s on %s",
                inetFmt(dst, s1), inetFmt(src, s2));
    }

    if(setigmpsource) {
        if (setloop) {
            k_set_loop(FALSE);
        }
        // Restore original...
        k_set_if(INADDR_ANY);
    }

    IF_DEBUG log(LOG_DEBUG, 0, "SENT %s from %-15s to %s",
        igmpPacketKind(type, code), src == INADDR_ANY ? "INADDR_ANY" :
        inetFmt(src, s1), inetFmt(dst, s2));
}

