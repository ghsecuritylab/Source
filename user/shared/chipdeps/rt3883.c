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
#include <ra_ioctl.h>
#include <rt3883.h>
#ifdef EXTERNAL_PHY_SUPPORT
#include <fcntl.h>

#define GPIO_DEV		"/dev/gpio"
#define RALINK_PHY_ADDR_READ	0x45
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

#ifdef EXTERNAL_PHY_SUPPORT
unsigned char ra_get_phy_addr(void)
{
	int fd, val;

	fd = open(GPIO_DEV, O_RDONLY);
	if (fd < 0) {
		perror(GPIO_DEV);
		return 0;
	}
	if (ioctl(fd, RALINK_PHY_ADDR_READ, &val) < 0) {
		perror("ioctl");
		close(fd);
		return 0;
	}
	close(fd);

	return (unsigned char)(val & 0xff);
}

static char *mii_mgr_get_reg(char *addr, int reg)
{
	FILE *fp;
	char cmdbuf[128], reg_val[5];

	sprintf(cmdbuf, "mii_mgr -g -p %s -r %d", addr, reg);
	fp = popen(cmdbuf, "r");
	if (fp) {
		int rlen;
		char *pt;

		rlen = fread(cmdbuf, 1, sizeof(cmdbuf), fp);
		pclose(fp);
		if (rlen > 0) {
			cmdbuf[rlen-1] = '\0';
			pt = strstr(cmdbuf, " = ");
			if (pt) {
				strncpy(reg_val, pt+3, 4);
				reg_val[4] = '\0';
				return reg_val;
			}
		}
	}
	return NULL;
}

static void external_phy_status(unsigned int *link, unsigned int *speed)
{
	unsigned int value;
	char phy_addr[5], reg_val[5];

	sprintf(phy_addr, "%u", ra_get_phy_addr());
	strncpy(reg_val, mii_mgr_get_reg(phy_addr, 2), sizeof(reg_val));
	if (!strncmp(reg_val, "001c", 4)) {
		//fprintf(stderr, "Ralink PHY!\n");
		strncpy(reg_val, mii_mgr_get_reg(phy_addr, 17), sizeof(reg_val));
		value = strtoul(reg_val, NULL, 16);
		*link = (value >> 10) & 0x1;
		*speed = (value >> 14) & 0x3;
	}
	else if (!strncmp(reg_val, "0143", 4)) {
		//fprintf(stderr, "Broadcom PHY!\n");
		strncpy(reg_val, mii_mgr_get_reg(phy_addr, 17), sizeof(reg_val));
		value = strtoul(reg_val, NULL, 16);
		*link = (value >> 8) & 0x1;
		if (*link) {
			char cmdbuf[128];

			sprintf(cmdbuf, "mii_mgr -s -p %s -r 28 -v 0x2000 > /dev/null", phy_addr);
			system(cmdbuf);

			strncpy(reg_val, mii_mgr_get_reg(phy_addr, 28), sizeof(reg_val));
			value = strtoul(reg_val, NULL, 16);
			*speed = (value >> 3) & 0x3;
		}
	}

}
#endif

int GetPhyStatus(int port, int print_format)
{
	unsigned int value, link = 0, speed = 0;
	char buf[4];

#ifdef EXTERNAL_PHY_SUPPORT
	external_phy_status(&link, &speed);
#else
	if (switch_init() < 0)
		return 0;
	esw_reg_read((REG_ESW_MAC_PMSR_P0 + 0x100*port), &value);
	switch_fini();

	link = value & 0x1;
	speed = (value >> 2) & 0x3;
#endif

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
#ifdef EXTERNAL_PHY_SUPPORT
	unsigned int link = 0, speed = 0;

	external_phy_status(&link, &speed);

	return link;
#else
	unsigned int value;

	if (switch_init() < 0)
		return 0;
	esw_reg_read((REG_ESW_MAC_PMSR_P0 + 0x100*port), &value);
	switch_fini();

	return (value & 0x1);
#endif
}
