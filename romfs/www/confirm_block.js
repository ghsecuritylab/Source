var confirm_exist = 0;

function confirm_asus(now, latest){
	if (confirm_exist == 1)
		return;

	var model = '<% nvram_get("productid"); %>';
	var tmp = model.replace("-", "");
	var code = "";
	code += '<div class="confirm_block" style="position:absolute;margin:100px 0px 0px 25px;">';
	code += '<div class="confirm">';
	code += '<div class="confirm_title"><#FW_new_title#></div>';
	code += '<div class="confirm_contentA"><#FW_new_desc1#>' + tmp + '<#FW_new_desc2#></div>';
	code += '<div class="confirm_contentC"><#QIS_fwup_desc2#><span>' + now + '</span></div>';
	code += '<div class="confirm_contentC"><#QIS_fwup_desc3#><span>' + latest + '</span></div>';
	code += '<div style="display:table;width:100%;margin-top:45px">';
	code += '<div class="confirm_button_gen_long_right" onClick="confirm_ok();"><#CTL_ok#></div>';
	code += '</div>';
	code += '</div></div>';
	$j('body').append(code);
	confirm_exist = 1;
}

function confirm_ok(){
	confirm_exist = 0;
	$j(".confirm_block").remove();
}

/*
function confirm_go_website(){
	var model = '<% nvram_get("productid"); %>';
	var tmp = model.replace("-", "");
	var website = 'https://www.asus.com/Networking/' + tmp + '/HelpDesk_Download/'
	window.open(website);
}*/
