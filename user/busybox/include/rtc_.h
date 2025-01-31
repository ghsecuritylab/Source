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
 * Common defines/structures/etc... for applets that need to work with the RTC.
 *
 * Licensed under the GPL-2 or later.
 */

#ifndef _BB_RTC_H_
#define _BB_RTC_H_

#include "libbb.h"

#if __GNUC_PREREQ(4,1)
# pragma GCC visibility push(hidden)
#endif

extern int rtc_adjtime_is_utc(void) FAST_FUNC;
extern int rtc_xopen(const char **default_rtc, int flags) FAST_FUNC;
extern time_t rtc_read_time(int fd, int utc) FAST_FUNC;

/*
 * Everything below this point has been copied from linux/rtc.h
 * to eliminate the kernel header dependency
 */

struct linux_rtc_time {
	int tm_sec;
	int tm_min;
	int tm_hour;
	int tm_mday;
	int tm_mon;
	int tm_year;
	int tm_wday;
	int tm_yday;
	int tm_isdst;
};

struct linux_rtc_wkalrm {
	unsigned char enabled;	/* 0 = alarm disabled, 1 = alarm enabled */
	unsigned char pending;  /* 0 = alarm not pending, 1 = alarm pending */
	struct linux_rtc_time time;	/* time the alarm is set to */
};

/*
 * ioctl calls that are permitted to the /dev/rtc interface, if
 * any of the RTC drivers are enabled.
 */

#define RTC_AIE_ON	_IO('p', 0x01)	/* Alarm int. enable on		*/
#define RTC_AIE_OFF	_IO('p', 0x02)	/* ... off			*/
#define RTC_UIE_ON	_IO('p', 0x03)	/* Update int. enable on	*/
#define RTC_UIE_OFF	_IO('p', 0x04)	/* ... off			*/
#define RTC_PIE_ON	_IO('p', 0x05)	/* Periodic int. enable on	*/
#define RTC_PIE_OFF	_IO('p', 0x06)	/* ... off			*/
#define RTC_WIE_ON	_IO('p', 0x0f)  /* Watchdog int. enable on	*/
#define RTC_WIE_OFF	_IO('p', 0x10)  /* ... off			*/

#define RTC_ALM_SET	_IOW('p', 0x07, struct linux_rtc_time) /* Set alarm time  */
#define RTC_ALM_READ	_IOR('p', 0x08, struct linux_rtc_time) /* Read alarm time */
#define RTC_RD_TIME	_IOR('p', 0x09, struct linux_rtc_time) /* Read RTC time   */
#define RTC_SET_TIME	_IOW('p', 0x0a, struct linux_rtc_time) /* Set RTC time    */
#define RTC_IRQP_READ	_IOR('p', 0x0b, unsigned long)	 /* Read IRQ rate   */
#define RTC_IRQP_SET	_IOW('p', 0x0c, unsigned long)	 /* Set IRQ rate    */
#define RTC_EPOCH_READ	_IOR('p', 0x0d, unsigned long)	 /* Read epoch      */
#define RTC_EPOCH_SET	_IOW('p', 0x0e, unsigned long)	 /* Set epoch       */

#define RTC_WKALM_SET	_IOW('p', 0x0f, struct linux_rtc_wkalrm)/* Set wakeup alarm*/
#define RTC_WKALM_RD	_IOR('p', 0x10, struct linux_rtc_wkalrm)/* Get wakeup alarm*/

/* interrupt flags */
#define RTC_IRQF 0x80 /* any of the following is active */
#define RTC_PF 0x40
#define RTC_AF 0x20
#define RTC_UF 0x10

#if __GNUC_PREREQ(4,1)
# pragma GCC visibility pop
#endif

#endif
