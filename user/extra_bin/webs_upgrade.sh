#!/bin/sh

# initialize
nvram set webs_state_upgrade=0
nvram set webs_state_error=0
nvram set rc_service=stop_services4upload
killall -SIGUSR1 init
forsq=`nvram get apps_sq`
# get download file name
firmware_file=`nvram get productid`_`nvram get webs_state_info`.zip
# wget options
options="-q -t 2 -T 150"
if [ "$forsq" == "1" ]; then
	url="http://dlcdnet.asus.com/pub/ASUS/LiveUpdate/Release/Wireless_SQ/$firmware_file"
else
	url="http://dlcdnet.asus.com/pub/ASUS/wireless/REPEATER/$firmware_file"
fi
local_path="/tmp/linux"
# start download
wget $options $url -O $local_path.zip

if [ "$?" != "0" ]; then
	nvram set webs_state_error=1
	nvram set rc_service=start_services4upload
	killall -SIGUSR1 init
else
	unzip -o $local_path.zip -d /tmp
	if [ -f "$local_path.trx" ]; then
		killall watchdog
		killall apcli_monitor
		killall -SIGTTOU init
	else
		nvram set webs_state_error=1
		nvram set rc_service=start_services4upload
		killall -SIGUSR1 init
	fi
	rm -f $local_path.zip
fi
nvram set webs_state_upgrade=1
