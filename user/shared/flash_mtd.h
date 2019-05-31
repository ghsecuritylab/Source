#ifndef __FLASH_MTD_H
#define __FLASH_MTD_H

#define BASIC_MTD_MAX		5
#ifdef USE_SQUASHFS
#define INC_SQUASHFS_MTD_MAX	2
#else
#define INC_SQUASHFS_MTD_MAX	0
#endif
#ifdef INTERNET_RADIO
#define INC_AUDIO_MTD_MAX	1
#else
#define INC_AUDIO_MTD_MAX	0
#endif
#define NUM_INFO		BASIC_MTD_MAX \
				+ INC_SQUASHFS_MTD_MAX \
				+ INC_AUDIO_MTD_MAX

#define MAX_READ_CNT 0x10000

struct mtd_info {
	char dev[8];
	int size;
	int erasesize;
	char name[12];
};

extern int find_mtdX(const char *mtd_name);
extern int flash_mtd_init_info(void);
extern int flash_mtd_open(int num, int flags);
extern int flash_mtd_read(int offset, int count);
extern int FRead(char *dst, int src, int count);
extern int flash_mtd_write(int offset, int value);
extern int FWrite(char *src, int dst, int count);
extern int flash_mtd_erase(int start, int end);
#endif
