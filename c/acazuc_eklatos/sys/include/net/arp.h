#ifndef NET_ARP_H
#define NET_ARP_H

#include <net/ether.h>
#include <net/ip4.h>

#include <types.h>

#define ARPOP_REQUEST 0x1
#define ARPOP_REPLY   0x2

struct arphdr
{
	uint16_t ar_hrd;
	uint16_t ar_pro;
	uint8_t ar_hln;
	uint8_t ar_pln;
	uint16_t ar_op;
};

enum arp_entry_state
{
	ARP_ENTRY_UNKNOWN,
	ARP_ENTRY_RESOLVING,
	ARP_ENTRY_RESOLVED,
};

struct arp_entry
{
	enum arp_entry_state state;
	struct ether_addr ether;
	struct in_addr in;
	refcount_t refcount;
	struct mutex mutex;
	TAILQ_HEAD(, netpkt) packets;
	TAILQ_ENTRY(arp_entry) chain;
};

int arp_input(struct netif *netif, struct netpkt *pkt);
struct arp_entry *arp_fetch(struct in_addr addr);
void arp_free(struct arp_entry *entry);
void arp_resolve(struct arp_entry *entry, struct netif *netif,
                 struct netpkt *pkt);

#endif
