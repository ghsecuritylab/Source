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
#include <signal.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <nvram/bcmnvram.h>

#include <shared.h>
#include <iwlib.h>

#include "rc.h"
#include "watchdog.h"
#include <unistd.h>
#include <string.h>

#define NORMAL_PERIOD		1		/* second */
#define URGENT_PERIOD		100 * 1000	/* microsecond */	
#define RUSHURGENT_PERIOD	50 * 1000	/* microsecond */
#define BTN_WAIT_COUNT		3 * 10		/* 10 times a second */
#define AUTO_SYNC_TIME		8640	/* 1 day */

enum RESET_BTN_STATE {
	RESET_BTN_NONE = 0,
	RESET_BTN_DETECT,
	RESET_BTN_ACCEPT
};
#ifdef WPS_SUPPORT
enum WPS_BTN_STATE {
	WPS_BTN_NONE = 0,
	WPS_BTN_DETECT,
	WPS_BTN_ACCEPT
};
#endif

static int _sw_mode;
static struct itimerval itv;
static int divide_period = 0;
static int period_12hours = 0;
/* reset button */
static int rst_btn_pressed = 0;
static int rst_btn_count = 0;
/* sync time */
static int syncTime_period = 0;
#ifdef WPS_SUPPORT
/* wps button/GUI */
static int wps_btn_pressed = 0;
static int wps_btn_count = 0;
static int wps_timeout = 0;
static int WscStatus_old[2] = {-1, -1};
#ifdef DUAL_BAND_NONCONCURRENT
int wps_freq = 0;
#endif
#endif

int http_check(const char *server, char *buf, size_t count, off_t offset)
{
	char *pid_file = "/var/run/httpd.pid";
	char pid_buf[10], proc_path[32];
	int fd, pid;
	struct stat f_st;

	if((fd=open(pid_file, O_RDONLY)) <= 0)
		return 0;
	memset(pid_buf, '\0', sizeof(pid_buf));
	read(fd, pid_buf, sizeof(pid_buf));
	close(fd);
	if((pid = atoi(pid_buf)) <= 0)
		return 0;

	memset(proc_path, '\0', sizeof(proc_path));
	sprintf(proc_path, "/proc/%d", pid);

	lstat(proc_path, &f_st);
	if(!S_ISDIR(f_st.st_mode))
		return 0;

	return 1;
}

static void alarmtimer(unsigned long sec,unsigned long usec)
{
	itv.it_value.tv_sec  = sec;
	itv.it_value.tv_usec = usec;
	itv.it_interval = itv.it_value;
	setitimer(ITIMER_REAL, &itv, NULL);
}

#ifdef WPS_SUPPORT
static int can_do_wsc(void)
{
	if (_sw_mode == 1 || _sw_mode == 3 || _sw_mode == 6) {
		char *wl0_auth_mode = nvram_safe_get("wl0_auth_mode");
		char *wl0_closed = nvram_safe_get("wl0_closed");
		char *sw0_radio = nvram_safe_get("sw0_radio");
		char *wl1_auth_mode = nvram_safe_get("wl1_auth_mode");
		char *wl1_closed = nvram_safe_get("wl1_closed");
		char *sw1_radio = nvram_safe_get("sw1_radio");

		if (!strcmp(wl0_auth_mode, "shared") 
				|| !strcmp(wl0_auth_mode, "wpa") 
				|| !strcmp(wl0_auth_mode, "wpa2") 
				|| !strcmp(wl0_auth_mode, "radius") 
				|| !strcmp(wl0_closed, "1") 
				|| !strcmp(sw0_radio, "0") 
				|| !strcmp(wl1_auth_mode, "shared") 
				|| !strcmp(wl1_auth_mode, "wpa") 
				|| !strcmp(wl1_auth_mode, "wpa2") 
				|| !strcmp(wl1_auth_mode, "radius") 
				|| !strcmp(wl1_closed, "1") 
				|| !strcmp(sw1_radio, "0"))
			return 0;
	}
	return 1;
}

static void set_wsc_timeout(void)
{
	if ((_sw_mode == 2 && nvram_invmatch("rep_wps_be_ap", "1")) 
#ifdef SWMODE_ADAPTER_SUPPORT
			|| _sw_mode == 4
#endif
	)
#ifdef DUAL_BAND_NONCONCURRENT
		wps_timeout = 60*20;
#else
		wps_timeout = 120*20;
#endif
	else
		wps_timeout = 120*20;
}
#endif /* WPS_SUPPORT */
#ifdef DUAL_BAND_AUTOSELECT
struct save_fuple {
	int length;
	char *cmppart;
	char *setpart1;
	char *setpart2;
};
static int comparetmp( char *arraylist[], int sizelist, char ssidptr1[], char *ssidptr2) {
	int sizetmp = 0;
	char ssidcat[128];

	strcpy ( ssidcat, ssidptr1 );
	strcat ( ssidcat, ssidptr2 );
	while( sizetmp < sizelist) {
		if( !strcmp( arraylist[sizetmp], ssidcat ) ) {
			strcat ( ssidptr1, ssidptr2 );
			return 1;
		}
		sizetmp ++;
	}
	return 0;
}
static void auto_detect_ssid(void) {
	char file_name[128], substrl[128], strNULL[]="";
	char *ssid_buf, *getptr1, *getptr2, *substrr, *gettmp[128];
	int  band_chk, fsize, idlength=0, cmpresult=0;
	struct save_fuple *bandlist;	
	struct save_fuple getSsidRule0[] = {
		{ 5	, "-2.4G"  , "-5G"	, ""  },
		{ 5	, "_2.4G"  , "_5G"	, ""  },
		{ 5	, ".2.4G"  , ".5G"	, ""  },
		{ 5	, " 2.4G"  , " 5G"	, ""  },
		{ 5	, "-2.4g"  , "-5g"	, ""  },
		{ 5	, "_2.4g"  , "_5g"	, ""  },
		{ 5	, ".2.4g"  , ".5g"      , ""  },
		{ 5	, " 2.4g"  , " 5g"      , ""  },
		{ 4     , "2.4G"   , "5G"       , ""  },
		{ 4	, "2.4g"   , "5g"  	, ""  },
		{ 4	, "-2.4"   , "-5"       , ""  },
		{ 4	, "_2.4"   , "_5"       , ""  },
		{ 4	, ".2.4"   , ".5"       , ""  },
		{ 4	, " 2.4"   , " 5"       , ""  },
		{ 3	, "2.4"    , "5"        , ""  },
		{ 3	, "-2G"    , "-5G"      , ""  },
		{ 3	, "_2G"    , "_5G"      , ""  },
		{ 3	, ".2G"    , ".5G"      , ""  },
		{ 3	, " 2G"    , " 5G"      , ""  },
		{ 3     , "-2g"    , "-5g"      , ""  },
		{ 0     , "_2g"    , "_5g"      , ""  },
		{ 3     , ".2g"    , ".5g"      , ""  },
		{ 3     , " 2g"    , " 5g"      , ""  },
		{ 2	, "2G"     , "5G"       , ""  },
		{ 2     , "2g"     , "5g"       , ""  },
		{ 2	, "-2"     , "-5"       , ""  },
		{ 2	, "_2"     , "_5"       , ""  },
		{ 2	, ".2"     , ".5"       , ""  },
		{ 2	, " 2"     , " 5"       , ""  },
		{ 1	, "2"      , "5"        , ""  }, 
		{ 0	, ""	   , "-5G"      , ""  },
		{ 0	, ""	   , "_5G"      , ""  },
		{ 0	, ""	   , ".5G"      , ""  },
		{ 0	, ""	   , " 5G"      , ""  },
		{ 0	, ""	   , "-5g"      , ""  },
		{ 0	, ""	   , "_5g"      , ""  },
		{ 0	, ""	   , ".5g"      , ""  },
		{ 0	, ""	   , " 5g"      , ""  },
		{ 0	, ""	   , "5G"       , ""  },
		{ 0	, ""	   , "5g"       , ""  },
		{ 0	, ""	   , "-5"       , ""  },
		{ 0	, ""	   , "_5"       , ""  },
		{ 0	, ""	   , ".5"       , ""  },
		{ 0	, ""	   , " 5"       , ""  },
		{ 0	, ""	   , "5"        , ""  },
		{ 0	, ""	   , ""         , ""  },
		{ 99	, ""	   , ""         , ""  }
	};	
	struct save_fuple getSsidRule1[] = {
	        { 3	, "-5G"	, "-2G"	, "-2.4G"  },
		{ 3	, "_5G"	, "_2G"	, "_2.4G"  },
		{ 3	, ".5G"	, ".2G"	, ".2.4G"  },
		{ 3	, " 5G"	, " 2G"	, " 2.4G"  },	
		{ 3	, "-5g"	, "-2g"	, "-2.4g"  },
		{ 3	, "_5g"	, "_2g"	, "_2.4g"  },
		{ 3	, ".5g"	, ".2g"	, ".2.4g"  },	
		{ 3	, " 5g"	, " 2g"	, " 2.4g"  },
		{ 2	, "5G"	, "2G"	, "2.4G"   },
		{ 2	, "5g"	, "2g" 	, "2.4g"   },
		{ 2	, "-5"	, "-2"	, "-2.4"   },
		{ 2	, "_5"	, "_2"	, "_2.4"   },
		{ 2	, ".5"	, ".2"	, ".2.4"   },
		{ 2	, " 5"	, " 2"	, " 2.4"   },
		{ 1	, "5"	, "2" 	, "2.4"    },
		{ 0	, ""	, "-2G"	, "-2.4G"  },
		{ 0     , ""    , "_2G" , "_2.4G"  },
		{ 0     , ""    , ".2G" , ".2.4G"  },
		{ 0     , ""    , " 2G" , " 2.4G"  },
		{ 0     , ""    , "-2g" , "-2.4g"  },
		{ 0     , ""    , "_2g" , "_2.4g"  },
		{ 0     , ""    , ".2g" , ".2.4g"  },
		{ 0     , ""    , " 2g" , " 2.4g"  },
		{ 0     , ""    , "2G" 	, "2.4G"   },
		{ 0     , ""    , "2g"  , "2.4g"   },
		{ 0     , ""    , "-2"  , "-2.4"   },
		{ 0     , ""    , "_2"  , "_2.4"   },
		{ 0     , ""    , ".2"  , ".2.4"   },
		{ 0     , ""    , " 2"  , " 2.4"   },
		{ 0     , ""    , "2"  	, "2.4"    },
		{ 0     , ""    , ""  	, ""       },
		{ 99    , ""    , ""    , ""       }
	};

	if ( check_if_file_exist("/var/run/re_wpsc0.pid") && check_if_file_exist("/var/run/re_wpsc1.pid") ) {
		eval("rm", "-rf", "/tmp/ssidList");
		return;
	}
	else if ( check_if_file_exist("/var/run/re_wpsc0.pid" ))
		band_chk = 0;
	else if ( check_if_file_exist("/var/run/re_wpsc1.pid"))
		band_chk = 1;
	else { 
		eval("rm", "-rf", "/tmp/ssidList");
		return;
	}

	//Get another band's SSID list
	sprintf(file_name, "/tmp/ssidList/ssid%d.txt", band_chk );
	if( access( file_name, F_OK ) != -1 ) {
		getptr1 = readfile(file_name, &fsize);
		getptr2 = strstr(getptr1, "\n");
		while( getptr2 != NULL ) {
			*getptr2 = '\0';
			gettmp[idlength] = getptr1;
			getptr1 = getptr2 +1;
			getptr2 = strstr(getptr1, "\n");
			idlength ++;
		}
	}
	free( file_name );
	
	sprintf(file_name, "sta%d_ssid", band_chk==0? 1:0 );
	ssid_buf = nvram_get(file_name);

	if ( band_chk )
		bandlist = getSsidRule0;
	else
		bandlist = getSsidRule1;

	//compare the SSID with SCAN LIST 
	while( bandlist->length != 99 ) {
		if ( strlen(ssid_buf) > bandlist->length  ) {	
			substrr = ssid_buf + strlen(ssid_buf) - bandlist->length;
			strcpy( substrl, ssid_buf );
		}
		else {
			bandlist++;
			continue;
		}

		if( bandlist->length == 0 ) {
			if ( comparetmp( gettmp, idlength, substrl, bandlist->setpart1 ) ) {
				cmpresult=1;
				break ;
			}
			if ( bandlist->setpart2 != "" ) {
				if ( comparetmp( gettmp, idlength, substrl, bandlist->setpart2 ) ) {
					cmpresult=1;
					break;
				}
			}
		}
		else if(  !strcmp(substrr, bandlist->cmppart) ){
			substrl[strlen(ssid_buf)-bandlist->length]='\0';
			if( comparetmp( gettmp, idlength, substrl, bandlist->setpart1 ) ) {
				cmpresult=1;
				break;
			}
			if ( bandlist->setpart2 != "" ) {
				if ( comparetmp( gettmp, idlength, substrl, bandlist->setpart2 ) ) {
					cmpresult=1;
					break;
				}
			}
			if ( comparetmp( gettmp, idlength, substrl, strNULL ) ) {
				cmpresult = 1;
				break;
			}
		}
		bandlist++;
	}	

	if( cmpresult == 1 ) {
		fprintf(stderr, "=== Find the SSID : [ %s ]\n", substrl);	
		doSystem("rm -f /var/run/re_wpsc%d.pid", band_chk);
		GuessSSIDProfile(band_chk, substrl);
	}
	else
		fprintf(stderr, "=== Can't found the SSID : [  ]\n");
	
//	eval("rm", "-rf", "/tmp/ssidList");   //Open it & get ssid list file 
	return;	
}
#endif

static void btn_check(void)
{
#ifdef WPS_SUPPORT
	if (wps_btn_pressed == WPS_BTN_NONE) {
#endif
		if (!ra_gpio_read(RESET_BTN_GPIO_IRQ)) {
			if (nvram_invmatch("asus_mfg", "0"))
				nvram_set("btn_rst", "1");
			else {
				if (rst_btn_pressed == RESET_BTN_NONE) {
					rst_btn_pressed = RESET_BTN_DETECT;
					rst_btn_count = 0;
					alarmtimer(0, URGENT_PERIOD);
				}
				else {	/* Whenever it is pushed steady */
					rst_btn_count++;
					fprintf(stderr, "press reset button %d sec\n", rst_btn_count/10);
					if (rst_btn_pressed < RESET_BTN_ACCEPT) {
						if (rst_btn_count > BTN_WAIT_COUNT) {
							rst_btn_pressed = RESET_BTN_ACCEPT;
#ifdef LED_DRV_SUPPORT
							nvram_set("led_drv_stage", "2");
#endif
						}
					}
				}
			}

			return;
		}
		else {
			if (rst_btn_pressed == RESET_BTN_DETECT) {
				rst_btn_pressed = RESET_BTN_NONE;
				rst_btn_count = 0;
				alarmtimer(NORMAL_PERIOD, 0);
			}
			else if (rst_btn_pressed == RESET_BTN_ACCEPT) {
#ifdef LED_DRV_SUPPORT
				nvram_set("led_drv_stage", "3");
#endif
				alarmtimer(0, 0);
				sys_default();
				notify_rc("restart_reboot");
			}
		}
#ifdef WPS_SUPPORT
	}

	if (wps_btn_pressed < WPS_BTN_ACCEPT) {
#ifdef WPS_BTN_SUPPORT
		if (!ra_gpio_read(WPS_BTN_GPIO_IRQ)) {
			if (nvram_invmatch("asus_mfg", "0"))
				nvram_set("btn_ez", "1");
			else {
				if (wps_btn_pressed == WPS_BTN_NONE) {
					wps_btn_pressed = WPS_BTN_DETECT;
					wps_btn_count = 0;
					alarmtimer(0, RUSHURGENT_PERIOD);
				}
				else {	/* Whenever it is pushed steady */
					if (++wps_btn_count > BTN_WAIT_COUNT) {
						if (!can_do_wsc()) return;

						wps_btn_pressed = WPS_BTN_ACCEPT;
						wps_btn_count = 0;

						set_wsc_timeout();
						nvram_set("wps_mode", "2");
						start_wsc();
					}
				}
			}
		}
		else if (wps_btn_pressed == WPS_BTN_DETECT) {
			wps_btn_pressed = WPS_BTN_NONE;
			wps_btn_count = 0;
			alarmtimer(NORMAL_PERIOD, 0);
		}
#endif /* WPS_BTN_SUPPORT */
	}
	else {
		int n;
		char wif[8], *wif_next, aif[8], *aif_next;
		char file_name[32];
		int WscStatus;
		int stop_wps = 0;

		for2each(n, wif, nvram_safe_get("wl_ifnames"), wif_next,
				aif, nvram_safe_get("sta_ifnames"), aif_next) {
			if (_sw_mode == 2 && nvram_invmatch("rep_wps_be_ap", "1")) {
				sprintf(file_name, "/var/run/re_wpsc%d.pid", n);
				if (!check_if_file_exist(file_name))
					continue;

				WscStatus = getWscStatusCli(aif);
			}
			else
				WscStatus = getWscStatus(wif);

			if (WscStatus_old[n] != WscStatus) {
				WscStatus_old[n] = WscStatus;
#ifdef DUAL_BAND_NONCONCURRENT
				fprintf(stderr, "WscStatus: %d\n", WscStatus);
#else
				fprintf(stderr, "%sG WscStatus: %d\n", n==0?"2.4":"5", WscStatus);
#endif
			}

			if (WscStatus == 2) {			/* Wsc Process failed */
				fprintf(stderr, "%s", "Error occured. Is the PIN correct?\n");
				stop_wps = 1;
			}
			// Driver 1.9 supports AP PBC Session Overlapping Detection.
			else if (WscStatus == 0x109) {		/* PBC_SESSION_OVERLAP */
				fprintf(stderr, "PBC_SESSION_OVERLAP\n");
				stop_wps = 1;
			}
			else if (WscStatus == 34) {		/* Configured */
				fprintf(stderr, "Configured\n");
				if (_sw_mode == 2 && nvram_invmatch("rep_wps_be_ap", "1")) {
					eval("rm", "-f", file_name);
					if (RetrieveWscPapProfile(n, wif)) {
						if (!check_if_file_exist("/var/run/re_wpsc0.pid") 
								&& !check_if_file_exist("/var/run/re_wpsc1.pid")) {
							stop_wps = 1;
#ifdef DUAL_BAND_NONCONCURRENT
							nvram_set("cur_freq", wps_freq==0 ? "0" : "1");
							wps_freq = 1;
#endif
						}
						else {
#ifdef LED_DRV_SUPPORT
							nvram_set("led_drv_stage", n==0 ? "6" : "7");
#endif
							if (wps_timeout > 15*20)
								wps_timeout = 15*20;
						}

#ifdef SWMODE_REPEATER_EXPRESSWAY_SUPPORT
						/* if WPS success, disable ExpressWay mode */
						nvram_set("re_expressway", "0");
						/* if WPS success, disable MediaBridge mode */
						nvram_set("re_mediabridge", "0");
#endif
					}

				}
#ifdef SWMODE_ADAPTER_SUPPORT
				else if (_sw_mode == 4) {
					if (RetrieveWscPapProfile(n, wif)) {
						stop_wps = 1;
						nvram_set("x_Setting", "1");
#ifdef DUAL_BAND_NONCONCURRENT
						nvram_set("cur_freq", wps_freq==0 ? "0" : "1");
						wps_freq = 1;
#endif
					}
				}
#endif
				else {
					stop_wps = 1;
				}
			}
		}

		if (stop_wps || --wps_timeout == 0) {
			fprintf(stderr, "stop wsc\n");

			wps_timeout = 0;
			stop_wsc();

#ifdef DUAL_BAND_NONCONCURRENT
			if (!wps_freq) {
				wps_freq = 1;
				goto TryAnotherBand;
			}
			wps_freq = 0;
#endif

			wps_btn_pressed = WPS_BTN_NONE;
			wps_btn_count = 0;
			alarmtimer(NORMAL_PERIOD, 0);

			if (_sw_mode == 2 && nvram_invmatch("rep_wps_be_ap", "1")) {
#ifdef DUAL_BAND_AUTOSELECT
				auto_detect_ssid();
#endif
				restart_apcli(1);
			}
#ifdef SWMODE_ADAPTER_SUPPORT
			else if (_sw_mode == 4)
				restart_sta(1);
#endif
#ifdef LED_DRV_SUPPORT
			else
				nvram_set("led_drv_stage", "1");
#endif
		}
	}

#ifdef DUAL_BAND_NONCONCURRENT
	return;

TryAnotherBand:
	set_wsc_timeout();
	start_wsc();
#endif
#endif /* WPS_SUPPORT */
}

#ifdef ROAMING_SUPPORT
static void roaming_check(void)
{
	int n;
	char wif[8], *wif_next, aif[8], *aif_next;

	for2each(n, wif, nvram_safe_get("wl_ifnames"), wif_next,
			aif, nvram_safe_get("sta_ifnames"), aif_next)
		rssi_check(n, wif, aif);
}
#endif

#define SVC_DATE_SUN		0x40
#define SVC_DATE_MON		0x20
#define SVC_DATE_TUE		0x10
#define SVC_DATE_WED		0x08
#define SVC_DATE_THU		0x04
#define SVC_DATE_FRI		0x02
#define SVC_DATE_SAT		0x01
#define SVC_DATE_TUE_TO_FRI	SVC_DATE_TUE+SVC_DATE_WED+SVC_DATE_THU+SVC_DATE_FRI

enum {
	RADIOACTIVE,
	ACTIVEITEMS
} ACTIVE;

int svcStatus[2][ACTIVEITEMS] = {{ -1},{ -1}};
char svcDate[2][ACTIVEITEMS][10];
char svcTime[2][ACTIVEITEMS][10];
char svcTime2[2][ACTIVEITEMS][10];

/*
 * Case1:
 *
 * Sat                 |                   Sun                    |                 Mon
 *      sched_end      |     sched_begin           sched_end      |     sched_begin
 *           |         |         | |     region1      | |         |         |
 *           |------- OFF -------| |------- On -------| |------- OFF -------|
 *
 * Case2:
 *
 * Sat                 |                   Sun                    |                 Mon
 *     sched_begin     |      sched_end           sched_begin     |      sched_end
 *           |         | region3| |                   | | region2 |        |
 *           |------- ON -------| |------- OFF -------| |------- ON -------|
 */
static int in_sched(int now_mins, int now_dow, int sched_begin, int sched_end, int sched_begin2, int sched_end2, int sched_dow)
{
	//fprintf(stderr, "%s: now_mins=%d sched_begin=%d sched_end=%d sched_begin2=%d sched_end2=%d now_dow=%d sched_dow=%d\n", __FUNCTION__, now_mins, sched_begin, sched_end, sched_begin2, sched_end2, now_dow, sched_dow);

	// Sun
	if (now_dow & SVC_DATE_SUN) {
		// under Sunday's sched time (region1)
		if ((now_dow & sched_dow) 
				&& (now_mins >= sched_begin2) 
				&& (now_mins <= sched_end2) 
				&& (sched_begin2 < sched_end2))
			return 1;

		// under Sunday's sched time and cross-night (region2)
		if ((now_dow & sched_dow) 
				&& (now_mins >= sched_begin2) 
				&& (sched_begin2 >= sched_end2))
			return 1;

		// under Saturday's sched time (region3)
		if ((sched_dow & SVC_DATE_SAT) 
				&& (now_mins <= sched_end2) 
				&& (sched_begin2 >= sched_end2))
			return 1;
	}
	// Mon
	else if (now_dow & SVC_DATE_MON) {
		// under Monday's sched time
		if ((now_dow & sched_dow) 
				&& (now_mins >= sched_begin) 
				&& (now_mins <= sched_end) 
				&& (sched_begin < sched_end))
			return 1;

		// under Monday's sched time and cross-night
		if ((now_dow & sched_dow) 
				&& (now_mins >= sched_begin) 
				&& (sched_begin >= sched_end))
			return 1;

		// under Sunday's sched time
		if ((sched_dow & SVC_DATE_SUN) 
				&& (now_mins <= sched_end2) 
				&& (sched_begin2 >= sched_end2))
			return 1;
	}
	// Tue-Fri
	else if (now_dow & SVC_DATE_TUE_TO_FRI) {
		// under today's sched time
		if ((now_dow & sched_dow) 
				&& (now_mins >= sched_begin) 
				&& (now_mins <= sched_end) 
				&& (sched_begin < sched_end))
			return 1;

		// under today's sched time and cross-night
		if ((now_dow & sched_dow) 
				&& (now_mins >= sched_begin) 
				&& (sched_begin >= sched_end))
			return 1;

		// under yesterday's sched time
		now_dow <<= 1;
		if ((now_dow & sched_dow) 
				&& (now_mins <= sched_end) 
				&& (sched_begin >= sched_end))
			return 1;
	}
	// Sat
	else if (now_dow & SVC_DATE_SAT) {
		// under Saturday's sched time
		if ((now_dow & sched_dow) 
				&& (now_mins >= sched_begin2) 
				&& (now_mins <= sched_end2) 
				&& (sched_begin2 < sched_end2))
			return 1;

		// under Saturday's sched time and cross-night
		if ((now_dow & sched_dow) 
				&& (now_mins >= sched_begin2) 
				&& (sched_begin2 >= sched_end2))
			return 1;

		// under Friday's sched time
		if((sched_dow & SVC_DATE_FRI) 
				&& (now_mins <= sched_end) 
				&& (sched_begin >= sched_end))
			return 1;
	}

	return 0;
}

static int timecheck_item(char *activeDate, char *activeTime, char *activeTime2)
{
	time_t now;
	struct tm *tm;
	int current, i;
	int activeTimeStart, activeTimeEnd;
	int activeTimeStart2, activeTimeEnd2;
	int now_dow, sched_dow = 0;

	time(&now);
	tm = localtime(&now);
	current = tm->tm_hour*60 + tm->tm_min;

	// weekdays time
	activeTimeStart = ((activeTime[0]-'0')*10 + (activeTime[1]-'0'))*60 + (activeTime[2]-'0')*10 + (activeTime[3]-'0');
	activeTimeEnd = ((activeTime[4]-'0')*10 + (activeTime[5]-'0'))*60 + (activeTime[6]-'0')*10 + (activeTime[7]-'0');

	// weekend time
	activeTimeStart2 = ((activeTime2[0]-'0')*10 + (activeTime2[1]-'0'))*60 + (activeTime2[2]-'0')*10 + (activeTime2[3]-'0');
	activeTimeEnd2 = ((activeTime2[4]-'0')*10 + (activeTime2[5]-'0'))*60 + (activeTime2[6]-'0')*10 + (activeTime2[7]-'0');

	// now day of week
	now_dow = 1 << (6-tm->tm_wday);

	// schedule day of week
	for (i = 0; i <= 6; i++)
		sched_dow += (activeDate[i]-'0') << (6-i);

	return in_sched(current, now_dow, activeTimeStart, activeTimeEnd, activeTimeStart2, activeTimeEnd2, sched_dow);
}

/*
 * time-dependent services:
 * 1. wireless radio on/off
 */
static int svc_timecheck(void)
{
	int n;
	char wif[8], *next;
	char tmp[128], prefix[] = "wlXXXXXXXXXX_";
	char *pt;
	int activeNow;

	if (_sw_mode != 3 || nvram_match("ntp_sync", "0"))
		return 0;

	//fprintf(stderr, "%s\n", __FUNCTION);

	pt = getenv("TZ");
	if (!pt || nvram_invmatch("time_zone_x", pt))
		setenv("TZ", nvram_safe_get("time_zone_x"), 1);

	for1each(n, wif, nvram_safe_get("wl_ifnames"), next) {
		snprintf(prefix, sizeof(prefix), "wl%d_", n);

		if (!nvram_match(strcat_r(prefix, "radio_x", tmp), "0")) {
			(void)strcat_r(prefix, "reload_svc_wl", tmp);
			if (svcStatus[n][RADIOACTIVE] == -1 || nvram_match(tmp, "1")) {
				if (nvram_match(tmp, "1"))
					nvram_set(tmp, "0");

				strcpy(svcDate[n][RADIOACTIVE], nvram_safe_get(strcat_r(prefix, "radio_date_x", tmp)));
				strcpy(svcTime[n][RADIOACTIVE], nvram_safe_get(strcat_r(prefix, "radio_time_x", tmp)));
				strcpy(svcTime2[n][RADIOACTIVE], nvram_safe_get(strcat_r(prefix, "radio_time2_x", tmp)));
				svcStatus[n][RADIOACTIVE] = timecheck_item(svcDate[n][RADIOACTIVE], svcTime[n][RADIOACTIVE], svcTime2[n][RADIOACTIVE]);

				if (svcStatus[n][RADIOACTIVE] == 0)
					wireless_radio(n, wif, 0);
			}
			else {
				activeNow = timecheck_item(svcDate[n][RADIOACTIVE], svcTime[n][RADIOACTIVE], svcTime2[n][RADIOACTIVE]);
				if (activeNow != svcStatus[n][RADIOACTIVE]) {
					svcStatus[n][RADIOACTIVE] = activeNow;
					wireless_radio(n, wif, activeNow);
				}
			}
		}
	}
}

/* Sometimes, httpd becomes inaccessible, try to re-run it */
int http_processcheck(void)
{
	char http_cmd[32];
	char buf[256];

	sprintf(http_cmd, "http://127.0.0.1/");
	if (!http_check(http_cmd, buf, sizeof(buf), 0) 
			&& nvram_invmatch("ap_selecting", "1")) {
		printf("[watchdog] rerun httpd\n");
		start_httpd();
	}
	return 0;
}

void sync_timeprocess(void)
{
	syncTime_period = (syncTime_period +1) % AUTO_SYNC_TIME;
	if (syncTime_period) return;
	
	time_zone_x_mapping();
	do_timer();
	stop_ntpc();
	start_ntpc();
	fprintf(stderr, "[%s]\n", __FUNCTION__);

	return;
}

static void catch_sig(int sig)
{
	if (sig == SIGUSR1) {
#ifdef TOUCH_SUPPORT
		if (nvram_invmatch("asus_mfg", "0"))
			nvram_set("btn_touch", "1");
#endif
#if (defined(AUDIO_SUPPORT) && defined(VOL_KNOB_SUPPORT))
		int volume;
#ifdef USB_TO_CM6510
		/* volume: 0~100% convert to 0~62, and round off */
		volume = (atoi(nvram_safe_get("audio_volume")) * 62 + 50) / 100;
		doSystem("amixer -q cset numid=4 %d", volume);
		if (nvram_match("audio_mute", "1")) {
			doSystem("amixer -q cset numid=3 1");
			nvram_set("audio_mute", "0");
		}
#endif
#endif
	}
	else if (sig == SIGUSR2) {
#ifdef AUDIO_SUPPORT
#ifdef AUDIO_JACK_SUPPORT
		if (nvram_invmatch("asus_mfg", "0"))
			nvram_set("btn_jack", "1");
#endif
#ifdef VOL_KNOB_SUPPORT
		int mute = atoi(nvram_safe_get("audio_mute"));
		doSystem("nvram set audio_mute=%d", 1- mute);
#ifdef USB_TO_CM6510
		doSystem("amixer -q cset numid=3 %d", mute);
#endif
#endif
#endif
	}
#ifdef WPS_SUPPORT
	else if (sig == SIGTSTP 
			&& _sw_mode == 3 
				|| (_sw_mode == 2 && nvram_match("rep_wps_be_ap", "1"))) {
		int _wps_start_flag = atoi(nvram_safe_get("wps_start_flag"));

		nvram_set("wps_start_flag", "0");
		if (_wps_start_flag == 3) {
			wps_btn_pressed = WPS_BTN_NONE;
			wps_btn_count = 0;
			alarmtimer(NORMAL_PERIOD, 0);
			wps_oob();
		}
		else if (nvram_match("wps_enable", "0")) {
			wps_btn_pressed = WPS_BTN_NONE;
			wps_btn_count = 0;
			alarmtimer(NORMAL_PERIOD, 0);
			stop_wsc();
			if (_sw_mode == 2 && nvram_invmatch("rep_wps_be_ap", "1")) {
				int n;
				char wif[8], *wif_next, aif[8], *aif_next;

				for2each(n, wif, nvram_safe_get("wl_ifnames"), wif_next,
						aif, nvram_safe_get("sta_ifnames"), aif_next) {
					ifconfig(wif, 0, NULL, NULL);
					ifconfig(aif, 0, NULL, NULL);
					ifconfig(wif, IFUP, NULL, NULL);
					ifconfig(aif, IFUP, NULL, NULL);
				}
				start_apcli_monitor();
			}
#ifdef LED_DRV_SUPPORT
			nvram_set("led_drv_stage", "1");
#endif
		}
		else if (_wps_start_flag == 1) {
			ui_prestart_wsc();
		}
		else if (_wps_start_flag == 2) {
			wps_btn_pressed = WPS_BTN_ACCEPT;
			wps_btn_count = 0;
			start_wsc();
			wps_timeout = 120*20;
			alarmtimer(0, RUSHURGENT_PERIOD);
		}
	}
#endif
}

/* wathchdog is runned in NORMAL_PERIOD, 1 seconds
 * check in each NORMAL_PERIOD
 *	1. button
 *	2. touch setting: audio mute/unmute
 *
 * check in each NORAML_PERIOD*10
 *      1. time-dependent service
 *      2. http-process
 */
void watchdog(void)
{
	/* handle button */
#if !defined(CONFIG_DEFAULTS_ASUS_RPAC56)
	btn_check();
#endif

	if (nvram_invmatch("asus_mfg", "0") 
			/* if timer is set to less than 1 sec, then bypass the following
			 * eg: press WPS/Reset button, during WPS, etc.
			 */
			|| itv.it_value.tv_sec == 0)
		return;

	divide_period = (divide_period + 1) % 10;
	if (divide_period)
		return;

#ifdef ROAMING_SUPPORT
	roaming_check();
#endif

	/* check for time-dependent services */
	svc_timecheck();

	/* http server check */
	http_processcheck();

	/* time sync per day */
	sync_timeprocess();

	if (_sw_mode != 4 && nvram_match("webs_state_info", "") && nvram_match("dnsqmode", "1"))
		notify_rc("restart_webs_update");

	if (++period_12hours < 4320)
		return;
	period_12hours = 0;
	notify_rc("start_webs_update");
}

static int chld_handle(int sig)
{
	while (waitpid(-1,NULL,WNOHANG) > 0);
}

int watchdog_main(void)
{
	FILE *fp;
	int n;
	char wif[8], *next;
	char tmp[128], prefix[] = "wlXXXXXXXXXX_";

	/* write pid */
	if ((fp = fopen("/var/run/watchdog.pid", "w")) != NULL) {
		fprintf(fp, "%d", getpid());
		fclose(fp);
	}

	/* get nvram */
	_sw_mode = atoi(nvram_safe_get("sw_mode"));

	enable_gpio_irq();

	/* set the signal handler */
	signal(SIGCHLD, chld_handle);
	signal(SIGUSR1, catch_sig);
	signal(SIGUSR2, catch_sig);
#ifdef WPS_SUPPORT
	signal(SIGTSTP, catch_sig);
#endif
	signal(SIGALRM, watchdog);

	for1each(n, wif, nvram_safe_get("wl_ifnames"), next) {
		snprintf(prefix, sizeof(prefix), "wl%d_", n);
		doSystem("nvram set sw%d_radio=%s", n, nvram_safe_get(strcat_r(prefix, "radio_x", tmp)));
		if (nvram_match(strcat_r(prefix, "radio_x", tmp), "0"))
			wireless_radio(n, wif, 0);
	}

	start_ntpc();

	/* set timer */
	alarmtimer(NORMAL_PERIOD, 0);

	/* Most of time it goes to sleep */
	while(1) {
		pause();
	}

	return 0;
}
