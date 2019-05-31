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
<title><#Web_Title#> - <#menu7_4#></title>
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
	//addOnlineHelp($("faq"), ["ASUSWRT", "DMZ"]);//Barton tmp mark
	dmz_enable_check();
}

function applyRule(){
	if (validForm()) {
		showLoading();
		document.form.submit();
	}
}

function validForm(){
	if (document.form.dmz_ip.value != "") {
		if (!validate_ipaddr_final(document.form.dmz_ip, 'dmz_ip'))
			return false;
		if (document.form.dmz_enable[1].checked)
			document.form.dmz_ip.value = "";
	} else {
		if (document.form.dmz_enable[0].checked) {
			alert("<#JS_fieldblank#>");
			document.form.dmz_ip.focus();
			return false;
		}
	}
	return true;
}

function dmz_enable_check(){
	if (document.form.dmz_ip.value == "") {
		document.form.dmz_enable[1].checked = "true";
		document.form.dmz_ip.parentNode.parentNode.style.display = "none";
	} else {
		document.form.dmz_enable[0].checked = "true";
		document.form.dmz_ip.parentNode.parentNode.style.display = "";
	}
}

function dmz_on_off(flag){
	if (flag == 1)
		document.form.dmz_ip.parentNode.parentNode.style.display = "";
	else
		document.form.dmz_ip.parentNode.parentNode.style.display = "none";
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
<input type="hidden" name="current_page" value="Advanced_Exposed_Content.asp">
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
	  <div  id="mainMenu"></div>
	  <div  id="subMenu"></div>
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
				  <div class="formfonttitle"><#menu7#> - <#menu7_4#></div>
				  <div style="margin-left:5px;margin-top:10px;margin-bottom:10px"><img src="/images/New_ui/export/line_export.png"></div>
				  <div class="formfontdesc"><#IPConnection_ExposedIP_sectiondesc#><br/><#IPConnection_BattleNet_sectionname#>: <#IPConnection_BattleNet_sectiondesc#></div>
				  <div class="formfontdesc" style="margin-top:-10px;">
					<a id="faq" href="" target="_blank" style="font-family:Lucida Console;text-decoration:underline;">DMZ FAQ</a>
				  </div>

				  <table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
					<tr>
					  <th><#Enable_DMZ#></th>
					  <td>
						<input type="radio" name="dmz_enable" class="input" value="1" onclick="dmz_on_off(1)" ><#checkbox_Yes#>
						<input type="radio" name="dmz_enable" class="input" value="0" onclick="dmz_on_off(0)" ><#checkbox_No#>
					  </td>
					</tr>
					<tr>
					  <th><#IPConnection_ExposedIP_itemname#></th>
					  <td><input type="text" maxlength="15" class="input_15_table" name="dmz_ip" value="<% nvram_get("dmz_ip"); %>" onkeypress="return is_ipaddr(this, event)"/></td>
					</tr>
				  </table>

				  <div class="apply_gen">
					<input name="button" type="button" class="button_gen" onclick="applyRule()" value="<#CTL_apply#>"/>
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
