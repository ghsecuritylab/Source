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
/* vi: set sw=4 ts=4: */
/*
 * Utility routines.
 *
 * Copyright (C) 2007 Denys Vlasenko
 *
 * Licensed under GPL version 2, see file LICENSE in this tarball for details.
 */

#include "libbb.h"

/*
 * This asks kernel to let us know dst addr/port of incoming packets
 * We don't check for errors here. Not supported == won't be used
 */
void FAST_FUNC
socket_want_pktinfo(int fd)
{
#ifdef IP_PKTINFO
	setsockopt(fd, IPPROTO_IP, IP_PKTINFO, &const_int_1, sizeof(int));
#endif
#if ENABLE_FEATURE_IPV6 && defined(IPV6_PKTINFO)
	setsockopt(fd, IPPROTO_IPV6, IPV6_PKTINFO, &const_int_1, sizeof(int));
#endif
}


ssize_t FAST_FUNC
send_to_from(int fd, void *buf, size_t len, int flags,
		const struct sockaddr *to,
		const struct sockaddr *from,
		socklen_t tolen)
{
#ifndef IP_PKTINFO
	return sendto(fd, buf, len, flags, to, tolen);
#else
	struct iovec iov[1];
	struct msghdr msg;
	union {
		char cmsg[CMSG_SPACE(sizeof(struct in_pktinfo))];
#if ENABLE_FEATURE_IPV6 && defined(IPV6_PKTINFO)
		char cmsg6[CMSG_SPACE(sizeof(struct in6_pktinfo))];
#endif
	} u;
	struct cmsghdr* cmsgptr;

	if (from->sa_family != AF_INET
#if ENABLE_FEATURE_IPV6
	 && from->sa_family != AF_INET6
#endif
	) {
		/* ANY local address */
		return sendto(fd, buf, len, flags, to, tolen);
	}

	/* man recvmsg and man cmsg is needed to make sense of code below */

	iov[0].iov_base = buf;
	iov[0].iov_len = len;

	memset(&u, 0, sizeof(u));

	memset(&msg, 0, sizeof(msg));
	msg.msg_name = (void *)(struct sockaddr *)to; /* or compiler will annoy us */
	msg.msg_namelen = tolen;
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;
	msg.msg_control = &u;
	msg.msg_controllen = sizeof(u);
	msg.msg_flags = flags;

	cmsgptr = CMSG_FIRSTHDR(&msg);
	if (to->sa_family == AF_INET && from->sa_family == AF_INET) {
		struct in_pktinfo *pktptr;
		cmsgptr->cmsg_level = IPPROTO_IP;
		cmsgptr->cmsg_type = IP_PKTINFO;
		cmsgptr->cmsg_len = CMSG_LEN(sizeof(struct in_pktinfo));
		pktptr = (struct in_pktinfo *)(CMSG_DATA(cmsgptr));
		/* pktptr->ipi_ifindex = 0; -- already done by memset(cbuf...) */
		pktptr->ipi_spec_dst = ((struct sockaddr_in*)from)->sin_addr;
	}
#if ENABLE_FEATURE_IPV6 && defined(IPV6_PKTINFO)
	else if (to->sa_family == AF_INET6 && from->sa_family == AF_INET6) {
		struct in6_pktinfo *pktptr;
		cmsgptr->cmsg_level = IPPROTO_IPV6;
		cmsgptr->cmsg_type = IPV6_PKTINFO;
		cmsgptr->cmsg_len = CMSG_LEN(sizeof(struct in6_pktinfo));
		pktptr = (struct in6_pktinfo *)(CMSG_DATA(cmsgptr));
		/* pktptr->ipi6_ifindex = 0; -- already done by memset(cbuf...) */
		pktptr->ipi6_addr = ((struct sockaddr_in6*)from)->sin6_addr;
	}
#endif
	msg.msg_controllen = cmsgptr->cmsg_len;

	return sendmsg(fd, &msg, flags);
#endif
}

/* NB: this will never set port# in 'to'!
 * _Only_ IP/IPv6 address part of 'to' is _maybe_ modified.
 * Typical usage is to preinit 'to' with "default" value
 * before calling recv_from_to(). */
ssize_t FAST_FUNC
recv_from_to(int fd, void *buf, size_t len, int flags,
		struct sockaddr *from, struct sockaddr *to,
		socklen_t sa_size)
{
#ifndef IP_PKTINFO
	return recvfrom(fd, buf, len, flags, from, &sa_size);
#else
	/* man recvmsg and man cmsg is needed to make sense of code below */
	struct iovec iov[1];
	union {
		char cmsg[CMSG_SPACE(sizeof(struct in_pktinfo))];
#if ENABLE_FEATURE_IPV6 && defined(IPV6_PKTINFO)
		char cmsg6[CMSG_SPACE(sizeof(struct in6_pktinfo))];
#endif
	} u;
	struct cmsghdr *cmsgptr;
	struct msghdr msg;
	ssize_t recv_length;

	iov[0].iov_base = buf;
	iov[0].iov_len = len;

	memset(&msg, 0, sizeof(msg));
	msg.msg_name = (struct sockaddr *)from;
	msg.msg_namelen = sa_size;
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;
	msg.msg_control = &u;
	msg.msg_controllen = sizeof(u);

	recv_length = recvmsg(fd, &msg, flags);
	if (recv_length < 0)
		return recv_length;

	/* Here we try to retrieve destination IP and memorize it */
	for (cmsgptr = CMSG_FIRSTHDR(&msg);
			cmsgptr != NULL;
			cmsgptr = CMSG_NXTHDR(&msg, cmsgptr)
	) {
		if (cmsgptr->cmsg_level == IPPROTO_IP
		 && cmsgptr->cmsg_type == IP_PKTINFO
		) {
#define pktinfo(cmsgptr) ( (struct in_pktinfo*)(CMSG_DATA(cmsgptr)) )
			to->sa_family = AF_INET;
			((struct sockaddr_in*)to)->sin_addr = pktinfo(cmsgptr)->ipi_addr;
			/* ((struct sockaddr_in*)to)->sin_port = 123; */
#undef pktinfo
			break;
		}
#if ENABLE_FEATURE_IPV6 && defined(IPV6_PKTINFO)
		if (cmsgptr->cmsg_level == IPPROTO_IPV6
		 && cmsgptr->cmsg_type == IPV6_PKTINFO
		) {
#define pktinfo(cmsgptr) ( (struct in6_pktinfo*)(CMSG_DATA(cmsgptr)) )
			to->sa_family = AF_INET6;
			((struct sockaddr_in6*)to)->sin6_addr = pktinfo(cmsgptr)->ipi6_addr;
			/* ((struct sockaddr_in6*)to)->sin6_port = 123; */
#undef pktinfo
			break;
		}
#endif
	}
	return recv_length;
#endif
}
