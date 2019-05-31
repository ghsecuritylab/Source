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
<title><#Web_Title#> - <#menu2_3#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script language="JavaScript" type="text/javascript" src="/help.js"></script>
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script language="JavaScript" type="text/javascript" src="/detect.js"></script>
<script type="text/javascript" src="/jquery.js"></script>
<script type="text/javascript" src="/plugin/jquery.iphone-switch.js"></script>
<script>
<% wl_get_parameter(); %>

var $j = jQuery.noConflict();
var wl_macnum_x = '<% nvram_get("wl_macnum_x"); %>';
var wl_maclist_x_array = '<% nvram_get("wl_maclist_x"); %>';

function initial(){
	show_menu();
	show_band_select_option();
	show_wl_maclist_x();
	need_action_script(document.form.action_script);
}

function show_wl_maclist_x(){
	var wl_maclist_x_row = wl_maclist_x_array.split('&#60');
	var code = "";
	code +='<table width="100%" border="1" cellspacing="0" cellpadding="4" align="center" class="list_table"  id="wl_maclist_x_table">'; 
	if(wl_maclist_x_row.length == 1)
		code +='<tr><td style="color:#FFCC00;"><#IPConnection_VSList_Norule#></td>';
	else{
		for(var i = 1; i < wl_maclist_x_row.length; i++){
			code +='<tr id="row'+i+'">';
			code +='<td width="40%">'+ wl_maclist_x_row[i] +'</td>';	
			code +='<td width="20%"><input type="button" class=\"remove_btn\" onclick=\"deleteRow(this);\" value=\"\"/></td></tr>';		
		}
	}	
  	code +='</tr></table>';
	
	$("wl_maclist_x_Block").innerHTML = code;
}


function deleteRow(r){
  var i=r.parentNode.parentNode.rowIndex;
  $('wl_maclist_x_table').deleteRow(i);
  
  var wl_maclist_x_value = "";
	for(i=0; i<$('wl_maclist_x_table').rows.length; i++){
		wl_maclist_x_value += "&#60";
		wl_maclist_x_value += $('wl_maclist_x_table').rows[i].cells[0].innerHTML;
	}
	
	wl_maclist_x_array = wl_maclist_x_value;
	if(wl_maclist_x_array == "")
		show_wl_maclist_x();
}

function addRow(obj, upper){
	var rule_num = $('wl_maclist_x_table').rows.length;
	var item_num = $('wl_maclist_x_table').rows[0].cells.length;

	if(rule_num >= upper){
		alert("<#JS_itemlimit1#> " + upper + " <#JS_itemlimit2#>");
		return false;	
	}	
	
	if(!check_hwaddr(obj))
		return false;		
		
	if(obj.value==""){
		alert("<#JS_fieldblank#>");
		obj.focus();
		obj.select();			
	}else if(check_hwaddr(obj)){
		
		//Viz check same rule
		for(i=0; i<rule_num; i++){
			for(j=0; j<item_num-1; j++){	
				if(obj.value.toLowerCase() == $('wl_maclist_x_table').rows[i].cells[j].innerHTML.toLowerCase()){
					alert("<#JS_duplicate#>");
					return false;
				}	
			}		
		}		
				
		wl_maclist_x_array += "&#60"
		wl_maclist_x_array += obj.value;
		obj.value = ""
		show_wl_maclist_x();
	}else
		return false;			
}

function applyRule(){
	var rule_num = $('wl_maclist_x_table').rows.length;
	var item_num = $('wl_maclist_x_table').rows[0].cells.length;
	var tmp_value = "";

	need_action_wait(document.form.action_script, sw_mode);

	for(i=0; i<rule_num; i++){
		tmp_value += "<"		
		for(j=0; j<item_num-1; j++){	
			tmp_value += $('wl_maclist_x_table').rows[i].cells[j].innerHTML;
			if(j != item_num-2)	
				tmp_value += ">";
		}
	}
	if(tmp_value == "<"+"<#IPConnection_VSList_Norule#>" || tmp_value == "<")
		tmp_value = "";	
		
	if(prevent_lock(tmp_value)){
		document.form.wl_maclist_x.value = tmp_value;
		document.form.submit();	
	}
}

function prevent_lock(rule_num){
	if(document.form.wl_macmode.value == "allow" && rule_num == ""){
		alert("<#FirewallConfig_MFList_accept_hint1#>");
		return false;
	}
	else
		return true;
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
<input type="hidden" name="current_page" value="Advanced_ACL_Content.asp">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="action_mode" value=" Apply ">
<input type="hidden" name="action_script" value="restart_wifi">
<input type="hidden" name="action_wait" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="wl_maclist_x" value="">		
<input type="hidden" name="wl_subunit" value="-1">

<table width="98%" border="0" align="left" cellpadding="0" cellspacing="0">
	<tr>
		<td valign="top" >
		
<table width="760px" border="0" cellpadding="4" cellspacing="0" class="FormTitle" id="FormTitle">
<tbody>
		<tr>
		  <td bgcolor="#4D595D" valign="top">
		  <div>&nbsp;</div>
		  <div class="formfonttitle"><#menu2#> - <#menu2_3#></div>
		  <div style="margin-left:5px;margin-top:10px;margin-bottom:10px"><img src="/images/New_ui/export/line_export.png"></div>
		  <div class="formfontdesc"><#DeviceSecurity11a_display1_sectiondesc#></div>

		<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
			<thead>
			  <tr>
				<td colspan="2"><#LANHostConfig_BasicConfig#></td>
			  </tr>
			</thead>		
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
					<a class="hintstyle" href="javascript:void(0);" onClick="openHint(18,1);"><#FirewallConfig_MFMethod_itemname#></a>
				</th>
				<td>
					<select name="wl_macmode" class="input_option"  onChange="return change_common(this, 'DeviceSecurity11a', 'wl_macmode')">
						<option class="content_input_fd" value="disabled" <% nvram_match("wl_macmode", "disable","selected"); %>><#CTL_Disabled#></option>
						<option class="content_input_fd" value="allow" <% nvram_match("wl_macmode", "allow","selected"); %>><#FirewallConfig_MFMethod_item1#></option>
						<option class="content_input_fd" value="deny" <% nvram_match("wl_macmode", "deny","selected"); %>><#FirewallConfig_MFMethod_item2#></option>
					</select>
				</td>
			</tr>
		</table>
		
			<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable_table">
			  <thead>
			  <tr>
				<td colspan="2"><#FirewallConfig_MFList_groupitemname#></td>
			  </tr>
			  </thead>

          		<tr>
	          		<th width="40%"><a class="hintstyle" href="javascript:void(0);" onClick="openHint(5,10);"><#FirewallConfig_MFList_groupitemname#></th>
				<th width="20%"><#list_add_delete#></th>
          		</tr>
          		<tr>
            		<td width="40%">
              			<input type="text" maxlength="17" class="input_macaddr_table" name="wl_maclist_x_0" onKeyPress="return is_hwaddr(this, event)">
              		</td>
              		<td width="20%">	
              			<input type="button" class="add_btn" onClick="addRow(document.form.wl_maclist_x_0, 32);" value="">
              		</td>
          		</tr>      		
        		</table>
	
			<div id="wl_maclist_x_Block"></div>
		
			<div class="apply_gen">
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
