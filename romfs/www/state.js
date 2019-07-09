//For operation mode;
var sw_mode = '<% nvram_get("sw_mode"); %>';
var productid = '<% nvram_get("fac_model_name"); %>';
var cur_dict = '<% nvram_get("preferred_lang"); %>';

var uptimeStr = "<% uptime(); %>";
var timezone = uptimeStr.substring(26,31);
var boottime = parseInt(uptimeStr.substring(32,42));
var newformat_systime = uptimeStr.substring(8,11) + " " + uptimeStr.substring(5,7) + " " + uptimeStr.substring(17,25) + " " + uptimeStr.substring(12,16);  //Ex format: Jun 23 10:33:31 2008
var systime_millsec = Date.parse(newformat_systime); // millsec from system
var JS_timeObj = new Date(); // 1970.1.1
var wan_route_x = "";
var wan_nat_x = "";
var wan_proto = "";
var test_page = 0;
var testEventID = "";
var httpd_dir = "/cifs1"

// parsing www_support
var www_support = '<% nvram_get("www_support"); %>';
var band5g_support = www_support.search("5G");
var band5g_11ac_support = www_support.search("11AC");
var router_mode_support = www_support.search("RT");
var repeater_mode_v2_support = www_support.search("REv2");
var repeater_mode_expressway_support = www_support.search("REew");
var repeater_mode_mediabridge_support = www_support.search("REmb");
var ap_mode_support = www_support.search("AP");
var ea_mode_support = www_support.search("EA");
var hotspot_mode_support = www_support.search("HS");
var chk_pap_pw_support = www_support.search("CPW");
var touch_support = www_support.search("TOUCH");
var internet_radio_support = www_support.search("IR");
var vol_max_support = www_support.search("VOLMAX");
var equalizer_support = www_support.search("EQ");
var qis_admin_support = www_support.search("QISAD");
var qis_opmode_support = www_support.search("QISOP");
var qis_finish_redirect_website_support = www_support.search("WEBSITE");
var nonconcurrent_support = www_support.search("NONCC");
var wifi_proxy_support = www_support.search("WPXY");
var led_ctl_support = www_support.search("LEDCTL");
var live_update_support = www_support.search("LU");

// for detect if the status of the machine is changed. {
var wanstate = -1;
var wansbstate = -1;
var wanauxstate = -1;
var stopFlag = 0;

//var gn_array_2g = <% wl_get_guestnetwork("0"); %>;
//var gn_array_5g = <% wl_get_guestnetwork("1"); %>;

// hide navibar on iOS
/*window.addEventListener("load", function(){ 
	setTimeout(scrollTo, 0, 0, 1);
}, false);*/

if ('<% nvram_get("http_passwd"); %>' == 'admin' && window.location.pathname.toUpperCase().search("QIS_") < 0 && window.location.pathname.length > 1)
	location.href = 'Main_Password.asp?nextPage='+window.location.pathname.substring(1, window.location.pathname.length);

function unload_body(){
}

function enableCheckChangedStatus(){
}

function disableCheckChangedStatus(){
	stopFlag = 1;
}

function check_if_support_dr_surf(){
}

function check_changed_status(){
}

function get_changed_status(){
}

function set_changed_status(){
}

function set_Dr_work(){
}

function clearHintTimeout(){
}

function showHelpofDrSurf(){
}

function showDrSurf(){
}

function slowHide(){
}

function hideHint(){
}

function drdiagnose(){
}

function show_banner(){
	var banner_code = "";	

	banner_code +='<form method="post" name="titleForm" id="titleForm" action="/start_apply.htm" target="hidden_frame">\n';
	banner_code +='<input type="hidden" name="next_page" value="">\n';
	banner_code +='<input type="hidden" name="current_page" value="">\n';
	banner_code +='<input type="hidden" name="action_mode" value=" Apply ">\n';
	banner_code +='<input type="hidden" name="action_script" value="">\n';
	banner_code +='<input type="hidden" name="action_wait" value="5">\n';
	banner_code +='<input type="hidden" name="wl_unit" value="" disabled>\n';
	banner_code +='<input type="hidden" name="preferred_lang" value="">\n';
	banner_code +='<input type="hidden" name="flag" value="">\n';
	banner_code +='</form>\n';

	banner_code +='<form method="post" name="diskForm_title" action="/device-map/safely_remove_disk.asp" target="hidden_frame">\n';
	banner_code +='<input type="hidden" name="disk" value="">\n';
	banner_code +='</form>\n';

	banner_code +='<form method="post" name="internetForm_title" action="/start_apply.htm" target="hidden_frame">\n';
	banner_code +='<input type="hidden" name="current_page" value="">\n';
	banner_code +='<input type="hidden" name="next_page" value="">\n';
	banner_code +='<input type="hidden" name="action_mode" value="apply">\n';
	banner_code +='<input type="hidden" name="action_script" value="restart_wan">\n';
	banner_code +='<input type="hidden" name="action_wait" value="5">\n';
	banner_code +='<input type="hidden" name="wan0_enable" value="<% nvram_get("wan0_enable"); %>">\n';
	banner_code +='<input type="hidden" name="wan_unit" value="<% nvram_get("wan_unit"); %>">\n';
	banner_code +='</form>\n';

	banner_code +='<div class="banner1" align="center"><img src="images/New_ui/asustitle.png" width="218" height="54" align="left">\n';
	banner_code +='<div style="margin-top:13px;margin-left:-90px;*margin-top:0px;*margin-left:0px;" align="center"><span id="modelName_top" onclick="this.focus();" class="modelName_top"><#model_name#></span></div>';

	// logout, reboot
	banner_code +='<a href="javascript:logout();"><div style="margin-top:13px;margin-left:25px;*width:136px;" class="titlebtn" align="center"><span><#logout_title#></span></div></a>\n';
	banner_code +='<a href="javascript:reboot();"><div style="margin-top:13px;margin-left:0px;*width:136px;" class="titlebtn" align="center"><span><#CTL_Reboot#></span></div></a>\n';

	// language
	banner_code +='<ul class="navigation"><a href="#">';
	banner_code +='<% shown_language_option(); %>';
	banner_code +='</a></ul>';

	banner_code +='</div>\n';
	banner_code +='<table width="998" border="0" align="center" cellpadding="0" cellspacing="0" class="statusBar">\n';
	banner_code +='<tr>\n';
	banner_code +='<td background="images/New_ui/midup_bg.png" height="179" valign="top"><table width="764" border="0" cellpadding="0" cellspacing="0" height="12px" style="margin-left:230px;">\n';
	banner_code +='<tbody><tr>\n';
 	banner_code +='<td valign="center" class="titledown" width="auto">';
	banner_code +='</td>\n';

	banner_code +='<td width="17"></td>\n';
	banner_code +='</tr></tbody></table></td></tr></table>\n';

	$("TopBanner").innerHTML = banner_code;
	show_loading_obj();
	
	//show_top_status();
	set_Dr_work();
	//updateStatus_AJAX(); // tmp remove for testing Vanic
}

//Level 3 Tab
var tabtitle = new Array();
tabtitle[0] = new Array("", "<#menu2_1#>", "<#menu2_2#>", "<#menu2_3_t#>", "<#menu2_4#>", "<#menu2_5#>", "<#menu2_6_t#>", "<#menu2_7#>");
tabtitle[1] = new Array("", "<#menu3_1#>", "<#menu3_2#>", "<#menu3_3#>");
tabtitle[2] = new Array("", "<#menu7_1#>", "<#menu7_2#>", "<#menu7_3#>", "<#menu7_4#>", "<#menu7_5#>");
tabtitle[3] = new Array("", "<#menu2_1#>", "<#menu8_2#>", "<#menu8_3#>", "<#menu8_4#>");
tabtitle[4] = new Array("", "<#menu4_1#>", "<#menu4_2#>", "<#menu4_3_t#>", "<#menu4_4_t#>", "<#menu4_5_t#>");
tabtitle[5] = new Array("", "<#menu5_1#>", "<#menu5_2#>", "<#menu5_3#>", "<#menu5_4#>", "<#menu5_5#>");
tabtitle[6] = new Array("", "");
//tabtitle[6] = new Array("", "<#menu6_1#>");
var tablink = new Array();
tablink[0] = new Array("", "Advanced_Wireless_Content.asp", "Advanced_WWPS_Content.asp", "Advanced_ACL_Content.asp", "Advanced_WSecurity_Content.asp", "Advanced_WAdvanced_Content.asp", "Advanced_ExtendWireless_Content.asp", "Advanced_WiFi_Proxy.asp");
tablink[1] = new Array("", "Advanced_LAN_Content.asp", "Advanced_DHCP_Content.asp", "Advanced_GWStaticRoute_Content.asp");
tablink[2] = new Array("", "Advanced_WAN_Content.asp", "Advanced_PortTrigger_Content.asp", "Advanced_VirtualServer_Content.asp", "Advanced_Exposed_Content.asp", "Advanced_ASUSDDNS_Content.asp");
tablink[3] = new Array("", "Advanced_BasicFirewall_Content.asp", "Advanced_URLFilter_Content.asp", "Advanced_KeywordFilter_Content.asp", "Advanced_Firewall_Content.asp");
tablink[4] = new Array("", "Advanced_OperationMode_Content.asp", "Advanced_System_Content.asp", "Advanced_FirmwareUpgrade_Content.asp", "Advanced_SettingBackup_Content.asp", "Advanced_TouchSetting.asp");
tablink[5] = new Array("", "Main_LogStatus_Content.asp", "Main_DHCPStatus_Content.asp", "Main_WStatus_Content.asp", "Main_IPTStatus_Content.asp", "Main_RouteStatus_Content.asp");
tablink[6] = new Array("", "Advanced_Audio_Player.asp");

//Level 2 Menu
menuL2_title = new Array("", "<#menu2#>", "<#menu3#>", "<#menu7#>", "<#menu8#>", "<#menu4#>", "<#menu5#>", "<#menu6#>");
menuL2_link  = new Array();

//Level 1 Menu
menuL1_title = new Array("", "<#menu1#>");
menuL1_link = new Array("", "index.asp");

function remove_url(){
	if (router_mode_support == -1 && ap_mode_support == -1 && ea_mode_support == -1 && hotspot_mode_support == -1)
		remove_menu_item(4, "Advanced_OperationMode_Content.asp");
	if (touch_support == -1)
		remove_menu_item(4, "Advanced_TouchSetting.asp");
	if (internet_radio_support == -1) {
		menuL2_link[7]="";
		menuL2_title[7]="";
	}

	if (sw_mode == "1" || sw_mode == "6") {
		remove_menu_item(0, "Advanced_ExtendWireless_Content.asp");
		remove_menu_item(0, "Advanced_WiFi_Proxy.asp");
	} else if (sw_mode == "2") {
		if (repeater_mode_v2_support == -1 && '<% nvram_get("re_mediabridge"); %>' == 0) {
			remove_menu_item(0, "Advanced_Wireless_Content.asp");
			remove_menu_item(0, "Advanced_WWPS_Content.asp");
			remove_menu_item(0, "Advanced_WSecurity_Content.asp");
			if (wifi_proxy_support == -1)
				remove_menu_item(0, "Advanced_WiFi_Proxy.asp");
		} else {
			menuL2_link[1]="";
			menuL2_title[1]="";
			remove_menu_item(5, "Main_WStatus_Content.asp");
		}
		remove_menu_item(1, "Advanced_GWStaticRoute_Content.asp");
		menuL2_link[3]="";
		menuL2_title[3]="";
		menuL2_link[4]="";
		menuL2_title[4]="";
		remove_menu_item(5, "Main_DHCPStatus_Content.asp");
		remove_menu_item(5, "Main_IPTStatus_Content.asp");
		remove_menu_item(5, "Main_RouteStatus_Content.asp");
	} else if (sw_mode == "3") {
		remove_menu_item(0, "Advanced_ExtendWireless_Content.asp");
		remove_menu_item(0, "Advanced_WiFi_Proxy.asp");
		remove_menu_item(1, "Advanced_GWStaticRoute_Content.asp");
		menuL2_link[3]="";
		menuL2_title[3]="";
		menuL2_link[4]="";
		menuL2_title[4]="";
		remove_menu_item(5, "Main_DHCPStatus_Content.asp");
		remove_menu_item(5, "Main_IPTStatus_Content.asp");
		remove_menu_item(5, "Main_RouteStatus_Content.asp");
	} else if (sw_mode == "4") {
		menuL2_link[1]="";
		menuL2_title[1]="";
		remove_menu_item(1, "Advanced_GWStaticRoute_Content.asp");
		menuL2_link[3]="";
		menuL2_title[3]="";
		menuL2_link[4]="";
		menuL2_title[4]="";
		remove_menu_item(5, "Main_DHCPStatus_Content.asp");
		remove_menu_item(5, "Main_WStatus_Content.asp");
		remove_menu_item(5, "Main_IPTStatus_Content.asp");
		remove_menu_item(5, "Main_RouteStatus_Content.asp");
		menuL2_link[7]="";
		menuL2_title[7]="";
	} else if (sw_mode == "5") {
		remove_menu_item(0, "Advanced_WWPS_Content.asp");
		remove_menu_item(0, "Advanced_ExtendWireless_Content.asp");
		remove_menu_item(0, "Advanced_WiFi_Proxy.asp");
		remove_menu_item(2, "Advanced_ASUSDDNS_Content.asp");
	}
	menuL2_link = ["", tablink[0][1], tablink[1][1], tablink[2][1], tablink[3][1], tablink[4][1], tablink[5][1], tablink[6][1]];
}

function remove_menu_item(L2, remove_url){
	var dx;
	for(var i = 0; i < tablink[L2].length; i++){
		dx = tablink[L2].getIndexByValue(remove_url);
		tabtitle[L2].splice(dx, 1);
		tablink[L2].splice(dx, 1);
		break;
		
	}
}

Array.prototype.getIndexByValue= function(value){
	var index = -1;
	
	for (var i = 0; i < this.length; i++){
		if (this[i] == value){
			index = i;
			break;
		}
	}
	return index;
}

var current_url = location.pathname.substring(location.pathname.lastIndexOf('/') + 1);
function show_menu(){
	var L1 = 0, L2 = 0, L3 = 0;
	if(current_url == "") current_url = "index.asp";

	remove_url();

	for (var i = 1; i < menuL1_link.length; i++) {
		if (current_url == menuL1_link[i]) {
			L1 = i;
			break;
		}
	}
	if (L1 == 0) {
		for (var j = 0; j < tablink.length; j++) {
			for (var k = 1; k < tablink[j].length; k++) {
				if (current_url == tablink[j][k]) {
					L2 = j+1;
					L3 = k;
					break;
				}
			}
		}
	}

	show_banner();
	show_footer();
	browser_compatibility();
	show_selected_language();

	// main menu
	var main_code = "";
	main_code += '<div class="m_qis_n" style="margin-top:-170px;cursor:pointer;" onclick="go_setting(\'QIS_wizard.htm\');"><table><tr><td><img border="0" width="50px" height="50px" src="images/New_ui/setupwizard_icon.png"/></td><td><div><#QIS_title#></div></td></tr></table></div>\n';
	main_code += '<div class="m0_n" style="margin-top:10px;"><#menu2_1#></div>\n';
	for (var i = 1; i <= menuL1_title.length-1; i++) {
		if (L1 == i)
			main_code += '<div class="m1_f"><table><tr><td><img border="0" width="50px" height="50px" src="images/New_ui/icon_index_'+i+'.png"/></td><td><div>'+menuL1_title[i]+'</div></td></tr></table></div>\n';
		else
			main_code += '<div class="m1_n" onclick="location.href=\''+menuL1_link[i]+'\'" style="cursor:pointer;"><table><tr><td><img border="0" width="50px" height="50px" src="images/New_ui/icon_index_'+i+'.png"/></td><td><div>'+menuL1_title[i]+'</div></td></tr></table></div>\n';
	}
	$("mainMenu").innerHTML = main_code;

	// sub menu
	var sub_code = "";
	var last_submenu = 0;
	var idx = 0;
	for (var i = 1; i <= menuL2_title.length-1; ++i) {
		if (menuL2_title[i] != "")
			last_submenu++;
	}
	for (var i = 1; i <= menuL2_title.length-1; ++i) {
		if (menuL2_title[i] == "")
			continue;
		else {
			idx++;
			if (idx == last_submenu) {
				if (L2 == i)
					sub_code += '<div class="m2_f"><table><tr><td><img border="0" width="50px" height="50px" src="images/New_ui/icon_menu_'+i+'.png"/></td><td><div>'+menuL2_title[i]+'</div></td></tr></table></div>\n';
				else
					sub_code += '<div class="m2_n" onclick="location.href=\''+menuL2_link[i]+'\'" style="cursor:pointer;"><table><tr><td><img border="0" width="50px" height="50px" src="images/New_ui/icon_menu_'+i+'.png"/></td><td><div>'+menuL2_title[i]+'</div></td></tr></table></div>\n';
			} else {
				if (L2 == i)
					sub_code += '<div class="m1_f"><table><tr><td><img border="0" width="50px" height="50px" src="images/New_ui/icon_menu_'+i+'.png"/></td><td><div>'+menuL2_title[i]+'</div></td></tr></table></div>\n';
				else
					sub_code += '<div class="m1_n" onclick="location.href=\''+menuL2_link[i]+'\'" style="cursor:pointer;"><table><tr><td><img border="0" width="50px" height="50px" src="images/New_ui/icon_menu_'+i+'.png"/></td><td><div>'+menuL2_title[i]+'</div></td></tr></table></div>\n';
			}
		}
	}
	$("subMenu").innerHTML = sub_code;

	// tab menu
	var tab_code = "";
	if (L3 != 0) {
		tab_code = '<table border="0" cellspacing="0" cellpadding="0"><tr>\n';
		for (var i = 1; i < tabtitle[L2-1].length; ++i) {
			if (tabtitle[L2-1][i] == "")
				continue;
			else if (L3 == i)
				tab_code += '<td><div class="tabclick"><span><table><tbody><tr><td>'+tabtitle[L2-1][i]+'</td></tr></tbody></table></span></div></td>';
			else
				tab_code += '<td><div class="tab" onclick="location.href=\''+tablink[L2-1][i]+'\'" style="cursor:pointer;"><span><table><tbody><tr><td>'+tabtitle[L2-1][i]+'</td></tr></tbody></table></span></div></td>';
		}
		tab_code += '</tr></table>\n';
		$("tabMenu").innerHTML = tab_code;
	}
}

function Block_chars(obj, keywordArray){
	// bolck ascii code 32~126 first
	var invalid_char = "";
	for (var i = 0; i < obj.value.length; ++i) {
		if (obj.value.charCodeAt(i) < '32' || obj.value.charCodeAt(i) > '126')
			invalid_char += obj.value.charAt(i);
	}
	if (invalid_char != "") {
		alert('<#JS_validstr2#>" '+ invalid_char +'" !');
		obj.focus();
		return false;
	}

	// check if char in the specified array
	if (obj.value) {
		for (var i=0; i<keywordArray.length; i++) {
			if (obj.value.indexOf(keywordArray[i]) >= 0) {
				alert(keywordArray+ " are invalid characters.");
				obj.focus();
				return false;
			}
		}
	}

	return true;
}

function show_footer(){
	var footer_code = "";

	footer_code = '<div align="center" class="bottom-image"></div>\n';
	footer_code += '<div style="margin-top:-72px;margin-left:220px;"><table width="740" border="0" align="center" cellpadding="0" cellspacing="0"><tr>';
	footer_code += '<td width="20" align="right"><div id="bottom_help_icon" style="margin-right:3px;"></div></td><td width="100" id="bottom_help_title" align="left"><#footer_help_title#></td>';
	footer_code += '<td width="290" id="bottom_help_link" align="left">&nbsp&nbsp<a style="font-weight: bolder;text-decoration:underline;cursor:pointer;" href="<#JS_SupportLink#>" target="_blank"><#footer_help_link0#></a> | <a style="font-weight: bolder;text-decoration:underline;cursor:pointer;" href="<#JS_SupportLink#>" target="_blank"><#footer_help_link1#></a> | <a style="font-weight: bolder;text-decoration:underline;cursor:pointer;" onClick="sendmail();" target="_blank"><#footer_help_link2#></a></td>';
	footer_code += '<td width="300" id="bottom_help_FAQ" align="right" style="font-family:Arial, Helvetica, sans-serif;">FAQ&nbsp&nbsp<input type="text" id="FAQ_input" name="FAQ_input" class="input_FAQ_table" maxlength="40"></td>';
	footer_code += '<td width="30" align="left"><div id="bottom_help_FAQ_icon" class="bottom_help_FAQ_icon" style="cursor:pointer;margin-left:3px;" target="_blank" onClick="supportsite_search();"></div></td>';
	footer_code += '</tr></table></div>\n';

	footer_code +='<div align="center" style="margin-top:7px;padding-right:30px" class="copyright"><#footer_copyright_desc#></div>\n';

	$("footer").innerHTML = footer_code;
	flash_button();
}

function browser_type(){
	var type = "";
	if (navigator.userAgent.search("MSIE 9") > -1)
		type = 'IE9';
	else if (navigator.userAgent.search("MSIE 8") > -1)
		type = 'IE8';
	else if (navigator.userAgent.search("MSIE 7") > -1)
		type = 'IE7';
	else if (navigator.userAgent.search("MSIE") > -1)
		type = 'IE';
	else if (navigator.userAgent.search("Firefox") > -1)
		type = 'Firefox';
	else if (navigator.userAgent.search("Chrome") > -1)
		type = 'Chrome';
	else if (navigator.userAgent.search("Safari") > -1)
		type = 'Safari';
	else if (navigator.userAgent.search("Opera") > -1)
		type = 'Opera';
	else if (navigator.userAgent.search("iP") > -1)
		type = 'iOS';
	return type;
}

function OS_type(){
	var type = "";
	if (navigator.userAgent.search("Windows NT 6.2") > -1)
		type = 'Windows 8';
	else if (navigator.userAgent.search("Windows NT 6.1") > -1)
		type = 'Windows 7';
	else if (navigator.userAgent.search("Windows NT 6.0") > -1)
		type = 'Windows Vista';
	else if (navigator.userAgent.search("Windows NT 5.1") > -1)
		type = 'Windows XP';
	else if (navigator.userAgent.search("Windows") > -1)
		type = 'Windows';
	else if (navigator.userAgent.match(/(iPad|iPhone|iPod)/g))
		type = "iOS";
	else if (navigator.userAgent.search("Mac") > -1)
		type = 'Mac OS';
	else if (navigator.userAgent.search("Android") > -1)
		type = 'Android';
	else if (navigator.userAgent.search("X11") > -1)
		type = 'UNIX';
	else if (navigator.userAgent.search("Linux") > -1)
		type = 'Linux';
	return type;
}

function sendmail(){
	$j.ajax({
		url: '/ajax_get_syslog.asp',
		dataType: 'script',
		error: function(xhr){
			setTimeout("sendmail();", 1000);
		},
		success: function(){
			var mailto_content = "";
			var i = 0;

			if (mailto_syslog.length > 11)
				i = mailto_syslog.length - 11;

			mailto_content += 'mailto:asusrouters@gmail.com?subject=ASUS <#model_name#> Feedback&body=';
			mailto_content += 'Please enter your question/suggestion and append your system log: %0a'
			mailto_content += '%0a';	// new line
			mailto_content += '%0a';
			mailto_content += 'Firmware version: <% nvram_get("firmver"); %>%0a';
			mailto_content += 'Browser: '+browser_type()+'%0a';
			mailto_content += 'O/S: '+OS_type()+'%0a';
			mailto_content += 'Syslog: %0a';
			for(; i<mailto_syslog.length; i++){
				mailto_content += "          ";
				mailto_content += mailto_syslog[i];
				mailto_content += '%0a';
			}

			location.href = mailto_content;
		}
	});
}

function supportsite_search(){
	var lang = {
		EN : "en",
		TW : "tw",
		CN : "cn",
		CZ : "en",
		PL : "pl",
		RU : "ru",
		DE : "de",
		FR : "fr",
		TR : "en",
		TH : "en",
		MS : "my",
		JP : "en"
	}
	var keywordArray = $("FAQ_input").value.split(" ");
	var href;
	href = "http://www.asus.com/"+lang.<% nvram_get("preferred_lang"); %>+"/support/Knowledge-searchV2/?keyword=";
	for (var i=0; i<keywordArray.length; i++) {
		href += keywordArray[i];
		href += "%20";
	}
	window.open(href);
}

function browser_compatibility(){
	var isFirefox = navigator.userAgent.search("Firefox") > -1;
	var isOpera = navigator.userAgent.search("Opera") > -1;
	var isIE8 = navigator.userAgent.search("MSIE 8") > -1; 
	var isiOS = navigator.userAgent.search("iP") > -1; 
	var obj_inputBtn;
	
	if((isFirefox || isOpera) && document.getElementById("FormTitle")){
	//   if(current_url == tablink[4][1])
	//	document.getElementById("FormTitle").className = "FormTitle_audio_firefox";   
	//   else   
		document.getElementById("FormTitle").className = "FormTitle_firefox"; 
	}
	if((isFirefox || isOpera) && document.getElementById("drag_FormTitle")){
		document.getElementById("drag_FormTitle").className = "FormTitle"; 
	}

	if(isiOS){
		obj_inputBtn = document.getElementsByClassName("button_gen");
		for(var i=0; i<obj_inputBtn.length; i++){
			obj_inputBtn[i].addEventListener('touchstart', function(){this.className = 'button_gen_touch';}, false);
			obj_inputBtn[i].addEventListener('touchend', function(){this.className = 'button_gen';}, false);
		}
		obj_inputBtn = document.getElementsByClassName("button_gen_long");
		for(var i=0; i<obj_inputBtn.length; i++){
			obj_inputBtn[i].addEventListener('touchstart', function(){this.className = 'button_gen_long_touch';}, false);
			obj_inputBtn[i].addEventListener('touchend', function(){this.className = 'button_gen_long';}, false);
		}
	}
}	

function show_top_status(){
	var ssid_status_2g =  decodeURIComponent('<% nvram_char_to_ascii("wl0_ssid"); %>');
	var ssid_status_5g =  decodeURIComponent('<% nvram_char_to_ascii("wl1_ssid"); %>');
	
	if(band5g_support == -1){
		ssid_status_5g = "";
		if(ssid_status_2g.length > 23){
			ssid_status_2g = ssid_status_2g.substring(0,20) + "...";
			$("elliptic_ssid_2g").onmouseover = function(){
				return overlib('<p style="font-weight:bold;">2.4G SSID:</p>'+decodeURIComponent('<% nvram_char_to_ascii("wl0_ssid"); %>'));
			};
			$("elliptic_ssid_2g").onmouseout = function(){
				nd();
			};
		}

	}
	else{
		if(ssid_status_2g.length > 12){
			ssid_status_2g = ssid_status_2g.substring(0,10) + "...";
			$("elliptic_ssid_2g").onmouseover = function(){
				return overlib('<p style="font-weight:bold;">2.4G SSID:</p>'+decodeURIComponent('<% nvram_char_to_ascii("wl0_ssid"); %>'));
			};
			$("elliptic_ssid_2g").onmouseout = function(){
				nd();
			};
		}
	
		if(ssid_status_5g.length > 12){
			ssid_status_5g = ssid_status_5g.substring(0,10) + "...";
			$("elliptic_ssid_5g").onmouseover = function(){
				return overlib('<p style="font-weight:bold;">5G SSID:</p>'+decodeURIComponent('<% nvram_char_to_ascii("wl1_ssid"); %>'));
			};
			$("elliptic_ssid_5g").onmouseout = function(){
				nd();
			};
		}
	}
	//$("elliptic_ssid_2g").innerHTML = ssid_status_2g;
	//$("elliptic_ssid_5g").innerHTML = ssid_status_5g;	

	//showtext($("firmver"), document.form.firmver.value + "." + <% nvram_get("buildno"); %>);
	showtext($("firmver"), document.form.firmver.value);
	
	if(sw_mode == "1")  // Show operation mode in banner, Lock add at 2009/02/19
		$("sw_mode_span").innerHTML = "Router";
	else if(sw_mode == "2")
		$("sw_mode_span").innerHTML = "Repeater";
	else if(sw_mode == "3")	
		$("sw_mode_span").innerHTML = "AP";
	else if(sw_mode == "4")	
		$("sw_mode_span").innerHTML = "Adapter";
}

function go_setting(page){
		location.href = page;
}

function go_setting_parent(page){
		parent.location.href = page;
}

function show_time(){	
	JS_timeObj.setTime(systime_millsec); // Add millsec to it.	
	JS_timeObj3 = JS_timeObj.toString();	
	JS_timeObj3 = checkTime(JS_timeObj.getHours()) + ":" +
				  			checkTime(JS_timeObj.getMinutes()) + ":" +
				  			checkTime(JS_timeObj.getSeconds());
	$('systemtime').innerHTML ="<a href='/Advanced_System_Content.asp'>" + JS_timeObj3 + "</a>";
	systime_millsec += 1000;		
	
	stime_ID = setTimeout("show_time();", 1000);
}

function checkTime(i)
{
if (i<10) 
  {i="0" + i}
  return i
}

function show_loading_obj(){
	var obj = $("Loading");
	var code = "";
	
	code +='<table cellpadding="5" cellspacing="0" id="loadingBlock" class="loadingBlock" align="center">\n';
	code +='<tr>\n';
	code +='<td width="20%" height="80" align="center"><img src="/images/loading.gif"></td>\n';
	code +='<td><span id="proceeding_main_txt"><#Main_alert_proceeding_desc4#></span><span id="proceeding_txt" style="color:#FFFFCC;"></span></td>\n';
	code +='</tr>\n';
	code +='</table>\n';
	code +='<!--[if lte IE 6.5]><iframe class="hackiframe"></iframe><![endif]-->\n';
	
	obj.innerHTML = code;
}

var nav;

if(navigator.appName == 'Netscape')
	nav = true;
else{
	nav = false;
	document.onkeydown = MicrosoftEventHandler_KeyDown;
}

function MicrosoftEventHandler_KeyDown(){
	return true;
}

function show_selected_language(){
	switch('<% nvram_get("preferred_lang"); %>'){
		case 'EN':{
			$('selected_lang').innerHTML = "English";
			break;
		}
		case 'CN':{
			$('selected_lang').innerHTML = "简体中文";
			break;
		}	
		case 'TW':{
			$('selected_lang').innerHTML = "繁體中文";
			break;
		}
		case 'CZ':{
			$('selected_lang').innerHTML = "Česky";
			break;
		}
		case 'PL':{
			$('selected_lang').innerHTML = "Polski";
			break;
		}
		case 'RU':{
			$('selected_lang').innerHTML = "Pусский";
			break;
		}	
		case 'DE':{
			$('selected_lang').innerHTML = "Deutsch";
			break;
		}
		case 'FR':{
			$('selected_lang').innerHTML = "Français";
			break;
		}
		case 'TR':{
			$('selected_lang').innerHTML = "Türkçe";
			break;
		}
		case 'TH':{
			$('selected_lang').innerHTML = "ไทย";
			break;
		}	
		case 'MS':{
			$('selected_lang').innerHTML = "Malay";
			break;
		}
		case 'NO':{
			$('selected_lang').innerHTML = "Norsk";
			break;
		}
		case 'FI':{
			$('selected_lang').innerHTML = "Suomi";
			break;
		}
		case 'DA':{
			$('selected_lang').innerHTML = "Dansk";
			break;
		}	
		case 'SV':{
			$('selected_lang').innerHTML = "Svensk";
			break;
		}
		case 'BR':{
			$('selected_lang').innerHTML = "Brazil";
			break;
		}
		case 'JP':{
			$('selected_lang').innerHTML = "日本語";
			break;
		}
		case 'ES':{
			$('selected_lang').innerHTML = "Español";
			break;
		}
	}
}

function submit_language(obj){
	//if($("select_lang").value != $("preferred_lang").value){
	if(obj.id != $("preferred_lang").value){
		showLoading();
		
		with(document.titleForm){
			action = "/start_apply.htm";
			
			if(location.pathname == "/")
				current_page.value = "/index.asp";
			else
				current_page.value = location.pathname;
			
			preferred_lang.value = obj.id;
			//preferred_lang.value = $("select_lang").value;
			flag.value = "set_language";
			
			submit();
		}
	}
	else
		alert("No change LANGUAGE!");
}

function change_language(){
	if($("select_lang").value != $("preferred_lang").value)
		$("change_lang_btn").disabled = false;
	else
		$("change_lang_btn").disabled = true;
}

function logout(){
	if(confirm("<#JS_logout#>")){
		setTimeout('location = "Logout.asp";', 1);
	}
}

function reboot(){
	if (confirm("<#Main_content_Login_Item7#>")) {
		if (document.form.current_page.value.indexOf("Advanced_FirmwareUpgrade_Content")>=0 
				|| document.form.current_page.value.indexOf("Advanced_SettingBackup_Content")>=0) {
			if (document.form.encoding)
				document.form.encoding = "application/x-www-form-urlencoded";
			else
				document.form.enctype = "application/x-www-form-urlencoded";
		}
		FormActions("apply.cgi", "Restart", "", "");
		document.form.submit();
	}
}

function kb_to_gb(kilobytes){
	if(typeof(kilobytes) == "string" && kilobytes.length == 0)
		return 0;
	
	return (kilobytes*1024)/(1024*1024*1024);
}

function simpleNum(num){
	if(typeof(num) == "string" && num.length == 0)
		return 0;
	
	return parseInt(kb_to_gb(num)*1000)/1000;
}

function simpleNum2(num){
	if(typeof(num) == "string" && num.length == 0)
		return 0;
	
	return parseInt(num*1000)/1000;
}

function simpleNum3(num){
	if(typeof(num) == "string" && num.length == 0)
		return 0;
	
	return parseInt(num)/1024;
}

function $(){
	var elements = new Array();
	
	for(var i = 0; i < arguments.length; ++i){
		var element = arguments[i];
		if(typeof element == 'string')
			element = document.getElementById(element);
		
		if(arguments.length == 1)
			return element;
		
		elements.push(element);
	}
	
	return elements;
}

function getElementsByName_iefix(tag, name){
	var tagObjs = document.getElementsByTagName(tag);
	var objsName;
	var targetObjs = new Array();
	var targetObjs_length;
	
	if(!(typeof(name) == "string" && name.length > 0))
		return [];
	
	for(var i = 0, targetObjs_length = 0; i < tagObjs.length; ++i){
		objsName = tagObjs[i].getAttribute("name");
		
		if(objsName && objsName.indexOf(name) == 0){
			targetObjs[targetObjs_length] = tagObjs[i];
			++targetObjs_length;
		}
	}
	
	return targetObjs;
}

function getElementsByClassName_iefix(tag, name){
	var tagObjs = document.getElementsByTagName(tag);
	var objsName;
	var targetObjs = new Array();
	var targetObjs_length;
	
	if(!(typeof(name) == "string" && name.length > 0))
		return [];
	
	for(var i = 0, targetObjs_length = 0; i < tagObjs.length; ++i){
		if(navigator.appName == 'Netscape')
			objsName = tagObjs[i].getAttribute("class");
		else
			objsName = tagObjs[i].getAttribute("className");
		
		if(objsName == name){
			targetObjs[targetObjs_length] = tagObjs[i];
			++targetObjs_length;
		}
	}
	
	return targetObjs;
}

function showtext(obj, str){
	if(obj)
		obj.innerHTML = str;//*/
}

function showhtmlspace(ori_str){
	var str = "", head, tail_num;
	
	head = ori_str;
	while((tail_num = head.indexOf(" ")) >= 0){
		str += head.substring(0, tail_num);
		str += "&nbsp;";
		
		head = head.substr(tail_num+1, head.length-(tail_num+1));
	}
	str += head;
	
	return str;
}

function showhtmland(ori_str){
	var str = "", head, tail_num;
	
	head = ori_str;
	while((tail_num = head.indexOf("&")) >= 0){
		str += head.substring(0, tail_num);
		str += "&amp;";
		
		head = head.substr(tail_num+1, head.length-(tail_num+1));
	}
	str += head;
	
	return str;
}

// A dummy function which just returns its argument. This was needed for localization purpose
function translate(str){
	return str;
}

function trim(val){
	val = val+'';
	for (var startIndex=0;startIndex<val.length && val.substring(startIndex,startIndex+1) == ' ';startIndex++);
	for (var endIndex=val.length-1; endIndex>startIndex && val.substring(endIndex,endIndex+1) == ' ';endIndex--);
	return val.substring(startIndex,endIndex+1);
}

function IEKey(){
	return event.keyCode;
}

function NSKey(){
	return 0;
}

function is_string(o){
	if(!nav)
		keyPressed = IEKey();
	else
		keyPressed = NSKey();
	
	if(keyPressed == 0)
		return true;
	else if(keyPressed >= 0 && keyPressed <= 126)
		return true;
	
	alert("<#JS_validchar#>");
	return false;
}

function validate_string(o, flag){
	if (o.value.charAt(0) == '"') {
		if (flag != "noalert") {
			alert("<#JS_validstr1#> [\"]");
			o.value = "";
			o.focus();
			return false;
		} else 
			return "<#JS_validstr1#> [\"]";
	} else {
		invalid_char = "";
		for (var i = 0; i < o.value.length; ++i) {
			if (o.value.charAt(i) < ' ' || o.value.charAt(i) > '~')
				invalid_char = invalid_char+o.value.charAt(i);
		}
		if (invalid_char != "") {
			if (flag != "noalert") {
				alert("<#JS_validstr2#> '"+invalid_char+"' !");
				o.value = "";
				o.focus();
				return false;
			} else
				return "<#JS_validstr2#> '"+invalid_char+"' !";
		}
	}

	if (flag != "noalert")
		return true;
	else
		return "";
}

function validate_hex(obj){
	var obj_value = obj.value
	var re = new RegExp("[^a-fA-F0-9]+","gi");
	
	if(re.test(obj_value))
		return false;
	else
		return true;
}

function validate_psk(psk_obj){
	var psk_length = psk_obj.value.length;
	
	if(psk_length < 8){
		alert("<#JS_passzero#>");
		psk_obj.value = "00000000";
		psk_obj.focus();
		psk_obj.select();
		
		return false;
	}
	
	if(psk_length > 64){
		alert("<#JS_PSK64Hex#>");
		psk_obj.value = psk_obj.value.substring(0, 64);
		psk_obj.focus();
		psk_obj.select();
		
		return false;
	}
	
	if(psk_length >= 8 && psk_length <= 63 && !validate_string(psk_obj)){
		alert("<#JS_PSK64Hex#>");
		psk_obj.value = "00000000";
		psk_obj.focus();
		psk_obj.select();
		
		return false;
	}
	
	if(psk_length == 64 && !validate_hex(psk_obj)){
		alert("<#JS_PSK64Hex#>");
		psk_obj.value = "00000000";
		psk_obj.focus();
		psk_obj.select();
		
		return false;
	}
	
	return true;
}

function re_validate_psk(obj){
	var len = obj.value.length;

	if (len < 8 || len > 64
			|| (len <= 63 && !validate_string(obj))
			|| (len == 64 && !validate_hex(obj))) {
		alert("<#JS_PSK64Hex#>");
		obj.focus();
		obj.select();
		return false;
	}
	return true;
}

function validate_wep_key(wep_x, key, key_type){
	var ret = true;
	var len = key.value.length;
	var str = "<#JS_wepkey#>";

	if (wep_x.value == "0")
		;
	else if (wep_x.value == "1") {
		if (len == 5 && validate_string(key))
			key_type.value = 1;
		else if (len == 10 && validate_hex(key))
			key_type.value = 0;
		else {
			str += "(<#WLANConfig11b_WEPKey_itemtype1#>)";
			ret = false;
		}
	} else if (wep_x.value == "2") {
		if (len == 13 && validate_string(key))
			key_type.value = 1;
		else if (len == 26 && validate_hex(key))
			key_type.value = 0;
		else {
			str += "(<#WLANConfig11b_WEPKey_itemtype2#>)";
			ret = false;
		}
	}

	if (ret == false) {
		alert(str);
		key.focus();
		key.select();
	}

	return ret;
}

function validate_LoginUsername(o){
	var re = new RegExp("^[a-zA-Z0-9][a-zA-Z0-9\-\_]+$","gi");
	if (o.value.length == 0)
		return "<#QIS_pass_alert_desc4#>";
	if (re.test(o.value) == false)
		return "<#QIS_pass_alert_desc5#>";
	if (o.value == "root" || o.value == "guest" || o.value == "anonymous")
		return "<#QIS_pass_alert_desc8#>";
	return "";
}

function validate_LoginPasswd(o1, o2){
	var alert_str = "";
	if (o1.value == "" || o2.value == "")
		return "<#QIS_pass_alert_desc1#>";
	if (o1.value.length > 0 && o1.value.length < 5)
		return "<#JS_short_password#>";
	if (o1.value != o2.value)
		return "<#QIS_pass_alert_desc2#>";
	if (o1.value == "admin")
		return "<#QIS_pass_alert_desc7#>";
	return validate_string(o1, "noalert");
}

function checkDuplicateName(newname, targetArray){
	var existing_string = targetArray.join(',');
	existing_string = ","+existing_string+",";
	var newstr = ","+trim(newname)+",";
	
	var re = new RegExp(newstr, "gi");
	var matchArray = existing_string.match(re);
	
	if(matchArray != null)
		return true;
	else
		return false;
}

function alert_error_msg(error_msg){
	alert(error_msg);
	refreshpage();
}

function refreshpage(seconds){
	if(typeof(seconds) == "number")
		setTimeout("refreshpage()", seconds*1000);
	else
		location.href = location.href;
}

function hideLinkTag(){
	if(document.all){
		var tagObjs = document.all.tags("a");
		
		for(var i = 0; i < tagObjs.length; ++i)
			tagObjs(i).outerHTML = tagObjs(i).outerHTML.replace(">"," hidefocus=true>");
	}
}

function buttonOver(o){	//Lockchou 1206 modified
	o.style.color = "#FFFFFF";
	o.style.background = "url(/images/bgaibutton.gif) #ACCCE1";
	o.style.cursor = "hand";
}

function buttonOut(o){	//Lockchou 1206 modified
	o.style.color = "#000000";
	o.style.background = "url(/images/bgaibutton0.gif) #ACCCE1";
}

function flash_button(){
	if(navigator.appName.indexOf("Microsoft") < 0)
		return;
	
	var btnObj = getElementsByClassName_iefix("input", "button");
	
	for(var i = 0; i < btnObj.length; ++i){
		btnObj[i].onmouseover = function(){
				buttonOver(this);
			};
		
		btnObj[i].onmouseout = function(){
				buttonOut(this);
			};
	}
}

function no_flash_button(){
	if(navigator.appName.indexOf("Microsoft") < 0)
		return;
	
	var btnObj = getElementsByClassName_iefix("input", "button");
	
	for(var i = 0; i < btnObj.length; ++i){
		btnObj[i].onmouseover = "";
		
		btnObj[i].onmouseout = "";
	}
}

function gotoprev(formObj){
	var prev_page = formObj.prev_page.value;
	
	if(prev_page == "/")
		prev_page = "/index.asp";
	
	if(prev_page.indexOf('QIS') < 0){
		formObj.action = prev_page;
		formObj.target = "_parent";
		formObj.submit();
	}
	else{
		formObj.action = prev_page;
		formObj.target = "";
		formObj.submit();
	}
}

function add_option(selectObj, str, value, selected){
	var tail = selectObj.options.length;
	
	if(typeof(str) != "undefined")
		selectObj.options[tail] = new Option(str);
	else
		selectObj.options[tail] = new Option();
	
	if(typeof(value) != "undefined")
		selectObj.options[tail].value = value;
	else
		selectObj.options[tail].value = "";
	
	if(selected == 1)
		selectObj.options[tail].selected = selected;
}

function free_options(selectObj){
	if(selectObj == null)
		return;
	
	for(var i = selectObj.options.length-1; i >= 0; --i){
  		selectObj.options[i].value = null;
		selectObj.options[i] = null;
	}
}

function blocking(obj_id, show){
	var state = show?'block':'none';
	
	if(document.getElementById)
		$(obj_id).style.display = state;
	else if(document.layers)
		document.layers[obj_id].display = state;
	else if(document.all)
		document.all[obj_id].style.display = state;
}

function inputCtrl(obj, flag){
	if(flag == 0){
		obj.disabled = true;
		if(obj.type != "select-one")
			obj.style.backgroundColor = "#CCCCCC";
		if(obj.type == "radio" || obj.type == "checkbox")
			obj.style.backgroundColor = "#475A5F";
		if(obj.type == "text" || obj.type == "password")
			obj.style.backgroundImage = "url(/images/New_ui/inputbg_disable.png)";
	}
	else{
		obj.disabled = false;
		if(obj.type != "select-one")	
			obj.style.backgroundColor = "#FFF";
		if(obj.type == "radio" || obj.type == "checkbox")
			obj.style.backgroundColor = "#475A5F";
		if(obj.type == "text" || obj.type == "password")
			obj.style.backgroundImage = "url(/images/New_ui/inputbg.png)";
	}

	if(current_url.indexOf("Advanced_Wireless_Content") == 0
	|| current_url.indexOf("Advanced_ExtendWireless_Content") == 0
	|| current_url.indexOf("Advanced_WAN_Content") == 0
	|| current_url.indexOf("Guest_network") == 0
	|| current_url.indexOf("Advanced_PerformanceTuning_Content") == 0
	|| current_url.indexOf("Advanced_Modem_Content") == 0
	|| current_url.indexOf("Advanced_IPv6_Content") == 0
	|| current_url.indexOf("Advanced_WAdvanced_Content") == 0
	|| current_url.indexOf("Advanced_IPTV_Content") == 0
	|| current_url.indexOf("Advanced_ASUSDDNS_Content.asp") == 0)
	{
		if(obj.type == "checkbox")
			return true;
		if(flag == 0)
			obj.parentNode.parentNode.style.display = "none";
		else
			obj.parentNode.parentNode.style.display = "";
		return true;
	}
}

function inputHideCtrl(obj, flag){
	if(obj.type == "checkbox")
		return true;
	if(flag == 0)
		obj.parentNode.parentNode.style.display = "none";
	else
		obj.parentNode.parentNode.style.display = "";
	return true;
}

//Update current status via AJAX
var http_request_status = false;

function makeRequest_status(url) {
	http_request_status = new XMLHttpRequest();
	if (http_request_status && http_request_status.overrideMimeType)
		http_request_status.overrideMimeType('text/xml');
	else
		return false;

	http_request_status.onreadystatechange = alertContents_status;
	http_request_status.open('GET', url, true);
	http_request_status.send(null);
}

var xmlDoc_ie;

function makeRequest_status_ie(file)
{
	xmlDoc_ie = new ActiveXObject("Microsoft.XMLDOM");
	xmlDoc_ie.async = false;
	if (xmlDoc_ie.readyState==4)
	{
		xmlDoc_ie.load(file);
		setTimeout("refresh_info_status(xmlDoc_ie);", 1000);
	}
}

function alertContents_status()
{
	if (http_request_status != null && http_request_status.readyState != null && http_request_status.readyState == 4)
	{
		if (http_request_status.status != null && http_request_status.status == 200)
		{
			var xmldoc_mz = http_request_status.responseXML;
			refresh_info_status(xmldoc_mz);
		}
	}
}

function updateStatus_AJAX()
{
	//alert("AJAX XMLHttp Request sent!");
	var ie = window.ActiveXObject;

	if (ie)
		makeRequest_status_ie('/ajax_status.asp');
	else
		makeRequest_status('/ajax_status.asp');
}

function updateUSBStatus(){
	if(current_url == "index.asp" || current_url == "")
		detectUSBStatusIndex();
	else
		detectUSBStatus();
}

function detectUSBStatus(){
	$j.ajax({
    		url: '/detect_firmware.asp',
    		dataType: 'script',
    		error: function(xhr){
    			detectUSBStatus();
    		},
    		success: function(){
					return true;
  			}
  });
}

var link_status;
var link_auxstatus;
var link_sbstatus;
var usb_path1;
var usb_path2;
var usb_path1_tmp = "init";
var usb_path2_tmp = "init";

function refresh_info_status(xmldoc)
{
	var devicemapXML = xmldoc.getElementsByTagName("devicemap");
	var wanStatus = devicemapXML[0].getElementsByTagName("wan");

	link_status = wanStatus[0].firstChild.nodeValue;
	link_auxstatus = wanStatus[1].firstChild.nodeValue;
	link_sbstatus = wanStatus[2].firstChild.nodeValue;
	//usb_path1 = wanStatus[4].firstChild.nodeValue;
	//usb_path2 = wanStatus[3].firstChild.nodeValue;
	//monoClient = wanStatus[5].firstChild.nodeValue;	
	//cooler = wanStatus[6].firstChild.nodeValue;	

	if(location.pathname == "/QIS_wizard.htm")
		return false;	
	
	// internet
	if(sw_mode == 1){
		if((link_status == "2" && link_auxstatus == "0") || (link_status == "2" && link_auxstatus == "2")){
			$("conncet_status").className = "conncetstatuson";
			$("conncet_status").onclick = function(){openHint(24,3);}
			if(location.pathname == "/" || location.pathname == "/index.asp")
				$("NM_connect_status").innerHTML = "<#CTL_Connect#>";
		}
		else{
			$("conncet_status").className = "conncetstatusoff";
			if(link_status == 5)
				$("conncet_status").onclick = function(){openHint(24,3);}
			else
				$("conncet_status").onclick = function(){overHint(3);}
			if(location.pathname == "/" || location.pathname == "/index.asp")
			  $("NM_connect_status").innerHTML = "<#CTL_Disconnect#>";		 	
		}
		$("conncet_status").onmouseover = function(){overHint(3);}
		$("conncet_status").onmouseout = function(){nd();}
	}

	if(window.frames["statusframe"] && window.frames["statusframe"].stopFlag == 1 || stopFlag == 1) return;	
	setTimeout("updateStatus_AJAX();", 3000);
}

function FormActions(_Action, _ActionMode, _ActionScript, _ActionWait){
	if(_Action != "")
		document.form.action = _Action;
	if(_ActionMode != "")
		document.form.action_mode.value = _ActionMode;
	if(_ActionScript != "")
		document.form.action_script.value = _ActionScript;
	if(_ActionWait != "")
		document.form.action_wait.value = _ActionWait;
}

function change_wl_unit(){
	FormActions("apply.cgi", "change_wl_unit", "", "");
	document.form.target = "";
	document.form.submit();
}

function compareWirelessClient(target1, target2){
	if(target1.length != target2.length)
		return (target2.length-target1.length);
	
	for(var i = 0; i < target1.length; ++i)
		for(var j = 0; j < 3; ++j)
			if(target1[i][j] != target2[i][j])
					return 1;
	
	return 0;
}

function DecodeSpecificChar(s, uri){
	var decodeSTR;
	if (uri == 1)
		decodeSTR = decodeURIComponent(s);
	else
		decodeSTR = s;
	decodeSTR = decodeSTR.replace(/&/g,'&amp;');
	decodeSTR = decodeSTR.replace(/</g,'&lt;');
	decodeSTR = decodeSTR.replace(/ /g,'&nbsp;');
	return decodeSTR;
}

function switchType(obj, hide){
	var _Type = hide ? "text" : "password";
	if (obj.type != _Type) {
		if (navigator.userAgent.indexOf("MSIE") != -1) {
			var input2 = document.createElement('input');
			input2.mergeAttributes(obj);
			input2.id = obj.id? obj.id : obj.name;
			input2.value = obj.value;
			input2.type = _Type;
			input2.name = obj.name;
			obj.parentNode.replaceChild(input2 ,obj);
			if (hide)
				setTimeout(function(){document.getElementById(input2.id).focus();document.getElementById(input2.id).select();},10);
		} else
			obj.type= _Type;
	}
}

function corrected_timezone(){
	var today = new Date();
	var StrIndex;

	if (today.toString().lastIndexOf("-") > 0)
		StrIndex = today.toString().lastIndexOf("-");
	else if (today.toString().lastIndexOf("+") > 0)
		StrIndex = today.toString().lastIndexOf("+");

	if (StrIndex > 0) {
		if (timezone != today.toString().substring(StrIndex, StrIndex+5)) {
			$("timezone_hint").style.display = "block";
			$("timezone_hint").innerHTML = "<#LANHostConfig_x_TimeZone_itemhint#>";
		} else
			return;
	} else
		return;
}
