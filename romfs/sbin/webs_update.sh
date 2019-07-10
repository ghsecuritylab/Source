#!/bin/sh

# initialize
nvram set webs_state_update=0
nvram set webs_state_error=0
forsq=`nvram get apps_sq`
# get model name
model=`nvram get productid`
# wget options
options="-q -t 2 -T 30"
if [ "$forsq" == "1" ]; then
	url="http://dlcdnet.asus.com/pub/ASUS/LiveUpdate/Release/Wireless_SQ/repeater_update.zip"
else
	url="http://dlcdnet.asus.com/pub/ASUS/LiveUpdate/Release/Wireless/repeater_update.zip"
fi
local_path="/tmp/wlan_update"
# start download
wget $options $url -O $local_path.zip

if [ "$?" != "0" ]; then
	nvram set webs_state_error=1
else
	unzip -o $local_path.zip -d /tmp
	if [ -f "$local_path.txt" ]; then
		firmver=`grep $model $local_path.txt | sed s/.*#FW//;`
		nvram set webs_state_info=$firmver
	else
		nvram set webs_state_error=1
	fi
	rm -f $local_path.*
fi
nvram set webs_state_update=1
