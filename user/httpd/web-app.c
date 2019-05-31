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

#ifdef APP_SUPPORT
#include <stdio.h>

#include <nvram/bcmnvram.h>

#include <shared.h>

#include "httpd.h"
#include "web-app.h"

/*
 * App get link quality of apcli
 * JSON format
 */
static void GetApcliQuality(FILE *stream)
{
	int n, quality;
	char aif[8], *next;
	char tmp[128], prefix[] = "staXXXXXXX_";

	websWrite(stream, "{");
	for1each(n, aif, nvram_safe_get("sta_ifnames"), next) {
		snprintf(prefix, sizeof(prefix), "sta%d_", n);

		quality = 0;
		if (nvram_match(strcat_r(prefix, "connStatus", tmp), "1"))
			get_apcli_quality(aif, &quality);

		if (n > 0)
			websWrite(stream, ",");
		websWrite(stream, "\"sta%d_quality\": %d", n, quality);
	}
	websWrite(stream, "}");
}

/*
 * App check information of the auto firmware upgrade
 * JSON format
 */
static void GetAutoFwUpgradeStatus(FILE *stream)
{
	int i = 0, j = 0;
	char buf[16], fwver[8];

	websWrite(stream, "{");
	websWrite(stream, "\"webs_state_update\": %d,", atoi(nvram_safe_get("webs_state_update")));
	websWrite(stream, "\"webs_state_error\": %d,", atoi(nvram_safe_get("webs_state_error")));
	websWrite(stream, "\"webs_state_info\": \"%s\",", nvram_safe_get("webs_state_info"));
	websWrite(stream, "\"webs_state_upgrade\": %d,", atoi(nvram_safe_get("webs_state_upgrade")));

	memset(buf, 0x0, sizeof(buf));
	strcpy(buf, nvram_safe_get("firmver"));
	for (i = 0; i < strlen(buf); i++) {
		if (buf[i] == '.')
			continue;
		else
			fwver[j++] = buf[i];
	}
	fwver[j] = '\0';
	websWrite(stream, "\"firmver\": \"%s\"", fwver);

	websWrite(stream, "}");
}

#ifdef RUNTIME_CHECK_PAP_PASSWORD
/*
 * App get the result of the check parent-AP password
 */
static void GetChkPapPwResult(FILE *stream)
{
	FILE *fp;
	char *connStatus[2] = {0};
	int rlen;

	if ((fp = fopen(CHECK_PAP_PASSWORD_RESULT, "r")) != NULL) {
		rlen = fread(connStatus, 1, sizeof(connStatus), fp);
		fclose(fp);
		unlink(CHECK_PAP_PASSWORD_RESULT);
	}

	websWrite(stream, "%s", connStatus);
}
#endif

#ifdef INTERNET_RADIO
#ifdef LED_INDICATE_AUDIO_PLAY
/*
 * App choose LED indicate type during the audio playing
 * JSON format
 */
static void CtrlAudioLed(FILE *stream)
{
	int i;
	char *type = websGetVar(stream, "type", "");
	char *audio_led_list[4] = {"Music light 1", "Music light 2", "Music light 3", "No music light"};

	websWrite(stream, "{");

	websWrite(stream, "\"audio_led_list\": \"");
	for (i = 0; i < sizeof(audio_led_list)/sizeof(char *); i++) {
		if (i > 0)
			websWrite(stream, ",");
		websWrite(stream, "%s", audio_led_list[i]);
	}
	websWrite(stream, "\",");

	if (strcmp(type, "")) {
		nvram_set("audio_led_type", type);
		websWrite(stream, "\"audio_led_type\": %s", type);
		nvram_commit();
	}
	else
		websWrite(stream, "\"audio_led_type\": %s", nvram_safe_get("audio_led_type"));

	websWrite(stream, "}");
}
#else
static void CtrlAudioLed(FILE *stream)
{
	websWrite(stream, "{\"audio_led_list\": \"none\",\"audio_led_type\": 0}");
}
#endif
#endif /* INTERNET_RADIO */


void do_app_none_cgi(char *url, FILE *stream)
{
	char *action_mode = websGetVar(stream, "action_mode", "");

	fprintf(stderr, "%s: [%s]\n", __FUNCTION__, action_mode);

	if (!strcmp(action_mode, "GetApcliQuality")) {
		GetApcliQuality(stream);
	}
	else if (!strcmp(action_mode, "GetAutoFwUpgradeStatus")) {
		GetAutoFwUpgradeStatus(stream);
	}
#ifdef RUNTIME_CHECK_PAP_PASSWORD
	else if (!strcmp(action_mode, "GetChkPapPwResult")) {
		GetChkPapPwResult(stream);
	}
#endif
#ifdef INTERNET_RADIO
	else if (!strcmp(action_mode, "CtrlAudioLed")) {
		CtrlAudioLed(stream);
	}
#endif
}

void do_app_auth_cgi(char *url, FILE *stream)
{
	char *action_mode = websGetVar(stream, "action_mode", "");

	fprintf(stderr, "%s: [%s]\n", __FUNCTION__, action_mode);

	if (!strcmp(action_mode, "GetNvram")) {
		char *tmp = websGetVar(stream, "nvram", "");

		/* JSON format */
		websWrite(stream, "{\"%s\": \"%s\"}", tmp, nvram_safe_get(tmp));
	}
}
#endif /* APP_SUPPORT */
