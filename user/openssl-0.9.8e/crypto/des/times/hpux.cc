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
HPUX 10 - 9000/887 - cc -D_HPUX_SOURCE -Aa +ESlit +O2 -Wl,-a,archive

options    des ecb/s
16  c i    149448.90 100.0%
 4  c i    145861.79  97.6%
16 r2 i    141710.96  94.8%
16 r1 i    139455.33  93.3%
 4 r2 i    138800.00  92.9%
 4 r1 i    136692.65  91.5%
16 r2 p    110228.17  73.8%
16 r1 p    109397.07  73.2%
16  c p    109209.89  73.1%
 4  c p    108014.71  72.3%
 4 r2 p    107873.88  72.2%
 4 r1 p    107685.83  72.1%
-DDES_UNROLL

