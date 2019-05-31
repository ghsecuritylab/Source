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
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/detect.js"></script>
<script>
function initial(){
	show_menu();
	change_firewall('<% nvram_get("fw_enable_x"); %>');
}

function applyRule(){
	inputRCtrl1(document.form.misc_ping_x, 1);
	showLoading();
	document.form.submit();
}
</script>
</head>

<body onload="initial();">
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
<input type="hidden" name="current_page" value="Advanced_BasicFirewall_Content.asp">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="action_mode" value=" Apply ">
<input type="hidden" name="action_script" value="restart_firewall">
<input type="hidden" name="action_wait" value="5">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">

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

			<table width="760px" border="0" cellpadding="5" cellspacing="0" class="FormTitle" id="FormTitle">
			  <tbody>
			  <tr>
				<td bgcolor="#4D595D" valign="top">
				  <div>&nbsp;</div>
				  <div class="formfonttitle"><#menu8#> - <#menu2_1#></div>
				  <div style="margin-left:5px;margin-top:10px;margin-bottom:10px"><img src="/images/New_ui/export/line_export.png"></div>
				  <div class="formfontdesc"><#FirewallConfig_display2_sectiondesc#></div>

				  <table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
					<tr>
					  <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(8,6);"><#FirewallConfig_FirewallEnable_itemname#></a></th>
					  <td>
						<input type="radio" value="1" name="fw_enable_x"  onClick="return change_common_radio(this, 'FirewallConfig', 'fw_enable_x', '1')" <% nvram_match("fw_enable_x", "1", "checked"); %>><#checkbox_Yes#>
						<input type="radio" value="0" name="fw_enable_x"  onClick="return change_common_radio(this, 'FirewallConfig', 'fw_enable_x', '0')" <% nvram_match("fw_enable_x", "0", "checked"); %>><#checkbox_No#>
					  </td>
					</tr>
					<tr>
					  <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(8,7);"><#FirewallConfig_DoSEnable_itemname#></a></th>
					  <td>
						<input type="radio" value="1" name="fw_dos_x" class="input" onClick="return change_common_radio(this, 'FirewallConfig', 'fw_dos_x', '1')" <% nvram_match("fw_dos_x", "1", "checked"); %>><#checkbox_Yes#>
						<input type="radio" value="0" name="fw_dos_x" class="input" onClick="return change_common_radio(this, 'FirewallConfig', 'fw_dos_x', '0')" <% nvram_match("fw_dos_x", "0", "checked"); %>><#checkbox_No#>
					  </td>
					</tr>
					<tr>
					  <th align="right"><a class="hintstyle" href="javascript:void(0);" onClick="openHint(8,1);"><#FirewallConfig_WanLanLog_itemname#></a></th>
					  <td>
						<select name="fw_log_x" class="input_option" onchange="return change_common(this, 'FirewallConfig', 'fw_log_x')">
						  <option value="none" <% nvram_match("fw_log_x", "none","selected"); %>><#PPPConnection_Authentication_null#></option>
						  <option value="drop" <% nvram_match("fw_log_x", "drop","selected"); %>>Dropped</option>
						  <option value="accept" <% nvram_match("fw_log_x", "accept","selected"); %>>Accepted</option>
						  <option value="both" <% nvram_match("fw_log_x", "both","selected"); %>>Both</option>
						</select>
					  </td>
					</tr>
					<tr>
					  <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(8,5);"><#FirewallConfig_x_WanPingEnable_itemname#></a></th>
					  <td>
						<input type="radio" value="1" name="misc_ping_x" class="input" onClick="return change_common_radio(this, 'FirewallConfig', 'misc_ping_x', '1')" <% nvram_match("misc_ping_x", "1", "checked"); %>><#checkbox_Yes#>
						<input type="radio" value="0" name="misc_ping_x" class="input" onClick="return change_common_radio(this, 'FirewallConfig', 'misc_ping_x', '0')" <% nvram_match("misc_ping_x", "0", "checked"); %>><#checkbox_No#>
					  </td>
					</tr>
				  </table>

				  <div class="apply_gen">
					<input name="button" type="button" class="button_gen" onclick="applyRule();" value="<#CTL_apply#>"/>
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
