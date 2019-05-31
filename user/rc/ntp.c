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
 *
 * Copyright 2013, ASUSTek Inc.
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND ASUS GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 */
 
#include <stdio.h>
#include <fcntl.h>

#include <nvram/bcmnvram.h>

#include <shared.h>

int ntp_main(void)
{
	char servers[32]; 
	int ret;
	int fd;

	fd=open("/dev/null",O_WRONLY);
	if (fd!=-1)
	 dup2(fd,2);
	
	strcpy(servers, nvram_safe_get("ntp_servers"));

	if (!strlen(servers)) return 0;

	while(1)
	{
		ret=eval("/usr/sbin/ntpclient", "-h", servers, "-i", "3", "-l", "-s");
		if(ret == 0) {	// success update the time
			if (nvram_match("ntp_sync", "0")) {
				logmessage("ntp client", "time is synchronized to %s", nvram_safe_get("ntp_servers"));
				nvram_set("ntp_sync", "1");
			}
			return 0;
		}
		sleep(5);
	}
}
