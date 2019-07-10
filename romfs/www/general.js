﻿var nav;
var change;
var keyPressed;
var wItem;
var ip = "";
var ip_addr = '<!--#echo var="REMOTE_ADDR"-->';
var chanList = 0;
var wep1, wep2, wep3, wep4;
var varload=0;
var final_flag = 0;	// for validate_ipaddr() always return true.

if(navigator.appName.indexOf("Microsoft") != -1){
	nav = false;
	document.onkeydown = MicrosoftEventHandler_KeyDown;
}
else{
	nav = true;
}

function inet_network(ip_str){
	if(!ip_str)
		return -1;
	
	var re = /^(\d+)\.(\d+)\.(\d+)\.(\d+)$/;
	if(re.test(ip_str)){
		var v1 = parseInt(RegExp.$1);
		var v2 = parseInt(RegExp.$2);
		var v3 = parseInt(RegExp.$3);
		var v4 = parseInt(RegExp.$4);
		
		if(v1 < 256 && v2 < 256 && v3 < 256 && v4 < 256)
			return v1*256*256*256+v2*256*256+v3*256+v4;
	}
	
	return -2;
}

function isMask(ip_str){
	if(!ip_str)
		return 0;
	
	var re = /^(\d+)\.(\d+)\.(\d+)\.(\d+)$/;
	if(re.test(ip_str)){
		var v1 = parseInt(RegExp.$1);
		var v2 = parseInt(RegExp.$2);
		var v3 = parseInt(RegExp.$3);
		var v4 = parseInt(RegExp.$4);
		
		if(v4 == 255 || !(v4 == 0 || (is1to0(v4) && v1 == 255 && v2 == 255 && v3 == 255)))
			return -4;
		
		if(!(v3 == 0 || (is1to0(v3) && v1 == 255 && v2 == 255)))
			return -3;
		
		if(!(v2 == 0 || (is1to0(v2) && v1 == 255)))
			return -2;
		
		if(!is1to0(v1))
			return -1;
	}
	
	return 1;
}

function is1to0(num){
	if(typeof(num) != "number")
		return 0;
	
	if(num == 255 || num == 254 || num == 252 || num == 248
			|| num == 240 || num == 224 || num == 192 || num == 128)
		return 1;
	
	return 0;
}

function getSubnet(ip_str, mask_str, flag){
	var ip_num, mask_num;
	var sub_head, sub_end;
	
	if(!ip_str || !mask_str)
		return -1;
	
	if(isMask(mask_str) <= 0)
		return -2;
	
	if(!flag || (flag != "head" && flag != "end"))
		flag = "head";
	
	ip_num = inet_network(ip_str);
	mask_num = inet_network(mask_str);
	
	if(ip_num < 0 || mask_num < 0)
		return -3;
	
	sub_head = ip_num-(ip_num&~mask_num);
	sub_end = sub_head+~mask_num;
	
	if(flag == "head")
		return sub_head;
	else
		return sub_end;
}

function NetscapeEventHandler_KeyDown(e)
{keyPressed = e.which;
if (keyPressed<48 && keyPressed!=16)
{keyPressed = 0;
}
return true;
}

function changeDate()
{pageChanged = 1;
return true;
}
function MicrosoftEventHandler_KeyDown()
{/* if (keyPressed == 13  && event.srcElement.type != 'button' && event.srcElement.type != 'textarea' && event.srcElement.type != 'submit')
return false;
return true;*/
return true;
}

function str2val(v)
{for(i=0; i<v.length; i++)
{if (v.charAt(i) !='0') break;
}
return v.substring(i);
}
function inputRCtrl1(o, flag)
{if (flag == 0)
{o[0].disabled = 1;
o[1].disabled = 1;
}
else
{o[0].disabled = 0;
o[1].disabled = 0;
}
}
function inputRCtrl2(o, flag)
{if (flag == 0)
{o[0].checked = true;
o[1].checked = false;
}
else
{o[0].checked = false;
o[1].checked = true;
}
}

function markGroup(o, s, c, b) {
	return true;
}

function portrange_min(o, v){
	var num = 0;
	var common_index = o.substring(0, v).indexOf(':');
	
	if(common_index == -1)
		num = parseInt(o.substring(0, v));
	else
		num = parseInt(o.substring(0, common_index));
		
	return num;
}
function portrange_max(o, v){
	var num = 0;
	var common_index = o.substring(0, v).indexOf(':');

	if(common_index == -1)
		num = parseInt(o.substring(0, v));
	else
		num = parseInt(o.substring(common_index+1, v+1));
		
	return num;
}
function isBlank(s) {
for(i=0; i<s.length; i++) {
c=s.charAt(i);
if((c!=' ')&&(c!='\n')&&(c!='\t'))return false;}
return true;
}
function numbersonly()
{if (keyPressed>47 && keyPressed<58)
return true;
else
return false;
}
function check_ptl()
{if(keyPressed==38)
return false;
else
return true;
}

function entry_cmp(entry, match, len){  //compare string length function
	
	var j;
	
	if(entry.length < match.length)
		return (1);
	
	for(j=0; j < entry.length && j<len; j++){
		c1 = entry.charCodeAt(j);
		if (j>=match.length) 
			c2=160;
		else 
			c2 = match.charCodeAt(j);
			
		if (c1==160) 
			c1 = 32;
			
		if (c2==160) 
			c2 = 32;
			
		if (c1>c2) 
			return (1);
		else if (c1<c2) 
			return(-1);
	}
	return 0;
}
function validate_duplicate_noalert(o, v, l, off){
	
	for (var i=0; i<o.options.length; i++)
	{		
		if (entry_cmp(o.options[i].text.substring(off).toLowerCase(), v.toLowerCase(), l)==0){
			return false;
		}
	}
	return true;
}

function validate_duplicate(o, v, l, off){
	for(var i = 0; i < o.options.length; i++){		
		if(entry_cmp(o.options[i].text.substring(off).toLowerCase(), v.toLowerCase(), l) == 0){
			alert("<#JS_duplicate#>");
			
			return false;
		}
	}	
	return true;
}

function validate_duplicate2(o, v, l, off){
	var i;
	for (i=0; i<o.options.length; i++)
	{
		if (entry_cmp(o.options[i].text.substring(off).toLowerCase(), v.toLowerCase(), l)==0)
		{
			return false;
		}
	}
	return true;
}

function is_functionButton(e){
//function keycode for Firefox/Opera
	var keyCode = e.keyCode;
	if(e.which == 0) {
		if (keyCode == 0
			|| keyCode == 27 //Esc
			|| keyCode == 35 //end
			|| keyCode == 36 //home
			|| keyCode == 37 //<-
			|| keyCode == 39 //->
			|| keyCode == 45 //Insert
			|| keyCode == 46 //Del
			){
			return true;
		}
	}
	if (       keyCode == 8 	//backspace
		|| keyCode == 9 	//tab
		){
		return true;
	}
	return false;
}

function is_hwaddr(o,event){
	keyPressed = event.keyCode ? event.keyCode : event.which;

	if (is_functionButton(event)){
		return true;
	}

	if((keyPressed>47 && keyPressed<58)||(keyPressed>64 && keyPressed<71)||(keyPressed>96 && keyPressed<103)){	//Hex
		j = 0;
		
		for(i = 0; i < o.value.length; i++){
			if(o.value.charAt(i) == ':'){
				j++;
			}
		}
		
		if(j < 5 && i >= 2){
			if(o.value.charAt(i-2) != ':' && o.value.charAt(i-1) != ':'){
				o.value = o.value+':';
			}
		}
		
		return true;
	}	
	else if(keyPressed == 58 || keyPressed == 13){	//symbol ':' & 'ENTER'
		return true;
	}
	else{
		return false;
	}
}

function validate_hwaddr(o)
{
	if (o.value.length == 0) return true;
	if(o.value != "")
	{
		if(o.value.length == 12)
		{
			for(i=0; i<o.value.length; i++)
			{c=o.value.charAt(i);
			if (!(c>='0'&&c<='9') && !(c>='a'&&c<='f') && !(c>='A'&&c<='F'))
				{alert("<#JS_validmac#>");
				o.value = "";
				o.focus();
				o.select();
				return false;
			}
			}
		return true;
		}
	}
	alert("<#JS_validmac#>");
	o.value = "";
	o.focus();
	o.select();
	return false;
}
function is_string(o) {
if (!nav)
keyPressed = IEKey();
else
keyPressed = NSKey();
if (keyPressed==0)
return true;
else if (keyPressed>=0&&keyPressed<=126)
return true;
alert("<#JS_validchar#>");
return false;
}
function is_string2(o) {
if (!nav)
keyPressed = IEKey();
else
keyPressed = NSKey();
if (keyPressed==0)
return true;
else if ((keyPressed>=48&&keyPressed<=57) ||
(keyPressed>=97&&keyPressed<=122) ||
(keyPressed>=65&&keyPressed<=90) ||
(keyPressed==45)
)
return true;
alert("<#JS_validchar#>");
return false;
}

function validate_ssidchar(ch){
	if(ch >= 32 && ch <= 126)
		return true;
	
	return false;
}

function validate_string_ssid(o){
	var c;	// character code
	
	for(var i = 0; i < o.value.length; ++i){
		c = o.value.charCodeAt(i);
		
		if(!validate_ssidchar(c)){
			alert("<#JS_validSSID1#> "+o.value.charAt(i)+" <#JS_validSSID2#>");
			o.value = "";
			o.focus();
			o.select();
			return false;
		}
	}
	return true;
}

function is_number(o,event){	
	keyPressed = event.keyCode ? event.keyCode : event.which;
	
	if (is_functionButton(event)){
		return true;
	}

	if (keyPressed>47 && keyPressed<58){
		/*if (keyPressed==48 && o.value.length==0){	//single 0
			return false;	
		}*/
		return true;
	}
	else{
		return false;
	}
}
function validate_range(o, min, max) {
	for(i=0; i<o.value.length; i++)
	{
		if (o.value.charAt(i)<'0' || o.value.charAt(i)>'9')
		{
			alert("<#JS_validrange#> " + min + " <#JS_validrange_to#> " + max);
			//o.value = max;
			o.focus();
			o.select();
			return false;
		}
	}
	if(o.value<min || o.value>max) {
		alert("<#JS_validrange#> " + min + " <#JS_validrange_to#> " + max);
		//o.value = max;
		o.focus();
		o.select();
		return false;
	}
	else {
		o.value = str2val(o.value);
		if(o.value=="")
		o.value="0";
		return true;
	}
}

function validate_range_sp(o, min, max) {
	if (o.value.length==0) return true;
	
	if(o.value<min || o.value>max) {
		alert("<#JS_validrange#> " + min + " to " + max + ".");
		o.value = min;
		o.focus();
		o.select();
		return false;
	}
	else {
		o.value = str2val(o.value);
		if(o.value=="") o.value="0";
		return true;
	}
}

function change_ipaddr(o){}

function is_ipaddr(o,event){
	keyPressed = event.keyCode ? event.keyCode : event.which;

	if (is_functionButton(event)){
		return true;
	}

	if((keyPressed > 47 && keyPressed < 58)){
		j = 0;
		
		for(i = 0; i < o.value.length; i++){
			if(o.value.charAt(i) == '.'){
				j++;
			}
		}
		
		if(j < 3 && i >= 3){
			if(o.value.charAt(i-3) != '.' && o.value.charAt(i-2) != '.' && o.value.charAt(i-1) != '.'){
				o.value = o.value+'.';
			}
		}
		
		return true;
	}
	else if(keyPressed == 46){
		j = 0;
		
		for(i = 0; i < o.value.length; i++){
			if(o.value.charAt(i) == '.'){
				j++;
			}
		}
		
		if(o.value.charAt(i-1) == '.' || j == 3){
			return false;
		}
		
		return true;
	}else if(keyPressed == 13){	// 'ENTER'
		return true;
	}
	
	return false;
}

function intoa(ip)
{n=0;
vip=0;
for(i=0;i<ip.length;i++)
{c = ip.charAt(i);
if (c == '.')
{vip = vip * 256 + n;
n = 0;
}
else if(c>='0' && c<='9')
{n = n*10 + (c-'0');
}
}
vip = vip*256 + n;
return(vip);
}

function requireWANIP(v){
	if(v == 'wan_ipaddr_x' || v == 'wan_netmask_x' ||
			v == 'lan_ipaddr' || v == 'lan_netmask' ||
			v == 'lan1_ipaddr' || v == 'lan1_netmask'){
		if(document.form.wan_proto.value == "static")
			return 1;
		else if(document.form.wan_proto.value == "pppoe" && intoa(document.form.wan_ipaddr_x.value))
			return 1;
		else if((document.form.wan_proto.value=="pptp" || document.form.wan_proto.value == "l2tp")
				&& document.form.wan_ipaddr_x.value != '0.0.0.0')
			return 1;
		else
			return 0;
	}
	
	else return 0;
}

function matchSubnet2(ip1, sb1, ip2, sb2){
	var nsb;
	var nsb1 = intoa(sb1.value);
	var nsb2 = intoa(sb2.value);
	//alert("");
	if(nsb1 < nsb2)
		nsb = nsb1;
	else
		nsb = nsb2;
	
	if((intoa(ip1)&nsb) == (intoa(ip2)&nsb))
		return 1;
	else
		return 0;
}

function validate_ipaddr(o, v){
	if(final_flag)
		return true;
	
num = -1;
pos = 0;
if (o.value.length==0)
{if (v=='dhcp_start' || v=='dhcp_end' || v=='wan_ipaddr_x' || v=='dhcp1_start' || v=='dhcp1_end' ||
v=='lan_ipaddr' || v=='lan_netmask' || v=='lan1_ipaddr' || v=='lan1_netmask')
{alert("<#JS_fieldblank#>");
if (v=='wan_ipaddr_x')
{document.form.wan_ipaddr_x.value = "10.1.1.1";
document.form.wan_netmask_x.value = "255.0.0.0";
}
else if (v=='lan_ipaddr')
{document.form.lan_ipaddr.value = "192.168.1.1";
document.form.lan_netmask.value = "255.255.255.0";
}
else if (v=='lan1_ipaddr')
{document.form.lan1_ipaddr.value = "192.168.2.1";
document.form.lan1_netmask.value = "255.255.255.0";
}
else if (v=='lan_netmask') document.form.lan_netmask.value = "255.255.255.0";
else if (v=='lan1_netmask') document.form.lan1_netmask.value = "255.255.255.0";
else if (v=='dhcp_start') document.form.dhcp_start.value = document.form.dhcp_end.value;
else if (v=='dhcp_end') document.form.dhcp_end.value = document.form.dhcp_start.value;
else if (v=='dhcp1_start') document.form.dhcp1_start.value = document.form.dhcp1_end.value;
else if (v=='dhcp1_end') document.form.dhcp1_end.value = document.form.dhcp1_start.value;
o.focus();
o.select();
return false;
}
else return true;
}
if(v=='wan_ipaddr_x' && document.form.wan_netmask_x.value=="")
document.form.wan_netmask_x.value="255.255.255.0";
for(i=0; i<o.value.length; i++) {
c=o.value.charAt(i);
if(c>='0' && c<='9')
{if ( num==-1 )
{num = (c-'0');
}
else
{num = num*10 + (c-'0');
}
}
else
{if ( num<0 || num>255 || c!='.')
{alert(o.value + " <#JS_validip#>");
o.value = "";
o.focus();
o.select();
return false;
}
if (pos==0) v1=num;
else if (pos==1) v2=num;
else if (pos==2) v3=num;
num = -1;
pos++;
}
}
if (pos!=3 || num<0 || num>255)
{alert(o.value + " <#JS_validip#>");
o.value = "";
o.focus();
o.select();
return false;
}
else v4=num;
if (v=='dhcp_start' || v=='dhcp_end' || v=='wan_ipaddr_x' || v=='dhcp1_start' || v=='dhcp1_end' || v=='lan_ipaddr' || v=='lan1_ipaddr' || v=='staticip')
{
	
	//if(v1==255||v2==255||v3==255||v4==255||v1==0||v4==0||v1==127||v1==224)
	if(v != 'wan_ipaddr_x' && (v1 == 255 || v4 == 255 || v1 == 0 || v4 == 0 || v1 == 127 || v1 == 224))
{alert(o.value + " <#JS_validip#>");
o.value = "";
o.focus();
o.select();
return false;
}
if ((v=='wan_ipaddr_x' &&  matchSubnet2(o.value, document.form.wan_netmask_x, document.form.lan_ipaddr.value, document.form.lan_netmask)) ||
(v=='lan_ipaddr' &&  matchSubnet2(o.value, document.form.lan_netmask, document.form.wan_ipaddr_x.value, document.form.wan_netmask_x))
)
{
	alert(o.value + " <#JS_validip#>");
	if (v=='wan_ipaddr_x')
	{document.form.wan_ipaddr_x.value = "10.1.1.1";
	document.form.wan_netmask_x.value = "255.0.0.0";
	}
	else if(v=='lan_ipaddr')
	{document.form.lan_ipaddr.value = "192.168.1.1";
	document.form.lan_netmask.value = "255.255.255.0";
	}
	else if(v=='lan1_ipaddr')
	{document.form.lan1_ipaddr.value = "192.168.2.1";
	document.form.lan1_netmask.value = "255.255.255.0";
	}
	o.focus();
	o.select();
	return false;
}

}
else if(v=='lan_netmask' || v=='lan1_netmask')
{if(v1==255&&v2==255&&v3==255&&v4==255)
{alert(o.value + " <#JS_validip#>");
o.value = "";
o.focus();
o.select();
return false;
}
}
if (requireWANIP(v) && (
(v=='wan_netmask_x' &&  matchSubnet2(document.form.wan_ipaddr_x.value, o, document.form.lan_ipaddr.value, document.form.lan_netmask)) ||
(v=='lan_netmask' &&  matchSubnet2(document.form.lan_ipaddr.value, o, document.form.wan_ipaddr_x.value, document.form.wan_netmask_x))
))
{alert(o.value + " <#JS_validip#>");
if (v=='wan_netmask_x')
{document.form.wan_ipaddr_x.value = "10.1.1.1";
document.form.wan_netmask_x.value = "255.0.0.0";
}
else if(v=='lan_netmask')
{document.form.lan_ipaddr.value = "192.168.1.1";
document.form.lan_netmask.value = "255.255.255.0";
}
else if(v=='lan1_netmask')
{document.form.lan1_ipaddr.value = "192.168.2.1";
document.form.lan1_netmask.value = "255.255.255.0";
}
o.focus();
o.select();
return false;
}
o.value = v1 + "." + v2 + "." + v3 + "." + v4;
if ((v1 > 0) && (v1 < 127)) mask = "255.0.0.0";
else if ((v1 > 127) && (v1 < 192)) mask = "255.255.0.0";
else if ((v1 > 191) && (v1 < 224)) mask = "255.255.255.0";
else mask = "0.0.0.0";
if (v=='wan_ipaddr_x' && document.form.wan_netmask_x.value=="")
{document.form.wan_netmask_x.value = mask;
}
else if (v=='lan_ipaddr' && document.form.lan_netmask.value=="" )
{document.form.lan_netmask.value = mask;
}
else if (v=='dhcp_start')
{if (!matchSubnet(document.form.lan_ipaddr.value, document.form.dhcp_start.value, 3))
{alert(o.value + " <#JS_validip#>");
o.focus();
o.select();
return false;
}
if (intoa(o.value)>intoa(document.form.dhcp_end.value))
{tmp = document.form.dhcp_start.value;
document.form.dhcp_start.value = document.form.dhcp_end.value;
document.form.dhcp_end.value = tmp;
}
}
else if (v=='dhcp_end')
{if (!matchSubnet(document.form.lan_ipaddr.value, document.form.dhcp_end.value, 3))
{alert(o.value + " <#JS_validip#>");
o.focus();
o.select();
return false;
}
if (intoa(document.form.dhcp_start.value)>intoa(o.value))
{tmp = document.form.dhcp_start.value;
document.form.dhcp_start.value = document.form.dhcp_end.value;
document.form.dhcp_end.value = tmp;
}
}
else if (v=='lan1_ipaddr')
{if(document.form.lan1_netmask.value=="" )
document.form.lan1_netmask.value = mask;
}
else if (v=='dhcp1_start')
{if (!matchSubnet(document.form.lan1_ipaddr.value, document.form.dhcp1_start.value, 3))
{alert(o.value + " <#JS_validip#>");
o.focus();
o.select();
return false;
}
if (intoa(o.value)>intoa(document.form.dhcp1_end.value))
{tmp = document.form.dhcp1_start.value;
document.form.dhcp1_start.value = document.form.dhcp1_end.value;
document.form.dhcp1_end.value = tmp;
}
}
else if (v=='dhcp1_end')
{if (!matchSubnet(document.form.lan1_ipaddr.value, document.form.dhcp1_end.value, 3))
{alert(o.value + " <#JS_validip#>");
o.focus();
o.select();
return false;
}
if (intoa(document.form.dhcp1_start.value)>intoa(o.value))
{tmp = document.form.dhcp1_start.value;
document.form.dhcp1_start.value = document.form.dhcp1_end.value;
document.form.dhcp1_end.value = tmp;
}
}
return true;
}

function validate_ipaddr_final(o, v){
	var num = -1;
	var pos = 0;
	
	if(o.value.length == 0){
		if(v == 'dhcp_start' || v == 'dhcp_end' ||
				v == 'wan_ipaddr_x' ||
				v == 'dhcp1_start' || v=='dhcp1_end' ||
				v == 'lan_ipaddr' || v=='lan_netmask' ||
				v=='lan1_ipaddr' || v=='lan1_netmask' ||
				v == 'wl_radius_ipaddr') {	
			alert("<#JS_fieldblank#>");
			
			if(v == 'wan_ipaddr_x'){
				document.form.wan_ipaddr_x.value = "10.1.1.1";
				document.form.wan_netmask_x.value = "255.0.0.0";
			}
			else if(v == 'lan_ipaddr'){
				document.form.lan_ipaddr.value = "192.168.1.1";
				document.form.lan_netmask.value = "255.255.255.0";
			}
			else if(v == 'lan1_ipaddr'){
				document.form.lan1_ipaddr.value = "192.168.2.1";
				document.form.lan1_netmask.value = "255.255.255.0";
			}
			else if(v == 'lan_netmask')
				document.form.lan_netmask.value = "255.255.255.0";
			else if(v == 'lan1_netmask')
				document.form.lan1_netmask.value = "255.255.255.0";
			
			o.focus();
			o.select();
			
			return false;
		}
		else
			return true;
	}
	
	if(v == 'wan_ipaddr_x' && document.form.wan_netmask_x.value == "")
		document.form.wan_netmask_x.value="255.255.255.0";
	
	for(var i = 0; i < o.value.length; ++i){
		var c = o.value.charAt(i);
		
		if(c >= '0' && c <= '9'){
			if(num == -1 ){
				num = (c-'0');
			}
			else{
				num = num*10+(c-'0');
			}
		}
		else{
			if(num < 0 || num > 255 || c != '.'){
				alert(o.value+" <#JS_validip#>");
				
				o.value = "";
				o.focus();
				o.select();
				
				return false;
			}
			
			if(pos == 0)
				v1 = num;
			else if(pos == 1)
				v2 = num;
			else if(pos == 2)
				v3 = num;
			
			num = -1;
			++pos;
		}
	}
	
	if(pos!=3 || num<0 || num>255){
		alert(o.value + " <#JS_validip#>");
		o.value = "";
		o.focus();
		o.select();
		
		return false;
	}
	else
		v4 = num;
	
	if(v == 'dhcp_start' || v == 'dhcp_end' ||
			v == 'wan_ipaddr_x' ||
			v == 'dhcp1_start' || v == 'dhcp1_end' ||
			v == 'lan_ipaddr' || v == 'lan1_ipaddr' ||
			v == 'staticip' || v == 'wl_radius_ipaddr' ||
			v == 'dhcp_dns1_x' || v == 'dhcp_gateway_x' || v == 'dhcp_wins_x'){
		if((v!='wan_ipaddr_x')&& (v1==255||v4==255||v1==0||v4==0||v1==127||v1==224)){
			alert(o.value + " <#JS_validip#>");
			
			o.value = "";
			o.focus();
			o.select();
			
			return false;
		}
		
		if (wan_route_x == "0")	// defined in state.js
			;	// there is no WAN in AP mode, so it wouldn't be compared with the wan ip..., etc.
		else if(requireWANIP(v) && (
				(v=='wan_ipaddr_x' &&  matchSubnet2(o.value, document.form.wan_netmask_x, document.form.lan_ipaddr.value, document.form.lan_netmask)) ||
				(v=='lan_ipaddr' &&  matchSubnet2(o.value, document.form.lan_netmask, document.form.wan_ipaddr_x.value, document.form.wan_netmask_x))
				)){
			alert(o.value + " <#JS_validip#>");
			if(v == 'wan_ipaddr_x'){
				document.form.wan_ipaddr_x.value = "10.1.1.1";
				document.form.wan_netmask_x.value = "255.0.0.0";
			}
			else if(v == 'lan_ipaddr'){
				document.form.lan_ipaddr.value = "192.168.1.1";
				document.form.lan_netmask.value = "255.255.255.0";
			}
			else if(v == 'lan1_ipaddr'){
				document.form.lan1_ipaddr.value = "192.168.2.1";
				document.form.lan1_netmask.value = "255.255.255.0";
			}
			
			o.focus();
			o.select();
			
			return false;
		}
	}
	else if(v=='lan_netmask' || v=='lan1_netmask'){
		if(v1==255&&v2==255&&v3==255&&v4==255){
			alert(o.value + " <#JS_validip#>");
			o.value = "";
			o.focus();
			o.select();
			return false;
		}
	}
	
	if (wan_route_x == "0")	// defined in state.js
			;	// there is no WAN in AP mode, so it wouldn't be compared with the wan ip..., etc.
	else if(requireWANIP(v) && (
			(v=='wan_netmask_x' &&  matchSubnet2(document.form.wan_ipaddr_x.value, o, document.form.lan_ipaddr.value, document.form.lan_netmask)) ||
			(v=='lan_netmask' &&  matchSubnet2(document.form.lan_ipaddr.value, o, document.form.wan_ipaddr_x.value, document.form.wan_netmask_x))
			)){
		alert(o.value + " <#JS_validip#>");
		
		if (v=='wan_netmask_x'){
			document.form.wan_ipaddr_x.value = "10.1.1.1";
			document.form.wan_netmask_x.value = "255.0.0.0";
		}
		else if(v=='lan_netmask'){
			document.form.lan_ipaddr.value = "192.168.1.1";
			document.form.lan_netmask.value = "255.255.255.0";
		}
		else if(v=='lan1_netmask'){
			document.form.lan1_ipaddr.value = "192.168.2.1";
			document.form.lan1_netmask.value = "255.255.255.0";
		}
		
		o.focus();
		o.select();
		return false;
	}
	
	o.value = v1 + "." + v2 + "." + v3 + "." + v4;
	
	if((v1 > 0) && (v1 < 127)) mask = "255.0.0.0";
	else if ((v1 > 127) && (v1 < 192)) mask = "255.255.0.0";
	else if ((v1 > 191) && (v1 < 224)) mask = "255.255.255.0";
	else mask = "0.0.0.0";
	
	if(v=='wan_ipaddr_x' && document.form.wan_netmask_x.value==""){
		document.form.wan_netmask_x.value = mask;
	}
	else if (v=='lan_ipaddr' && document.form.lan_netmask.value=="" ){
		document.form.lan_netmask.value = mask;
	}else if (v=='dhcp_start'){
		if (!matchSubnet(document.form.lan_ipaddr.value, document.form.dhcp_start.value, 3)){
			alert(o.value + " <#JS_validip#>");
			o.focus();
			o.select();
			return false;
		}
	}
	else if (v=='dhcp_end'){
		if (!matchSubnet(document.form.lan_ipaddr.value, document.form.dhcp_end.value, 3)){
			alert(o.value + " <#JS_validip#>");
			o.focus();
			o.select();
			return false;
		}
	}
	else if (v=='lan1_ipaddr'){
		if(document.form.lan1_netmask.value=="") document.form.lan1_netmask.value = mask;
	}
	else if (v=='dhcp1_start'){
		if (!matchSubnet(document.form.lan1_ipaddr.value, document.form.dhcp1_start.value, 3)){
			alert(o.value + " <#JS_validip#>");
			o.focus();
			o.select();
			return false;
		}
	}
	else if (v=='dhcp1_end'){
		if (!matchSubnet(document.form.lan1_ipaddr.value, document.form.dhcp1_end.value, 3)){
			alert(o.value + " <#JS_validip#>");
			o.focus();
			o.select();
			return false;
		}
	}
	
	return true;
}

function change_ipaddrport(o)
{}
function is_ipaddrport(o)
{if (!nav) keyPressed = IEKey();
else keyPressed=NSKey();
if (keyPressed==0)
{return true;
}
if ((keyPressed>47 && keyPressed<58) || keyPressed == 46 || keyPressed == 58)
{return true;
}
return false;
}
function validate_ipaddrport(o, v)
{num = -1;
pos = 0;
if (o.value.length==0)
return true;
str = o.value;
portidx = str.indexOf(":");
if (portidx!=-1)
{port = str.substring(portidx+1);
len = portidx;
if (port>65535)
{alert(port + " <#JS_validport#>");
o.value = "";
o.focus();
o.select();
return false;
}
}
else
{len = o.value.length;
}
for(i=0; i<len; i++) {
c=o.value.charAt(i);
if(c>='0' && c<='9')
{if ( num==-1 )
{num = (c-'0');
}
else
{num = num*10 + (c-'0');
}
}
else
{if ( num<0 || num>255 || c!='.')
{alert(o.value + " <#JS_validip#>");
o.value = "";
o.focus();
o.select();
return false;
}
num = -1;
pos++;
}
}
if (pos!=3 || num<0 || num>255)
{alert(o.value + " <#JS_validip#>");
o.value = "";
o.focus();
o.select();
return false;
}
if (v=='ExternalIPAddress' && document.form.wan_netmask_x.value == '')
{document.form.wan_netmask_x.value = "255.255.255.0";
}
else if (v=='IPRouters' && document.form.lan_netmask.value == '')
{document.form.lan_netmask.value = "255.255.255.0";
}
return true;
}
function change_iprange(o)
{}
function is_iprange(o)
{if (!nav) keyPressed = IEKey();
else keyPressed=NSKey();
if (keyPressed==0)
{return true;
}
if (o.value.length>=15) return false;
if ((keyPressed>47 && keyPressed<58))
{j = 0;
for(i=0; i<o.value.length; i++)
{if (o.value.charAt(i)=='.')
{j++;
}
}
if (j<3 && i>=3)
{if (o.value.charAt(i-3)!='.' && o.value.charAt(i-2)!='.' && o.value.charAt(i-1)!='.')
o.value = o.value + '.';
}
return true;
}
else if (keyPressed == 46)
{j = 0;
for(i=0; i<o.value.length; i++)
{if (o.value.charAt(i)=='.')
{j++;
}
}
if (o.value.charAt(i-1)=='.' || j==3)
{return false;
}
return true;
}
else if (keyPressed == 42) /* '*' */
{return true;
}
else
{return false;
}
return false;
}
function validate_iprange(o, v)
{num = -1;
pos = 0;
if (o.value.length==0)
return true;
for(i=0; i<o.value.length; i++)
{c=o.value.charAt(i);
if(c>='0'&&c<='9')
{if ( num==-1 )
{num = (c-'0');
}
else
{num = num*10 + (c-'0');
}
}
else if (c=='*'&&num==-1)
{num = 0;
}
else
{if ( num<0 || num>255 || (c!='.'))
{alert(o.value + " <#JS_validip#>");
o.value = "";
o.focus();
o.select();
return false;
}
num = -1;
pos++;
}
}
if (pos!=3 || num<0 || num>255)
{alert(o.value + " <#JS_validip#>");
o.value = "";
o.focus();
o.select();
return false;
}
if (v=='ExternalIPAddress' && document.form.wan_netmask_x.value == '')
{document.form.wan_netmask_x.value = "255.255.255.0";
}
else if (v=='IPRouters' && document.form.lan_netmask.value == '')
{document.form.lan_netmask.value = "255.255.255.0";
}
return true;
}
function is_portrange(o){
	if (!nav) keyPressed = IEKey();
	else keyPressed=NSKey();
		
	if (keyPressed==0) return true;
	//if (o.value.length>11) return false;  //limit the input length;
	
	if ((keyPressed>47 && keyPressed<58)){
		return true;
	}
	else if (keyPressed == 58 && o.value.length>0){
		for(i=0; i<o.value.length; i++){
			c=o.value.charAt(i);
			if (c==':' || c=='>' || c=='<' || c=='=')
			return false;
		}
		return true;
	}
	else if (keyPressed==44){  // can be type in first charAt
		if (o.value.length==0)
			return false;		
		else
			return true;
	}
	else if (keyPressed==60 || keyPressed==62){  //">" and "<" only can be type in first charAt
		if (o.value.length==0)
			return true;		
		else
			return false;
	}
	else{
		return false;
	}
}

function validate_portrange(o, v){
		if(o.value.length == 0)
			return true;
		
		prev = -1;
		num = -1;
		num_front = 0;
		for(var i = 0; i < o.value.length; i++){
			c=o.value.charAt(i);
			if (c>='0'&&c<='9'){
				if(num==-1) num=0;
				num = num*10 + (c-'0');				
			}
			else{				
				if (num>65535 || num==0 || (c!=':' && c!='>' && c!='<' && c!=',')){
					alert(o.value + " <#JS_validport#>");
					//o.value = "";
					o.focus();
					o.select();
					return false;
				}
			
				if (c=='>') prev = -2;
				else if (c=='<') prev = -3;
				else if (c==','){ 
					prev = -4;
					num = 0;					
				}
				else{ //when c=":"
					if (prev==-4)
						prev == -4;
					else{
						prev = num;
						num = 0;
					}					
				}
			}			
		}
		
		if ((num>65535 && prev!=-3) || (num<1&&prev!=-2) || (prev>num) || (num>=65535&&prev==-2) || (num<=1&&prev==-3)){
			if (num>65535){				
				alert(o.value + " <#JS_validport#>");
				o.focus();
				o.select();
				return false;
			}
			else{				
				alert(o.value + " <#JS_validportrange#>");
				//o.value = "";
				o.focus();
				o.select();
				return false;
			}
		} // wrong port 
		else{
			if (prev==-2){
				if (num==65535) o.value = num;
				else o.value = (num+1) + ":65535";  //ex. o.value=">2000", it will change to 2001:65535
			}
			else if (prev==-3){
				if (num==1) o.value = num;
				else o.value = "1:" + (num-1);     //ex. o.value="<2000", it will change to 1:1999
			}
			else if (prev==-4){ 			
				if(document.form.current_page.value == "Advanced_VirtualServer_Content.asp"){  //Allow input "," in Virtual Server page
					multi_vts_port = o.value.split(",");
					//o.value = multi_vts_port[0];
					split_vts_rule(multi_vts_port);
					return false;
				}
				else{
					alert(o.value + " <#JS_validport#>");
					o.focus();
					o.select();
					return false;						
				}
			}
			else if (prev!=-1)
				o.value = prev + ":" + num;
			else
				o.value = num;                  //single port number case;
		}// correct port		
		return true;
}
function is_portlist(o)
{if (!nav) keyPressed = IEKey();
else keyPressed = NSKey();
if (keyPressed == 0 ) return true;
if (o.value.length>36) return false;
if ((keyPressed>47 && keyPressed<58) || keyPressed == 32)
{return true;
}
else
{return false;
}
}
function validate_portlist(o, v)
{if (o.value.length==0)
return true;
num = 0;
for(i=0; i<o.value.length; i++)
{c=o.value.charAt(i);
if (c>='0'&&c<='9')
{num = num*10 + (c-'0');
}
else
{if (num>255)
{alert(num + " <#JS_validport#>");
o.value = "";
o.focus();
o.select();
return false;
}
num = 0;
}
}
if (num>255)
{alert(num + " <#JS_validport#>");
o.value = "";
o.focus();
o.select();
return false;
}
return true;
}

function add_option_match(o, s, f)
{tail = o.options.length;
o.options[tail] = new Option(s);
o.options[tail].value = s;
if (f==s)
{o.options[tail].selected = 1;
return(1);
}
else return(0);
}
function add_option_match_x(o, s, f)
{tail = o.options.length;
o.options[tail] = new Option(s);
o.options[tail].value = tail;
if (tail==f)
{o.options[tail].selected = 1;
return(1);
}
else return(0);
}
function find_option(o)
{count = o.options.length;
for (i=0; i<count; i++)
{if (o.options[i].selected)
return(o.options[i].value);
}
return(null);
}

function add_options(o, arr, orig){
	free_options(o);
	for(var i = 0; i < arr.length; i++){
		if(orig == arr[i])
			add_option(o, arr[i], arr[i], 1); // obj, str, val, sel
		else
			add_option(o, arr[i], arr[i], 0);
	}
}

function add_options_x2(o, arr, varr, orig){
	free_options(o);
	for(var i = 0; i < arr.length; ++i){
		if(orig == varr[i])
			add_option(o, arr[i], varr[i], 1);
		else
			add_option(o, arr[i], varr[i], 0);
	}
}

function add_a_option(o, i, s)
{tail = o.options.length;
	o.options[tail] = new Option(s);
	o.options[tail].value = i;
}

function rcheck(o){
	if(o.value == "1")
		return("1");
	else
	return("0");
}

function getDateCheck(str, pos)
{if (str.charAt(pos) == '1')
return true;
else
return false;
}
function getTimeRange(str, pos)
{if (pos == 0)
return str.substring(0,2);
else if (pos == 1)
return str.substring(2,4);
else if (pos == 2)
return str.substring(4,6);
else if (pos == 3)
return str.substring(6,8);
}
function setDateCheck(d1, d2, d3, d4, d5, d6, d7)
{str = "";
if (d7.checked == true ) str = "1" + str;
else str = "0" + str;
if (d6.checked == true ) str = "1" + str;
else str = "0" + str;
if (d5.checked == true ) str = "1" + str;
else str = "0" + str;
if (d4.checked == true ) str = "1" + str;
else str = "0" + str;
if (d3.checked == true ) str = "1" + str;
else str = "0" + str;
if (d2.checked == true ) str = "1" + str;
else str = "0" + str;
if (d1.checked == true ) str = "1" + str;
else str = "0" + str;
return str;
}
function setTimeRange(sh, sm, eh, em)
{return(sh.value+sm.value+eh.value+em.value);
}

function change_firewall(r){
	if (r == "0") {
		inputCtrl(document.form.fw_log_x, 0);
		inputRCtrl1(document.form.misc_ping_x, 0);
	} else {
		inputCtrl(document.form.fw_log_x, 1);
		inputRCtrl1(document.form.misc_ping_x, 1);
	}
}

function onSubmit(){
	change = 0;
	pageChanged = 0;
	pageChangedCount = 0;
	
if (document.form.current_page.value == "Advanced_RLANWAN_Content.asp")
{checkSubnet();
}
else if (document.form.current_page.value == "Advanced_DMZIP_Content.asp")
{checkDmzSubnet();
}
else if (document.form.current_page.value == "Advanced_LFirewall_Content.asp")
{updateDateTime(document.form.current_page.value);
}
else if (document.form.current_page.value == "Advanced_WebCam_Content.asp")
{updateDateTime(document.form.current_page.value);
}
else if (document.form.current_page.value == "Advanced_WAdvanced_Content.asp")
{updateDateTime(document.form.current_page.value);
}
else if (document.form.current_page.value == "Advanced_WirelessGuest_Content.asp")
{if (!validate_ipaddr_final(document.form.lan1_ipaddr, 'lan1_ipaddr') ||
!validate_ipaddr_final(document.form.lan1_netmask, 'lan1_netmask') ||
!validate_ipaddr_final(document.form.dhcp1_start, 'dhcp1_start') ||
!validate_ipaddr_final(document.form.dhcp1_end, 'dhcp1_end')) return false;
if (intoa(document.form.dhcp1_start.value)>intoa(document.form.dhcp1_end.value))
{tmp = document.form.dhcp1_start.value;
document.form.dhcp1_start.value = document.form.dhcp1_end.value;
document.form.dhcp1_end.value = tmp;
}
inputCtrl(document.form.wl_guest_phrase_x_1, 1);
inputCtrl(document.form.wl_guest_wep_x_1, 1);
inputCtrl(document.form.wl_guest_key1_1, 1);
inputCtrl(document.form.wl_guest_key2_1, 1);
inputCtrl(document.form.wl_guest_key3_1, 1);
inputCtrl(document.form.wl_guest_key4_1, 1);
inputCtrl(document.form.wl_guest_key_1, 1);
unmasq_wepkey_guest();
}

	return true;
}

function onSubmitCtrl(o, s) {
document.form.action_mode.value = s;
return (onSubmit());
}

function onSubmitCtrlOnly(o, s){
	if(s != 'Upload' && s != 'Upload1')
		document.form.action_mode.value = s;
	
	if(s == 'Upload1'){
		if(document.form.file.value){
			showLoading();
			disableCheckChangedStatus();
			document.form.submit();
		}
		else{
			alert("<#JS_Shareblanktest#>");
			document.form.file.focus();
		}
	}
	//stopFlag = 1;	
	disableCheckChangedStatus();
	return true;
}

function onSubmitApply(s){
	pageChanged = 0;
	pageChangedCount = 0;
	if(document.form.current_page.value == "Advanced_ASUSDDNS_Content.asp"){
		if(s == "hostname_check"){
			showLoading();
			if(!validate_ddns_hostname(document.form.ddns_hostname_x)){
				hideLoading();
				return false;
			}
		}
	}
	document.form.action_mode.value = "Update";
	document.form.action_script.value = s;
	return true;
}

function setup_script(s)
{if (document.form.current_page.value == "Advanced_ACL_Content.asp")
{document.form.action_script.value = s;
}
}

function automode_hint(){ //for 54Mbps limitation in auto mode + WEP/TKIP.
	if(document.form.wl_nmode_x.value == "0" 
			&& (document.form.wl_wep_x.value == 1 
				|| document.form.wl_wep_x.value == 2 
				|| document.form.wl_crypto.value == "tkip"
				|| document.form.wl_auth_mode_x.value == "radius")){
		if(document.form.current_page.value == "Advanced_Wireless_Content.asp")
			$("wl_nmode_x_hint").style.display = "";
	}
	else{
		if(document.form.current_page.value == "Advanced_Wireless_Content.asp")
			$("wl_nmode_x_hint").style.display = "none";
	}
}

function nmode_limitation(){ //for TKIP limitation in n mode.
	if (document.form.wl_nmode_x.value == "1") {
		if(document.form.wl_auth_mode_x.selectedIndex == 0 && (document.form.wl_wep_x.selectedIndex == "1" || document.form.wl_wep_x.selectedIndex == "2")){
			alert("<#WLANConfig11n_nmode_limition_hint#>");
			document.form.wl_auth_mode_x.selectedIndex = 3;
			document.form.wl_wpa_mode.value = 2;
		}
		else if(document.form.wl_auth_mode_x.selectedIndex == 1){
			alert("<#WLANConfig11n_nmode_limition_hint#>");
			document.form.wl_auth_mode_x.selectedIndex = 3;
			document.form.wl_wpa_mode.value = 2;
		}
		else if(document.form.wl_auth_mode_x.selectedIndex == 2){
			alert("<#WLANConfig11n_nmode_limition_hint#>");
			document.form.wl_auth_mode_x.selectedIndex = 3;
			document.form.wl_wpa_mode.value = 2;
		}
		else if(document.form.wl_auth_mode_x.selectedIndex == 5){
			alert("<#WLANConfig11n_nmode_limition_hint#>");
			document.form.wl_auth_mode_x.selectedIndex = 6;
		}
		else if(document.form.wl_auth_mode_x.selectedIndex == 7 && (document.form.wl_crypto.selectedIndex == 0 || document.form.wl_crypto.selectedIndex == 2)){
			alert("<#WLANConfig11n_nmode_limition_hint#>");
			document.form.wl_crypto.selectedIndex = 1;
		}
		wl_auth_mode_change(0);
	}
}

function remove_11ac_80MHz_option(){
	document.form.wl_bw[0].text = "20/40 MHz";
	document.form.wl_bw.remove(3);
}

function change_common(o, s, v){
	change = 1;
	pageChanged = 1;
	if(v == "wl_auth_mode_x"){ /* Handle AuthenticationMethod Change */
		wl_auth_mode_change(0);
		if(o.value == "psk" || o.value == "psk2" || o.value == "pskpsk2" || o.value == "wpa" || o.value == "wpawpa2"){
			opts = document.form.wl_auth_mode_x.options;			
			if(opts[opts.selectedIndex].text == "WPA-Personal"){
				document.form.wl_wpa_mode.value="1";
				automode_hint();
			}
			else if(opts[opts.selectedIndex].text == "WPA2-Personal")
				document.form.wl_wpa_mode.value="2";
			else if(opts[opts.selectedIndex].text == "WPA-Auto-Personal")
				document.form.wl_wpa_mode.value="0";
			else if(opts[opts.selectedIndex].text == "WPA-Enterprise")
				document.form.wl_wpa_mode.value="3";
			else if(opts[opts.selectedIndex].text == "WPA-Auto-Enterprise")
				document.form.wl_wpa_mode.value="4";
			
			if(o.value == "psk" || o.value == "psk2" || o.value == "pskpsk2"){
				document.form.wl_wpa_psk.focus();
				document.form.wl_wpa_psk.select();
			}
		}
		else if(o.value == "shared"){
			document.form.wl_key1.focus();
			document.form.wl_key1.select();
		}
		nmode_limitation();
		automode_hint();
	}
	else if (v == "wl_crypto"){
		wl_auth_mode_change(0);
		nmode_limitation();
		automode_hint();
	}
	else if(v == "wl_wep_x"){ /* Handle AuthenticationMethod Change */
		change_wlweptype(o, "WLANConfig11b");
		nmode_limitation();
		automode_hint();
	}
	else if(v == "wl_key"){ /* Handle AuthenticationMethod Change */
		var selected_key = eval("document.form.wl_key"+o.value);
		
		selected_key.focus();
		selected_key.select();
	}
	else if(s=="WLANConfig11b" && v == "wl_channel"){
		show_DFS_hint(o);
		insertExtChannelOption();
	}
	else if(s=="WLANConfig11b" && v == "wl_bw"){
		insertChannelOption();
		insertExtChannelOption();
	}
	else if (s=="WLANConfig11b" && v=="wl_gmode_check"){
		if (document.form.wl_gmode_check.checked == true)
			document.form.wl_gmode_protection.value = "auto";
		else
			document.form.wl_gmode_protection.value = "off";
	}
	else if(s=="WLANConfig11b" && v == "wl_nmode_x"){
		if (o.value == "1")
			inputCtrl(document.form.wl_gmode_check, 0);
		else
			inputCtrl(document.form.wl_gmode_check, 1);
		
		if (o.value == "2")
			inputCtrl(document.form.wl_bw, 0);
		else
			inputCtrl(document.form.wl_bw, 1);

		insertChannelOption();
		insertExtChannelOption();
		if (o.value == "3")
			document.form.wl_wme.value = "1";

		nmode_limitation();
		automode_hint();
	}
	else if (v == "wl_guest_auth_mode_1") /* Handle AuthenticationMethod Change */
	{wl_auth_mode_change_guest(0);
if (o.value == "psk" || mode == "psk2" || mode == "pskpsk2"){
document.form.wl_guest_wpa_psk_1.focus();
document.form.wl_guest_wpa_psk_1.select();
}
else if (o.value == "shared" || o.value == "radius"){
document.form.wl_guest_phrase_x_1.focus();
document.form.wl_guest_phrase_x_1.select();
}
	}
	else if (v == "wl_guest_wep_x_1") /* Handle AuthenticationMethod Change */
	{
		change_wlweptype_guest(o, "WLANConfig11b");
	}
else if (v == "wl_guest_crypto_1")
{wl_auth_mode_change_guest(0);
}
else if (s == "FirewallConfig" && v=="WanLanDefaultAct")
{if (o.value == "DROP")
alert("<#JS_WanLanAlert#>");
}
else if (s == "FirewallConfig" && v=="LanWanDefaultAct")
{if (o.value == "DROP")
alert("<#JS_LanWanAlert#>");
}
else if (s=="WLANConfig11b" && v=="Channel" && document.form.current_page.value=="Advanced_WMode_Content.asp")
{if (document.form.WLANConfig11b_x_APMode.value != "0" && document.form.WLANConfig11b_Channel.value == "0")
{alert("<#JS_fixchannel#>");
document.form.WLANConfig11b_Channel.options[0].selected = 0;
document.form.WLANConfig11b_Channel.options[1].selected = 1;
}
}
else if (v=="ddns_server_x")
{change_ddns_setting(o.value);
}
else if (v == "wl_wme"){
	if(o.value == "0"){
		inputCtrl(document.form.wl_wme_no_ack, 0);
		//inputCtrl(document.form.APSDCapable, 0); // jerry5 nvram error
		//inputCtrl(document.form.DLSCapable, 0); // jerry5 nvram error 
	}
	else{
		inputCtrl(document.form.wl_wme_no_ack, 1);
		//inputCtrl(document.form.APSDCapable, 1); // jerry5 error 
		//inputCtrl(document.form.DLSCapable, 1); // jerry5 error 
	}
}else if (v == "time_zone_select"){
		if(o.value.indexOf("DST")!=-1){	//include "DST"			
				document.getElementById("chkbox_time_zone_dst").style.display="";	
				if(!document.getElementById("time_zone_dst_chk").checked){
					document.form.time_zone_dst.value=0;
					document.getElementById("dst_start").style.display="none";
					document.getElementById("dst_end").style.display="none";
				}else{
					document.form.time_zone_dst.value=1;
					document.getElementById("dst_start").style.display="";	
					document.getElementById("dst_end").style.display="";						
				}
		}else{
				document.getElementById("chkbox_time_zone_dst").style.display="none";	
				document.getElementById("time_zone_dst_chk").checked = false;
				document.form.time_zone_dst.value=0;			
				document.getElementById("dst_start").style.display="none";
				document.getElementById("dst_end").style.display="none";
}
	
}else if (v == "time_zone_dst_chk"){
		if (document.form.time_zone_dst_chk.checked){
				document.form.time_zone_dst.value = "1";
				document.getElementById("dst_start").style.display="";	
				document.getElementById("dst_end").style.display="";
				
		}else{
				document.form.time_zone_dst.value = "0";	
				document.getElementById("dst_start").style.display="none";	
				document.getElementById("dst_end").style.display="none";
	}		
}

return true;
}

function change_ddns_setting(v){
	var hostname_x = '<% nvram_get("ddns_hostname_x"); %>';
	if (v == "WWW.ASUS.COM"){
		document.form.ddns_hostname_x.parentNode.style.display = "none";
		document.form.DDNSName.parentNode.style.display = "";
		var ddns_hostname_title = hostname_x.substring(0, hostname_x.indexOf('.asuscomm.com'));
		if(hostname_x != '' && ddns_hostname_title)
			document.getElementById("DDNSName").value = ddns_hostname_title;
		else
			document.getElementById("DDNSName").value = "<#asusddns_inputhint#>";

		inputCtrl(document.form.ddns_username_x, 0);
		inputCtrl(document.form.ddns_passwd_x, 0);
		document.form.ddns_wildcard_x[0].disabled= 1;
		document.form.ddns_wildcard_x[1].disabled= 1;
		showhide("link", 0);
		showhide("wildcard_field",0);
	}else{
		document.form.ddns_hostname_x.parentNode.style.display = "";
		document.form.DDNSName.parentNode.style.display = "none";
		inputCtrl(document.form.ddns_username_x, 1);
		inputCtrl(document.form.ddns_passwd_x, 1);
		var disable_wild = (v == "WWW.TUNNELBROKER.NET") ? 1 : 0;
		document.form.ddns_wildcard_x[0].disabled= disable_wild;
		document.form.ddns_wildcard_x[1].disabled= disable_wild;
		if(v == "WWW.ZONEEDIT.COM"){// remove free trail of zoneedit and add a link to direct to zoneedit 
			showhide("link", 0);
		}else{
			showhide("link", 1);
		}

		showhide("wildcard_field",!disable_wild);
	}
}

function change_common_radio(o, s, v, r){
	change = 1;
	pageChanged = 1;
	if (v == "wl_radio") {
		if (document.form.wl_radio_x[0].checked == true) {
			document.getElementById("weekday_tr").style.display = "";
			document.getElementById("weekday_time_tr").style.display = "";
			document.getElementById("weekend_tr").style.display = "";
			document.getElementById("weekend_time_tr").style.display = "";
		} else {
			document.getElementById("weekday_tr").style.display = "none";
			document.getElementById("weekday_time_tr").style.display = "none";
			document.getElementById("weekend_tr").style.display = "none";
			document.getElementById("weekend_time_tr").style.display = "none";
		}
	} else if (v == "ddns_enable_x") {
		var hostname_x = '<% nvram_get("ddns_hostname_x"); %>';
		if(r == 1){
			inputCtrl(document.form.ddns_server_x, 1);
			if('<% nvram_get("ddns_server_x"); %>' == 'WWW.ASUS.COM'){
				document.form.DDNSName.disabled = false;
				document.form.DDNSName.parentNode.parentNode.parentNode.style.display = "";
				if(hostname_x != ''){
					var ddns_hostname_title = hostname_x.substring(0, hostname_x.indexOf('.asuscomm.com'));
					if(!ddns_hostname_title)
						document.getElementById("DDNSName").value = "<#asusddns_inputhint#>";
					else
						document.getElementById("DDNSName").value = ddns_hostname_title;
				}else{
					document.getElementById("DDNSName").value = "<#asusddns_inputhint#>";
				}
				showhide("wildcard_field",0);
			}else{
				document.form.ddns_hostname_x.parentNode.parentNode.parentNode.style.display = "";
				inputCtrl(document.form.ddns_username_x, 1);
				inputCtrl(document.form.ddns_passwd_x, 1);
				showhide("wildcard_field",1);
			}
			change_ddns_setting(document.form.ddns_server_x.value);
		}else{
			if(document.form.ddns_server_x.value == "WWW.ASUS.COM"){
				document.form.DDNSName.parentNode.parentNode.parentNode.style.display = "none";
			}else{
				document.form.ddns_hostname_x.parentNode.parentNode.parentNode.style.display = "none";
				inputCtrl(document.form.ddns_username_x, 0);
				inputCtrl(document.form.ddns_passwd_x, 0);
			}
			inputCtrl(document.form.ddns_server_x, 0);
			document.form.ddns_wildcard_x[0].disabled= 1;
			document.form.ddns_wildcard_x[1].disabled= 1;
			showhide("wildcard_field",0);
		}
	}
	else if (v=='wps_mode')
	{
		if (r == '1')
			inputCtrl(document.form.wsc_sta_pin, 1);
		else
			inputCtrl(document.form.wsc_sta_pin, 0);
	}
else if (v=="qos_dfragment_enable")
{if (r == '1')
{inputCtrl(document.form.qos_dfragment_size, 1);
}
else
{inputCtrl(document.form.qos_dfragment_size, 0);
}
}
	else if(v == "wan_dnsenable_x"){
		if(r == 1){
			inputCtrl(document.form.wan_dns1_x, 0);
			inputCtrl(document.form.wan_dns2_x, 0);
		}
		else{
			inputCtrl(document.form.wan_dns1_x, 1);
			inputCtrl(document.form.wan_dns2_x, 1);
		}
	}
else if (v=="fw_enable_x")
{change_firewall(r);
}
else if (v=="x_AESEnable")
{if (r == '1')
{if (document.form.WLANConfig11a_AuthenticationMethod.value!="Open System")
{rst = confirm("<#JS_AES2#>");
if (rst)
document.form.WLANConfig11a_AuthenticationMethod.value = "Open System";
else
{inputRCtrl2(document.form.WLANConfig11a_x_AESEnable, 1);
return false;
}
}
else
{if (document.form.WLANConfig11a_WEPType.value == "None")
{rst = confirm("<#JS_AES3#>");
if (rst)
{document.form.WLANConfig11a_WEPType.value = "64bits";
change_wlweptype(document.form.WLANConfig11a_WEPType, "WLANConfig11a");
}
else
{inputRCtrl2(document.form.WLANConfig11a_x_AESEnable, 1);
return false;
}
}
}
}
}
else if (s=="PrinterStatus" && v=="usb_webhttpport_x")
{if (document.form.usb_webhttpport_x_check.checked)
{document.form.usb_webhttpcheck_x.value = "1";
}
else
{document.form.usb_webhttpcheck_x.value = "0";
}
}
else if (v=="lan_proto_x")
{if (r == '1')
{inputCtrl(document.form.lan_ipaddr, 0);
inputCtrl(document.form.lan_netmask, 0);
inputCtrl(document.form.lan_gateway, 0);
}
else
{inputCtrl(document.form.lan_ipaddr, 1);
inputCtrl(document.form.lan_netmask, 1);
inputCtrl(document.form.lan_gateway, 1);
}
}
else if (s=="PPPConnection" && v=="wan_pppoe_idletime")
{if (document.form.wan_pppoe_idletime_check.checked)
{document.form.wan_pppoe_txonly_x.value = "1";
}
else
{document.form.wan_pppoe_txonly_x.value = "0";
}
}
else if (s=="PPPConnection" && v=="x_IdleTime1")
{if (document.form.PPPConnection_x_IdleTime1_check.checked)
{document.form.PPPConnection_x_IdleTxOnly1.value = "1";
}
else
{document.form.PPPConnection_x_IdleTxOnly1.value = "0";
}
}
else if (s=="PPPConnection" && v=="x_MultiPPPoEEnable1")
{if (document.form.PPPConnection_x_MultiPPPoEEnable1[0].checked == true)
{flag=1;
}
else
{flag=0;
}
inputCtrl(document.form.PPPConnection_x_UserName1, flag);
inputCtrl(document.form.PPPConnection_x_Password1, flag);
inputCtrl(document.form.PPPConnection_x_IdleTime1, flag);
inputCtrl(document.form.PPPConnection_x_IdleTime1_check, flag);
inputCtrl(document.form.PPPConnection_x_PPPoEMTU1, flag);
inputCtrl(document.form.PPPConnection_x_PPPoEMRU1, flag);
inputCtrl(document.form.PPPConnection_x_ServiceName1, flag);
inputCtrl(document.form.PPPConnection_x_AccessConcentrator1, flag);
}
else if (s=="PPPConnection" && v=="x_IdleTime2")
{if (document.form.PPPConnection_x_IdleTime2_check.checked)
{document.form.PPPConnection_x_IdleTxOnly2.value = "1";
}
else
{document.form.PPPConnection_x_IdleTxOnly2.value = "0";
}
}
else if (s=="PPPConnection" && v=="x_MultiPPPoEEnable2")
{if (document.form.PPPConnection_x_MultiPPPoEEnable2[0].checked == true)
{flag=1;
}
else
{flag=0;
}
inputCtrl(document.form.PPPConnection_x_UserName2, flag);
inputCtrl(document.form.PPPConnection_x_Password2, flag);
inputCtrl(document.form.PPPConnection_x_IdleTime2, flag);
inputCtrl(document.form.PPPConnection_x_IdleTime2_check, flag);
inputCtrl(document.form.PPPConnection_x_PPPoEMTU2, flag);
inputCtrl(document.form.PPPConnection_x_PPPoEMRU2, flag);
inputCtrl(document.form.PPPConnection_x_ServiceName2, flag);
inputCtrl(document.form.PPPConnection_x_AccessConcentrator2, flag);
}
return true;
}

function valid_WPAPSK(o){
	if(o.value.length >= 64){
		o.value = o.value.substring(0, 63);
		alert("<#JS_wpapass#>");
		return false;
	}
	
	return true;
}

function encryptionType(authType, wepType)
{pflag = "1";
if (authType.value == "1")
{if (wepType.value == "0") wepLen = "64";
else wepLen = "128";
}
else if (authType.value == "2")
{wepLen = "0";
}
else if (authType.value == "3")
{wepLen = "0";
pflag = "0";
}
else if (authType.value == "4")
{if (wepType.value == "0") wepLen = "64";
else wepLen = "128";
}
else
{if (wepType.value == "0")
{wepLen = "0";
pflag = "0";
}
else if (wepType.value == "1") wepLen = "64";
else wepLen = "128";
}
return(pflag + wepLen);
}

function change_wlweptype(o, s, isload){
	var wflag = 0;

	if(o.value != "0"){
		wflag = 1;
	}

	inputCtrl(document.form.wl_key1, wflag);
	inputCtrl(document.form.wl_key2, wflag);
	inputCtrl(document.form.wl_key3, wflag);
	inputCtrl(document.form.wl_key4, wflag);
	inputCtrl(document.form.wl_key, wflag);
	
	wl_wep_change();
}

function is_wlkey(o, s){
	if(!nav)
		keyPressed = IEKey();
	else
		keyPressed = NSKey();
	
	if(keyPressed == 0)
		return true;
	
	if(document.form.current_page.value == "Advanced_WirelessGuest_Content.asp")
		wep = document.form.wl_guest_wep_x_1.value;
	else
		wep = document.form.wl_wep_x.value;
	
	if((keyPressed > 47 && keyPressed < 58)
			|| (keyPressed > 64 && keyPressed < 71)
			|| (keyPressed > 96 && keyPressed < 103)){
		if(wep == "1"){
			if(o.value != "" && o.value.length > 10)
				return false;
		}
		else if(wep == "2"){
			if(o.value != "" && o.value.length > 26)
				return false;
		}
		else
			return false;
	}
	else
		return false;
	
	return true;
}

function change_wlkey(o, s){
	if(document.form.current_page.value == "Advanced_WirelessGuest_Content.asp")
		wep = document.form.wl_guest_wep_x_1.value;
	else
		wep = document.form.wl_wep_x.value;
	
	if(wep == "1"){
		if(o.value.length > 10)
			o.value = o.value.substring(0, 10);
	}
	else if(wep == "2"){
		if(o.value.length > 26)
			o.value = o.value.substring(0, 26);
	}
	
	return true;
}

function validate_timerange(o, p)
{
	if (o.value.length==0) 
		o.value = "00";
	else if (o.value.length==1) 
		o.value = "0" + o.value;
		
	if (o.value.charAt(0)<'0' || o.value.charAt(0)>'9') 
		o.value = "00";
	else if (o.value.charAt(1)<'0' || o.value.charAt(1)>'9') 
		o.value = "00";
	else if (p==0 || p==2)
	{
		if(o.value>23){
			alert("<#JS_validrange#> 00 <#JS_validrange_to#> 23");
			o.value = "00";
			o.focus();
			o.select();
			return false;			
		}	
		return true;
	}
	else
	{
		if(o.value>59){
			alert("<#JS_validrange#> 00 <#JS_validrange_to#> 59");
			o.value = "00";
			o.focus();
			o.select();
			return false;			
		}	
		return true;
	}
	return true;
}

function matchSubnet(ip1, ip2, count)
{var c = 0;
var v1 = 0;
var v2 = 0;
for(i=0;i<ip1.length;i++)
{if (ip1.charAt(i) == '.')
{if (ip2.charAt(i) != '.')
return false;
c++;
if (v1!=v2) return false;
v1 = 0;
v2 = 0;
}
else
{if (ip2.charAt(i)=='.') return false;
v1 = v1*10 + (ip1.charAt(i) - '0');
v2 = v2*10 + (ip2.charAt(i) - '0');
}
if (c==count) return true;
}
return false;
}
function subnetPrefix(ip, orig, count)
{r='';
c=0;
for(i=0;i<ip.length;i++)
{if (ip.charAt(i) == '.')
c++;
if (c==count) break;
r = r + ip.charAt(i);
}
c=0;
for(i=0;i<orig.length;i++)
{if (orig.charAt(i) == '.')
{c++;
}
if (c>=count)
r = r + orig.charAt(i);
}
return (r);
}

//Change subnet postfix .xxx
function subnetPostfix(ip, num, count){
	r = '';
	orig = "";
	c = 0;
	for (i=0 ; i<ip.length ; i++) {
		if (ip.charAt(i) == '.') c++;
		r = r + ip.charAt(i);
		if (c == count) break;
	}
	c = 0;
	orig = String(num);
	for (i=0 ; i<orig.length ; i++)
		r = r + orig.charAt(i);
	return (r);
}

function checkSubnet(){
	/* Rule : If addresses in pool are match to subnet, don't change */
	/* Rule : If addresses in pool are not match to subnet, change to subnet.2~subnet.254 */
	if(!matchSubnet(document.form.lan_ipaddr.value, document.form.dhcp_start.value, 3) ||
			!matchSubnet(document.form.lan_ipaddr.value, document.form.dhcp_end.value, 3)){
            if(confirm("<#JS_DHCP1#>")){
                    document.form.dhcp_start.value = subnetPrefix(document.form.lan_ipaddr.value, document.form.dhcp_start.value, 3);
                    document.form.dhcp_end.value = subnetPrefix(document.form.lan_ipaddr.value, document.form.dhcp_end.value, 3);
                    return true;
            }else{
                    return false;   
            }
	}
}

function checkDmzSubnet()
{/* Rule : If addresses in pool are match to subnet, don't change */
/* Rule : If addresses in pool are not match to subnet, change to subnet.2~subnet.254 */
if (!matchSubnet(document.form.FirewallConfig_DmzIP.value, document.form.LANHostConfig_DmzMinAddress.value, 3) ||
!matchSubnet(document.form.FirewallConfig_DmzIP.value, document.form.LANHostConfig_DmzMaxAddress.value, 3))
{if (confirm("<#JS_DHCP2#>"))
{document.form.LANHostConfig_DmzMinAddress.value = subnetPrefix(document.form.FirewallConfig_DmzIP.value, document.form.LANHostConfig_DmzMinAddress.value, 3);
document.form.LANHostConfig_DmzMaxAddress.value = subnetPrefix(document.form.FirewallConfig_DmzIP.value, document.form.LANHostConfig_DmzMaxAddress.value, 3);
}
}
}
function checkSubnetGuest()
{
	/* Rule : If addresses in pool are match to subnet, don't change */
	/* Rule : If addresses in pool are not match to subnet, change to subnet.2~subnet.254 */
	if (!matchSubnet(document.form.lan1_ipaddr.value, document.form.dhcp1_start.value, 3) ||
	!matchSubnet(document.form.lan1_ipaddr.value, document.form.dhcp1_end.value, 3))
	{
		if (confirm("<#JS_DHCP1#>"))
		{
			document.form.dhcp1_start.value = subnetPrefix(document.form.lan1_ipaddr.value, document.form.dhcp1_start.value, 3);
			document.form.dhcp1_end.value = subnetPrefix(document.form.lan1_ipaddr.value, document.form.dhcp1_end.value, 3);
		}
	}
}
function wan_netmask_x_check(o) {
ip =intoa(document.form.wan_ipaddr_x.value);
gw =intoa(document.form.wan_gateway_x.value);
nm =intoa(document.form.wan_netmask_x.value);
lip=intoa(document.form.lan_ipaddr.value);
lnm=intoa(document.form.lan_netmask.value);
rem=1;
if (document.form.wan_ipaddr_x.value != '0.0.0.0' && (ip&lnm)==(lip&lnm))
{alert(o.value + " <#JS_validip#>");
document.form.wan_ipaddr_x.value="10.1.1.1";
document.form.wan_netmask_x.value="255.0.0.0";
o.focus();
o.select();
return false;
}
if (gw==0 || gw==0xffffffff || (ip&nm)==(gw&nm))
{return true;
}
else
{alert(o.value + " <#JS_validip#>");
o.focus();
o.select();
return false;
}
}

function updateDateTime(s)
{
	if (s == "Advanced_Firewall_Content.asp")
	{
		document.form.filter_lw_date_x.value = setDateCheck(
		document.form.filter_lw_date_x_Sun,
		document.form.filter_lw_date_x_Mon,
		document.form.filter_lw_date_x_Tue,
		document.form.filter_lw_date_x_Wed,
		document.form.filter_lw_date_x_Thu,
		document.form.filter_lw_date_x_Fri,
		document.form.filter_lw_date_x_Sat);
		document.form.filter_lw_time_x.value = setTimeRange(
		document.form.filter_lw_time_x_starthour,
		document.form.filter_lw_time_x_startmin,
		document.form.filter_lw_time_x_endhour,
		document.form.filter_lw_time_x_endmin);
	}
	else if (s == "Advanced_WAdvanced_Content.asp")
	{
		document.form.wl_radio_date_x.value = setDateCheck(
		document.form.wl_radio_date_x_Sun,
		document.form.wl_radio_date_x_Mon,
		document.form.wl_radio_date_x_Tue,
		document.form.wl_radio_date_x_Wed,
		document.form.wl_radio_date_x_Thu,
		document.form.wl_radio_date_x_Fri,
		document.form.wl_radio_date_x_Sat);
		document.form.wl_radio_time_x.value = setTimeRange(
		document.form.wl_radio_time_x_starthour,
		document.form.wl_radio_time_x_startmin,
		document.form.wl_radio_time_x_endhour,
		document.form.wl_radio_time_x_endmin);
		document.form.wl_radio_time2_x.value = setTimeRange(
		document.form.wl_radio_time2_x_starthour,
		document.form.wl_radio_time2_x_startmin,
		document.form.wl_radio_time2_x_endhour,
		document.form.wl_radio_time2_x_endmin);
	}
	else if (s == "Advanced_URLFilter_Content.asp")
	{
		document.form.url_date_x.value = setDateCheck(
		document.form.url_date_x_Sun,
		document.form.url_date_x_Mon,
		document.form.url_date_x_Tue,
		document.form.url_date_x_Wed,
		document.form.url_date_x_Thu,
		document.form.url_date_x_Fri,
		document.form.url_date_x_Sat);
		document.form.url_time_x.value = setTimeRange(
		document.form.url_time_x_starthour,
		document.form.url_time_x_startmin,
		document.form.url_time_x_endhour,
		document.form.url_time_x_endmin);
		document.form.url_time_x_1.value = setTimeRange(
		document.form.url_time_x_starthour_1,
		document.form.url_time_x_startmin_1,
		document.form.url_time_x_endhour_1,
		document.form.url_time_x_endmin_1);
	}		
	else if (s == "Advanced_LFirewall_Content.asp")
	{
		document.form.FirewallConfig_WanLocalActiveDate.value = setDateCheck(
		document.form.FirewallConfig_WanLocalActiveDate_Sun,
		document.form.FirewallConfig_WanLocalActiveDate_Mon,
		document.form.FirewallConfig_WanLocalActiveDate_Tue,
		document.form.FirewallConfig_WanLocalActiveDate_Wed,
		document.form.FirewallConfig_WanLocalActiveDate_Thu,
		document.form.FirewallConfig_WanLocalActiveDate_Fri,
		document.form.FirewallConfig_WanLocalActiveDate_Sat);
		document.form.FirewallConfig_WanLocalActiveTime.value = setTimeRange(
		document.form.FirewallConfig_WanLocalActiveTime_starthour,
		document.form.FirewallConfig_WanLocalActiveTime_startmin,
		document.form.FirewallConfig_WanLocalActiveTime_endhour,
		document.form.FirewallConfig_WanLocalActiveTime_endmin);
	}
}

function openLink(s)
{if (s=='x_DDNSServer')
{if (document.form.ddns_server_x.value.indexOf("WWW.DYNDNS.ORG")!=-1)
tourl = "https://www.dyndns.org/account/create.html"
else if (document.form.ddns_server_x.value == 'WWW.ZONEEDIT.COM')
tourl = "http://www.zoneedit.com/signUp.html"
else if (document.form.ddns_server_x.value == 'WWW.ASUS.COM')
tourl = "";
else
tourl = "https://controlpanel.tzo.com/cgi-bin/tzopanel.exe"
link = window.open(tourl, "DDNSLink",
"toolbar=yes,location=yes,directories=no,status=yes,menubar=yes,scrollbars=yes,resizable=yes,copyhistory=no,width=640,height=480");
}
else if (s=='x_NTPServer1')
{tourl = "http://support.ntp.org/bin/view/Servers/WebHome"
link = window.open(tourl, "NTPLink",
"toolbar=yes,location=yes,directories=no,status=yes,menubar=yes,scrollbars=yes,resizable=yes,copyhistory=no,width=640,height=480");
}
else if (s=='x_WImageSize')
{tourl = "Advanced_ShowTime_Widzard.asp"
link = window.open(tourl, "WebCamera",
"toolbar=yes,location=yes,directories=no,status=yes,menubar=yes,scrollbars=yes,resizable=yes,copyhistory=no,width=640,height=560");
}
else if (s=='x_WImageStatic')
{tourl = "ShowWebCamPic.asp"
link = window.open(tourl, "WebCamera",
"toolbar=yes,location=yes,directories=no,status=yes,menubar=yes,scrollbars=yes,resizable=yes,copyhistory=no,width=640,height=560");
}
else if (s=='x_WRemote')
{tourl = "Advanced_RemoteControl_Widzard.asp"
link = window.open(tourl, "RemoteMonitor",
"toolbar=no,location=no,directories=no,status=no,menubar=yes,scrollbars=yes,resizable=yes,copyhistory=no,top=0,left=0,width=" + screen.width + ",height=" + screen.height);
}
else if (s=='x_FIsAnonymous' || s=='x_FIsSuperuser')
{urlstr = location.href;
url = urlstr.indexOf("http://");
port = document.form.usb_ftpport_x.value;
if (url == -1) urlpref = LANIP;
else
{urlstr = urlstr.substring(7, urlstr.length);
url = urlstr.indexOf(":");
if (url!=-1)
{urlpref = urlstr.substring(0, url);
}
else
{url = urlstr.indexOf("/");
if (url!=-1) urlpref = urlstr.substring(0, url);
else urlpref = urlstr;
}
}
if (s=='x_FIsAnonymous')
{user = 'anonymous';
tourl = "ftp://" + urlpref;
}
else
{user = 'admin';
tourl = "ftp://" + user + "@" + urlpref;
}
if (port!=21) tourl = tourl + ":" + port;
link = window.open(tourl, "FTPServer",
"toolbar=yes,location=yes,directories=no,status=yes,menubar=yes,scrollbars=yes,resizable=yes,copyhistory=no,width=640,height=560");
}
if (!link.opener) link.opener = self;
}

function blur_body()
{alert("<#JS_focus#>");
}
/* Used when WEP is changed */
function changeWEPType()
{if (document.form.wl_wep.value == "0")
{flag = 0;
}
else
{flag = 1;
}
inputCtrl(document.form.wl_key1, flag);
inputCtrl(document.form.wl_key2, flag);
inputCtrl(document.form.wl_key3, flag);
inputCtrl(document.form.wl_key4, flag);
inputCtrl(document.form.wl_key, flag);
}
/* Used when Authenication Method is changed */
function changeAuthType()
{if (document.form.wl_auth_mode_x.value == "shared")
{inputCtrl(document.form.wl_crypto, 0);
inputCtrl(document.form.wl_wpa_psk, 0);
inputCtrl(document.form.wl_wep, 1);
inputCtrl(document.form.wl_key1, 1);
inputCtrl(document.form.wl_key2, 1);
inputCtrl(document.form.wl_key3, 1);
inputCtrl(document.form.wl_key4, 1);
inputCtrl(document.form.wl_key, 1);
inputCtrl(document.form.wl_wpa_gtk_rekey,  0);
}
else if (document.form.wl_auth_mode_x.value == "psk")
{inputCtrl(document.form.wl_crypto, 1);
inputCtrl(document.form.wl_wpa_psk, 1);
inputCtrl(document.form.wl_wep, 1);
inputCtrl(document.form.wl_key1, 1);
inputCtrl(document.form.wl_key2, 1);
inputCtrl(document.form.wl_key3, 1);
inputCtrl(document.form.wl_key4, 1);
inputCtrl(document.form.wl_key, 1);
inputCtrl(document.form.wl_wpa_gtk_rekey,  0);
}
else if (document.form.wl_auth_mode_x.value == "wpa")
{inputCtrl(document.form.wl_crypto, 0);
inputCtrl(document.form.wl_wpa_psk, 0);
inputCtrl(document.form.wl_wep, 0);
inputCtrl(document.form.wl_key1, 0);
inputCtrl(document.form.wl_key2, 0);
inputCtrl(document.form.wl_key3, 0);
inputCtrl(document.form.wl_key4, 0);
inputCtrl(document.form.wl_key, 0);
inputCtrl(document.form.wl_wpa_gtk_rekey,  0);
}
else if (document.form.wl_auth_mode_x.value == "radius")
{inputCtrl(document.form.wl_crypto, 1);
inputCtrl(document.form.wl_wpa_psk, 1);
inputCtrl(document.form.wl_wep, 1);
inputCtrl(document.form.wl_key1, 1);
inputCtrl(document.form.wl_key2, 1);
inputCtrl(document.form.wl_key3, 1);
inputCtrl(document.form.wl_key4, 1);
inputCtrl(document.form.wl_key, 1);
inputCtrl(document.form.wl_wpa_gtk_rekey,  0);
}
else
{inputCtrl(document.form.wl_crypto, 0);
inputCtrl(document.form.wl_wpa_psk, 0);
inputCtrl(document.form.wl_wep, 1);
inputCtrl(document.form.wl_key1, 1);
inputCtrl(document.form.wl_key1, 1);
inputCtrl(document.form.wl_key1, 1);
inputCtrl(document.form.wl_key1, 1);
inputCtrl(document.form.wl_key1, 1);
inputCtrl(document.form.wl_key, 1);
inputCtrl(document.form.wl_wpa_gtk_rekey,  0);
}
}
/* input : s: service id, v: value name, o: current value */
/* output: wep key1~4       */
function is_wlphrase(s, v, o){
	var pseed = new Array;
	var wep_key = new Array(5);
	
	if(v=='wl_wpa_psk')
		return(valid_WPAPSK(o));
	
	if(document.form.current_page.value == "Advanced_WirelessGuest_Content.asp"){
		wepType = document.form.wl_guest_wep_x_1.value;
		wepKey1 = document.form.wl_guest_key1_1;
		wepKey2 = document.form.wl_guest_key2_1;
		wepKey3 = document.form.wl_guest_key3_1;
		wepKey4 = document.form.wl_guest_key4_1;
	}
	else{	// note: current_page == "Advanced_Wireless_Content.asp"
		wepType = document.form.wl_wep_x.value;
		wepKey1 = document.form.wl_key1;
		wepKey2 = document.form.wl_key2;
		wepKey3 = document.form.wl_key3;
		wepKey4 = document.form.wl_key4;
	}
	
	phrase = o.value;
	if(wepType == "1"){
		for(var i = 0; i < phrase.length; i++){
			pseed[i%4] ^= phrase.charCodeAt(i);
		}
		
		randNumber = pseed[0] | (pseed[1]<<8) | (pseed[2]<<16) | (pseed[3]<<24);
		for(var j = 0; j < 5; j++){
			randNumber = ((randNumber*0x343fd)%0x1000000);
			randNumber = ((randNumber+0x269ec3)%0x1000000);
			wep_key[j] = ((randNumber>>16)&0xff);
		}
		
		wepKey1.value = binl2hex_c(wep_key);
		for(var j = 0; j < 5; j++){
			randNumber = ((randNumber * 0x343fd) % 0x1000000);
			randNumber = ((randNumber + 0x269ec3) % 0x1000000);
			wep_key[j] = ((randNumber>>16) & 0xff);
		}
		
		wepKey2.value = binl2hex_c(wep_key);
		for(var j = 0; j < 5; j++){
			randNumber = ((randNumber * 0x343fd) % 0x1000000);
			randNumber = ((randNumber + 0x269ec3) % 0x1000000);
			wep_key[j] = ((randNumber>>16) & 0xff);
		}
		
		wepKey3.value = binl2hex_c(wep_key);
		for(var j = 0; j < 5; j++){
			randNumber = ((randNumber * 0x343fd) % 0x1000000);
			randNumber = ((randNumber + 0x269ec3) % 0x1000000);
			wep_key[j] = ((randNumber>>16) & 0xff);
		}
		
		wepKey4.value = binl2hex_c(wep_key);
	}
	else if(wepType == "2" || wepType == "3"){
		password = "";
		
		if(phrase.length > 0){
			for(var i = 0; i < 64; i++){
				ch = phrase.charAt(i%phrase.length);
				password = password+ch;
			}
		}
		
		password = calcMD5(password);
		if(wepType == "2"){
			wepKey1.value = password.substr(0, 26);
		}
		else{
			wepKey1.value = password.substr(0, 32);
		}
		
		wepKey2.value = wepKey1.value;
		wepKey3.value = wepKey1.value;
		wepKey4.value = wepKey1.value;
	}
	
	return true;
}

function wl_wep_change(){
	var mode = document.form.wl_auth_mode_x.value;
	var wep = document.form.wl_wep_x.value;
	
	if(mode == "psk" || mode == "wpa" || mode == "wpa2" ){
		if(mode != "wpa" && mode != "wpa2"){
			inputCtrl(document.form.wl_crypto, 1);
			inputCtrl(document.form.wl_wpa_psk, 1);
		}
		inputCtrl(document.form.wl_wpa_gtk_rekey, 1);
		inputCtrl(document.form.wl_wep_x, 0);
		inputCtrl(document.form.wl_key1, 0);
		inputCtrl(document.form.wl_key2, 0);
		inputCtrl(document.form.wl_key3, 0);
		inputCtrl(document.form.wl_key4, 0);
		inputCtrl(document.form.wl_key, 0);
	}
	else if(mode == "radius"){
		inputCtrl(document.form.wl_crypto, 0);
		inputCtrl(document.form.wl_wpa_psk, 0);
		inputCtrl(document.form.wl_wpa_gtk_rekey, 0);
		inputCtrl(document.form.wl_wep_x, 0);
		inputCtrl(document.form.wl_key1, 0);
		inputCtrl(document.form.wl_key2, 0);
		inputCtrl(document.form.wl_key3, 0);
		inputCtrl(document.form.wl_key4, 0);
		inputCtrl(document.form.wl_key, 0);
	}
	else{
		inputCtrl(document.form.wl_crypto, 0);
		inputCtrl(document.form.wl_wpa_psk, 0);
		inputCtrl(document.form.wl_wpa_gtk_rekey, 0);
		inputCtrl(document.form.wl_wep_x, 1);
		if(wep != "0"){
			inputCtrl(document.form.wl_key1, 1);
			inputCtrl(document.form.wl_key2, 1);
			inputCtrl(document.form.wl_key3, 1);
			inputCtrl(document.form.wl_key4, 1);
			inputCtrl(document.form.wl_key, 1);
		}
		else{
			inputCtrl(document.form.wl_key1, 0);
			inputCtrl(document.form.wl_key2, 0);
			inputCtrl(document.form.wl_key3, 0);
			inputCtrl(document.form.wl_key4, 0);
			inputCtrl(document.form.wl_key, 0);
		}
	}
	
	change_key_des();
}

function change_wep_type(mode, isload){
	var cur_wep = document.form.wl_wep_x.value;
	var wep_type_array;
	var value_array;
	
	free_options(document.form.wl_wep_x);
	
	if(mode == "shared"){
		wep_type_array = new Array("WEP-64bits", "WEP-128bits");
		value_array = new Array("1", "2");
	}
	else{
		wep_type_array = new Array("None", "WEP-64bits", "WEP-128bits");
		value_array = new Array("0", "1", "2");
	}

	add_options_x2(document.form.wl_wep_x, wep_type_array, value_array, cur_wep);
	
	if(mode == "psk" || mode == "psk2" || mode == "pskpsk2" || mode == "wpa" || mode == "wpa2" || mode == "wpawpa2" || mode == "radius")
		document.form.wl_wep_x.value = "0";
	change_wlweptype(document.form.wl_wep_x, "WLANConfig11b", isload);
}

function wl_auth_mode_reconf(){
	if(document.form.wl_auth_mode_x.value == "radius" || document.form.wl_auth_mode_x.value == "wpa" || document.form.wl_auth_mode_x.value == "wpa2" || document.form.wl_auth_mode_x.value == "wpawpa2")
		document.form.wl_auth_mode_x.value = "open";
	
	for(var i = 5; i <= 8; ++i){
		document.form.wl_auth_mode_x.options[i].value = null;
		document.form.wl_auth_mode_x.options[i] = null;
	}
}

function show_DFS_hint(o){
	var hint_obj = document.getElementById("wl_channel_hint");
	if (hint_obj) {
		if (o.value >= 52 && o.value <= 140)
			hint_obj.style.display = "";
		else
			hint_obj.style.display = "none";
	}
}

function insertChannelOption(){
	var wl_ch_obj = document.form.wl_channel;
	var org_ch = wl_ch_obj.value;
	add_options(wl_ch_obj, wl_ch_vitems, org_ch);
	wl_ch_obj.options[0].text = "Auto";
	if (document.form.wl_bw.value != "0") {//for 20/40, 40MHz
		for (var i = wl_ch_vitems.length-1; i >= 0; i--) {
			if (wl_ch_obj.options[i].value == "116" || wl_ch_obj.options[i].value == "165")
				wl_ch_obj.remove(i);
		}
	}
	show_DFS_hint(wl_ch_obj);
}

function insertExtChannelOption(){
	if(wl_unit_var == '1')
		insertExtChannelOption_5g();
	else
		insertExtChannelOption_2g();
}

function insertExtChannelOption_5g(){
	var wmode = document.form.wl_nmode_x.value;
	if ((wmode == "0"|| wmode == "1") && document.form.wl_bw.value != "0") {
		var x = document.form.wl_nctrlsb;
		inputCtrl(document.form.wl_nctrlsb, 1);
		free_options(x);
		add_a_option(x, "auto", "Auto");
		x.selectedIndex = 0;
	}
	else
		inputCtrl(document.form.wl_nctrlsb, 0);
}

function insertExtChannelOption_2g(){
	var wmode = document.form.wl_nmode_x.value;
	var CurrentCh = document.form.wl_channel.value;
	var option_length = document.form.wl_channel.options.length;
	if ((wmode == "0"|| wmode == "1") && document.form.wl_bw.value != "0") {
		inputCtrl(document.form.wl_nctrlsb, 1);
		var x = document.form.wl_nctrlsb;
		var length = document.form.wl_nctrlsb.options.length;
		if (length > 1) {
			x.selectedIndex = 1;
			x.remove(x.selectedIndex);
		}
		if ((CurrentCh >=1) && (CurrentCh <= 4)) {
			x.options[0].text = "Lower";
			x.options[0].value = "lower";
		} else if ((CurrentCh >= 5) && (CurrentCh <= 7)) {
			x.options[0].text = "Lower";
			x.options[0].value = "lower";
			add_a_option(document.form.wl_nctrlsb, "upper", "Upper");
			if (document.form.wl_nctrlsb_old.value == "upper")
				document.form.wl_nctrlsb.options.selectedIndex = 1;
		} else if ((CurrentCh >= 8) && (CurrentCh <= 9)) {
			x.options[0].text = "Upper";
			x.options[0].value = "upper";
			if (option_length >=14) {
				add_a_option(document.form.wl_nctrlsb, "lower", "Lower");
				if (document.form.wl_nctrlsb_old.value == "lower")
					document.form.wl_nctrlsb.options.selectedIndex = 1;
			}
		} else if (CurrentCh == 10) {
			x.options[0].text = "Upper";
			x.options[0].value = "upper";
			if (option_length > 14) {
				add_a_option(document.form.wl_nctrlsb, "lower", "Lower");
				if (document.form.wl_nctrlsb_old.value == "lower")
					document.form.wl_nctrlsb.options.selectedIndex = 1;
			}
		} else if (CurrentCh >= 11) {
			x.options[0].text = "Upper";
			x.options[0].value = "upper";
		} else {
			x.options[0].text = "Auto";
			x.options[0].value = "auto";
		}
	} else
		inputCtrl(document.form.wl_nctrlsb, 0);
}

function wl_auth_mode_change(isload){
	var mode = document.form.wl_auth_mode_x.value;
	var i, cur, algos;
	inputCtrl(document.form.wl_wep_x,  1);
	
	/* enable/disable crypto algorithm */
	if(mode == "wpa" || mode == "wpa2" || mode == "wpawpa2" || mode == "psk" || mode == "psk2" || mode == "pskpsk2")
		inputCtrl(document.form.wl_crypto,  1);
	else
		inputCtrl(document.form.wl_crypto,  0);
	
	/* enable/disable psk passphrase */
	if(mode == "psk" || mode == "psk2" || mode == "pskpsk2")
		inputCtrl(document.form.wl_wpa_psk,  1);
	else
		inputCtrl(document.form.wl_wpa_psk,  0);

	/* update wl_crypto */
	if(mode == "psk" || mode == "psk2" || mode == "pskpsk2"){
		/* Save current crypto algorithm */
		for(var i = 0; i < document.form.wl_crypto.length; i++){
			if(document.form.wl_crypto[i].selected){
				cur = document.form.wl_crypto[i].value;
				break;
			}
		}

		opts = document.form.wl_auth_mode_x.options;

		if(opts[opts.selectedIndex].text == "WPA-Personal")
			algos = new Array("TKIP");
		else if(opts[opts.selectedIndex].text == "WPA2-Personal")
			algos = new Array("AES");
		else
			algos = new Array("AES", "TKIP+AES");

		/* Reconstruct algorithm array from new crypto algorithms */
		document.form.wl_crypto.length = algos.length;
		for(i=0; i<algos.length; i++){
			document.form.wl_crypto[i] = new Option(algos[i], algos[i].toLowerCase());
			document.form.wl_crypto[i].value = algos[i].toLowerCase();
			
			if(algos[i].toLowerCase() == cur)
				document.form.wl_crypto[i].selected = true;
		}
	}
	else if(mode == "wpa" || mode == "wpawpa2"){
		for(var i = 0; i < document.form.wl_crypto.length; i++){
			if(document.form.wl_crypto[i].selected){
				cur = document.form.wl_crypto[i].value;
				break;
			}
		}
		
		opts = document.form.wl_auth_mode_x.options;
		if(opts[opts.selectedIndex].text == "WPA-Enterprise")
			algos = new Array("TKIP");
		else
			algos = new Array("AES", "TKIP+AES");
			//algos = new Array("TKIP", "AES", "TKIP+AES");
		
		document.form.wl_crypto.length = algos.length;
		for(i=0; i<algos.length; i++){
			document.form.wl_crypto[i] = new Option(algos[i], algos[i].toLowerCase());
			document.form.wl_crypto[i].value = algos[i].toLowerCase();
			
			if(algos[i].toLowerCase() == cur)
				document.form.wl_crypto[i].selected = true;
		}
	}
	else if(mode == "wpa2"){
		for(var i = 0; i < document.form.wl_crypto.length; i++){
			if(document.form.wl_crypto[i].selected){
				cur = document.form.wl_crypto[i].value;
				break;
			}
		}
		
		algos = new Array("AES");
		
		document.form.wl_crypto.length = algos.length;
		for(i=0; i<algos.length; i++){
			document.form.wl_crypto[i] = new Option(algos[i], algos[i].toLowerCase());
			document.form.wl_crypto[i].value = algos[i].toLowerCase();
			
			if(algos[i].toLowerCase() == cur)
				document.form.wl_crypto[i].selected = true;
		}
	}
	
	change_wep_type(mode, isload);
	
	/* Save current network key index */
	for(var i = 0; i < document.form.wl_key.length; i++){
		if(document.form.wl_key[i].selected){
			cur = document.form.wl_key[i].value;
			break;
		}
	}
	
	/* Define new network key indices */
	if(mode == "wpa" || mode == "wpa2" || mode == "wpawpa2" || mode == "psk" || mode == "psk2" || mode == "pskpsk2" || mode == "radius")
		algos = new Array("1", "2", "3", "4");
	else{
		algos = new Array("1", "2", "3", "4");
		if(!isload)
			cur = "1";
	}
	
	/* Reconstruct network key indices array from new network key indices */
	document.form.wl_key.length = algos.length;
	for(i=0; i<algos.length; i++){
		document.form.wl_key[i] = new Option(algos[i], algos[i]);
		document.form.wl_key[i].value = algos[i];
		if(algos[i] == cur)
			document.form.wl_key[i].selected = true;
	}
	
	wl_wep_change();
}

function masq_wepkey()
{wep1 = document.form.wl_key1.value;
wep2 = document.form.wl_key2.value;
wep3 = document.form.wl_key3.value;
wep4 = document.form.wl_key4.value;
if (wep1.length == 10)
{wep = "**********";
}
else if (wep1.length == 26)
{wep = "**************************";
}
else wep = "";
document.form.wl_key1.value = wep;
document.form.wl_key2.value = wep;
document.form.wl_key3.value = wep;
document.form.wl_key4.value = wep;
}
function unmasq_wepkey()
{if (document.form.wl_key1.value.indexOf("**********") != -1)
document.form.wl_key1.value = wep1;
if (document.form.wl_key2.value.indexOf("**********") != -1)
document.form.wl_key2.value = wep2;
if (document.form.wl_key3.value.indexOf("**********") != -1)
document.form.wl_key3.value = wep3;
if (document.form.wl_key4.value.indexOf("**********") != -1)
document.form.wl_key4.value = wep4;
}

function showhide(element, sh)
{
	var status;
	if (sh == 1){
		status = "";
	}
	else{
		status = "none"
	}
	
	if(document.getElementById){
		document.getElementById(element).style.display = status;
	}
	else if (document.all){
		document.all[element].style.display = status;
	}
	else if (document.layers){
		document.layers[element].display = status;
	}
}

var pageChanged = 0;
var pageChangedCount = 0;

function is_number_sp(event, o){
	keyPressed = event.keyCode ? event.keyCode : event.which ? event.which : event.charCode;
	if (nav)
	{
		if (	(keyPressed==8)||	// backspace
			(keyPressed==35)||	// end
			(keyPressed==36)||	// home
			(keyPressed==37)||	// <-
			(keyPressed==39)||	// ->
			(keyPressed==46)	// delete
		)
			keyPressed=0;
	}

	if (keyPressed==0)
		return true;
	if (keyPressed>47 && keyPressed<58)
	{
		if (keyPressed==48 && o.length == 0) return false;
		return true;
	}
	else
	{
		return false;
	}
}

/*Viz 2011.03 for Input value check, start*/
function valid_string_domain(obj, flag){
	
	if(flag==1){
		var re = new RegExp("[^a-zA-Z0-9-_]+", "gi");		// ,"g" whole erea match & "i" Ignore Letter
		if(re.test(obj.value)){
			alert("<#JS_validchar#>");
    			obj.focus();
    			obj.select();
    			return false;
  		}else{}
	}
	
	return true;	
}		

function valid_IP_form(obj, flag){
	if(obj.value == ""){
			return true;
	}else if(flag==0){	//without netMask
			if(!validate_ipaddr_final(obj, obj.name)){
				obj.focus();
				obj.select();		
				return false;	
			}else
				return true;
	}else if(flag==1){	//with netMask						
			var strIP = obj.value;
			//var re=/^(\d+)\.(\d+)\.(\d+)\.(\d+)$/g;
			var re = new RegExp("^([0-9]{1,3})\.([0-9]{1,3})\.([0-9]{1,3})\.([0-9]{1,3})$", "gi");			
			if(!validate_ipaddr_final(obj, obj.name)){
				obj.focus();
				obj.select();		
				return false;	
			}else if(obj.name=="sr_ipaddr_x_0" && re.test(strIP)){
					if((RegExp.$1 > 0) && (RegExp.$1 < 127)) document.form.sr_netmask_x_0.value = "255.0.0.0";
					else if ((RegExp.$1 > 127) && (RegExp.$1 < 192)) document.form.sr_netmask_x_0.value = "255.255.0.0";
					else if ((RegExp.$1 > 191) && (RegExp.$1 < 224)) document.form.sr_netmask_x_0.value = "255.255.255.0";
					else document.form.sr_netmask_x_0.value = "0.0.0.0";												
			}else if(obj.name=="wan_ipaddr_x" && re.test(strIP)){
					if((RegExp.$1 > 0) && (RegExp.$1 < 127)) document.form.wan_netmask_x.value = "255.0.0.0";
					else if ((RegExp.$1 > 127) && (RegExp.$1 < 192)) document.form.wan_netmask_x.value = "255.255.0.0";
					else if ((RegExp.$1 > 191) && (RegExp.$1 < 224)) document.form.wan_netmask_x.value = "255.255.255.0";
					else document.form.wan_netmask_x.value = "0.0.0.0";												
			}			
	}else
		return false;
}

function check_hwaddr(obj){
	if(obj.value == ""){
			return true;
	}else{
		var hwaddr = new RegExp("(([a-fA-F0-9]{2}(\:|$)){6})", "gi");		// ,"g" whole erea match & "i" Ignore Letter		
		if(!hwaddr.test(obj.value)){		
			alert("<#LANHostConfig_ManualDHCPMacaddr_itemdesc#>");    		
    	obj.focus();
    	obj.select();
    	return false;
  	}
		else
			return true;
  }
}

function valid_IPorMAC(obj){	
	var hwaddr = new RegExp("(([a-fA-F0-9]{2}(\:|$)){6})", "gi");		// ,"g" whole erea match & "i" Ignore Letter		
	
	if(obj.value == ""){
			return true;
	}else if(hwaddr.test(obj.value)){
			return true;
	}else if(validate_ipaddr_final(obj, obj.name)){
			return true;
	}else{		
    			obj.focus();
    			obj.select();
    			return false;
	}	
}

function check_hwaddr_flag(obj){  //check_hwaddr() remove alert()
	if (obj.value == "") {
		return 0;
	} else {
		var hwaddr = new RegExp("(([a-fA-F0-9]{2}(\:|$)){6})", "gi");
		var legal_hwaddr = new RegExp("(^([a-fA-F0-9][aAcCeE02468])(\:))", "gi"); // for legal MAC, unicast & globally unique (OUI enforced)

		if (!hwaddr.test(obj.value))
			return 1;
		else if (!legal_hwaddr.test(obj.value))
			return 2;
		else
			return 0;
	}
}

function validate_number_range(obj, mini, maxi){
	var PortRange = obj.value;
	var rangere=new RegExp("^([0-9]{1,5})\:([0-9]{1,5})$", "gi");

	if(rangere.test(PortRange)){
			if(parseInt(RegExp.$1) >= parseInt(RegExp.$2)){
				alert("<#JS_validport#>");	
				obj.focus();
				obj.select();		
				return false;												
			}
			else{
				if(!validate_each_port(obj, RegExp.$1, mini, maxi) || !validate_each_port(obj, RegExp.$2, mini, maxi)){
					obj.focus();
					obj.select();
					return false;											
				}
				return true;								
			}
	}
	else{
		if(!validate_range(obj, mini, maxi)){		
			obj.focus();
			obj.select();
			return false;
		}
		return true;	
	}	
}
	
function validate_each_port(o, num, min, max) {	
	if(num<min || num>max) {
		alert("<#JS_validport#>");
		return false;
	}else {
		//o.value = str2val(o.value);
		if(o.value=="")
			o.value="0";
		return true;
	}
}
/*Viz 2011.03 for Input value check,  end */

function show_band_select_option(){
	if (band5g_support != -1 && '<% nvram_get("re_expressway"); %>' == "0")
		document.getElementById("wl_unit_tr").style.display="";
}

function open_wifi_hint(v){
	if (wl_unit_var == "0") {
		if (v == "wl_nmode_x")
			openHint(0, 4);
	}
	else {
		if (v == "wl_nmode_x") {
			if (band5g_11ac_support != -1)
				openHint(0, 24);
			else
				openHint(0, 16);
		}
	}
}

function need_action_wait(as, s){
	if (as.value.indexOf("restart_wifi") >= 0) {
		if (s == "2") {
			var _sta_freq = '<% nvram_get("sta_freq"); %>';
			if ((as.value == "restart_wifi0" && _sta_freq == "2.4") 
					|| (as.value == "restart_wifi1" && _sta_freq == "5"))
				document.form.action_wait.value = "20";
			else
				document.form.action_wait.value = "10";
		} else
			document.form.action_wait.value = "10";
	} else if (as.value == "restart_reboot") {
		if (s == "2" || sw_mode == "3")
			document.form.action_wait.value = "120";
		else if (s == "4")
			document.form.action_wait.value = "60";
	}
}

function need_action_script(as){
	as.value = as.value+document.form.wl_unit.value;
}
