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

#include <stdio.h>

#include <nvram/bcmnvram.h>
#include <net/ethernet.h>

#include "ralink.h"
#include "iwlib.h"

/*
 * Constants fof WE-9->15
 */
#define IW15_MAX_FREQUENCIES	16
#define IW15_MAX_BITRATES	8
#define IW15_MAX_TXPOWER	8
#define IW15_MAX_ENCODING_SIZES	8
#define IW15_MAX_SPY		8
#define IW15_MAX_AP		8

/*
 *	Struct iw_range up to WE-15
 */
struct	iw15_range {
	__u32		throughput;
	__u32		min_nwid;
	__u32		max_nwid;
	__u16		num_channels;
	__u8		num_frequency;
	struct iw_freq	freq[IW15_MAX_FREQUENCIES];
	__s32		sensitivity;
	struct iw_quality	max_qual;
	__u8		num_bitrates;
	__s32		bitrate[IW15_MAX_BITRATES];
	__s32		min_rts;
	__s32		max_rts;
	__s32		min_frag;
	__s32		max_frag;
	__s32		min_pmp;
	__s32		max_pmp;
	__s32		min_pmt;
	__s32		max_pmt;
	__u16		pmp_flags;
	__u16		pmt_flags;
	__u16		pm_capa;
	__u16		encoding_size[IW15_MAX_ENCODING_SIZES];
	__u8		num_encoding_sizes;
	__u8		max_encoding_tokens;
	__u16		txpower_capa;
	__u8		num_txpower;
	__s32		txpower[IW15_MAX_TXPOWER];
	__u8		we_version_compiled;
	__u8		we_version_source;
	__u16		retry_capa;
	__u16		retry_flags;
	__u16		r_time_flags;
	__s32		min_retry;
	__s32		max_retry;
	__s32		min_r_time;
	__s32		max_r_time;
	struct iw_quality	avg_qual;
};

/*
 * Union for all the versions of iwrange.
 * Fortunately, I mostly only add fields at the end, and big-bang
 * reorganisations are few.
 */
union iw_range_raw {
	struct iw15_range	range15;	/* WE 9->15 */
	struct iw_range		range;		/* WE 16->current */
};

/*
 * Offsets in iw_range struct
 */
#define iwr15_off(f)	( ((char *) &(((struct iw15_range *) NULL)->f)) - \
			  (char *) NULL)
#define iwr_off(f)	( ((char *) &(((struct iw_range *) NULL)->f)) - \
			  (char *) NULL)

/* Disable runtime version warning in ralink_get_range_info() */
int iw_ignore_version_sp = 0;

/*
 * Get the range information out of the driver
 */
int ralink_get_range_info(char *wif, iwrange *range, char* buffer, int length)
{
	union iw_range_raw *range_raw;

	/* Point to the buffer */
	range_raw = (union iw_range_raw *) buffer;

	/* For new versions, we can check the version directly, for old versions
	 * we use magic. 300 bytes is a also magic number, don't touch... */
	if (length < 300) {
		/* That's v10 or earlier. Ouch ! Let's make a guess...*/
		range_raw->range.we_version_compiled = 9;
	}

	/* Check how it needs to be processed */
	if (range_raw->range.we_version_compiled > 15) {
		/* This is our native format, that's easy... */
		/* Copy stuff at the right place, ignore extra */
		memcpy((char *) range, buffer, sizeof(iwrange));
	}
	else {
		/* Zero unknown fields */
		bzero((char *) range, sizeof(struct iw_range));

		/* Initial part unmoved */
		memcpy((char *) range, buffer, iwr15_off(num_channels));
		/* Frequencies pushed futher down towards the end */
		memcpy((char *) range + iwr_off(num_channels),
			buffer + iwr15_off(num_channels),
			iwr15_off(sensitivity) - iwr15_off(num_channels));
		/* This one moved up */
		memcpy((char *) range + iwr_off(sensitivity),
			buffer + iwr15_off(sensitivity),
			iwr15_off(num_bitrates) - iwr15_off(sensitivity));
		/* This one goes after avg_qual */
		memcpy((char *) range + iwr_off(num_bitrates),
			buffer + iwr15_off(num_bitrates),
			iwr15_off(min_rts) - iwr15_off(num_bitrates));
		/* Number of bitrates has changed, put it after */
		memcpy((char *) range + iwr_off(min_rts),
			buffer + iwr15_off(min_rts),
			iwr15_off(txpower_capa) - iwr15_off(min_rts));
		/* Added encoding_login_index, put it after */
		memcpy((char *) range + iwr_off(txpower_capa),
			buffer + iwr15_off(txpower_capa),
			iwr15_off(txpower) - iwr15_off(txpower_capa));
		/* Hum... That's an unexpected glitch. Bummer. */
		memcpy((char *) range + iwr_off(txpower),
			buffer + iwr15_off(txpower),
			iwr15_off(avg_qual) - iwr15_off(txpower));
		/* Avg qual moved up next to max_qual */
		memcpy((char *) range + iwr_off(avg_qual),
			buffer + iwr15_off(avg_qual),
			sizeof(struct iw_quality));
	}

	/* We are now checking much less than we used to do, because we can
	 * accomodate more WE version. But, there are still cases where things
	 * will break... */
	if (!iw_ignore_version_sp) {
		/* We don't like very old version (unfortunately kernel 2.2.X) */
		if (range->we_version_compiled <= 10) {
			fprintf(stderr, "Warning: Driver for device %s has been compiled with an ancient version\n", wif);
			fprintf(stderr, "of Wireless Extension, while this program support version 11 and later.\n");
			fprintf(stderr, "Some things may be broken...\n\n");
		}

		/* We don't like future versions of WE, because we can't cope with
		 * the unknown */
		if (range->we_version_compiled > WE_MAX_VERSION) {
			fprintf(stderr, "Warning: Driver for device %s has been compiled with version %d\n", wif, range->we_version_compiled);
			fprintf(stderr, "of Wireless Extension, while this program supports up to version %d.\n", WE_VERSION);
			fprintf(stderr, "Some things may be broken...\n\n");
		}

		/* Driver version verification */
		if((range->we_version_compiled > 10) && (range->we_version_compiled < range->we_version_source)) {
			fprintf(stderr, "Warning: Driver for device %s recommend version %d of Wireless Extension,\n", wif, range->we_version_source);
			fprintf(stderr, "but has been compiled with version %d, therefore some driver features\n", range->we_version_compiled);
			fprintf(stderr, "may not be available...\n\n");
		}
	/* Note : we are only trying to catch compile difference, not source.
	 * If the driver source has not been updated to the latest, it doesn't
	 * matter because the new fields are set to zero */
	}

	/* Don't complain twice.
	 * In theory, the test apply to each individual driver, but usually
	 * all drivers are compiled from the same kernel. */
	iw_ignore_version_sp = 1;

	return 0;
}

/*
 * Get wireless informations & config from the device driver
 * We will call all the classical wireless ioctl on the driver through
 * the socket to know what is supported and to get the settings...
 */
int get_info(int skfd, char *ifname, struct wireless_info *info)
{
	struct iwreq wrq;

	memset((char *) info, 0, sizeof(struct wireless_info));
	/* Get basic information */
	if (iw_get_basic_config(skfd, ifname, &(info->b)) < 0) {
		/* If no wireless name : no wireless extensions */
		/* But let's check if the interface exists at all */
		struct ifreq ifr;

		strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
		if (ioctl(skfd, SIOCGIFFLAGS, &ifr) < 0)
			return -ENODEV;
		else
			return -ENOTSUP;
	}

	/* Get AP address */
	if (iw_get_ext(skfd, ifname, SIOCGIWAP, &wrq) >= 0) {
		info->has_ap_addr = 1;
		memcpy(&(info->ap_addr), &(wrq.u.ap_addr), sizeof (sockaddr));
	}
	else
		return -1;

	return 0;
}


int wl_ioctl(const char *ifname, int cmd, struct iwreq *pwrq)
{
	int ret = 0;
	int s;

	/* open socket to kernel */
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		return errno;
	}

	/* do it */
	memcpy(pwrq->ifr_name, ifname, IFNAMSIZ);
	if ((ret = ioctl(s, cmd, pwrq)) < 0)
		perror(pwrq->ifr_name);

	/* cleanup */
	close(s);
	return ret;
}

int asuscmd_wl_ioctl(const char *ifname, int flags, char *buf)
{
	struct iwreq wrq;

	wrq.u.data.pointer = buf;
	wrq.u.data.length = sizeof(buf);
	wrq.u.data.flags = flags;

	if (wl_ioctl(ifname, RTPRIV_IOCTL_ASUSCMD, &wrq) < 0)
		return -1;

	return 0;
}

int get_apcli_quality(char *wif, int *quality)
{
	char data[16];

	memset(data, 0, sizeof(data));
	*quality = 0;

	if (asuscmd_wl_ioctl(wif, ASUS_SUBCMD_CLIQ, data) < 0) {
		fprintf(stderr, "wl_ioctl error!!!\n");
		return -1;
	}

	*quality = atoi(data);

	return 0;
}

int get_channel_list_via_driver(char *wif, char *ch_list)
{
	char data[128];

	memset(data, 0, sizeof(data));

	if (asuscmd_wl_ioctl(wif, ASUS_SUBCMD_CHLIST, data) < 0) {
		fprintf(stderr, "wl_ioctl error!!!\n");
		return -1;
	}

	strcpy(ch_list, data);

	return strlen(data);
}

int get_apcli_connStatus(char *wif, int *connStatus)
{
	char data[2];

	memset(data, 0, sizeof(data));

	if (asuscmd_wl_ioctl(wif, ASUS_SUBCMD_CONN_STATUS, data) < 0) {
		fprintf(stderr, "wl_ioctl error!!!\n");
		return -1;
	}

	*connStatus = atoi(data);

	return 0;
}

int get_extcha(char *wif)
{
	char data[3];

	memset(data, 0, sizeof(data));

	if (asuscmd_wl_ioctl(wif, ASUS_SUBCMD_GEXTCHA, data) < 0) {
		fprintf(stderr, "wl_ioctl error!!!\n");
		return -1;
	}

	return atoi(data);
}

void RPN53_get_cli_strengh(char *wif, int *outpt)
{
	int value;
	if (get_apcli_quality(wif, &value) != 0)
		value = 100;
	if (value >= 70)
		*outpt = 3;
	else if (value >= 45)
		*outpt = 2;
	else
		*outpt = 1;
	//printf("[%s]value is %d, *outpt is %d\n",wif,value,*outpt);
}

/* 
 * like: iwpriv [wif] set [pv_pair]
 */
int ap_set(char *wif, const char *pv_pair)
{
	struct iwreq wrq;
	char data[256];

	memset(data, 0x0, 256);
	strcpy(data, pv_pair);
	wrq.u.data.pointer = data;
	wrq.u.data.length = strlen(data);
	wrq.u.data.flags = 0;

	fprintf(stderr, "[%s] set %s\n", wif, pv_pair);

	if (wl_ioctl(wif, RTPRIV_IOCTL_SET, &wrq) < 0)
		return 0;
	else
		return 1;
}

/* 
 * run-time change WirelessMode for scan all AP
 */
void change_WirelessMode(int n, char *wif)
{
	if (!n)
		ap_set(wif, "WirelessMode=9");
	else
#ifdef IEEE802_11AC
		ap_set(wif, "WirelessMode=14");
#else
		ap_set(wif, "WirelessMode=8");
#endif
}

/* 
 * for WirelessMode parameter of wireless driver convert wl_nmode_x to WirelessMode
 */
int wl_nmode2wmode(int n, int nmode)
{
#ifdef DUAL_BAND_NONCONCURRENT
	if (nvram_match("cur_freq", "1")) {
#else
	if (n) {
#endif
		if (nmode == 0)		// (Auto)
#ifdef IEEE802_11AC
			return 14;	// A + AN + AC mixed
#else
			return 8;	// A + AN mixed
#endif
		else if (nmode == 1)	// (N Only)
#ifdef IEEE802_11AC
			return 15;	// AN + AC mixed
#else
			return 11;	// N in 5G
#endif
		else if (nmode == 2)	// A (Legacy)
			return 2;
		else {			// (Auto)
			fprintf(stderr, "%s: invaild wl1_nmode_x\n", __FUNCTION__);
#ifdef IEEE802_11AC
			return 14;	// A + AN + AC mixed
#else
			return 8;	// A + AN mixed
#endif
		}
	}
	else {
		if (nmode == 0)		// B,G,N (Auto)
			return 9;
		else if (nmode == 1)	// N (N Only)
			return 6;
		else if (nmode == 2)	// B,G (Legacy)
			return 0;
		else {			// B,G,N (Auto)
			fprintf(stderr, "%s: invaild wl0_nmode_x\n", __FUNCTION__);
			return 9;
		}
	}
}

/* 
 * check connection status of Adapter mode
 */
int ea_connStatus(void)
{
	FILE *fp;
	char connStatus[512];
	int rlen;
	fp = popen("iwpriv ra0 connStatus", "r");
	if (fp) {
		rlen = fread(connStatus, 1, sizeof(connStatus), fp);
		pclose(fp);
		if (rlen > 1) {
			connStatus[rlen-1] = '\0';
			if (strstr(connStatus, "Connected(AP: "))
				return 1;
		}
	}
	return 0;
}
