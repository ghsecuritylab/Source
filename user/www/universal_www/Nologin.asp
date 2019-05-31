<html>
<head>
<title><#Web_Title#></title>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<link href="other.css"  rel="stylesheet" type="text/css">
<style type="text/css">
body {
	margin:50px auto;
}
</style>

<script language="javascript">
<% login_state_hook(); %>

function initial(){
	document.getElementById("logined_ip_str").innerHTML = login_ip_str();
}
</script>
</head>

<body onload="initial()">
<form name="formname" method="POST">
<table width="500" border="0" align="center" cellpadding="10" cellspacing="0" class="erTable">
<thead>
  <tr>
    <td height="52"></td>
  </tr>
</thead> 
  <tr>
    <th align="left" valign="top">
		<div class="drword_Nologin">
	  	  	<p><#login_hint1#> <span id="logined_ip_str"></span></p>
	  	  	<p><#login_hint2#></p>
		</div>
		<!--div class="drImg"><img src="images/alertImg.gif"></div-->		
		<div style="height:70px; "></div>
	  	</th>
  </tr>
  <tr>
    <td height="22"><span></span></td>
  </tr>			
</table>
</form>
</body>
</html>
