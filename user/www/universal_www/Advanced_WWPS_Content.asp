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
<title><#Web_Title#> - <#menu2_2#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/ajax.js"></script>
<script type="text/javascript" src="/detect.js"></script>
<script type="text/javascript" src="/jquery.js"></script>
<script type="text/javascript" src="/plugin/jquery.iphone-switch.js"></script>
<script>
<% wl_get_parameter(); %>

var $j = jQuery.noConflict();
var wps_enable_old = '<% nvram_get("wps_enable"); %>';
var secs;
var timerID = null;
var timerRunning = false;
var timeout = 2000;
var delay = 1000;

function initial(){
	show_menu();
	if (band5g_support != -1)
		$("wps_state1_tr").style.display = "";
	show_WPS_Repeater_btn(); 	// for wps repeater mode button
	if(disableWPS_item()) return;	// for wps client mode option under repeater mode
	
	if(!ValidateChecksum(document.form.wps_pin.value) || document.form.wps_pin.value == "00000000"){
		document.form.wps_method[0].checked = true;
		changemethod(0);
	}
	else{
		document.form.wps_method[1].checked = true;		
		changemethod(1);
	}

	loadXML();
}

function show_WPS_Repeater_btn(){
	if(sw_mode == 2){
		document.getElementById("WPS_Repeater_switch").style.display="";
		
		/* WPS client mode note */
		if(<% nvram_get("rep_wps_be_ap"); %> == 0)
			document.getElementById("wps_client_note").style.display="";
		else
			document.getElementById("wps_client_note").style.display="none";
	}
	else
		document.getElementById("WPS_Repeater_switch").style.display="none";
		
}

function applyRule(){
	stopFlag = 1;
	document.form.submit();
}

function Repeater_switch(){
	document.form.action_mode.value = "apply";
	applyRule();
}

function enableWPS(){
	FormActions("apply.cgi", "WPS_apply", "", "");
	document.form.target = "";
	applyRule();
}

function configCommand(){
	if(document.form.wps_method[1].checked == true){
		if(PIN_PBC_Check())
			enableWPS();
	}
	else{
		document.form.wps_pin.value = "00000000";
		enableWPS();
	}
}

function resetWPS(){
	FormActions("apply.cgi", "Reset_OOB", "", "");
	document.form.target = "";
	applyRule();
}

function resetTimer()
{
	if (stopFlag == 1)
	{
		stopFlag = 0;
		InitializeTimer();
	}
}

function ValidateChecksum(PIN)
{
	var accum = 0;

	accum += 3 * (parseInt(PIN / 10000000) % 10);
	accum += 1 * (parseInt(PIN / 1000000) % 10);
	accum += 3 * (parseInt(PIN / 100000) % 10);
	accum += 1 * (parseInt(PIN / 10000) % 10);
	accum += 3 * (parseInt(PIN / 1000) % 10);
	accum += 1 * (parseInt(PIN / 100) % 10);
	accum += 3 * (parseInt(PIN / 10) % 10);
	accum += 1 * (parseInt(PIN / 1) % 10);

	return ((accum % 10) == 0);
}

function PIN_PBC_Check(){
	if(document.form.wps_pin.value != ""){
		if(document.form.wps_pin.value.length != 8 || !ValidateChecksum(document.form.wps_pin.value)){
			alert("<#JS_InvalidPIN#>");
			document.form.wps_pin.focus();
			document.form.wps_pin.select();
			return false;
		}
	}	
	
	return true;
}

function InitializeTimer()
{
	if(document.form.wl_auth_mode[0].value == "shared" 
			|| document.form.wl_auth_mode[0].value == "wpa" 
			|| document.form.wl_auth_mode[0].value == "wpawpa2" 
			|| document.form.wl_auth_mode[0].value == "radius" 
			|| document.form.wl_auth_mode[1].value == "shared" 
			|| document.form.wl_auth_mode[1].value == "wpa" 
			|| document.form.wl_auth_mode[1].value == "wpawpa2" 
			|| document.form.wl_auth_mode[1].value == "radius")
		return;
	
	msecs = timeout;
	StopTheClock();
	StartTheTimer();
}

function StopTheClock()
{
	if(timerRunning)
		clearTimeout(timerID);
	timerRunning = false;
}

function StartTheTimer(){
	if(msecs == 0){
		StopTheClock();
		
		if(stopFlag == 1)
			return;
		
		updateWPS();
		msecs = timeout;
		StartTheTimer();
	}
	else{
		msecs = msecs-500;
		timerRunning = true;
		timerID = setTimeout("StartTheTimer();", delay);
	}
}

function updateWPS(){
	var ie = window.ActiveXObject;

	if (ie)
		makeRequest_ie('/WPS_info.asp');
	else
		makeRequest('/WPS_info.asp');
}

function loadXML(){
	updateWPS();
	InitializeTimer();
}

function refresh_wpsinfo(xmldoc){
	var wps0 = xmldoc.getElementsByTagName("wps0");
	if (band5g_support != -1)
		var wps1 = xmldoc.getElementsByTagName("wps1");
	else // single band: 5G info as 2.4G
		var wps1 = wps0;

	if(wps0 == null || wps0[0] == null 
			|| wps1 == null || wps1[0] == null){
		if (confirm("<#JS_badconnection#>"))
			;
		else
			stopFlag=1;
		return;
	}
	
	var wps0_infos = wps0[0].getElementsByTagName("wps_info");
	var wps1_infos = wps1[0].getElementsByTagName("wps_info");
	show_wsc_status(wps0_infos, wps1_infos);
}

function show_wsc_status(wps0_infos, wps1_infos){
	if (wps0_infos[3].firstChild.nodeValue == "shared"
			|| wps0_infos[3].firstChild.nodeValue == "wpa"
			|| wps0_infos[3].firstChild.nodeValue == "wpa2"
			|| wps0_infos[3].firstChild.nodeValue == "radius"
			|| wps0_infos[4].firstChild.nodeValue == "1"
			|| wps0_infos[5].firstChild.nodeValue == "0"
			|| wps1_infos[3].firstChild.nodeValue == "shared"
			|| wps1_infos[3].firstChild.nodeValue == "wpa"
			|| wps1_infos[3].firstChild.nodeValue == "wpa2"
			|| wps1_infos[3].firstChild.nodeValue == "radius"
			|| wps1_infos[4].firstChild.nodeValue == "1"
			|| wps1_infos[5].firstChild.nodeValue == "0") {
		if (wps0_infos[4].firstChild.nodeValue == "1" || wps1_infos[4].firstChild.nodeValue == "1")
			$("wps_enable_hint").innerHTML = "<#WPS_hint4#> <a href=\"Advanced_Wireless_Content.asp\"><#menu2_1#></a> <#WPS_hint1#>";
		else if (wps0_infos[5].firstChild.nodeValue == "0" || wps1_infos[5].firstChild.nodeValue == "0")
			$("wps_enable_hint").innerHTML = "<#WPS_hint3#> <a href=\"Advanced_WAdvanced_Content.asp\"><#menu2_5#></a> <#WPS_hint1#>";
		else
			$("wps_enable_hint").innerHTML = "<#WPS_hint2#> <a href=\"Advanced_Wireless_Content.asp\"><#menu2_1#></a> <#WPS_hint1#>";
		$("radio_wps_enable").style.display = "none";
		$("wps_state0_tr").style.display = "none";
		if (band5g_support != -1)
			$("wps_state1_tr").style.display = "none";
		$("wps_config_tr").style.display = "none";
		$("devicePIN_tr").style.display = "none";
		return;
	}

	// WPS status
	if(wps_enable_old == "0"){
		$("wps_state0_tr").style.display = "";
		if (band5g_support != -1)
			$("wps_state1_tr").style.display = "";
		$("wps_state0_td").innerHTML = "Not used";
		$("wps_state1_td").innerHTML = "Not used";
		$("WPSConnTble").style.display = "none";
		$("wpsDesc").style.display = "none";
	}
	else{
		$("wps_state0_tr").style.display = "";
		if (band5g_support != -1)
			$("wps_state1_tr").style.display = "";
		$("wps_state0_td").innerHTML = wps0_infos[0].firstChild.nodeValue;
		$("wps_state1_td").innerHTML = wps1_infos[0].firstChild.nodeValue;
		$("WPSConnTble").style.display = "";
		$("wpsDesc").style.display = "";
	}
	
	// device's PIN code
	$("devicePIN_tr").style.display = "";
	$("devicePIN").value = wps0_infos[2].firstChild.nodeValue;
	
	// the input of the client's PIN code
	$("wpsmethod_tr").style.display = "";
	if(wps_enable_old == "1"){
		inputCtrl(document.form.wps_pin, 1);
		if(wps0_infos[1].firstChild.nodeValue == "Yes" 
				|| wps1_infos[1].firstChild.nodeValue == "Yes")
			$("Reset_OOB").style.display = "";
		else
			$("Reset_OOB").style.display = "none";
	}
	else{
		inputCtrl(document.form.wps_pin, 0);
		$("Reset_OOB").style.display = "none";
	}
	
	if(wps0_infos[0].firstChild.nodeValue == "Start enrolling...")
		$("wps_pin_hint0").style.display = "inline";
	else
		$("wps_pin_hint0").style.display = "none";
	if(wps1_infos[0].firstChild.nodeValue == "Start enrolling...")
		$("wps_pin_hint1").style.display = "inline";
	else
		$("wps_pin_hint1").style.display = "none";

	if(wps0_infos[1].firstChild.nodeValue == "No" 
			|| wps1_infos[1].firstChild.nodeValue == "No")
		$("wps_config_td").innerHTML = "No";
	else
		$("wps_config_td").innerHTML = "Yes";
}

function changemethod(wpsmethod){
	if(wpsmethod == 0){
		document.form.wps_mode.value = "0"; // PBC
		$("starBtn").style.marginTop = "9px";
		$("wps_pin").style.display = "none";
	}
	else{
		document.form.wps_mode.value = "1"; // PIN
		$("starBtn").style.marginTop = "5px";
		$("wps_pin").style.display = "";
	}
}
function disableWPS_item(){  /* this function can hide WPS option */
	if(<% nvram_get("rep_wps_be_ap"); %> == 0 && sw_mode == 2){
		for(var i=5; i>=1; --i)
			$("WPS_item").deleteRow(i);
		return true;
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
<form method="POST" name="form" action="/start_apply.htm" target="hidden_frame">
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
<input type="hidden" name="current_page" value="Advanced_WWPS_Content.asp">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="action_wait" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="wps_enable" value="<% nvram_get("wps_enable"); %>">
<input type="hidden" name="wps_mode" value="<% nvram_get("wps_mode"); %>" disabled>
<input type="hidden" name="wl_auth_mode" value="<% nvram_get("wl0_auth_mode"); %>">
<input type="hidden" name="wl_auth_mode" value="<% nvram_get("wl1_auth_mode"); %>">

<table width="98%" border="0" align="left" cellpadding="0" cellspacing="0">
	<tr>
		<td valign="top" >
		
<table width="760px" border="0" cellpadding="4" cellspacing="0" class="FormTitle" id="FormTitle" height="343px">

	<tbody>
	<tr>
		  <td bgcolor="#4D595D" valign="top"  >
		  <div>&nbsp;</div>
		  <div class="formfonttitle"><#menu2#> - <#menu2_2#></div>
		  <div style="margin-left:5px;margin-top:10px;margin-bottom:10px"><img src="/images/New_ui/export/line_export.png"></div>
		  <div class="formfontdesc"><#WLANConfig11b_display6_sectiondesc#></div>

		<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0"  class="FormTable" id="WPS_item">
			<!-- For Repeater -->
			<tr id="WPS_Repeater_switch"display="">
			  	<th width="30%"><#WPS_PBC_itemname#></th>
				<td>
					<input type="radio" name="rep_wps_be_ap" class="input" value="1" <% nvram_match("rep_wps_be_ap", "1", "checked"); %>><#WPS_PBC_Registrar#>
					<input type="radio" name="rep_wps_be_ap" class="input" value="0" <% nvram_match("rep_wps_be_ap", "0", "checked"); %>><#WPS_PBC_Clinet#>
					<input type="button" id="applyButton" class="button_gen" value="<#CTL_apply#>" onclick="Repeater_switch();">
				</td>
			</tr>
			<!-- For Repeater -->
	
			<tr>
			  	<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(13,1);"><#WPS_itemname#></a></th>
			  	<td>
			    	<div class="left" style="width: 94px;" id="radio_wps_enable"></div>
						<script type="text/javascript">
							$j('#radio_wps_enable').iphoneSwitch('<% nvram_get("wps_enable"); %>', 
								 function() {
									document.form.wps_enable.value = "1";
									enableWPS(1);
								 },
								 function() {
									document.form.wps_enable.value = "0";
									enableWPS(0);
								 },
								 {
									switch_on_container_path: '/plugin/iphone_switch_container_off.png'
								 }
							);
						</script>
						<span id="wps_enable_hint"></span>
		  	  </td>
			</tr>

			<tr id="wps_state0_tr">
				<th>2.4GHz <#PPPConnection_x_WANLink_itemname#></th>
				<td width="300">
					<span id="wps_state0_td"></span>
					<img id="wps_pin_hint0" style="display:none;" src="images/InternetScan.gif" />
				</td>
			</tr>

			<tr id="wps_state1_tr" style="display:none;">
				<th>5GHz <#PPPConnection_x_WANLink_itemname#></th>
				<td width="300">
					<span id="wps_state1_td"></span>
					<img id="wps_pin_hint1" style="display:none;" src="images/InternetScan.gif" />
				</td>
			</tr>

			<tr id="wps_config_tr">
				<th>Configured</th>
				<td>
					<span class="devicepin" style="color:#FFF;" id="wps_config_td"></span>&nbsp;&nbsp;
					<input class="button_gen" type="button" onClick="resetWPS();" id="Reset_OOB" name="Reset_OOB" value="<#CTL_Reset_OOB#>" style="padding:0 0.3em 0 0.3em;" >
				</td>
			</tr>
			
			<tr id="devicePIN_tr">
			  <th>
			  	<span id="devicePIN_name"><a class="hintstyle" href="javascript:void(0);" onclick="openHint(13,4);"><#WLANConfig11b_x_DevicePIN_itemname#></a></span>			  
			  </th>
			  <td>
			  	<input type="text" name="devicePIN" id="devicePIN" value="" class="input_15_table" readonly="1" style="float:left;"></input>
			  </td>
			</tr>
		</table>

		<div class="formfontdesc" style="padding-bottom:10px;padding-top:10px;display:none;" id="wps_client_note">
			<p style="color:#FFCC00"><#FW_note#></p>
			<p style="color:#FFCC00"><#WPS_note_desc#></p>
		</div>

		<table id="WPSConnTble" width="100%" border="1" align="center" cellpadding="4" cellspacing="0"  class="FormTable" style="display:none;">	

			<div  class="formfontdesc" style="padding-bottom:10px;padding-top:10px;display:none;" id="wpsDesc">
				<#WPS_add_client#>
				<div style="color:#FFCC00"><#WPS_weptkip_hint#><div>
			</div>
			
			<tr id="wpsmethod_tr">
				<th>
			  	<span id="wps_method"><a class="hintstyle" href="javascript:void(0);" onclick="openHint(13,2);">WPS Method</a></span>
			  </th>
			  <td>
					<input type="radio" name="wps_method" onclick="changemethod(0);" value="0">Push Button
					<input type="radio" name="wps_method" onclick="changemethod(1);" value="1">Client PIN Code
			  	<input type="text" name="wps_pin" id="wps_pin" value="<% nvram_get("wps_pin"); %>" size="8" maxlength="8" class="input_15_table">
				  <div id="starBtn" style="margin-top:10px;"><input class="button_gen" type="button" style="margin-left:5px;" onClick="configCommand();" id="addEnrolleebtn_client" name="addEnrolleebtn"  value="<#WPS_start_btn#>"></div>
				</td>
			</tr>

		</table>
		
		<div></div>

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
		
    <td width="10" align="center" valign="top">&nbsp;</td>
	</tr>
</table>
<div id="footer"></div>
</body>
</html>
