#ifndef NETINET_IN_H
#define NETINET_IN_H

#include <sys/socket.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IPPROTO_IP   0
#define IPPROTO_ICMP 1
#define IPPROTO_TCP  6
#define IPPROTO_UDP  17
#define IPPROTO_IPV6 41

#define IP_HDRINCL 1

#define INET_ADDRSTRLEN 16

#define INADDR_ANY       0x00000000UL
#define INADDR_BROADCAST 0xFFFFFFFFUL

#define IP_RF (1 << 15)
#define IP_DF (1 << 14)
#define IP_MF (1 << 13)
#define IP_OFFMASK (0x1FFF)

struct in_addr
{
	uint32_t s_addr;
};

struct ip
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	uint8_t ip_hl : 4;
	uint8_t ip_v : 4;
#else
	uint8_t ip_v : 4;
	uint8_t ip_hl : 4;
#endif
	uint8_t ip_tos;
	int16_t ip_len;
	uint16_t ip_id;
	int16_t ip_off;
	uint8_t ip_ttl;
	uint8_t ip_p;
	uint16_t ip_sum;
	struct in_addr ip_src;
	struct in_addr ip_dst;
};

struct sockaddr_in
{
	sa_family_t sin_family;
	uint16_t sin_port;
	struct in_addr sin_addr;
	uint8_t sin_zero[8];
};

struct in6_addr
{
	uint8_t s6_addr[16];
};

struct sockaddr_in6
{
	sa_family_t sin6_family;
	uint16_t sin6_port;
	uint32_t sin6_flowinfo;
	struct in6_addr sin6_addr;
	uint32_t sin6_scope_id;
};

#ifdef __cplusplus
}
#endif

#endif
