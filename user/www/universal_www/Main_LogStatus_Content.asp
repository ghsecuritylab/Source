<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="X-UA-Compatible" content="IE=EmulateIE7">
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title><#Web_Title#> - <#menu5_1#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">

<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script language="JavaScript" type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/jquery.js"></script>
<script>

var $j = jQuery.noConflict();
wan_route_x = '<% nvram_get("wan_route_x"); %>';
wan_nat_x = '<% nvram_get("wan_nat_x"); %>';
wan_proto = '<% nvram_get("wan_proto"); %>';

function initial(){
	show_menu();
	hidden_System_Time();
	showclock();
	showbootTime();
}

function hidden_System_Time(){
	if(sw_mode == 4){
		document.getElementById("System_Time_item").style.display="none";
	}
	else{
		document.getElementById("System_Time_item").style.display="";
	}
}

function showclock(){
	JS_timeObj.setTime(systime_millsec);
	systime_millsec += 1000;
	JS_timeObj2 = JS_timeObj.toString();	
	JS_timeObj2 = JS_timeObj2.substring(0,3) + ", " +
	              JS_timeObj2.substring(4,10) + "  " +
				  checkTime(JS_timeObj.getHours()) + ":" +
				  checkTime(JS_timeObj.getMinutes()) + ":" +
				  checkTime(JS_timeObj.getSeconds()) + "  " +
				  /*JS_timeObj.getFullYear() + " GMT" +
				  timezone;*/ // Viz remove GMT timezone 2011.08
				  JS_timeObj.getFullYear();
	$("system_time").value = JS_timeObj2;
	setTimeout("showclock()", 1000);
	if(navigator.appName.indexOf("Microsoft") >= 0)
		document.getElementById("textarea").style.width = "99%";
    //$("banner3").style.height = "13";
}

function showbootTime(){
	Days = Math.floor(boottime / (60*60*24));	
	Hours = Math.floor((boottime / 3600) % 24);
	Minutes = Math.floor(boottime % 3600 / 60);
	Seconds = Math.floor(boottime % 60);
	
	$("boot_days").innerHTML = Days;
	$("boot_hours").innerHTML = Hours;
	$("boot_minutes").innerHTML = Minutes;
	$("boot_seconds").innerHTML = Seconds;
	boottime += 1;
	setTimeout("showbootTime()", 1000);
}
</script>
</head>

<body onload="initial();" onunLoad="return unload_body();">
<div id="TopBanner"></div>

<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>

<form method="post" name="form" action="apply.cgi" target="hidden_frame">
<input type="hidden" name="current_page" value="Main_LogStatus_Content.asp">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="action_wait" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
</form>
<table class="content" align="center" cellpadding="0" cellspacing="0">
	<tr>
		<td width="17">&nbsp;</td>
    	<td valign="top" width="202">
			<div id="mainMenu"></div>
			<div id="subMenu"></div>
		</td>
		
		<td valign="top">
			<div id="tabMenu" class="submenuBlock"></div>
			
			<table width="98%" border="0" align="left" cellpadding="0" cellspacing="0">
				<tr>
					<td align="left" valign="top">
						
			<table width="760px" border="0" cellpadding="5" cellspacing="0" bordercolor="#6b8fa3"  class="FormTitle" id="FormTitle">		
			<tr>
		  		<td bgcolor="#4D595D" colspan="3" valign="top">
		  			<div>&nbsp;</div>
		  			<div class="formfonttitle"><#menu5#> - <#menu5_1#></div>
		  			<div style="margin-left:5px;margin-top:10px;margin-bottom:10px"><img src="/images/New_ui/export/line_export.png"></div>
		  			<div class="formfontdesc"><#MainLog_GeneralLog_title#></div>

						<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
							<tr id="System_Time_item"  display="">
								<th><#General_x_SystemTime_itemname#></th>
								<td>
									<input type="text" id="system_time" name="system_time" size="40" class="devicepin" value="" readonly="1" style="font-size:12px;">
								</td>										
							</tr>
							<tr>
								<th><!--a class="hintstyle" href="javascript:void(0);" onClick="openHint(12, 1);"--><#General_x_SystemUpTime_itemname#></a></th>
								<td><span id="boot_days"></span> <#General_Day#> <span id="boot_hours"></span> <#General_Hour#> <span id="boot_minutes"></span> <#General_Minute#> <span id="boot_seconds"></span> <#General_Second#></td>
							</tr>
						</table>
						<div style="margin-top:8px">
							<textarea cols="63" rows="27" wrap="off" readonly="readonly" id="textarea" style="width:99%; height:250px; font-family:'Courier New', Courier, mono; font-size:11px;background:#475A5F;color:#FFFFFF;"><% nvram_dump("syslog.log","syslog.sh"); %></textarea>
						</div>
					</td>
			</tr>

			<tr class="apply_gen" valign="top">
				<td width="40%" align="right">
					<form method="post" name="form1" action="apply.cgi">
						<input type="hidden" name="current_page" value="Main_LogStatus_Content.asp">
						<input type="hidden" name="action_mode" value="Clear">
						<input type="hidden" name="next_host" value="">
						<input type="submit" onClick="document.form1.next_host.value = location.host; onSubmitCtrl(this, 'Clear')" value="<#CTL_clear#>" class="button_gen">
					</form>
				</td>	
				<td width="20%" align="center">
					<form method="post" name="form2" action="syslog.txt">
						<input type="hidden" name="next_host" value="">
						<input type="submit" onClick="document.form2.next_host.value = location.host; onSubmitCtrl(this, 'Save')" value="<#CTL_onlysave#>" class="button_gen">
					</form>
				</td>	
				<td width="40%" align="left" >
					<form method="post" name="form3" action="apply.cgi">
						<input type="hidden" name="current_page" value="Main_LogStatus_Content.asp">
						<input type="hidden" name="action_mode" value=" Refresh ">
						<input type="hidden" name="next_host" value="">
						<input type="button" onClick="location.reload();" value="<#CTL_refresh#>" class="button_gen">
					</form>
				</td>	
			</tr>
			</table>
		</td>

	</tr>
</table>
      <!--===================================Ending of Main Content===========================================-->
</td>
      <td width="10" align="center" valign="top"></td>
  </tr>
</table>
<div id="footer"></div>
		</form>
</body>
</html>
