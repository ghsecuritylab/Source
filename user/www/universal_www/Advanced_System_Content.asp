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
<title><#Web_Title#> - <#menu4_2#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="pwdmeter.css">
<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script language="JavaScript" type="text/javascript" src="/help.js"></script>
<script language="JavaScript" type="text/javascript" src="/detect.js"></script>
<script type="text/javascript" src="/jquery.js"></script>
<script>
var $j = jQuery.noConflict();
time_day = uptimeStr.substring(5,7);//Mon, 01 Aug 2011 16:25:44 +0800(1467 secs since boot....
time_mon = uptimeStr.substring(9,12);
time_time = uptimeStr.substring(18,20);
dstoffset = '<% nvram_get("time_zone_dstoff"); %>';

function initial(){
	show_menu();
	corrected_timezone();
	load_timezones();
	hidden_NTP_server();
	show_dst_chk();	
	load_dst_m_Options();
	load_dst_w_Options();
	load_dst_d_Options();
	load_dst_h_Options();
	document.form.http_passwd2.value = "";
	chkPasswrodStrength('<% nvram_get("http_passwd"); %>', 'http_passwd');
	if (led_ctl_support != -1)
		document.getElementById("led_ctl").style.display = "";
}

function hidden_NTP_server(){
	if(sw_mode == 4){
		document.getElementById("systemtitle").innerHTML="<#System_title_no_ntp#>";
		document.getElementById("log_server").style.display="none";
		document.getElementById("Time_server").style.display="none";
		document.getElementById("NTP_server").style.display="none";
	}
	else{
		document.getElementById("systemtitle").innerHTML="<#System_title#>";
		document.getElementById("log_server").style.display="";
		document.getElementById("Time_server").style.display="";
		document.getElementById("NTP_server").style.display="";
	}
}

var time_zone_tmp="";
var time_zone_s_tmp="";
var time_zone_e_tmp="";
var time_zone_withdst="";
function applyRule(){
	if(validForm()){
		if(document.form.http_passwd2.value.length > 0)
			document.form.http_passwd.value = document.form.http_passwd2.value;

		if(document.form.time_zone_dst_chk.checked){	// Exist dstoffset
				time_zone_tmp = document.form.time_zone_select.value.split("_");	//0:time_zone 1:serial number
				time_zone_s_tmp = "M"+document.form.dst_start_m.value+"."+document.form.dst_start_w.value+"."+document.form.dst_start_d.value+"/"+document.form.dst_start_h.value;
				time_zone_e_tmp = "M"+document.form.dst_end_m.value+"."+document.form.dst_end_w.value+"."+document.form.dst_end_d.value+"/"+document.form.dst_end_h.value;
				document.form.time_zone_dstoff.value=time_zone_s_tmp+","+time_zone_e_tmp;
				document.form.time_zone.value = document.form.time_zone_select.value;	
		}else{			
				document.form.time_zone_dstoff.value="";
				document.form.time_zone.value = document.form.time_zone_select.value;
		}
		document.form.submit();
	}
}

function validForm(){
	var alert_str = "";

	showtext($("alert_msg1"), "");
	showtext($("alert_msg2"), "");

	alert_str = validate_LoginUsername(document.form.http_username);
	showtext($("alert_msg1"), alert_str);
	if (alert_str != "") {
		document.form.http_username.focus();
		document.form.http_username.select();
		return false;
	}

	if (document.form.http_passwd2.value != "" ) {
		alert_str = validate_LoginPasswd(document.form.http_passwd2, document.form.v_password2);
		showtext($("alert_msg2"), alert_str);
		if (alert_str != "") {
			document.form.http_passwd2.focus();
			document.form.http_passwd2.select();
			return false;
		}
		var is_common_string = check_common_string(document.form.http_passwd2.value, "httpd_password");
		if (is_common_string) {
			if (!confirm("<#JS_common_passwd#>")) {
				document.form.http_passwd2.focus();
				document.form.http_passwd2.select();
				return false;
			}
		}
	}

	if(!validate_ipaddr(document.form.log_ipaddr, 'log_ipaddr')
			|| !validate_string(document.form.ntp_server0)
			)
		return false;
		
	if(document.form.time_zone_dst_chk.checked && 
		document.form.dst_start_m.value == document.form.dst_end_m.value &&
		document.form.dst_start_w.value == document.form.dst_end_w.value &&
		document.form.dst_start_d.value == document.form.dst_end_d.value)
	{
		alert("<#FirewallConfig_URLActiveTime_itemhint4#>");
		return false;
	}
	
	if(document.form.http_passwd2.value.length > 0 
			|| document.form.http_username.value != '<% nvram_get("http_username"); %>')
		alert("<#System_alert_desc1#>");
	
	return true;
}

function show_dst_chk(){
	var tzdst = new RegExp("^[a-z]+[0-9\-\.:]+[a-z]+", "i");
	// match "[std name][offset][dst name]"
	if(document.form.time_zone_select.value.match(tzdst)){
		document.getElementById("chkbox_time_zone_dst").style.display="";	
		if(!document.getElementById("time_zone_dst_chk").checked){
				document.form.time_zone_dst.value=0;
				document.getElementById("dst_start").style.display="none";
				document.getElementById("dst_end").style.display="none";
		}else{
				parse_dstoffset();
				document.form.time_zone_dst.value=1;
				document.getElementById("dst_start").style.display="";
				document.getElementById("dst_end").style.display="";
		}
	}else{
		document.getElementById("chkbox_time_zone_dst").style.display="none";
		document.getElementById("chkbox_time_zone_dst").checked=false;
		document.form.time_zone_dst.value=0;
		document.getElementById("dst_start").style.display="none";
		document.getElementById("dst_end").style.display="none";
	}	
}

var timezones = [
	["UTC12",	"(GMT-12:00) <#TZ01#>"],
	["UTC11",	"(GMT-11:00) <#TZ02#>"],
	["UTC10",	"(GMT-10:00) <#TZ03#>"],
	["NAST9DST",	"(GMT-09:00) <#TZ04#>"],
	["PST8DST",	"(GMT-08:00) <#TZ05#>"],
	["MST7DST_1",	"(GMT-07:00) <#TZ06#>"],
	["MST7_2",	"(GMT-07:00) <#TZ07#>"],
	["MST7DST_3",	"(GMT-07:00) <#TZ08#>"],
	["CST6DST_1",	"(GMT-06:00) <#TZ09#>"],
	["CST6DST_2",	"(GMT-06:00) <#TZ10#>"],
	["CST6DST_3",	"(GMT-06:00) <#TZ11#>"],
	["CST6DST_3_1",	"(GMT-06:00) <#TZ12#>"],
	["UTC6",	"(GMT-06:00) <#TZ13#>"],
	["EST5DST",	"(GMT-05:00) <#TZ14#>"],
	["UTC5_1",	"(GMT-05:00) <#TZ15#>"],
	["UTC5_2",	"(GMT-05:00) <#TZ16#>"],
	["AST4DST",	"(GMT-04:00) <#TZ17#>"],
	["UTC4_1",	"(GMT-04:00) <#TZ18#>"],
	["UTC4DST_2",	"(GMT-04:00) <#TZ19#>"],
	["NST3.30DST",	"(GMT-03:30) <#TZ20#>"],
	["EBST3DST_1",	"(GMT-03:00) <#TZ21#>"],
	["UTC3",	"(GMT-03:00) <#TZ22#>"],
	["EBST3DST_2",	"(GMT-03:00) <#TZ23#>"],
	["NORO2DST",	"(GMT-02:00) <#TZ24#>"],
	["EUT1DST",	"(GMT-01:00) <#TZ25#>"],
	["UTC1",	"(GMT-01:00) <#TZ26#>"],
	["GMT0DST_1",	"(GMT) <#TZ27#>"],
	["GMT0DST_2",	"(GMT) <#TZ28#>"],
	/*["GMT0DST_3",	"(GMT) <#TZ85#>"],	use adj_dst to desc*/
	["UTC-1DST_1",	"(GMT+01:00) <#TZ29#>"],
	["UTC-1_1_1",	"(GMT+01:00) <#TZ30#>"],
	["UTC-1_2",	"(GMT+01:00) <#TZ31#>"],
	["UTC-1_2_1",	"(GMT+01:00) <#TZ32#>"],
	["MET-1DST",	"(GMT+01:00) <#TZ33#>"],
	["MET-1DST_1",	"(GMT+01:00) <#TZ34#>"],
	["MEZ-1DST",	"(GMT+01:00) <#TZ35#>"],
	["MEZ-1DST_1",	"(GMT+01:00) <#TZ36#>"],
	["UTC-1_3",	"(GMT+01:00) <#TZ37#>"],
	["UTC-2DST",	"(GMT+02:00) <#TZ38#>"],
	["EST-2DST",	"(GMT+02:00) <#TZ39#>"],
	["UTC-2_1",	"(GMT+02:00) <#TZ40#>"],
	["UTC-2DST_2",	"(GMT+02:00) <#TZ41#>"],
	["IST-2DST",	"(GMT+02:00) <#TZ42#>"],
	["SAST-2",	"(GMT+02:00) <#TZ43#>"],
	["EET-2DST",	"(GMT+02:00) <#TZ43_2#>"],
	["UTC-3_1",	"(GMT+03:00) <#TZ46#>"],
	["UTC-3_2",	"(GMT+03:00) <#TZ47#>"],
	["IST-3DST",	"(GMT+03:00) <#TZ48#>"],
	["UTC-3.30DST",	"(GMT+03:30) <#TZ49#>"],
	["UTC-4_2",	"(GMT+04:00) <#TZ44#>"],
	["UTC-4_3",	"(GMT+04:00) <#TZ45#>"],
	["UTC-4_1",	"(GMT+04:00) <#TZ50#>"],
	["UTC-4DST_2",	"(GMT+04:00) <#TZ51#>"],
	["UTC-4.30",	"(GMT+04:30) <#TZ52#>"],
	["UTC-5",	"(GMT+05:00) <#TZ54#>"],
	["UTC-5.30_2",	"(GMT+05:30) <#TZ55#>"],
	["UTC-5.30_1",	"(GMT+05:30) <#TZ56#>"],
	["UTC-5.30",	"(GMT+05:30) <#TZ59#>"],
	["UTC-5.45",	"(GMT+05:45) <#TZ57#>"],
	["RFT-6DST",	"(GMT+06:00) <#TZ60#>"],
	["UTC-6_1",	"(GMT+06:00) <#TZ53#>"],
	["UTC-6",	"(GMT+06:00) <#TZ58#>"],
	["UTC-6.30",	"(GMT+06:30) <#TZ61#>"],
	["UTC-7",	"(GMT+07:00) <#TZ62#>"],
	["CST-8_2",	"(GMT+08:00) <#TZ63#>"],
	["CST-8",	"(GMT+08:00) <#TZ64#>"],
	["CST-8_1",	"(GMT+08:00) <#TZ65#>"],
	["SST-8",	"(GMT+08:00) <#TZ66#>"],
	["CCT-8",	"(GMT+08:00) <#TZ67#>"],
	["WAS-8DST",	"(GMT+08:00) <#TZ68#>"],
	["UTC-8DST",	"(GMT+08:00) <#TZ69#>"],
	["UTC-9_1",	"(GMT+09:00) <#TZ70#>"],
	["JST-9",	"(GMT+09:00) <#TZ71#>"],
	["CST-9.30DST",	"(GMT+09:30) <#TZ73#>"],
	["UTC-9.30",	"(GMT+09:30) <#TZ74#>"],
	["UTC-10_3",	"(GMT+10:00) <#TZ72#>"],
	["UTC-10_1",	"(GMT+10:00) <#TZ75#>"],
	["UTC-10_2",	"(GMT+10:00) <#TZ76#>"],
	["TST-10TDT",	"(GMT+10:00) <#TZ77#>"],
	["UTC-10_5",	"(GMT+10:00) <#TZ79#>"],
	["UTC-11_2",	"(GMT+11:00) <#TZ78#>"],
	["UTC-11",	"(GMT+11:00) <#TZ80#>"],
	["UTC-11_1",	"(GMT+11:00) <#TZ81#>"],
	["UTC-12",	"(GMT+12:00) <#TZ82#>"],
	["NZST-12DST",	"(GMT+12:00) <#TZ83#>"],
	["UTC-13",	"(GMT+13:00) <#TZ84#>"]];

function load_timezones(){
	free_options(document.form.time_zone_select);
	for(var i = 0; i < timezones.length; i++){
		add_tz_option(document.form.time_zone_select,
			timezones[i][1], timezones[i][0],
			(document.form.time_zone.value == timezones[i][0]));
	}
}

var dst_month = new Array("", "M1", "M2", "M3", "M4", "M5", "M6", "M7", "M8", "M9", "M10", "M11", "M12");
var dst_week = new Array("", "1", "2", "3", "4", "5");
var dst_day = new Array("0", "1", "2", "3", "4", "5", "6");
var dst_hour = new Array("0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16", "17", "18", "19", "20", "21", "22", "23");

var dstoff_start_m,dstoff_start_w,dstoff_start_d,dstoff_start_h;
var dstoff_end_m,dstoff_end_w,dstoff_end_d,dstoff_end_h;

function parse_dstoffset(){	//Mm.w.d/h,Mm.w.d/h
	var dstoffset_startend = dstoffset.split(",");
		var dstoffset_start = dstoffset_startend[0];
		var dstoff_start = dstoffset_start.split(".");
			dstoff_start_m = dstoff_start[0];
			dstoff_start_w = dstoff_start[1];
			dstoff_start_d = dstoff_start[2].split("/")[0];
			dstoff_start_h = dstoff_start[2].split("/")[1];
		var dstoffset_end = dstoffset_startend[1];
		var dstoff_end = dstoffset_end.split(".");
			dstoff_end_m = dstoff_end[0];
			dstoff_end_w = dstoff_end[1];
			dstoff_end_d = dstoff_end[2].split("/")[0];
			dstoff_end_h = dstoff_end[2].split("/")[1];
}
															
function load_dst_m_Options(){
	free_options(document.form.dst_start_m);
	free_options(document.form.dst_end_m);
	for(var i = 1; i < dst_month.length; i++){
		if(!dstoffset){		//none dst_offset
			if(i==3){
				add_option(document.form.dst_start_m, dst_month[i], i, 1);
				add_option(document.form.dst_end_m, dst_month[i], i, 0);
			}else if(i==10){
				add_option(document.form.dst_start_m, dst_month[i], i, 0);
				add_option(document.form.dst_end_m, dst_month[i], i, 1);
			}else{
				add_option(document.form.dst_start_m, dst_month[i], i, 0);
				add_option(document.form.dst_end_m, dst_month[i], i, 0);
			}
		}else{
			if(dstoff_start_m =='M'+i)
				add_option(document.form.dst_start_m, dst_month[i], i, 1);
			else	
				add_option(document.form.dst_start_m, dst_month[i], i, 0);
			
			if(dstoff_end_m =='M'+i)
				add_option(document.form.dst_end_m, dst_month[i], i, 1);
			else
				add_option(document.form.dst_end_m, dst_month[i], i, 0);							
		}	
		
	}	
}

function load_dst_w_Options(){
	free_options(document.form.dst_start_w);
	free_options(document.form.dst_end_w);
	for(var i = 1; i < dst_week.length; i++){
		if(!dstoffset){		//none dst_offset
			if(i==2){
				add_option(document.form.dst_start_w, dst_week[i], i, 1);
				add_option(document.form.dst_end_w, dst_week[i], i, 1);
			}else{
				add_option(document.form.dst_start_w, dst_week[i], i, 0);
				add_option(document.form.dst_end_w, dst_week[i], i, 0);
			}
		}else{		
			if(dstoff_start_w == i)
				add_option(document.form.dst_start_w, dst_week[i], i, 1);
			else	
				add_option(document.form.dst_start_w, dst_week[i], i, 0);
			
			if(dstoff_end_w == i)
				add_option(document.form.dst_end_w, dst_week[i], i, 1);
			else
				add_option(document.form.dst_end_w, dst_week[i], i, 0);			
		}		
		
	}	
}

function load_dst_d_Options(){
	free_options(document.form.dst_start_d);
	free_options(document.form.dst_end_d);
	for(var i = 0; i < dst_day.length; i++){
		if(!dstoffset){		//none dst_offset
			if(i==0){
				add_option(document.form.dst_start_d, dst_day[i], i, 1);
				add_option(document.form.dst_end_d, dst_day[i], i, 1);
			}else{
				add_option(document.form.dst_start_d, dst_day[i], i, 0);
				add_option(document.form.dst_end_d, dst_day[i], i, 0);
			}
		}else{
			if(dstoff_start_d == i)
				add_option(document.form.dst_start_d, dst_day[i], i, 1);
			else	
				add_option(document.form.dst_start_d, dst_day[i], i, 0);
			
			if(dstoff_end_d == i)
				add_option(document.form.dst_end_d, dst_day[i], i, 1);
			else
				add_option(document.form.dst_end_d, dst_day[i], i, 0);			
		}
			
	}	
}

function load_dst_h_Options(){
	free_options(document.form.dst_start_h);
	free_options(document.form.dst_end_h);
	for(var i = 0; i < dst_hour.length; i++){
		if(!dstoffset){		//none dst_offset
			if(i==2){
				add_option(document.form.dst_start_h, dst_hour[i], i, 1);
				add_option(document.form.dst_end_h, dst_hour[i], i, 1);
			}else{
				add_option(document.form.dst_start_h, dst_hour[i], i, 0);
				add_option(document.form.dst_end_h, dst_hour[i], i, 0);
			}
		}else{
			if(dstoff_start_h == i)
				add_option(document.form.dst_start_h, dst_hour[i], i, 1);
			else	
				add_option(document.form.dst_start_h, dst_hour[i], i, 0);
			
			if(dstoff_end_h == i)
				add_option(document.form.dst_end_h, dst_hour[i], i, 1);
			else
				add_option(document.form.dst_end_h, dst_hour[i], i, 0);			
		}
			
	}	
}

function add_tz_option(selectObj, str, value, selected){
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
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="current_page" value="Advanced_System_Content.asp">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="action_mode" value=" Apply ">
<input type="hidden" name="action_script" value="restart_time">
<input type="hidden" name="action_wait" value="5">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="time_zone_dst" value="<% nvram_get("time_zone_dst"); %>">
<input type="hidden" name="time_zone" value="<% nvram_get("time_zone"); %>">
<input type="hidden" name="time_zone_dstoff" value="<% nvram_get("time_zone_dstoff"); %>">

<input type="hidden" name="http_passwd" value="<% nvram_get("http_passwd"); %>">

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
		<td valign="top">
<table width="760px" border="0" cellpadding="4" cellspacing="0" class="FormTitle" id="FormTitle">
	<tbody>

	<tr>
  <td bgcolor="#4D595D" valign="top">
		  <div>&nbsp;</div>
		  <div class="formfonttitle"><#menu4#> - <#menu4_2#></div>
		  <div style="margin-left:5px;margin-top:10px;margin-bottom:10px"><img src="/images/New_ui/export/line_export.png"></div>
		  <div id="systemtitle" class="formfontdesc"><#System_title#></div>

	  	<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
	  	<thead>
	  	<tr>
          <td colspan="2"><#System_PASS_changepasswd#></td>
        </tr>
    	</thead>
        <tr>
          <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(11,5)"><#QIS_pass_LoginName#></a></th>
          <td>
		<input type="text" name="http_username" class="input_15_table" maxlength="20" value="<% nvram_get("http_username"); %>">
		<span id="alert_msg1" style="color:#FC0;margin-left:8px;"></span>
          </td>
        </tr>
        <tr>
          <th><a class="hintstyle"  href="javascript:void(0);" onClick="openHint(11,4)"><#System_PASS_new#></a></th>
          <td>
		<input type="password" autocapitalization="off" name="http_passwd2" onKeyPress="return is_string(this);" onkeyup="chkPasswrodStrength(this.value, 'http_passwd');" onBlur="switchType(this, false);" onFocus="switchType(this, true);" class="input_15_table" maxlength="16" />
		<div id="scorebarBorder" style="margin-left:140px; margin-top:-25px; display:none;" title="<#LANHostConfig_x_Password_itemSecur#>">
		  <div id="score"></div>
		  <div id="scorebar">&nbsp;</div>
		</div>
          </td>
        </tr>
        <tr>
          <th><a class="hintstyle"  href="javascript:void(0);" onClick="openHint(11,4)"><#System_PASS_retype#></a></th>
          <td>
		<input type="password" name="v_password2" onKeyPress="return is_string(this);" onBlur="switchType(this, false);" onFocus="switchType(this, true);" class="input_15_table" maxlength="16" />
		<span id="alert_msg2" style="color:#FC0;margin-left:8px;"></span>
          </td>
        </tr>
      </table>
      <table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable" style="margin-top:8px;">
      	<thead>
	  	<tr>
          <td colspan="2"><#System_Misc#></td>
        </tr>
    	</thead>
        <tr id="log_server" display="">
          <th><a class="hintstyle"  href="javascript:void(0);" onClick="openHint(11,1)"><#LANHostConfig_x_ServerLogEnable_itemname#></a></th>
          <td><input type="text" maxlength="15" class="input_15_table" name="log_ipaddr" value="<% nvram_get("log_ipaddr"); %>" onKeyPress="return is_ipaddr(this, event)" onKeyUp="change_ipaddr(this)"></td>
        </tr>
        <tr id="Time_server" display="">
          <th><a class="hintstyle"  href="javascript:void(0);" onClick="openHint(11,2)"><#LANHostConfig_x_TimeZone_itemname#></a></th>
          <td>
            <select name="time_zone_select" class="input_option" onchange="return change_common(this, 'LANHostConfig', 'time_zone_select')">
           		<option value="<% nvram_get("time_zone"); %>" selected><% nvram_get("time_zone"); %></option>
            </select>
          	 </div>
          	<div>
          		<span id="chkbox_time_zone_dst" style="color:white;display:none;">
          			<input type="checkbox" name="time_zone_dst_chk" id="time_zone_dst_chk" <% nvram_match("time_zone_dst", "1", "checked"); %> class="input" onClick="return change_common(this,'LANHostConfig','time_zone_dst_chk')">
				<label for="time_zone_dst_chk"><#TimeZone_DST_itemname#></label>
          			<br>
          		</span>	
          		<span id="dst_start" style="color:white;display:none;">
				<b><#TimeZone_DST_start#></b>
				<select name="dst_start_m" class="input_option" onchange=""></select>&nbsp;<#TimeZone_DST_month#> &nbsp;
				<select name="dst_start_w" class="input_option" onchange=""></select>&nbsp;<#TimeZone_DST_week#> &nbsp;
				<select name="dst_start_d" class="input_option" onchange=""></select>&nbsp;<#TimeZone_DST_weekday#> &nbsp;
				<select name="dst_start_h" class="input_option" onchange=""></select>&nbsp;<#TimeZone_DST_hour#> &nbsp;
          			<br>
          		</span>
          		<span id="dst_end" style="color:white;display:none;">
				<b><#TimeZone_DST_end#></b>
				<select name="dst_end_m" class="input_option" onchange=""></select>&nbsp;<#TimeZone_DST_month#> &nbsp;
				<select name="dst_end_w" class="input_option" onchange=""></select>&nbsp;<#TimeZone_DST_week#> &nbsp;
				<select name="dst_end_d" class="input_option" onchange=""></select>&nbsp;<#TimeZone_DST_weekday#> &nbsp;
				<select name="dst_end_h" class="input_option" onchange=""></select>&nbsp;<#TimeZone_DST_hour#> &nbsp;
          				<br>
          		</span>          			
            	<span id="timezone_hint" style="display:none;"></span>
          	</div>
            </td>
        </tr>
        <tr id="NTP_server" display="" >
          <th><a class="hintstyle"  href="javascript:void(0);" onClick="openHint(11,3)"><#LANHostConfig_x_NTPServer1_itemname#></a></th>
          <td>
						<input type="text" maxlength="256" class="input_32_table" name="ntp_server0" value="<% nvram_get("ntp_server0"); %>" onKeyPress="return is_string(this);">
    	      <a href="javascript:openLink('x_NTPServer1')"  name="x_NTPServer1_link" style=" margin-left:5px; text-decoration: underline;"><#LANHostConfig_x_NTPServer1_linkname#>
					</td>
        </tr>

				<tr>
				  <th><#System_Enable_Telnet#></th>
				  <td>
				    <input type="radio" name="telnetd" class="input" value="1" <% nvram_match("telnetd", "1", "checked"); %>><#checkbox_Yes#>
				    <input type="radio" name="telnetd" class="input" value="0" <% nvram_match("telnetd", "0", "checked"); %>><#checkbox_No#>
				  </td>
				</tr>

      </table>

      <table id="led_ctl" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable" style="margin-top:8px;display:none;">
	  	<thead>
	  	<tr>
          <td colspan="2">LED control</td>
        </tr>
    	</thead>
        <tr>
          <th width="40%">LED brightness</th>
          <td>
            <select name="led_bright" class="input_option">
            	<option value="high" <% nvram_match("led_bright", "high", "selected"); %>>high</option>
            	<option value="dim" <% nvram_match("led_bright", "dim", "selected"); %>>dim</option>
            	<option value="off" <% nvram_match("led_bright", "off", "selected"); %>>off</option>
            </select>
          </td>
        </tr>
        <tr>
          <th width="40%">LED indication</th>
          <td>
            <select name="led_ind" class="input_option">
            	<option value="stable" <% nvram_match("led_ind", "stable", "selected"); %>>stable</option>
            	<option value="flash" <% nvram_match("led_ind", "flash", "selected"); %>>flash</option>
            </select>
          </td>
        </tr>
      </table>

      	<div class="apply_gen">
      		<input name="button" type="button" class="button_gen" onclick="applyRule();" value="<#CTL_apply#>"/>
      	</div>
            
      
      </td></tr>
</tbody>

</table></td>
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
