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
 * $Id: jffs2-user.h,v 1.1.1.1 2006-10-13 02:44:17 steven Exp $
 *
 * JFFS2 definitions for use in user space only
 */

#ifndef __JFFS2_USER_H__
#define __JFFS2_USER_H__

/* This file is blessed for inclusion by userspace */
#include <linux/jffs2.h>
#include <endian.h>
#include <byteswap.h>

#undef cpu_to_je16
#undef cpu_to_je32
#undef cpu_to_jemode
#undef je16_to_cpu
#undef je32_to_cpu
#undef jemode_to_cpu

extern int target_endian;

#define t16(x) ({ uint16_t __b = (x); (target_endian==__BYTE_ORDER)?__b:bswap_16(__b); })
#define t32(x) ({ uint32_t __b = (x); (target_endian==__BYTE_ORDER)?__b:bswap_32(__b); })

#define cpu_to_je16(x) ((jint16_t){t16(x)})
#define cpu_to_je32(x) ((jint32_t){t32(x)})
#define cpu_to_jemode(x) ((jmode_t){t32(x)})

#define je16_to_cpu(x) (t16((x).v16))
#define je32_to_cpu(x) (t32((x).v32))
#define jemode_to_cpu(x) (t32((x).m))

#endif /* __JFFS2_USER_H__ */
