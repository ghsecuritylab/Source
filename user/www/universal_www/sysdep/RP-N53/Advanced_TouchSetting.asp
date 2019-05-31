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
<title><#Web_Title#> - <#menu4_5#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="other.css">
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/detect.js"></script>
<script>
function initial(){
	show_menu();
	if (band5g_support != -1)
		document.getElementById("touch_ctl_item2_span").style.display="";
	show_touch_ctl();
}

function show_touch_ctl(){
	var ctl = document.form.touch_setting.value.split("");

	if (ctl[0] == 1)
		document.form.touch_ctl_item1.checked = true;
	else
		document.form.touch_ctl_item1.checked = false;
	if (ctl[1] == 1)
		document.form.touch_ctl_item2.checked = true;
	else
		document.form.touch_ctl_item2.checked = false;
	if (ctl[2] == 1)
		document.form.touch_ctl_item3.checked = true;
	else
		document.form.touch_ctl_item3.checked = false;
	if (ctl[3] == 1)
		document.form.touch_ctl_item4.checked = true;
	else
		document.form.touch_ctl_item4.checked = false;
	if (ctl[4] == 1)
		document.form.touch_ctl_item5.checked = true;
	else
		document.form.touch_ctl_item5.checked = false;
}

function change_touch_ctl(){
	if (document.form.touch_ctl_item1.checked == true)
		var W2G_light = "1";
	else
		var W2G_light = "0";
	if (document.form.touch_ctl_item2.checked == true)
		var W5G_light = "1";
	else
		var W5G_light = "0";
	if (document.form.touch_ctl_item3.checked == true)
		var BK_light = "1";
	else
		var BK_light = "0";
	if (document.form.touch_ctl_item4.checked == true)
		var SYS_light = "1";
	else
		var SYS_light = "0";
	if (document.form.touch_ctl_item5.checked == true)
		var AUDIO_out = "1";
	else
		var AUDIO_out = "0";

	document.form.touch_setting.value = W2G_light+W5G_light+BK_light+SYS_light+AUDIO_out;
}

function applyRule(){
	change_touch_ctl();
	document.form.submit();
}

</script>
</head>

<body onload="initial();">
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
			<div style="height:70px; "></div>
		</td>
		</tr>
	</table>
<!--[if lte IE 6.5]><iframe class="hackiframe"></iframe><![endif]-->
</div>

<iframe name="hidden_frame" id="hidden_frame" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" id="touchForm" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="current_page" value="Advanced_TouchSetting.asp">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="action_mode" value=" Apply ">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="action_wait" value="3">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="touch_setting" value="<% nvram_get("touch_setting"); %>">
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
<table width="98%" border="0" align="left" cellpadding="0" cellspacing="0">
	<tr>
		<td align="left" valign="top" >
          
		<table width="760px" border="0" cellpadding="5" cellspacing="0" class="FormTitle" id="FormTitle" height="343px">
		<tbody>
		<tr>
		  <td bgcolor="#4D595D" valign="top" height="100px">
		  <div>&nbsp;</div>
		  <div class="formfonttitle"><#menu4#> - <#menu4_5#></div>
		  <div style="margin-left:5px;margin-top:10px;margin-bottom:10px"><img src="/images/New_ui/export/line_export.png"></div>
		  <div class="formfontdesc"><#Touch_title#></div>
		  </td>
		</tr>
		<tr>
		  <td bgcolor="#4D595D" valign="top">
		  <div id="touch_ctl_item" style="width:95%; margin:0 auto; padding-bottom:3px;">
			<span style="font-size:13px; font-weight:bold;color:white; text-shadow:1px 1px 1px black">
			  <input class="input" type="checkbox" name="touch_ctl_item1"><#Touch_control_item1#><br>
			  <span id="touch_ctl_item2_span" style="display:none;"><input class="input" type="checkbox" name="touch_ctl_item2"><#Touch_control_item2#><br></span>
			  <input class="input" type="checkbox" name="touch_ctl_item3"><#Touch_control_item3#><br>
			  <input class="input" type="checkbox" name="touch_ctl_item4"><#Touch_control_item4#><br>
			  <input class="input" type="checkbox" name="touch_ctl_item5"><#Touch_control_item5#><br>
			<br></span>

		  </div>
		  <div>
			<img name="touch_ctl_img" style="position:relative; margin-top:-125px; margin-left:375px" src="/images/New_ui/RPN53_LED.gif">
		  </div>
		  <div class="apply_gen">
			<input class="button_gen" onclick="applyRule();" type="button" value="<#CTL_apply#>"/>
		  </div>
		  </td>
		</tr>
		</tbody>
		</table>
		</td>
        </tr>
      </table>		
	</td>
		
    <td width="10" align="center" valign="top">&nbsp;</td>
	</tr>
</table>
</form>
<div id="footer"></div>
</body>
</html>
