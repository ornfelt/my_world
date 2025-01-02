#ifndef NET_IP4_H
#define NET_IP4_H

#include <net/ether.h>

#include <refcount.h>
#include <mutex.h>
#include <queue.h>
#include <types.h>

#define IPPROTO_IP   0
#define IPPROTO_ICMP 1
#define IPPROTO_TCP  6
#define IPPROTO_UDP  17
#define IPPROTO_IPV6 41

#define IP_HDRINCL 1

#define ICMP_ECHOREPLY     0
#define ICMP_DEST_UNREACH  3
#define ICMP_ECHO          8
#define ICMP_TIME_EXCEEDED 11
#define ICMP_PARAMPROB     12

#define INADDR_ANY       0x00000000UL
#define INADDR_BROADCAST 0xFFFFFFFFUL

struct netif_addr;
struct sock_op;
struct netpkt;
struct netif;
struct sock;

struct in_addr
{
	uint32_t s_addr;
};

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__

#define IN_ADDR_PRINTF(ina) (uint8_t)(((ina)->s_addr >>  0) & 0xFF), \
                            (uint8_t)(((ina)->s_addr >>  8) & 0xFF), \
                            (uint8_t)(((ina)->s_addr >> 16) & 0xFF), \
                            (uint8_t)(((ina)->s_addr >> 24) & 0xFF)

#else

#define IN_ADDR_PRINTF(ina) (uint8_t)(((ina)->s_addr >> 24) & 0xFF), \
                            (uint8_t)(((ina)->s_addr >> 16) & 0xFF), \
                            (uint8_t)(((ina)->s_addr >>  8) & 0xFF), \
                            (uint8_t)(((ina)->s_addr >>  0) & 0xFF)

#endif

#define IP_RF (1 << 15)
#define IP_DF (1 << 14)
#define IP_MF (1 << 13)
#define IP_OFFMASK (0x1FFF)

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
	uint16_t ip_len;
	uint16_t ip_id;
	uint16_t ip_off;
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

uint16_t ip_checksum(const void *data, size_t size, uint32_t init);
int ip4_input(struct netif *netif, struct netpkt *pkt);
int ip4_output(struct sock *sock, struct netpkt *pkt, struct netif *netif,
               struct in_addr src, struct in_addr dst, uint16_t proto);

int ip4_setopt(struct sock *sock, int level, int opt, const void *uval,
               socklen_t len);
int ip4_getopt(struct sock *sock, int level, int opt, void *uval,
               socklen_t *ulen);

struct netif *ip4_get_dst_netif(struct in_addr *dst,
                                struct netif_addr **netif_addr);

int ip4_open(int domain, int type, int protocol, struct sock **sock);

struct in_addr ip4_get_gateway(void);
void ip4_set_gateway(const struct in_addr addr);
int ip4_register_proto(uint8_t proto, const struct sock_op *op);

#endif
