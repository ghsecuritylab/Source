<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<html xmlns:v>
<head>
<meta http-equiv="X-UA-Compatible" content="IE=Edge"/>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title><#Web_Title#> - <#menu1#></title>
<link rel="stylesheet" type="text/css" href="qis/qis_style.css">
<link rel="stylesheet" type="text/css" href="index_style.css">
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="NM_style.css">
<link rel="stylesheet" type="text/css" href="other.css">
<link rel="stylesheet" type="text/css" href="confirm_block.css">
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/client_function.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/detect.js"></script>
<script language="JavaScript" type="text/javascript" src="/jquery.js"></script>
<script type="text/javascript" src="/plugin/jquery.iphone-switch.js"></script>
<script type="text/javascript" src="/confirm_block.js"></script>
<script>
<% wl_get_parameter(); %>

var $j = jQuery.noConflict();	
var gateway = '<% nvram_get("lan_gateway"); %>';
var link_status = '<%link_status();%>';//show LAN link spped, 1000 for 1Gbps, 100 for 100Mbps, 10 for 10Mbps
var arps = [<% get_arp_table(); %>];		// [[ip, x, x, MAC, x, type], ...]
var case_select = '<% get_parameter("flag"); %>';//for refresh client
var sw_mode = '<% nvram_get("sw_mode"); %>';
var wl_unit_var = '<% nvram_get("wl_unit"); %>';
var cli_enables='<%  nvram_get("en_networkmap"); %>';
var wl_ch_vitems = <% select_channel(); %>;
var re_expressway = '<% nvram_get("re_expressway"); %>';
var re_mediabridge = '<% nvram_get("re_mediabridge"); %>';
var webs_state_update = '<% nvram_get("webs_state_update"); %>';
var webs_state_error = '<% nvram_get("webs_state_error"); %>';
var webs_state_info = '<% nvram_get("webs_state_info"); %>';
var show_confirm_enable = 1;

function initial(){
	document.form.wl_ssid.value = decodeURIComponent(document.form.wl_ssid.value);
	document.form.wl_ssid_input.value = decodeURIComponent(document.form.wl_ssid_input.value);
	if (document.form.wl_ssid_input.value.length > 24)
		document.form.wl_ssid_input.value = document.form.wl_ssid_input.value.substring(0,24) + "...";

	show_menu();
	show_band_select_option();
	insertChannelOption();
	document.form.wl_channel.value = document.form.wl_channel_orig.value;
	get_static_clients();
	updatePapStatus();
	showKeyField();
	if(case_select =='refresh'){
		select_table(2);
	}
	else{
		select_table(1);
	}

	if (navigator.userAgent.search("MSIE 6") > -1)
		alert("<#JS_IE6_alert#>");
	need_action_script(document.form.action_script);
	update_fw_info();
}

/*update parent AP status every 5 seconds*/
function updatePapStatus(){
	$j.ajax({
		url: '/ajax_pap_status.asp',
		dataType: 'script', 
	error: function(xhr) {
		setTimeout("updatePapStatus();", 5000); //repeat calling
	},
	success: function(response) {	
		show_op_mode();
		show_link_status();
		setTimeout("updatePapStatus();", 5000); //repeat calling
	}
	});
}

var client_updating = 0;
function update_clients(){
	if (client_updating == 1)
		return;
	client_updating = 1;

	$j.ajax({
		url: '/ajax_refresh_clients.asp',
		dataType: 'script', 
	
		error: function(xhr) {
			client_updating = 0;
		},
		success: function(response) {
			get_static_clients();
		}
	});
}

var retry = 0;
function get_static_clients(){
	$('SearchingIcon').style.display = "";
	$j.ajax({
		url: '/ajax_update_clients.asp',
		dataType: 'script', 
	
		error: function(xhr) {
			setTimeout("get_static_clients();", 1000);
		},
		success: function(response) {	
			show_client_list(client_list_array);
			show_client_status(client_list_array);
			if (retry < 35) {
				setTimeout("get_static_clients();", 1000);
				retry++;
			} else {
				$('SearchingIcon').style.display = "none";
				retry = 0;
				client_updating = 0;
			}
		}
	});
}

function show_client_status(client_list_array){
	var client_str = "";
	if(sw_mode=='4'){
		$('client_status').innerHTML="<#NM_login_user#>";
		client_str += "<#NM_Full_Clients#>: <span id='_clientNumber'>1</span>";
	}
	else{
		$('client_status').innerHTML="<#NM_login_user1#>";
		var net_client_number='<%  nvram_get("en_networkmap"); %>';
		if(net_client_number==1)
	               client_str += "<#NM_Full_Clients#>: <span id='_clientNumber'>"+client_list_array.length+"</span>";
	        else
		       client_str += "<#NM_Full_Clients#>: <span id='_clientNumber'>OFF</span>";
	}
	$("clientNumber").innerHTML = client_str;
}

function show_op_mode(){
	var html_code = "";
	var radio_2g='<% nvram_get("sw0_radio"); %>';
	var radio_5g='<% nvram_get("sw1_radio"); %>';
	var decodeSSID = DecodeSpecificChar(pap_status[0], 1);

	if (sw_mode == "2") {
		if (re_expressway != 0)
			$("sw_mode_span").innerHTML = "<#OP_REew_item0#>";
		else if (re_mediabridge == 1)
			$("sw_mode_span").innerHTML = "<#OP_REmb_item#>";
		else
			$("sw_mode_span").innerHTML = "<#NM_opmode_RE#>";
		$j('#ea_ssid').html(decodeSSID);
		$j('#ap_macaddr').html(pap_status[4]);
		if (repeater_mode_v2_support != -1 || re_mediabridge == 1) {
			html_code = "<p style='width:200px;padding-left:5px; margin-top:3px; padding-bottom:3px; margin-right:10px; line-height:20px;' >"+document.form.wl_ssid.value+"</p>";
			$j('#wl_ssid_field').html(html_code);
			if (pap_status[5] == "0")
				document.getElementById('wl_ssid_tr').style.display = "";
			else
				document.getElementById('wl_ssid_tr').style.display = "none";
			document.getElementById('wl_key_tr').style.display = "none";
			document.getElementById('wl_bw_tr').style.display = "none";
			document.getElementById('wl_channel_tr').style.display = "none";
			document.getElementById('apply_gen').style.display = "none";
		} else {
			document.getElementById('extend_wireless_field').style.display = "";
			showBW();
		}

		showAuthMethod();
		showChannel();
		showFrequency();

		if ((pap_status[3] == "2.4" && document.form.wl_unit.value == "0") 
				|| (pap_status[3] == "5" && document.form.wl_unit.value == "1") 
				|| nonconcurrent_support != -1)
			html_code = "<p style='width:200px;padding-left:5px; margin-top:3px; padding-bottom:3px; margin-right:10px; line-height:20px;' >"+ pap_status[2] +"</p>";
		else
			html_code = "<p style='width:200px;padding-left:5px; margin-top:3px; padding-bottom:3px; margin-right:10px; line-height:20px;' >"+ pap_status[7] +"</p>";
		$j('#wl_channel_field').html(html_code);

		if (cli_enables == 1) {
			document.getElementById('commits').style.display="";
			document.getElementById('refresh_list').style.display="";
			document.getElementById('xclients').style.display="";
		} else {
			document.getElementById('commits').style.display="none";
			document.getElementById('xclients').style.display="none";
			document.getElementById('refresh_list').style.display="none";
		}
	} else if(sw_mode == "3") {
		document.getElementById('apply_gen').style.marginTop="10px"; // for AP only
		$("sw_mode_span").innerHTML = "<#NM_opmode_AP#>";
		document.getElementById('sta_ssid_tr').style.display="none";
		document.getElementById('sta_auth_tr').style.display="none";
		document.getElementById('sta_channel_tr').style.display="none";
		document.getElementById('sta_band_tr').style.display="none";
		document.getElementById('qis_hint').style.display="none";
		document.getElementById('sta_ip_tr').style.display="";
		document.getElementById('link_speed').style.display="";

		if (wl_unit_var == '0') {
			if (radio_2g == "0")
				document.getElementById('AP_radio_field').style.display="";
			remove_11ac_80MHz_option();
		} else {
			if (radio_5g == "0")
				document.getElementById('AP_radio_field').style.display="";
			if (band5g_11ac_support == -1)
				remove_11ac_80MHz_option();
		}
		$j('#lan_gateway').html(gateway);
		for (var i=0 ; i<arps.length ; i++) {
			if (gateway.match(arps[i][0]))
				$j('#ap_macaddr').html(arps[i][3]);
		}
		if (link_status == "1000")
			$("link_status").innerHTML = "1 Gbps";
		else if (link_status == "100")
			$("link_status").innerHTML = "100 Mbps";
		else if (link_status == "10")
			$("link_status").innerHTML = "10 Mbps";
		else
			$("link_status").innerHTML = "no link";

		if (cli_enables == 1) {
			document.getElementById('commits').style.display="";
			document.getElementById('refresh_list').style.display="";
			document.getElementById('xclients').style.display="";
		} else {
			document.getElementById('commits').style.display="none";
			document.getElementById('xclients').style.display="none";
			document.getElementById('refresh_list').style.display="none";
		}
	} else if(sw_mode == "4") {
		document.getElementById('apply_gen').style.display="none";
		$("sw_mode_span").innerHTML = "<#NM_opmode_EA#>";
		document.getElementById('wl_ssid_tr').style.display="none";
		document.getElementById('wl_key_tr').style.display="none";
		document.getElementById('sta_mac_tr').style.display="none";
		document.getElementById('refresh_list').style.display="none";//client status table refresh button
		$j('#ea_ssid').html(decodeSSID);
		$j('#ap_macaddr').html(pap_status[4]);

		document.getElementById('client_enable').style.display="none";

		showAuthMethod();
		showChannel();
		showFrequency();
		showBW();

		if (pap_status[2] == "0")
			html_code = "<p style='width:200px;padding-left:5px; margin-top:3px; padding-bottom:3px; margin-right:10px; line-height:20px;' >Auto</p>";
		else
			html_code = "<p style='width:200px;padding-left:5px; margin-top:3px; padding-bottom:3px; margin-right:10px; line-height:20px;' >"+ pap_status[2] +"</p>";
		$j('#wl_channel_field').html(html_code);
	}

	if (wl_unit_var == '0')
		html_code = "<% nvram_get("wl0_macaddr"); %>";
	else
		html_code = "<% nvram_get("wl1_macaddr"); %>";
	$j('#wl_macaddr').html(html_code);
}

function show_client_list(){
	var code="";
	var readme="";
	if (sw_mode == "4") {
		code += "<tr align='center' >\n"
		code += "<td style='width:120px;border:1px solid #304044; '><span>"+login_ip_str();+"</span></td>"
		code += "<td style='width:140px;border:1px solid #304044; '><span>"+login_mac_str();+"</span></td>"
		code += "<td style='width:200px;border:1px solid #304044; '><span></span></td>"
		code += "</tr>\n"
	} else {
		for (var i=0 ; i<client_list_array.length ; i++) {
			code += "<tr align='center' >\n"
			code += "<td style='width:120px;border:1px solid #304044; '><span>"+client_list_array[i][0]+"</span></td>"
			code += "<td style='width:140px;border:1px solid #304044; '><span>"+client_list_array[i][1]+"</span></td>"		
			if(client_list_array[i][2]==null)
				code += "<td style='width:200px;border:1px solid #304044; '><span></span></td>"
			else
				code += "<td style='width:200px;border:1px solid #304044; '><span>"+client_list_array[i][2]+"</span></td>"//Max length of Computer_Name is 17
			code += "</tr>\n"
		}
		var lens;  //lens=35*(client_list_array.length+1)+25;
		if(client_list_array.length<=1)
			lens=60;
	        else
	                lens=95+33*(client_list_array.length-2);
		
		if(lens >200)
		        lens=220;
		readme += "<tr align='left' >\n"
		readme += "<td style='position:absolute;color:#FFFF00;left:20px;width:470px;top:"+lens+"px;'><#NM_scan_alert#></td>";
		readme += "</tr>\n"
		$j("#commits tbody").html(readme);
	}
	$j("#ap_client tbody").html(code);
}
function select_table(i){
	if(i=='0'){
		document.getElementById("clients_status").style.display="none";
		document.getElementById("router_status").style.display="none";
		document.getElementById("network_status").style.display="block";
	}
	else if(i=='1'){ 
		document.getElementById("clients_status").style.display="none";
		document.getElementById("router_status").style.display="block";
		document.getElementById("network_status").style.display="none";
	}
	else{
		document.getElementById("clients_status").style.display="block";
		document.getElementById("router_status").style.display="none";
		document.getElementById("network_status").style.display="none";
	}
}
function change_style(id, s){
	if (id == "wl_ssid_input") {
		if (s == '0') {
			document.getElementById(id).style.backgroundColor="#6F8789";
			document.form.wl_ssid_input.value = document.form.wl_ssid.value;
		} else if (s == '1') {
			document.getElementById(id).style.backgroundColor="#58656B";
			document.form.wl_ssid.value = document.form.wl_ssid_input.value;
			if (document.form.wl_ssid_input.value.length > 24)
				document.form.wl_ssid_input.value = document.form.wl_ssid_input.value.substring(0,24) + "...";
		}
	}
}
function showKeyField(){
	if(document.form.wl_auth_mode.value=="shared"){
		switch_wep_key(document.form.wl_key.value);
	}
	else if(document.form.wl_auth_mode.value=="psk" || document.form.wl_auth_mode.value=="wpa" || document.form.wl_auth_mode.value=="wpa2"){
		var wpa_password = decodeURIComponent(document.form.wl_wpa_psk_x.value);
		if(wpa_password.length > 27)
			wpa_password = wpa_password.substring(0,27) + "...";
		document.getElementById('key_field').innerHTML = wpa_password;
	}
	else if(document.form.wl_auth_mode.value=="open" && document.form.wl_wep_x.value != "0"){
		switch_wep_key(document.form.wl_key.value);
	}
	else{// OPEN-NONE
		// do nothing
	}
}
/* check wep_key index and show this key */
function switch_wep_key(index){
	if(index == "1")
		document.getElementById('key_field').innerHTML = decodeURIComponent(document.form.wl_key1_org.value);
	else if(index == "2")
		document.getElementById('key_field').innerHTML = decodeURIComponent(document.form.wl_key2_org.value);
	else if(index == "3")
		document.getElementById('key_field').innerHTML = decodeURIComponent(document.form.wl_key3_org.value);
	else if(index == "4")
		document.getElementById('key_field').innerHTML = decodeURIComponent(document.form.wl_key4_org.value);
	else
		document.getElementById('key_field').innerHTML = decodeURIComponent(document.form.wl_key1_org.value);
}

function showChannel(){
	if (pap_status[2] == "0")
		$('sta_channel').innerHTML="Auto";
	else
		$('sta_channel').innerHTML=pap_status[2];
}

function showFrequency(){
	if (pap_status[3] == "5")
		$('sta_freq_td').innerHTML = "5 GHz";
	else if (pap_status[3] == "2.4")
		$('sta_freq_td').innerHTML = "2.4 GHz";
}

function showBW(){
	var HT_BW='<% nvram_get("wl_HT_BW"); %>';

	if (HT_BW == "0")
		$('wl_bw_td').innerHTML = "<p style='width:200px;padding-left:5px; margin-top:3px; padding-bottom:3px; margin-right:10px; line-height:20px;' >20 MHz</p>";
	else if (HT_BW == "1") {
		if (wl_unit_var == '1' && band5g_11ac_support != -1)
			$('wl_bw_td').innerHTML = "<p style='width:200px;padding-left:5px; margin-top:3px; padding-bottom:3px; margin-right:10px; line-height:20px;' >20/40/80 MHz</p>";
		else
			$('wl_bw_td').innerHTML = "<p style='width:200px;padding-left:5px; margin-top:3px; padding-bottom:3px; margin-right:10px; line-height:20px;' >20/40 MHz</p>";
	} else if (HT_BW == "2")
		$('wl_bw_td').innerHTML = "<p style='width:200px;padding-left:5px; margin-top:3px; padding-bottom:3px; margin-right:10px; line-height:20px;' >40 MHz</p>";
	else if (HT_BW == "3")
		$('wl_bw_td').innerHTML = "<p style='width:200px;padding-left:5px; margin-top:3px; padding-bottom:3px; margin-right:10px; line-height:20px;' >80 MHz</p>";
}

function showAuthMethod(){
	var html_code = "";
	if (pap_status[5] == "0") //if P-AP disconnect, HostAP status table doesn't show Authentication method
		;
	else if (pap_status[1] == "0") {
		html_code = "Open System";
		if (document.form.wl_wep_x.value == "1")
			html_code += " (WEP-64bits)";
		else if (document.form.wl_wep_x.value == "2")
			html_code += " (WEP-128bits)";
		else
			html_code += " (None)";
	}
	else if (pap_status[1] == "1") {
		html_code = "Shared Key";
		if (document.form.wl_wep_x.value == "1")
			html_code += " (WEP-64bits)";
		else if(document.form.wl_wep_x.value == "2")
			html_code += " (WEP-128bits)";
	}
	else if (pap_status[1] == "2")
		html_code = "WPA-Personal";
	else if (pap_status[1] == "3")
		html_code = "WPA2-Personal";
	else if (pap_status[1] == "4")
		html_code = "WPA-Enterprise";
	else if (pap_status[1] == "5")
		html_code = "WPA2-Enterprise";
	$('auth_method').innerHTML = html_code;
}
function applyrule(){
	need_action_wait(document.form.action_script, sw_mode);
	if(sw_mode == "3")
		document.form.wl_HT_BW.value = document.form.wl_bw.value;
	document.form.submit();
}

function enableCli(){
	FormActions("apply.cgi", "Cli_apply", "", "");
	document.form.target = "";
	document.form.submit();
}

function show_link_status(){
	// check PAP connection status
	if (sw_mode == "2" || sw_mode == "4") {
		if (pap_status[5] == "1") {
			$('pap_link_status').innerHTML="<#CTL_Connect#>";
			$('wireless_link_pap').style.display="";
			$('link_disconnect').style.display="none";
		} else {
			$('pap_link_status').innerHTML="<#CTL_Disconnect#>";
			$('wireless_link_pap').style.display="none";
			$('link_disconnect').style.display="";
		}
	} else if (sw_mode == "3") {
		if (gateway != '') {
			$('pap_link_status').innerHTML="<#CTL_Connect#>";
			$('wired_link_pap').style.display="";
			$('link_disconnect').style.display="none";
		} else {
			$('pap_link_status').innerHTML="<#CTL_Disconnect#>";
			$('wired_link_pap').style.display="none";
			$('link_disconnect').style.display="";
		}
	}
	// check wireless client
	if (isWLclient()) {
		$('wireless_link_client').style.display="";
		$('wired_link_client').style.display="none";
	} else {
		$('wireless_link_client').style.display="none";
		$('wired_link_client').style.display="";
	}
}

function gotoSurvey(){
	var sub_sw_mode;

	if (re_mediabridge == "1")
		sub_sw_mode = "3";
	else
		sub_sw_mode = re_expressway;
	location.href = "QIS_wizard.htm?flag=survey&sw_mode=<% nvram_get("sw_mode"); %>&sub_sw_mode="+sub_sw_mode;
}

var cnt = 0;
function update_fw_info(){
	$j.ajax({
		url: '/ajax_detect_firmware_status.asp',
		dataType: 'script',
		timeout: 3000,
		error: function(xhr) {
			setTimeout("update_fw_info();", 1000);
		},
		success: function(response) {
			if (++cnt > 120) {
				cnt = 0;
				show_confirm_enable = 1;
			}

			if (webs_state_error != 1 && webs_state_info.length > 0) {
				var current_firmver = '<% nvram_get("firmver"); %>';
				var tmp = current_firmver.split(".");
				var _firmver = tmp[0]+tmp[1]+tmp[2]+tmp[3];
				if (webs_state_info > _firmver) {
					var tmp = webs_state_info.split("");
					var latest_firmver = tmp[0]+'.'+tmp[1]+'.'+tmp[2]+'.'+tmp[3]+tmp[4];
					if (show_confirm_enable == 1)
						confirm_asus(current_firmver, latest_firmver);
					show_confirm_enable = 0;
				}
			}
			setTimeout("update_fw_info();", 5000);
		}
	});
}
</script>
</head>

<body onload="initial();" onunload="return unload_body();">
<noscript>
	<div class="popup_bg" style="visibility:visible; z-index:999;">
		<div style="margin:200px auto; width:300px; background-color:#006699; color:#FFFFFF; line-height:150%; border:3px solid #FFF; padding:5px;"><#not_support_script#></p></div>
	</div>
</noscript>

<div id="TopBanner"></div>

<div id="Loading" class="popup_bg"></div>
<div id="hiddenMask" class="popup_bg">
	<table cellpadding="5" cellspacing="0" id="dr_sweet_advise" class="dr_sweet_advise" align="center">
		<tr>
		<td>
			<div class="drword" id="drword"><#Main_alert_proceeding_desc4#> <#Main_alert_proceeding_desc1#>...
				<br>
				<br>
		    </div>
		  <div class="drImg"><img src="images/alertImg.gif"></div>
			<div style="height:70px; "></div>
		</td>
		</tr>
	</table>
<!--[if lte IE 6.5]><iframe class="hackiframe"></iframe><![endif]-->
</div>

<iframe name="hidden_frame" id="hidden_frame" width="0" height="0" frameborder="0" scrolling="no"></iframe>

<form method="post" name="form" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="prev_page" value="index.asp">
<input type="hidden" name="current_page" value="index.asp">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="action_mode" value=" Apply ">
<input type="hidden" name="action_script" value="restart_wifi">
<input type="hidden" name="action_wait" value="">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="sta_bssid" id="sta_bssid" value="<% nvram_get("sta_bssid"); %>">
<input type="hidden" name="wl_ssid" id="wl_ssid" value="<% nvram_char_to_ascii("wl_ssid"); %>">
<input type="hidden" name="wl_channel_org" id="wl_channel_orig" value="<% nvram_get("wl_channel"); %>">
<input type="hidden" name="wl_auth_mode" value="<% nvram_get("wl_auth_mode"); %>">
<input type="hidden" name="wl_wpa_mode" value="<% nvram_get("wl_wpa_mode"); %>">
<input type="hidden" name="wl_wpa_psk_x" value="<% nvram_char_to_ascii("wl_wpa_psk"); %>">
<input type="hidden" name="wl_wep_x" value='<% nvram_get("wl_wep_x"); %>'>
<input type="hidden" name="wl_key" value="<% nvram_get("wl_key"); %>">
<input type="hidden" name="wl_key1_org" value="<% nvram_char_to_ascii("wl_key1"); %>">
<input type="hidden" name="wl_key2_org" value="<% nvram_char_to_ascii("wl_key2"); %>">
<input type="hidden" name="wl_key3_org" value="<% nvram_char_to_ascii("wl_key3"); %>">
<input type="hidden" name="wl_key4_org" value="<% nvram_char_to_ascii("wl_key4"); %>">
<input type="hidden" name="sta_auth_mode" value='<% nvram_get("sta_auth_mode"); %>'>
<input type="hidden" name="sta_wpa_mode" value='<% nvram_get("sta_wpa_mode"); %>'>
<input type="hidden" name="sta_wpa_psk" value="<% nvram_get("sta_wpa_psk"); %>">
<input type="hidden" name="sta_wep_x" value='<% nvram_get("sta_wep_x"); %>'>
<input type="hidden" name="sta_key1_org" value="<% nvram_char_to_ascii("sta_key1"); %>">
<input type="hidden" name="sta_key2_org" value="<% nvram_char_to_ascii("sta_key2"); %>">
<input type="hidden" name="sta_key3_org" value="<% nvram_char_to_ascii("sta_key3"); %>">
<input type="hidden" name="sta_key4_org" value="<% nvram_char_to_ascii("sta_key4"); %>">
<input type="hidden" name="sta_encryption" id="sta_encryption" value="">
<input type="hidden" name="flag" value="">
<input type="hidden" name="sw_mode" value="<% nvram_get("sw_mode"); %>">
<input type="hidden" name="wl_HT_BW" id="wl_HT_BW" value="<% nvram_get("wl_HT_BW"); %>">
<input type="hidden" name="cli_enable" value="<% nvram_get("en_networkmap"); %>">

<table class="content" align="center" cellpadding="0" cellspacing="0" >
  <tr>
	<td valign="top" width="17">&nbsp;</td>
		
		<!--=====Beginning of Main Menu=====-->
		<td valign="top" width="204">
			<div id="mainMenu"></div>
			<div id="subMenu"></div>
		</td>	
		<td align="left" valign="top" class="">
		
		<!--=====Beginning of Network Map=====-->
			<div id="tabMenu"></div><br>
			<div id="NM_shift" style="margin-top:-186px;"></div>
			<div id="NM_table" class="NM_table">
			<div align="center">
			<table width="600px" height="136px" id="_NM_table" >	
			<tr height="111px" align="center">
				<td id="router_image"><img style="cursor:pointer;" src="/images/New_ui/networkmap/router-1.png" onclick="select_table(0);" onmouseover="this.src='images/New_ui/networkmap/router-2.png'" onmouseout="this.src='images/New_ui/networkmap/router-1.png'">			
				</td>	
				<td ><img id="map_dut" style="cursor:pointer;" src="/images/New_ui/networkmap/product-1.png" onclick="select_table(1);" onmouseover="this.src='images/New_ui/networkmap/product-2.png'" onmouseout="this.src='images/New_ui/networkmap/product-1.png'">			
				</td>
				<td ><img id="map_client" style="cursor:pointer;" src="/images/New_ui/networkmap/TV.png" onclick="select_table(2);" onmouseover="this.src='images/New_ui/networkmap/TV_2.png'" onmouseout="this.src='images/New_ui/networkmap/TV.png'">			
				</td>				
			</tr>
			<tr height="25px" style="color:rgb(118,193,195);font-size:16px;font-family:Verdana;width:240px;">
				<td style="width:226px;" align="center">
					<p id="pap_link_status"></p>
				</td>
				<td id="map_n66" style="width:183px;" align="center">
					<p id="sw_mode_span" ></p>
				</td>
				<td id="map_client" style="width:183px;" align="center">
					<p id="clientNumber"></p>
				</td>
			</tr>
				<img src="/images/New_ui/networkmap/plane.png" style="position:absolute; top:127px;left:15px;">
				<img id="wireless_link_pap" src="/images/New_ui/networkmap/wifi.png" style="position:absolute;top:80px;left:275px;display:none;">
				<img id="wired_link_pap" src="/images/New_ui/networkmap/link-m.png" style="position:absolute;top:91px;left:255px;display:none;">
				<img id="link_disconnect" src="/images/New_ui/networkmap/disconnect.png" style="position:absolute;top:85px;left:254px;display:none;">
				<img id="wireless_link_client" src="/images/New_ui/networkmap/wifi.png" style="position:absolute;top:80px;left:465px;display:none;">
				<img id="wired_link_client" src="/images/New_ui/networkmap/link-m.png" style="position:absolute;top:91px;left:442px;display:none;">
			</table>
			</div>
			<!--Internet status table display in Repeater and Adapter mode-->
			<div id="network_status" align="center" >
				<img src="/images/New_ui/networkmap/triangle-80.png" style="position:absolute;top:163px;*top:168px;left:170px">
				<div class="shadow" style="margin-top:25px;margin-bottom:20px;background:rgb(78,89,93);width:500px;height:400px;border-radius: 7px 7px 7px 7px;" id="ap_main_content" align="left">
					
					<div style="text-align:center;color:rgb(118,193,195);font-family:Arial;font-size:16px;padding-bottom:3px;padding-top:3px;"><#NM_pap_status#></div>
						<div>
						<table width="480px;"  border="1" align="center" cellpadding="2" cellspacing="0" style="bordercolor=#6b8fa3" class="FormTable">
							<tr align="center" >
								<th style="background-color:#151E21;text-align:center;font-weight:bolder;font-family:Verdana;border:1px solid #334044;"><#NM_item#></th>
								<th style="background-color:#151E21;text-align:center;font-weight:bolder; font-family:Verdana;border:1px solid #334044;"><#NM_info#></th>
							</tr>
							<tr id="sta_ssid_tr">
								<th style="width:48%;padding-left:10px;font-weight:bolder;font-family:Verdana;border:1px solid #334044;" ><#NM_Wireless_name#>(SSID)</th>
								<td style="border:1px solid #334044;">		
									<p style="width:200px;padding-left:5px; margin-top:3px; padding-bottom:3px; margin-right:10px; line-height:20px;" id="ea_ssid"></p>			
								</td>
							</tr>
							<tr id="sta_auth_tr">
								<th style="width:48%;padding-left:10px;font-weight:bolder;font-family:Verdana;border:1px solid #334044;" ><#QIS_Security#></th>
								<td style="border:1px solid #334044;">		
									<p style="width:200px;padding-left:5px; margin-top:3px; padding-bottom:3px; margin-right:10px; line-height:20px;" id="auth_method"></p>			
								</td>
							</tr>  
							<tr id="sta_channel_tr">
								<th style="width:48%;padding-left:10px;font-weight:bolder;font-family:Verdana;border:1px solid #334044;"><#WLANConfig11b_Channel_itemname#></th>
								<td style="border:1px solid #334044;">
									<p style="width:200px;padding-left:5px; margin-top:3px; padding-bottom:3px; margin-right:10px; line-height:20px;" id="sta_channel"></p>	
								</td>
							</tr>
							<tr id="sta_band_tr">
								<th style="width:48%;padding-left:10px;font-weight:bolder;font-family:Verdana;border:1px solid #334044;"><#NM_Current_band#></th>
								<td style="border:1px solid #334044;">
									<p style="width:200px;padding-left:5px; margin-top:3px; padding-bottom:3px; margin-right:10px; line-height:20px;" id="sta_freq_td"></p>	
								</td>
							</tr>
							<tr id="sta_ip_tr" style="display:none;">
							<th style="width:48%;padding-left:10px;font-weight:bolder;font-family:Verdana;border:1px solid #334044;"><#LANHostConfig_IPRouters_itemname#></th>
								<td style="border:1px solid #334044;">		
									<p style="width:200px;padding-left:5px; margin-top:3px; padding-bottom:3px; margin-right:10px; line-height:20px;" id="lan_gateway"></p>
		
								</td>
							</tr> 
							<tr id="sta_mac_tr">
							<th style="width:48%;padding-left:10px;font-weight:bolder;font-family:Verdana;border:1px solid #334044;"><#NM_MAC_Address#></th>
								<td style="border:1px solid #334044;">		
									<p style="width:200px;padding-left:5px; margin-top:3px; padding-bottom:3px; margin-right:10px; line-height:20px;" id="ap_macaddr"></p>
		
								</td>
							</tr> 	
							<tr id="link_speed" style="display:none;">
							<th style="width:48%;padding-left:10px;font-weight:bolder;font-family:Verdana;border:1px solid #334044;"><#LANHostConfig_RouterSpeed_itemdesc#></th>
								<td style="border:1px solid #334044;">		
									<p style="width:200px;padding-left:5px; margin-top:3px; padding-bottom:3px; margin-right:10px; line-height:20px;" id="link_status"></p>
		
								</td>
							</tr>	
						</table>
						</div>
						<div  style="margin-top:-5px;*margin-top:-10px;margin-left:-12px;" id="qis_hint">
						<ul>
						  <li><#NM_PAP_alert1#> <a class="nm_firmver" href="javascript:gotoSurvey();"><#QIS_title#></a> <#NM_alert_end_1#></li>
						</ul>
						</div>
				</div>
			</div>
			<!--status table-->
			<div id="router_status" align="center">
				<img src="/images/New_ui/networkmap/triangle-80.png" style="position:absolute;top:163px;*top:168px;left:365px">
				<div class="shadow" style="margin-top:25px;margin-bottom:20px;;background:rgb(78,89,93);width:500px;height:400px;border-radius: 7px 7px 7px 7px;" id="ap_main_content" align="left">
					<div style="text-align:center;color:rgb(118,193,195);font-family:Arial;font-size:16px;padding-bottom:3px;padding-top:3px;"><#NM_dut_status#></div>	
						<div>
						<table width="480px;"  border="1" align="center" cellpadding="2" cellspacing="0" style="bordercolor=#6b8fa3" class="FormTable">
							<tr align="center" >
								<th style="background-color:#151E21;text-align:center;font-weight:bolder;font-family:Verdana;border:1px solid #334044;"><#NM_item#></th>
								<th style="background-color:#151E21;text-align:center;font-weight:bolder; font-family:Verdana;border:1px solid #334044;"><#NM_info#></th>
							</tr>
							<tr id="wl_unit_tr" style="display:none;">
								<th style="width:48%;padding-left:10px;font-weight:bolder;font-family:Verdana;border:1px solid #334044;"><#QIS_frequency#></th>
								<td style="border:1px solid #334044;">
									<select name="wl_unit" class="input_option" onChange="change_wl_unit();">
										<option class="content_input_fd" value="0" <% nvram_match("wl_unit", "0","selected"); %>>2.4GHz</option>
										<option class="content_input_fd" value="1" <% nvram_match("wl_unit", "1","selected"); %>>5GHz</option>
									</select>			
								</td>
							</tr>
							<tr id="wl_ssid_tr">
								<th style="width:48%;padding-left:10px;font-weight:bolder;font-family:Verdana;border:1px solid #334044;" ><#NM_Wireless_name#>(SSID)</th>
								<td style="border:1px solid #334044;" id="wl_ssid_field">
									<input style="width:200px;" id="wl_ssid_input" type="text" name="wl_ssid_input" value="<% nvram_char_to_ascii("wl_ssid"); %>" maxlength="32" size="22" class="input_25_table" onfocus="change_style('wl_ssid_input', 0);" onblur="change_style('wl_ssid_input', 1);">
								</td>
							</tr>
							<tr id="wl_key_tr">
								<th style="width:48%;padding-left:10px;font-weight:bolder;font-family:Verdana;border:1px solid #334044;" ><#QIS_Security_Key#></th>
								<td style="border:1px solid #334044;">
									<p id="key_field"></p>
								</td>
							</tr>			
							<tr id="wl_bw_tr">
								<th style="width:48%;padding-left:10px;font-weight:bolder;font-family:Verdana;border:1px solid #334044;"><#WLANConfig11b_ChannelBW_itemname#></th>
								<td style="border:1px solid #334044;" id="wl_bw_td">
									<select name="wl_bw" class="input_option" onChange="insertChannelOption();">
										<option class="content_input_fd" value="1" <% nvram_match("wl_HT_BW", "1","selected"); %>>20/40/80 MHz</option>
										<option class="content_input_fd" value="0" <% nvram_match("wl_HT_BW", "0","selected"); %>>20 MHz</option>
										<option class="content_input_fd" value="2" <% nvram_match("wl_HT_BW", "2","selected"); %>>40 MHz</option>
										<option class="content_input_fd" value="3" <% nvram_match("wl_HT_BW", "3","selected"); %>>80 MHz</option>
									</select>				
								</td>
							</tr>	
							<tr id="wl_channel_tr">
								<th style="width:48%;padding-left:10px;font-weight:bolder;font-family:Verdana;border:1px solid #334044;"><#WLANConfig11b_Channel_itemname#></th>
								<td style="border:1px solid #334044;" id="wl_channel_field">
									<select name="wl_channel" class="input_option"></select>
								</td>
							</tr>
							<tr>
							<th style="width:48%;padding-left:10px;font-weight:bolder;font-family:Verdana;border:1px solid #334044;"><#LANHostConfig_IPRouters_itemname#></th>
								<td style="border:1px solid #334044;">		
									<p style="width:200px;padding-left:5px; margin-top:3px; padding-bottom:3px; margin-right:10px; line-height:20px;"><% nvram_get("lan_ipaddr"); %></p>
								</td>
							</tr> 					

							<tr>
							<th style="width:48%;padding-left:10px;font-weight:bolder;font-family:Verdana;border:1px solid #334044;"><#NM_MAC_Address#></th>
								<td style="border:1px solid #334044;">		
									<p style="width:200px;padding-left:5px; margin-top:3px; padding-bottom:3px; margin-right:10px; line-height:20px;" id="wl_macaddr"></p>
								</td>
							</tr> 					
							<tr id="firmver_t">
							<th style="width:48%;padding-left:10px;font-weight:bolder;font-family:Verdana;border:1px solid #334044;"><#FW_item2#>:</th>
								<td style="border:1px solid #334044;">	
									<a href="Advanced_FirmwareUpgrade_Content.asp" class="nm_firmver" style="width:200px;padding-left:5px; margin-top:3px;padding-bottom:3px; margin-right:10px; line-height:20px;" id="firmver"><% nvram_get("firmver"); %></a>
								</td>
							</tr>
						</table>
						</div>
						<div id="extend_wireless_field" style="margin-top:-5px;margin-left:-12px;display:none;">
						<ul><li>
							 <#NM_entend_alert1#> <a href="Advanced_ExtendWireless_Content.asp" class="nm_firmver"><#menu2_6#></a> <#NM_alert_end_1#>
						</li></ul>
						</div>
						<div id="AP_radio_field" style="color:#FFCC00;margin-top:-5px;margin-left:-12px;display:none;">
						<ul><li>
							<#NM_wireless_radio_alert1#> <a href="Advanced_WAdvanced_Content.asp" class="nm_firmver"><#menu2_5#></a> <#NM_alert_end2#>
						</li></ul>
						</div>
						<div id="apply_gen" class="apply_gen" style="margin-top:-5px;*margin-top:-15px;">
							<input id="applyButton" class="button_gen" type="button"  onclick="applyrule();" value="<#CTL_apply#>">
						</div>											
				</div>
			</div >
			<!--clients table-->
			<div id="clients_status" align="center" >
				<img src="/images/New_ui/networkmap/triangle-80.png" style="position:absolute;top:163px;*top:168px;left:560px">
				<div class="shadow" style="margin-top:25px;margin-bottom:20px;background:rgb(78,89,93);width:500px;height:400px;border-radius: 7px 7px 7px 7px;" id="ap_main_content" >	
				<div id="client_status" style="text-align:center;color:rgb(118,193,195);font-family:Arial;font-size:16px;padding-bottom:3px;padding-top:3px;"></div>	

				<div id="client_enable" align="left" width="90%" >
				<table  cellpadding="0" cellspacing="0" class="FormTable" id="client_list_item"  style="margin-left:12px;width:461px;border-color:rgb(78,89,93);">
					<tbody>
					<tr>       
						<th style="width:241px; border-color:rgb(78,89,93);"><#NM_scan#></th>
							<td   style="width:220px;border-color:rgb(78,89,93);">
			    					<div id="cli_enable_block" style="display:none;">
			    						<span style="color:#FFF;" id="cli_enable_word">disable</span>
			    						<input type="button" name="enableClibtn" id="enableClibtn" value="use" class="button_gen" onclick="enableCli();">
			    						<br>
			    					</div>
						
			    					<div class="left" style="width: 94px; cursor: pointer;" id="networkmap_enable"></div>
								<div class="clear"></div>					
								<script type="text/javascript">
								$j('#networkmap_enable').iphoneSwitch('<% nvram_get("en_networkmap"); %>', 
								    function() {
								
										document.form.cli_enable.value = "1";
										enableCli();
									 },
									 function() {
										
										document.form.cli_enable.value = "0";
										enableCli();
									 },
									 {
										switch_on_container_path: '/plugin/iphone_switch_container_off.png'
									 }
								);
								</script>
								<span id="cli_enable_hint"></span>
		  	  				</td>
					</tr>		
					</tbody>	
              			</table>
				</div>
				<div style="position:absolute;z-index:5;margin-top:5px;margin-left:-5px;">
					<table id="commits" >
						<tbody></tbody>
					</table> 
				</div>

					<div style="visibility: none;width:460px;margin-left:-15px;" id="xclients" >
						<table class="QIS_survey"  >
							<tr align='center' >
								<th style='width:120px;border:1px solid #304044;background-color:#151E21;'><#NM_LAN_IP#></th>
								<th style='width:143px;border:1px solid #304044;background-color:#151E21;'><#NM_MAC_Address#></th>
								<th style='width:200px;border:1px solid #304044;background-color:#151E21;'><#NM_Computer_Name#></th>
							</tr>
						</table>
						<div align="left" style="width:478px;height:190px;overflow-y:auto;">
							<table class="QIS_survey" id="ap_client" style="width:460px;" align="left">
								<tbody ></tbody>
							</table>
						</div>
					</div>
					 <div style="position:relative;margin-top:83px;margin-left:15px;" id="refresh_list">
						<input type="button" class="button_gen" onclick="update_clients();" value="<#CTL_refresh#>" >
						<img id="SearchingIcon" style="display:none;" src="/images/InternetScan.gif">
					</div>
				</div>
			</div >	
<!--==============Ending of hint content=============-->
	</td>
  </tr>
</table>
</form>
<div id="footer"></div>
</body>
</html>
