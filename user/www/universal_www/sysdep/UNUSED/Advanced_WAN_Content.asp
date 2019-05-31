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
<title><#Web_Title#> - <#menu7_1#></title>
<link rel="stylesheet" type="text/css" href="index_style.css">
<link rel="stylesheet" type="text/css" href="form_style.css">
<style>
.FormTable{
	margin-top:10px;
}
</style>
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/detect.js"></script>
<script>
wan_route_x = '<% nvram_get("wan_route_x"); %>';
wan_nat_x = '<% nvram_get("wan_nat_x"); %>';
wan_proto = '<% nvram_get("wan_proto"); %>';

<% wan_get_parameter(); %>

var original_wan_type = wan_proto;
var original_wan_dhcpenable = parseInt('<% nvram_get("wan_dhcpenable_x"); %>');
var original_dnsenable = parseInt('<% nvram_get("wan_dnsenable_x"); %>');

function initial(){
	show_menu();
	change_wan_type(document.form.wan_proto.value, 0);
	fixed_change_wan_type(document.form.wan_proto.value);
	//addOnlineHelp($("faq"), ["UPnP"]);//Barton tmp mark
	change_wan_type(document.form.wan_proto.value, 0);

	if (document.form.wan_proto.value == "pppoe" 
			|| document.form.wan_proto.value == "pptp" 
			|| document.form.wan_proto.value == "l2tp") {
		document.form.wan_pppoe_username.value = decodeURIComponent('<% nvram_char_to_ascii("wan_pppoe_username"); %>');
		document.form.wan_pppoe_passwd.value = decodeURIComponent('<% nvram_char_to_ascii("wan_pppoe_passwd"); %>');
	}
}

function applyRule(){
	if (validForm()) {
		showLoading();

		inputCtrl(document.form.wan_dhcpenable_x[0], 1);
		inputCtrl(document.form.wan_dhcpenable_x[1], 1);
		if (!document.form.wan_dhcpenable_x[0].checked) {
			inputCtrl(document.form.wan_ipaddr, 1);
			inputCtrl(document.form.wan_netmask, 1);
			inputCtrl(document.form.wan_gateway, 1);
		}

		inputCtrl(document.form.wan_dnsenable_x[0], 1);
		inputCtrl(document.form.wan_dnsenable_x[1], 1);
		if (!document.form.wan_dnsenable_x[0].checked) {
			inputCtrl(document.form.wan_dns1_x, 1);
			inputCtrl(document.form.wan_dns2_x, 1);
		}

		document.form.submit();
	}
}

// test if WAN IP & Gateway & DNS IP is a valid IP
// DNS IP allows to input nothing
function valid_IP(obj_name, obj_flag){
	// A : 1.0.0.0~126.255.255.255
	// B : 127.0.0.0~127.255.255.255 (forbidden)
	// C : 128.0.0.0~255.255.255.254
	var A_class_start = inet_network("1.0.0.0");
	var A_class_end = inet_network("126.255.255.255");
	var B_class_start = inet_network("127.0.0.0");
	var B_class_end = inet_network("127.255.255.255");
	var C_class_start = inet_network("128.0.0.0");
	var C_class_end = inet_network("255.255.255.255");

	var ip_obj = obj_name;
	var ip_num = inet_network(ip_obj.value);

	if (obj_flag == "DNS" && ip_num == -1) //DNS allows to input nothing
		return true;

	if (obj_flag == "GW" && ip_num == -1) //GW allows to input nothing
		return true;

	if (ip_num > A_class_start && ip_num < A_class_end)
		return true;
	else if (ip_num > B_class_start && ip_num < B_class_end) {
		alert(ip_obj.value+" <#JS_validip#>");
		ip_obj.focus();
		ip_obj.select();
		return false;
	} else if (ip_num > C_class_start && ip_num < C_class_end)
		return true;
	else {
		alert(ip_obj.value+" <#JS_validip#>");
		ip_obj.focus();
		ip_obj.select();
		return false;
	}
}

function validForm(){
	if (!document.form.wan_dhcpenable_x[0].checked) {// Set IP address by userself
		if (!valid_IP(document.form.wan_ipaddr, "")) return false;  //WAN IP
		if (!valid_IP(document.form.wan_gateway, "GW"))return false;  //Gateway IP

		if (document.form.wan_gateway.value == document.form.wan_ipaddr.value) {
			document.form.wan_ipaddr.focus();
			alert("<#IPConnection_warning_WANIPEQUALGatewayIP#>");
			return false;
		}

		// test if netmask is valid.
		var default_netmask = "";
		var wrong_netmask = 0;
		var netmask_obj = document.form.wan_netmask;
		var netmask_num = inet_network(netmask_obj.value);

		if (netmask_num == 0)
			var netmask_reverse_num = 0;
		else
			var netmask_reverse_num = ~netmask_num;

		if (netmask_num < 0) wrong_netmask = 1;

		var test_num = netmask_reverse_num;
		while (test_num != 0) {
			if ((test_num+1)%2 == 0)
				test_num = (test_num+1)/2-1;
			else {
				wrong_netmask = 1;
				break;
			}
		}
		if (wrong_netmask == 1) {
			alert(netmask_obj.value+" <#JS_validip#>");
			netmask_obj.value = default_netmask;
			netmask_obj.focus();
			netmask_obj.select();
			return false;
		}
	}

	if (document.form.wan_dnsenable_x[1].checked == true 
			&& document.form.wan_proto.value != "dhcp" 
			&& document.form.wan_dns1_x.value == "" 
			&& document.form.wan_dns2_x.value == "") {
		document.form.wan_dns1_x.focus();
		alert("<#IPConnection_x_DNSServer_blank#>");
		return false;
	}

	if (!document.form.wan_dnsenable_x[0].checked) {
		if (!valid_IP(document.form.wan_dns1_x, "DNS")) return false;  //DNS1
		if (!valid_IP(document.form.wan_dns2_x, "DNS")) return false;  //DNS2
	}

	if (document.form.wan_proto.value == "pppoe" 
			|| document.form.wan_proto.value == "pptp" 
			|| document.form.wan_proto.value == "l2tp") {
		if (!validate_string(document.form.wan_pppoe_username)
				|| !validate_string(document.form.wan_pppoe_passwd))
			return false;

		if (!validate_number_range(document.form.wan_pppoe_idletime, 0, 4294967295))
			return false;
	}

	if (document.form.wan_proto.value == "pppoe") {
		if (!validate_number_range(document.form.wan_pppoe_mtu, 576, 1492)
				|| !validate_number_range(document.form.wan_pppoe_mru, 576, 1492))
			return false;

		if (!validate_string(document.form.wan_pppoe_service)
				|| !validate_string(document.form.wan_pppoe_ac))
			return false;
	}

	if (document.form.wan_hostname.value.length > 0)
		if (!validate_string(document.form.wan_hostname))
			return false;

	if (document.form.wan_hwaddr_x.value.length > 0)
		if(!check_macaddr(document.form.wan_hwaddr_x,check_hwaddr_flag(document.form.wan_hwaddr_x))) {
			document.form.wan_hwaddr_x.select();
			document.form.wan_hwaddr_x.focus();
			return false;
		}

	if (document.form.wan_heartbeat_x.value.length > 0)
		if (!validate_string(document.form.wan_heartbeat_x))
			return false;

	return true;
}

function change_wan_type(wan_type, flag){
	if (typeof(flag) != "undefined")
		change_wan_dhcp_enable(flag);
	else
		change_wan_dhcp_enable(1);

	if (wan_type == "pppoe") {
		inputCtrl(document.form.wan_dnsenable_x[0], 1);
		inputCtrl(document.form.wan_dnsenable_x[1], 1);

		inputCtrl(document.form.wan_auth_x, 0);
		inputCtrl(document.form.wan_pppoe_username, 1);
		$('tr_pppoe_password').style.display = "";
		document.form.wan_pppoe_passwd.disabled = false;
		inputCtrl(document.form.wan_pppoe_idletime, 1);
		inputCtrl(document.form.wan_pppoe_idletime_check, 1);
		inputCtrl(document.form.wan_pppoe_mtu, 1);
		inputCtrl(document.form.wan_pppoe_mru, 1);
		inputCtrl(document.form.wan_pppoe_service, 1);
		inputCtrl(document.form.wan_pppoe_ac, 1);
		inputCtrl(document.form.dhcpc_mode, 0);

		inputCtrl(document.form.wan_pppoe_options_x, 1);
		inputCtrl(document.form.wan_pptp_options_x, 0);
		$("vpn_server").style.display = "none";
		$("vpn_dhcp").style.display = "";
	} else if (wan_type == "pptp") {
		inputCtrl(document.form.wan_dnsenable_x[0], 1);
		inputCtrl(document.form.wan_dnsenable_x[1], 1);

		inputCtrl(document.form.wan_auth_x, 0);
		inputCtrl(document.form.wan_pppoe_username, 1);
		$('tr_pppoe_password').style.display = "";
		document.form.wan_pppoe_passwd.disabled = false;
		inputCtrl(document.form.wan_pppoe_idletime, 1);
		inputCtrl(document.form.wan_pppoe_idletime_check, 1);
		inputCtrl(document.form.wan_pppoe_mtu, 0);
		inputCtrl(document.form.wan_pppoe_mru, 0);
		inputCtrl(document.form.wan_pppoe_service, 0);
		inputCtrl(document.form.wan_pppoe_ac, 0);
		inputCtrl(document.form.dhcpc_mode, 0);

		inputCtrl(document.form.wan_pppoe_options_x, 1);
		inputCtrl(document.form.wan_pptp_options_x, 1);
		$("vpn_server").style.display = "";
		$("vpn_dhcp").style.display = "none";
	} else if (wan_type == "l2tp") {
		inputCtrl(document.form.wan_dnsenable_x[0], 1);
		inputCtrl(document.form.wan_dnsenable_x[1], 1);

		inputCtrl(document.form.wan_auth_x, 0);
		inputCtrl(document.form.wan_pppoe_username, 1);
		$('tr_pppoe_password').style.display = "";
		document.form.wan_pppoe_passwd.disabled = false;
		inputCtrl(document.form.wan_pppoe_idletime, 0);
		inputCtrl(document.form.wan_pppoe_idletime_check, 0);
		inputCtrl(document.form.wan_pppoe_mtu, 0);
		inputCtrl(document.form.wan_pppoe_mru, 0);
		inputCtrl(document.form.wan_pppoe_service, 0);
		inputCtrl(document.form.wan_pppoe_ac, 0);
		inputCtrl(document.form.dhcpc_mode, 0);

		inputCtrl(document.form.wan_pppoe_options_x, 1);
		inputCtrl(document.form.wan_pptp_options_x, 0);
		$("vpn_server").style.display = "";
		$("vpn_dhcp").style.display = "none";
	} else if (wan_type == "static") {
		inputCtrl(document.form.wan_dnsenable_x[0], 0);
		inputCtrl(document.form.wan_dnsenable_x[1], 0);

		inputCtrl(document.form.wan_auth_x, 1);
		inputCtrl(document.form.wan_pppoe_username, (document.form.wan_auth_x.value != ""));
		$('tr_pppoe_password').style.display = (document.form.wan_auth_x.value != "") ? "" : "none";
		document.form.wan_pppoe_passwd.disabled = (document.form.wan_auth_x.value != "") ? false : true;
		inputCtrl(document.form.wan_pppoe_idletime, 0);
		inputCtrl(document.form.wan_pppoe_idletime_check, 0);
		inputCtrl(document.form.wan_pppoe_mtu, 0);
		inputCtrl(document.form.wan_pppoe_mru, 0);
		inputCtrl(document.form.wan_pppoe_service, 0);
		inputCtrl(document.form.wan_pppoe_ac, 0);
		inputCtrl(document.form.dhcpc_mode, 0);

		inputCtrl(document.form.wan_pppoe_options_x, 0);
		inputCtrl(document.form.wan_pptp_options_x, 0);
		$("vpn_server").style.display = "none";
		$("vpn_dhcp").style.display = "none";
	} else {
		inputCtrl(document.form.wan_dnsenable_x[0], 1);
		inputCtrl(document.form.wan_dnsenable_x[1], 1);

		inputCtrl(document.form.wan_auth_x, 1);

		inputCtrl(document.form.wan_pppoe_username, (document.form.wan_auth_x.value != ""));
		$('tr_pppoe_password').style.display = (document.form.wan_auth_x.value != "") ? "" : "none";
		document.form.wan_pppoe_passwd.disabled = (document.form.wan_auth_x.value != "") ? false : true;

		inputCtrl(document.form.wan_pppoe_idletime, 0);
		inputCtrl(document.form.wan_pppoe_idletime_check, 0);
		inputCtrl(document.form.wan_pppoe_mtu, 0);
		inputCtrl(document.form.wan_pppoe_mru, 0);
		inputCtrl(document.form.wan_pppoe_service, 0);
		inputCtrl(document.form.wan_pppoe_ac, 0);
		inputCtrl(document.form.dhcpc_mode, 1);

		inputCtrl(document.form.wan_pppoe_options_x, 0);
		inputCtrl(document.form.wan_pptp_options_x, 0);
		$("vpn_server").style.display = "none";
		$("vpn_dhcp").style.display = "none";
	}
}

function fixed_change_wan_type(wan_type){
	var flag = false;

	if (!document.form.wan_dhcpenable_x[0].checked) {
		if(document.form.wan_ipaddr.value.length == 0)
			document.form.wan_ipaddr.focus();
		else if (document.form.wan_netmask.value.length == 0)
			document.form.wan_netmask.focus();
		else if (document.form.wan_gateway.value.length == 0)
			document.form.wan_gateway.focus();
		else
			flag = true;
	} else
		flag = true;

	if (wan_type == "pppoe") {
		if (wan_type == original_wan_type) {
			document.form.wan_dnsenable_x[0].checked = original_dnsenable;
			document.form.wan_dnsenable_x[1].checked = !original_dnsenable;
			change_common_radio(document.form.wan_dnsenable_x, 'IPConnection', 'wan_dnsenable_x', original_dnsenable);
		} else {
			document.form.wan_dnsenable_x[0].checked = 1;
			document.form.wan_dnsenable_x[1].checked = 0;
			change_common_radio(document.form.wan_dnsenable_x, 'IPConnection', 'wan_dnsenable_x', 0);

			inputCtrl(document.form.wan_dns1_x, 0);
			inputCtrl(document.form.wan_dns2_x, 0);
		}
	} else if(wan_type == "pptp" || wan_type == "l2tp") {
		if (wan_type == original_wan_type) {
			document.form.wan_dnsenable_x[0].checked = original_dnsenable;
			document.form.wan_dnsenable_x[1].checked = !original_dnsenable;
			change_common_radio(document.form.wan_dnsenable_x, 'IPConnection', 'wan_dnsenable_x', original_dnsenable);
		} else {
			document.form.wan_dnsenable_x[0].checked = 0;
			document.form.wan_dnsenable_x[1].checked = 1;
			change_common_radio(document.form.wan_dnsenable_x, 'IPConnection', 'wan_dnsenable_x', 0);

			inputCtrl(document.form.wan_dnsenable_x[0], 1);
			inputCtrl(document.form.wan_dnsenable_x[1], 1);
		}
	} else if(wan_type == "static") {
		document.form.wan_dnsenable_x[0].checked = 0;
		document.form.wan_dnsenable_x[1].checked = 1;
		document.form.wan_dnsenable_x[0].disabled = true;
		change_common_radio(document.form.wan_dnsenable_x, 'IPConnection', 'wan_dnsenable_x', 0);
	} else {
		if (wan_type == original_wan_type) {
			document.form.wan_dnsenable_x[0].checked = original_dnsenable;
			document.form.wan_dnsenable_x[1].checked = !original_dnsenable;
			change_common_radio(document.form.wan_dnsenable_x, 'IPConnection', 'wan_dnsenable_x', original_dnsenable);
		} else {
			document.form.wan_dnsenable_x[0].checked = 1;
			document.form.wan_dnsenable_x[1].checked = 0;
			change_common_radio(document.form.wan_dnsenable_x, 'IPConnection', 'wan_dnsenable_x', 0);

			inputCtrl(document.form.wan_dns1_x, 0);
			inputCtrl(document.form.wan_dns2_x, 0);
		}
	}

	if (wan_type != "static") {// disable DNS server "Yes" when choosing static IP, Jieming add at 2012/12/18
		if (document.form.wan_dhcpenable_x[0].checked) {
			inputCtrl(document.form.wan_dnsenable_x[0], 1);
			inputCtrl(document.form.wan_dnsenable_x[1], 1);
		} else {//wan_dhcpenable_x NO
			document.form.wan_dnsenable_x[0].checked = 0;
			document.form.wan_dnsenable_x[1].checked = 1;

			inputCtrl(document.form.wan_dnsenable_x[0], 1);
			inputCtrl(document.form.wan_dnsenable_x[1], 1);
			document.form.wan_dnsenable_x[0].disabled = true;
		}
	}
}

function change_wan_dhcp_enable(flag){
	var wan_type = document.form.wan_proto.value;

	if (wan_type == "pppoe") {
		if (flag == 1) {
			if (wan_type == original_wan_type) {
				document.form.wan_dhcpenable_x[0].checked = original_wan_dhcpenable;
				document.form.wan_dhcpenable_x[1].checked = !original_wan_dhcpenable;
			} else {
				document.form.wan_dhcpenable_x[0].checked = 1;
				document.form.wan_dhcpenable_x[1].checked = 0;
			}
		}

		$('IPsetting').style.display = "";
		inputCtrl(document.form.wan_dhcpenable_x[0], 1);
		inputCtrl(document.form.wan_dhcpenable_x[1], 1);

		var wan_dhcpenable = document.form.wan_dhcpenable_x[0].checked;

		inputCtrl(document.form.wan_ipaddr, !wan_dhcpenable);
		inputCtrl(document.form.wan_netmask, !wan_dhcpenable);
		inputCtrl(document.form.wan_gateway, !wan_dhcpenable);
	} else if (wan_type == "pptp" || wan_type == "l2tp") {
		if (flag == 1) {
			if (wan_type == original_wan_type) {
				document.form.wan_dhcpenable_x[0].checked = original_wan_dhcpenable;
				document.form.wan_dhcpenable_x[1].checked = !original_wan_dhcpenable;
			} else {
				document.form.wan_dhcpenable_x[0].checked = 0;
				document.form.wan_dhcpenable_x[1].checked = 1;
			}
		}

		$('IPsetting').style.display = "";
		inputCtrl(document.form.wan_dhcpenable_x[0], 1);
		inputCtrl(document.form.wan_dhcpenable_x[1], 1);

		var wan_dhcpenable = document.form.wan_dhcpenable_x[0].checked;

		inputCtrl(document.form.wan_ipaddr, !wan_dhcpenable);
		inputCtrl(document.form.wan_netmask, !wan_dhcpenable);
		inputCtrl(document.form.wan_gateway, !wan_dhcpenable);
	} else if (wan_type == "static") {
		document.form.wan_dhcpenable_x[0].checked = 0;
		document.form.wan_dhcpenable_x[1].checked = 1;

		inputCtrl(document.form.wan_dhcpenable_x[0], 0);
		inputCtrl(document.form.wan_dhcpenable_x[1], 0);

		$('IPsetting').style.display = "";
		inputCtrl(document.form.wan_ipaddr, 1);
		inputCtrl(document.form.wan_netmask, 1);
		inputCtrl(document.form.wan_gateway, 1);
	} else {
		document.form.wan_dhcpenable_x[0].checked = 1;
		document.form.wan_dhcpenable_x[1].checked = 0;

		inputCtrl(document.form.wan_dhcpenable_x[0], 0);
		inputCtrl(document.form.wan_dhcpenable_x[1], 0);

		inputCtrl(document.form.wan_ipaddr, 0);
		inputCtrl(document.form.wan_netmask, 0);
		inputCtrl(document.form.wan_gateway, 0);
		$('IPsetting').style.display = "none";
	}

	if (document.form.wan_dhcpenable_x[0].checked) {
		inputCtrl(document.form.wan_dnsenable_x[0], 1);
		inputCtrl(document.form.wan_dnsenable_x[1], 1);
	} else {//wan_dhcpenable_x NO
		document.form.wan_dnsenable_x[0].checked = 0;
		document.form.wan_dnsenable_x[1].checked = 1;
		change_common_radio(document.form.wan_dnsenable_x, 'IPConnection', 'wan_dnsenable_x', 0);

		inputCtrl(document.form.wan_dnsenable_x[0], 1);
		inputCtrl(document.form.wan_dnsenable_x[1], 1);
		document.form.wan_dnsenable_x[0].disabled = true;
	}
}

function showMAC(){
	document.form.wan_hwaddr_x.value = login_mac_str();
	document.form.wan_hwaddr_x.focus();
}

function check_macaddr(obj,flag){ //control hint of input mac address
	if (flag == 1) {
		var childsel = document.createElement("div");
		childsel.setAttribute("id","check_mac");
		childsel.style.color = "#FFCC00";
		obj.parentNode.appendChild(childsel);
		$("check_mac").innerHTML = "<#LANHostConfig_ManualDHCPMacaddr_itemdesc#>";
		return false;
	} else if (flag == 2) {
		var childsel = document.createElement("div");
		childsel.setAttribute("id","check_mac");
		childsel.style.color = "#FFCC00";
		obj.parentNode.appendChild(childsel);
		$("check_mac").innerHTML = "<#IPConnection_x_illegal_mac#>";
		return false;
	} else {
		$("check_mac") ? $("check_mac").style.display = "none" : true;
		return true;
	}
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
<input type="hidden" name="current_page" value="Advanced_WAN_Content.asp">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="action_mode" value=" Apply ">
<input type="hidden" name="action_script" value="restart_wan">
<input type="hidden" name="action_wait" value="10">
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

	<td height="430" valign="top">
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
				  <div class="formfonttitle"><#menu7#> - <#menu7_1#></div>
				  <div style="margin-left:5px;margin-top:10px;margin-bottom:10px"><img src="/images/New_ui/export/line_export.png"></div>
				  <div class="formfontdesc"><#IPConnection_x_ConnectionType_sectiondesc#></div>

				  <table id="t2BC" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
					<thead>
					<tr>
					  <td colspan="2"><#LANHostConfig_BasicConfig#></td>
					</tr>
					</thead>

					<tr>
					  <th><#PPPConnection_ConnectionType_itemname#></th>
					  <td align="left">
						<select id="wan_proto_menu" class="input_option" name="wan_proto" onchange="change_wan_type(this.value);fixed_change_wan_type(this.value);">
						  <option value="dhcp" <% nvram_match("wan_proto", "dhcp", "selected"); %>><#QIS_internet_type_item1#></option>
						  <option value="pppoe" <% nvram_match("wan_proto", "pppoe", "selected"); %>>PPPoE</option>
						  <option value="static" <% nvram_match("wan_proto", "static", "selected"); %>><#QIS_internet_type_item3#></option>
						  <option value="pptp" <% nvram_match("wan_proto", "pptp", "selected"); %>>PPTP</option>
						  <option value="l2tp" <% nvram_match("wan_proto", "l2tp", "selected"); %>>L2TP</option>
						</select>
					  </td>
					</tr>

					<tr>
					  <th><#Enable_WAN#></th>
					  <td>
						<input type="radio" name="wan_enable" class="input" value="1" <% nvram_match("wan_enable", "1", "checked"); %>><#checkbox_Yes#>
						<input type="radio" name="wan_enable" class="input" value="0" <% nvram_match("wan_enable", "0", "checked"); %>><#checkbox_No#>
					  </td>
					</tr>

					<tr>
					  <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,22);"><#Enable_NAT#></a></th>
					  <td>
						<input type="radio" name="wan_nat_x" class="input" value="1" <% nvram_match("wan_nat_x", "1", "checked"); %>><#checkbox_Yes#>
						<input type="radio" name="wan_nat_x" class="input" value="0" <% nvram_match("wan_nat_x", "0", "checked"); %>><#checkbox_No#>
					  </td>
					</tr>

					<tr>
					  <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,23);"><#Enable_UPnP#></a>&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp<a id="faq" href="" target="_blank" style="font-family:Lucida Console;text-decoration:underline;">UPnP&nbspFAQ</a></th>
					  <td>
						<input type="radio" name="wan_upnp_enable" class="input" value="1" onclick="return change_common_radio(this, 'LANHostConfig', 'wan_upnp_enable', '1')" <% nvram_match("wan_upnp_enable", "1", "checked"); %>><#checkbox_Yes#>
						<input type="radio" name="wan_upnp_enable" class="input" value="0" onclick="return change_common_radio(this, 'LANHostConfig', 'wan_upnp_enable', '0')" <% nvram_match("wan_upnp_enable", "0", "checked"); %>><#checkbox_No#>
					  </td>
					</tr>
				  </table>

				  <table id="IPsetting" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
					<thead>
					<tr>
					  <td colspan="2"><#IPConnection_ExternalIPAddress_sectionname#></td>
					</tr>
					</thead>

					<tr>
					  <th><#IPConnection_x_DHCPClient_itemname#></th>
					  <td>
						<input type="radio" name="wan_dhcpenable_x" class="input" value="1" onclick="change_wan_dhcp_enable(0);" <% nvram_match("wan_dhcpenable_x", "1", "checked"); %>><#checkbox_Yes#>
						<input type="radio" name="wan_dhcpenable_x" class="input" value="0" onclick="change_wan_dhcp_enable(0);" <% nvram_match("wan_dhcpenable_x", "0", "checked"); %>><#checkbox_No#>
					  </td>
					</tr>

					<tr>
					  <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,1);"><#IPConnection_ExternalIPAddress_itemname#></a></th>
					  <td><input type="text" name="wan_ipaddr" maxlength="15" class="input_15_table" value="<% nvram_get("wan_ipaddr"); %>" onKeyPress="return is_ipaddr(this, event);" ></td>
					</tr>

					<tr>
					  <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,2);"><#IPConnection_x_ExternalSubnetMask_itemname#></a></th>
					  <td><input type="text" name="wan_netmask" maxlength="15" class="input_15_table" value="<% nvram_get("wan_netmask"); %>" onKeyPress="return is_ipaddr(this, event);" ></td>
					</tr>

					<tr>
					  <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,3);"><#IPConnection_x_ExternalGateway_itemname#></a></th>
					  <td><input type="text" name="wan_gateway" maxlength="15" class="input_15_table" value="<% nvram_get("wan_gateway"); %>" onKeyPress="return is_ipaddr(this, event);" ></td>
					</tr>
				  </table>

				  <table id="DNSsetting" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
					<thead>
					<tr>
					  <td colspan="2"><#IPConnection_x_DNSServerEnable_sectionname#></td>
					</tr>
					</thead>
					<tr>
					  <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,12);"><#IPConnection_x_DNSServerEnable_itemname#></a></th>
					  <td>
						<input type="radio" name="wan_dnsenable_x" class="input" value="1" onclick="return change_common_radio(this, 'IPConnection', 'wan_dnsenable_x', 1)" <% nvram_match("wan_dnsenable_x", "1", "checked"); %> /><#checkbox_Yes#>
						<input type="radio" name="wan_dnsenable_x" class="input" value="0" onclick="return change_common_radio(this, 'IPConnection', 'wan_dnsenable_x', 0)" <% nvram_match("wan_dnsenable_x", "0", "checked"); %> /><#checkbox_No#>
					  </td>
					</tr>

					<tr>
					  <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,13);"><#IPConnection_x_DNSServer1_itemname#></a></th>
					  <td><input type="text" maxlength="15" class="input_15_table" name="wan_dns1_x" value="<% nvram_get("wan_dns1_x"); %>" onkeypress="return is_ipaddr(this, event)" ></td>
					</tr>
					<tr>
					  <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,14);"><#IPConnection_x_DNSServer2_itemname#></a></th>
					  <td><input type="text" maxlength="15" class="input_15_table" name="wan_dns2_x" value="<% nvram_get("wan_dns2_x"); %>" onkeypress="return is_ipaddr(this, event)" ></td>
					</tr>
				  </table>

				  <table id="PPPsetting" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
					<thead>
					<tr>
					  <td colspan="2"><#PPPConnection_UserName_sectionname#></td>
					</tr>
					</thead>
					<tr>
					  <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,29);"><#PPPConnection_Authentication_itemname#></a></th>
					  <td align="left">
						<select class="input_option" name="wan_auth_x" onChange="change_wan_type(document.form.wan_proto.value);">
						  <option value="" <% nvram_match("wan_auth_x", "", "selected"); %>><#PPPConnection_Authentication_null#></option>
						  <option value="8021x-md5" <% nvram_match("wan_auth_x", "8021x-md5", "selected"); %>>802.1x MD5</option>
						</select>
					  </td>
					</tr>
					<tr>
					  <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,4);"><#PPPConnection_UserName_itemname#></a></th>
					  <td><input type="text" maxlength="64" class="input_32_table" name="wan_pppoe_username" value="<% nvram_get("wan_pppoe_username"); %>" onkeypress="return is_string(this, event)"></td>
					</tr>
					<tr id="tr_pppoe_password">
					  <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,5);"><#PPPConnection_Password_itemname#></a></th>
					  <td>
						<div style="margin-top:2px;"><input type="password" autocapitalization="off" maxlength="64" class="input_32_table" id="wan_pppoe_passwd" name="wan_pppoe_passwd" value="<% nvram_get("wan_pppoe_passwd"); %>" onBlur="switchType(this, false);" onFocus="switchType(this, true);"></div>
					  </td>
					</tr>
					<tr style="display:none">
					  <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,6);"><#PPPConnection_IdleDisconnectTime_itemname#></a></th>
					  <td>
						<input type="text" maxlength="10" class="input_12_table" name="wan_pppoe_idletime" value="<% nvram_get("wan_pppoe_idletime"); %>" onKeyPress="return is_number(this,event);" />
						<input type="checkbox" style="margin-left:30;display:none;" name="wan_pppoe_idletime_check" value="" />
					  </td>
					</tr>
					<tr>
					  <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,7);"><#PPPConnection_x_PPPoEMTU_itemname#></a></th>
					  <td><input type="text" maxlength="5" name="wan_pppoe_mtu" class="input_6_table" value="<% nvram_get("wan_pppoe_mtu"); %>" onKeyPress="return is_number(this,event);"/></td>
					</tr>
					<tr>
					  <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,8);"><#PPPConnection_x_PPPoEMRU_itemname#></a></th>
					  <td><input type="text" maxlength="5" name="wan_pppoe_mru" class="input_6_table" value="<% nvram_get("wan_pppoe_mru"); %>" onKeyPress="return is_number(this,event);"/></td>
					</tr>
					<tr>
					  <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,9);"><#PPPConnection_x_ServiceName_itemname#></a></th>
					  <td><input type="text" maxlength="32" class="input_32_table" name="wan_pppoe_service" value="<% nvram_get("wan_pppoe_service"); %>" onkeypress="return is_string(this, event)"/></td>
					</tr>
					<tr>
					  <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,10);"><#PPPConnection_x_AccessConcentrator_itemname#></a></th>
					  <td><input type="text" maxlength="32" class="input_32_table" name="wan_pppoe_ac" value="<% nvram_get("wan_pppoe_ac"); %>" onkeypress="return is_string(this, event)"/></td>
					</tr>
					<tr>
					  <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,17);"><#PPPConnection_x_PPTPOptions_itemname#></a></th>
					  <td>
						<select name="wan_pptp_options_x" class="input_option">
						  <option value="" <% nvram_match("wan_pptp_options_x", "","selected"); %>><#Auto#></option>
						  <option value="-mppc" <% nvram_match("wan_pptp_options_x", "-mppc","selected"); %>><#No_Encryp#></option>
						  <option value="+mppe-40" <% nvram_match("wan_pptp_options_x", "+mppe-40","selected"); %>>MPPE 40</option>
						  <option value="+mppe-128" <% nvram_match("wan_pptp_options_x", "+mppe-128","selected"); %>>MPPE 128</option>
						</select>
					  </td>
					</tr>
					<tr>
					  <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,18);"><#PPPConnection_x_AdditionalOptions_itemname#></a></th>
					  <td><input type="text" name="wan_pppoe_options_x" value="<% nvram_get("wan_pppoe_options_x"); %>" class="input_32_table" maxlength="255" onKeyPress="return is_string(this, event)" onBlur="validate_string(this)"></td>
					</tr>
				  </table>

				  <table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
					<thead>
					<tr>
					  <td colspan="2"><#PPPConnection_x_HostNameForISP_sectionname#></td>
					</tr>
					</thead>
					<tr id="vpn_server">
					  <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,19);"><#PPPConnection_x_VPN_item#></a></th>
					  <td><input type="text" name="wan_heartbeat_x" class="input_32_table" maxlength="256" value="<% nvram_get("wan_heartbeat_x"); %>" onKeyPress="return is_string(this, event)"></td>
					</tr>
					<tr id="vpn_dhcp">
					  <th><!--a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,);"-->Enable VPN + DHCP Connection<!--/a--></th>
					  <td>
						<input type="radio" name="wan_vpndhcp" class="input" value="1" onclick="return change_common_radio(this, 'IPConnection', 'wan_vpndhcp', 1)" <% nvram_match("wan_vpndhcp", "1", "checked"); %> /><#checkbox_Yes#>
						<input type="radio" name="wan_vpndhcp" class="input" value="0" onclick="return change_common_radio(this, 'IPConnection', 'wan_vpndhcp', 0)" <% nvram_match("wan_vpndhcp", "0", "checked"); %> /><#checkbox_No#>
					  </td>
					</tr>
					<tr>
					  <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,15);"><#PPPConnection_x_HostNameForISP_itemname#></a></th>
					  <td><input type="text" name="wan_hostname" class="input_32_table" maxlength="32" value="<% nvram_get("wan_hostname"); %>" onkeypress="return is_string(this, event)"></td>
					</tr>
					<tr>
					  <th ><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,16);"><#PPPConnection_x_MacAddressForISP_itemname#></a></th>
					  <td>
						<input type="text" name="wan_hwaddr_x" class="input_20_table" maxlength="17" value="<% nvram_get("wan_hwaddr_x"); %>" onKeyPress="return is_hwaddr(this,event)">
						<input type="button" class="button_gen_long" onclick="showMAC();" value="<#QIS_internet_isp_item3#>">
					  </td>
					</tr>

					<tr>
					  <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,30);">DHCP query frequency</a></th>
					  <td>
						<select name="dhcpc_mode" class="input_option">
						  <option value="0" <% nvram_match(" dhcpc_mode", "0","selected"); %>>Normal Mode</option>
						  <option value="1" <% nvram_match(" dhcpc_mode", "1","selected"); %>>Aggressive Mode</option>
						</select>
					  </td>
					</tr>
				  </table>
				  <div class="apply_gen" style="height:auto">
					<input class="button_gen" onclick="applyRule();" type="button" value="<#CTL_apply#>"/>
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
