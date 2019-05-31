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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <net/if_arp.h>
#include <linux/if.h>
#define CONFIG_RALINK_MT7628 1
#include <ra_ioctl.h>
#include <mt7628.h>

#ifdef VLAN
//0:WAN, 1:LAN, lan_wan_partition[][0] is port0
static int lan_wan_partition[5][5] = {	{0,1,1,1,1}, //WLLLL
					{1,0,1,1,1}, //LWLLL
					{1,1,0,1,1}, //LLWLL
					{1,1,1,0,1}, //LLLWL
					{1,1,1,1,0}};//LLLLW
#endif

int switch_init(void)
{
	esw_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (esw_fd < 0) {
		perror("socket");
		return -1;
	}
	return 0;
}

void switch_fini(void)
{
	close(esw_fd);
}

int esw_reg_read(int offset, unsigned int *value)
{
	struct ifreq ifr;
	esw_reg reg;

	if (value == NULL)
		return -1;
	reg.off = offset;
	strncpy(ifr.ifr_name, "eth2", 5);
	ifr.ifr_data = (void*) &reg;
	if (-1 == ioctl(esw_fd, RAETH_ESW_REG_READ, &ifr)) {
		perror("ioctl");
		close(esw_fd);
		return -1;
	}
	*value = reg.val;

	//fprintf(stderr, "%s: offset=%4x, value=%8x\n", __FUNCTION__, offset, *value);

	return 0;
}


int esw_reg_write(int offset, int value)
{
	struct ifreq ifr;
	esw_reg reg;

	//fprintf(stderr, "%s: offset=%4x, value=%8x\n", __FUNCTION__, offset, value);

	reg.off = offset;
	reg.val = value;
	strncpy(ifr.ifr_name, "eth2", 5);
	ifr.ifr_data = (void*) &reg;
	if (-1 == ioctl(esw_fd, RAETH_ESW_REG_WRITE, &ifr)) {
		perror("ioctl");
		close(esw_fd);
		exit(0);
	}
	return 0;
}

int GetPhyStatus(int port, int print_format)
{
	unsigned int value, link = 0, speed = 0;
	char buf[4];

	if (switch_init() < 0)
		return 0;
	esw_reg_read(REG_ESW_POA, &value);
	switch_fini();

	link = (value >> (25 + port)) & 0x1;
	if (port < 5)
		speed = (value >> port) & 0x1;
	else
		speed = (value >> ((port - 5) * 2 + 5)) & 0x3;

	if (print_format == 1) {//ATE
		sprintf(buf, "%C", (link == 1) ? (speed == 2) ? 'G' : 'M': 'X');
		puts(buf);
	}
	else {
		if (link == 1) {
			switch (speed) {
				case 0: printf("10\n");
					break;
				case 1: printf("100\n");
					break;
				case 2:	printf("1000\n");
					break;
				default:
					printf("invalid\n");
					break;
			}
		}
		else
			printf("0\n");
	}

	return 1;
}

int GetLinkStatus(int port)
{
	unsigned int value;

	if (switch_init() < 0)
		return 0;
	esw_reg_read(REG_ESW_POA, &value);
	switch_fini();

	return ((value >> (25 + port)) & 0x1);
}

#ifdef VLAN
void config_esw(int type)
{
#error TBD. MT7628 config_esw() function...
}

void restore_esw(void)
{
	if (switch_init() < 0)
		return;

	esw_reg_write(REG_ESW_PFC1, 0x5555);
	esw_reg_write(REG_ESW_PVIDC0, 0x1001);
	esw_reg_write(REG_ESW_PVIDC1, 0x1001);
	esw_reg_write(REG_ESW_PVIDC2, 0x1001);
	esw_reg_write(REG_ESW_PVIDC3, 0x1);
	esw_reg_write(REG_ESW_VLAN_ID_BASE, 0x2001);
	esw_reg_write(REG_ESW_VLAN_MEMB_BASE, 0xffffffff);
	esw_reg_write(REG_ESW_POC2, 0x7f7f);
	esw_reg_write(REG_ESW_SGC2, 0x7f);

	switch_fini();
}
#endif
