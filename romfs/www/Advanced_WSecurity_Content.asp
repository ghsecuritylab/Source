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
<title><#Web_Title#> - <#menu2_4#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/detect.js"></script>
<script>
<% wl_get_parameter(); %>

wan_route_x = '<% nvram_get("wan_route_x"); %>';
wan_nat_x = '<% nvram_get("wan_nat_x"); %>';
wan_proto = '<% nvram_get("wan_proto"); %>';

function initial(){
	show_menu();
	show_band_select_option();
	need_action_script(document.form.action_script);
}

function applyRule(){
	if (validForm())
		document.form.submit();
}

function validForm(){
	if(!validate_ipaddr(document.form.wl_radius_ipaddr, 'wl_radius_ipaddr'))
		return false;
	
	if(!validate_range(document.form.wl_radius_port, 0, 65535))
		return false;
	
	if(!validate_string(document.form.wl_radius_key))
		return false;
	
	return true;
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
<input type="hidden" name="current_page" value="Advanced_WSecurity_Content.asp">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="action_mode" value=" Apply ">
<input type="hidden" name="action_script" value="restart_wifi">
<input type="hidden" name="action_wait" value="10">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="wl_subunit" value="-1">

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
		<td valign="top" >
		
<table width="760px" border="0" cellpadding="4" cellspacing="0" class="FormTitle" id="FormTitle" height="343px">
<tbody>
		<tr>
		  <td bgcolor="#4D595D" valign="top"  >
		  <div>&nbsp;</div>
		  <div class="formfonttitle"><#menu2#> - <#menu2_4#></div>
		  <div style="margin-left:5px;margin-top:10px;margin-bottom:10px"><img src="/images/New_ui/export/line_export.png"></div>
		  <div class="formfontdesc"><#WLANAuthentication11a_display1_sectiondesc#></div>
		  
		<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable">
		<tr id="wl_unit_tr" style="display:none;">
			<th><#QIS_frequency#></th>
			<td>
				<select name="wl_unit" class="input_option" onChange="change_wl_unit();">
					<option class="content_input_fd" value="0" <% nvram_match("wl_unit", "0","selected"); %>>2.4GHz</option>
					<option class="content_input_fd" value="1" <% nvram_match("wl_unit", "1","selected"); %>>5GHz</option>
				</select>			
			</td>
		</tr>
		<tr>
			<th>
				<a class="hintstyle" href="javascript:void(0);"  onClick="openHint(2,1);">
			  	<#WLANAuthentication11a_ExAuthDBIPAddr_itemname#></a>			  
			</th>
			<td>
				<input type="text" maxlength="15" class="input_15_table" name="wl_radius_ipaddr" value="<% nvram_get("wl_radius_ipaddr"); %>" onKeyPress="return is_ipaddr(this, event)" onKeyUp="change_ipaddr(this)" onblur="valid_IP_form(this, 0)">
			</td>
		</tr>
		<tr>
			<th>
				<a class="hintstyle" href="javascript:void(0);"  onClick="openHint(2,2);">
			  	<#WLANAuthentication11a_ExAuthDBPortNumber_itemname#></a>
			</th>
			<td>
				<input type="text" maxlength="5" class="input_6_table" name="wl_radius_port" value="<% nvram_get("wl_radius_port"); %>" onkeypress="return is_number(this, event)" onblur="validate_number_range(this, 0, 65535)"/>
			</td>
		</tr>
		<tr>
			<th >
				<a class="hintstyle" href="javascript:void(0);"  onClick="openHint(2,3);">
				<#WLANAuthentication11a_ExAuthDBPassword_itemname#></a>
			</th>
			<td>
				<input type="password" maxlength="64" class="input_32_table" name="wl_radius_key" value="<% nvram_get("wl_radius_key"); %>">
			</td>
		</tr>
		</table>
		
			<div class="apply_gen">
				<!--input type="button" class="button5" value="<#GO_5G#>" onclick="location.href='Advanced_WSecurity_Content.asp';"-->    
				<input class="button_gen" onclick="applyRule()" type="button" value="<#CTL_apply#>"/>
			</div>		
		
		
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
