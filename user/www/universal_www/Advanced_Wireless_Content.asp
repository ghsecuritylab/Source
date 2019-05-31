<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<html xmlns:v>
<head>
<meta http-equiv="X-UA-Compatible" content="IE=EmulateIE7"/>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">

<title><#Web_Title#> - <#menu2_1#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<link href="other.css"  rel="stylesheet" type="text/css">
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/md5.js"></script>
<script type="text/javascript" src="/detect.js"></script>
<script>
<% wl_get_parameter(); %>

var wl_unit_var = '<% nvram_get("wl_unit"); %>';
var wl_ch_vitems = <% select_channel(); %>;

function initial(){
	show_menu();	
	show_band_select_option();

	change_common(document.form.wl_nmode_x, 'WLANConfig11b', 'wl_nmode_x');
	wl_auth_mode_change(1);

	document.form.wl_ssid.value = decodeURIComponent(document.form.wl_ssid_org.value);
	document.form.wl_wpa_psk.value = decodeURIComponent(document.form.wl_wpa_psk_org.value);
	document.form.wl_key1.value = decodeURIComponent(document.form.wl_key1_org.value);
	document.form.wl_key2.value = decodeURIComponent(document.form.wl_key2_org.value);
	document.form.wl_key3.value = decodeURIComponent(document.form.wl_key3_org.value);
	document.form.wl_key4.value = decodeURIComponent(document.form.wl_key4_org.value);
	document.form.wl_channel.value = document.form.wl_channel_orig.value;
	show_DFS_hint(document.form.wl_channel);

	if (wl_unit_var == '0') {
		$("wl_gmode_checkbox").style.display = "";
		remove_11ac_80MHz_option();
	} else {
		if (band5g_11ac_support != -1)
			document.form.wl_nmode_x[1].text = "N + AC";
		else
			remove_11ac_80MHz_option();
	}
	if(document.form.wl_nmode_x.value=='1'){
		document.form.wl_gmode_check.checked = false;
		$("wl_gmode_check").disabled = true;
	}
	else{
		document.form.wl_gmode_check.checked = true;
		$("wl_gmode_check").disabled = false;
	}
	if(document.form.wl_gmode_protection.value == "auto")
		document.form.wl_gmode_check.checked = true;
	else
		document.form.wl_gmode_check.checked = false;
	if(sw_mode == 2)
		disableAdvFn();
	need_action_script(document.form.action_script);
}

/* convert ASUSWRT value into Ralink platform value*/
function convert_wireless_auth_value(){
	var auth_mode = document.form.wl_auth_mode_x.value;
	var opts = document.form.wl_auth_mode_x.options;

	if(opts[opts.selectedIndex].text == "Open System"){
		document.form.wl_auth_mode.value = "open";
		document.form.wl_wpa_mode.value = "0";
	}
	else if(opts[opts.selectedIndex].text == "Shared Key"){
		document.form.wl_auth_mode.value = "shared";
	}
	else if(opts[opts.selectedIndex].text == "WPA-Personal"){
		document.form.wl_auth_mode.value = "psk";
		document.form.wl_wpa_mode.value = "1";
	}
	else if(opts[opts.selectedIndex].text == "WPA2-Personal"){
		document.form.wl_auth_mode.value = "psk";
		document.form.wl_wpa_mode.value = "2";
	}
	else if(opts[opts.selectedIndex].text == "WPA-Auto-Personal"){
		document.form.wl_auth_mode.value = "psk";
		document.form.wl_wpa_mode.value = "0";
	}
	else if(opts[opts.selectedIndex].text == "WPA-Enterprise"){
		document.form.wl_auth_mode.value = "wpa";
		document.form.wl_wpa_mode.value = "3";
	}
	else if(opts[opts.selectedIndex].text == "WPA-Auto-Enterprise"){
		document.form.wl_auth_mode.value = "wpa";
		document.form.wl_wpa_mode.value = "4";
	}
	else if(opts[opts.selectedIndex].text == "WPA2-Enterprise"){
		document.form.wl_auth_mode.value = "wpa2";
	}
}

function applyRule(){
	var auth_mode = document.form.wl_auth_mode_x.value;

	document.form.wl_HT_BW.value = document.form.wl_bw.value;
	
	if(document.form.wl_wpa_psk.value == "Please type Password")
		document.form.wl_wpa_psk.value = "";

	if(validForm()){
		convert_wireless_auth_value();

		if((auth_mode == "shared" || auth_mode == "wpa" || auth_mode == "wpa2"  || auth_mode == "wpawpa2" || auth_mode == "radius" ||
				((auth_mode == "open") && !(document.form.wl_wep_x.value == "0")))
				&& document.form.wps_mode.value == "enabled")
			document.form.wps_mode.value = "disabled";
		
		if(auth_mode == "wpa" || auth_mode == "wpa2" || auth_mode == "wpawpa2" || auth_mode == "radius")
			document.form.next_page.value = "/Advanced_WSecurity_Content.asp";
		
		inputCtrl(document.form.wl_crypto, 1);
		inputCtrl(document.form.wl_wpa_psk, 1);
		inputCtrl(document.form.wl_wep_x, 1);
		inputCtrl(document.form.wl_key, 1);
		inputCtrl(document.form.wl_key1, 1);
		inputCtrl(document.form.wl_key2, 1);
		inputCtrl(document.form.wl_key3, 1);
		inputCtrl(document.form.wl_key4, 1);
		inputCtrl(document.form.wl_wpa_gtk_rekey, 1);
		
		document.form.submit();
	}
}

function validForm(){
	var auth_mode = document.form.wl_auth_mode_x.value;
	
	if(!validate_string_ssid(document.form.wl_ssid))
		return false;
	
	if(auth_mode == "psk" || auth_mode == "psk2" || auth_mode == "pskpsk2"){ //2008.08.04 lock modified
		if(!validate_psk(document.form.wl_wpa_psk))
			return false;
		
		if(!validate_range(document.form.wl_wpa_gtk_rekey, 0, 2592000))
			return false;
	}
	else if(auth_mode == "wpa" || auth_mode == "wpa2" || auth_mode == "wpawpa2"){
		if(!validate_range(document.form.wl_wpa_gtk_rekey, 0, 2592000))
			return false;
	}
	else{
		var cur_wep_key = eval('document.form.wl_key'+document.form.wl_key.value);		
		if(auth_mode != "radius" && !validate_wep_key(document.form.wl_wep_x, cur_wep_key, document.form.wl_key_type))
			return false;
	}	
	return true;
}

function change_key_des(){
	var objs = getElementsByName_iefix("span", "key_des");
	var wep_type = document.form.wl_wep_x.value;
	var str = "";

	if(wep_type == "1")
		str = "(<#WLANConfig11b_WEPKey_itemtype1#>)";
	else if(wep_type == "2")
		str = "(<#WLANConfig11b_WEPKey_itemtype2#>)";
	
	for(var i = 0; i < objs.length; ++i)
		showtext(objs[i], str);
}

function disableAdvFn(){  /* this function can hide wirless option */
	$("WLgeneral").deleteRow(0);
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

<iframe name="hidden_frame" id="hidden_frame" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="current_page" value="Advanced_Wireless_Content.asp">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="action_mode" value=" Apply ">
<input type="hidden" name="action_script" value="restart_wifi">
<input type="hidden" name="action_wait" value="10">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="wps_mode" value="<% nvram_get("wps_mode"); %>">
<input type="hidden" name="wl_ssid_org" value="<% nvram_char_to_ascii("wl_ssid"); %>">
<input type="hidden" name="wl_auth_mode" value="<% nvram_get("wl_auth_mode"); %>">
<input type="hidden" name="wl_wpa_mode" value="<% nvram_get("wl_wpa_mode"); %>">
<input type="hidden" name="wl_wpa_psk_org" value="<% nvram_char_to_ascii("wl_wpa_psk"); %>">
<input type="hidden" name="wl_key1_org" value="<% nvram_char_to_ascii("wl_key1"); %>">
<input type="hidden" name="wl_key2_org" value="<% nvram_char_to_ascii("wl_key2"); %>">
<input type="hidden" name="wl_key3_org" value="<% nvram_char_to_ascii("wl_key3"); %>">
<input type="hidden" name="wl_key4_org" value="<% nvram_char_to_ascii("wl_key4"); %>">
<input type="hidden" name="wl_gmode_protection" value="<% nvram_get("wl_gmode_protection"); %>">
<input type="hidden" name="wl_wme" value="<% nvram_get("wl_wme"); %>">
<input type="hidden" name="wl_nctrlsb_old" value="<% nvram_get("wl_nctrlsb"); %>">
<input type="hidden" name="wl_key_type" value='<% nvram_get("wl_key_type"); %>'>
<input type="hidden" name="wl_channel_orig" value='<% nvram_get("wl_channel"); %>'>
<input type="hidden" name="wl_wep_x_orig" value='<% nvram_get("wl_wep_x"); %>'>
<input type="hidden" name="wl_HT_BW" id="wl_HT_BW" value="<% nvram_get("wl_HT_BW"); %>">

<table class="content" align="center" cellpadding="0" cellspacing="0">
  <tr>
	<td width="17">&nbsp;</td>
	
	<!--=====Beginning of Main Menu=====-->
	<td valign="top" width="202">
	  <div id="mainMenu"></div>
	  <div id="subMenu"></div>
	</td>
	
	<td valign="top">
	  <div id="tabMenu" class="submenuBlock"></div>

<!--===================================Beginning of Main Content===========================================-->
<table width="98%" border="0" align="left" cellpadding="0" cellspacing="0">
  <tr>
	<td align="left" valign="top" >
	  <table width="760px" border="0" cellpadding="4" cellspacing="0" class="FormTitle" id="FormTitle">
		<tbody>
		<tr>
		  <td bgcolor="#4D595D" valign="top"  >
		  <div>&nbsp;</div>
		  <div class="formfonttitle"><#menu2#> - <#menu2_1#></div>
		
      <div style="margin-left:5px;margin-top:10px;margin-bottom:10px"><img src="/images/New_ui/export/line_export.png"></div>
      <div class="formfontdesc"><#adv_wl_desc#></div>
		
			<table width="99%" border="1" align="center" cellpadding="4" cellspacing="0" id="WLgeneral" class="FormTable">
				<tr id="wl_unit_tr" style="display:none;">
					<th><#QIS_frequency#></th>
					<td>
						<select id="wl_unit" name="wl_unit" class="input_option" onChange="change_wl_unit();">
							<option class="content_input_fd" value="0" <% nvram_match("wl_unit", "0","selected"); %>>2.4GHz</option>
							<option class="content_input_fd" value="1" <% nvram_match("wl_unit", "1","selected"); %>>5GHz</option>
						</select>			
					</td>
			  	</tr>


				<tr>
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 1);"><#WLANConfig11b_SSID_itemname#></a></th>
					<td>
						<input type="text" maxlength="32" class="input_32_table" name="wl_ssid" value="<% nvram_get("wl_ssid"); %>" onkeypress="return is_string(this)">
					</td>
		  	</tr>
			  
				<tr>
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 2);"><#WLANConfig11b_x_BlockBCSSID_itemname#></a></th>
					<td>
						<input type="radio" value="1" name="wl_closed" class="input" onClick="return change_common_radio(this, 'WLANConfig11b', 'wl_closed', '1')" <% nvram_match("wl_closed", "1", "checked"); %>><#checkbox_Yes#>
						<input type="radio" value="0" name="wl_closed" class="input" onClick="return change_common_radio(this, 'WLANConfig11b', 'wl_closed', '0')" <% nvram_match("wl_closed", "0", "checked"); %>><#checkbox_No#>
					</td>
				</tr>
					  
			  <tr id="wl_nmode_x">
					<th><a class="hintstyle" href="javascript:void(0);" onClick="open_wifi_hint('wl_nmode_x');"><#WLANConfig11b_nMode_itemname#></a></th>
					<td>									
						<select name="wl_nmode_x" class="input_option" onChange="return change_common(this, 'WLANConfig11b', 'wl_nmode_x');">
							<option value="0" <% nvram_match("wl_nmode_x", "0", "selected"); %>>Auto</option>
							<option value="1" <% nvram_match("wl_nmode_x", "1", "selected"); %>>N Only</option>
							<option value="2" <% nvram_match("wl_nmode_x", "2", "selected"); %>>Legacy</option>
						</select>
						
					<span id="wl_gmode_checkbox" style="display:none;"><input type="checkbox" name="wl_gmode_check" id="wl_gmode_check" value="" onClick="return change_common(this, 'WLANConfig11b', 'wl_gmode_check', '1')"> b/g Protection</input></span>
						<span id="wl_nmode_x_hint" style="display:none;"><br><#WLANConfig11n_automode_limition_hint#></span>
					</td>
			  </tr>
			  
				<tr id="wl_channel_field">
					<th><a id="wl_channel_select" class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 3);"><#WLANConfig11b_Channel_itemname#></a></th>
					<td>
						<select name="wl_channel" class="input_option" onChange="return change_common(this, 'WLANConfig11b', 'wl_channel')"></select>
						<br><span id="wl_channel_hint" style="display:none;"><#WLANConfig11b_Channel_DFS_hint#></span>
					</td>
			  </tr>
			  
			 	<tr id="wl_bw_field">
			   	<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 14);"><#WLANConfig11b_ChannelBW_itemname#></a></th>
			   	<td>				    			
						<select name="wl_bw" class="input_option" onChange="return change_common(this, 'WLANConfig11b', 'wl_bw')">
							<option class="content_input_fd" value="1" <% nvram_match("wl_HT_BW", "1","selected"); %>>20/40/80 MHz</option>
							<option class="content_input_fd" value="0" <% nvram_match("wl_HT_BW", "0","selected"); %>>20 MHz</option>
							<option class="content_input_fd" value="2" <% nvram_match("wl_HT_BW", "2","selected"); %>>40 MHz</option>
							<option class="content_input_fd" value="3" <% nvram_match("wl_HT_BW", "3","selected"); %>>80 MHz</option>
						</select>				
			   	</td>
			 	</tr>

			  <tr id="wl_nctrlsb_field">
			  	<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 15);"><#WLANConfig11b_EChannel_itemname#></a></th>
		   		<td>
					<select name="wl_nctrlsb" class="input_option">
						<option value="lower" <% nvram_match("wl_nctrlsb", "lower", "selected"); %>>lower</option>
						<option value="upper"<% nvram_match("wl_nctrlsb", "upper", "selected"); %>>upper</option>
					</select>
					</td>
		  	</tr>
			  
			  	<tr>
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 5);"><#WLANConfig11b_AuthenticationMethod_itemname#></a></th>
					<td>
				  		<select name="wl_auth_mode_x" class="input_option" onChange="return change_common(this, 'WLANConfig11b', 'wl_auth_mode_x');">
							<option value="open" <% nvram_match("wl_auth_mode", "open", "selected"); %>>Open System</option>
							<option value="shared" <% nvram_match("wl_auth_mode", "shared", "selected"); %>>Shared Key</option>
							<option value="psk" <% nvram_double_match("wl_auth_mode", "psk", "wl_wpa_mode", "1", "selected"); %>>WPA-Personal</option>
							<option value="psk" <% nvram_double_match("wl_auth_mode", "psk", "wl_wpa_mode", "2", "selected"); %>>WPA2-Personal</option>
							<option value="psk" <% nvram_double_match("wl_auth_mode", "psk", "wl_wpa_mode", "0", "selected"); %>>WPA-Auto-Personal</option>
							<option value="wpa" <% nvram_double_match("wl_auth_mode", "wpa", "wl_wpa_mode", "3", "selected"); %>>WPA-Enterprise</option>
							<option value="wpa2" <% nvram_match("wl_auth_mode", "wpa2", "selected"); %>>WPA2-Enterprise</option>
							<option value="wpa" <% nvram_double_match("wl_auth_mode", "wpa", "wl_wpa_mode", "4", "selected"); %>>WPA-Auto-Enterprise</option>
							<!--option value="radius" <% nvram_match("wl_auth_mode", "radius", "selected"); %>>Radius with 802.1x</option-->
				  		</select>
					</td>
			  	</tr>
			  	
			  	<tr>
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 6);"><#WLANConfig11b_WPAType_itemname#></a></th>
					<td>		
				  		<select name="wl_crypto" class="input_option" onChange="return change_common(this, 'WLANConfig11b', 'wl_crypto')">
								<option value="aes" <% nvram_match("wl_crypto", "aes", "selected"); %>>AES</option>
								<option value="tkip+aes" <% nvram_match("wl_crypto", "tkip+aes", "selected"); %>>TKIP+AES</option>
				  		</select>
					</td>
			  	</tr>
			  
			  	<tr>
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 7);"><#WLANConfig11b_x_PSKKey_itemname#></a></th>
					<td>
				  		<input type="text" name="wl_wpa_psk" maxlength="64" class="input_32_table" value="<% nvram_get("wl_wpa_psk"); %>">
					</td>
			  	</tr>
			  		  
			  	<tr>
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 9);"><#WLANConfig11b_WEPType_itemname#></a></th>
					<td>
				  		<select name="wl_wep_x" class="input_option" onChange="return change_common(this, 'WLANConfig11b', 'wl_wep_x');">
								<option value="0" <% nvram_match("wl_wep_x", "0", "selected"); %>>None</option>
								<option value="1" <% nvram_match("wl_wep_x", "1", "selected"); %>>WEP-64bits</option>
								<option value="2" <% nvram_match("wl_wep_x", "2", "selected"); %>>WEP-128bits</option>
				  		</select>
				  		<span name="key_des"></span>
					</td>
			  	</tr>
			  
			  	<tr>
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 10);"><#WLANConfig11b_WEPDefaultKey_itemname#></a></th>
					<td>		
				  		<select name="wl_key" class="input_option"  onChange="return change_common(this, 'WLANConfig11b', 'wl_key');">
							<option value="1" <% nvram_match("wl_key", "1","selected"); %>>1</option>
							<option value="2" <% nvram_match("wl_key", "2","selected"); %>>2</option>
							<option value="3" <% nvram_match("wl_key", "3","selected"); %>>3</option>
							<option value="4" <% nvram_match("wl_key", "4","selected"); %>>4</option>
				  		</select>
					</td>
			  	</tr>
			  
			  	<tr>
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 18);"><#WLANConfig11b_WEPKey1_itemname#></th>
					<td><input type="text" name="wl_key1" id="wl_key1" maxlength="32" class="input_32_table" value="<% nvram_get("wl_key1"); %>" onKeyUp="return change_wlkey(this, 'WLANConfig11b');"></td>
			  	</tr>
			  
			  	<tr>
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 18);"><#WLANConfig11b_WEPKey2_itemname#></th>
					<td><input type="text" name="wl_key2" id="wl_key2" maxlength="32" class="input_32_table" value="<% nvram_get("wl_key2"); %>" onKeyUp="return change_wlkey(this, 'WLANConfig11b');"></td>
			  	</tr>
			  
			  	<tr>
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 18);"><#WLANConfig11b_WEPKey3_itemname#></th>
					<td><input type="text" name="wl_key3" id="wl_key3" maxlength="32" class="input_32_table" value="<% nvram_get("wl_key3"); %>" onKeyUp="return change_wlkey(this, 'WLANConfig11b');"></td>
			  	</tr>
			  
			  	<tr>
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 18);"><#WLANConfig11b_WEPKey4_itemname#></th>
					<td><input type="text" name="wl_key4" id="wl_key4" maxlength="32" class="input_32_table" value="<% nvram_get("wl_key4"); %>" onKeyUp="return change_wlkey(this, 'WLANConfig11b');"></td>
		  		</tr>

			  	<tr>
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 11);"><#WLANConfig11b_x_Rekey_itemname#></a></th>
					<td><input type="text" maxlength="7" name="wl_wpa_gtk_rekey" class="input_6_table"  value="<% nvram_get("wl_wpa_gtk_rekey"); %>" onKeyPress="return is_number_sp(event, this);" onblur="validate_number_range(this, 0, 259200)"></td>
			  	</tr>
				
		  	</table>
			  
				<div class="apply_gen">
					<input type="button" id="applyButton" class="button_gen" value="<#CTL_apply#>" onclick="applyRule();">
				</div>			  	
			  	
		  	</td>
		</tr>
		</tbody>
		
	  </table>
	</td>
</form>
</tr>
</table>
<!--===================================Ending of Main Content===========================================-->

	</td>
	
	<td width="10" align="center" valign="top"></td>
  </tr>
</table>
<div id="footer"></div>
</body>
</html>
