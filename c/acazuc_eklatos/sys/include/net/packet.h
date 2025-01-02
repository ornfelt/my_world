#ifndef NET_PACKET_H
#define NET_PACKET_H

#include <types.h>

struct netpkt;
struct sock;

struct sockaddr_ll
{
	sa_family_t sll_family;
	uint16_t sll_protocol;
	int32_t sll_ifindex;
	uint16_t sll_hatype;
	uint8_t sll_pkttype;
	uint8_t sll_halen;
	uint8_t sll_addr[8];
};

int pfp_raw_open(int protocol, struct sock **sock);

void pfp_raw_queue(struct netpkt *pkt);

#endif
