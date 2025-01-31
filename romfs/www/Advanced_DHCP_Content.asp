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
<title><#Web_Title#> - <#menu3_2#></title>
<link rel="stylesheet" type="text/css" href="index_style.css">
<link rel="stylesheet" type="text/css" href="form_style.css">
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/client_function.js"></script>
<script type="text/javascript" src="/detect.js"></script>
<style>
#ClientList_Block_PC{
	border:1px outset #999;
	background-color:#576D73;
	position:absolute;
	*margin-top:27px;
	margin-left:10px;
	*margin-left:-263px;
	width:255px;
	*width:270px;
	text-align:left;
	height:auto;
	overflow-y:auto;
	z-index:200;
	padding: 1px;
	display:none;
}
#ClientList_Block_PC div{
	background-color:#576D73;
	height:auto;
	*height:20px;
	line-height:20px;
	text-decoration:none;
	font-family: Lucida Console;
	padding-left:2px;
}
#ClientList_Block_PC a{
	background-color:#EFEFEF;
	color:#FFF;
	font-size:12px;
	font-family:Arial, Helvetica, sans-serif;
	text-decoration:none;
}
#ClientList_Block_PC div:hover, #ClientList_Block a:hover{
	background-color:#3366FF;
	color:#FFFFFF;
	cursor:default;
}
</style>
<script>
var arps = [<% get_arp_table(); %>];
var ipmonitor = [<% get_static_client(); %>];
var networkmap_fullscan = '<% nvram_match("networkmap_fullscan", "0", "done"); %>';
var clients_info = getclients();

wan_route_x = '<% nvram_get("wan_route_x"); %>';
var dhcp_staticlist_array = '<% nvram_get("dhcp_staticlist"); %>';

var dhcp_enable = '<% nvram_get("dhcp_enable_x"); %>';
var pool_start = '<% nvram_get("dhcp_start"); %>';
var pool_end = '<% nvram_get("dhcp_end"); %>';
var pool_subnet = pool_start.split(".")[0]+"."+pool_start.split(".")[1]+"."+pool_start.split(".")[2]+".";
var pool_start_end = parseInt(pool_start.split(".")[3]);
var pool_end_end = parseInt(pool_end.split(".")[3]);

var static_enable = '<% nvram_get("dhcp_static_x"); %>';
var dhcp_staticlists = '<% nvram_get("dhcp_staticlist"); %>';
var staticclist_row = dhcp_staticlists.split('&#60');

function initial(){
	show_menu();

	if (wan_route_x == 1) {
		//for LAN ip in DHCP pool or Static list
		showtext($("LANIP"), '<% nvram_get("lan_ipaddr"); %>');
		if ((inet_network(document.form.lan_ipaddr.value) >= inet_network(document.form.dhcp_start.value)) 
				&& (inet_network(document.form.lan_ipaddr.value) <= inet_network(document.form.dhcp_end.value))) {
			$('router_in_pool').style.display = "";
		} else if (dhcp_staticlists != "") {
			for (var i = 1; i < staticclist_row.length; i++) {
				var static_ip = staticclist_row[i].split('&#62')[1];
				if (static_ip == document.form.lan_ipaddr.value)
					$('router_in_pool').style.display="";
			}
		}

		showdhcp_staticlist();
		showLANIPList();
		//addOnlineHelp($("faq"), ["set", "up", "specific", "IP", "address"]);//Barton tmp mark
	} else {
		$("div_desc").innerHTML = "<#LANHostConfig_DHCPServerConfigurable_sectiondesc0#>";
		$("div_faq").style.display = "none";
		$("thead_basic").style.display = "none";
		$("tr_domain").style.display = "none";
		$("tr_start").style.display = "none";
		$("tr_stop").style.display = "none";
		$("tr_lease").style.display = "none";
		$("tr_gw").style.display = "none";
		$("table_dns_wins").style.display = "none";
		$("table_static_enable").style.display = "none";
		$("table_static_list").style.display = "none";
	}
}

function addRow(obj, head){
	if (head == 1)
		dhcp_staticlist_array += "&#60";
	else
		dhcp_staticlist_array += "&#62";
	dhcp_staticlist_array += obj.value;
	obj.value = "";
}

function addRow_Group(upper){
	if (dhcp_enable != "1")
		document.form.dhcp_enable_x[0].checked = true;
	if (static_enable != "1")
		document.form.dhcp_static_x[0].checked = true;

	var rule_num = $('dhcp_staticlist_table').rows.length;
	var item_num = $('dhcp_staticlist_table').rows[0].cells.length;
	if (rule_num >= upper) {
		alert("<#JS_itemlimit1#> " + upper + " <#JS_itemlimit2#>");
		return false;
	}

	if (document.form.dhcp_staticmac_x_0.value == "") {
		alert("<#JS_fieldblank#>");
		document.form.dhcp_staticmac_x_0.focus();
		document.form.dhcp_staticmac_x_0.select();
		return false;
	} else if (document.form.dhcp_staticip_x_0.value == "") {
		alert("<#JS_fieldblank#>");
		document.form.dhcp_staticip_x_0.focus();
		document.form.dhcp_staticip_x_0.select();
		return false;
	} else if (check_macaddr(document.form.dhcp_staticmac_x_0, check_hwaddr_flag(document.form.dhcp_staticmac_x_0)) == true 
			&& valid_IP_form(document.form.dhcp_staticip_x_0,0) == true 
			&& validate_dhcp_range(document.form.dhcp_staticip_x_0) == true) {
		//match(ip or mac) is not accepted
		if (item_num >= 2) {
			for (i=0; i<rule_num; i++) {
				if (document.form.dhcp_staticmac_x_0.value.toLowerCase() == $('dhcp_staticlist_table').rows[i].cells[0].innerHTML.toLowerCase()) {
					alert("<#JS_duplicate#>");
					document.form.dhcp_staticmac_x_0.focus();
					document.form.dhcp_staticmac_x_0.select();
					return false;
				}
				if (document.form.dhcp_staticip_x_0.value == $('dhcp_staticlist_table').rows[i].cells[1].innerHTML) {
					alert("<#JS_duplicate#>");
					document.form.dhcp_staticip_x_0.focus();
					document.form.dhcp_staticip_x_0.select();
					return false;
				}
			}
		}

		addRow(document.form.dhcp_staticmac_x_0 ,1);
		addRow(document.form.dhcp_staticip_x_0, 0);
		showdhcp_staticlist();
	} else
		return false;
}

function del_Row(r){
	var i=r.parentNode.parentNode.rowIndex;
	$('dhcp_staticlist_table').deleteRow(i);

	var dhcp_staticlist_value = "";
	for (k=0; k<$('dhcp_staticlist_table').rows.length; k++) {
		for (j=0; j<$('dhcp_staticlist_table').rows[k].cells.length-1; j++) {
			if (j == 0)
				dhcp_staticlist_value += "&#60";
			else
				dhcp_staticlist_value += "&#62";
			dhcp_staticlist_value += $('dhcp_staticlist_table').rows[k].cells[j].innerHTML;
		}
	}

	dhcp_staticlist_array = dhcp_staticlist_value;
	if (dhcp_staticlist_array == "")
		showdhcp_staticlist();
}

function edit_Row(r){
	var i = r.parentNode.parentNode.rowIndex;
	document.form.dhcp_staticmac_x_0.value = $('dhcp_staticlist_table').rows[i].cells[0].innerHTML;
	document.form.dhcp_staticip_x_0.value = $('dhcp_staticlist_table').rows[i].cells[1].innerHTML;
	del_Row(r);
}

function showdhcp_staticlist(){
	var dhcp_staticlist_row = dhcp_staticlist_array.split('&#60');
	var code = "";

	code += '<table width="100%" cellspacing="0" cellpadding="4" align="center" class="list_table" id="dhcp_staticlist_table">';
	if (dhcp_staticlist_row.length == 1)
		code += '<tr><td style="color:#FFCC00;" colspan="6"><#IPConnection_VSList_Norule#></td></tr>';
	else {
		for (var i = 1; i < dhcp_staticlist_row.length; i++) {
			code += '<tr id="row'+i+'">';
			var dhcp_staticlist_col = dhcp_staticlist_row[i].split('&#62');
			for (var j = 0; j < dhcp_staticlist_col.length; j++)
				code += '<td width="40%">'+ dhcp_staticlist_col[j] +'</td>';//IP width="98"
			code += '<td width="20%">';
			code += '<input class="remove_btn" onclick="del_Row(this);" value=""/></td></tr>';
		}
	}
	code += '</table>';
	$("dhcp_staticlist_Block").innerHTML = code;
}

function applyRule(){
	if (validForm()) {
		if (wan_route_x == 1) {
			var rule_num = $('dhcp_staticlist_table').rows.length;
			var item_num = $('dhcp_staticlist_table').rows[0].cells.length;
			var tmp_value = "";

			for (i=0; i<rule_num; i++) {
				tmp_value += "<";
				for (j=0; j<item_num-1; j++) {
					tmp_value += $('dhcp_staticlist_table').rows[i].cells[j].innerHTML;
					if (j != item_num-2)
						tmp_value += ">";
				}
			}
			if (tmp_value == "<"+"<#IPConnection_VSList_Norule#>" || tmp_value == "<")
				tmp_value = "";

			document.form.dhcp_staticlist.value = tmp_value;
		}
		showLoading();
		document.form.submit();
	}
}

function validate_dhcp_range(ip_obj){
	var ip_num = inet_network(ip_obj.value);
	var subnet_head, subnet_end;

	if (ip_num <= 0) {
		alert(ip_obj.value+" <#JS_validip#>");
		ip_obj.value = "";
		ip_obj.focus();
		ip_obj.select();
		return 0;
	}

	subnet_head = getSubnet(document.form.lan_ipaddr.value, document.form.lan_netmask.value, "head");
	subnet_end = getSubnet(document.form.lan_ipaddr.value, document.form.lan_netmask.value, "end");

	if (ip_num <= subnet_head || ip_num >= subnet_end) {
		alert(ip_obj.value+" <#JS_validip#>");
		ip_obj.value = "";
		ip_obj.focus();
		ip_obj.select();
		return 0;
	}

	return 1;
}

function validForm(){
	if (wan_route_x == 0)
		return true;

	var re = new RegExp('^[a-zA-Z0-9][a-zA-Z0-9\.\-]*[a-zA-Z0-9]$','gi');
	if (!re.test(document.form.lan_domain.value) && document.form.lan_domain.value != "") {
		alert("<#JS_validchar#>");
		document.form.lan_domain.focus();
		document.form.lan_domain.select();
		return false;
	}

	if (!validate_ipaddr_final(document.form.dhcp_gateway_x, 'dhcp_gateway_x') 
			|| !validate_ipaddr_final(document.form.dhcp_dns1_x, 'dhcp_dns1_x') 
			|| !validate_ipaddr_final(document.form.dhcp_wins_x, 'dhcp_wins_x'))
		return false;

	if (!validate_dhcp_range(document.form.dhcp_start) 
			|| !validate_dhcp_range(document.form.dhcp_end))
		return false;

	var dhcp_start_num = inet_network(document.form.dhcp_start.value);
	var dhcp_end_num = inet_network(document.form.dhcp_end.value);

	if (dhcp_start_num > dhcp_end_num) {
		var tmp = document.form.dhcp_start.value;
		document.form.dhcp_start.value = document.form.dhcp_end.value;
		document.form.dhcp_end.value = tmp;
	}

	// check if DHCP pool in default pool
	var default_pool = new Array();
	default_pool = get_default_pool(document.form.lan_ipaddr.value, document.form.lan_netmask.value);
	if ((inet_network(document.form.dhcp_start.value) < inet_network(default_pool[0])) 
			|| (inet_network(document.form.dhcp_end.value) > inet_network(default_pool[1]))) {
		if (confirm("<#JS_DHCP3#>")) {
			document.form.dhcp_start.value=default_pool[0];
			document.form.dhcp_end.value=default_pool[1];
		} else
			return false;
	}

	if (!validate_range(document.form.dhcp_lease, 120, 604800))
		return false;

	return true;
}

// default DHCP pool range
function get_default_pool(ip, netmask){
	// get lan_ipaddr post set .xxx
	z = 0;
	tmp_ip = 0;
	for (i=0 ; i<document.form.lan_ipaddr.value.length ; i++) {
		if (document.form.lan_ipaddr.value.charAt(i) == '.')
			z++;
		if (z == 3) {
			tmp_ip = i+1;
			break;
		}
	}
	post_lan_ipaddr = document.form.lan_ipaddr.value.substr(tmp_ip,3);
	// get lan_netmask post set .xxx
	c = 0;
	tmp_nm = 0;
	for (i=0 ; i<document.form.lan_netmask.value.length ; i++) {
		if (document.form.lan_netmask.value.charAt(i) == '.')
			c++;
		if (c == 3) {
			tmp_nm = i+1;
			break;
		}
	}

	var post_lan_netmask = document.form.lan_netmask.value.substr(tmp_nm,3);
	var nm = new Array("0", "128", "192", "224", "240", "248", "252");
	for (i=0 ; i<nm.length ; i++) {
		 if (post_lan_netmask == nm[i]) {
			gap = 256-Number(nm[i]);
			subnet_set = 256/gap;
			for (j=1 ; j<=subnet_set ; j++) {
				if (post_lan_ipaddr < j*gap) {
					pool_start = (j-1)*gap+1;
					pool_end = j*gap-2;
					break;
				}
			}

			var default_pool_start = subnetPostfix(document.form.dhcp_start.value, pool_start, 3);
			var default_pool_end = subnetPostfix(document.form.dhcp_end.value, pool_end, 3);
			var default_pool = new Array(default_pool_start, default_pool_end);
			return default_pool;
		}
	}
}

function showLANIPList(){
	var code = "";
	var show_name = "";

	for (var i = 0; i < clients_info.length ; i++) {
		if (clients_info[i][0] && clients_info[i][0].length > 12)
			show_name = clients_info[i][0].substring(0, 10) + "..";
		else
			show_name = clients_info[i][0];

		if (clients_info[i][1]) {
			code += '<a><div onmouseover="over_var=1;" onmouseout="over_var=0;" onclick="setClientIP('+i+');"><strong>'+clients_info[i][1]+'</strong> ';
			if (show_name && show_name.length > 0)
				code += '( '+show_name+')';
			code += ' </div></a>';
		}
	}
	code += '<!--[if lte IE 6.5]><iframe class="hackiframe2"></iframe><![endif]-->';
	$("ClientList_Block_PC").innerHTML = code;
}

function setClientIP(num){
	document.form.dhcp_staticmac_x_0.value = clients_info[num][2];
	document.form.dhcp_staticip_x_0.value = clients_info[num][1];
	hideClients_Block();
	over_var = 0;
}

var over_var = 0;
var isMenuopen = 0;

function hideClients_Block(){
	$("pull_arrow").src = "/images/arrow-down.gif";
	$('ClientList_Block_PC').style.display='none';
	isMenuopen = 0;
}

function pullLANIPList(obj){
	if (isMenuopen == 0) {
		obj.src = "/images/arrow-top.gif"
		$("ClientList_Block_PC").style.display = 'block';
		document.form.dhcp_staticmac_x_0.focus();
		isMenuopen = 1;
	} else
		hideClients_Block();
}

function check_macaddr(obj,flag){ //control hint of input mac address
	if (flag == 1) {
		var childsel = document.createElement("div");
		childsel.setAttribute("id","check_mac");
		childsel.style.color = "#FFCC00";
		obj.parentNode.appendChild(childsel);
		$("check_mac").innerHTML = "<#LANHostConfig_ManualDHCPMacaddr_itemdesc#>";
		$("check_mac").style.display = "";
		obj.focus();
		obj.select();
		return false;
	} else if (flag == 2) {
		var childsel = document.createElement("div");
		childsel.setAttribute("id","check_mac");
		childsel.style.color = "#FFCC00";
		obj.parentNode.appendChild(childsel);
		$("check_mac").innerHTML = "<#IPConnection_x_illegal_mac#>";
		$("check_mac").style.display = "";
		obj.focus();
		obj.select();
		return false;
	} else {
		$("check_mac") ? $("check_mac").style.display = "none" : true;
		return true;
	}
}

function showdr_advise(){
	$('drword').innerHTML = "<#DrSurf_sweet_advise6#><br/><br/>";
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
<input type="hidden" name="current_page" value="Advanced_DHCP_Content.asp">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="action_mode" value=" Apply ">
<input type="hidden" name="action_script" value="restart_dhcpd">
<input type="hidden" name="action_wait" value="25">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="lan_ipaddr" value="<% nvram_get("lan_ipaddr"); %>">
<input type="hidden" name="lan_netmask" value="<% nvram_get("lan_netmask"); %>">
<input type="hidden" name="dhcp_staticlist" value="">

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
			<table width="760" border="0" cellpadding="4" cellspacing="0" class="FormTitle" id="FormTitle">
			  <tbody>
			  <tr>
				<td bgcolor="#4D595D" valign="top">
				  <div>&nbsp;</div>
				  <div class="formfonttitle"><#menu3#> - <#menu3_2#></div>
				  <div style="margin-left:5px;margin-top:10px;margin-bottom:10px"><img src="/images/New_ui/export/line_export.png"></div>
				  <div id="div_desc" class="formfontdesc"><#LANHostConfig_DHCPServerConfigurable_sectiondesc1#></div>
				  <div id="router_in_pool" class="formfontdesc" style="color:#FFCC00;display:none;"><#LANHostConfig_DHCPServerConfigurable_sectiondesc2#><span id="LANIP"></span></div>
				  <div id="div_faq" class="formfontdesc" style="margin-top:-10px;">
					<a id="faq" href="" target="_blank" style="font-family:Lucida Console;text-decoration:underline;"><#LANHostConfig_ManualDHCPList_groupitemdesc#>&nbsp;FAQ</a>
				  </div>

				  <table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
					<thead id="thead_basic">
					<tr>
					  <td colspan="2"><#LANHostConfig_BasicConfig#></td>
					</tr>
					</thead>

					<tr>
					  <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(5,1);"><#LANHostConfig_DHCPServerConfigurable_itemname#></a></th>
					  <td>
						<input type="radio" value="1" name="dhcp_enable_x" class="content_input_fd" onClick="return change_common_radio(this, 'LANHostConfig', 'dhcp_enable_x', '1')" <% nvram_match("dhcp_enable_x", "1", "checked"); %>><#checkbox_Yes#>
						<input type="radio" value="0" name="dhcp_enable_x" class="content_input_fd" onClick="return change_common_radio(this, 'LANHostConfig', 'dhcp_enable_x', '0')" <% nvram_match("dhcp_enable_x", "0", "checked"); %>><#checkbox_No#>
					  </td>
					</tr>

					<tr id="tr_domain">
					  <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(5,2);"><#LANHostConfig_DomainName_itemname#></a></th>
					  <td>
						<input type="text" maxlength="32" class="input_25_table" name="lan_domain" value="<% nvram_get("lan_domain"); %>">
					  </td>
					</tr>

					<tr id ="tr_start">
					  <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(5,3);"><#LANHostConfig_MinAddress_itemname#></a></th>
					  <td>
						<input type="text" maxlength="15" class="input_15_table" name="dhcp_start" value="<% nvram_get("dhcp_start"); %>" onKeyPress="return is_ipaddr(this,event);" >
					  </td>
					</tr>

					<tr id="tr_stop">
					  <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(5,4);"><#LANHostConfig_MaxAddress_itemname#></a></th>
					  <td>
						<input type="text" maxlength="15" class="input_15_table" name="dhcp_end" value="<% nvram_get("dhcp_end"); %>" onKeyPress="return is_ipaddr(this,event)" >
					  </td>
					</tr>

					<tr id="tr_lease">
					  <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(5,5);"><#LANHostConfig_LeaseTime_itemname#></a></th>
					  <td>
						<input type="text" maxlength="6" name="dhcp_lease" class="input_15_table" value="<% nvram_get("dhcp_lease"); %>" onKeyPress="return is_number(this,event)">
					  </td>
					</tr>

					<tr id="tr_gw">
					  <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(5,6);"><#LANHostConfig_x_LGateway_itemname#></a></th>
					  <td>
						<input type="text" maxlength="15" class="input_15_table" name="dhcp_gateway_x" value="<% nvram_get("dhcp_gateway_x"); %>" onKeyPress="return is_ipaddr(this,event)">
					  </td>
					</tr>
				  </table>

				  <table id="table_dns_wins" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable" style="margin-top:8px">
					<thead>
					<tr>
					  <td colspan="2"><#LANHostConfig_x_LDNSServer1_sectionname#></td>
					</tr>
					</thead>

					<tr>
					  <th width="200"><a class="hintstyle" href="javascript:void(0);" onClick="openHint(5,7);"><#LANHostConfig_x_LDNSServer1_itemname#></a></th>
					  <td>
						<input type="text" maxlength="15" class="input_15_table" name="dhcp_dns1_x" value="<% nvram_get("dhcp_dns1_x"); %>" onKeyPress="return is_ipaddr(this,event)">
					  </td>
					</tr>

					<tr>
					  <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(5,8);"><#LANHostConfig_x_WINSServer_itemname#></a></th>
					  <td>
						<input type="text" maxlength="15" class="input_15_table" name="dhcp_wins_x" value="<% nvram_get("dhcp_wins_x"); %>" onkeypress="return is_ipaddr(this,event)"/>
					  </td>
					</tr>
				  </table>

				  <table id="table_static_enable" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable" style="margin-top:8px;" >
					<thead>
					<tr>
					  <td colspan="3"><#LANHostConfig_ManualDHCPEnable_itemname#></td>
					</tr>
					</thead>

					<tr>
					  <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(5,9);"><#LANHostConfig_ManualDHCPEnable_itemname#></a></th>
					  <td colspan="2" style="text-align:left;">
						<input type="radio" value="1" name="dhcp_static_x"  onclick="return change_common_radio(this, 'LANHostConfig', 'dhcp_static_x', '1')" <% nvram_match("dhcp_static_x", "1", "checked"); %> /><#checkbox_Yes#>
						<input type="radio" value="0" name="dhcp_static_x"  onclick="return change_common_radio(this, 'LANHostConfig', 'dhcp_static_x', '0')" <% nvram_match("dhcp_static_x", "0", "checked"); %> /><#checkbox_No#>
					  </td>
					</tr>
				  </table>

				  <table id="table_static_list" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable_table" style="margin-top:8px;">
					<thead>
					<tr>
					  <td colspan="3" id="GWStatic"><#LANHostConfig_ManualDHCPList_groupitemdesc#>&nbsp;(<#List_limit#>&nbsp;64)</td>
					</tr>
					</thead>

					<tr>
					  <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(5,10);"><#NM_MAC_Address#></a></th>
					  <th><#IPConnection_ExternalIPAddress_itemname#></th>
					  <th>Add / Delete</th>
					</tr>
					<tr>
					  <!-- client info -->
					  <td width="40%">
						<input type="text" class="input_20_table" maxlength="17" name="dhcp_staticmac_x_0" style="margin-left:-12px;width:255px;" onKeyPress="return is_hwaddr(this,event)" onClick="hideClients_Block();">
						<img id="pull_arrow" height="14px;" src="/images/arrow-down.gif" style="position:absolute;*margin-left:-3px;*margin-top:1px;" onclick="pullLANIPList(this);" title="" onmouseover="over_var=1;" onmouseout="over_var=0;">
						<div id="ClientList_Block_PC" class="ClientList_Block_PC"></div>
					  </td>
					  <td width="40%">
						<input type="text" class="input_15_table" maxlength="15" name="dhcp_staticip_x_0" onkeypress="return is_ipaddr(this,event)">
					  </td>
					  <td width="20%">
						<div>
						  <input type="button" class="add_btn" onClick="addRow_Group(64);" value="">
						</div>
					  </td>
					</tr>
				  </table>

				  <div id="dhcp_staticlist_Block"></div>
				  <!-- manually assigned the DHCP List end-->
				  <div class="apply_gen">
					<input type="button" name="button" class="button_gen" onclick="applyRule();" value="<#CTL_apply#>"/>
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
