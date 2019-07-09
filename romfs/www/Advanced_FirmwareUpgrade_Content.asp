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
<title><#Web_Title#> - <#menu4_3#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<style>
.Bar_container{
	width:85%;
	height:21px;
	border:2px inset #999;
	margin:0 auto;
	background-color:#FFFFFF;
	z-index:100;
}
#proceeding_img_text{
	position:absolute; z-index:101; font-size:11px; color:#000000; margin-left:110px; line-height:21px;
}
#proceeding_img{
 	height:21px;
	background:#C0D1D3 url(/images/proceeding_img.gif);
}
</style>

<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script type="text/javascript" language="JavaScript" src="/help.js"></script>
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script language="JavaScript" type="text/javascript" src="/jquery.js"></script>
<script language="JavaScript" type="text/javascript" src="/ajax.js"></script>
<script type="text/javascript" src="/detect.js"></script>
<script>
var $j = jQuery.noConflict();	
wan_route_x = '<% nvram_get("wan_route_x"); %>';
wan_nat_x = '<% nvram_get("wan_nat_x"); %>';
wan_proto = '<% nvram_get("wan_proto"); %>';

var varload = 0;
var count=0;

function initial(){
	show_menu();
	if (live_update_support != -1)
		document.getElementById("FW_update").style.display = "";
}

function WirelessClient_Alert(){
	if(isWLclient())
		alert("<#DrSurf_sweet_advise5#>");
}

function restore_ui(){
	setTimeout("hideLoading();",1000);
}

function detect_firmware(){
	$j.ajax({
		url: '/ajax_detect_firmware.asp',
		dataType: 'script',
		data: 'webs_cmd=start_webs_update',
		error: function(xhr) {
			setTimeout("detect_firmware();", 1000);
		},
		success: function(response) {
			setTimeout("check_firmver();", 3000);
			document.getElementById("FW_up_prompt_str").innerHTML = "<#QIS_fwup_hint1#>";
			document.getElementById("FW_up_prompt_img").style.display = "";
			document.getElementById("FW_up_prompt").style.display = "";
		}
	});
}

var retry = 0;
var current_firmver = "";
function check_firmver(){
	$j.ajax({
		url: '/ajax_detect_firmware_status.asp',
		dataType: 'script',
		error: function(xhr) {
			setTimeout("check_firmver();", 1000);
		},
		success: function(response) {
			if(retry > 60){ // check 60 sec for download latest firmware version
				document.getElementById("FW_up_prompt_str").innerHTML = "<#FW_fail_desc#>";
				document.getElementById("FW_up_prompt_img").style.display = "none";
				retry = 0;
			}
			else if(webs_state_update == 0){
				setTimeout("check_firmver();", 1000);
				retry++;
			}
			else{
				if(webs_state_error != 1 && webs_state_info.length > 0){
					var tmp = document.form.firmver.value.split(".");
					current_firmver = tmp[0]+tmp[1]+tmp[2]+tmp[3];
					if(webs_state_info > current_firmver){
						if(confirm("<#QIS_fwup_desc1#>"))
							setTimeout("doLiveUpdate();", 1000);
						document.getElementById("FW_up_prompt").style.display = "none";
					}
					else
						document.getElementById("FW_up_prompt_str").innerHTML = "<#QIS_fwup_hint2#>";
				}
				else
					document.getElementById("FW_up_prompt_str").innerHTML = "<#FW_fail_desc#>";
				document.getElementById("FW_up_prompt_img").style.display = "none";
			}
		}
	});
}

function doLiveUpdate(){
	$j.ajax({
		url: '/ajax_detect_firmware.asp',
		dataType: 'script',
		data: 'webs_cmd=start_webs_upgrade',
		error: function(xhr) {
			setTimeout("doLiveUpdate();", 1000);
		},
		success: function(response) {
			document.getElementById("drword").innerHTML = "&nbsp;&nbsp;&nbsp;<#QIS_fwup_hint4#>&nbsp;&nbsp;<img src='images/InternetScan.gif'>";
			document.getElementById("drword").style.fontSize = "16px";
			dr_advise();
			retry = 0;
			setTimeout("check_fwupgrade();", 1000);
		}
	});
}

function check_fwupgrade(){
	$j.ajax({
		url: '/ajax_detect_firmware_status.asp',
		dataType: 'script',
		error: function(xhr) {
			setTimeout("check_fwupgrade();", 1000);
		},
		success: function(response) {
			if(retry > 300){ // check 300 sec for download latest firmware
				document.getElementById("hiddenMask").style.visibility = "hidden";
				document.getElementById("FW_up_prompt_str").innerHTML = "<#FW_fail_desc#>";
				document.getElementById("FW_up_prompt").style.display = "";
				stopLiveUpdate();
			}
			else if(webs_state_upgrade == 0){
				setTimeout("check_fwupgrade();", 1000);
				retry++;
			}
			else{
				document.getElementById("hiddenMask").style.visibility = "hidden";
				if(webs_state_error != 1){
					if(webs_state_upgrade == 1){
						showLoadingBar(180);
						setTimeout("detect_domain_url();", 182000);
					}
				}
				else{	// download fail
					document.getElementById("FW_up_prompt_str").innerHTML = "<#FW_fail_desc#>";
					document.getElementById("FW_up_prompt").style.display = "";
				}
			}
		}
	});
}

function detect_domain_url(){
	$j.ajax({
		url: 'http://repeater.asus.com/index.asp',
		dataType: 'script',
		cache:false,
		error: function(xhr) {
			setTimeout("detect_domain_url();", 1000);
		},
		success: function(response) {
			location.href = "index.asp";
		}
	});
}

function stopLiveUpdate(){
	$j.ajax({
		url: '/ajax_detect_firmware.asp',
		dataType: 'script',
		data: 'webs_cmd=stop_webs_upgrade',
		error: function(xhr) {
			setTimeout("stopLiveUpdate();", 1000);
		},
		success: function(response) {
		}
	});
}
</script>
</head>

<body onload="initial();">
<div id="TopBanner"></div>

<div id="LoadingBar" class="popup_bar_bg">
<table cellpadding="5" cellspacing="0" id="loadingBarBlock" class="loadingBarBlock" align="center">
	<tr>
		<td height="80">
		<div class="Bar_container">
			<span id="proceeding_img_text" ></span>
			<div id="proceeding_img"></div>
		</div>
		<div style="margin:5px auto; width:85%;"><#FW_ok_desc#></div>
		</td>
	</tr>
</table>
<!--[if lte IE 6.5]><iframe class="hackiframe"></iframe><![endif]-->
</div>
<div id="Loading" class="popup_bg"></div><!--for uniform show, useless but have exist-->
<div id="hiddenMask" class="popup_bg">
	<table cellpadding="5" cellspacing="0" id="dr_sweet_advise" class="dr_sweet_advise" align="center">
		<tr>
		<td>
			<br/>
			<div class="drword" id="drword" style="height:50px;">&nbsp;&nbsp;&nbsp;<#Main_alert_proceeding_desc1#>...
			<br/>
			<br/>
	    </div>
		  <!--div class="drImg"><img src="/images/DrsurfImg.gif"></div-->
		</td>
		</tr>
	</table>
<!--[if lte IE 6.5]><iframe class="hackiframe"></iframe><![endif]-->
</div>
<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>

<form method="post" action="upgrade.cgi" name="form" target="hidden_frame" enctype="multipart/form-data">
<input type="hidden" name="current_page" value="Advanced_FirmwareUpgrade_Content.asp">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="action_mode" value=" Apply ">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="action_wait" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">

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
		<td align="left" valign="top" >
          
		<table width="760px" border="0" cellpadding="5" cellspacing="0" class="FormTitle" id="FormTitle">
		<tbody>
		<tr>
		  <td bgcolor="#4D595D" valign="top"  >
		  <div>&nbsp;</div>
		  <div class="formfonttitle"><#menu4#> - <#menu4_3#></div>
		  <div style="margin-left:5px;margin-top:10px;margin-bottom:10px"><img src="/images/New_ui/export/line_export.png"></div>
		  <div class="formfontdesc"><#FW_desc1#></div>

		<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
			<tr>
				<th><#FW_item1#></th>
				<td><input type="text" class="input_15_table" value="<% nvram_get("fac_model_name"); %>" readonly="1"></td>
			</tr>
			<tr>
				<th><#FW_item2#></th>
				<td>
					<input type="text" name="firmver_table" class="input_15_table" value="<% nvram_get("firmver"); %>" readonly="1">
					<input type="button" name="FW_update" id="FW_update" class="button_gen" onclick="detect_firmware();" value="<#CTL_check#>" style="display:none;"/>
					<div id="FW_up_prompt" style="color:#FFCC00;display:none;">
					  <span id="FW_up_prompt_str"></span>
					  <img id="FW_up_prompt_img" src="images/InternetScan.gif">
					</div>
				</td>
			</tr>
			<tr>
				<th><#FW_item5#></th>
				<td><input type="file" name="file" class="input" style="color:#FFCC00;"></td>
			</tr>
			<tr align="center">
			  <td colspan="2"><input type="button" name="button" class="button_gen" onclick="onSubmitCtrlOnly(this, 'Upload1');WirelessClient_Alert();" value="<#CTL_upload#>" /></td>
			</tr>
			<tr>
				<td colspan="2">
				<strong><#FW_note#></strong>
				<ol>
					<li><#FW_n1#></li>
					<li><#FW_n2#></li>
				</ol>
				</td>
			</tr>															
		</table>
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
<div id="footer"></div>
</form>

<form method="post" name="start_update" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">	
<input type="hidden" name="current_page" value="Advanced_FirmwareUpgrade_Content.asp">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="action_wait" value="">	
</form>
<form name="hint_form"></form>
</body>
</html>
