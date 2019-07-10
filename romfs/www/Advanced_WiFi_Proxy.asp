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
<title><#Web_Title#> - <#menu2_7#></title>
<link rel="stylesheet" type="text/css" href="index_style.css">
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="other.css">
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/detect.js"></script>
<script type="text/javascript" src="/jquery.js"></script>
<script>
var $j = jQuery.noConflict();

function initial(){
	show_menu();

	if (productid == "RP-N53")
		document.getElementById("wifipxy_enable_prompt").style.display = "";
}

function enable_wifipxy(enable){
	document.form.re_wifipxy.value = enable;
}

function applyRule(){
	document.form.submit();
}

</script>
</head>

<body onload="initial();" onunLoad="return unload_body();">
<div id="TopBanner"></div>
<div id="Loading" class="popup_bg"></div>
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

<iframe name="hidden_frame" id="hidden_frame" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="current_page" value="Advanced_WiFi_Proxy.asp">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="action_mode" value=" Apply ">
<input type="hidden" name="action_script" value="restart_wifi">
<input type="hidden" name="action_wait" value="40">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="re_wifipxy" value="<% nvram_get("re_wifipxy"); %>">

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
		  <td align="left" valign="top">
			<table width="760px" border="0" cellpadding="5" cellspacing="0" class="FormTitle" id="FormTitle">
			<tbody>
			<tr>
			  <td bgcolor="#4D595D" valign="top" height="360px">
				<div>&nbsp;</div>
				<div class="formfonttitle"><#menu2#> - <#menu2_7#></div>
				<div style="margin-left:5px;margin-top:10px;margin-bottom:10px"><img src="/images/New_ui/export/line_export.png"></div>
				<div class="formfontdesc"><#WiFiProxy_sectiondesc#></div>

				<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">

				  <tr id="table_proto">
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 25);"><#WiFiProxy_itemname#></a></th>
					<td>
					  <input type="radio" name="wifipxy_enable" class="input" value="1" onclick="enable_wifipxy('1')" <% nvram_match("re_wifipxy", "1", "checked"); %>><#checkbox_Yes#>
					  <input type="radio" name="wifipxy_enable" class="input" value="0" onclick="enable_wifipxy('0')" <% nvram_match("re_wifipxy", "0", "checked"); %>><#checkbox_No#>
					  <span id="wifipxy_enable_prompt" style="display:none;"><#WiFiProxy_prompt#></span>
					</td>
				  </tr>
				</table>

				<div class="apply_gen">
				  <input class="button_gen" onclick="applyRule()" type="button" value="<#CTL_apply#>"/>
				</div>
			  </td>
			</tr>
			</tbody>
			</table>
		  </td>
		</tr>
	  </table>
	  <!--===================================End of Main Content===========================================-->
	</td>
	<td width="10" align="center" valign="top">&nbsp;</td>
  </tr>
</table>
</form>

<div id="footer"></div>
</body>
</html>
