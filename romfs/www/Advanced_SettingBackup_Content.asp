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
<title><#Web_Title#> - <#menu4_4#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="other.css">

<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/jquery.js"></script>
<script>

var $j = jQuery.noConflict();
var varload = 0;
var lan_ipaddr = '<% nvram_get("lan_ipaddr"); %>';

function initial(){
	show_menu();
}

function restoreRule(){
	var alert_string = "<#Setting_factorydefault_hint1#>";
	if(lan_ipaddr != "192.168.1.1")
		alert_string += "<#Setting_factorydefault_iphint#>\n\n";			
	alert_string += "<#Setting_factorydefault_hint2#>";
	if(confirm(alert_string))
	{
		document.form.action1.blur();
		showtext($("loading_text"), "<#SAVE_restart_desc#>");
		showLoading();
		document.restoreform.submit();
	}
	else
		return false;
}

function saveSetting(){
	location.href='Settings_'+productid+'.CFG';
}

function uploadSetting(){
  var file_obj = document.form.file;
	
	if(file_obj.value == ""){
		alert("<#JS_fieldblank#>");
		file_obj.focus();
	}
	else if(file_obj.value.length < 6 ||
					((file_obj.value.lastIndexOf(".CFG")  < 0)&&(file_obj.value.lastIndexOf(".cfg")  < 0)) || 
					((file_obj.value.lastIndexOf(".CFG") != (file_obj.value.length)-4)&&(file_obj.value.lastIndexOf(".cfg") != (file_obj.value.length)-4))){		
		alert("<#Setting_upload_hint#>");
		file_obj.focus();
	}
	else{		
		disableCheckChangedStatus();
		showtext($("loading_text"), "<#SET_ok_desc#>");
		document.form.submit();
	}	
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
		<div id="loading_text" style="margin:5px auto; width:85%;"><#SAVE_restart_desc#></div>
		</td>
	</tr>
</table>
<!--[if lte IE 6.5]><iframe class="hackiframe"></iframe><![endif]-->
</div>
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
<div id="Loading" class="popup_bg"></div>
<!--for uniform show, useless but have exist-->

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>

<form method="post" name="form" action="upload.cgi" target="hidden_frame" enctype="multipart/form-data">
<input type="hidden" name="action_mode" value=" Apply ">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="action_wait" value="">
<input type="hidden" name="productid" value="<% nvram_get("fac_model_name"); %>">
<input type="hidden" name="current_page" value="Advanced_SettingBackup_Content.asp">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">

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
		  <td bgcolor="#4D595D" valign="top"  >
		  <div>&nbsp;</div>
		  <div class="formfonttitle"><#menu4#> - <#menu4_4#></div>
		  <div style="margin-left:5px;margin-top:10px;margin-bottom:10px"><img src="/images/New_ui/export/line_export.png"></div>
		  <div class="formfontdesc"><#Setting_save_upload_desc#></div>

		<table width="100%" border="1" align="center" cellpadding="6" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
          <tr>
            <th><a class="hintstyle"  href="javascript:void(0);" onclick="openHint(19,1)"><#Setting_factorydefault_itemname#></a></th>
            <td>
              <input class="button_gen" onclick="restoreRule();" type="button" value="<#CTL_restore#>" name="action1" />
          </tr>
          <tr>
            <th><a class="hintstyle"  href="javascript:void(0);" onclick="openHint(19,2)"><#Setting_save_itemname#></a></th>
            <td>
              <input class="button_gen" onclick="saveSetting();" type="button" value="<#CTL_onlysave#>" name="action2" />
            </td>
          </tr>
          <tr bgcolor="#a7d2e2">
            <th><a class="hintstyle" href="javascript:void(0);" onclick="openHint(19,3)"><#Setting_upload_itemname#></a></th>
            <td>
							<input type="button" class="button_gen" onclick="uploadSetting();" value="<#CTL_upload#>"/>
              <input type="file" name="file" class="input" style="color:#FFCC00;"/>
            </td>
          </tr>
        </table>
			  </td>
            </tr>
          </tbody>
		</table>
		</td>
</form>


        </tr>
      </table>		
	</td>
		
    <td width="10" align="center" valign="top">&nbsp;</td>
	</tr>
</table>
<div id="footer"></div>

<form method="post" name="restoreform" action="apply.cgi" target="hidden_frame">
<input type="hidden" name="action_mode" value="Restore">
<input type="hidden" name="next_page" value="">
</form>

<form name="hint_form"></form>
</body>
</html>
