#!/bin/sh

# initialize
nvram set webs_state_update=0
nvram set webs_state_error=0
forsq=`nvram get apps_sq`
# wget options
options="-q -t 2 -T 70"
if [ "$forsq" == "1" ]; then
	url="http://dlcdnet.asus.com/pub/ASUS/LiveUpdate/Release/Wireless_SQ/radio_list.zip"
else
	url="http://dlcdnet.asus.com/pub/ASUS/LiveUpdate/Release/Wireless/radio_list.zip"
fi
local_path="/tmp/radio_list"
# start download
wget $options $url -O $local_path.zip

if [ "$?" != "0" ]; then
	nvram set webs_state_error=1
else
	unzip -o $local_path.zip -d /tmp
	rm -f $local_path.*
fi
nvram set webs_state_update=1
