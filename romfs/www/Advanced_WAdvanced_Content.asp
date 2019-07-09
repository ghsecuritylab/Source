<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<html xmlns:v>
<head>
<meta http-equiv="X-UA-Compatible" content="IE=EmulateIE7"/>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8"/>
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title><#Web_Title#> - <#menu2_5#></title>
<link rel="stylesheet" type="text/css" href="index_style.css">
<link rel="stylesheet" type="text/css" href="form_style.css">
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/detect.js"></script>
<script type="text/javascript" src="/jquery.js"></script>
<script>
<% wl_get_parameter(); %>

var $j = jQuery.noConflict();
var old_rssi_dB = '<% nvram_get("wl_user_rssi"); %>';

function initial(){
	show_menu();
	show_band_select_option();

	if (old_rssi_dB != 0) {
		changeRSSI(old_rssi_dB);
		document.form.rssi_enable.selectedIndex = 1;
	}

	if (sw_mode == "2")
		disableAdvFn(19);
	else {
		$("wl_rate").style.display = "none";

		if (document.form.wl_nmode_x.value == "1" || document.form.wl_nmode_x.value == "0"){
			$("wl_wme").value = "on";
			inputCtrl(document.form.wl_frag, 0);
			inputCtrl(document.form.wl_wme, 0);
			inputCtrl(document.form.wl_wme_no_ack, 0);
		} else {
			inputCtrl(document.form.wl_frag, 1);
			inputCtrl(document.form.wl_wme, 1);
			inputCtrl(document.form.wl_wme_no_ack, 1);
		}

		document.form.wl_radio_date_x_Sun.checked = getDateCheck(document.form.wl_radio_date_x.value, 0);
		document.form.wl_radio_date_x_Mon.checked = getDateCheck(document.form.wl_radio_date_x.value, 1);
		document.form.wl_radio_date_x_Tue.checked = getDateCheck(document.form.wl_radio_date_x.value, 2);
		document.form.wl_radio_date_x_Wed.checked = getDateCheck(document.form.wl_radio_date_x.value, 3);
		document.form.wl_radio_date_x_Thu.checked = getDateCheck(document.form.wl_radio_date_x.value, 4);
		document.form.wl_radio_date_x_Fri.checked = getDateCheck(document.form.wl_radio_date_x.value, 5);
		document.form.wl_radio_date_x_Sat.checked = getDateCheck(document.form.wl_radio_date_x.value, 6);
		document.form.wl_radio_time_x_starthour.value = getTimeRange(document.form.wl_radio_time_x.value, 0);
		document.form.wl_radio_time_x_startmin.value = getTimeRange(document.form.wl_radio_time_x.value, 1);
		document.form.wl_radio_time_x_endhour.value = getTimeRange(document.form.wl_radio_time_x.value, 2);
		document.form.wl_radio_time_x_endmin.value = getTimeRange(document.form.wl_radio_time_x.value, 3);
		document.form.wl_radio_time2_x_starthour.value = getTimeRange(document.form.wl_radio_time2_x.value, 0);
		document.form.wl_radio_time2_x_startmin.value = getTimeRange(document.form.wl_radio_time2_x.value, 1);
		document.form.wl_radio_time2_x_endhour.value = getTimeRange(document.form.wl_radio_time2_x.value, 2);
		document.form.wl_radio_time2_x_endmin.value = getTimeRange(document.form.wl_radio_time2_x.value, 3);
		change_common_radio(document.form.wl_radio_x, '', 'wl_radio', '');
	}
	need_action_script(document.form.action_script);
}

function changeRSSI(v){
	if (v == 0) {
		document.getElementById("rssi_dB").style.display = "none";
		document.form.wl_user_rssi.value = 0;
	} else {
		document.getElementById("rssi_dB").style.display = "";
		if (old_rssi_dB == 0)
			document.form.wl_user_rssi.value = "-70";
		else
			document.form.wl_user_rssi.value = old_rssi_dB;
	}
}

function applyRule(){
	need_action_wait(document.form.action_script, sw_mode);
	if (sw_mode == "2") //not to verify any settings
		document.form.submit();
	else {
		if (validForm()) {
			updateDateTime(document.form.current_page.value);
			document.form.submit();
		}
	}
}

function validForm(){
	if (!validate_range(document.form.wl_frag, 256, 2346) 
			|| !validate_range(document.form.wl_rts, 0, 2347) 
			|| !validate_range(document.form.wl_dtim, 1, 255) 
			|| !validate_range(document.form.wl_bcn, 20, 1024))
		return false;

	if (!validate_timerange(document.form.wl_radio_time_x_starthour, 0) 
			|| !validate_timerange(document.form.wl_radio_time2_x_starthour, 0) 
			|| !validate_timerange(document.form.wl_radio_time_x_startmin, 1) 
			|| !validate_timerange(document.form.wl_radio_time2_x_startmin, 1) 
			|| !validate_timerange(document.form.wl_radio_time_x_endhour, 2) 
			|| !validate_timerange(document.form.wl_radio_time2_x_endhour, 2) 
			|| !validate_timerange(document.form.wl_radio_time_x_endmin, 3) 
			|| !validate_timerange(document.form.wl_radio_time2_x_endmin, 3))
		return false;

	return true;
}

function disableAdvFn(row){
	for (var i = row; i > 0; i--) {
		if (i == 8) continue;//roaming option
		$("WAdvTable").deleteRow(i);
	}
}
</script>
</head>

<body onload="initial();" onunLoad="return unload_body();">
<div id="TopBanner"></div>
<div id="Loading" class="popup_bg"></div>
<div id="hiddenMask" class="popup_bg">
  <table cellpadding="4" cellspacing="0" id="dr_sweet_advise" class="dr_sweet_advise" align="center">
	<tr>
	  <td>
		<div class="drword" id="drword"><#Main_alert_proceeding_desc4#> <#Main_alert_proceeding_desc1#>...
		  <br/>
		  <br/>
		</div>
		<div class="drImg"><img src="images/alertImg.gif"></div>
		<div style="height:70px; "></div>
	  </td>
	</tr>
  </table>
  <!--[if lte IE 6.5]><iframe class="hackiframe"></iframe><![endif]-->
</div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>

<form method="post" name="form" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="wl_nmode_x" value="<% nvram_get("wl_nmode_x"); %>">
<input type="hidden" name="current_page" value="Advanced_WAdvanced_Content.asp">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="action_mode" value=" Apply ">
<input type="hidden" name="action_script" value="restart_wifi">
<input type="hidden" name="action_wait" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="wl_radio_date_x" value="<% nvram_get("wl_radio_date_x"); %>">
<input type="hidden" name="wl_radio_time_x" value="<% nvram_get("wl_radio_time_x"); %>">
<input type="hidden" name="wl_radio_time2_x" value="<% nvram_get("wl_radio_time2_x"); %>">
<input type="hidden" name="wl_subunit" value="-1">

<table class="content" align="center" cellpadding="0" cellspacing="0">
  <tr>
	<td width="17">&nbsp;</td>
	<td valign="top" width="202">
	  <div  id="mainMenu"></div>
	  <div  id="subMenu"></div>
	</td>

	<td valign="top">
	  <div id="tabMenu" class="submenuBlock"></div>
	  <!--===================================Beginning of Main Content===========================================-->
	  <table width="98%" border="0" align="left" cellpadding="0" cellspacing="0">
		<tr>
		  <td valign="top">
			<table width="760px" border="0" cellpadding="4" cellspacing="0" class="FormTitle" id="FormTitle">
			  <tbody>
			  <tr>
				<td bgcolor="#4D595D" valign="top">
				  <div>&nbsp;</div>
				  <div class="formfonttitle"><#menu2#> - <#menu2_5#></div>
				  <div style="margin-left:5px;margin-top:10px;margin-bottom:10px"><img src="/images/New_ui/export/line_export.png"></div>
				  <div class="formfontdesc"><#WLANConfig11b_display5_sectiondesc#></div>

				  <table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable" id="WAdvTable">
					<tr id="wl_unit_tr" style="display:none;">
					  <th><#QIS_frequency#></th>
					  <td>
						<select name="wl_unit" class="input_option" onChange="change_wl_unit();">
						  <option class="content_input_fd" value="0" <% nvram_match("wl_unit", "0","selected"); %>>2.4GHz</option>
						  <option class="content_input_fd" value="1" <% nvram_match("wl_unit", "1","selected"); %>>5GHz</option>
						</select>
					  </td>
					</tr>
					<tr>
					  <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(3, 1);"><#WLANConfig11b_x_RadioEnable_itemname#></a></th>
					  <td>
						<input type="radio" value="1" name="wl_radio_x" class="input" onClick="return change_common_radio(this, 'WLANConfig11b', 'wl_radio', '1')" <% nvram_match("wl_radio_x", "1", "checked"); %>><#checkbox_Yes#>
						<input type="radio" value="0" name="wl_radio_x" class="input" onClick="return change_common_radio(this, 'WLANConfig11b', 'wl_radio', '0')" <% nvram_match("wl_radio_x", "0", "checked"); %>><#checkbox_No#>
					  </td>
					</tr>
					<tr id="weekday_tr">
					  <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(3, 2);"><#WLANConfig11b_x_RadioEnableDate_itemname#> (week days)</a></th>
					  <td>
						<input type="checkbox" class="input" name="wl_radio_date_x_Mon" onChange="return changeDate();"><#date_Mon_itemdesc#>
						<input type="checkbox" class="input" name="wl_radio_date_x_Tue" onChange="return changeDate();"><#date_Tue_itemdesc#>
						<input type="checkbox" class="input" name="wl_radio_date_x_Wed" onChange="return changeDate();"><#date_Wed_itemdesc#>
						<input type="checkbox" class="input" name="wl_radio_date_x_Thu" onChange="return changeDate();"><#date_Thu_itemdesc#>
						<input type="checkbox" class="input" name="wl_radio_date_x_Fri" onChange="return changeDate();"><#date_Fri_itemdesc#>
					  </td>
					</tr>
					<tr id="weekday_time_tr">
					  <th><a class="hintstyle"  href="javascript:void(0);" onClick="openHint(3, 3);"><#WLANConfig11b_x_RadioEnableTime_itemname#></a></th>
					  <td>
						<input type="text" maxlength="2" class="input_3_table" name="wl_radio_time_x_starthour" onKeyPress="return is_number_sp(event, this)" onblur="validate_timerange(this, 0);">:
						<input type="text" maxlength="2" class="input_3_table" name="wl_radio_time_x_startmin" onKeyPress="return is_number_sp(event, this)" onblur="validate_timerange(this, 1);">-
						<input type="text" maxlength="2" class="input_3_table" name="wl_radio_time_x_endhour" onKeyPress="return is_number_sp(event, this)" onblur="validate_timerange(this, 2);">:
						<input type="text" maxlength="2" class="input_3_table" name="wl_radio_time_x_endmin" onKeyPress="return is_number_sp(event, this)" onblur="validate_timerange(this, 3);">
					  </td>
					</tr>
					<tr id="weekend_tr">
					  <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(3, 2);"><#WLANConfig11b_x_RadioEnableDate_itemname#> (weekend)</a></th>
					  <td>
						<input type="checkbox" class="input" name="wl_radio_date_x_Sun" onChange="return changeDate();"><#date_Sun_itemdesc#>
						<input type="checkbox" class="input" name="wl_radio_date_x_Sat" onChange="return changeDate();"><#date_Sat_itemdesc#>
					  </td>
					</tr>
					<tr id="weekend_time_tr">
					  <th><a class="hintstyle"  href="javascript:void(0);" onClick="openHint(3, 3);"><#WLANConfig11b_x_RadioEnableTime_itemname#></a></th>
					  <td>
						<input type="text" maxlength="2" class="input_3_table" name="wl_radio_time2_x_starthour" onKeyPress="return is_number_sp(event, this)" onblur="validate_timerange(this, 0);">:
						<input type="text" maxlength="2" class="input_3_table" name="wl_radio_time2_x_startmin" onKeyPress="return is_number_sp(event, this)" onblur="validate_timerange(this, 1);">-
						<input type="text" maxlength="2" class="input_3_table" name="wl_radio_time2_x_endhour" onKeyPress="return is_number_sp(event, this)" onblur="validate_timerange(this, 2);">:
						<input type="text" maxlength="2" class="input_3_table" name="wl_radio_time2_x_endmin" onKeyPress="return is_number_sp(event, this)" onblur="validate_timerange(this, 3);">
					  </td>
					</tr>
					<tr>
					  <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(3, 5);"><#WLANConfig11b_x_IsolateAP_itemname#></a></th>
					  <td>
						<input type="radio" value="1" name="wl_ap_isolate" class="input" onClick="return change_common_radio(this, 'WLANConfig11b', 'wl_ap_isolate', '1')" <% nvram_match("wl_ap_isolate", "1", "checked"); %>><#checkbox_Yes#>
						<input type="radio" value="0" name="wl_ap_isolate" class="input" onClick="return change_common_radio(this, 'WLANConfig11b', 'wl_ap_isolate', '0')" <% nvram_match("wl_ap_isolate", "0", "checked"); %>><#checkbox_No#>
					  </td>
					</tr>
					<tr id="wl_rate">
					  <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(3, 6);"><#WLANConfig11b_DataRateAll_itemname#></a></th>
					  <td>
						<select name="wl_rate" class="input_option" onChange="return change_common(this, 'WLANConfig11b', 'wl_rate')">
						  <option value="0" <% nvram_match("wl_rate", "0","selected"); %>>Auto</option>
						  <option value="1000000" <% nvram_match("wl_rate", "1000000","selected"); %>>1</option>
						  <option value="2000000" <% nvram_match("wl_rate", "2000000","selected"); %>>2</option>
						  <option value="5500000" <% nvram_match("wl_rate", "5500000","selected"); %>>5.5</option>
						  <option value="6000000" <% nvram_match("wl_rate", "6000000","selected"); %>>6</option>
						  <option value="9000000" <% nvram_match("wl_rate", "9000000","selected"); %>>9</option>
						  <option value="11000000" <% nvram_match("wl_rate", "11000000","selected"); %>>11</option>
						  <option value="12000000" <% nvram_match("wl_rate", "12000000","selected"); %>>12</option>
						  <option value="18000000" <% nvram_match("wl_rate", "18000000","selected"); %>>18</option>
						  <option value="24000000" <% nvram_match("wl_rate", "24000000","selected"); %>>24</option>
						  <option value="36000000" <% nvram_match("wl_rate", "36000000","selected"); %>>36</option>
						  <option value="48000000" <% nvram_match("wl_rate", "48000000","selected"); %>>48</option>
						  <option value="54000000" <% nvram_match("wl_rate", "54000000","selected"); %>>54</option>
						</select>
					  </td>
					</tr>
					<tr>
					  <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(3, 21);"><#WLANConfig11b_Roaming_itemname#></a></th>
					  <td>
						<select name="rssi_enable" class="input_option" onchange="changeRSSI(this.value);">
						  <option value="0"><#WLANConfig11b_WirelessCtrl_buttonname#></option>
						  <option value="1"><#WLANConfig11b_WirelessCtrl_button1name#></option>
						</select>
						<span id="rssi_dB" style="color:#FFF;display:none"><#WLANConfig11b_Roaming_desc#><input type="text" maxlength="3" name="wl_user_rssi" class="input_3_table" value="<% nvram_get("wl_user_rssi"); %>">dB</span>
					  </td>
					</tr>
					<tr>
					  <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(3, 7);"><#WLANConfig11b_MultiRateAll_itemname#></a></th>
					  <td>
						<select name="wl_mrate" class="input_option" onChange="return change_common(this, 'WLANConfig11b', 'wl_mrate')">
						  <option value="0" <% nvram_match("wl_mrate", "0", "selected"); %>>Disable</option>
						  <option value="13" <% nvram_match("wl_mrate", "13", "selected"); %>>Auto</option>
						</select>
					  </td>
					</tr>
					<tr>
					  <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(3, 9);"><#WLANConfig11b_x_Frag_itemname#></a></th>
					  <td><input type="text" maxlength="4" name="wl_frag" id="wl_frag" class="input_6_table" value="<% nvram_get("wl_frag"); %>" onKeyPress="return is_number(this, event)" onChange="page_changed()"></td>
					</tr>
					<tr>
					  <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(3, 10);"><#WLANConfig11b_x_RTS_itemname#></a></th>
					  <td><input type="text" maxlength="4" name="wl_rts" class="input_6_table" value="<% nvram_get("wl_rts"); %>" onKeyPress="return is_number(this, event)"></td>
					</tr>
					<tr>
					  <th><a class="hintstyle"  href="javascript:void(0);" onClick="openHint(3, 11);"><#WLANConfig11b_x_DTIM_itemname#></a></th>
					  <td><input type="text" maxlength="3" name="wl_dtim" class="input_6_table" value="<% nvram_get("wl_dtim"); %>" onKeyPress="return is_number(this, event)"></td>
					</tr>
					<tr>
					  <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(3, 12);"><#WLANConfig11b_x_Beacon_itemname#></a></th>
					  <td><input type="text" maxlength="4" name="wl_bcn" class="input_6_table" value="<% nvram_get("wl_bcn"); %>" onKeyPress="return is_number(this, event)"></td>
					</tr>
					<tr>
					  <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(3, 13);"><#WLANConfig11b_x_TxBurst_itemname#></a></th>
					  <td>
						<select name="wl_TxBurst" class="input_option" onChange="return change_common(this, 'WLANConfig11b', 'wl_frameburst')">
						  <option value="0" <% nvram_match("wl_TxBurst", "0","selected"); %>><#WLANConfig11b_WirelessCtrl_buttonname#></option>
						  <option value="1" <% nvram_match("wl_TxBurst", "1","selected"); %>><#WLANConfig11b_WirelessCtrl_button1name#></option>
						</select>
					  </td>
					</tr>
					<tr>
					  <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(3, 16);"><#WLANConfig11b_x_PktAggregate_itemname#></a></th>
					  <td>
						<select name="wl_PktAggregate" class="input_option" onChange="return change_common(this, 'WLANConfig11b', 'wl_PktAggregate')">
						  <option value="0" <% nvram_match("wl_PktAggregate", "0","selected"); %>><#WLANConfig11b_WirelessCtrl_buttonname#></option>
						  <option value="1" <% nvram_match("wl_PktAggregate", "1","selected"); %>><#WLANConfig11b_WirelessCtrl_button1name#></option>
						</select>
					  </td>
					</tr>
					<tr>
					  <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(3, 19);"><#WLANConfig11b_x_HT_OpMode_itemname#></a></th>
					  <td>
						<select class="input_option" name="wl_HT_OpMode" onChange="return change_common(this, 'WLANConfig11b', 'wl_HT_OpMode')">
						  <option value="0" <% nvram_match("wl_HT_OpMode", "0","selected"); %>><#WLANConfig11b_WirelessCtrl_buttonname#></option>
						  <option value="1" <% nvram_match("wl_HT_OpMode", "1","selected"); %>><#WLANConfig11b_WirelessCtrl_button1name#></option>
						</select>
					  </td>
					</tr>
					<tr>
					  <th><a class="hintstyle"  href="javascript:void(0);" onClick="openHint(3, 14);"><#WLANConfig11b_x_WMM_itemname#></a></th>
					  <td>
						<select name="wl_wme" id="wl_wme" class="input_option" onChange="return change_common(this, 'WLANConfig11b', 'wl_wme')">
						  <option value="0" <% nvram_match("wl_wme", "0", "selected"); %>><#WLANConfig11b_WirelessCtrl_buttonname#></option>
						  <option value="1" <% nvram_match("wl_wme", "1", "selected"); %>><#WLANConfig11b_WirelessCtrl_button1name#></option>
						</select>
					  </td>
					</tr>
					<tr>
					  <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(3,15);"><#WLANConfig11b_x_NOACK_itemname#></a></th>
					  <td>
						<select name="wl_wme_no_ack" class="input_option" onChange="return change_common(this, 'WLANConfig11b', 'wl_wme_no_ack')">
						  <option value="off" <% nvram_match("wl_wme_no_ack", "off","selected"); %>><#WLANConfig11b_WirelessCtrl_buttonname#></option>
						  <option value="on" <% nvram_match("wl_wme_no_ack", "on","selected"); %>><#WLANConfig11b_WirelessCtrl_button1name#></option>
						</select>
					  </td>
					</tr>
					<tr>
					  <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(3,17);"><#WLANConfig11b_x_APSD_itemname#></a></th>
					  <td>
						<select name="wl_APSDCapable" class="input_option" onchange="return change_common(this, 'WLANConfig11b', 'wl_APSDCapable')">
						  <option value="0" <% nvram_match("wl_APSDCapable", "0","selected"); %> ><#WLANConfig11b_WirelessCtrl_buttonname#></option>
						  <option value="1" <% nvram_match("wl_APSDCapable", "1","selected"); %> ><#WLANConfig11b_WirelessCtrl_button1name#></option>
						</select>
					  </td>
					</tr>
					<tr>
					  <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(3,18);"><#WLANConfig11b_x_DLS_itemname#></a></th>
					  <td>
						<select name="wl_DLSCapable" class="input_option" onChange="return change_common(this, 'WLANConfig11b', 'wl_DLSCapable')">
						  <option value="0" <% nvram_match("wl_DLSCapable", "0","selected"); %>><#WLANConfig11b_WirelessCtrl_buttonname#></option>
						  <option value="1" <% nvram_match("wl_DLSCapable", "1","selected"); %>><#WLANConfig11b_WirelessCtrl_button1name#></option>
						</select>
					  </td>
					</tr>
				  </table>

				  <div class="apply_gen">
					<input class="button_gen" onclick="applyRule();" type="button" value="<#CTL_apply#>"/>
				  </div>
				</td>
			  </tr>
			  </tbody>
			</table>
		  </td>
		</tr>
	  </table>
	  <!--===================================Ending of Main Content===========================================-->
	</td>
	<td width="10" align="center" valign="top">&nbsp;</td>
  </tr>
</table>
</form>

<div id="footer"></div>

</body>
</html>
