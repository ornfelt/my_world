#ifndef NET_IF_H
#define NET_IF_H

#include <net/ether.h>
#include <net/net.h>

#include <refcount.h>

#define IFNAMSIZ 16

struct netif;

struct netif_op
{
	int (*emit)(struct netif *netif, struct netpkt *pkt);
	void (*update_rx_filter)(struct netif *netif);
};

struct ifreq
{
	char ifr_name[IFNAMSIZ];
	union
	{
		struct sockaddr ifr_addr;
		struct sockaddr ifr_netmask;
		struct sockaddr ifr_hwaddr;
		uint16_t ifr_flags;
	};
};

struct ifconf
{
	int ifc_len;
	union
	{
		char *ifc_buf;
		struct ifreq *ifc_req;
	};
};

struct netif_stats
{
	uint64_t rx_packets;
	uint64_t rx_bytes;
	uint64_t rx_errors;
	uint64_t tx_packets;
	uint64_t tx_bytes;
	uint64_t tx_errors;
};

struct netif_addr
{
	struct sockaddr addr;
	struct sockaddr mask;
	TAILQ_ENTRY(netif_addr) chain;
};

TAILQ_HEAD(netif_addr_head, netif_addr);

#define IFF_LOOPBACK  (1 << 0)
#define IFF_UP        (1 << 1)
#define IFF_BROADCAST (1 << 2)
#define IFF_PROMISC   (1 << 3)
#define IFF_MULTICAST (1 << 4)
#define IFF_ALLMULTI  (1 << 5)
#define IFF_RUNNING   (1 << 6)

struct netif
{
	const struct netif_op *op;
	uint16_t flags;
	char name[IFNAMSIZ];
	struct ether_addr ether;
	struct netif_addr_head addrs;
	struct netif_stats stats;
	struct node *sysfs_node;
	void *userdata;
	refcount_t refcount;
	TAILQ_ENTRY(netif) chain;
};

TAILQ_HEAD(netif_head, netif);

int netif_alloc(const char *name, const struct netif_op *op,
                struct netif **netifp);
void netif_free(struct netif *netif);
void netif_ref(struct netif *netif);
size_t netif_count(void);
int netif_fill_ifconf(struct ifconf *ifconf);
struct netif *netif_from_name(const char *name);
struct netif *netif_from_addr(const struct sockaddr *addr,
                              struct netif_addr **netif_addr);
struct netif *netif_from_net(const struct sockaddr *addr,
                             struct netif_addr **netif_addr);
struct netif *netif_from_ether(const struct ether_addr *addr);

struct netif_addr *netif_addr_alloc(void);
void netif_addr_free(struct netif_addr *netif_addr);

void net_loopback_init(void);

#endif
