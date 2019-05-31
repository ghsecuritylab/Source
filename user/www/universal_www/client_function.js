function getclients_noMonitor(flag){
	var clients = new Array();

	// get wired clients from arp table
	for (var i = 0; i < arps.length; ++i) {
		clients[i] = new Array(9);
		clients[i][0] = "";		// hostname
		clients[i][1] = arps[i][0];	// IP
		clients[i][2] = arps[i][3];	// MAC
		clients[i][3] = "";
		clients[i][4] = null;
		clients[i][5] = null;
		clients[i][6] = "0";
		clients[i][7] = "0";
		clients[i][8] = "0";

		for (var j = 0; j < wireless.length; ++j) {
			if (clients[i][2] == wireless[j][0]) {
				clients[i][3] = 10;	// 10 is meant the client is wireless.
				clients[i][4] = new Array(2);
				if (wireless[j][1] == "Yes")
					clients[i][4][0] = "Associated";
				else
					clients[i][4][0] = "Disassociated";
				clients[i][4][1] = wireless[j][2];
				break;
			}
		}

		// judge which is the local device.
		if (clients[i][1] == login_ip_str() && clients[i][2] == login_mac_str()) {
			if (clients[i][0] == null || clients[i][0].length <= 0)
				clients[i][0] = "<#CTL_localdevice#>";
			else
				clients[i][0] += "(<#CTL_localdevice#>)";
		}

		if (flag == 1)
			clients[i][2] = simplyMAC(clients[i][2]);
	}

	return clients;
}

function getclients_Monitor(flag){
	var clients = new Array();

	for (var i = 0; i < ipmonitor.length; ++i) {
		clients[i] = new Array(9);
		clients[i][0] = ipmonitor[i][2];// Device name
		clients[i][1] = ipmonitor[i][0];// IP
		clients[i][2] = ipmonitor[i][1];// MAC
		clients[i][3] = null;		// if this is a wireless client.
		clients[i][4] = null;		// wireless information.
		clients[i][5] = ipmonitor[i][3];// TYPE
		clients[i][6] = ipmonitor[i][4];// if there's the HTTP service.
		clients[i][7] = ipmonitor[i][5];// if there's the Printer service.
		clients[i][8] = ipmonitor[i][6];// if there's the iTune service.

		for (var j = 0; j < wireless.length; ++j) {
			if (clients[i][2] == wireless[j][0]) {
				clients[i][3] = 10;	// 10 is meant the client is wireless.
				clients[i][4] = new Array(2);
				if (wireless[j][1] == "Yes")
					clients[i][4][0] = "Associated";
				else
					clients[i][4][0] = "Disassociated";
				clients[i][4][1] = wireless[j][2];
				break;
			}
		}

		// judge which is the local device.
		if (clients[i][1] == login_ip_str() && clients[i][2] == login_mac_str()) {
			if (clients[i][0] == null || clients[i][0].length <= 0)
				clients[i][0] = "<#CTL_localdevice#>";
			else
				clients[i][0] += "(<#CTL_localdevice#>)";
		}

		if (flag == 1)
			clients[i][2] = simplyMAC(clients[i][2]);
	}

	return clients;
}

function getclients(flag){
	if (networkmap_fullscan == "done")
		return getclients_Monitor(flag);
	else
		return getclients_noMonitor(flag);
}

function simplyMAC(fullMAC){
	var ptr;
	var tempMAC = "";
	var pos1 = 0;
	var pos2 = 0;

	ptr = fullMAC;
	for (var i = 0; i < 5; ++i) {
		pos2 = pos1 + ptr.indexOf(":");
		tempMAC += fullMAC.substring(pos1, pos2);
		pos1 = pos2 + 1;
		ptr = fullMAC.substring(pos1);
	}
	tempMAC += fullMAC.substring(pos1);
	return tempMAC;
}
