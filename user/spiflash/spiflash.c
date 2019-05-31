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
 * Frontend command-line utility for Linux NVRAM layer
 *
 * Copyright 2004, ASUSTek Inc.
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: nvram.c,v 1.1 2007/06/08 10:22:42 arthur Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define ASUS_NVRAM
#include <unistd.h>	// for sleep()
#include <nvram/typedefs.h>
#include <nvram/bcmnvram.h>

#define PATH_DEV_SPIFLASH 	"/dev/spiflash"

char value_str[20];

typedef struct SPICMD_T
{
	unsigned int addr;
	int len;
	char prt;
	char buf[20];
} spicmd_t;

spicmd_t spicmd;

char*
spiflash_get(spicmd_t *sc)
{
        size_t count = sizeof(spicmd);
	int fd;

	if((fd = open(PATH_DEV_SPIFLASH, O_RDWR)) <= 0)
	{
		perror("open spiflash");
		return NULL;
	}
	count = read(fd, (unsigned char*)sc, count);

	memset(value_str, 0, sizeof(value_str));
	if(sc->buf)
		strcpy(value_str, sc->buf);
	else	
		printf("spiflash get fail\n");

	close(fd);

	return value_str;
}

int 
spiflash_set(spicmd_t *sc)
{
        size_t count = sizeof(spicmd);
        int fd;

        if((fd = open(PATH_DEV_SPIFLASH, O_RDWR)) <= 0)
        {
                perror("open spiflash");
                return NULL;
        }
        count = write(fd, (unsigned char*)sc, count);

	close(fd);
	return 0;
}

int
usage()
{
	printf("usage:\n");	
	printf("spiflash get [offset] [len]\n");	
	printf("spiflash set [offset] [value]\n");	

	return 0;
}

/* NVRAM utility */
int
main(int argc, char **argv)
{
	char *name, *value, *buf;
	unsigned int addr, len;

	int size, i;

	/* Skip program name */
	--argc;
	++argv;

	buf = malloc (100);
	if (buf == NULL)	{
		perror ("Out of memory!\n");
		return -1;
	}

	/* Process the remaining arguments. */
	for (; *argv; argv++) {
		if (!strncmp(*argv, "get", 3)) {
			if (*++argv && *(argv+1)) {
				addr = (unsigned int)strtol(*argv, NULL, 16);
				len = (unsigned int)strtol(*(argv+1), NULL, 16);
				//printf("[spiflash] get %x (%d)\n", addr, len);	// tmp test
				memset(&spicmd, 0, sizeof(spicmd));
				spicmd.addr = addr;
				spicmd.len = len;
				spicmd.prt = 1;
				if ((value = spiflash_get(&spicmd)))
				{
				/*
					for(i=0; i<len; ++i)
					{
						printf("%c(0x%x) ", spicmd.buf[i], spicmd.buf[i]);
					}
					//puts(value);
				*/
				}
			} else
				usage();
		}
		else if (!strncmp(*argv, "set", 3)) {
			if (*++argv && *(argv+1)) {
                		//strncpy(value = buf, *argv, 100);
				//name = strsep(&value, "=");
				addr = (unsigned int)strtol(*argv, NULL, 16);
				value = *(argv+1);
				//printf("[spiflash] spiflash set (%x)(%s)\n", addr, value);	// tmp test
				spicmd.addr = addr;
				spicmd.len = strlen(value);
				spicmd.prt = 1;
				strncpy(spicmd.buf, value, strlen(value));
				spiflash_set(&spicmd);
			} else
				usage();
		}
		if (!*argv)
			break;
	}

	if (buf != NULL)
		free (buf);

	return 0;
}	
