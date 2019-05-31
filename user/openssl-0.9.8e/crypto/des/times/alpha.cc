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
cc -O2
DES_LONG is 'unsigned int'

options    des ecb/s
 4 r2 p    181146.14 100.0%
16 r2 p    172102.94  95.0%
 4 r2 i    165424.11  91.3%
16  c p    160468.64  88.6%
 4  c p    156653.59  86.5%
 4  c i    155245.18  85.7%
 4 r1 p    154729.68  85.4%
16 r2 i    154137.69  85.1%
16 r1 p    152357.96  84.1%
16  c i    148743.91  82.1%
 4 r1 i    146695.59  81.0%
16 r1 i    144961.00  80.0%
-DDES_RISC2 -DDES_PTR

