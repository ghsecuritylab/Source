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
 * ASUS Home Gateway Reference Design
 * Web Page Configuration Support Routines
 *
 * Copyright 2013, ASUSTek Inc.
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of ASUSTek Inc.;
 * the contents of this file may not be disclosed to third parties, copied or
 * duplicated in any form, in whole or in part, without the prior written
 * permission of ASUSTek Inc..
 *
 */

#ifdef INTERNET_RADIO
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>

#include <nvram/bcmnvram.h>

#include <shared.h>

#include "httpd.h"
#include "web-iradio.h"

#define PATH1		"/etc_ro/radio/user_builtin.list"
#define PATH2		"/etc_ro/radio/builtin.list"
#define RADIOGROUP	410	//table 400 + user 10
#define USER_URL_NUM	10	//static

static int BUILT_IN_NUM = 10;	//dynamic
static int mark_play = 0;
static int mark_nvramurl = 0;
static int mark_iradio = 0;
static time_t mark_t = 0;

extern void do_file(char *path, FILE *stream);
/*
 * GUI: export internet radio list
 */
void do_download_ini_cgi(char *url, FILE *stream)
{
	char buf[1024];
	FILE *fpr, *fpw;
	int isend;

	doSystem("cat /dev/mtd%d > /var/tmp/radio_table.tmp", find_mtdX("Radio"));
	fpr = fopen("/var/tmp/radio_table.tmp", "r");
	if (fpr) {
		memset(buf, 0, sizeof(buf));
		if (fgets(buf, sizeof(buf), fpr)) {
			if (strncmp(buf, "WIBU", 4) == 0) {
				fpw = fopen("/tmp/radio_list.ini", "w");
				memset(buf, 0, sizeof(buf));
				isend = 0;
				while (fgets(buf, sizeof(buf), fpr)) {
					if (buf[0] != '#') {
						if (strstr(buf, "[PlayList End]")) //detect endtext
							isend = 1;
					}
					if (fpw)
						fputs(buf, fpw);

					memset(buf, 0, sizeof(buf));
					if (isend)
						break;
				}
				fclose(fpr);
				fclose(fpw);
				do_file("/tmp/radio_list.ini", stream);
				unlink("/tmp/radio_list.ini");
				unlink("/var/tmp/radio_table.tmp");
				return;
			}
		}
	}

list_err:
	fclose(fpr);
	do_file("/etc_ro/radio/default_usr_builtin.list", stream);
}

extern int file_post_done;
#ifdef USB_TO_CM6510
/* 
 * GUI: upgrade CM6510 firmware
 */
void do_upgrade_cm6510_cgi(char *url, FILE *stream)
{
	fprintf(stderr, "%s\n", __FUNCTION__);

	if (file_post_done) {
		eval("i2ctrl", "u", CM6510_FW_UPLOAD_PATH);
		websApply(stream, "Updating.asp");
		fclose(stream);
		notify_rc("restart_reboot");
	}
	else {
		websApply(stream, "UpdateError.asp");
		fclose(stream);
		unlink(CM6510_FW_UPLOAD_PATH);
	}
}
#endif

static int table_renew = 1;
/* 
 * GUI: import internet radio list
 */
void do_upload_ini_cgi(char *url, FILE *stream)
{
	fprintf(stderr, "%s\n", __FUNCTION__);

	if (file_post_done) {
		if (update_radio_table()) {
			fprintf(stderr, "%s: finish!\n", __FUNCTION__);
			table_renew = 1;
			nvram_set("radio_table","1");
			nvram_commit();
		}
		else
			fprintf(stderr, "%s: error!\n", __FUNCTION__);
	}

	unlink(INI_UPLOAD_PATH);
	unlink("/tmp/radio_tb.last");
	fclose(stream);
}

#ifdef I2C_TO_ALC5627
int open_mixer_fd(void);
#define RT2880_I2C_SET_DAC_MUTE		2
#define RT2880_I2C_SET_DAC_UNMUTE	4
#endif

#ifdef URL_PLAYER
/*
 * GUI: URL player CGI
 */
void do_url_cgi(char *url, FILE *stream)
{
	/* mute/unmute */
	char *remote_vol_mute = websGetVar(stream, "remote_vol_mute", "");
	if (strcmp(remote_vol_mute, "")) {
		int mute = atoi(remote_vol_mute);
#ifdef I2C_TO_ALC5627
		int fd;

		if ((fd = open_mixer_fd()) >= 0) {
			if (mute)
				ioctl(fd, RT2880_I2C_SET_DAC_UNMUTE, 0);
			else
				ioctl(fd, RT2880_I2C_SET_DAC_MUTE, 0);
			close(fd);
		}
#endif
#ifdef USB_TO_CM6510
		doSystem("amixer -q cset numid=3 %d", 1 - mute);
#endif
		doSystem("nvram set audio_mute=%d", mute);
	}

	/* volume */
	char *remote_vol = websGetVar(stream, "remote_vol", "");
	if (strcmp(remote_vol, "")) {
		int volume;
#ifdef I2C_TO_ALC5627
		int volume_max = 31;
		int numid = 1;
		int fd;
#endif
#ifdef USB_TO_CM6510
		int volume_max = 62;
		int numid = 4;
#endif
		/* volume: 0~100% convert to 0~volume_max, and round off */
		volume = (atoi(remote_vol) * volume_max + 50) / 100;
		doSystem("amixer -q cset numid=%d %d", numid, volume);
		doSystem("nvram set audio_volume=%s", remote_vol);
		if (nvram_match("audio_mute", "1")) {
#ifdef I2C_TO_ALC5627
			if ((fd = open_mixer_fd()) >= 0) {
				ioctl(fd, RT2880_I2C_SET_DAC_UNMUTE, 0);
				close(fd);
			}
#endif
#ifdef USB_TO_CM6510
			doSystem("amixer -q cset numid=3 1");
#endif
			nvram_set("audio_mute", "0");
		}
	}

	/* URL */
	char *remote_url = websGetVar(stream, "remote_url", "");
	if (strcmp(remote_url, "")) {
		fprintf(stderr, "%s: URL=%s\n", __FUNCTION__, remote_url);
		writefile("/tmp/iradio", remote_url);
	}

	/* play/stop */
	char *remote_act = websGetVar(stream, "remote_act", "");
	if (!strcmp(remote_act, "play"))
		eval("killall", "-SIGUSR1", "audiod");
	else if (!strcmp(remote_act, "stop")) {
		unlink("/tmp/iradio");
		eval("killall", "-SIGUSR1", "audiod");
		nvram_set("now_play","999");
	}

	fclose(stream);
}
#endif

/*
 * GUI: get/set volume
 */
int ej_get_vol(int eid, webs_t wp, int argc, char_t **argv)
{
	int volume_gui;

	/* volume: 0~100% convert to 0~20, and round up */
	volume_gui = (atoi(nvram_safe_get("audio_volume")) * 20 + 80) / 100;
	websWrite(wp, "%d", volume_gui);

	return 0;
}

int ej_set_vol(int eid, webs_t wp, int argc, char_t **argv)
{
	int volume;
#ifdef I2C_TO_ALC5627
	int volume_max = 31;
	int numid = 1;
	int fd;
#endif
#ifdef USB_TO_CM6510
	int volume_max = 62;
	int numid = 4;
#endif
	int volume_gui = atoi(websGetVar(wp, "vol", ""));

	/* volume: 0~20 convert to 0~volume_max, and round off */
	volume = (volume_gui * volume_max + 10) / 20;
	doSystem("amixer -q cset numid=%d %d", numid, volume);
	doSystem("nvram set audio_volume=%d", volume_gui * 5);
	if (nvram_match("audio_mute", "1")) {
#ifdef I2C_TO_ALC5627
		if ((fd = open_mixer_fd()) >= 0) {
			ioctl(fd, RT2880_I2C_SET_DAC_UNMUTE, 0);
			close(fd);
		}
#endif
#ifdef USB_TO_CM6510
		doSystem("amixer -q cset numid=3 1");
#endif
		nvram_set("audio_mute", "0");
	}
	nvram_commit();

	return 0;
}

/*
 * GUI: mute/unmute
 */
int ej_audio_mute(int eid, webs_t wp, int argc, char_t **argv)
{
	int mute = atoi(nvram_safe_get("audio_mute"));
#ifdef I2C_TO_ALC5627
	int fd;

	if ((fd = open_mixer_fd()) >= 0) {
		if (mute)
			ioctl(fd, RT2880_I2C_SET_DAC_UNMUTE, 0);
		else
			ioctl(fd, RT2880_I2C_SET_DAC_MUTE, 0);
		close(fd);
	}
#endif

#ifdef USB_TO_CM6510
	doSystem("amixer -q cset numid=3 %d", mute);
#endif
	doSystem("nvram set audio_mute=%d", 1 - mute);
	websWrite(wp, "%d", 1 - mute);

	return 0;
}

/*
 * GUI: set EQ
 */
int ej_set_audio_eq(int eid, webs_t wp, int argc, char_t **argv)
{
	char* eq;

	eq = websGetVar(wp, "eq", "");
#ifdef USB_TO_CM6510
	doSystem("i2ctrl e %s", eq);
#endif
	nvram_set("eq_cur_idx", eq);

	return 0;
}

static char urlTable[RADIOGROUP][256];		//set clean when builtin file is reloading
static int urlpointer = 0;			//set clean when builtin file is reloading

#define UI_ELEMENT_NUM	4
static int find_element(char *start, char*end, char* result)
{
	int i;
	char *pta = NULL, *ptb = NULL;

	pta = start;
	for (i = 0; i < UI_ELEMENT_NUM; i++) {
		if ((ptb = strstr(pta, ",")) == NULL)
			goto ele_err;
		pta = ptb + 1;
	}

	strncpy(urlTable[urlpointer],pta, end-pta);
	//printf("result[%d]=%s\n",urlpointer,urlTable[urlpointer]);
	urlpointer++;
	*(pta-1) = ']';
	strncpy(result, start, pta-start);

ele_err:
	return 0;
}

int ej_get_url_table(int eid, webs_t wp, int argc, char_t **argv)
{
	int i;
	char buf[256];
	FILE *fp;

	fp = fopen("/tmp/url", "w");
	websWrite(wp, "[");
	for (i = 0; i < urlpointer; i++) {
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "\"%s\"", urlTable[i]);
		websWrite(wp, "%s", buf);
		if (i != (urlpointer - 1))
			websWrite(wp, ",");
		if (i >= BUILT_IN_NUM) //only for user's private list
			if (fp)
				fprintf(fp, "%s\n", urlTable[i]);
	}
	websWrite(wp, "]");
	if (fp)
		fclose(fp);

	return 0;
}

/*
 * GUI: get current play index number
 */
int ej_get_play_num(int eid, webs_t wp, int argc, char_t **argv)
{
	websWrite(wp, "%s", nvram_get("now_play"));
	return 0;
}

static char Rinfo[4][10] = {"Region:", "Title:", "Type:", "File:"};

static int get_radio_table(void)
{
	char buf[256], res[256];
	char *pt1;
	int sep, i;
	FILE *fp;
	FILE *fpw;

	if (table_renew) {
		unlink(PATH1);
		doSystem("cat /dev/mtd%d > /tmp/radiolist_tmp.ini", find_mtdX("Radio"));
		table_renew = 0;
	}
	else {
		//printf("Radio table does not renew from Radio SPI partition\n");
		return 0;
	}

	fp = fopen("/tmp/radiolist_tmp.ini", "r");
	fpw = fopen(PATH1, "w");

	if (fp != NULL) {
		sep = 0;
		while (1) {
			memset(res, 0, sizeof(res));
			memset(buf, 0, sizeof(buf));
			if (fgets(buf, sizeof(buf), fp) != NULL) {
				if (buf[0] == '#') //comments in *.INI
					continue;
				else if ((pt1 = strstr(buf, Rinfo[0])) != NULL) {
					fputs("[", fpw);
					strncpy(res, pt1+strlen(Rinfo[0]), strlen(buf)-strlen(Rinfo[0])-1);
					fputs("\"", fpw);
					fputs(res, fpw);
					fputs("\",", fpw);

					for (i = 1; i < 4; i++) {
						memset(res, 0, sizeof(res));
						memset(buf, 0, sizeof(buf));
						fgets(buf, sizeof(buf), fp);
						pt1 = strstr(buf, Rinfo[i]);
						strncpy(res, pt1+strlen(Rinfo[i]), strlen(buf)-strlen(Rinfo[i])-1);
						if (i == 3) {
							fputs("      \"-\",", fpw);//kbps
							if (strlen(res) == 0)//file
								fputs("http://", fpw);
							else
								fputs(res, fpw);
						}
						else {//title,type
							fputs("\"", fpw);
							fputs(res, fpw);
							fputs("\",", fpw);
						}
					}

					fputs("]\n", fpw);
				}
				else if (strstr(buf, "[PlayList End]")) //endtext in *.INI
					break;
			}
			else
				break;
		}
	}

	fclose(fpw);
	fclose(fp);
	unlink("/tmp/radiolist_tmp.ini");
	//websWrite(wp, "0");

	return 0;
}

/* chk_rs_err:
 * 1. file can not be blank / file format or path is invalid
 * 2. file-size is too large
 * 3. file-content format is invalid
 * 4. system error
 * 5. max group
 * set chk_rs_err as 0 in get_upload_table_status()
 */
static int chk_rs_err = 0;

/* max characters:
 * Region: 8, only 0~9/a~z/A-Z
 * Title:30, only 0~9/a~z/A-Z
 * Type:8, only 0~9/a~z/A-Z
 * File:n/a
 */
static int check_radiotable_content(char *buf,int which_item)
{
	int i, len;

	// printf("[content]#%s#%d\n",buf,strlen(buf));
	if (which_item == 0 || which_item == 2)
		len = 8;
	else if (which_item == 1)
		len = 30;
	else
		return 1;

	if ((strlen(buf)-1) > len)
		goto err_content;

	for (i = 0; i < strlen(buf)-1; i++) {
		if (!((buf[i] >= 0x41 && buf[i] <= 0x5a) 
				|| (buf[i] >= 0x61 && buf[i] <= 0x7a) 
				|| (buf[i] >= 0x30 && buf[i] <= 0x39) 
				|| buf[i] == 0x20 ))
			goto err_content;
	}

	return 1;

err_content:
	//printf("[content]: error\n");
	return 0;

}

static int check_radiotable(void)
{
	int count_group;
	int k;
	unsigned char buf[256];
	FILE *fp = fopen("/tmp/radio_tb.prf", "r");
	char *pr;
	int isvalid,isend;

	if (fp != NULL) {
		isend = 0;
		isvalid = 0;
		count_group = 0;
		while (1) {
			memset(buf, 0, sizeof(buf));
			if (fgets(buf, sizeof(buf), fp) != NULL) {
				if (count_group > RADIOGROUP) {
					chk_rs_err = 5;
					fclose(fp);
					return 0;
				}

				if (buf[0] == '#') //comments in *.INI
					continue;
				else if (pr = strstr(buf, Rinfo[0])) {
					if (!check_radiotable_content(pr+strlen(Rinfo[0]), 0))
						goto err_rs;

					for (k = 1; k < 4; k++) {
						memset(buf, 0, sizeof(buf));
						if (fgets(buf, sizeof(buf), fp) != NULL) {
							if ((pr = strstr(buf, Rinfo[k])) == NULL)
								goto err_rs;
							else {
								if (!check_radiotable_content(pr+strlen(Rinfo[k]), k))
									goto err_rs;
							}
							isvalid = 1;
						}
					}
					count_group++;
					//printf("gcount=%d\n",count_group);
				}
				else if (strstr(buf, Rinfo[1]) || strstr(buf, Rinfo[2]) || strstr(buf, Rinfo[3]))
					goto err_rs;
				else if (strstr(buf, "[PlayList End]")) {
					//printf("detcet endtext in *.ini file\n");
					isend = 1;
					fclose(fp);
					return 1;
				}
			}
			else {
				if (!isvalid)
					goto err_rs;
				fclose(fp);
				break;
			}
		}
	}
	if (!isend) //don't detect endtext in *.INI file
		goto err_rs;
	else
		return 1;

err_rs:
	chk_rs_err = 3;
	fclose(fp);

	return 0;
}

static int update_radio_table(void)
{
	struct stat fileStat;

	if (!stat("/tmp/radio_tb.prf", &fileStat)) {
		printf("[do upload cgi]:upload file size=%d\n", fileStat.st_size);
		if (fileStat.st_size <= 0x10000) {//64K
			if (check_radiotable()) {
				unlink("/tmp/iradio");
				eval("killall", "-SIGUSR1", "audiod"); // stop everything
				nvram_set("now_play", "999");
				sleep(1);
				eval("/bin/mtd_write", "erase", "Radio");
				system("cat /etc_ro/radio/magic /tmp/radio_tb.prf > /tmp/radio_tb.last");
				eval("/bin/mtd_write", "write", "/tmp/radio_tb.last", "Radio");
				return 1;//success to upgrade
			}
		}
		else
			chk_rs_err = 2;//file size is too large
	}
	else
		chk_rs_err = 4; //radio tmpfile state err?

	return 0;//fail to upgrade
}

/*
 * GUI: get builtin internet radio list
 */
int ej_get_builtin_list(int eid, webs_t wp, int argc, char_t **argv)
{
	FILE *fp ;
	char buf[350],res[100];
	char *pt1,*pt2;
	int sep;
	struct stat fileStat;

	if (atoi(nvram_get("radio_table"))) {
		get_radio_table();
		if (!stat(PATH1, &fileStat)) {
			if (fileStat.st_size != 0)
				fp = fopen(PATH1, "r");
			else {
				nvram_set("radio_table", "0");
				table_renew = 1;
				fp = fopen(PATH2, "r");
			}
		}
	}
	else
		fp = fopen(PATH2, "r");

	if (fp != NULL) {
		sep = 0;
		websWrite(wp, "[");
		urlpointer = 0;
		memset(urlTable, 0, sizeof(urlTable));
		while (1) {
			memset(buf, 0, sizeof(buf));
			memset(res, 0, sizeof(res));
			if (fgets(buf, sizeof(buf), fp) != NULL) {
				if (sep)
					websWrite(wp, ",");
				else
					sep = 1;
				pt1 = strstr(buf, "[");
				if (pt1) {
					pt2 = strstr(pt1, "]");
					if (pt2) {
						find_element(pt1, pt2, res);
						websWrite(wp, "%s", res);
					}
					else
						break;
				}
				else
					break;
			}
			else
				break;
		}
		websWrite(wp, "]");
	}

	fclose(fp);
	BUILT_IN_NUM = urlpointer;
	//printf("Now,BUILT_IN_NUM=%d",BUILT_IN_NUM);

	return 0;
}

/*
 * GUI: get user-add internet radio list
 */
int ej_get_user_list(int eid, webs_t wp, int argc, char_t **argv)
{
	char usr_url[10], buf[350], res[100];
	char *pt1, *pt2;
	int sep, i;

	sep = 0;
	websWrite(wp, "[");
	for (i = 0; i < USER_URL_NUM; i++) {
		memset(res, 0, sizeof(res));
		memset(buf, 0, sizeof(buf));
		memset(usr_url, 0, sizeof(usr_url));
		sprintf(usr_url, "usr_url%d", i);
		if ((pt1 = nvram_get(usr_url)) != NULL) {
			strcpy(buf, pt1);
			pt1 = strstr(buf, "[");
			if (pt1) {
				if (sep)
					websWrite(wp, ",");
				else
					sep = 1;
				pt2 = strstr(pt1, "]");
				if (pt2) {
					find_element(pt1, pt2, res);
					websWrite(wp, "%s", res);
				}
			}
#if 0 //debug
			else
				printf("#nvram usr_url%d is NULL\n",i);
#endif
		}
	}
	websWrite(wp, "]");

	return 0;
}

int ej_check_iradio(int eid, webs_t wp, int argc, char_t **argv)
{
	struct stat fileStat;

	if (!stat("/tmp/iradio", &fileStat))
		websWrite(wp,"1"); //file exist
	else {
		nvram_set("now_play", "999");
		websWrite(wp, "0");
	}

	return 0;
}   

#define RALINK_GPIO3924_READ	0x52
#define GPIO_DEV		"/dev/gpio"
/*
 * GUI: detect status of audio jack
 */
int ej_detect_audiojack(int eid, webs_t wp, int argc, char_t **argv)
{
#ifdef AUDIO_JACK_SUPPORT
	int fd, value;

	fd = open(GPIO_DEV, O_RDONLY);
	if (fd < 0) {
		perror(GPIO_DEV);
		return -1;
	}
	if (ioctl(fd, RALINK_GPIO3924_READ, &value) < 0) {
		perror("ioctl");
		close(fd);
		return -1;
	}
	close(fd);
	if (value & (0x1 << 3))
		websWrite(wp, "1"); //insert
	else
#endif
		websWrite(wp, "0"); //remove

	return 0;
}

int ej_play_rs(int eid, webs_t wp, int argc, char_t **argv)
{
	char *value, *address, *now_play;

	value = websGetVar(wp, "urlnum", "");
	address = websGetVar(wp, "address", "");

	now_play = nvram_get("now_play");
	printf("now=%s pre=%s\n", value, now_play);
	if (!strcmp(value, now_play)) {
		unlink("/tmp/iradio");
		eval("killall", "-SIGUSR1", "audiod"); // stop everything
		printf("double click the same URL!!!\n");
		nvram_set("now_play", "999");
		return 0;
	}
	if (atoi(value) < urlpointer) {
		nvram_set("now_play", value);
		writefile("/tmp/iradio", address);
		eval("killall", "-SIGUSR1", "audiod");
		//printf("run \"%s......\"\n",url);
	}

	return 0;
}

/*
 * GUI: add internet radio
 */
int ej_update_ralist(int eid, webs_t wp, int argc, char_t **argv)
{
	char *para[5] = {"", "", "", "-", ""}; //region,radiostation,type,speed,urllink
	char buf[250];
	char usrurl[10];
	int t;

	para[0] = websGetVar(wp, "region", "");
	para[1] = websGetVar(wp, "name", "");
	para[2] = websGetVar(wp, "type", "");
	para[4] = websGetVar(wp, "addr", "");

	unlink("/tmp/iradio");
	eval("killall", "-SIGUSR1", "audiod"); // stop everything
	nvram_set("now_play", "999");

	fprintf(stderr,"addr=%s\n", para[4]);
	if ((strstr(para[0], "\"") 
			|| strstr(para[1], "\"") 
			|| strstr(para[2], "\"")/*||strstr(para[4],"\"")*/) 
			|| (strstr(para[0], ",") 
				|| strstr(para[1], ",") 
				|| strstr(para[2], ",") 
				|| strstr(para[4], ",")) 
			|| (strstr(para[0], "[")
				|| strstr(para[1], "[") 
				||strstr(para[2], "[") 
				||strstr(para[4], "[")) 
			|| (strstr(para[0], "]") 
				|| strstr(para[1], "]") 
				|| strstr(para[2], "]") 
				|| strstr(para[4], "]"))) {
		fprintf(stderr, "invalid char!!!!!!\n");
		websWrite(wp, "1");
		return 1;
	}

	printf("# write nvram usr_url, urpointer=%d: %s %s %s %s  #\n",urlpointer,para[0],para[1],para[2],para[4]);
	memset(buf, 0, sizeof(buf));

	if (urlpointer >= BUILT_IN_NUM && urlpointer < (BUILT_IN_NUM + USER_URL_NUM)) {
		strcat(buf, "[");
		for (t = 0; t < 4; t++) {//region,radiostation,type,speed
			strcat(buf, "\"");
			strcat(buf, para[t]);
			strcat(buf, "\"");
			strcat(buf, ",");
		}
		strcat(buf, para[4]);
		strcat(buf, "]");
		sprintf(usrurl, "usr_url%d", (urlpointer - BUILT_IN_NUM));
		printf("set %s as %s\n", usrurl, buf);
		nvram_set(usrurl, buf);
		nvram_commit();
		mark_nvramurl = 1;
		eval("killall", "-SIGUSR2", "audiod"); // update list
	}
	else
		printf("user's url-filed is overflow!\n");

	return 0;
}

/*
 * GUI: delete internet radio
 */
int ej_del_ralist(int eid, webs_t wp, int argc, char_t **argv)
{
	char *delnum;
	int i;
	char usrurl1[10];
	char tmp[200];
	char *touch_index,temp[5];

	touch_index = nvram_get("touch_ctl1_radio_no");
	delnum = websGetVar(wp, "delnum", "");
	unlink("/tmp/iradio");
	eval("killall", "-SIGUSR1", "audiod"); // stop everything
	//printf("delnum=%d, ctlnum=%d\n",atoi(delnum)+1,atoi(touch_index));
	if ((atoi(delnum) + 1) == atoi(touch_index))  //delnum:0~19, touch_index:11~20
		nvram_set("touch_ctl1_radio_no", "1");
	else if ((atoi(delnum) + 1) < atoi(touch_index)) {
		sprintf(temp, "%d", atoi(touch_index) - 1);
		nvram_set("touch_ctl1_radio_no", temp);
	}
	nvram_set("now_play", "999");
	printf("kill url=%d\n", atoi(delnum) - BUILT_IN_NUM);
	for (i = (atoi(delnum) - BUILT_IN_NUM); i < USER_URL_NUM; i++) {
		memset(usrurl1, 0, sizeof(usrurl1));
		sprintf(usrurl1, "usr_url%d", i);
		nvram_unset(usrurl1);
#if 0 //debug
		printf("unset %s\n", usrurl1);
		system("nvram show | grep usr_url");
#endif
		if (i != (USER_URL_NUM - 1)) {
			memset(tmp, 0, sizeof(tmp));
			sprintf(usrurl1, "usr_url%d", i + 1);
			strcpy(tmp,nvram_safe_get(usrurl1));
			//reorganize_url(tmp);
			sprintf(usrurl1, "usr_url%d", i);
			nvram_set(usrurl1, tmp);
#if 0 //debug
			printf("set %s\n", usrurl1);
			system("nvram show | grep usr_url");
#endif
		}
		else
			nvram_set(usrurl1, "");
	}
	nvram_commit();
	eval("killall", "-SIGUSR2", "audiod"); // update list

	return 0;
}

int ej_get_status(int eid, webs_t wp, int argc, char_t **argv)
{
	struct stat fileStat;
	struct stat fileStat2;
	char PATH[200];

	if (nvram_match("radio_table", "1"))
		strcpy(PATH, PATH1);
	else
		strcpy(PATH, PATH2);

	if (!stat(PATH, &fileStat)) {
		//printf("%s, sat_mtime= %x\n", PATH,fileStat.st_mtime);
		if (difftime(fileStat.st_mtime, mark_t)) {
			mark_t = fileStat.st_mtime;
			goto done;
		}
	}

	if (mark_nvramurl) {
		//printf("mark_nvram_url\n");
		mark_nvramurl = 0;
		goto done;
	}

	if (nvram_match("nvram_kbps", "1")) {
		//printf("==> nvram kbps\n");
		nvram_set("nvram_kbps", "0");
		goto done;
	}

	if (nvram_match("invalid_url", "1")) {
		//printf("==> invalid_url\n");
		nvram_set("invalid_url", "0");
		goto done;
	}

	if (nvram_match("shair_vol", "1")) {
		nvram_set("shair_vol", "0");
		goto done;
	}

	if (atoi(nvram_get("now_play")) != mark_play) {
		//printf("mark_play_n\n");
		mark_play = atoi(nvram_get("now_play"));
		goto done;
	}

	if (!stat("/tmp/iradio", &fileStat2)) {
		if (!mark_iradio) {
			//printf("mark_iradio[1]\n");
			mark_iradio = 1;
			goto done;
		}
	}
	else {
		if (mark_iradio) {
			//printf("mark_iradio[0]\n");
			mark_iradio = 0;
			goto done;
		}
	}
	websWrite(wp, "0");

	return 0;

done:
	printf("[refresh..Internet radio UI]\n");
	websWrite(wp, "1");

	return 1;
}

/*
 * GUI: test audio
 */
int ej_test_audio(int eid, webs_t wp, int argc, char_t **argv)
{
	char *btn;

	btn = websGetVar(wp, "test_audio", "");

	unlink("/tmp/iradio");
	eval("killall", "-SIGUSR1", "audiod"); // stop everything
	sleep(1);

	if (!strcmp(btn, "1"))
		system("sh /usr/lib/testmp3/single.sh &");
	else if (!strcmp(btn, "2"))
		system("sh /usr/lib/testmp3/loop.sh &");
	else if (!strcmp(btn, "3"))
		system("sh /usr/lib/testmp3/enum.sh &");

	return 0;
}

int ej_upgrade_radio_list(int eid, webs_t wp, int argc, char_t **argv)
{
	char *value;

	value = websGetVar(wp, "cmd", "");
	//printf("get cmd=%s\n",value);
	if (atoi(value)) {
		if (update_radio_table()) {
			printf("[httpd]: upgrade radio-list ...\n");
			table_renew = 1;
			nvram_set("radio_table", "1");
			nvram_commit();
			websWrite(wp, "1");
		}
		else
			websWrite(wp, "0");
	}

	unlink("/tmp/radio_tb.prf");
	unlink("/tmp/radio_tb.last");

	return 0;
}

int ej_get_upload_table_status(int eid, webs_t wp, int argc, char_t **argv)
{
	char buf[5];

	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%d", chk_rs_err);
	//printf("ej_get_upload_table_status\n");
	websWrite(wp, buf);
	chk_rs_err = 0;

	return 0;
}
#endif /* INTERNET_RADIO */
