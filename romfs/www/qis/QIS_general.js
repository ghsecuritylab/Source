function validate_sta_wep_key(key_obj, n){
	var ret = true;

	if (key_obj.value.length == 5 && validate_string(key_obj))
		parent.document.QKform.sta_key_type[n].value = 1;
	else if (key_obj.value.length == 10 && validate_hex(key_obj))
		parent.document.QKform.sta_key_type[n].value = 0;
	else if (key_obj.value.length == 13 && validate_string(key_obj))
		parent.document.QKform.sta_key_type[n].value = 1;
	else if (key_obj.value.length == 26 && validate_hex(key_obj))
		parent.document.QKform.sta_key_type[n].value = 0;
	else
		ret = false;

	if (ret == false){
		alert("<#JS_wepkey#>\nWEP-64bits: <#WLANConfig11b_WEPKey_itemtype1#>\nWEP-128bits: <#WLANConfig11b_WEPKey_itemtype2#>\n");
		key_obj.focus();
		key_obj.select();
	}

	return ret;
}

var sw_mode = parent.document.QKform.sw_mode.value;
var winH;
	
function winW_H(){
	if(parseInt(navigator.appVersion) > 3){
		if(document.documentElement.clientHeight > document.documentElement.scrollHeight)
			winH = document.documentElement.clientHeight;
		else
			winH = document.documentElement.scrollHeight;
	}
} 

function QIS_showLoadingBar(seconds){
	disableCheckChangedStatus();
	
	if(location.pathname.indexOf("QIS_wizard.htm") < 0)
	clearHintTimeout();

	htmlbodyforIE = document.getElementsByTagName("html");  //this both for IE&FF, use "html" but not "body" because <!DOCTYPE html PUBLIC.......>
	htmlbodyforIE[0].style.overflow = "hidden";	  //hidden the Y-scrollbar for preventing from user scroll it.
	
	winW_H();
	loadingSeconds = seconds;
	progress = 100/loadingSeconds;
	y = 0;
	loading_complete = 0;
	
	/* depend on pages */
	if (document.form.current_page.value=='QIS_survey.htm')
		QIS_sitesurvey(seconds);
	else if (document.form.current_page.value=='QIS_finish.htm') {
		if (new_FW == false)
			QIS_apply(seconds);
		else
			QIS_fwupgrade(seconds);
	}
}

function QIS_sitesurvey(seconds){
	//alert(y + "+ "+ progress);
	y = y + progress;
	var img = 590 * y / 100; // widht = 590 * x %
	if(typeof(seconds) == "number" && seconds >= 0){
		if(seconds != 0){
			document.getElementById("proceeding_img").style.width = Math.round(img) + "px";
			document.getElementById("proceeding_per").innerHTML= Math.round(y) + "%";
			--seconds;
			setTimeout("QIS_sitesurvey("+seconds+");", 1000);
		}
		else{
			//document.getElementById("proceeding_img").innerHTML = "<#Main_alert_proceeding_desc3#>";
			document.getElementById("proceeding_per").innerHTML= "<#Main_alert_proceeding_desc3#>";
			document.getElementById("proceeding_info").innerHTML = "<#QIS_survey_desc3#>";
			y = 0;
			setTimeout("gotoNext();",1000);
		}
	}
}

var ajax_connection_Flag = 0;
var ajax_FWupdate_Flag = 0;	// 1:dhcpc fail, 2:FW version download fail, 3:FW version download success
function QIS_apply(seconds){
	//alert(y + "+ "+ progress);
	if(typeof(seconds) == "number" && seconds >= 0){
		if(seconds != 0){
			if(seconds > 1 && (ajax_connection_Flag == 1 || ajax_connection_Flag == 3
						|| ajax_FWupdate_Flag != 0)){	// speed up to 100%
				y = 100;
				seconds = 0;
			}
			else if(seconds <= 1 && ajax_FWupdate_Flag == 0){	// block in 99%
				if(ajax_connection_Flag == 0){
					y = 100;
					seconds = 0;
				}
				else
					y = 99;
			}
			else{
				y = y + progress;
				--seconds;
			}
			var img = 590 * y / 100; // widht = 590 * x %

			document.getElementById("proceeding_img").style.width = Math.round(img) + "px";
			document.getElementById("proceeding_per").innerHTML= Math.round(y) + "%";
			setTimeout("QIS_apply("+seconds+");", 1000);
		}
		else{
			document.getElementById("proceeding_per").innerHTML= "<#Main_alert_proceeding_desc3#>";
			document.getElementById("proceeding_info").innerHTML = "<#QIS_apply_desc3#>";
			y = 0;
			loading_complete = 1;//apply 100%
		}
	}
}

var ajax_FWupgrade_Flag = 0;	// 1:FW download fail, 2:FW download success, 3:FW upgrade success
function QIS_fwupgrade(seconds){
	//alert(y + "+ "+ progress);
	if(typeof(seconds) == "number" && seconds >= 0){
		if(seconds != 0){
			if(seconds > 1 && (ajax_FWupgrade_Flag == 1 || ajax_FWupgrade_Flag == 3)){
				y = 100;
				seconds = 0;
			}
			else if((seconds <= 140 && ajax_FWupgrade_Flag == 0)
					|| (seconds <= 1 && ajax_FWupgrade_Flag == 2))
				;
			else{
				y = y + progress;
				--seconds;
			}
			var img = 590 * y / 100; // widht = 590 * x %

			document.getElementById("proceeding_img").style.width = Math.round(img) + "px";
			document.getElementById("proceeding_per").innerHTML= Math.round(y) + "%";
			setTimeout("QIS_fwupgrade("+seconds+");", 1000);
		}
		else{
			document.getElementById("proceeding_per").innerHTML= "<#Main_alert_proceeding_desc3#>";
			y = 0;
			loading_complete = 1;//upgrade 100%
		}
	}
}

function extended_default_checked(){
	if(document.form.QIS_extended_default.checked == true){
		var extend_ssid0;	// 2.4G extended SSID
		var extend_ssid1;	// 5G extended SSID

		if(parent.select_firstPAP == "5G"){
			if (re_expressway == "0") {
				extend_ssid0 = parent.document.QKform.sta_ssid[1].value + '_RPT';
				extend_ssid1 = parent.document.QKform.sta_ssid[1].value + '_RPT5G';
				extended_default_key_checked(5);
			} else {
				extend_ssid0 = parent.document.QKform.sta_ssid[0].value + '_RPT';
				extend_ssid1 = parent.document.QKform.sta_ssid[0].value + '_RPT5G';
				extended_default_key_checked(2);
			}
		}
		else{
			if (re_expressway == "0") {
				extend_ssid0 = parent.document.QKform.sta_ssid[0].value + '_RPT';
				extend_ssid1 = parent.document.QKform.sta_ssid[0].value + '_RPT5G';
				extended_default_key_checked(2);
			} else {
				extend_ssid0 = parent.document.QKform.sta_ssid[1].value + '_RPT';
				extend_ssid1 = parent.document.QKform.sta_ssid[1].value + '_RPT5G';
				extended_default_key_checked(5);
			}
		}
		document.getElementById("QIS_apextended_ssid0").value = extend_ssid0;
		document.getElementById("QIS_apextended_ssid1").value = extend_ssid1;

		inputCtrl(document.form.QIS_apextended_ssid0, 0);
		inputCtrl(document.form.QIS_apextended_pwd0, 0);
		inputCtrl(document.form.QIS_apextended_ssid1, 0);
		inputCtrl(document.form.QIS_apextended_pwd1, 0);
		document.getElementById("no_syn_hint").style.visibility="hidden"; // hide hint
	}
	else{
		inputCtrl(document.form.QIS_apextended_ssid0, 1);
		inputCtrl(document.form.QIS_apextended_pwd0, 1);
		inputCtrl(document.form.QIS_apextended_ssid1, 1);
		inputCtrl(document.form.QIS_apextended_pwd1, 1);
		document.getElementById("no_syn_hint").style.visibility="visible"; // show hint
	}
}

function extended_default_key_checked(band){
	if(band == 5){
		var wpa_psk = parent.document.QKform.sta_wpa_psk[1].value;
		var wep_key1 = parent.document.QKform.sta_key1[1].value;
		var wep_key2 = parent.document.QKform.sta_key2[1].value;
		var wep_key3 = parent.document.QKform.sta_key3[1].value;
		var wep_key4 = parent.document.QKform.sta_key4[1].value;
	}else{
		var wpa_psk = parent.document.QKform.sta_wpa_psk[0].value;
		var wep_key1 = parent.document.QKform.sta_key1[0].value;
		var wep_key2 = parent.document.QKform.sta_key2[0].value;
		var wep_key3 = parent.document.QKform.sta_key3[0].value;
		var wep_key4 = parent.document.QKform.sta_key4[0].value;
	}

	if((wpa_psk == "") && (wep_key1 == "" && wep_key2 == "" && wep_key3 == "" && wep_key4 == ""))
		document.form.QIS_apextended_pwd0.value = "";
	else if(wpa_psk != "" )
		document.form.QIS_apextended_pwd0.value = wpa_psk;
	else if(wep_key1 != "")
		document.form.QIS_apextended_pwd0.value = wep_key1;
	else if(wep_key2 != "")
		document.form.QIS_apextended_pwd0.value = wep_key2;
	else if(wep_key3 != "")
		document.form.QIS_apextended_pwd0.value = wep_key3;
	else if(wep_key4 != "")
		document.form.QIS_apextended_pwd0.value = wep_key4;

	if(band == 2){
		var wpa_psk = parent.document.QKform.sta_wpa_psk[0].value;
		var wep_key1 = parent.document.QKform.sta_key1[0].value;
		var wep_key2 = parent.document.QKform.sta_key2[0].value;
		var wep_key3 = parent.document.QKform.sta_key3[0].value;
		var wep_key4 = parent.document.QKform.sta_key4[0].value;
	}else{
		var wpa_psk = parent.document.QKform.sta_wpa_psk[1].value;
		var wep_key1 = parent.document.QKform.sta_key1[1].value;
		var wep_key2 = parent.document.QKform.sta_key2[1].value;
		var wep_key3 = parent.document.QKform.sta_key3[1].value;
		var wep_key4 = parent.document.QKform.sta_key4[1].value;
	}

	if((wpa_psk == "") && (wep_key1 == "" && wep_key2 == "" && wep_key3 == "" && wep_key4 == ""))
		document.form.QIS_apextended_pwd1.value = "";
	else if(wpa_psk != "" )
		document.form.QIS_apextended_pwd1.value = wpa_psk;
	else if(wep_key1 != "")
		document.form.QIS_apextended_pwd1.value = wep_key1;
	else if(wep_key2 != "")
		document.form.QIS_apextended_pwd1.value = wep_key2;
	else if(wep_key3 != "")
		document.form.QIS_apextended_pwd1.value = wep_key3;
	else if(wep_key4 != "")
		document.form.QIS_apextended_pwd1.value = wep_key4;
}

/*****************************/
/*   For get AP list Array   */
/*****************************/
var aplist = parent.document.QKform.aplist_array.value; // aplist
var newAplist = new Array();
var profile_ssid="";

function LoadStringToArray(){
	var strArray = aplist.split(",");
	var ArrayNum = strArray.length/9;

	for(i=0; i<ArrayNum; i++){
		newAplist[i] = new Array(strArray[9*i], strArray[9*i+1],strArray[9*i+2],strArray[9*i+3],strArray[9*i+4],strArray[9*i+5],strArray[9*i+6],strArray[9*i+7],strArray[9*i+8]);
	}
}


function tune_list(arr){ // Shift connected ap to 1st position.
	var temp_obj = arr[0];
	for(var i=0; i<arr.length ;i++){
		if(arr[i][8] == 2){
			arr[0] = arr[i];
			arr[i] = temp_obj;
			break;
		}
	}
	return arr;
}


function updateIdx(selectedAP_bssid){
	for(var i=0; i<aplist.length ;i++){
		if(aplist[i][5] == selectedAP_bssid){
			idx = i;
			return true;
		}
		else{
			continue;
		}
	}
	return false;
}

function showList(){
	var html_code = "";
	var wl_RSSI;
	var n;
	
	/* [ssid, channel, encryption, crypto, rssi, bssid, bssidtype, frequency, connect_status] */
	aplist = tune_list(newAplist);
	
	if(aplist == ",,,,,,,,"){
		html_code += "<tr><td colspan='4' style='width:659px;font-size:17px;height:75px;text-align:center;'><#error_page_searching_noresult#></td></tr>";
		$("#aplist_table tbody").html(html_code);	
		return;
	}
	
	for(var i=0; i<aplist.length ;i++){
		/* If Char to ASCII have no matched strings, it will ignore, never show it!!! */
		/* fix traditional chinese can't show aplist issue */
		if(aplist[i][0].search("%FFFF") != -1) 
			continue;

		var frequency_value = frequency_2g_or_5g(aplist[i][7]);
		if(frequency_value == "5G"){
			parent.aroundAP_5G = 1;
			n = 1;
		}else{
			parent.aroundAP_2G = 1;
			n = 0;
		}

		if(Math.abs(aplist[i][4]) <= 20)     // signal:	Excellent
			wl_RSSI = 5;
		else if(Math.abs(aplist[i][4]) <= 40)// 	Very good
			wl_RSSI = 4;
		else if(Math.abs(aplist[i][4]) <= 60)//		Good
			wl_RSSI = 3;
		else if(Math.abs(aplist[i][4]) <= 80)//		Fairly
			wl_RSSI = 2;
		else                                 //		Poor
			wl_RSSI = 1;

		var decodeSSID, ellipsisSSID;
		// decode SSID to char
		if (aplist[i][8] != 0 && aplist[i][0] == "")
			decodeSSID = decodeURIComponent(parent.ha_sta_ssid[n]);
		else
			decodeSSID = decodeURIComponent(aplist[i][0]);
		// Shortening the SSID which the length is over 18 characters.
		ellipsisSSID = (decodeSSID.length > 18)?decodeSSID.substring(0,18)+"...":decodeSSID;
		ellipsisSSID = DecodeSpecificChar(ellipsisSSID, 0);

		if(parent.select_firstPAP == frequency_value && parent.select_anotherPAP == 1)
			html_code += "<tr style='display:none;'>\n";
		else
			html_code += "<tr>\n";
		html_code += "<td width='191px' align='left' style='padding-left:15px;' abbr=''>";

		if (aplist[i][8] != 0 && aplist[i][0] == "")
			html_code += "<span class='ssid' style='color:#999;'>"+ ellipsisSSID +"</span>";
		else
			html_code += "<span class='ssid'>"+ ellipsisSSID + "</span>";
			
		html_code += (aplist[i][8] == 2)?"<span class='connected' title=\"<#CTL_Add_enrollee#>\"></span>\n":"<span class='blank_block'>&nbsp;</span>";  // isConnected		
		//html_code += (aplist[i][8]==1)?"<span class='star'></span>\n":"<span class='blank_block'>&nbsp;</span>";  // isProfile
		//html_code += (aplist[i][7])?"<span class='wl_mode'>" +aplist[i][7]+ "</span>":"";  //bgn
		html_code += "<input type='hidden' value='"+ i +"'>"; // index for get value from Array		
		html_code += "</td>\n";
		
		html_code += "<td width='70px' align='center' >"+ aplist[i][1] +"</td>\n";  //Channel
		html_code += "<td width='160px' class='auth' align='left' style='padding-left:15px;'>";  //Authentication (encryption)
		html_code += aplist[i][2] +" (" +aplist[i][3]+ ")";
		
		if(aplist[i][2] == "Open System" && aplist[i][3] == "NONE"){
			html_code += "<img src='/images/survey/security.png' align='top'/></td>\n";
		}
		
		/* table sorting */
		//add sinal's attr
		html_code += "<td width='100px' align='center'><img src='/images/survey/radio"+ wl_RSSI +".png' title='" + Math.abs(aplist[i][4]) + "' /></td>\n"; //RSSI 
		/* table sorting */
		html_code += "<td width='100px' align='center'>" + frequency_value + "</td>\n"; // frequency
		html_code += "</tr>\n";
	}
	//$("#aplist_table thead").html('<tr><th width="225px"><#NM_Wireless_name#></th><th width="75px"><#WLANConfig11b_Channel_itemname#></th><th width="225px"><#QIS_Security#></th><th width="75px"><#QIS_Radio#></th></tr>');
	$("#aplist_table tbody").html(html_code);																									
	//$("#aplist_table tbody").css("overflow-y", "scroll");
	//$("#aplist_table tbody tr:even").css("background", "#FFFFFF");
	bind_event();
}

/* check frequency and return to show aplist */
function frequency_2g_or_5g(wmode){
	var freq;
	if (wmode == "b" || wmode == "bg" || wmode == "bgn")
		freq = "2.4G";
	else if (wmode == "a" || wmode == "an")
		freq = "5G";
	else
		freq = "5G";
	return freq;
}

var Connected0Tr = "";
var Connected0TrBg = "";
var Connected0TrC = "";
var Connected1Tr = "";
var Connected1TrBg = "";
var Connected1TrC = "";
var lastClickTr = "";  //for click event and style change.
var lastClickTrBg = "";
var lastClickTrC = "";
var lastClickTrAlert = "";
var lastClickTrSSID = "";
var idx = "1";
var disable_bind_event = "0";

function bind_event(){ //bind <tr> click and hover event
	$("#aplist_table tbody tr").bind("click", function(e){	// bind click event to <tr> in <tbody>
		if (disable_bind_event == "1")
			return;

		if(Connected0Tr){
			$(".inputRow").remove();				// remove input Row
			Connected0Tr.css("background",Connected0TrBg);		// background-color recover
			Connected0Tr.find("td").css("color",Connected0TrC);	// text-color recover
		}
		if(Connected1Tr){
			$(".inputRow").remove();				// remove input Row
			Connected1Tr.css("background",Connected1TrBg);		// background-color recover
			Connected1Tr.find("td").css("color",Connected1TrC);	// text-color recover
		}
		if(lastClickTr){
			$(".inputRow").remove();				// remove input Row
			lastClickTr.css("background",lastClickTrBg);		// background-color recover
			lastClickTr.find("td").css("color",lastClickTrC);	// text-color recover
		}

		lastClickTr = $(this);
		lastClickTrBg = $(this).css("background-color");
		lastClickTrC = $(this).find("td").css("color"); 		// warning : don't use $(this).css("color");
		lastClickTrAlert = $(this).children().eq(2).html(); 		// check Security Content
		lastClickTrSSID = $(this).children().eq(0).children().eq(0).html(); 		// check SSID Content

		$(this).css("background-color", "#708789");			// change bg-color
		$(this).find("td").css("color", "#FFFFFF");			// change text-color
		
		idx = $(this).children().eq(0).find("input").val();		//Get index value
		var n;
		if (frequency_2g_or_5g(aplist[idx][7]) == "5G")
			n = 1;
		else
			n = 0;
		parent.document.QKform.cur_freq.value = n;
		if (nonconcurrent_support != -1)
			n = 0;
		
		/* inputRow  */
		var initRow = "<tr class=\"inputRow\" style=\"display:none;background-color:#5E7173;\"><td style=\"height:55px;padding-left:10px;\" colspan=\"5\"><table><tr>";
		var unEncrypt = "<td style=\"border:0px;\"><span style=\"color:#FFCC00;\"><#QIS_aplist_desc4#></span></td>";
		var connectBtn = "<td style=\"border:0px;\"><input id=\"connect_btn\" type=\"button\" value=\"<#CTL_Add_enrollee#>\" onclick=\"btnHandler.APList_conn("+n+");\" class=\"button_gen\"></td></tr>";
		var networkKey = "<td style=\"border:0px;\"><span style=\"color:#FFFFFF;\"><#QIS_Security_Key#>: </span></td><td style=\"border:0px;\"><input type=\"password\" id=\"QIS_apencryption_pwd\" name=\"QIS_apencryption_pwd\" onKeyPress=\"return _submitenter(this,event);\" onBlur=\"switchType(this, false);\" onFocus=\"switchType(this, true);\" class=\"input_aplist_table\"></td>";
                var wepkeyIndex = "<td style=\"border:0px;\"><span style=\"margin-left:5px;color:#FFFFFF;\"><#QIS_Key_index#></span><select name=\"sta_key_index\" class=\"input_option\"><option value=\"1\" selected>1</option><option value=\"2\">2</option><option value=\"3\">3</option><option value=\"4\">4</option></select></td>";
		var wep_auth = "<td style=\"border:0px;\"><span style=\"margin-left:5px;color:#FFFFFF;\">Auth: </span><select name=\"wl_wep_auth\" class=\"input_option\"><option value=\"open\" selected>open</option><option value=\"shared\">shared</option></select></td>";
		var no_enterprise = "<td style=\"border:0px;\"><span style=\"color:#FFCC00;\"><#QIS_RE_not_support_Enterprise#></span></td></tr></table></tr>";
		var runtime_hint = "<tr id=\"runtime_chk_hint_tr\" style=\"display:none;\"><td style=\"border:0px;color:#FFCC00;\" colspan=\"5\"><span id=\"runtime_chk_hint\"></span></td></tr>";
		var end_of_Row = "</table></td></tr>";

		/* enterprise */
		var enterprise_Row = '';
		enterprise_Row += "<td style=\"border:0px;color:#FFFFFF;\"><#QIS_aplist_auth_mode#> :</td>";
		enterprise_Row += "<td style=\"border:0px;\" id=\"cert_auth_type_from_wpa\"><select onchange=\"switch_Tunnel();\" class=\"input_aplist_option\" name=\"cert_auth_type_from_wpa\"><option selected=\"\" value=\"5\"><#QIS_aplist_auth_mode_peap#></option><option value=\"6\"><#QIS_aplist_auth_mode_ttls#></option></select></td>";
		enterprise_Row += "<td style=\"border:0px;color:#FFFFFF;padding-left:10px;\"><#QIS_aplist_id#> :</td><td style=\"border:0px;\"><input type=\"text\" class=\"input_aplist_table\" style=\"\" id=\"cert_id\" name=\"cert_id\"></td></tr>";
		enterprise_Row += "<tr><td style=\"border:0px;color:#FFFFFF;\"><#QIS_aplist_tunnel#> :</td>";
		enterprise_Row += "<td id=\"cert_tunnel_auth_peap\" style=\"border:0px;\"><select class=\"input_aplist_option\" name=\"cert_tunnel_auth_peap\"><option selected=\"\" value=\"1\"><#QIS_aplist_tunnel_MSCHAPV2#></option></select></td>";
		enterprise_Row += "<td id=\"cert_tunnel_auth_ttls\" style=\"border:0px;display:none;\"><select class=\"input_aplist_option\" name=\"cert_tunnel_auth_ttls\"><option value=\"0\"><#QIS_aplist_tunnel_MSCHAP#></option><option selected=\"\" value=\"1\"><#QIS_aplist_tunnel_MSCHAPV2#></option><option value=\"2\"><#QIS_aplist_tunnel_PAP#></option></select></td>";
		enterprise_Row += "<td style=\"border:0px;color:#FFFFFF;padding-left:10px;\"><#QIS_aplist_password#> :</td><td style=\"border:0px;\"><input type=\"password\" class=\"input_aplist_table\" style=\"\" id=\"cert_password\" name=\"cert_password\" onBlur=\"switchType(this, false);\" onFocus=\"switchType(this, true);\"></td>";
		enterprise_Row += "<td style=\"border:0px;padding-left:10px;\"><input id=\"connect_btn\" type=\"button\" value=\"<#CTL_Add_enrollee#>\" onclick=\"btnHandler.APList_conn("+n+");\" class=\"button_gen\"></td></tr>";

		var hiddenssid = "<td style=\"border:0px;color:#FFFFFF;\"><#QIS_SSID#> :</td><td style=\"border:0px;\"><input type=\"text\" name=\"QIS_apencryption_id\" class=\"input_aplist_table\"></td></tr><tr>";

		if(lastClickTrAlert.indexOf("Open") > -1){ // not -1 : "Open System(NONE)"
			if(lastClickTrSSID == "") initRow += hiddenssid; // hidden SSID
			initRow += unEncrypt;
			initRow += connectBtn;
			initRow += end_of_Row;
		}
		else if(lastClickTrAlert.indexOf("Unknown") > -1){ // not -1 : "Unknown(WEP)"
			if(lastClickTrSSID == "") initRow += hiddenssid; // hidden SSID
			initRow += networkKey;
			initRow += wep_auth;
			initRow += wepkeyIndex;
			initRow += connectBtn;
			initRow += runtime_hint;
			initRow += end_of_Row;
		}
		else if(lastClickTrAlert.indexOf("WPA-Personal") > -1 
			|| lastClickTrAlert.indexOf("WPA2-Personal") > -1){ // not -1 : "WPA/WPA2-Personal"
			if(lastClickTrSSID == "") initRow += hiddenssid; // hidden SSID
			initRow += networkKey;
			initRow += connectBtn;
			initRow += runtime_hint;
			initRow += end_of_Row;
		}
		/* EA mode support enterprise */
		else if((lastClickTrAlert.indexOf("WPA-Enterprise") > -1 
			|| lastClickTrAlert.indexOf("WPA2-Enterprise") > -1) 
			&& sw_mode == "4"){ // not -1 : "WPA/WPA2-Enterprise"
			if(lastClickTrSSID == "") initRow += hiddenssid; // hidden SSID
			initRow += enterprise_Row;
		}
		/* RE mode NOT support enterprise */
		else if((lastClickTrAlert.indexOf("WPA-Enterprise") > -1 || lastClickTrAlert.indexOf("WPA2-Enterprise") > -1) 
			&& sw_mode == "2"){ // not -1 : "WPA/WPA2-Enterprise"
			initRow += no_enterprise;
		}	

		$(this).after(initRow); // add input Row after click item
                $(".inputRow").show(); 	// show input Row

		// selectd AP info key into QKform
		// [ssid, channel, encryption, crypto, rssi,bssid, bssidtype, frequency, connect_status]
		if (aplist[idx][8] != 0 && aplist[idx][0] == "")
			parent.document.QKform.sta_ssid[n].value = decodeURIComponent(parent.ha_sta_ssid[n]);
		else
			parent.document.QKform.sta_ssid[n].value = decodeURIComponent(aplist[idx][0]);
		runtime_sta_ch = aplist[idx][1];
		parent.document.QKform.sta_bssid[n].value = aplist[idx][5];
		parent.document.QKform.sta_auth_mode_tmp[n].value = aplist[idx][2];
		parent.document.QKform.sta_encryption[n].value = aplist[idx][3];
	}),
	$(".ssid").hover(   //Show tooltips of SSID & MAC
		function(){
			idx = $(this).parent().eq(0).find("input").val();

			var n, decodeSSID, abbrtext = "";
			if (aplist[idx][8] != 0 && aplist[idx][0] == "") {
				if (frequency_2g_or_5g(aplist[idx][7]) == "5G")
					n = 1;
				else
					n = 0;
				decodeSSID = DecodeSpecificChar(parent.ha_sta_ssid[n], 1);
			} else
				decodeSSID = DecodeSpecificChar(aplist[idx][0], 1);
			abbrtext = "<strong>SSID: </strong>"+ decodeSSID + "<br/><strong>MAC: </strong>" + aplist[idx][5];
			return overlib(abbrtext, HAUTO, VAUTO);
		},
		function(){
			return cClick(); // clear text
		}
	);
	for(var i=0; i<aplist.length ;i++){
		if(aplist[i][8] == 2){  //set the last click is selected <tr>
			if(frequency_2g_or_5g(aplist[i][7]) == "5G"){
				Connected0Tr = $("#aplist_table tbody tr").eq(i);
				Connected0TrBg = Connected0Tr.css("background-color");
				Connected0TrC = Connected0Tr.find("td").css("color");
			}else{
				Connected1Tr = $("#aplist_table tbody tr").eq(i);
				Connected1TrBg = Connected1Tr.css("background-color");
				Connected1TrC = Connected1Tr.find("td").css("color");
			}
			$("#aplist_table tbody tr").eq(i).css("background-color", "#708789");
			$("#aplist_table tbody tr").eq(i).find("td").css("color", "#FFFFFF");
		}
	}
}

function chk_sta_password(n){
	var auth_mode = parent.document.QKform.sta_auth_mode_tmp[n].value;
	var encryption = parent.document.QKform.sta_encryption[n].value;
	
	if(!validate_key(n)){
		return false;	
	}
	
	if(auth_mode == "Open System"){ // open-none
		parent.document.QKform.sta_auth_mode[n].value = "open";
		parent.document.QKform.sta_wep_x[n].value = "0";
	}
	else if(auth_mode == "Unknown" && encryption == "WEP"){ //wep
		if(document.form.QIS_apencryption_pwd.value.length == 5 || document.form.QIS_apencryption_pwd.value.length == 10)
			parent.document.QKform.sta_wep_x[n].value = "1";
		else if(document.form.QIS_apencryption_pwd.value.length == 13 || document.form.QIS_apencryption_pwd.value.length == 26)
			parent.document.QKform.sta_wep_x[n].value = "2";
		else
			parent.document.QKform.sta_wep_x[n].value = "0";
		
		if(document.form.wl_wep_auth.value == "open"){
			parent.document.QKform.sta_auth_mode[n].value = "open";
		}
		else if(document.form.wl_wep_auth.value == "shared"){
			parent.document.QKform.sta_auth_mode[n].value = "shared";
		}
			
		parent.document.QKform.sta_key[n].value = document.form.sta_key_index.options[document.form.sta_key_index.selectedIndex].value;
		if (parent.document.QKform.sta_key[n].value == "1")
			parent.document.QKform.sta_key1[n].value = document.form.QIS_apencryption_pwd.value;
		else if(parent.document.QKform.sta_key[n].value == "2")
			parent.document.QKform.sta_key2[n].value = document.form.QIS_apencryption_pwd.value;
		else if(parent.document.QKform.sta_key[n].value == "3")
			parent.document.QKform.sta_key3[n].value = document.form.QIS_apencryption_pwd.value;
		else if(parent.document.QKform.sta_key[n].value == "4")
			parent.document.QKform.sta_key4[n].value = document.form.QIS_apencryption_pwd.value;
	}	
	else if(auth_mode == "WPA-Personal"){
		parent.document.QKform.sta_auth_mode[n].value = "psk";
		parent.document.QKform.sta_wep_x[n].value = "0";
		parent.document.QKform.sta_wpa_mode[n].value = "1";

		if (parent.document.QKform.sta_encryption[n].value == "TKIP")
			parent.document.QKform.sta_crypto[n].value = "tkip";
		else if (parent.document.QKform.sta_encryption[n].value == "AES")
			parent.document.QKform.sta_crypto[n].value = "aes";

		parent.document.QKform.sta_wpa_psk[n].value = document.form.QIS_apencryption_pwd.value;
	}	
	else if(auth_mode == "WPA2-Personal"){
		parent.document.QKform.sta_auth_mode[n].value = "psk";
		parent.document.QKform.sta_wep_x[n].value = "0";
		parent.document.QKform.sta_wpa_mode[n].value = "2";

		if (parent.document.QKform.sta_encryption[n].value == "TKIP")
			parent.document.QKform.sta_crypto[n].value = "tkip";
		else if (parent.document.QKform.sta_encryption[n].value == "AES")
			parent.document.QKform.sta_crypto[n].value = "aes";

		parent.document.QKform.sta_wpa_psk[n].value = document.form.QIS_apencryption_pwd.value;
	}
	else if(auth_mode == "WPA-Enterprise" || auth_mode == "WPA2-Enterprise"){ // For Enterprise
		if(!chk_wpa_supplicant(n)) return false;
	}
	
	return true;
}

function validate_key(n){
	var auth_mode = parent.document.QKform.sta_auth_mode_tmp[n].value;
	if(auth_mode == "WPA-Personal" || auth_mode == "WPA2-Personal"){
		if (!re_validate_psk(document.form.QIS_apencryption_pwd))
			return false;
		else
			return true;
	}
	else if(auth_mode == "Unknown" || auth_mode == "WEP"){
		if (!validate_sta_wep_key(document.form.QIS_apencryption_pwd, n))
			return false;
		else
			return true;
	}
	else
		return true;
}

/* change small flow image */
function change_setting_img(m, p){
	/* follow mode and page to check small_flow img */
	if (p == "survey") {
		if (m == 2 || m == 5) {
			document.getElementById("smaill_flow_4").src="/images/small_flow/blank.png";
			document.getElementById("smaill_flow_5").src="/images/small_flow/flow03-2.png";
		} else if (m == 4) {
			document.getElementById("smaill_flow_4").style.top="16px";
			document.getElementById("smaill_flow_4").src="/images/small_flow/flow-wire.png";
			document.getElementById("smaill_flow_5").src="/images/small_flow/flow03.png";
		}
	} else if (p == "aplist") {
		if (m == 2 || m == 5) {
			document.getElementById("smaill_flow_4").src="/images/small_flow/blank.png";
			document.getElementById("smaill_flow_5").src="/images/small_flow/flow03-2.png";
		} else if (m == 4) {
			document.getElementById("smaill_flow_4").style.top="16px";
			document.getElementById("smaill_flow_4").src="/images/small_flow/flow-wire.png";
			document.getElementById("smaill_flow_5").src="/images/small_flow/flow03.png";
		}
	} else if (p == "internet") {// internet(RT)
		document.getElementById("smaill_flow_4").src="/images/small_flow/blank.png";
		document.getElementById("smaill_flow_5").src="/images/small_flow/flow03-2.png";
	} else if (p == "wireless") {// wireless(RT/AP)
		document.getElementById("smaill_flow_2").style.top="16px";
	} else if (p == "apextended") {// extended(RE)
		document.getElementById("smaill_flow_2").style.top="9px";
	} else if (p == "finish") {
		if (m == 2 || m == 5) {
			document.getElementById("smaill_flow_1_a").src="/images/small_flow/flow01.png";
			document.getElementById("smaill_flow_2_a").style.top="9px";
			document.getElementById("smaill_flow_2_a").src="/images/small_flow/flow-wifi.png";
			document.getElementById("smaill_flow_2_f").style.top="9px";
			document.getElementById("smaill_flow_4_f").style.top="9px";
		} else if (m == 1 || m == 3) {
			document.getElementById("smaill_flow_1_a").src="/images/small_flow/flow01.png";
			document.getElementById("smaill_flow_2_a").style.top="16px";
			document.getElementById("smaill_flow_2_a").src="/images/small_flow/flow-wire.png";
			document.getElementById("smaill_flow_4_a").src="/images/small_flow/setting.gif";
			document.getElementById("smaill_flow_1_f").src="/images/small_flow/flow01.png";
			document.getElementById("smaill_flow_2_f").style.top="16px";
			document.getElementById("smaill_flow_2_f").src="/images/small_flow/flow-wire.png";
			document.getElementById("smaill_flow_4_f").style.top="9px";
		} else if (m == 4) {
			document.getElementById("smaill_flow_4_a").style.top="16px";
			document.getElementById("smaill_flow_4_a").src="/images/small_flow/flow-wire.png";
			document.getElementById("smaill_flow_5_a").src="/images/small_flow/flow03.png";
			document.getElementById("smaill_flow_2_f").style.top="9px";
			document.getElementById("smaill_flow_4_f").style.top="16px";
			document.getElementById("smaill_flow_4_f").src="/images/small_flow/flow-wire.png";
		}
	} else if (p == "led") {
		if (m == 2 || m == 5) {
			document.getElementById("smaill_flow_2_f").style.top="9px";
			document.getElementById("smaill_flow_4_f").style.top="9px";
		} else if (m == 1 || m == 3) {
			document.getElementById("smaill_flow_1_f").src="/images/small_flow/flow01-2.png";
			document.getElementById("smaill_flow_2_f").src="/images/small_flow/blank.png";
			document.getElementById("smaill_flow_4_f").style.top="16px";
			document.getElementById("smaill_flow_4_f").src="/images/small_flow/flow-wire.png";
		} else if (m == 4) {
			document.getElementById("smaill_flow_2_f").style.top="9px";
			document.getElementById("smaill_flow_4_f").style.top="16px";
			document.getElementById("smaill_flow_4_f").src="/images/small_flow/flow-wire.png";
		}
	}
}

/* change hint image of wireless info */
function change_hint_img(o){
	var os = OS_type();
	if (os == "Windows XP")
		o.src = "/images/qis/winxp_wifi.png";
	else if (os == "iOS")
		o.src = "/images/qis/ios_wifi.png";
	else if (os == "Mac OS")
		o.src = "/images/qis/mac_wifi.png";
	else if (os == "Android")
		o.src = "/images/qis/android_wifi.png";
}
/* show wireless info for client */
function fill_wireless_info(n, preID){
	/* SSID*/
	document.getElementById(preID+'SSID'+n).innerHTML = ssid_decode[n];

	/* Security && Key*/
	if (auth_mode[n] == "open" && wep_x[n] == "0") {
		document.getElementById(preID+'auth'+n).innerHTML = "OPEN-NONE";
		document.getElementById(preID+'key'+n).innerHTML = "";
	} else if ((auth_mode[n] == "open" || auth_mode[n] == "shared") && (wep_x != "0")) {
		document.getElementById(preID+'auth'+n).innerHTML = "WEP";
		wep_key_show(n, preID);
	} else if (auth_mode[n] == "psk") {
		if (wpa_mode[n] == "0") {
			document.getElementById(preID+'auth'+n).innerHTML = "WPA/WPA2";
			document.getElementById(preID+'key'+n).innerHTML = wpa_psk_decode[n];
		} else if (wpa_mode[n] == "1") {
			document.getElementById(preID+'auth'+n).innerHTML = "WPA";
			document.getElementById(preID+'key'+n).innerHTML = wpa_psk_decode[n];
		} else if (wpa_mode[n] == "2") {
			document.getElementById(preID+'auth'+n).innerHTML = "WPA2";
			document.getElementById(preID+'key'+n).innerHTML = wpa_psk_decode[n];
		}
	}
}

function wep_key_show(n, preID){
	if (key_index[n] == '1')
		document.getElementById(preID+'key'+n).innerHTML = key1[n];
	else if (key_index[n] == '2')
		document.getElementById(preID+'key'+n).innerHTML = key2[n];
	else if (key_index[n] == '3')
		document.getElementById(preID+'key'+n).innerHTML = key3[n];
	else if (key_index[n] == '4')
		document.getElementById(preID+'key'+n).innerHTML = key4[n];
}

/* check wpa/wpa2 enterprise for wpa supplicant*/
function chk_wpa_supplicant(n){
	var cert_id = document.form.cert_id.value;
	var cert_password = document.form.cert_password.value;
	var auth_mode = parent.document.QKform.sta_auth_mode_tmp[n].value;
	var encryption = parent.document.QKform.sta_encryption[n].value;
	
	if( cert_id == "" || cert_password == "" ){
		alert("<#QIS_aplist_warning#>");
		return false;
	}
	
	/* security_infra*/
	if(auth_mode == "WPA-Enterprise"){
		// Auth
		parent.document.QKform.sta_auth_mode[n].value = "wpa";
		parent.document.QKform.sta_wpa_mode[n].value = "3";
		parent.document.QKform.security_infra.value = "3";
	}
	else if(auth_mode == "WPA2-Enterprise"){
		// Auth
		parent.document.QKform.sta_auth_mode[n].value = "wpa2";
		parent.document.QKform.security_infra.value = "6";
	}

	/* wl_crypto */
	if (parent.document.QKform.sta_encryption[n].value == "TKIP")
		parent.document.QKform.sta_crypto[n].value = "tkip";
	else if (parent.document.QKform.sta_encryption[n].value == "AES")
		parent.document.QKform.sta_crypto[n].value = "aes";
	
	/* cert_auth_type_from_wpa, cert_tunnel_auth_peap, cert_tunnel_auth_ttls, cert_id, cert_password */
	parent.document.QKform.cert_auth_type_from_wpa.value = document.form.cert_auth_type_from_wpa.value;
	parent.document.QKform.cert_tunnel_auth_peap.value = document.form.cert_tunnel_auth_peap.value;
	parent.document.QKform.cert_tunnel_auth_ttls.value = document.form.cert_tunnel_auth_ttls.value;
	parent.document.QKform.cert_id.value = document.form.cert_id.value;
	parent.document.QKform.cert_password.value = document.form.cert_password.value;

	return true;

}

function switch_Tunnel(){
	if(document.form.cert_auth_type_from_wpa.value == "5"){
		document.getElementById("cert_tunnel_auth_peap").style.display="";
		document.getElementById("cert_tunnel_auth_ttls").style.display="none";
	}
	else if(document.form.cert_auth_type_from_wpa.value == "6"){
		document.getElementById("cert_tunnel_auth_peap").style.display="none";
		document.getElementById("cert_tunnel_auth_ttls").style.display="";
	}
}

function submitenter(o, e){
	var keycode;
	if (window.event) keycode = window.event.keyCode;
	else if (e) keycode = e.which;
	else return;

	if (keycode == 13)
		gotoNext();
}
