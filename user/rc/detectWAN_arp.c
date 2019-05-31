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
#include <errno.h>
#include <signal.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <netpacket/packet.h>
#include <linux/sockios.h>

#include <nvram/bcmnvram.h>

#include <shared.h>

#pragma (1)
struct globals {
	struct in_addr src;
	struct in_addr dst;
	struct sockaddr_ll me;
	struct sockaddr_ll he;
	int sock_fd;

	int count;
	unsigned last;
	unsigned timeout_us;
	unsigned start;

	unsigned sent;
	unsigned brd_sent;
	unsigned received;
	unsigned brd_recv;
	unsigned req_recv;
};

static char gbuf[sizeof(struct globals)];
static unsigned char inputpacket[4096];
static int fail_counts = 0;
static sigset_t sset, osset;

static int _sw_mode;
static int _lan_proto_x;
static char _wan_proto[8];
#pragma pack()

#define G (*(struct globals*)&gbuf)
#define src        (G.src       )
#define dst        (G.dst       )
#define me         (G.me        )
#define he         (G.he        )
#define sock_fd    (G.sock_fd   )
#define count      (G.count     )
#define last       (G.last      )
#define timeout_us (G.timeout_us)
#define start      (G.start     )
#define sent       (G.sent      )
#define brd_sent   (G.brd_sent  )
#define received   (G.received  )
#define brd_recv   (G.brd_recv  )
#define req_recv   (G.req_recv  )
#define INIT_G() do { \
        count = -1; \
} while (0)

static void block_sig(void)
{
	sigemptyset(&sset);
	sigaddset(&sset, SIGALRM);
	sigprocmask(SIG_BLOCK, &sset, &osset);
}

static void restore_sig(void)
{
	sigprocmask(SIG_SETMASK, &osset, NULL);
}

static void force_client_renew_ip(void)
{
	restart_dnsmasq(1);
}

static int send_pack(struct in_addr *src_addr, struct in_addr *dst_addr, struct sockaddr_ll *ME, struct sockaddr_ll *HE)
{
	int err;
	unsigned char buf[256];
	struct arphdr *ah = (struct arphdr*) buf;
	unsigned char *p = (unsigned char*) (ah + 1);
#ifdef DEBUG
	int i;
#endif

	ah->ar_hrd = htons(ARPHRD_ETHER);
	ah->ar_pro = htons(ETH_P_IP);
	ah->ar_hln = ME->sll_halen;
	ah->ar_pln = 4;
	ah->ar_op = htons(ARPOP_REQUEST);
#ifdef DEBUG
	printf("[detectWan_arp] me ha_addr:(%d): ", ah->ar_hln);
	for (i = 0; i < ah->ar_hln; ++i)
		printf("[%x]", *((unsigned char*)&HE->sll_addr + i));
	printf("\n");
#endif

	p = mempcpy(p, &ME->sll_addr, ah->ar_hln);
	p = mempcpy(p, src_addr, 4);
	p = mempcpy(p, &HE->sll_addr, ah->ar_hln);
	p = mempcpy(p, dst_addr, 4);

	err = sendto(sock_fd, buf, p - buf, 0, (struct sockaddr*)HE, sizeof(*HE));
#ifdef DEBUG
	printf("[detectWan_arp] send %d (%d)bytes:\n", err, p-buf);
#endif
	++fail_counts;		// increase here at stable rate

	return err;
}

static int recv_pack(unsigned char *buf, int len, struct sockaddr_ll *FROM)
{
	struct arphdr *ah = (struct arphdr*) buf;
	unsigned char *p = (unsigned char*) (ah + 1);
	struct in_addr src_ip, dst_ip;

        /* Filter out wild packets */
	if (FROM->sll_pkttype != PACKET_HOST 
			&& FROM->sll_pkttype != PACKET_BROADCAST 
			&& FROM->sll_pkttype != PACKET_MULTICAST) {
#ifdef DEBUG
		printf("[detectWan_arp] recv pack filter: wrong pkt type\n");
#endif
		return 0;
	}

	if (ah->ar_op != htons(ARPOP_REQUEST) && ah->ar_op != htons(ARPOP_REPLY)) {
#ifdef DEBUG
		printf("[detectWan_arp] recv pack filter: wrong op\n");
#endif
		return 0;
	}

        /* hrd chk and FDDI hack */
	if (ah->ar_hrd != htons(FROM->sll_hatype) 
			&& (FROM->sll_hatype != ARPHRD_FDDI || ah->ar_hrd != htons(ARPHRD_ETHER))) {
#ifdef DEBUG
		printf("[detectWan_arp] recv pack filter: doubt FDDI !\n");
#endif
		return 0;
	}

	/* protocol must be ip */
	if (ah->ar_pro != htons(ETH_P_IP) 
			|| (ah->ar_pln != 4) 
			|| (ah->ar_hln != me.sll_halen) 
			|| (len < (int)sizeof(*ah) + 2*(4 + ah->ar_hln))) {
#ifdef DEBUG
		printf("[detectWan_arp] recv pack filter: wrong pro/len\n");
#endif
		return 0;
	}

	memcpy(&src_ip, p + ah->ar_hln, 4);
	memcpy(&dst_ip, p + ah->ar_hln + 4 + ah->ar_hln, 4);

	if (dst.s_addr != src_ip.s_addr) {
#ifdef DEBUG
		printf("[detectWan_arp] recv pack filter: wrong addr\n");
#endif
		return 0;
	}

#ifdef DEBUG
	printf("[detectWan_arp] %scast re%s from %s[%s]\n", 
		FROM->sll_pkttype == PACKET_HOST ? "Uni" : "Broad", 
		ah->ar_op == htons(ARPOP_REPLY) ? "ply" : "quest", 
		inet_ntoa(src_ip), 
		ether_ntoa((struct ether_addr*) p));
		fflush(stdout);
#endif
	fail_counts = 0;

	// remove lost gateway (static IP) situation
	if (nvram_match("wan_state", "3"))
		nvram_set("wan_state", "0");

	return 1;
}

static void catcher(int sig)
{
	send_pack(&src, &dst, &me, &he);
	alarm(10);
}

static int wait_response(int sock, struct sockaddr_in *from, int reset_timer)
{
	fd_set fds;
	int recv_len = 0;
	int fromlen = sizeof(struct sockaddr_in);
	struct timeval wait_timeval;

	FD_ZERO(&fds);
	FD_SET(sock, &fds);

	wait_timeval.tv_sec = reset_timer;
	wait_timeval.tv_usec = 0;

	if (select(sock+1, &fds, (fd_set *)0, (fd_set *)0, &wait_timeval) > 0)
		recv_len = recvfrom(sock, (char *)inputpacket, sizeof(inputpacket), 0, (struct sockaddr *)from, &fromlen);

#ifdef DEBUG
	printf("[detectWan_arp] wait test: from:%s(%d)\n", inet_ntoa(from->sin_addr), recv_len);
#endif

	return recv_len;
}

static void poll_udhcpc(void)
{
	struct timeval tval;
	char *gateway_ip = NULL;

	for(;;) {
		tval.tv_sec = 10;
		tval.tv_usec = 0;
		select(0, NULL, NULL, NULL, &tval);

		if (_sw_mode == 1)
			gateway_ip = nvram_safe_get("wan_gateway");
		else
			gateway_ip = nvram_safe_get("lan_gateway");

		if (nvram_match("wan_state", "2") 
				|| (!gateway_ip || (gateway_ip && strlen(gateway_ip) < 7) 
						|| (gateway_ip && !strncmp(gateway_ip, "0.0.0.0", 7)))) {
#ifdef DEBUG
			fprintf(stderr, "[detectWan_arp] renew IP...\n");
#endif
			eval("killall", "-SIGUSR1", "udhcpc");
		}
		else {
#ifdef DEBUG
			fprintf(stderr, "[detectWan_arp] we have gateway: %s\n", gateway_ip);
#endif
			break;
		}
	}
}

static int detectARP(void)
{
	char *device, *source, *target;
	struct ifreq ifr;

	if (_sw_mode == 1) {
		source = nvram_safe_get("wan_ipaddr");
		target = nvram_safe_get("wan_gateway");
		device = nvram_safe_get("wan_ifname");
	}
	else {
		source = nvram_safe_get("lan_ipaddr");
		target = nvram_safe_get("lan_gateway");
		device = nvram_safe_get("lan_ifname");
	}

#ifdef DEBUG
	fprintf(stderr, "[detectWan_arp]: source:[%s] taget:[%s], dev:[%s]\n", source, target, device);
#endif

	sock_fd = socket(AF_PACKET, SOCK_DGRAM, 0);
	if (sock_fd < 0) {
		perror("sockfd");
		return -1;
	}
	else if (sock_fd < 3) {
		fprintf(stderr, "wierd sockfd(%d) !\n", sock_fd);
		close(sock_fd);
		return -1;
	}

	memset(&ifr, 0x0, sizeof(ifr));
	strncpy(ifr.ifr_name, device, sizeof(ifr.ifr_name) - 1);
	ioctl(sock_fd, SIOCGIFINDEX, (char*)&ifr);
	me.sll_ifindex = ifr.ifr_ifindex;
	ioctl(sock_fd, SIOCGIFFLAGS, (char*)&ifr);
	if (!ifr.ifr_flags & IFF_UP) {
#ifdef DEBUG
		fprintf(stderr, "[detectWan_arp] %s is down\n", device);
#endif
		close(sock_fd);
		return -1;
	}

	inet_aton(target, &dst);
	inet_aton(source, &src);

	if (src.s_addr) {
		struct sockaddr_in saddr;
		int probe_fd = socket(AF_INET, SOCK_DGRAM, 0);

		if (setsockopt(probe_fd, SOL_SOCKET, SO_BINDTODEVICE, device, strlen(device)+1) == -1)
			perror("BindToDevice");
		memset(&saddr, 0, sizeof(saddr));
		saddr.sin_family = AF_INET;
		saddr.sin_addr = src;
		bind(probe_fd, (struct sockaddr*) &saddr, sizeof(saddr));
		close(probe_fd);
	}
	else
		printf("no saddr !\n");

	me.sll_family = AF_PACKET;
	me.sll_protocol = htons(ETH_P_ARP);
	bind(sock_fd, (struct sockaddr*)&me, sizeof(me));

	socklen_t alen = sizeof(me);
	/* get hwaddr here */
	if (getsockname(sock_fd, (struct sockaddr*)&me, &alen) == -1) {
		perror("getsockname");
		close(sock_fd);
		return -1;
	}
	he = me;
	memset(he.sll_addr, -1, he.sll_halen);
#ifdef DEBUG
	fprintf(stderr, "[detectWan_arp] ARPING to %s", inet_ntoa(dst));
	fprintf(stderr, " from %s via %s\n", inet_ntoa(src), device);
#endif

	catcher(0);

	while(1) {
		struct sockaddr_in from;
		int cc;

#ifdef DEBUG
		fprintf(stderr, "[detectWan_arp] wait response...(%d)\n", fail_counts);
#endif

		if (fail_counts > 3) {
			fail_counts = 0;

			/* enable DHCP server */
			if (_sw_mode == 3 
					&& !nvram_match("dnsqmode", "2") 
					&& (nvram_match("dhcp_enable_x", "1") 
						|| nvram_match("x_Setting", "0"))) {
				nvram_set("dnsqmode", "2");
				force_client_renew_ip();
			}

			if (_sw_mode == 1 && !strcmp(_wan_proto, "dhcp"))
				nvram_set("wan_gateway", "");
			else if (_sw_mode == 3 && _lan_proto_x == 1)
				nvram_set("lan_gateway", "");
			// lost gateway (static IP) situation
			else {
				nvram_set("wan_state", "3");
				goto static_lost_gw;
			}
			goto dhcp_lost_gw;
		}
		else if (nvram_match("wan_state", "2"))
			goto dhcp_lost_gw;

		memset(&from, 0, sizeof(from));
		cc = wait_response(sock_fd, &from, 2);
		if (cc <= 0) {
			if (errno != EAGAIN && errno != EINTR) {
				printf("[err:%d] ", errno);
				perror("recvfrom");
			}
			continue;
		}

#ifdef DEBUG
		fprintf(stderr, "[detectWan_arp] recv %d bytes\n", cc);
#endif

		block_sig();
		recv_pack(inputpacket, cc, (struct sockaddr_ll*)&from);
		restore_sig();

		/* disable DHCP server */
		if (!fail_counts 
				&& _sw_mode == 3 
				&& _lan_proto_x == 0 
				&& !nvram_match("dnsqmode", "1") 
				&& (nvram_match("dhcp_enable_x", "1") 
					|| nvram_match("x_Setting", "0"))) {
			nvram_set("dnsqmode", "1");
			force_client_renew_ip();
		}
	}

	close(sock_fd);
	return 0;

dhcp_lost_gw:
#ifdef DEBUG
	fprintf(stderr, "[detectWan_arp] we lost gateway, request udhcpc...\n");
#endif
	block_sig();
	poll_udhcpc();
	restore_sig();

static_lost_gw:
	close(sock_fd);
	return 1;
}

int detectWAN_arp_main(int argc, char **argv)
{
	_sw_mode = atoi(nvram_safe_get("sw_mode"));
	_lan_proto_x = atoi(nvram_safe_get("lan_proto_x"));
	memset(_wan_proto, 0x0, sizeof(_wan_proto));
	strcpy(_wan_proto, nvram_safe_get("wan_proto"));

	if (!strcmp(_wan_proto, "pppoe") 
			|| !strcmp(_wan_proto, "pptp") 
			|| !strcmp(_wan_proto, "l2tp"))
		return -1;

	signal(SIGALRM, catcher);

	poll_udhcpc();
	INIT_G();

	while(1) {
		detectARP();
		sleep(2);
	}

	return 0;
}
