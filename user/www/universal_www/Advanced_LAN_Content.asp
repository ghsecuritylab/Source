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
<title><#Web_Title#> - <#menu3_1#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="other.css">
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/detect.js"></script>
<script>
function initial(){
	final_flag = 1;	// for the function in general.js
	show_menu();

	if (sw_mode == 2 || sw_mode == 3) {
		display_lan_dns(<% nvram_get("wan_dnsenable_x"); %>);
		change_ip_setting('<% nvram_get("lan_proto_x"); %>');
	} else {
		$("table_proto").style.display = "none";
		$("table_gateway").style.display = "none";
		$("table_dnsenable").style.display = "none";
		$("table_dns1").style.display = "none";
		$("table_dns2").style.display = "none";
	}
}

function applyRule(){
	need_action_wait(document.form.action_script, sw_mode);
	if (validForm())
		document.form.submit();
}

function validForm(){
	if (sw_mode == 2 || sw_mode == 3) {
		if (document.form.wan_dnsenable_radio[0].checked == 1)
			document.form.wan_dnsenable_x.value = "1";
		else
			document.form.wan_dnsenable_x.value = "0";

		if (document.form.lan_proto_radio[0].checked == 1) {
			document.form.lan_proto_x.value = "1";
			return true;
		} else
			document.form.lan_proto_x.value = "0";
	} else
		document.form.lan_proto_x.value = "0";

	// test if LAN IP is a private IP.
	var A_class_start = inet_network("10.0.0.0");
	var A_class_end = inet_network("10.255.255.255");
	var B_class_start = inet_network("172.16.0.0");
	var B_class_end = inet_network("172.31.255.255");
	var C_class_start = inet_network("192.168.0.0");
	var C_class_end = inet_network("192.168.255.255");
	
	var ip_obj = document.form.lan_ipaddr;
	var ip_num = inet_network(ip_obj.value);
	var ip_class = "";
	
	if(ip_num > A_class_start && ip_num < A_class_end)
		ip_class = 'A';
	else if(ip_num > B_class_start && ip_num < B_class_end)
		ip_class = 'B';
	else if(ip_num > C_class_start && ip_num < C_class_end)
		ip_class = 'C';
	else{
		alert(ip_obj.value+" <#JS_validip#>");
		ip_obj.value = "";
		ip_obj.focus();
		ip_obj.select();
		return false;
	}
	
	// test if netmask is valid.
	var netmask_obj = document.form.lan_netmask;
	var netmask_num = inet_network(netmask_obj.value);
	var netmask_reverse_num = ~netmask_num;
	var default_netmask = "";
	var wrong_netmask = 0;

	if(netmask_num < 0) wrong_netmask = 1;	

	if(ip_class == 'A')
		default_netmask = "255.0.0.0";
	else if(ip_class == 'B')
		default_netmask = "255.255.0.0";
	else
		default_netmask = "255.255.255.0";
	
	var test_num = netmask_reverse_num;
	while(test_num != 0){
		if((test_num+1)%2 == 0)
			test_num = (test_num+1)/2-1;
		else{
			wrong_netmask = 1;
			break;
		}
	}
	if(wrong_netmask == 1){
		alert(netmask_obj.value+" <#JS_validip#>");
		netmask_obj.value = default_netmask;
		netmask_obj.focus();
		netmask_obj.select();
		return false;
	}
	
	var subnet_head = getSubnet(ip_obj.value, netmask_obj.value, "head");
	var subnet_end = getSubnet(ip_obj.value, netmask_obj.value, "end");
	
	if(ip_num == subnet_head || ip_num == subnet_end){
		alert(ip_obj.value+" <#JS_validip#>");
		ip_obj.value = "";
		ip_obj.focus();
		ip_obj.select();
		return false;
	}
	
	// check IP changed or not
  // No matter it changes or not, it will submit the form
	if(sw_mode == "1")
		changed_DHCP_IP_pool();

	return true;
}

// step1. check IP changed. // step2. check Subnet is the same 
var old_lan_ipaddr = "<% nvram_get("lan_ipaddr"); %>";
function changed_DHCP_IP_pool(){
	if(document.form.lan_ipaddr.value != old_lan_ipaddr){ // IP changed
		if(!matchSubnet(document.form.lan_ipaddr.value, document.form.dhcp_start.value, 3) ||
				!matchSubnet(document.form.lan_ipaddr.value, document.form.dhcp_end.value, 3)){ // Different Subnet
				document.form.dhcp_start.value = subnetPrefix(document.form.lan_ipaddr.value, document.form.dhcp_start.value, 3);
				document.form.dhcp_end.value = subnetPrefix(document.form.lan_ipaddr.value, document.form.dhcp_end.value, 3);				
		}
	}
}

function display_lan_dns(flag){
	inputCtrl(document.form.wan_dns1_x, conv_flag(flag));
	inputCtrl(document.form.wan_dns2_x, conv_flag(flag));
}

function change_ip_setting(flag){
	if(flag == "1"){ //dhcp
		inputCtrl(document.form.wan_dnsenable_radio[0], 1);
		inputCtrl(document.form.wan_dnsenable_radio[1], 1);
		flag = 1; 
	}
	else if(flag == "0"){ // static
		document.form.wan_dnsenable_radio[1].checked = 1;
		inputCtrl(document.form.wan_dnsenable_radio[0], 0);
		inputCtrl(document.form.wan_dnsenable_radio[1], 0);
		flag = 0; 
		display_lan_dns(flag)
	}

	inputCtrl(document.form.lan_ipaddr, conv_flag(flag));
	inputCtrl(document.form.lan_netmask, conv_flag(flag));
	inputCtrl(document.form.lan_gateway, conv_flag(flag));
}

function conv_flag(_flag){
	if(_flag == 0)
		_flag = 1;
	else
		_flag = 0;		
	return _flag;
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

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>

<form method="post" name="form" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="current_page" value="Advanced_LAN_Content.asp">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="action_mode" value=" Apply ">
<input type="hidden" name="action_script" value="restart_reboot">
<input type="hidden" name="action_wait" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="dhcp_start" value="<% nvram_get("dhcp_start"); %>">
<input type="hidden" name="dhcp_end" value="<% nvram_get("dhcp_end"); %>">
<input type="hidden" name="lan_proto_x" value="<% nvram_get("lan_proto_x"); %>">
<input type="hidden" name="wan_dnsenable_x" value="<% nvram_get("wan_dnsenable_x"); %>">

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
		  <div class="formfonttitle"><#menu3#> - <#menu3_1#></div>
      <div style="margin-left:5px;margin-top:10px;margin-bottom:10px"><img src="/images/New_ui/export/line_export.png"></div>
      <div class="formfontdesc"><#LANHostConfig_display1_sectiondesc#></div>
		  
		  <table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
		  
			<tr id="table_proto">
			<th><#LANHostConfig_GetLANIP_auto#></th>
			<td>
				<input type="radio" name="lan_proto_radio" class="input" onclick="change_ip_setting('1')" value="1" <% nvram_match("lan_proto_x", "1", "checked"); %>><#checkbox_Yes#>
				<input type="radio" name="lan_proto_radio" class="input" onclick="change_ip_setting('0')" value="0" <% nvram_match("lan_proto_x", "0", "checked"); %>><#checkbox_No#>
			</td>
			</tr>
            
		  <tr>
			<th>
			  <a class="hintstyle" href="javascript:void(0);" onClick="openHint(4,1);"><#LANHostConfig_IPRouters_itemname#></a>
			</th>			
			<td>
			  <input type="text" maxlength="15" class="input_15_table" id="lan_ipaddr" name="lan_ipaddr" value="<% nvram_get("lan_ipaddr"); %>" onKeyPress="return is_ipaddr(this, event);" onKeyUp="change_ipaddr(this);">
			</td>
		  </tr>
		  
		  <tr>
			<th>
			  <a class="hintstyle"  href="javascript:void(0);" onClick="openHint(4,2);"><#LANHostConfig_SubnetMask_itemname#></a>
			</th>
			<td>
				<input type="text" maxlength="15" class="input_15_table" name="lan_netmask" value="<% nvram_get("lan_netmask"); %>" onkeypress="return is_ipaddr(this, event);" onkeyup="change_ipaddr(this);" />
			</td>
		  </tr>

			<tr id="table_gateway">
			<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,3);"><#IPConnection_x_ExternalGateway_itemname#></a></th>
			<td>
				<input type="text" name="lan_gateway" maxlength="15" class="input_15_table" value="<% nvram_get("lan_gateway"); %>" onKeyPress="return is_ipaddr(this, event);" onKeyUp="change_ipaddr(this);" onblur="valid_IP_form(this, 0);">
			</td>
			</tr>

			<tr id="table_dnsenable">
      <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,12);"><#IPConnection_x_DNSServerEnable_itemname#></a></th>
			<td>
				<input type="radio" name="wan_dnsenable_radio" value="1" onclick="display_lan_dns(1)" <% nvram_match("wan_dnsenable_x", "1", "checked"); %> /><#checkbox_Yes#>
			  <input type="radio" name="wan_dnsenable_radio" value="0" onclick="display_lan_dns(0)" <% nvram_match("wan_dnsenable_x", "0", "checked"); %> /><#checkbox_No#>
			</td>
      </tr>          		
          		
      <tr id="table_dns1">
      <th>
				<a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,13);"><#IPConnection_x_DNSServer1_itemname#></a>
			</th>
      <td>
				<input type="text" maxlength="15" class="input_15_table" name="wan_dns1_x" value="<% nvram_get("wan_dns1_x"); %>" onkeypress="return is_ipaddr(this, event)" onkeyup="change_ipaddr(this)"/>
			</td>
      </tr>

      <tr id="table_dns2">
      <th>
				<a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,14);"><#IPConnection_x_DNSServer2_itemname#></a>
			</th>
      <td>
				<input type="text" maxlength="15" class="input_15_table" name="wan_dns2_x" value="<% nvram_get("wan_dns2_x"); %>" onkeypress="return is_ipaddr(this, event)" onkeyup="change_ipaddr(this)"/>
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
	</form>					
				</tr>
			</table>				
			<!--===================================End of Main Content===========================================-->
</td>

    <td width="10" align="center" valign="top">&nbsp;</td>
	</tr>
</table>
<div id="footer"></div>
</body>
</html>
