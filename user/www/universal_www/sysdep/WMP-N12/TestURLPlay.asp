<html>
<head>
<title><#Web_Title#> - URL Play</title>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
<link rel="stylesheet" type="text/css" href="/form_style.css" media="screen"></link>
<script>
function applysubmit(){
	if (document.form.remote_url_chkbox.checked == true)
		document.form.remote_url.value = encodeURIComponent(document.form.remote_url_char.value);
	else
		document.form.remote_url.value = document.form.remote_url_char.value;
	document.form.submit();
}
</script>

</head>  

<body onLoad="" style="background-color: #21333e;">
<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" action="url_play.cgi" target="hidden_frame">
<input type="hidden" name="remote_url" value="">

<table class="formTable"  width="600" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3">
  <thead>
  <tr>
	<td colspan="2" height="30">URL Play</td>
  </tr>
  </thead>
  <tbody>
  <tr>
	<th>URL</th>
	<td>
	  <input type="text" name="remote_url_char" class="input_32_table" value="">
	  <br><input type="checkbox" name="remote_url_chkbox" value="">convert to UTF-8</input>
	</td>
  </tr>
  <tr>
	<th>Action</th>
	<td><input type="text" name="remote_act" class="input_6_table" value=""> [play/stop]</td>
  </tr>
  <tr>
	<th>Volume mute/unmute</th>
	<td><input type="text" name="remote_vol_mute" class="input_3_table" value=""> [0:unmute/1:mute]</td>
  </tr>
  <tr>
	<th>Volume</th>
	<td><input type="text" name="remote_vol" class="input_3_table" value=""> % [0~100]</td>
  </tr>
  <tr align="center">
	<td colspan="2"><input type="button" name="button" class="button_gen" onclick="applysubmit();" value="submit" /></td>
  </tr>
  </tbody>
</table>

</form>
</body>
</html>
