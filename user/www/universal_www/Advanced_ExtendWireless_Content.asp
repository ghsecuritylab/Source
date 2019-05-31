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
<script type="text/javascript" src="/jquery.js"></script>
<script>
<% wl_get_parameter(); %>

var $j = jQuery.noConflict();
var wl_unit_var = '<% nvram_get("wl_unit"); %>';

function initial(){
	show_menu();	
	show_band_select_option();
	show_nmode_option();
	changeHandler.changeAuthMode(document.form.wl_auth_mode_x, 1);

	document.form.wl_ssid.value = decodeURIComponent(document.form.wl_ssid_org.value);
	document.form.wl_wpa_psk.value = decodeURIComponent(document.form.wl_wpa_psk_org.value);
	document.form.wl_key1.value = decodeURIComponent(document.form.wl_key1_org.value);
	document.form.wl_key2.value = decodeURIComponent(document.form.wl_key2_org.value);
	document.form.wl_key3.value = decodeURIComponent(document.form.wl_key3_org.value);
	document.form.wl_key4.value = decodeURIComponent(document.form.wl_key4_org.value);
	need_action_script(document.form.action_script);
}

function show_nmode_option(){
	if (wl_unit_var == '1') {
		if (band5g_11ac_support != -1)
			document.getElementById('wl_nmode_desc').innerHTML = "Auto : 11a/an/ac";
		else {
			document.getElementById('wl_nmode_desc').innerHTML = "Auto : 11a/an";
			remove_11ac_80MHz_option();
		}
	} else {
		document.getElementById('wl_nmode_desc').innerHTML = "Auto : 11b/g/n";
		remove_11ac_80MHz_option();
	}
}

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
}

function applyRule(){
	document.form.wl_HT_BW.value = document.form.wl_bw.value;
	if(validForm()){
		convert_wireless_auth_value();
		need_action_wait(document.form.action_script, sw_mode);
		document.form.submit();
	}
}

function validForm(){
	var auth_mode = document.form.wl_auth_mode_x.value;

	if(!validate_string_ssid(document.form.wl_ssid))
		return false;

	if (auth_mode == "psk") {
		if(!validate_psk(document.form.wl_wpa_psk))
			return false;
		if(!validate_range(document.form.wl_wpa_gtk_rekey, 0, 2592000))
			return false;
	} else {
		var cur_wep_key = eval('document.form.wl_key'+document.form.wl_key.value);		
		if (!validate_wep_key(document.form.wl_wep_x, cur_wep_key, document.form.wl_key_type))
			return false;
	}
	return true;
}

var changeHandler = {
	"changeAuthMode" : function(o, reload) {
		if (o.value.search("psk") != -1) {
			document.getElementById("wl_crypto").parentNode.parentNode.style.display = "";
			document.getElementById("wl_wpa_psk").parentNode.parentNode.style.display = "";
			document.getElementById("wl_wep_x").parentNode.parentNode.style.display = "none";
			document.getElementById("wl_key").parentNode.parentNode.style.display = "none";
			document.getElementById("wl_key1").parentNode.parentNode.style.display = "none";
			document.getElementById("wl_key2").parentNode.parentNode.style.display = "none";
			document.getElementById("wl_key3").parentNode.parentNode.style.display = "none";
			document.getElementById("wl_key4").parentNode.parentNode.style.display = "none";
			if (reload == 0) {
				if (o.selectedIndex == 2)
					document.getElementById("wl_crypto").selectedIndex = 0;
				else if (o.selectedIndex == 3)
					document.getElementById("wl_crypto").selectedIndex = 1;
			}
			this.changeCrypto();
		} else {
			document.getElementById("wl_crypto").parentNode.parentNode.style.display = "none";
			document.getElementById("wl_wpa_psk").parentNode.parentNode.style.display = "none";
			document.getElementById("wl_wep_x").parentNode.parentNode.style.display = "";
			if (o.value == "shared")
				this.regenOption(document.getElementById("wl_wep_x"), ["WEP-64bits", "WEP-128bits"], ["1", "2"]);
			else
				this.regenOption(document.getElementById("wl_wep_x"), ["<#PPPConnection_Authentication_null#>", "WEP-64bits", "WEP-128bits"], ["0", "1", "2"]);
			this.changeWepMode();
		}
	},
	"changeCrypto" : function() {
		var v = document.form.wl_crypto.value;
		if (v == "tkip")
			document.getElementById("wl_nmode_hint").style.display = "";
		else
			document.getElementById("wl_nmode_hint").style.display = "none";
	},
	"changeWepMode" : function() {
		var str = "";
		var o = getElementsByName_iefix("span", "key_des");
		var v = document.form.wl_wep_x.value;
		document.getElementById("wl_nmode_hint").style.display = (v == 0) ? "none" : "";
		document.getElementById("wl_key").parentNode.parentNode.style.display = (v == 0) ? "none" : "";
		document.getElementById("wl_key1").parentNode.parentNode.style.display = (v == 0) ? "none" : "";
		document.getElementById("wl_key2").parentNode.parentNode.style.display = (v == 0) ? "none" : "";
		document.getElementById("wl_key3").parentNode.parentNode.style.display = (v == 0) ? "none" : "";
		document.getElementById("wl_key4").parentNode.parentNode.style.display = (v == 0) ? "none" : "";
		if (v == 1)
			str = "(<#WLANConfig11b_WEPKey_itemtype1#>)";
		else if (v == 2)
			str = "(<#WLANConfig11b_WEPKey_itemtype2#>)";
		for(var i = 0; i < o.length; ++i)
			showtext(o[i], str);
	},
	"changeWepKeyIdx" : function(v) {
		var o = eval("document.form.wl_key"+v);
		o.focus();
		o.select();
	},
	"regenOption" : function(o, optArray, valArray) {
		o.options.length = 0;
		for (var i=0; i<optArray.length; i++) {
			newItem = new Option(optArray[i], valArray[i]);
			o.options.add(newItem);
		}
		o.selectedIndex = 0;
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

<iframe name="hidden_frame" id="hidden_frame" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="current_page" value="Advanced_ExtendWireless_Content.asp">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="action_mode" value=" Apply ">
<input type="hidden" name="action_script" value="restart_wifi">
<input type="hidden" name="action_wait" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="wl_ssid_org" value="<% nvram_char_to_ascii("wl_ssid"); %>">
<input type="hidden" name="wl_auth_mode" value="<% nvram_get("wl_auth_mode"); %>">
<input type="hidden" name="wl_wpa_mode" value="<% nvram_get("wl_wpa_mode"); %>">
<input type="hidden" name="wl_wpa_psk_org" value="<% nvram_char_to_ascii("wl_wpa_psk"); %>">
<input type="hidden" name="wl_key1_org" value="<% nvram_char_to_ascii("wl_key1"); %>">
<input type="hidden" name="wl_key2_org" value="<% nvram_char_to_ascii("wl_key2"); %>">
<input type="hidden" name="wl_key3_org" value="<% nvram_char_to_ascii("wl_key3"); %>">
<input type="hidden" name="wl_key4_org" value="<% nvram_char_to_ascii("wl_key4"); %>">
<input type="hidden" name="wl_key_type" value='<% nvram_get("wl_key_type"); %>'>
<input type="hidden" name="wl_wep_x_orig" value='<% nvram_get("wl_wep_x"); %>'>
<input type="hidden" name="wl_HT_BW" id="wl_HT_BW" value="<% nvram_get("wl_HT_BW"); %>">

<table class="content" align="center" cellpadding="0" cellspacing="0"><!--table1-->
	<tr>
		<td width="17">&nbsp;</td>
	
		<!--=====Beginning of Main Menu=====-->
		<td valign="top" width="202">
			<div id="mainMenu"></div>
			<div id="subMenu"></div>
		</td>

		<td height="430" valign="top">
			<div id="tabMenu" class="submenuBlock"></div>

			<!--===================================Beginning of Main Content===========================================-->
			<table width="98%" border="0" align="left" cellpadding="0" cellspacing="0"><!--table2-->
			<tr>
				<td align="left" valign="top" >
					<table width="760px" border="0" cellpadding="4" cellspacing="0" class="FormTitle" id="FormTitle"><!--table3-->
					<tbody>
					<tr>
						<td bgcolor="#4D595D" valign="top">
							<div>&nbsp;</div>
							<div class="formfonttitle"><#menu2#> - <#menu2_6#></div>
							<div style="margin-left:5px;margin-top:10px;margin-bottom:10px"><img src="/images/New_ui/export/line_export.png"></div>
							<div class="formfontdesc"><#adv_wl_desc#></div>
		
							<table width="99%" border="1" align="center" cellpadding="4" cellspacing="0" id="WLgeneral" class="FormTable"><!--table4-->
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
										<p id="wl_nmode_desc"></p>
										<span id="wl_nmode_hint"><#WLANConfig11n_automode_limition_hint#></span>
									</td>
								</tr>
			 					<tr id="wl_bw_field">
									<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 14);"><#WLANConfig11b_ChannelBW_itemname#></a></th>
									<td>				    			
										<select name="wl_bw" class="input_option">
											<option class="content_input_fd" value="1" <% nvram_match("wl_HT_BW", "1","selected"); %>>20/40/80 MHz</option>
											<option class="content_input_fd" value="0" <% nvram_match("wl_HT_BW", "0","selected"); %>>20 MHz</option>
											<option class="content_input_fd" value="2" <% nvram_match("wl_HT_BW", "2","selected"); %>>40 MHz</option>
											<option class="content_input_fd" value="3" <% nvram_match("wl_HT_BW", "3","selected"); %>>80 MHz</option>
										</select>
								   	</td>
							 	</tr>
			  					<tr>
									<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 5);"><#WLANConfig11b_AuthenticationMethod_itemname#></a></th>
									<td>
								  		<select name="wl_auth_mode_x" class="input_option" onChange="changeHandler.changeAuthMode(this, 0)">
											<option value="open" <% nvram_match("wl_auth_mode", "open", "selected"); %>>Open System</option>
											<option value="shared" <% nvram_match("wl_auth_mode", "shared", "selected"); %>>Shared Key</option>
											<option value="psk" <% nvram_double_match("wl_auth_mode", "psk", "wl_wpa_mode", "1", "selected"); %>>WPA-Personal</option>
											<option value="psk" <% nvram_double_match("wl_auth_mode", "psk", "wl_wpa_mode", "2", "selected"); %>>WPA2-Personal</option>
				  						</select>
									</td>
								</tr>
								<tr>
									<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 6);"><#WLANConfig11b_WPAType_itemname#></a></th>
									<td>		
								  		<select name="wl_crypto" id="wl_crypto" class="input_option" onChange="changeHandler.changeCrypto()">
											<option value="tkip" <% nvram_match("wl_crypto", "tkip", "selected"); %>>TKIP</option>
											<option value="aes" <% nvram_match("wl_crypto", "aes", "selected"); %>>AES</option>
								  		</select>
									</td>
							  	</tr>
			  					<tr>
									<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 7);"><#WLANConfig11b_x_PSKKey_itemname#></a></th>
									<td>
								  		<input type="text" name="wl_wpa_psk" id="wl_wpa_psk" maxlength="64" class="input_32_table" value="<% nvram_get("wl_wpa_psk"); %>">
									</td>
							  	</tr>
			  					<tr>
									<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 9);"><#WLANConfig11b_WEPType_itemname#></a></th>
									<td>
								  		<select name="wl_wep_x" id="wl_wep_x" class="input_option" onChange="changeHandler.changeWepMode()">
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
								  		<select name="wl_key" id="wl_key" class="input_option"  onChange="changeHandler.changeWepKeyIdx(this.value)">
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
						  	</table><!--table4-->
							<div class="apply_gen">
								<input type="button" id="applyButton" class="button_gen" value="<#CTL_apply#>" onclick="applyRule();">
							</div>			  	
		  				</td>
					</tr>
					</tbody>
					</table><!--table3-->
				</td>
			</tr>
			</table><!--table2-->
			<!--===================================Ending of Main Content===========================================-->
		</td>
		<td width="10" align="center" valign="top"></td>
	</tr>
</table><!--table1-->
</form>
<div id="footer"></div>
</body>
</html>
