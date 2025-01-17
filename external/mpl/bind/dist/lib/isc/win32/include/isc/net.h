/*	$NetBSD: net.h,v 1.7 2022/09/23 12:15:35 christos Exp $	*/

/*
 * Copyright (C) Internet Systems Consortium, Inc. ("ISC")
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0.  If a copy of the MPL was not distributed with this
 * file, you can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * See the COPYRIGHT file distributed with this work for additional
 * information regarding copyright ownership.
 */

#ifndef ISC_NET_H
#define ISC_NET_H 1

/*****
***** Module Info
*****/

/*
 * Basic Networking Types
 *
 * This module is responsible for defining the following basic networking
 * types:
 *
 *		struct in_addr
 *		struct in6_addr
 *		struct in6_pktinfo
 *		struct sockaddr
 *		struct sockaddr_in
 *		struct sockaddr_in6
 *		in_port_t
 *
 * It ensures that the AF_ and PF_ macros are defined.
 *
 * It declares ntoh[sl]() and hton[sl]().
 *
 * It declares inet_ntop(), and inet_pton().
 *
 * It ensures that INADDR_ANY, IN6ADDR_ANY_INIT, in6addr_any, and
 * in6addr_loopback are available.
 *
 * It ensures that IN_MULTICAST() is available to check for multicast
 * addresses.
 *
 * MP:
 *	No impact.
 *
 * Reliability:
 *	No anticipated impact.
 *
 * Resources:
 *	N/A.
 *
 * Security:
 *	No anticipated impact.
 *
 * Standards:
 *	BSD Socket API
 *	RFC2553
 */

/***
 *** Imports.
 ***/
#include <inttypes.h>

#include <isc/platform.h>

/*
 * Because of some sort of problem in the MS header files, this cannot
 * be simple "#include <winsock2.h>", because winsock2.h tries to include
 * windows.h, which then generates an error out of mswsock.h.  _You_
 * figure it out.
 */
#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_ /* Prevent inclusion of winsock.h in windows.h */
#endif		     /* ifndef _WINSOCKAPI_ */

#include <winsock2.h>
#include <ws2tcpip.h>

#include <isc/ipv6.h>
#include <isc/lang.h>
#include <isc/types.h>

#include <sys/types.h>

/*
 * This is here because named client, interfacemgr.c, etc. use the name as
 * a variable
 */
#undef interface

#ifndef INADDR_ANY
#define INADDR_ANY 0x00000000UL
#endif /* ifndef INADDR_ANY */

#ifndef INADDR_LOOPBACK
#define INADDR_LOOPBACK 0x7f000001UL
#endif /* ifndef INADDR_LOOPBACK */

#if _MSC_VER < 1300
#define in6addr_any	 isc_in6addr_any
#define in6addr_loopback isc_in6addr_loopback
#endif /* if _MSC_VER < 1300 */

/*
 * Ensure type in_port_t is defined.
 */
typedef uint16_t in_port_t;

/*
 * If this system does not have MSG_TRUNC (as returned from recvmsg())
 * ISC_PLATFORM_RECVOVERFLOW will be defined.  This will enable the MSG_TRUNC
 * faking code in socket.c.
 */
#ifndef MSG_TRUNC
#define ISC_PLATFORM_RECVOVERFLOW
#endif /* ifndef MSG_TRUNC */

#define ISC__IPADDR(x) ((uint32_t)htonl((uint32_t)(x)))

#define ISC_IPADDR_ISMULTICAST(i) \
	(((uint32_t)(i)&ISC__IPADDR(0xf0000000)) == ISC__IPADDR(0xe0000000))

#define ISC_IPADDR_ISEXPERIMENTAL(i) \
	(((uint32_t)(i)&ISC__IPADDR(0xf0000000)) == ISC__IPADDR(0xf0000000))

/*
 * Fix the FD_SET and FD_CLR Macros to properly cast
 */
#undef FD_CLR
#define FD_CLR(fd, set)                                                        \
	do {                                                                   \
		u_int __i;                                                     \
		for (__i = 0; __i < ((fd_set FAR *)(set))->fd_count; __i++) {  \
			if (((fd_set FAR *)(set))->fd_array[__i] ==            \
			    (SOCKET)fd) {                                      \
				while (__i <                                   \
				       ((fd_set FAR *)(set))->fd_count - 1) {  \
					((fd_set FAR *)(set))->fd_array[__i] = \
						((fd_set FAR *)(set))          \
							->fd_array[__i + 1];   \
					__i++;                                 \
				}                                              \
				((fd_set FAR *)(set))->fd_count--;             \
				break;                                         \
			}                                                      \
		}                                                              \
	} while (0)

#undef FD_SET
#define FD_SET(fd, set)                                                       \
	do {                                                                  \
		u_int __i;                                                    \
		for (__i = 0; __i < ((fd_set FAR *)(set))->fd_count; __i++) { \
			if (((fd_set FAR *)(set))->fd_array[__i] ==           \
			    (SOCKET)(fd)) {                                   \
				break;                                        \
			}                                                     \
		}                                                             \
		if (__i == ((fd_set FAR *)(set))->fd_count) {                 \
			if (((fd_set FAR *)(set))->fd_count < FD_SETSIZE) {   \
				((fd_set FAR *)(set))->fd_array[__i] =        \
					(SOCKET)(fd);                         \
				((fd_set FAR *)(set))->fd_count++;            \
			}                                                     \
		}                                                             \
	} while (0)

/*
 * Windows Sockets errors redefined as regular Berkeley error constants.
 * These are usually commented out in Windows NT to avoid conflicts with
 * errno.h. Use the WSA constants instead.
 */

#include <errno.h>

#ifndef EWOULDBLOCK
#define EWOULDBLOCK WSAEWOULDBLOCK
#endif /* ifndef EWOULDBLOCK */
#ifndef EINPROGRESS
#define EINPROGRESS WSAEINPROGRESS
#endif /* ifndef EINPROGRESS */
#ifndef EALREADY
#define EALREADY WSAEALREADY
#endif /* ifndef EALREADY */
#ifndef ENOTSOCK
#define ENOTSOCK WSAENOTSOCK
#endif /* ifndef ENOTSOCK */
#ifndef EDESTADDRREQ
#define EDESTADDRREQ WSAEDESTADDRREQ
#endif /* ifndef EDESTADDRREQ */
#ifndef EMSGSIZE
#define EMSGSIZE WSAEMSGSIZE
#endif /* ifndef EMSGSIZE */
#ifndef EPROTOTYPE
#define EPROTOTYPE WSAEPROTOTYPE
#endif /* ifndef EPROTOTYPE */
#ifndef ENOPROTOOPT
#define ENOPROTOOPT WSAENOPROTOOPT
#endif /* ifndef ENOPROTOOPT */
#ifndef EPROTONOSUPPORT
#define EPROTONOSUPPORT WSAEPROTONOSUPPORT
#endif /* ifndef EPROTONOSUPPORT */
#ifndef ESOCKTNOSUPPORT
#define ESOCKTNOSUPPORT WSAESOCKTNOSUPPORT
#endif /* ifndef ESOCKTNOSUPPORT */
#ifndef EOPNOTSUPP
#define EOPNOTSUPP WSAEOPNOTSUPP
#endif /* ifndef EOPNOTSUPP */
#ifndef EPFNOSUPPORT
#define EPFNOSUPPORT WSAEPFNOSUPPORT
#endif /* ifndef EPFNOSUPPORT */
#ifndef EAFNOSUPPORT
#define EAFNOSUPPORT WSAEAFNOSUPPORT
#endif /* ifndef EAFNOSUPPORT */
#ifndef EADDRINUSE
#define EADDRINUSE WSAEADDRINUSE
#endif /* ifndef EADDRINUSE */
#ifndef EADDRNOTAVAIL
#define EADDRNOTAVAIL WSAEADDRNOTAVAIL
#endif /* ifndef EADDRNOTAVAIL */
#ifndef ENETDOWN
#define ENETDOWN WSAENETDOWN
#endif /* ifndef ENETDOWN */
#ifndef ENETUNREACH
#define ENETUNREACH WSAENETUNREACH
#endif /* ifndef ENETUNREACH */
#ifndef ENETRESET
#define ENETRESET WSAENETRESET
#endif /* ifndef ENETRESET */
#ifndef ECONNABORTED
#define ECONNABORTED WSAECONNABORTED
#endif /* ifndef ECONNABORTED */
#ifndef ECONNRESET
#define ECONNRESET WSAECONNRESET
#endif /* ifndef ECONNRESET */
#ifndef ENOBUFS
#define ENOBUFS WSAENOBUFS
#endif /* ifndef ENOBUFS */
#ifndef EISCONN
#define EISCONN WSAEISCONN
#endif /* ifndef EISCONN */
#ifndef ENOTCONN
#define ENOTCONN WSAENOTCONN
#endif /* ifndef ENOTCONN */
#ifndef ESHUTDOWN
#define ESHUTDOWN WSAESHUTDOWN
#endif /* ifndef ESHUTDOWN */
#ifndef ETOOMANYREFS
#define ETOOMANYREFS WSAETOOMANYREFS
#endif /* ifndef ETOOMANYREFS */
#ifndef ETIMEDOUT
#define ETIMEDOUT WSAETIMEDOUT
#endif /* ifndef ETIMEDOUT */
#ifndef ECONNREFUSED
#define ECONNREFUSED WSAECONNREFUSED
#endif /* ifndef ECONNREFUSED */
#ifndef ELOOP
#define ELOOP WSAELOOP
#endif /* ifndef ELOOP */
#ifndef EHOSTDOWN
#define EHOSTDOWN WSAEHOSTDOWN
#endif /* ifndef EHOSTDOWN */
#ifndef EHOSTUNREACH
#define EHOSTUNREACH WSAEHOSTUNREACH
#endif /* ifndef EHOSTUNREACH */
#ifndef EPROCLIM
#define EPROCLIM WSAEPROCLIM
#endif /* ifndef EPROCLIM */
#ifndef EUSERS
#define EUSERS WSAEUSERS
#endif /* ifndef EUSERS */
#ifndef EDQUOT
#define EDQUOT WSAEDQUOT
#endif /* ifndef EDQUOT */
#ifndef ESTALE
#define ESTALE WSAESTALE
#endif /* ifndef ESTALE */
#ifndef EREMOTE
#define EREMOTE WSAEREMOTE
#endif /* ifndef EREMOTE */

/***
 *** Functions.
 ***/

ISC_LANG_BEGINDECLS

isc_result_t
isc_net_probeipv4(void);
/*
 * Check if the system's kernel supports IPv4.
 *
 * Returns:
 *
 *	ISC_R_SUCCESS		IPv4 is supported.
 *	ISC_R_NOTFOUND		IPv4 is not supported.
 *	ISC_R_DISABLED		IPv4 is disabled.
 *	ISC_R_UNEXPECTED
 */

isc_result_t
isc_net_probeipv6(void);
/*
 * Check if the system's kernel supports IPv6.
 *
 * Returns:
 *
 *	ISC_R_SUCCESS		IPv6 is supported.
 *	ISC_R_NOTFOUND		IPv6 is not supported.
 *	ISC_R_DISABLED		IPv6 is disabled.
 *	ISC_R_UNEXPECTED
 */

isc_result_t
isc_net_probeunix(void);
/*
 * Check if UNIX domain sockets are supported.
 *
 * Returns:
 *
 *	ISC_R_SUCCESS
 *	ISC_R_NOTFOUND
 */

#define ISC_NET_DSCPRECVV4 0x01 /* Can receive sent DSCP value IPv4 */
#define ISC_NET_DSCPRECVV6 0x02 /* Can receive sent DSCP value IPv6 */
#define ISC_NET_DSCPSETV4  0x04 /* Can set DSCP on socket IPv4 */
#define ISC_NET_DSCPSETV6  0x08 /* Can set DSCP on socket IPv6 */
#define ISC_NET_DSCPPKTV4  0x10 /* Can set DSCP on per packet IPv4 */
#define ISC_NET_DSCPPKTV6  0x20 /* Can set DSCP on per packet IPv6 */
#define ISC_NET_DSCPALL	   0x3f /* All valid flags */

unsigned int
isc_net_probedscp(void);
/*%<
 * Probe the level of DSCP support.
 */

isc_result_t
isc_net_probe_ipv6only(void);
/*
 * Check if the system's kernel supports the IPV6_V6ONLY socket option.
 *
 * Returns:
 *
 *	ISC_R_SUCCESS		the option is supported for both TCP and UDP.
 *	ISC_R_NOTFOUND		IPv6 itself or the option is not supported.
 *	ISC_R_UNEXPECTED
 */

isc_result_t
isc_net_probe_ipv6pktinfo(void);
/*
 * Check if the system's kernel supports the IPV6_(RECV)PKTINFO socket option
 * for UDP sockets.
 *
 * Returns:
 *
 *	ISC_R_SUCCESS		the option is supported.
 *	ISC_R_NOTFOUND		IPv6 itself or the option is not supported.
 *	ISC_R_UNEXPECTED
 */

void
isc_net_disableipv4(void);

void
isc_net_disableipv6(void);

void
isc_net_enableipv4(void);

void
isc_net_enableipv6(void);

isc_result_t
isc_net_getudpportrange(int af, in_port_t *low, in_port_t *high);
/*%<
 * Returns system's default range of ephemeral UDP ports, if defined.
 * If the range is not available or unknown, ISC_NET_PORTRANGELOW and
 * ISC_NET_PORTRANGEHIGH will be returned.
 *
 * Requires:
 *
 *\li	'low' and 'high' must be non NULL.
 *
 * Returns:
 *
 *\li	*low and *high will be the ports specifying the low and high ends of
 *	the range.
 */

ISC_LANG_ENDDECLS

#endif /* ISC_NET_H */
