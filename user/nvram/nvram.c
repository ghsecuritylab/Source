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
 * Copyright 2013, ASUSTek Inc.
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef ASUS_NVRAM
#include <nvram/typedefs.h>
#include <nvram/bcmnvram.h>
#else	// !ASUS_NVRAM
#include <typedefs.h>
#include <bcmnvram.h>
#endif	// ASUS_NVRAM

static void usage(void)
{
	printf("Usage: nvram [get name] [set name=value] [unset name] [commit] [show] [restore file] [save file]\n");
	exit(0);
}

#ifdef ENCRYPT_CFG
unsigned char get_rand()
{
	unsigned char buf[1];
	FILE *fp;

	fp = fopen("/dev/urandom", "r");
	if (fp == NULL) {
#ifdef DEBUG
		fprintf(stderr, "[nvram] Could not open /dev/urandom.\n");
#endif
		return 0;
	}
	fread(buf, 1, 1, fp);
	fclose(fp);

	return buf[0];
}
#endif

int nvram_save(char *file, char *buf)
{
	FILE *fp;
	char *name;
	unsigned long count, flen, i;
	unsigned char rand = 0, tmp = 0;

	if ((fp = fopen(file, "w")) == NULL) return -1;

#ifdef DEBUG
	fprintf(stderr, "[nvram] CFG Header:");
#endif

	// model name
	fwrite(PROFILE_HEADER, 1, sizeof(PROFILE_HEADER), fp);
#ifdef DEBUG
	for (i = 0; i < sizeof(PROFILE_HEADER); i++)
		fprintf(stderr, " %2x", PROFILE_HEADER[i]);
#endif

	// nvram content length
	count = 0;
	for (name = buf; *name; name += strlen(name) + 1) {
#ifdef DEBUG
		//puts(name);
#endif
		count = count + strlen(name) + 1;
	}
	flen = count + (1024 - count % 1024);
	fwrite(&flen, 1, sizeof(flen), fp);
#ifdef DEBUG
	for (i = 0; i < sizeof(flen); i++)
		fprintf(stderr, " %2x", ((unsigned char *)&flen)[i]);
#endif

	// random number
#ifdef ENCRYPT_CFG
	rand = get_rand() % 30;
#ifdef DEBUG
	fprintf(stderr, "random number: %x\n", rand);
#endif
#endif
	fwrite(&rand, 1, sizeof(rand), fp);
#ifdef DEBUG
	fprintf(stderr, " %2x\n", rand);
#endif

	// encrypt content & write to file
#ifdef ENCRYPT_CFG
	for (i = 0; i < count; i++) {
		if (buf[i] == 0x0)
			buf[i] = 0xfd + get_rand() % 3;
		else
			buf[i] = 0xff - buf[i] + rand;
	}
#endif
	fwrite(buf, 1, count, fp);
#ifdef DEBUG
	for (i = 0; i < count; i++) {
		if (i % 16 == 0)
			fprintf(stderr, "\n");
		fprintf(stderr, " %2x", (unsigned char)buf[i]);
	}
#endif

	for (i = count; i < flen; i++) {
#ifdef ENCRYPT_CFG
		tmp = 0xfd + get_rand() % 3;
#endif
		fwrite(&tmp, 1, sizeof(tmp), fp);
#ifdef DEBUG
		if (i % 16 == 0)
			fprintf(stderr, "\n");
		fprintf(stderr, " %2x", tmp);
#endif
	}
	fclose(fp);

	return 0;
}

int issyspara(char *p)
{
	struct nvram_tuple *t;
	extern struct nvram_tuple router_defaults[];

	for (t = router_defaults; t->name; t++) {
		if (strstr(p, t->name))
			break;
	}

	if (t->name) return 1;
	else return 0;
}

int nvram_restore(char *file, char *buf)
{
	FILE *fp;
	char header[sizeof(PROFILE_HEADER)+5], *p, *v;
	unsigned long count, flen, *flenptr, i;
#ifdef ENCRYPT_CFG
	unsigned char rand, *randptr;
#endif

	if ((fp = fopen(file, "r+")) == NULL) return -1;

	count = fread(header, 1, sizeof(header), fp);
	if (count >= sizeof(header) && !strncmp(header, PROFILE_HEADER, sizeof(PROFILE_HEADER))) {
		flenptr = (unsigned long *)(header + sizeof(PROFILE_HEADER));
		flen = *flenptr & 0xffffff;
#ifdef DEBUG
		fprintf(stderr, "[nvram] restoring CFG length: %x\n", flen);
#endif

#ifdef ENCRYPT_CFG
		randptr = (unsigned char *)(header + sizeof(PROFILE_HEADER) + sizeof(flen));
		rand = *randptr;
#ifdef DEBUG
		fprintf(stderr, "[nvram] CFG random number: %x\n", rand);
#endif
#endif

		count = fread(buf, 1, flen, fp);
#ifdef DEBUG
		fprintf(stderr, "[nvram] CFG count: %x\n", count);
#endif

#ifdef ENCRYPT_CFG
		for (i = 0; i < count; i++) {
			if ((unsigned char)buf[i] > (0xfd - 0x1))
				buf[i] = 0x0;
			else
				buf[i] = 0xff + rand - buf[i];
		}
#endif
#ifdef DEBUG
		fprintf(stderr, "[nvram] CFG content:\n");
		for (i = 0; i < count; i++) {
			if (i % 16 == 0)
				fprintf(stderr, "\n");
			fprintf(stderr, " %2x", (unsigned char)buf[i]);
		}
		fprintf(stderr, "\n");
#endif
	}
	else {
		fclose(fp);
		fprintf(stderr, "[nvram] illegal restore! model mismatch...\n");
		nvram_set("restore_err", "1");
		return -1;
	}
	fclose(fp);

	p = buf;
	while (*p) {
		v = strchr(p, '=');
		if (v != NULL) {
			*v++ = '\0';

			int ret = issyspara(p);
			if (!ret) {
				if (!memcmp("wl0_", p, 4) || !memcmp("wl1_", p, 4)) {
					char *p2;
					p2 = strdup(p);
					if (p2) {
						p2[1] = 'w';
						p2[2] = 'l';
						ret = issyspara(p2 + 1);
						free(p2);
					}
				}
			}
			if (ret)
				nvram_set(p, v);

			p = v + strlen(v) + 1;
		}
		else {
			nvram_unset(p);
			p = p + 1;
		}
	}
	nvram_set("x_Setting", "1");

	return 0;
}

/* NVRAM utility */
int main(int argc, char **argv)
{
#ifdef ASUS_NVRAM
	char *name, *value, *buf;
#else	// !ASUS_NVRAM
	char *name, *value, buf[NVRAM_SPACE];
#endif	// ASUS_NVRAM

	int size;

	/* Skip program name */
	--argc;
	++argv;

	if (!*argv) 
		usage();
	
#ifdef ASUS_NVRAM
		buf = malloc (NVRAM_SPACE);
		if (buf == NULL)	{
			perror ("Out of memory!\n");
			return -1;
		}
#endif	// ASUS_NVRAM

	/* Process the remaining arguments. */
	for (; *argv; argv++) {
		if (!strncmp(*argv, "get", 3)) {
			if (*++argv) {
				if ((value = nvram_get(*argv)))
					puts(value);
			}
		}
		else if (!strncmp(*argv, "set", 3)) {
			if (*++argv) {
#ifdef ASUS_NVRAM
                		strncpy(value = buf, *argv, NVRAM_SPACE);
#else   // !ASUS_NVRAM
                		strncpy(value = buf, *argv, sizeof(buf));
#endif  // ASUS_NVRAM
				name = strsep(&value, "=");
				//printf("[nvram] nvram set (%s)(%s)\n", name, value);	// tmp test
				nvram_set(name, value);
			}
		}
		else if (!strncmp(*argv, "unset", 5)) {
			if (*++argv)
				nvram_unset(*argv);
		}
		else if (!strncmp(*argv, "commit", 5)) {
			nvram_commit();
		}
		else if (!strncmp(*argv, "save", 4)) 
		{
			if (*++argv) 
			{
#ifdef ASUS_NVRAM
				nvram_getall(buf, NVRAM_SPACE);	
#else	// !ASUS_NVRAM
				nvram_getall(buf, sizeof(buf));	
#endif	// ASUS_NVRAM
				nvram_save(*argv, buf);
			}
			
		}
		else if (!strncmp(*argv, "restore", 7)) 
		{
			if (*++argv)
				nvram_restore(*argv, buf);
		}
		else if (!strncmp(*argv, "show", 4)) {
#ifdef ASUS_NVRAM
			nvram_getall(buf, NVRAM_SPACE);
#else	// !ASUS_NVRAM
			nvram_getall(buf, sizeof(buf));
#endif	// ASUS_NVRAM
			for (name = buf; *name; name += strlen(name) + 1)
				puts(name);
			size = sizeof(struct nvram_header) + (int) name - (int) buf;
			fprintf(stderr, "size: %d bytes (%d left)\n", size, NVRAM_SPACE - size);
		}
		if (!*argv)
			break;
	}

#ifdef ASUS_NVRAM
	if (buf != NULL)
		free (buf);
#endif	// ASUS_NVRAM

	return 0;
}
