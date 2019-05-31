<html>
<head>
<title><#Web_Title#> - Engineer Debug</title>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
<link rel="stylesheet" type="text/css" href="/form_style.css" media="screen"></link>
<script language="javascript">
function onSubmitCtrl(s) {
	document.form.action_mode.value = s;
	return true;
}

function submitenter(myfield,e){
	var keycode;
	if (window.event) keycode = window.event.keyCode;
	else if (e) keycode = e.which;
	else return;

	if (keycode == 13)
		onSubmitCtrl(' Refresh ');
}
</script>
</head>  

<body onLoad="document.form.SystemCmd.focus();" style="background-color: #21333e;">
<form method="GET" name="form" action="/apply.cgi"> 
<input type="hidden" name="current_page" value="Main_AdmStatus_Content.asp">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="action_wait" value="">
<input type="hidden" name="preferred_lang" value="<% nvram_get("preferred_lang"); %>">

<table class="formTable"  width="600" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3">
	<thead>
	<tr>
		<td colspan="2" height="30">System Command</td>
	</tr>
	</thead>
	<tbody>
	<tr>
		<td>
			<input class="input_option" onKeyPress="submitenter(this, event)" type="text" maxlength="255" size="50%" name="SystemCmd" value="">
			<input class="button_gen" onClick="onSubmitCtrl(' Refresh ')" type="submit" value="<#CTL_refresh#>" name="action">
		</td>
	</tr>
	<tr>
		<td>
			<textarea cols="80" rows="20" wrap="off" readonly="1" style="width:99%;font-family:Courier New, Courier, mono; font-size:11px;background:#475A5F;color:#FFFFFF;"><% nvram_dump("syscmd.log","syscmd.sh"); %></textarea>
		</td>
	</tr>
	</tbody>
</table>	

</form>
</body>
</html>
