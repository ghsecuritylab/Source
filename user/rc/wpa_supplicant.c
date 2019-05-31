/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifdef WPA_SUPPLICANT_SUPPORT
#include <stdio.h>
#include <net/if_arp.h>

#include <nvram/bcmnvram.h>

#include <shared.h>
#include <stapriv.h>

#include "oid.h"

#define debg_wpa 1 

unsigned char WpaSupplicant_flag = FALSE;

static unsigned char BtoH(char ch)
{
	if (ch >= '0' && ch <= '9') return (ch - '0');		// Handle numerals
	if (ch >= 'A' && ch <= 'F') return (ch - 'A' + 0xA);	// Handle capitol hex digits
	if (ch >= 'a' && ch <= 'f') return (ch - 'a' + 0xA);	// Handle small hex digits
	return(255);
}

static void AtoH(char *src, unsigned char *dest, int destlen)
{
	char *srcptr = src;
	unsigned char *destTemp = dest;

	while (destlen--) {
		*destTemp = BtoH(*srcptr++) << 4;  // Put 1st ascii byte in upper nibble.
		*destTemp += BtoH(*srcptr++);      // Add 2nd ascii byte to above.
		destTemp++;
	}
}

static void exec_WPASupplicant(char *ssid, NDIS_802_11_WEP_STATUS encryp, NDIS_802_11_AUTHENTICATION_MODE auth, RT_WPA_SUPPLICANT_KEY_MGMT keymgmt, int keyidx, char *wepkey)
{
	char *_argv[] = {"wpa_supplicant", "-B", "-ira0", "-bbr0", "-c/etc/wpa_supplicant.conf", "-Dralink", "-dd", NULL};
	pid_t _pid;
	// auth mode
	int s, ret;
	unsigned char wpa_supplicant_support = 2, ieee8021x_support = 1;
	NDIS_802_11_SSID Ssid;

	if (WpaSupplicant_flag) {
		eval_dumb("killall", "wpa_supplicant");
		WpaSupplicant_flag = FALSE;
	}
	sleep(1);
#if debg_wpa
	fprintf(stderr, "exec_WPASupplicant()\n");
#endif
	memset(&Ssid, 0x00, sizeof(NDIS_802_11_SSID));
	strcpy((char *)Ssid.Ssid ,ssid);
	Ssid.SsidLength = strlen(ssid);

	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (auth == Ndis802_11AuthModeMax)
		auth = Ndis802_11AuthModeOpen;

	if (keymgmt == Rtwpa_supplicantKeyMgmtNONE) {
		wpa_supplicant_support = 0;
		ieee8021x_support = 0;

		ret = OidSetInformation(OID_802_11_SET_IEEE8021X, s, "ra0", &ieee8021x_support, sizeof(ieee8021x_support));
		if (ret < 0) {
			fprintf(stderr, "Set OID_802_11_SET_IEEE8021X has error =%d, ieee8021x_support=%d\n", ret, ieee8021x_support);
			close(s);
			return;
		}

		ret = OidSetInformation(RT_OID_WPA_SUPPLICANT_SUPPORT, s, "ra0", &wpa_supplicant_support, sizeof(wpa_supplicant_support));
		if (ret < 0) {
			fprintf(stderr, "Set RT_OID_WPA_SUPPLICANT_SUPPORT has error =%d, wpa_supplicant_support=%d\n", ret, wpa_supplicant_support);
			fprintf(stderr, "Please check the driver configuration whether support WAP_SUPPORT!!");
			close(s);
			return;
		}
	}
	else {
		ret = OidSetInformation(OID_802_11_SET_IEEE8021X, s, "ra0", &ieee8021x_support, sizeof(ieee8021x_support));
		if (ret < 0) {
			fprintf(stderr, "Set OID_802_11_SET_IEEE8021X has error =%d, ieee8021x_support=%d\n", ret, ieee8021x_support);
			close(s);
			return;
		}

		ret = OidSetInformation(RT_OID_WPA_SUPPLICANT_SUPPORT, s, "ra0", &wpa_supplicant_support, sizeof(wpa_supplicant_support));
		if (ret < 0) {
			fprintf(stderr, "Set RT_OID_WPA_SUPPLICANT_SUPPORT has error =%d, wpa_supplicant_support=%d\n", ret, wpa_supplicant_support);
			fprintf(stderr, "Please check the driver configuration whether support WAP_SUPPORT!!");
			close(s);
			return;
		}
	}

	ret = OidSetInformation(OID_802_11_AUTHENTICATION_MODE, s, "ra0", &auth, sizeof(auth));
	if (ret < 0) {
		fprintf(stderr, "Set OID_802_11_AUTHENTICATION_MODE has error =%d, auth=%d\n", ret, auth);
		close(s);
		return;
	}

	// encryp mode
	ret = OidSetInformation(OID_802_11_ENCRYPTION_STATUS, s, "ra0", &encryp, sizeof(encryp));
	if (ret < 0) {
		fprintf(stderr, "Set OID_802_11_ENCRYPTION_STATUS has error =%d, encry=%d\n", ret, encryp);
		close(s);
		return;
	}

	if (encryp == Ndis802_11WEPEnabled) {
		PNDIS_802_11_WEP pWepKey = NULL;
		unsigned long lBufLen;
		int nKeyLen;
		fprintf(stderr, "eason: wep encry!!!\n");
		nKeyLen = strlen(wepkey);
		if (nKeyLen == 0) {
			NDIS_802_11_REMOVE_KEY removeKey;
			int j = 0;
			removeKey.Length = sizeof(NDIS_802_11_REMOVE_KEY);
			removeKey.KeyIndex = keyidx;
			for (j = 0; j < 6; j++)
				removeKey.BSSID[j] = 0xff;

			ret = OidSetInformation(OID_802_11_REMOVE_KEY, s, "ra0", &removeKey, removeKey.Length);
			if (ret < 0)
				fprintf(stderr, "Set OID_802_11_REMOVE_KEY has error =%d, \n", ret);
		}
		else {
			if (nKeyLen == 10)
				nKeyLen = 5;
			else if (nKeyLen == 26)
				nKeyLen = 13;

			lBufLen = sizeof(NDIS_802_11_WEP) + nKeyLen - 1;
			// Allocate Resource
			pWepKey = (PNDIS_802_11_WEP)malloc(lBufLen);
			pWepKey->Length = lBufLen;
			pWepKey->KeyLength = nKeyLen;
			pWepKey->KeyIndex = keyidx;
			pWepKey->KeyIndex |= 0x80000000;

			if (strlen(wepkey) == 5)
				memcpy(pWepKey->KeyMaterial, wepkey, 5);
			else if (strlen(wepkey) == 10)
				AtoH(wepkey, pWepKey->KeyMaterial, 5);
			else if (strlen(wepkey) == 13)
				memcpy(pWepKey->KeyMaterial, wepkey, 13);
			else if (strlen(wepkey) == 26)
				AtoH(wepkey, pWepKey->KeyMaterial, 13);

			OidSetInformation(OID_802_11_ADD_WEP, s, "ra0", pWepKey, pWepKey->Length);
			free(pWepKey);
		}
	}

	// set ssid for associate
	if (OidSetInformation(OID_802_11_SSID, s, "ra0", &Ssid, sizeof(NDIS_802_11_SSID)) < 0) {
		fprintf(stderr, "Set OID_802_11_SSID has error =%d, pSsid->Ssid=%s\n", ret, Ssid.Ssid);
		close(s);
		return;
	}

	/*
	if (OidSetInformation(OID_802_11_BSSID, s, "ra0", &bssid, 6) < 0) {
		error(E_L, E_LOG, T("Set OID_802_11_BSSID has error."));
		close(s);
		return;
	}
	*/

	close(s);

	_eval(_argv, NULL, 0, &_pid);
	WpaSupplicant_flag = TRUE;
}

static void conf_WPASupplicant(char *ssid, RT_WPA_SUPPLICANT_KEY_MGMT keymgmt, RT_WPA_SUPPLICANT_EAP eap, char *identity, char *password, char *cacert, char *clientcert, char *privatekey, char *privatekeypassword, char *wepkey, int keyidx, NDIS_802_11_WEP_STATUS encryp, RT_WPA_SUPPLICANT_TUNNEL tunnel, NDIS_802_11_AUTHENTICATION_MODE auth)
{
	FILE *wsconf;
	char wpaconf[] = "/etc/wpa_supplicant.conf";

	fprintf(stderr, "wpaconf=%s\n", wpaconf);
	fprintf(stderr, "conf_WPASupplicant(), keymgmt=%d, Rtwpa_supplicantKeyMgmtNONE=%d\n", keymgmt, Rtwpa_supplicantKeyMgmtNONE);

	wsconf = fopen(wpaconf, "w+");

	fprintf(wsconf, "ctrl_interface=/var/run/wpa_supplicant\n");
	fprintf(wsconf, "eapol_version=1\n");
	fprintf(wsconf, "ap_scan=0\n");
	fprintf(wsconf, "network={\n");
	fprintf(wsconf, "ssid=\"%s\"\n", ssid);

	if (keymgmt == Rtwpa_supplicantKeyMgmtWPAEAP) {
		fprintf(wsconf, "key_mgmt=%s\n", "WPA-EAP");

		if (auth == Ndis802_11AuthModeWPA)
			fprintf(wsconf, "proto=WPA\n");
		else if (auth == Ndis802_11AuthModeWPA2)
			fprintf(wsconf, "proto=RSN\n");

		if (encryp == Ndis802_11Encryption2Enabled) {		//tkip
			fprintf(wsconf, "pairwise=TKIP\n");
			fprintf(wsconf, "group=TKIP\n");
		}
		else if (encryp == Ndis802_11Encryption3Enabled) {	//aes
			fprintf(wsconf, "pairwise=CCMP TKIP\n");
			fprintf(wsconf, "group=CCMP TKIP\n");
		}
	}
	else if (keymgmt == Rtwpa_supplicantKeyMgmtIEEE8021X) {
		fprintf(wsconf, "key_mgmt=%s\n", "IEEE8021X");
		if (eap == Rtwpa_supplicantEAPTLS || eap == Rtwpa_supplicantEAPTTLS)
			fprintf(wsconf, "eapol_flags=3\n");
		else if (eap == Rtwpa_supplicantEAPMD5)
			fprintf(wsconf, "eapol_flags=0\n");
	}
	else if (keymgmt == Rtwpa_supplicantKeyMgmtNONE) {
		fprintf(wsconf, "key_mgmt=%s\n", "NONE");
		fprintf(wsconf, "}\n");
		fclose(wsconf);
		exec_WPASupplicant(ssid, encryp, auth, keymgmt, keyidx, wepkey);
		return;
	}

	//id
	fprintf(wsconf, "identity=\"%s\"\n",identity);

	//CA cert
	if (strcmp(cacert, "0" ) && strcmp(cacert, "")) //option
		fprintf(wsconf, "ca_cert=\"%s\"\n", cacert);

	//eap
	switch(eap) {
		case Rtwpa_supplicantEAPTLS:
			fprintf(wsconf, "eap=TLS\n");
			fprintf(wsconf, "client_cert=\"%s\"\n", clientcert);
			fprintf(wsconf, "private_key=\"%s\"\n", privatekey);
			fprintf(wsconf, "private_key_passwd=\"%s\"\n", privatekeypassword);
			break;
		case Rtwpa_supplicantEAPTTLS:
			fprintf(wsconf, "eap=TTLS\n");
			if (strcmp(clientcert, "0" ) && strcmp(clientcert, "")) {//option
				fprintf(wsconf, "client_cert=\"%s\"\n", clientcert);
				fprintf(wsconf, "private_key=\"%s\"\n", privatekey);
				fprintf(wsconf, "private_key_passwd=\"%s\"\n", privatekeypassword);
			}
			if (tunnel == Rtwpa_supplicantTUNNELMSCHAPV2)
				fprintf(wsconf, "phase2=\"auth=MSCHAPV2\"\n");
			else if (tunnel == Rtwpa_supplicantTUNNELMSCHAP)
				fprintf(wsconf, "phase2=\"auth=MSCHAP\"\n");
			else if (tunnel == Rtwpa_supplicantTUNNELPAP)
				fprintf(wsconf, "phase2=\"auth=PAP\"\n");

			fprintf(wsconf, "password=\"%s\"\n", password);
			break;
		case Rtwpa_supplicantEAPPEAP:
			fprintf(wsconf, "eap=PEAP\n");
			//fprintf(stderr, "clientcert=%s, strlen=%d\n", clientcert, strlen((char *)clientcert));
			if (strcmp(clientcert, "0" ) && strcmp(clientcert, "")) {//option
				fprintf(wsconf, "client_cert=\"%s\"\n", clientcert);
				fprintf(wsconf, "private_key=\"%s\"\n", privatekey);
				fprintf(wsconf, "private_key_passwd=\"%s\"\n", privatekeypassword);
			}
			fprintf(wsconf, "password=\"%s\"\n", password);
			fprintf(wsconf, "phase1=\"peaplable=0\"\n");

			if (tunnel == Rtwpa_supplicantTUNNELMSCHAPV2)
				fprintf(wsconf, "phase2=\"auth=MSCHAPV2\"\n");
			break;
		case Rtwpa_supplicantEAPMD5:
			fprintf(wsconf, "eap=MD5\n");
			fprintf(wsconf, "password=\"%s\"\n", password);
			fprintf(wsconf, "wep_tx_keyidx=%d\n", keyidx);
			if (strlen(wepkey) == 5 || strlen(wepkey) == 13)
				fprintf(wsconf, "wep_key%d=\"%s\"\n", keyidx, wepkey);
			else if (strlen(wepkey) == 10 || strlen(wepkey) == 26)
				fprintf(wsconf, "wep_key%d=%s\n", keyidx, wepkey);
			break;
		default:
			;
	}

	fprintf(wsconf, "}\n");
	fclose(wsconf);
#if debg_wpa
	fprintf(stderr, "wpa_supplicant: ssid=%s, encry(tkip/aes)=%d, auth(wpa/wpa2)=%d, keymgmt=%d, keyidx=%d, wepkey=%s\n",ssid,encryp,auth,keymgmt,keyidx,wepkey);
#endif
	exec_WPASupplicant(ssid, encryp, auth, keymgmt, keyidx, wepkey);
}

/*
  security mode: wpa-enterprise/wpa2-enterprise
  -------------------------------------------------------------
  wpa-algorithm: tkip/aes
  -------------------------------------------------------------
  auth mode    : peap      / tls  / ttls
  tunnel       : mschap v2 / N/A  / mschap, mschap v2, pap
  account      : id & pwd  / id   / id & pwd
  cert         : N/A       / YES  / N/A
*/
void setWPAconnect(void)
{
	int  tmp_auth = 0, tmp_encry = 0, tmp_defaultkeyid = 0;
	char *tmp_ssid, *tmp_key1, *tmp_key2, *tmp_key3, *tmp_key4;
	int  tmp_keymgmt = Rtwpa_supplicantKeyMgmtNONE, tmp_eap = Rtwpa_supplicantEAPNONE, tmp_tunnel = Rtwpa_supplicantTUNNENONE;
	char *tmp_identity, *tmp_cacert, *tmp_clientcert, *tmp_privatekey, *tmp_privatekeypassword, *tmp_password;
	char *value;

	tmp_auth  = Ndis802_11AuthModeOpen;
	tmp_encry = Ndis802_11WEPDisabled;

	//read ssid & networktype
	tmp_ssid = nvram_safe_get("sta0_ssid");
	value = nvram_safe_get("NetworkType");

#if debg_wpa
	fprintf(stderr, "wpa : wlssid =%s\n",tmp_ssid);
	fprintf(stderr, "wpa : networktype =%s\n",value);
#endif
	//security mode
	if (strcmp(nvram_safe_get("NetworkType"),"Infra") == 0) {
		value = nvram_safe_get("security_infra");
		if (strcmp(value, ""))
			tmp_auth = atoi(value);
	}
	else {//adhoc
		value = nvram_safe_get("security_adhoc");
		if (strcmp(value, ""))
			tmp_auth = atoi(value);
	}

	//key management
	if (tmp_auth == Ndis802_11AuthModeWPA || tmp_auth == Ndis802_11AuthModeWPA2)
		tmp_keymgmt = Rtwpa_supplicantKeyMgmtWPAEAP;
	else if (tmp_auth == Ndis802_11AuthModeMax) //802.1x
		tmp_keymgmt= Rtwpa_supplicantKeyMgmtIEEE8021X;
	else
		tmp_keymgmt = Rtwpa_supplicantKeyMgmtNONE;

#if debg_wpa
	fprintf(stderr, "wpa : auth method (wpa,wpa2) =%d\n",tmp_auth);
	fprintf(stderr, "wpa : keymgmt (peap ,tls , ttls) =%d\n",tmp_keymgmt);
#endif
	//wep key1~4 , key string
	tmp_key1 = nvram_safe_get("sta0_key1");
	tmp_key2 = nvram_safe_get("sta0_key2");
	tmp_key3 = nvram_safe_get("sta0_key3");
	tmp_key4 = nvram_safe_get("sta0_key4");
	if (strcmp(tmp_key1, "") 
			|| strcmp(tmp_key2, "") 
			|| strcmp(tmp_key3, "") 
			|| strcmp(tmp_key4, "")) {
		// Auth mode OPEN might use encryption type: none or wep
		// if set wep key, the encry must be WEPEnable
		tmp_encry = Ndis802_11WEPEnabled;
	}

	//default key
	value =nvram_safe_get("sta0_key");
	tmp_defaultkeyid = atoi(value);

	//cipher
	if (nvram_match("sta0_crypto", "tkip"))
		tmp_encry= Ndis802_11Encryption2Enabled;
	else if (nvram_match("sta0_crypto", "aes"))
		tmp_encry = Ndis802_11Encryption3Enabled;
	else if (nvram_match("sta0_crypto", "tkip+aes"))
		tmp_encry = Ndis802_11Encryption2Enabled; //still tkip
	else
		fprintf(stderr, "do not set encryptype\n");

#if debg_wpa
	fprintf(stderr, "wpa : encryptype algorithm =%d\n",tmp_encry);
#endif
	//eap
	if ( nvram_match("sta0_auth_mode", "wpa") || nvram_match("sta0_auth_mode", "wpa2")) {
		value = nvram_safe_get("cert_auth_type_from_wpa");
		if (strcmp(value, ""))
			tmp_eap = (RT_WPA_SUPPLICANT_EAP)atoi(value);

		//tunnel
		if (tmp_eap == 5) {//peap
			value = nvram_safe_get("cert_tunnel_auth_peap");
			if (strcmp(value, ""))
				tmp_tunnel = (RT_WPA_SUPPLICANT_TUNNEL)atoi(value);
		}
		else if (tmp_eap == 6) {//ttls
			value = nvram_safe_get("cert_tunnel_auth_ttls");
			if (strcmp(value, ""))
				tmp_tunnel = (RT_WPA_SUPPLICANT_TUNNEL)atoi(value);
		}
		else //tls auth mode
			fprintf(stderr, "no tunnel\n");
	}
	else {
		value = nvram_safe_get("cert_auth_type_from_1x");
		if (strcmp(value, ""))
			tmp_eap = (RT_WPA_SUPPLICANT_EAP)atoi(value);
	}

	//certificate
	tmp_identity = nvram_safe_get("cert_id");
	tmp_password = nvram_safe_get("cert_password");
	tmp_clientcert = nvram_safe_get("cert_client_cert_path");
	tmp_privatekey = nvram_safe_get("cert_private_key_path");
	tmp_privatekeypassword = nvram_safe_get("cert_private_key_password");
	tmp_cacert = nvram_safe_get("cert_ca_cert_path");

	//encryp
	if (tmp_auth <= Ndis802_11AuthModeShared) {//only for "OPEN" and "shared" auth mode
		if (strlen((char *)tmp_key1) > 1 
				|| strlen((char *)tmp_key2) > 1 
				|| strlen((char *)tmp_key3) > 1 
				|| strlen((char *)tmp_key4) > 1)
			tmp_encry= Ndis802_11WEPEnabled;
		else
			tmp_encry = Ndis802_11WEPDisabled;
	}
	else if (tmp_auth == Ndis802_11AuthModeMax) //802.1x
		tmp_encry = Ndis802_11WEPEnabled;

	if (tmp_auth == Ndis802_11AuthModeWPA 
			|| tmp_auth == Ndis802_11AuthModeWPA2 
			|| tmp_auth == Ndis802_11AuthModeMax) {//802.1x
		char tmp_key[27];
		if (tmp_defaultkeyid == 1) // 1~4
			strcpy(tmp_key, tmp_key1);
		else if (tmp_defaultkeyid == 2)
			strcpy(tmp_key, tmp_key2);
		else if (tmp_defaultkeyid == 3)
			strcpy(tmp_key, tmp_key3);
		else if (tmp_defaultkeyid == 4)
			strcpy(tmp_key, tmp_key4);

		tmp_defaultkeyid -= 1;
#if debg_wpa
		fprintf(stderr, "wpa : ssid=%s, keymgmt=%d, tmp_eap=%d, tmp_id=%s, tmp_pw=%s, tmp_cacert=%s, tmp_clicert=%s,tmp_pr_key=%s,tmp_priv_pw=%s,tmp_key=%s, tmp_default_key=%d, tmp_encry=%d, tmp_tunnel=%d, tmp_auth=%d\n",tmp_ssid, tmp_keymgmt, tmp_eap, tmp_identity, tmp_password, tmp_cacert, tmp_clientcert, tmp_privatekey, tmp_privatekeypassword, tmp_key, tmp_defaultkeyid, tmp_encry, tmp_tunnel, tmp_auth);
#endif
		conf_WPASupplicant(tmp_ssid, tmp_keymgmt, tmp_eap, tmp_identity, tmp_password, tmp_cacert, tmp_clientcert, tmp_privatekey, tmp_privatekeypassword, tmp_key, tmp_defaultkeyid, tmp_encry, tmp_tunnel, tmp_auth);
	}
}
#endif
