<html>
<head>
<title><#Web_Title#> - File Upload</title>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
<link rel="stylesheet" type="text/css" href="/form_style.css" media="screen"></link>
<script>
function applysubmit(){
	document.form.submit();
}
</script>

</head>  

<body onLoad="" style="background-color: #21333e;">
<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" action="upload_file.cgi" target="hidden_frame" enctype="multipart/form-data">

<table class="formTable"  width="600" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3">
  <thead>
  <tr>
	<td colspan="2" height="30">File Upload</td>
  </tr>
  </thead>
  <tbody>
  <tr>
	<th>File</th>
	<td><input type="file" name="file" class="input" style="color:#FFCC00;"></td>
  </tr>
  <tr align="center">
	<td colspan="2"><input type="button" class="button_gen" onclick="applysubmit();" value="Upload"></td>
  </tr>
  </tbody>
</table>

</form>
</body>
</html>
