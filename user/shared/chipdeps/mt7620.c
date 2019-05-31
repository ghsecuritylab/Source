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
#define CONFIG_RALINK_MT7620 1
#include <ra_ioctl.h>
#include <mt7620.h>

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
	esw_reg_read((REG_ESW_MAC_PMSR_P0 + 0x100*port), &value);
	switch_fini();

	link = value & 0x1;
	speed = (value >> 2) & 0x3;

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
	esw_reg_read((REG_ESW_MAC_PMSR_P0 + 0x100*port), &value);
	switch_fini();

	return (value & 0x1);
}

#ifdef VLAN
static void write_VTCR(unsigned int value)
{
	int i;
	unsigned int check;

	value |= 0x80000000;
	esw_reg_write(REG_ESW_VLAN_VTCR, value);

	for (i = 0; i < 20; i++) {
		esw_reg_read(REG_ESW_VLAN_VTCR, &check);
		if ((check & 0x80000000) == 0) //table busy
			break;
		usleep(1000);
	}
	if (i == 20)
		fprintf(stderr, "VTCR timeout.\n");
	else if(check & (1<<16))
		fprintf(stderr, "%s(%08x) invalid\n", __func__, value);
}

static int esw_vlan_set(int idx, int vid, char *portmap, int stag)
{
	unsigned int i, mbr, value;

	//fprintf(stderr, "%s: idx=%d, vid=%d, portmap=%s, stag=%d\n", __FUNCTION__, idx, vid, portmap, stag);

	mbr = 0;
	for (i = 0; i < 8; i++)
		mbr += (*(portmap + i) - '0') * (1 << i);
	//set vlan identifier
	esw_reg_read(REG_ESW_VLAN_ID_BASE + 4*(idx/2), &value);
	if (idx % 2 == 0) {
		value &= 0xfff000;
		value |= vid;
	}
	else {
		value &= 0xfff;
		value |= (vid << 12);
	}
	esw_reg_write(REG_ESW_VLAN_ID_BASE + 4*(idx/2), value);
	//set vlan member
	value = (mbr << 16);		//PORT_MEM
	value |= (1 << 30);		//IVL
	value |= (1 << 27);		//COPY_PRI
	value |= ((stag & 0xfff) << 4);	//S_TAG
	value |= 1;			//VALID
	esw_reg_write(REG_ESW_VLAN_VAWD1, value);

	value = (0x80001000 + idx); //w_vid_cmd
	write_VTCR(value);

	return 0;
}

void config_esw(int type)
{
	int i;
	char portmap[16];

	if (switch_init() < 0)
		return;

	for (i = 0; i < 6; i++) {
		//LAN/WAN ports as security mode
		esw_reg_write((REG_ESW_PORT_PCR_P0 + 0x100*i), 0xff0003);
		//LAN/WAN ports as transparent port
		esw_reg_write((REG_ESW_PORT_PVC_P0 + 0x100*i), 0x810000c2); //transparent port, admit untagged frames
	}

	//set CPU/P7 port as user port
	esw_reg_write((REG_ESW_PORT_PVC_P0 + 0x600), 0x81000000); //port6
	esw_reg_write((REG_ESW_PORT_PVC_P0 + 0x700), 0x81000000); //port7

	esw_reg_write((REG_ESW_PORT_PCR_P0 + 0x600), 0x20ff0003); //port6, Egress VLAN Tag Attribution=tagged
	esw_reg_write((REG_ESW_PORT_PCR_P0 + 0x700), 0x20ff0003); //port7, Egress VLAN Tag Attribution=tagged

	//set PVID
	for (i = 0; i < 5; i++) {
		if (lan_wan_partition[type][i] == 1)	//LAN
			esw_reg_write((REG_ESW_PORT_PPBVI_P0 + 0x100*i), 0x10001);
		else					//WAN
			esw_reg_write((REG_ESW_PORT_PPBVI_P0 + 0x100*i), 0x10002);
	}
	esw_reg_write((REG_ESW_PORT_PPBVI_P0 + 0x500), 0x10001);
	//VLAN member port
	//LAN
	sprintf(portmap, "%d%d%d%d%d111", lan_wan_partition[type][0]
					, lan_wan_partition[type][1]
					, lan_wan_partition[type][2]
					, lan_wan_partition[type][3]
					, lan_wan_partition[type][4]);
	esw_vlan_set(0, 1, portmap, 0);
	//WAN
	sprintf(portmap, "%d%d%d%d%d011", !lan_wan_partition[type][0]
					, !lan_wan_partition[type][1]
					, !lan_wan_partition[type][2]
					, !lan_wan_partition[type][3]
					, !lan_wan_partition[type][4]);
	esw_vlan_set(1, 2, portmap, 0);
	switch_fini();
}

void restore_esw(void)
{
	int i;

	if (switch_init() < 0)
		return;

	for (i = 0; i < 8; i++) {
		esw_reg_write((REG_ESW_PORT_PCR_P0 + 0x100*i), 0xff0000);
		esw_reg_write((REG_ESW_PORT_PVC_P0 + 0x100*i), 0x810000c0);
	}

	//Clear mac table
	esw_reg_write(REG_ESW_WT_MAC_ATC, 0x8002);
	usleep(5000);

	switch_fini();
}
#endif
