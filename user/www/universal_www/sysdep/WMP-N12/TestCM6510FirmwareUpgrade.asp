<html>
<head>
<title><#Web_Title#> - CM6510 firmware upgrade</title>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
<link rel="stylesheet" type="text/css" href="/index_style.css" media="screen"></link>
<link rel="stylesheet" type="text/css" href="/form_style.css" media="screen"></link>
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
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script>
function applysubmit(){
	document.form.submit();
}

function restore_ui(){
}
</script>

</head>

<body onLoad="" style="background-color: #21333e;">

<div id="LoadingBar" class="popup_bar_bg">
  <table cellpadding="5" cellspacing="0" id="loadingBarBlock" class="loadingBarBlock" align="center">
	<tr>
	  <td height="80">
		<div class="Bar_container">
		  <span id="proceeding_img_text"></span>
		  <div id="proceeding_img"></div>
		</div>
		<div style="margin:5px auto; width:85%;"><#FW_ok_desc#></div>
	  </td>
	</tr>
  </table>
  <!--[if lte IE 6.5]><iframe class="hackiframe"></iframe><![endif]-->
</div>
<div id="Loading" class="popup_bg"></div>
<div id="hiddenMask" class="popup_bg">
  <table cellpadding="5" cellspacing="0" id="dr_sweet_advise" class="dr_sweet_advise" align="center">
	<tr>
	  <td>
		<div class="drword" id="drword" style="height:50px;">&nbsp;&nbsp;&nbsp;<#Main_alert_proceeding_desc1#>...<br/><br/></div>
	  </td>
	</tr>
  </table>
  <!--[if lte IE 6.5]><iframe class="hackiframe"></iframe><![endif]-->
</div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" action="upgrade_cm6510.cgi" target="hidden_frame" enctype="multipart/form-data">

<table class="formTable"  width="600" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3">
  <thead>
  <tr>
	<td colspan="2" height="30">CM6510 firmware upgrade</td>
  </tr>
  </thead>
  <tbody>
  <tr>
	<th>Firmware version</th>
	<td><input type="text" name="firmver_table" class="input_12_table" value="<% nvram_get("cm6510_firmver"); %>" readonly="1"></td>
  </tr>
  <tr>
	<th>File</th>
	<td><input type="file" name="file" class="input" style="color:#FFCC00;"></td>
  </tr>
  <tr align="center">
	<td colspan="2"><input type="button" class="button_gen" onclick="applysubmit();" value="Upgrade"></td>
  </tr>
  </tbody>
</table>

</form>
</body>
</html>
