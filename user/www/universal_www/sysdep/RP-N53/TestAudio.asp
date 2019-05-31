<html>
<head>
<title><#Web_Title#> - Test Audio</title>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
<link rel="stylesheet" type="text/css" href="/form_style.css" media="screen"></link>
<script type="text/javascript" src="/jquery.js"></script>
<script>
var $j = jQuery.noConflict();

function test_audio(idx){
	var tmp = "test_audio="+idx;
	$j.ajax({
		url: '/ajax_test_audio.asp',
		dataType: 'script',
		data: tmp,
		error: function(xhr){},
		success: function(response){}
	});
}
</script>

</head>

<body onLoad="" style="background-color: #21333e;">
<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" action="" target="hidden_frame">

<table class="formTable"  width="600" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3">
  <thead>
  <tr>
	<td colspan="2" height="30">Test Audio</td>
  </tr>
  </thead>
  <tbody>
  <tr>
	<th>Play builtin mp3</th>
	<td><input class="button_gen" onclick="test_audio(1);" type="button" value="Start"></td>
  </tr>
  <tr>
	<th>Loop play builtin mp3</th>
	<td><input class="button_gen" onclick="test_audio(2);" type="button" value="Start"></td>
  </tr>
  <tr>
	<th>Play local LAN resource</th>
	<td><input class="button_gen" onclick="test_audio(3);" type="button" value="Start"></td>
  </tr>
  </tbody>
</table>

</form>
</body>
</html>
