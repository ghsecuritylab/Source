<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title><#Web_Title#></title>
<link rel="stylesheet" type="text/css" href="other.css">
<script>
var delay_time;
var action_mode = '<% get_parameter("action_mode"); %>';

function initial(){
	if (action_mode == "Restore")
		delay_time = 70;
	else {
		if (parent.sw_mode == 2 || parent.sw_mode == 5)
			delay_time = 120;
		else
			delay_time = 70;
	}
	parent.hideLoading();
	parent.showLoading(delay_time, "waiting");
	setTimeout("redirect();", delay_time*1000);
}

function redirect(){
	parent.parent.location.href = "/";
}
</script>
</head>

<body onLoad="initial();">
</body>
</html>
