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
 * ccp.h - Definitions for PPP Compression Control Protocol.
 *
 * Copyright (c) 1994-2002 Paul Mackerras. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. The name(s) of the authors of this software must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission.
 *
 * 4. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by Paul Mackerras
 *     <paulus@samba.org>".
 *
 * THE AUTHORS OF THIS SOFTWARE DISCLAIM ALL WARRANTIES WITH REGARD TO
 * THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS, IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY
 * SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
 * AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * $Id: ccp.h,v 1.1.1.1 2007-08-22 05:38:26 steven Exp $
 */

typedef struct ccp_options {
    bool bsd_compress;		/* do BSD Compress? */
    bool deflate;		/* do Deflate? */
    bool predictor_1;		/* do Predictor-1? */
    bool predictor_2;		/* do Predictor-2? */
    bool deflate_correct;	/* use correct code for deflate? */
    bool deflate_draft;		/* use draft RFC code for deflate? */
    bool lzs;			/* do Stac LZS? */
    bool mppc;			/* do MPPC? */
    bool mppe;			/* do MPPE? */
    bool mppe_40;		/* allow 40 bit encryption? */
    bool mppe_56;		/* allow 56 bit encryption? */
    bool mppe_128;		/* allow 128 bit encryption? */
    bool mppe_stateless;	/* allow stateless encryption */
    u_short bsd_bits;		/* # bits/code for BSD Compress */
    u_short deflate_size;	/* lg(window size) for Deflate */
    u_short lzs_mode;		/* LZS check mode */
    u_short lzs_hists;		/* number of LZS histories */
    short method;		/* code for chosen compression method */
} ccp_options;

extern fsm ccp_fsm[];
extern ccp_options ccp_wantoptions[];
extern ccp_options ccp_gotoptions[];
extern ccp_options ccp_allowoptions[];
extern ccp_options ccp_hisoptions[];

extern struct protent ccp_protent;
