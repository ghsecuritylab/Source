﻿<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<html xmlns:v>
<head>
<meta http-equiv="X-UA-Compatible" content="IE=EmulateIE7"/>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title><#Web_Title#> - <#menu8_2#></title>
<link rel="stylesheet" type="text/css" href="index_style.css">
<link rel="stylesheet" type="text/css" href="form_style.css">
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/detect.js"></script>
<script>
var url_rulelist_array = "<% nvram_char_to_ascii("url_rulelist"); %>";

function initial(){
	show_menu();
	show_url_rulelist();
}

function show_url_rulelist(){
	var url_rulelist_row = decodeURIComponent(url_rulelist_array).split('<');
	var code = "";
	code += '<table width="100%" border="1" cellspacing="0" cellpadding="4" align="center" class="list_table" id="url_rulelist_table">';
	if (url_rulelist_row.length == 1)
		code += '<tr><td style="color:#FFCC00;"><#IPConnection_VSList_Norule#></td>';
	else {
		for (var i = 1; i < url_rulelist_row.length; i++) {
			code += '<tr id="row'+i+'">';
			code += '<td width="80%">'+ url_rulelist_row[i] +'</td>';//Url keyword
			code += '<td width="20%">';
			code += "<input class=\"remove_btn\" type=\"button\" onclick=\"deleteRow(this);\" value=\"\"/></td>";
		}
	}
	code +='</tr></table>';

	$("url_rulelist_Block").innerHTML = code;
}

function deleteRow(r){
	var i=r.parentNode.parentNode.rowIndex;
	$('url_rulelist_table').deleteRow(i);
	var url_rulelist_value = "";
	for (i=0; i<$('url_rulelist_table').rows.length; i++) {
		url_rulelist_value += "<";
		url_rulelist_value += $('url_rulelist_table').rows[i].cells[0].innerHTML;
	}

	url_rulelist_array = url_rulelist_value;
	if (url_rulelist_array == "")
		show_url_rulelist();
}

function addRow(obj, upper){
	if (validForm(obj)) {
		if ('<% nvram_get("url_enable_x"); %>' != "1")
			document.form.url_enable_x[0].checked = true;

		var rule_num = $('url_rulelist_table').rows.length;
		var item_num = $('url_rulelist_table').rows[0].cells.length;

		//check same rule
		for (i=0; i<rule_num; i++) {
			for (j=0; j<item_num-1; j++) {//only 1 value column
				if (obj.value.toLowerCase() == $('url_rulelist_table').rows[i].cells[j].innerHTML.toLowerCase()) {
					alert("<#JS_duplicate#>");
					return false;
				}
			}
		}

		url_rulelist_array += "<";
		url_rulelist_array += obj.value;
		obj.value = "";
		show_url_rulelist();
	}
}

function applyRule(){
	var rule_num = $('url_rulelist_table').rows.length;
	var item_num = $('url_rulelist_table').rows[0].cells.length;
	var tmp_value = "";

	for (i=0; i<rule_num; i++) {
		tmp_value += "<";
		for (j=0; j<item_num-1; j++) {
			tmp_value += $('url_rulelist_table').rows[i].cells[j].innerHTML;
			if (j != item_num-2)
				tmp_value += ">";
		}
	}
	if (tmp_value == "<"+"<#IPConnection_VSList_Norule#>" || tmp_value == "<")
		tmp_value = "";
	document.form.url_rulelist.value = tmp_value;

	showLoading();
	document.form.submit();
}

function validForm(obj){
	if (obj.value == "") {
		alert("<#JS_fieldblank#>");
		obj.focus();
		obj.select();
		return false;
	} else if (!Block_chars(obj, ["#", "%" ,"&" ,"*" ,"{" ,"}" ,"\\" ,":" ,"<" ,">" ,"?" ,"/" ,"+"]))
		return false;
	return true;
}

function enable_url(){
	return change_common_radio(document.form.url_enable_x, 'FirewallConfig', 'url_enable_x', '1')
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
<input type="hidden" name="current_page" value="Advanced_URLFilter_Content.asp">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="action_mode" value=" Apply ">
<input type="hidden" name="action_script" value="restart_firewall">
<input type="hidden" name="action_wait" value="5">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="url_rulelist" value="<% nvram_get("url_rulelist"); %>">

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
				  <div class="formfonttitle"><#menu8#> - <#menu8_2#></div>
				  <div style="margin-left:5px;margin-top:10px;margin-bottom:10px"><img src="/images/New_ui/export/line_export.png"></div>
				  <div class="formfontdesc"><#FirewallConfig_UrlFilterEnable_sectiondesc#></div>

				  <table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
					<thead>
					<tr>
					  <td colspan="4"><#LANHostConfig_BasicConfig#></td>
					</tr>
					</thead>
					<tr>
					  <th><#FirewallConfig_UrlFilterEnable_itemname#></th>
					  <td>
						<input type="radio" value="1" name="url_enable_x" onClick="enable_url();" <% nvram_match("url_enable_x", "1", "checked"); %>><#CTL_Enabled#>
						<input type="radio" value="0" name="url_enable_x" onClick="enable_url();" <% nvram_match("url_enable_x", "0", "checked"); %>><#CTL_Disabled#>
					  </td>
					</tr>
				  </table>

				  <table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable_table">
					<thead>
					<tr>
					  <td colspan="4"><#FirewallConfig_UrlList_groupitemdesc#>&nbsp;(<#List_limit#>&nbsp;128)</td>
					</tr>
					</thead>
					<tr>
					  <th width="80%"><#FirewallConfig_UrlList_groupitemdesc#></th>
					  <th width="20%">Add / Delete</th>
					</tr>
					<tr>
					  <td width="80%">
						<input type="text" maxlength="32" class="input_32_table" name="url_keyword_x_0" onKeyPress="return is_string(this, event)">
					  </td>
					  <td width="20%">
						<input class="add_btn" type="button" onClick="addRow(document.form.url_keyword_x_0, 128);" value="">
					  </td>
					</tr>
				  </table>

				  <div id="url_rulelist_Block"></div>

				  <div class="apply_gen">
					<input type="button" class="button_gen" onclick="applyRule()" value="<#CTL_apply#>"/>
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
