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
#ifndef _rc_event_h_
#define _rc_event_h_
/* init states */
typedef enum {
	RESTART,
	STOP,
	START,
	TIMER,
	IDLE,
	SERVICE,
	MTDWRITE,
} INIT_STATES;

/* copy from kernel path: include/linux/kernel.h */
enum {
	ASUS_HIJ_ENABLE = 601,
	ASUS_HIJ_DISABLE,
	ASUS_HIJ_ENABLE_DEBUG,
	ASUS_HIJ_DISABLE_DEBUG,
	ASUS_HIJ_SET_TARGET_HOST,
	ASUS_HIJ_SET_TARGET_IP,
	ASUS_HIJ_SET_DUT_MAC
};
#endif
