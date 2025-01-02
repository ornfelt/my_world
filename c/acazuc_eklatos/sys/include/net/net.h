#ifndef NET_NET_H
#define NET_NET_H

#include <queue.h>
#include <types.h>

struct netif;

struct sockaddr
{
	sa_family_t sa_family;
	char sa_data[14];
};

struct netpkt
{
	void *alloc;
	void *data;
	size_t len;
	TAILQ_ENTRY(netpkt) chain; /* used for arp-resolve queue
	                            * XXX should be handled another way
	                            */
};

int ether_input(struct netif *netif, struct netpkt *pkt);
int ether_output(struct netif *netif, struct netpkt *pkt,
                 const struct ether_addr *dst, uint16_t ether_type);

static inline uint16_t ntohs(uint16_t v)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	return ((v & 0x00FF) << 8)
	     | ((v & 0xFF00) >> 8);
#else
	return v;
#endif
}

static inline uint32_t ntohl(uint32_t v)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	return ((v & 0x000000FF) << 24)
	     | ((v & 0x0000FF00) <<  8)
	     | ((v & 0x00FF0000) >>  8)
	     | ((v & 0xFF000000) >> 24);
#else
	return v;
#endif
}

static inline uint16_t htons(uint16_t v)
{
	return ntohs(v);
}

static inline uint32_t htonl(uint32_t v)
{
	return ntohl(v);
}

struct netpkt *netpkt_alloc(size_t bytes);
void netpkt_free(struct netpkt *pkt);
void netpkt_advance(struct netpkt *pkt, size_t bytes);
void *netpkt_grow_front(struct netpkt *pkt, size_t bytes);
int netpkt_shrink_tail(struct netpkt *pkt, size_t bytes);

int net_raw_open(int domain, int protocol, struct sock **sock);
void net_raw_queue(int domain, struct netpkt *pkt);

#endif
