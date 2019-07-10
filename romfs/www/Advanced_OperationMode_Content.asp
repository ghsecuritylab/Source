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
<title><#Web_Title#> - <#menu4_1#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="other.css">
<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script language="JavaScript" type="text/javascript" src="/jquery.js"></script>
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" language="JavaScript" src="/help.js"></script>
<script type="text/javascript" language="JavaScript" src="/detect.js"></script>
<script>
var $j = jQuery.noConflict();
var re_mediabridge = '<% nvram_get("re_mediabridge"); %>';

function initial(){
	show_menu();
	showOpMode_info();
	Senario_shift();
	setScenerion(sw_mode, re_mediabridge==1 ? "3" : '<% nvram_get("re_expressway"); %>');
}

function Senario_shift(){
	var isIE = navigator.userAgent.search("MSIE") > -1;
	if (isIE)
		$("Senario").style.width = "700px";
		$("Senario").style.marginLeft = "5px";
}

function saveMode(){
	var sw_mode = document.form.sw_mode.value;
	var sub_sw_mode = document.form.sub_sw_mode.value;

	if (sw_mode == '<% nvram_get("sw_mode"); %>' 
			&& ((sub_sw_mode == '<% nvram_get("re_expressway"); %>' && re_mediabridge == 0) 
				|| (sub_sw_mode == 3 && re_mediabridge == 1))) {
		alert("<#model_name#> <#OP_already_configured#>");
		return false;
	}

	if (sw_mode == 1)
		document.form.flag.value = "internet_type";
	else if (sw_mode == 2 || sw_mode == 4 || sw_mode == 5)
		document.form.flag.value = "survey";
	else if (sw_mode == 3)
		document.form.flag.value = "wireless";
	document.form.action = "/QIS_wizard.htm";
	document.form.target = "";
	document.form.submit();
}

function showOpMode_info(){
	var html_code = "";

	if (router_mode_support != -1)
		html_code += '<p><input type="radio" name="sw_mode_radio" class="input" value="1" onclick="setScenerion(1, 0);" <% nvram_match("sw_mode", "1", "checked"); %>><#OP_RT_item#>';
	html_code += '<p><input type="radio" name="sw_mode_radio" class="input" value="2" onclick="setScenerion(2, 0);" <% nvram_match("sw_mode", "2", "checked"); %>><#OP_RE_item#>';
	if (repeater_mode_expressway_support != -1)
		html_code += '<p><input type="radio" name="sw_mode_radio" class="input" value="2" onclick="setScenerion(2, 1);" <% nvram_match("re_expressway", "1", "checked"); %>><#OP_REew_item1#><p><input type="radio" name="sw_mode_radio" class="input" value="2" onclick="setScenerion(2, 2);" <% nvram_match("re_expressway", "2", "checked"); %>><#OP_REew_item2#>';
	if (repeater_mode_mediabridge_support != -1)
		html_code += '<p><input type="radio" name="sw_mode_radio" class="input" value="2" onclick="setScenerion(2, 3);" <% nvram_match("re_mediabridge", "1", "checked"); %>><#OP_REmb_item#>';
	if (ap_mode_support != -1)
		html_code += '<p><input type="radio" name="sw_mode_radio" class="input" value="3" onclick="setScenerion(3, 0);" <% nvram_match("sw_mode", "3", "checked"); %>><#OP_AP_item#>';
	if (ea_mode_support != -1)
		html_code += '<p><input type="radio" name="sw_mode_radio" class="input" value="4" onclick="setScenerion(4, 0);" <% nvram_match("sw_mode", "4", "checked"); %>><#OP_EA_item#>';
	if (hotspot_mode_support != -1)
		html_code += '<p><input type="radio" name="sw_mode_radio" class="input" value="5" onclick="setScenerion(5, 0);" <% nvram_match("sw_mode", "5", "checked"); %>><#OP_HS_item#>';

	document.getElementById("opmode_radio_field").innerHTML = html_code;
}

function setScenerion(mode, submode){
	document.form.sw_mode.value = mode;
	document.form.sub_sw_mode.value = submode;
	if (mode == '1') {
		document.getElementById("opmode_img").src = "/images/qis/RT.gif";
		$j("#mode_desc").html("<#OP_RT_desc#>")
	} else if (mode == '2') {
		document.getElementById("opmode_img").src = "/images/qis/RE.gif";
		if (submode == 0)
			$j("#mode_desc").html("<#OP_RE_desc#>")
		else if (submode == 1)
			$j("#mode_desc").html("<#OP_REew_desc1#>")
		else if (submode == 2)
			$j("#mode_desc").html("<#OP_REew_desc2#>")
		else {
			document.getElementById("opmode_img").src = "/images/qis/REmb.png";
			$j("#mode_desc").html("<#OP_REmb_desc#>")
		}
	} else if (mode == '3') {
		document.getElementById("opmode_img").src = "/images/qis/AP.gif";
		$j("#mode_desc").html("<#OP_AP_desc#>")
	} else if (mode == '4') {
		document.getElementById("opmode_img").src = "/images/qis/EA.gif";
		$j("#mode_desc").html("<#OP_EA_desc#>")
	} else if (mode == '5') {
		document.getElementById("opmode_img").src = "/images/qis/HS.gif";
		$j("#mode_desc").html("<#OP_HS_desc#>")
	}
}
</script>
</head>

<body onload="initial();" onunLoad="return unload_body();">
<div id="TopBanner"></div>
<div id="hiddenMask" class="popup_bg">
	<table cellpadding="5" cellspacing="0" id="dr_sweet_advise" class="dr_sweet_advise" align="center">
		<tr>
		<td>
			<div class="drword" id="drword"><#Main_alert_proceeding_desc4#> <#Main_alert_proceeding_desc1#>...
				<br/>
				<br/>
	    </div>
		  <div class="drImg"><img src="images/alertImg.gif"></div>
			<div style="height:70px;"></div>
		</td>
		</tr>
	</table>
<!--[if lte IE 6.5]><iframe class="hackiframe"></iframe><![endif]-->
</div>

<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="action_mode" value=" Apply ">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="action_wait" value="">
<input type="hidden" name="prev_page" value="Advanced_OperationMode_Content.asp">
<input type="hidden" name="current_page" value="Advanced_OperationMode_Content.asp">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="flag" value="">
<input type="hidden" name="lan_ipaddr" value="<% nvram_get("lan_ipaddr"); %>">
<input type="hidden" name="lan_netmask" value="<% nvram_get("lan_netmask"); %>">
<input type="hidden" name="lan_gateway" value="<% nvram_get("lan_gateway"); %>">
<input type="hidden" name="sw_mode" value="">
<input type="hidden" name="sub_sw_mode" value="">

<table class="content" align="center" cellpadding="0" cellspacing="0">
  <tr>
	<td width="17">&nbsp;</td>
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

			<table width="760px" border="0" cellpadding="5" cellspacing="0" class="FormTitle" id="FormTitle">
			  <tr bgcolor="#4D595D" valign="top" height="100px">
				<td>
				  <div>&nbsp;</div>
				  <div class="formfonttitle"><#menu4#> - <#menu4_1#></div>
				  <div style="margin-left:5px;margin-top:10px;margin-bottom:10px"><img src="/images/New_ui/export/line_export.png"></div>
				  <div class="formfontdesc"><#OP_desc1#></div>
				</td>
			  </tr>

			  <tr>
				<td bgcolor="#4D595D" valign="top" height="300px">
				  <div style="width:95%; margin:0 auto; padding-bottom:3px;">
					<span id="opmode_radio_field" style="font-size:16px;font-weight:bold;color:white;text-shadow:1px 1px 1px black;line-height:28px;"></span>
					<div id="mode_desc" style="position:relative;display:block;margin-top:15px;margin-left:5px;height:60px;z-index:90;"><#OP_RE_desc#></div>
					<br/><br/>
					<div id="Senario"><img id="opmode_img" style="padding-left:60px;" src=""></div>
				  </div>
				  <div class="apply_gen">
					<input name="button" type="button" class="button_gen" onClick="saveMode();" value="<#CTL_onlysave#>">
				  </div>
				</td>
			  </tr>
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
<form name="hint_form"></form>
<div id="footer"></div>
</body>
</html>
