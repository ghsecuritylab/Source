/*
 * Copyright (C) 2011 - jCoderz.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <nvram/bcmnvram.h>

#include "renderingcontrol.h"
#include "renderingcontrol.gen.c"

#include "last_change.h"

#ifdef I2C_TO_ALC5627
int open_mixer_fd(void);
#define RT2880_I2C_SET_DAC_MUTE		2
#define RT2880_I2C_SET_DAC_UNMUTE	4
#endif

gboolean
check_instanceId (GUPnPServiceAction *action)
{
    guint instance_id;
    gupnp_service_action_get (action,
                              "InstanceID", G_TYPE_UINT, &instance_id, NULL);
    if (instance_id != 0) 
    {
        gupnp_service_action_return_error (action, 718, "Invalid InstanceID");
        return FALSE;
    }
    return TRUE;
}

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// State Variables
//////////////////////////////////////////////////////////

/*
Optional:
<stateVariable>
			<name>Brightness</name>
			<name>Contrast</name>
			<name>Sharpness</name>
			<name>RedVideoGain</name>
			<name>GreenVideoGain</name>
			<name>BlueVideoGain</name>
			<name>RedVideoBlackLevel</name>
			<name>GreenVideoBlackLevel</name>
			<name>BlueVideoBlackLevel</name>
			<name>ColorTemperature</name>
			<name>HorizontalKeystone</name>
			<name>VerticalKeystone</name>
			<name>Mute</name>
			<name>Volume</name>
			<name>VolumeDB</name>
			<name>Loudness</name>
*/

const int
query_volume_cb (GUPnPService *service, 
                           gpointer user_data)
{
    g_debug("query_volume_cb");
    return 50;
}

//			<name>LastChange</name>

//			<name>PresetNameList</name>
const gchar*
query_preset_name_list_cb (GUPnPService *service, 
                           gpointer user_data)
{
    g_debug("query_preset_name_list_cb");

    return "";
}

const gchar*
query_last_change_cb (GUPnPService *service, 
                      gpointer user_data)
{
    g_debug ("query_last_change_cb");

    const gchar *result = NULL;

    GString *last_change = last_change_new();
/*
    last_change_append_gchar (last_change, LAST_CHANGE_KEY_TRANSPORT_STATE, 
                              transport_state_to_string (sv_transport_state));

    last_change_append_gchar (last_change, LAST_CHANGE_KEY_TRASNPORT_STATUS, 
                              transport_status_to_string (sv_transport_status));

    last_change_append_gchar (last_change, LAST_CHANGE_KEY_CURRENT_MEDIA_CATEGORY, 
                              current_media_category_to_string (sv_current_media_category));

    last_change_append_gchar (last_change, LAST_CHANGE_KEY_PLAY_MODE, 
                              play_mode_to_string (sv_current_play_mode));

    last_change_append_gchar (last_change, LAST_CHANGE_KEY_TRANSPORT_PLAY_SPEED, 
                              sv_current_play_speed_string);

    last_change_append_int (last_change, LAST_CHANGE_KEY_NUMBER_OF_TRACKS, 
                            sv_number_of_tracks);

    last_change_append_int (last_change, LAST_CHANGE_KEY_CURRENT_TRACK, 
                            sv_current_track);

    last_change_append_gstring (last_change, LAST_CHANGE_KEY_CURRENT_TRACK_DURATION, 
                                sv_current_track_duration_string);

    last_change_append_gstring (last_change, LAST_CHANGE_KEY_CURRENT_MEDIA_DURATION, 
                                sv_current_media_duration_string);

    last_change_append_gstring (last_change, LAST_CHANGE_KEY_CURRENT_TRACK_META_DATA, 
                                sv_current_track_meta_data);
    last_change_append_gstring (last_change, LAST_CHANGE_KEY_CURRENT_TRACK_URI, 
                                sv_current_track_uri);

    last_change_append_gchar (last_change, "AVTransportURIMetaData", 
                              NOT_IMPLEMENTED);
    last_change_append_gchar (last_change, "AVTransportURI", 
                              NOT_IMPLEMENTED);
    last_change_append_gchar (last_change, "RecordMediumWriteStatus", 
                              NOT_IMPLEMENTED);
    last_change_append_gchar (last_change, "CurrentRecordQualityMode", 
                              NOT_IMPLEMENTED);
    last_change_append_gchar (last_change, "PossibleRecordQualityModes", 
                              NOT_IMPLEMENTED);
    last_change_append_gchar (last_change, "PlaybackStorageMedium", 
                              NOT_IMPLEMENTED);
    last_change_append_gchar (last_change, "RecordStorageMedium", 
                              NOT_IMPLEMENTED);
    last_change_append_gchar (last_change, "PossiblePlaybackStorageMedia", 
                              NOT_IMPLEMENTED);
    last_change_append_gchar (last_change, "PossibleRecordStorageMedia", 
                              NOT_IMPLEMENTED);
    last_change_append_gchar (last_change, "NextAVTransportURI", 
                              NOT_IMPLEMENTED);
    last_change_append_gchar (last_change, "NextAVTransportURIMetaData", 
                              NOT_IMPLEMENTED);
*/
    result = last_change_free (last_change);

    g_debug ("\t%s", result);

    return result;
}

void
renderingcontrol_notify_last_change (GUPnPService *service, GString *last_change)
{
    g_debug ("notify_last_change");
    g_debug ("%s", last_change->str);

    renderingcontrol_last_change_variable_notify (service, last_change->str);

    g_string_free (last_change, TRUE);
}

//////////////////////////////////////////////////////////
// Actions
//////////////////////////////////////////////////////////

/*
Missing:
<action>
			<name>GetBrightness</name>
			<name>SetBrightness</name>
			<name>GetContrast</name>
    		<name>SetContrast</name>
			<name>GetSharpness</name>
			<name>SetSharpness</name>
			<name>GetRedVideoGain</name>
			<name>SetRedVideoGain</name>
			<name>GetGreenVideoGain</name>
			<name>SetGreenVideoGain</name>
			<name>GetBlueVideoGain</name>
			<name>SetBlueVideoGain</name>
			<name>GetRedVideoBlackLevel</name>
			<name>SetRedVideoBlackLevel</name>
			<name>GetGreenVideoBlackLevel</name>
			<name>SetGreenVideoBlackLevel</name>
			<name>GetBlueVideoBlackLevel</name>
			<name>SetBlueVideoBlackLevel</name>
			<name>GetColorTemperature</name>
			<name>SetColorTemperature</name>
			<name>GetHorizontalKeystone</name>
			<name>SetHorizontalKeystone</name>
			<name>GetVerticalKeystone</name>
			<name>SetVerticalKeystone</name>
			<name>GetMute</name>
			<name>SetMute</name>
			<name>GetVolume</name>
			<name>SetVolume</name>
			<name>GetVolumeDB</name>
			<name>SetVolumeDB</name>
			<name>GetVolumeDBRange</name>
			<name>GetLoudness</name>
			<name>SetLoudness</name>
			<name>GetStateVariables</name>
			<name>SetStateVariables</name>

*/


void 
volume_control(guint volume_percent)
{
 int volume;
#if defined(I2C_TO_ALC5627)
 int volume_max = 31;
 int numid = 1;
 int fd;
#elif defined(USB_TO_CM6510)
 int volume_max = 62;
 int numid = 4;
#elif defined(XXXXX)
//TBD.
 int volume_max = 62;
 int numid = 1;
#endif

 /* volume: 0~100% convert to 0~volume_max, and round off */
 volume = (volume_percent * volume_max + 50) / 100;
 g_debug ("volume_control: %d% -> %d\n", volume_percent, volume);
 doSystem("amixer -q cset numid=%d %d", numid, volume);
 if (nvram_match("audio_mute", "1")) {
#if defined(I2C_TO_ALC5627)
	if ((fd = open_mixer_fd()) >= 0) {
		ioctl(fd, RT2880_I2C_SET_DAC_UNMUTE, 0);
		close(fd);
	}
#elif defined(USB_TO_CM6510)
	doSystem("amixer -q cset numid=3 1");
#elif defined(XXXXX)
//TBD.
#endif
	nvram_set("audio_mute", "0");
 }
}

void
get_mute_cb (GUPnPService *service,
		     GUPnPServiceAction *action,
		     gpointer user_data)
{
 g_debug ("[GetMute]\n");

 if (!check_instanceId (action)) 
        return;

 renderingcontrol_get_mute_action_set(action, nvram_match("audio_mute", "1")?TRUE:FALSE);
 gupnp_service_action_return (action);    
}   

void
set_mute_cb (GUPnPService *service,
		     GUPnPServiceAction *action,
		     gpointer user_data)
{
 g_debug ("[SetMute]\n");
 
 if (!check_instanceId (action)) 
        return;

#ifdef I2C_TO_ALC5627
 int fd = open_mixer_fd();
#endif
 guint instance_id;
 gchar *channel = NULL;
 gboolean mute=FALSE;
 renderingcontrol_set_mute_action_get (action,
                                        &instance_id,
                                        &channel,
					&mute);

 if (nvram_match("audio_mute", "0") && mute == TRUE)
 {
#if defined(I2C_TO_ALC5627)
	ioctl(fd, RT2880_I2C_SET_DAC_MUTE, 0);
#elif defined(USB_TO_CM6510)
	system("amixer cset numid=3 0\n");
#elif defined(XXXXX)
//TBD.
#endif
	nvram_set("audio_mute", "1");
 }
 else
 {
#if defined(I2C_TO_ALC5627)
	ioctl(fd, RT2880_I2C_SET_DAC_UNMUTE, 0);
#elif defined(USB_TO_CM6510)
	system("amixer cset numid=3 1\n");
#elif defined(XXXXX)
//TBD.
#endif
	nvram_set("audio_mute", "0");
 }

#ifdef I2C_TO_ALC5627
 close(fd);
#endif
 gupnp_service_action_return (action);    
}   
//			<name>ListPresets</name>
//			<name>SelectPreset</name>


void
get_volume_cb (GUPnPService *service,
		     GUPnPServiceAction *action,
		     gpointer user_data)
{
 g_debug ("[GetVolume]\n");
 renderingcontrol_get_volume_action_set(action, atoi(nvram_safe_get("audio_volume")));
 gupnp_service_action_return (action);    
}   

void
set_volume_cb (GUPnPService *service,
		     GUPnPServiceAction *action,
		     gpointer user_data)
{
 g_debug ("[SetVolume]\n");

 if (!check_instanceId (action)) 
        return;

 guint instance_id;
 gchar *channel = NULL;
 guint volume = 0;

 renderingcontrol_set_volume_action_get (action,
                                        &instance_id,
                                        &channel,
					&volume);
 volume=(volume<=0)?0:volume;
 volume=(volume>=100)?100:volume;

 if (atoi(nvram_safe_get("audio_volume")) != volume)
 {   
	doSystem("nvram set audio_volume=%d", volume);
	nvram_commit();
 }	

 volume_control(volume);
 gupnp_service_action_return (action);    
}   

#define total_eq 10
char eq_mode[total_eq][20]={"Flat","Rock","R&B","Pop","Jazz","Hip-Hop","Smooth","Bass Booster","Dance","Treble Booster"};

void list_presets_cb (GUPnPService *service,
		     GUPnPServiceAction *action,
		     gpointer user_data)
{
 int i;
 gchar presetname[250];
 if (!check_instanceId (action)) 
 {
        return;
 }

 memset(presetname,0,sizeof(presetname));
 for(i=0;i<total_eq;i++)
 {   
 	strcat(presetname,eq_mode[i]);
  	if(i<(total_eq-1))
  		strcat(presetname,",");
 }
 renderingcontrol_list_presets_action_set (action,presetname);
 gupnp_service_action_return (action);    
}

int set_eq(int eq)
{
 char cmd[15];
 memset(cmd,0,sizeof(cmd));
#ifdef USB_TO_CM6510
 sprintf(cmd,"i2ctrl e %d",eq);
 system(cmd);
 //printf("set eq=%d\n",eq);
#endif
 return 0;
} 

void
select_preset_cb (GUPnPService *service,
		     GUPnPServiceAction *action,
		     gpointer user_data)
{
 char cmd[50];
 int i,eq_idx;
 if (!check_instanceId (action)) 
 {
        return;
 }
 guint instance_id;
 gchar *preset = NULL;
 renderingcontrol_select_preset_action_get (action,
                                        &instance_id,
                                        &preset);
 

 eq_idx=0;
 for(i=0;i<total_eq;i++)
    if(!strcmp(eq_mode[i],preset))
	eq_idx=i;

 set_eq(eq_idx);
 memset(cmd,0,sizeof(cmd));
 sprintf(cmd,"nvram set eq_cur_idx=%d",eq_idx);
 system(cmd);
 nvram_commit();
 gupnp_service_action_return (action);    

}


//////////////////////////////////////////////////////////
// Misc
//////////////////////////////////////////////////////////

