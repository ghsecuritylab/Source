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
<title><#Web_Title#> - <#menu6_1#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="other.css">
<link rel="stylesheet" type="text/css" href="vol_control.css">

<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/jquery.js"></script>
<script>
var $j = jQuery.noConflict();
var content_width = new Array("70px","210px","100px","100px","80px","70px");
var vol_max_idx = 20;
var mla,mlb;

function initial(){
	show_menu();
	if (vol_max_support != -1)
		vol_max_idx = www_support.substring(vol_max_support + 6, vol_max_support + 8);
	if (equalizer_support != -1)
		document.getElementById("audio_eq_span").style.display = "";
	get_url_list();
	init_vol_bar();
	get_url_table();
	detect_ajack();
        mlb=setTimeout('refresh_status()',3000);
	setTimeout('refresh_all()',600000);
        setTimeout('silence()',1500);
}

function init_vol_bar(){
	$j.ajax({
		url: '/ajax_get_volume.asp',
		dataType: 'script',
		error: function(xhr){
		},
		success: function(response){
			drawvolumecontroller(vol_max_idx, vol);
			speaker_enable(mute);
		}
	});
}

//length: how many bars
//height: height of the tallest bar
//selected: which bar is selected
//function drawvolumecontroller(length, height, selected){
function drawvolumecontroller(length, selected){
	var html_code = "";
	for (var i = 1; i <= length; i++) {
		magassag = 10;
		margintop = 10;
		//magassag = 7 + Math.round((1.4)*(length - i));
		//margintop = height-magassag;
		//if (margintop <= 0) {margintop=0;}
		if (i <= selected)
			html_code += '<div onmouseup="volumecontrolchanged('+i+')" style="background-color:#009ACD;height:'+magassag+'px;margin-top:'+margintop+'px;" class="volumecontrollerbar"></div>';
		else
			html_code += '<div onmouseup="volumecontrolchanged('+i+')" style="height:'+magassag+'px;margin-top:'+margintop+'px;"class="volumecontrollerbar"></div>';
	}

	if (ajack == 0)
		html_code += '<div id="Audiojack_prompt" style="float:left;margin-top:8px;margin-left:8px;display:none;"><img src="/images/New_ui/audio/headphones.png"></div>';
	else
		html_code += '<div id="Audiojack_prompt" style="float:left;margin-top:8px;margin-left:8px;"><img src="/images/New_ui/audio/headphones.png"></div>';

	document.getElementById("volumcontroller").innerHTML = html_code;
}

function volumecontrolchanged(idx){
	set_vol_value(idx);
	drawvolumecontroller(vol_max_idx, idx);
}


var t0,t1,t2,t3,t4;
var regex,regex2;
var t0_splits,t1_splits,t2_splits;
function write_files(){
   t0 = document.getElementsByName("rs_region");
   t1 = document.getElementsByName("rs_name");
   t2 = document.getElementsByName("rs_type");
   t3 = document.getElementsByName("rs_address");
   regex = new RegExp("[A-Za-z0-9\s]", "g");
   regex2 = new RegExp("[^\s && ^,]", "g");
   //alert(t1[0].value); 
   //alert(t2[0].value); 
   //alert(t3[0].value); 

   t0_splits = (t0[0].value).split(regex);
   t1_splits = (t1[0].value).split(regex);
   t2_splits = (t2[0].value).split(regex);
   //alert(t1_splits);
   //alert(t2_splits);

   if (regex2.test(t0_splits) || regex2.test(t1_splits) || regex2.test(t2_splits))
   {   
       alert("Please use a-z/A-Z/0-9/ for naming your radio station region /name / type!");
       return;
   }    

   if(t0[0].value.length > 8 || t1[0].value.length > 30 || t2[0].value.length >8)
   {
   	alert("Invalid Content-Length")
	return;   
   }   
	
   if(all_usr_list.length == 10 )
   {
   	alert("The number of stations should be less than or equal to ten");
	return;
   }   
   	
   t4="addr="+t3[0].value+"&type="+t2[0].value+"&name="+t1[0].value+"&region="+t0[0].value;
   
   document.getElementById("add_RadioStation").style.visibility="hidden";
   showLoading(2);	

   $j.ajax({
		url: '/ajax_update_radio_list.asp',
		dataType: 'script',
		data: t4,
		error: function(xhr){
			},
		success: function(response){
			 window.location.reload();
		}
	});		
}

function set_vol_value(idx){
	$j.ajax({
		url: '/ajax_update_vol_value.asp',
		dataType: 'script',
		data: 'vol='+idx,
		error: function(xhr){
			},
		success: function(response){
			}
	});
}


function check_import_table(){
	$j.ajax({
		url: '/ajax_check_upload_table.asp',
		dataType: 'script',
		error: function(xhr){
			},
		success: function(response){
			if(chk_table_err == 2)
				alert("Invalid File-Size");
			else if (chk_table_err == 3)
				alert("Invalid File-Content");
			else if (chk_table_err == 4)
				alert("System Error.Please try again later");
			else if (chk_table_err == 5 )
				alert("Error!Radio groups should be less than 400.");
			window.location.reload();
		
			}
	});
}

function write_partition(){
	var file_obj = document.formx.file;
	if (file_obj.value == "") {
		alert("<#JS_fieldblank#>");
		file_obj.focus();
	} else if (file_obj.value.length < 6 
			|| ((file_obj.value.lastIndexOf(".INI") < 0) && (file_obj.value.lastIndexOf(".ini") < 0)) 
			|| ((file_obj.value.lastIndexOf(".INI") != (file_obj.value.length)-4) && (file_obj.value.lastIndexOf(".ini") != (file_obj.value.length)-4))) {
		alert("<#Setting_upload_hint#>");
		file_obj.focus();
	} else {
		setTimeout('check_import_table()',4000);
		document.getElementById("add_RadioList").style.visibility = "hidden";
		showLoading(15);
		document.formx.submit();
	}
}

function cancel_list()
{
	document.getElementById("add_RadioList").style.visibility="hidden";
}

function cancel_url()
{
	document.getElementById("add_RadioStation").style.visibility="hidden";
}

function get_url_table(){
	$j.ajax({
		url: '/ajax_get_url_table.asp',
		dataType: 'script',
		error: function(xhr){
			},
		success: function(response){
			}
	});
}

function silence()
{
	$j.ajax({
		url: '/ajax_silence.asp',
		dataType: 'script',
		error: function(xhr){
			},
		success: function(response){	
			speaker_enable(sil);
			setTimeout('silence()',1500);
		}
	});
}

function refresh_all()
{
	window.location.reload();
        setTimeout('refresh_all()',600000);
}   

var ajack;
function detect_ajack(){
	$j.ajax({
		url: '/ajax_det_ajack.asp',
		dataType: 'script',
		error: function(xhr){
		},
		success: function(response){
			if (ajack == 0)
				document.getElementById("Audiojack_prompt").style.display = "none";
			else
				document.getElementById("Audiojack_prompt").style.display = "";
		}
	});
}   

function refresh_status(){
	detect_ajack();
	init_vol_bar();
	get_status();
	mla = setTimeout('refresh_status()', 3000);
	clearTimeout(mlb);
	mlb = mla;
}

var getstatus;
function get_status(){
	$j.ajax({
		url: '/ajax_get_status.asp',
		dataType: 'script',
		error: function(xhr){
		},
		success: function(response){	
			if (getstatus == 1)
				get_url_list();
		}
	});
}   

var all_builtin_list,all_usr_list,is_exist_iradio;
var get_nowplay,radiolist_code;
var content_num ; // depend on list array
var len,i,m,j;
function get_url_list(){

	content_num = 4; // depend on list array
	radiolist_code = "";
  	$j.ajax({
		url: '/ajax_get_url_list.asp',
		dataType: 'script',
		error: function(xhr){
			},
		success: function(response){
			len=all_builtin_list.length+all_usr_list.length;

			if(is_exist_iradio == 0)
			          get_nowplay=999; //free

			// url list
			for (i = 0; i < len; i++) {
				radiolist_code += '<tr>';
				m=i+1;
				radiolist_code += '<td width="30px"  align="center" style="border-color: #6b8fa3;">'+m+'</td>';
				if (i < all_builtin_list.length) 
				{
				   	for (j=0; j<content_num; j++)
						radiolist_code += '<td width="'+content_width[j]+'" align="center"  style="border-color: #6b8fa3;">'+all_builtin_list[i][j]+'</td>';
					
					//play icon
					if(i==get_nowplay)
						radiolist_code += '<td width="80px" align="center" style="border-color: #6b8fa3;" ><input type="button" name="audio_play" id="audio_play'+i+'" class="audio_play" onClick="play_btn('+i+');" value="" title="pause"></td>';	
					else 
						radiolist_code += '<td width="80px" align="center" style="border-color: #6b8fa3;"><input type="button" name="audio_play" id="audio_play'+i+'" class="audio_play_gray" onClick="play_btn('+i+');" value="" title="play"></td>';	
						 
					radiolist_code += '<td width="70px" style="border-color: #6b8fa3;" >&nbsp;</td>';
				}
				else  
				{
						   
				   	for (m=0; m<content_num; m++)
						radiolist_code += '<td width="'+content_width[m]+'" align="center" style="border-color: #6b8fa3;">'+all_usr_list[i-all_builtin_list.length][m]+'</td>';
				
					//play icon
					if(i==get_nowplay)
						radiolist_code += '<td width="80px" align="center" style="border-color: #6b8fa3;"><input type="button" name="audio_play" id="audio_play'+i+'" class="audio_play" onClick="play_btn('+i+');" value="" title="pause"></td>';	
					else 
						radiolist_code += '<td width="80px" align="center" style="border-color: #6b8fa3;"><input type="button" name="audio_play" id="audio_play'+i+'" class="audio_play_gray" onClick="play_btn('+i+');" value="" title="play"></td>';	
						 
					//del icon
					radiolist_code += '<td width="70px" align="center" style="border-color: #6b8fa3;"><input type="button" name="audio_del" id="audio_del" class="audio_del" onClick="del_btn('+i+');" value="" title="del"></td>';	
				
				}
				
				radiolist_code += '</tr>';
			} //for i...

			//radiolist_code += '<td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td>';
			$j("#builtin_radio_list tbody").html(radiolist_code);
			radiolist_code=null;
			}
	});
}

function speaker_enable(tmp_mute)
{
	 if(tmp_mute=="1")
		document.getElementById("audio_gate").className = "audio_mute";
	 else
		document.getElementById("audio_gate").className = "audio_unmute";
}

var ret;
var urlNum;
function del_btn(num)
{
	urlNum='delnum='+num;
	showLoading(5);
   	$j.ajax({
		url: '/ajax_del_radio_list.asp',
		dataType: 'script',
		data: urlNum,
		error: function(xhr){
			},
		success: function(response){
		window.location.reload();
		
		}
	});		
}


var urlNump,lens,an;
function play_btn(num)
{
	urlNump='urlnum='+num+'&address='+urltable[num];
	lens=all_builtin_list.length+all_usr_list.length;
	
	for(i=0;i<lens;i++)
	{
		//alert("gray="+i);
		an="audio_play"+i;
		if(i!=num)
			document.getElementById(an).className = "audio_play_gray";
		else
		{   
			 if(get_nowplay==num)	 
			 {
			    document.getElementById(an).className = "audio_play_gray";
			    get_nowplay=999; //free time 
		         }	    
			 else
		         {
		    	    document.getElementById(an).className = "audio_play";
			    get_nowplay=num; //playing ...\
			 }   
		}
	}
	$j.ajax({
		url: '/ajax_play_url.asp',
		dataType: 'script',
		data:urlNump,
		error: function(xhr){
			},
		success: function(response){
		}
	});

}

var tmp_mute;
function det_btn(){
	$j.ajax({
		url: '/ajax_audio_mute.asp',
		dataType: 'script',
		error: function(xhr){
			},
		success: function(response){
			speaker_enable(tmp_mute);
		}
	});
}

function chanege_audio_eq(obj){
	$j.ajax({
		url: '/ajax_set_audio_eq.asp',
		dataType: 'script',
		data: 'eq='+obj.value,
		error: function(xhr){
		},
		success: function(response){
		}
	});
}

var retry = 0;
function check_radio_upgrade(){
	$j.ajax({
		url: '/ajax_detect_firmware_status.asp',
		dataType: 'script',
		error: function(xhr) {
			setTimeout("check_radio_upgrade();", 1000);
		},
		success: function(response) {
			if(retry > 60){ // check 60 sec for download latest radio-list version
				document.getElementById("RS_up_prompt_str").innerHTML = "<#Internet_radio_hint1#>";
				document.getElementById("RS_up_prompt_img").style.display = "none";
				retry = 0;
			}
			else if(webs_state_update == 0){
				setTimeout("check_radio_upgrade();", 1000);
				retry++;
			}
			else{
				if(webs_state_error != 1 ){
				     document.getElementById("RS_up_prompt_str").innerHTML = "";
				     document.getElementById("RS_up_prompt_img").style.display = "none";
				     if (confirm("<#Internet_radio_hint5#>")) {
				     	setTimeout("doUpdate(1);", 1000);
   					showLoading(3);	
				    }	
				     else
				     	setTimeout("doUpdate(0);", 1000);
				     document.getElementById("RS_up_prompt").style.display = "none";
				}
				else
			             document.getElementById("RS_up_prompt_str").innerHTML = "<#Internet_radio_hint1#>";
				document.getElementById("RS_up_prompt_img").style.display = "none";
			}
		}
	});
}	


var t5;
function doUpdate(para){
	t5="cmd="+para; 
	$j.ajax({
		url: '/ajax_upgrade_radio_list.asp',
		data: t5,
		dataType: 'script',
		error: function(xhr){
			},
		success: function(response){
			get_url_list();
			get_url_table();
			if(result1 ==1)
				document.getElementById("RS_up_prompt_str").innerHTML = "<#Internet_radio_hint2#>";
			else 
				document.getElementById("RS_up_prompt_str").innerHTML = "<#Internet_radio_hint3#>";
		}

	});

	document.getElementById("RS_up_prompt").style.display = "";
}


function detect_liveUpdate()
{
       $j.ajax({
                 url: '/ajax_radio_detect_liveupdate.asp',
                 dataType: 'script',
                 data: 'webs_cmd=start_webs_upgrade_radio',
                 error: function(xhr) {
                 setTimeout("detect_liveUpdate();", 1000);
                 },
                 success: function(response) {
                         setTimeout("check_radio_upgrade();", 3000);
                         document.getElementById("RS_up_prompt_str").innerHTML = "<#Internet_radio_hint4#>";
                         document.getElementById("RS_up_prompt_img").style.display = "";
                         document.getElementById("RS_up_prompt").style.display = "";
	              }
       });
}   

function saveRadioList()
{
location.href='RadioList_'+productid+'.INI';
}

function applemarket()
{
alert("(apple) is not yet ready");
}

function googlemarket()
{
alert("(google) is not yet ready");
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
<!--[if lte IE 6.5]><iframe class="hackiframe"></iframe><![endif]-->
<div id="add_RadioStation" class="popup_bg" >
  <table cellpadding="5" cellspacing="0" id="dr_audio_advise" class="dr_audio_advise" style="position:relative;margin-left:500px;"> 
    <tr>
      <td>		
	<form>
 	<p><#Internet_radio_list1#>:<p><input type="text" name="rs_region" maxlength="8"  value="" >
 	<p><#Internet_radio_add1#><p><input type="text" name="rs_name" maxlength="30" value="new radio station" >
 	<p><#Internet_radio_add2#><p><input type="text" name="rs_type" maxlength="8" value="music" >
 	<p><#Internet_radio_add3#><p><input type="text" name="rs_address" value="http://" >
 	<p><input type="button" name="add_data" value="OK"  onClick="write_files();"/>
  	   <input type="button" name="cancel_data" value="Cancel" onclick="cancel_url();"/>
	</form>
    </td>
   </tr>
  </table>   
</div>


<div id="add_RadioList" class="popup_bg" >
  <table cellpadding="5" cellspacing="0" id="dr_audio_advise" class="dr_audio_advise" style="position:relative;margin-left:500px;"> 
    <tr>
      <td>		
	 <form method="post" name="formx" action="upload_ini.cgi" target="hidden_frame" enctype="multipart/form-data">
	   <input type="file" name="file" class="input" style="color:#FFCC00;"/>
	   <input type="button" value="OK" onclick="write_partition();"/>
  	   <input type="button" value="Cancel" onclick="cancel_list();"/>
	</form>
    </td>
   </tr>
  </table>   
</div>
<!--for uniform show, useless but have exist-->

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form"  target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="current_page" value="Advanced_Audio_Player.asp">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="action_mode" value=" Apply ">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="action_wait" value="3">
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
		  <td align="left" valign="top">

			<table width="760px" border="1" cellpadding="0" cellspacing="0" class="FormTitle" id="FormTitle" height="660px">
			<tbody>
			  <tr>
				<td bgcolor=#000000 valign="top">
				  <div>&nbsp;</div>
				  <div class="formfonttitle">
				     <font size="5">
				      <b><img align=top src="/images/New_ui/audio/cloud_radio.png">
						  <p><img align=right src="/images/New_ui/audio/eq.jpg"></b>
				      </font>
					</div>
					
				  <div class="formfontdesc"  style="margin-left:25px;height:60px;">
				      <font size="2"><#Internet_radio_title#></font>
				  </div>
		
         			  <!--div class="market1" style="z-index:2;position:relative;"-->
					<!--input type="button" name="google_play" id="google_play" class="gplay" onClick="googlemarket();" value="" title="google"-->
		 			 <!--input type="button" name="apple_store" id="apple_store" class="applestore" onClick="applemarket();" value="" title="apple"-->
				
		  		 <!--/div-->

  			 
  <!--===================================go===========================================-->
   
	<div style="position:absolute;margin-left:35px;margin-top:70px;" id="radio_main_content">

	<table class="radio_info" cellpadding="0" cellspacing="0"  style="border: 1px solid;width:673px;top:40px;height:40px;border-color: #6b8fa3;">
	 	<tr style="height:10px;">
			<td>
			<p><img align=left width="50px" height="50px" src="/images/New_ui/audio/radio_logo.png">
			<p><#Internet_radio_state#>
                        <p><input type="button" name="audio_gate" id="audio_gate" class="audio_gate" onClick="det_btn();" value="" title="mute/unmute">
			   <div style="margin-top:-32px;margin-left:80px;">
				<span id="volumcontroller"></span>
				<span id="audio_eq_span" style="margin-left:100px;color:#009acd;display:none;"><#Audio_EQ_title#>&nbsp;&nbsp;
				  <select name="audio_eq" onchange="chanege_audio_eq(this);" class="ir_input_option">
					<option value="0" <% nvram_match("eq_cur_idx", "0", "selected"); %>><#Audio_EQ_option0#></option>
					<option value="1" <% nvram_match("eq_cur_idx", "1", "selected"); %>><#Audio_EQ_option1#></option>
					<option value="2" <% nvram_match("eq_cur_idx", "2", "selected"); %>><#Audio_EQ_option2#></option>
					<option value="3" <% nvram_match("eq_cur_idx", "3", "selected"); %>><#Audio_EQ_option3#></option>
					<option value="4" <% nvram_match("eq_cur_idx", "4", "selected"); %>><#Audio_EQ_option4#></option>
					<option value="5" <% nvram_match("eq_cur_idx", "5", "selected"); %>><#Audio_EQ_option5#></option>
					<option value="6" <% nvram_match("eq_cur_idx", "6", "selected"); %>><#Audio_EQ_option6#></option>
					<option value="7" <% nvram_match("eq_cur_idx", "7", "selected"); %>><#Audio_EQ_option7#></option>
					<option value="8" <% nvram_match("eq_cur_idx", "8", "selected"); %>><#Audio_EQ_option8#></option>
					<option value="9" <% nvram_match("eq_cur_idx", "9", "selected"); %>><#Audio_EQ_option9#></option>
				  </select>
				</span>
			   </div>
		</td>
		</tr>
	</table>

	<table class="radio_list1" id="radio_title" cellpadding="0" cellspacing="0" style="border:1px solid;position:relative;width:673px;border-color:#6b8fa3;" rules="cols" frame="void" >
		<tr style="height:18px;">
			<th width="30px" class="table_sorting" align="center" style="border-color: #6b8fa3;">No.</th>
			<th width="70px" class="table_sorting" align="center" style="border-color: #6b8fa3;"><#Internet_radio_list1#></th>
			<th width="210px"  class="table_sorting" align="center" style="border-color: #6b8fa3;"><#Internet_radio_list2#></th>
			<th width="100px" class="table_sorting" align="center" style="border-color: #6b8fa3;"><#Internet_radio_list3#></th>
			<th width="100px" class="table_sorting" align="center" style="border-color: #6b8fa3;"><#Internet_radio_list4#></th>
			<th width="80px" class="table_sorting" align="center" style="border-color: #6b8fa3;"><#Internet_radio_list5#></th>
			<th width="70px" class="table_sorting" align="center" style="border-color: #6b8fa3;" ><#CTL_del#></th>
		</tr>
	</table>

	<div style="position:relative;height:270px;overflow-y:auto;width:690px;">
			<table class="radio_list2"   cellpadding="0" cellspacing="0"  style="border: 1px solid ;position:relative;width:673px;border-color: #6b8fa3;" rules="all" frame="void" id="builtin_radio_list">
				<tbody></tbody>
			</table>
	</div>

	<div style="top:12px;position:relative;z-index:2">
		<input type="button" name="add_btn" id="add_btn" class="IR_btn2" onClick="add_url();" value="<#Internet_radio_btn_add#>">
	</div>
	<div style="left:460px;top:-10px;position:relative;z-index:2">
		<input type="button" name="import_btn" id="import_btn" class="IR_btn" onClick="add_list();" value="<#Internet_radio_btn_import#>">
		<input type="button" name="export_btn" id="export_btn" class="IR_btn"  onclick="saveRadioList();" value="<#Internet_radio_btn_export#>">
		<input type="button" name="live_update_btn" id="live_update_btn" class="IR_btn"  onclick="detect_liveUpdate();" value="<#Internet_radio_btn_check#>">
		<div align="right" id="RS_up_prompt" style="left:-470px;width:700px;color:#FFCC00;display:none;position:relative;z-index:5">
		 	<div id="RS_up_prompt_str"></div>
			<img id="RS_up_prompt_img" src="images/InternetScan.gif">
		</div>
	</div>                                                                

	  		   </td>
			  </tr>
			</tbody>
			</table>
		  </td>
		</tr>
	  </table>
	</td>

	<td width="10" align="center" valign="top">&nbsp;</td>
  </tr>
</table>
</form>
<div id="footer"></div>
</body>
</html>
