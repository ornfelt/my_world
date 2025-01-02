#include <net/arp.h>
#include <net/ip4.h>
#include <net/net.h>
#include <net/if.h>

#include <endian.h>
#include <errno.h>
#include <sock.h>
#include <sma.h>
#include <std.h>

struct arp_request
{
	uint8_t sha[6];
	uint8_t spa[4];
	uint8_t tha[6];
	uint8_t tpa[4];
};

static struct spinlock arp_entries_lock = SPINLOCK_INITIALIZER(); /* XXX rwlock */
static TAILQ_HEAD(arp_entries_head, arp_entry) arp_entries = TAILQ_HEAD_INITIALIZER(arp_entries);

static const struct ether_addr ether_broadcast = {{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}};
static const struct ether_addr ether_any = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};

#define ARP_IS_ANY(arp) (!(arp)[0] && !(arp)[1] && !(arp)[2] \
                      && !(arp)[3] && !(arp)[4] && !(arp)[5])

#define ARP_IS_BROADCAST(arp) ((arp)[0] == 0xFF && (arp)[1] == 0xFF \
                            && (arp)[2] == 0xFF && (arp)[3] == 0xFF \
                            && (arp)[4] == 0xFF && (arp)[5] == 0xFF)

#define ARP_CMP(a1, a2) ((a1)[0] != (a2)[0] \
                      || (a1)[1] != (a2)[1] \
                      || (a1)[2] != (a2)[2] \
                      || (a1)[3] != (a2)[3] \
                      || (a1)[4] != (a2)[4] \
                      || (a1)[5] != (a2)[5])

static struct sma arp_entry_sma;

void arp_entry_init(void)
{
	sma_init(&arp_entry_sma, sizeof(struct arp_entry), NULL, NULL, "arp_entry");
}

static struct arp_entry *arp_alloc(uint8_t *arp, struct in_addr in)
{
	struct arp_entry *entry = sma_alloc(&arp_entry_sma, 0);
	if (!entry)
		return NULL;
	if (arp)
	{
		entry->state = ARP_ENTRY_RESOLVED;
		memcpy(entry->ether.addr, arp, ETHER_ADDR_LEN);
	}
	else
	{
		entry->state = ARP_ENTRY_UNKNOWN;
		memset(entry->ether.addr, 0, ETHER_ADDR_LEN);
	}
	entry->in.s_addr = in.s_addr;
	mutex_init(&entry->mutex, 0);
	refcount_init(&entry->refcount, 1);
	TAILQ_INIT(&entry->packets);
	return entry;
}

static int register_arp(uint8_t *arp, struct in_addr in)
{
	struct arp_entry *entry = arp_alloc(arp, in);
	if (!entry)
		return -ENOMEM;
	spinlock_lock(&arp_entries_lock);
	TAILQ_INSERT_TAIL(&arp_entries, entry, chain);
	spinlock_unlock(&arp_entries_lock);
	return 0;
}

static struct arp_entry *arp_entry_find(struct in_addr in)
{
	struct arp_entry *entry;
	spinlock_lock(&arp_entries_lock);
	TAILQ_FOREACH(entry, &arp_entries, chain)
	{
		if (entry->in.s_addr == in.s_addr)
			break;
	}
	if (entry)
		refcount_inc(&entry->refcount);
	spinlock_unlock(&arp_entries_lock);
	return entry;
}

void arp_free(struct arp_entry *entry)
{
	if (!entry)
		return;
	if (refcount_dec(&entry->refcount))
		return;
	spinlock_lock(&arp_entries_lock);
	if (refcount_get(&entry->refcount))
	{
		spinlock_unlock(&arp_entries_lock);
		return;
	}
	TAILQ_REMOVE(&arp_entries, entry, chain);
	spinlock_unlock(&arp_entries_lock);
	struct netpkt *pkt, *nxt;
	TAILQ_FOREACH_SAFE(pkt, &entry->packets, chain, nxt)
		netpkt_free(pkt);
	mutex_destroy(&entry->mutex);
	sma_free(&arp_entry_sma, entry);
}

struct arp_entry *arp_fetch(struct in_addr addr)
{
	spinlock_lock(&arp_entries_lock);
	struct arp_entry *entry;
	TAILQ_FOREACH(entry, &arp_entries, chain)
	{
		if (entry->in.s_addr != addr.s_addr)
			continue;
		refcount_inc(&entry->refcount);
		spinlock_unlock(&arp_entries_lock);
		return entry;
	}
	entry = arp_alloc(NULL, addr);
	if (!entry)
		return NULL;
	TAILQ_INSERT_TAIL(&arp_entries, entry, chain);
	refcount_inc(&entry->refcount); /* XXX should GC this on time basis */
	spinlock_unlock(&arp_entries_lock);
	return entry;
}

static int arp_request(struct arp_entry *entry, struct netif *netif)
{
	struct netpkt *pkt = netpkt_alloc(sizeof(struct ether_header)
	                                + sizeof(struct arphdr)
	                                + sizeof(struct arp_request));
	if (!pkt)
		return -ENOMEM;
	netpkt_advance(pkt, sizeof(struct ether_header));
	struct arphdr *arphdr = pkt->data;
	arphdr->ar_hrd = htons(1);
	arphdr->ar_pro = htons(ETHERTYPE_IP);
	arphdr->ar_hln = 6;
	arphdr->ar_pln = 4;
	arphdr->ar_op = htons(ARPOP_REQUEST);
	struct arp_request *req = (struct arp_request*)&arphdr[1];
	memcpy(req->sha, netif->ether.addr, ETHER_ADDR_LEN);
	struct sockaddr_in *src = (struct sockaddr_in*)&TAILQ_FIRST(&netif->addrs)->addr; /* XXX get better IP */
	memcpy(req->tha, ether_any.addr, ETHER_ADDR_LEN);
	be32enc(req->spa, ntohl(src->sin_addr.s_addr));
	be32enc(req->tpa, ntohl(entry->in.s_addr));
#if 0
	printf("generate arp request: from " MAC_FMT " / %u.%u.%u.%u to " MAC_FMT " / %u.%u.%u.%u\n",
	       MAC_PRINTF(req->sha),
	       req->spa[0], req->spa[1], req->spa[2], req->spa[3],
	       MAC_PRINTF(req->tha),
	       req->tpa[0], req->tpa[1], req->tpa[2], req->tpa[3]);
#endif
	int ret = ether_output(netif, pkt, &ether_broadcast, ETHERTYPE_ARP);
	if (ret)
		netpkt_free(pkt);
	return ret;
}

void arp_resolve(struct arp_entry *entry, struct netif *netif,
                 struct netpkt *pkt)
{
	mutex_lock(&entry->mutex);
	if (pkt)
		TAILQ_INSERT_TAIL(&entry->packets, pkt, chain);
	if (entry->state == ARP_ENTRY_UNKNOWN)
		entry->state = ARP_ENTRY_RESOLVING;
	mutex_unlock(&entry->mutex);
	int ret = arp_request(entry, netif);
	if (ret)
		printf("failed to request arp\n");
}

static int arp_reply(struct netif *netif, const struct arp_request *req,
                     struct ether_addr *addr)
{
	struct netpkt *pkt = netpkt_alloc(sizeof(struct ether_header) + sizeof(struct arphdr) + sizeof(struct arp_request));
	if (!pkt)
		return -ENOMEM;
	netpkt_advance(pkt, sizeof(struct ether_header)); /* we know ether will probably be needed */
	struct arphdr *hdr = pkt->data;
	hdr->ar_hrd = ntohs(1);
	hdr->ar_pro = ntohs(ETHERTYPE_IP);
	hdr->ar_hln = 6;
	hdr->ar_pln = 4;
	hdr->ar_op = ntohs(ARPOP_REPLY);
	struct arp_request *reply = (struct arp_request*)&hdr[1];
	memcpy(reply->sha, addr, 6);
	memcpy(reply->spa, req->tpa, 4);
	memcpy(reply->tha, req->sha, 6);
	memcpy(reply->tpa, req->spa, 4);
#if 0
	printf("generate arp reply: from " MAC_FMT " / %u.%u.%u.%u to " MAC_FMT " / %u.%u.%u.%u\n",
	       MAC_PRINTF(reply->sha),
	       reply->spa[0], reply->spa[1], reply->spa[2], reply->spa[3],
	       MAC_PRINTF(reply->tha),
	       reply->tpa[0], reply->tpa[1], reply->tpa[2], reply->tpa[3]);
#endif
	int ret = ether_output(netif, pkt, (struct ether_addr*)&reply->tha[0],
	                       ETHERTYPE_ARP);
	if (ret)
		netpkt_free(pkt);
	return ret;
}

static int handle_request(struct netif *netif, struct netpkt *pkt)
{
	struct arphdr *arphdr = (struct arphdr*)pkt->data;
	struct arp_request *req = (void*)&arphdr[1];
	if (pkt->len < sizeof(*arphdr) + sizeof(*req))
	{
		printf("invalid arp request length\n");
		return -EINVAL;
	}
#if 0
	printf("handle arp request: from " MAC_FMT " / %u.%u.%u.%u to " MAC_FMT " / %u.%u.%u.%u\n",
	       MAC_PRINTF(req->sha),
	       req->spa[0], req->spa[1], req->spa[2], req->spa[3],
	       MAC_PRINTF(req->tha),
	       req->tpa[0], req->tpa[1], req->tpa[2], req->tpa[3]);
#endif
	if (ARP_IS_ANY(req->sha))
	{
		printf("arp request from any\n");
		return -EINVAL;
	}
	if (ARP_IS_BROADCAST(req->sha))
	{
		printf("arp request from broadcast\n");
		return -EINVAL;
	}
	struct in_addr spa;
	spa.s_addr = htonl(be32dec(req->spa));
	struct arp_entry *entry = arp_entry_find(spa);
	if (entry)
	{
		switch (entry->state)
		{
			case ARP_ENTRY_RESOLVED:
				if (!ARP_CMP(entry->ether.addr, req->sha))
					break;
#if 1
				printf("ARP spoofing of %u.%u.%u.%u (expected " MAC_FMT ", got " MAC_FMT ")\n",
				       req->spa[0], req->spa[1], req->spa[2], req->spa[3],
				       MAC_PRINTF(entry->ether.addr),
				       MAC_PRINTF(req->sha));
#endif
				arp_free(entry);
				return -EINVAL;
			case ARP_ENTRY_UNKNOWN:
			case ARP_ENTRY_RESOLVING:
				memcpy(entry->ether.addr, req->sha, ETHER_ADDR_LEN);
				break;
		}
		arp_free(entry);
	}
	else
	{
		/* XXX should we really register for every arp messages types ?
		 * (probably not, it should be done only on arp reply)
		 */
		int ret = register_arp(req->sha, spa);
		if (ret)
			return ret;
	}
	struct in_addr tpa;
	tpa.s_addr = htonl(be32dec(req->tpa));
	if (ARP_IS_ANY(req->tha))
	{
		struct sockaddr_in sin;
		sin.sin_family = AF_INET;
		sin.sin_port = 0;
		sin.sin_addr = tpa;
		struct netif *dstif = netif_from_addr((struct sockaddr*)&sin,
		                                      NULL);
		if (!dstif)
			return -EINVAL;
		if (dstif->flags & IFF_LOOPBACK) /* XXX I guess ? */
		{
			netif_free(dstif);
			return -EINVAL;
		}
		arp_reply(netif, req, &dstif->ether);
		netif_free(dstif);
		return 0;
	}
	return 0;
}

static int handle_reply(struct netif *netif, struct netpkt *pkt)
{
	struct arphdr *arphdr = (struct arphdr*)pkt->data;
	struct arp_request *reply = (void*)&arphdr[1];
	if (pkt->len < sizeof(*arphdr) + sizeof(*reply))
	{
		printf("invalid arp reply length\n");
		return -EINVAL;
	}
#if 0
	printf("handle arp reply: from " MAC_FMT " / %u.%u.%u.%u to " MAC_FMT " / %u.%u.%u.%u\n",
	       MAC_PRINTF(reply->sha),
	       reply->spa[0], reply->spa[1], reply->spa[2], reply->spa[3],
	       MAC_PRINTF(reply->tha),
	       reply->tpa[0], reply->tpa[1], reply->tpa[2], reply->tpa[3]);
#endif
	if (ARP_IS_ANY(reply->sha))
	{
		printf("arp reply from any\n");
		return -EINVAL;
	}
	if (ARP_IS_BROADCAST(reply->sha))
	{
		printf("arp reply from broadcast\n");
		return -EINVAL;
	}
	struct in_addr spa;
	spa.s_addr = htonl(be32dec(reply->spa));
	struct arp_entry *entry = arp_entry_find(spa);
	if (!entry)
	{
		printf("gratuitous ARP\n");
		return -EINVAL;
	}
	mutex_lock(&entry->mutex);
	entry->state = ARP_ENTRY_RESOLVED;
	memcpy(entry->ether.addr, reply->sha, ETHER_ADDR_LEN);
	while ((pkt = TAILQ_FIRST(&entry->packets)))
	{
		TAILQ_REMOVE(&entry->packets, pkt, chain);
		int ret = ether_output(netif, pkt, &entry->ether, ETHERTYPE_IP);
		if (ret)
			netpkt_free(pkt);
	}
	mutex_unlock(&entry->mutex);
	arp_free(entry);
	return 0;
}

int arp_input(struct netif *netif, struct netpkt *pkt)
{
	if (pkt->len < sizeof(struct arphdr))
	{
		printf("invalid arp length\n");
		return -EINVAL;
	}
	struct arphdr *arphdr = (struct arphdr*)pkt->data;
	if (arphdr->ar_hrd != ntohs(1))
	{
		printf("unhandled arp hrd: 0x%04" PRIx16 "\n", arphdr->ar_hrd);
		return -EINVAL;
	}
	if (arphdr->ar_pro != ntohs(ETHERTYPE_IP))
	{
		printf("unhandled arp pro: 0x%04" PRIx16 "\n", arphdr->ar_pro);
		return -EINVAL;
	}
	if (arphdr->ar_hln != 6)
	{
		printf("invalid arp hln: %" PRIu8 "\n", arphdr->ar_hln);
		return -EINVAL;
	}
	if (arphdr->ar_pln != 4)
	{
		printf("invalid arp pln: %" PRIu8 "\n", arphdr->ar_pln);
		return -EINVAL;
	}
	int ret;
	switch (ntohs(arphdr->ar_op))
	{
		case ARPOP_REQUEST:
			ret = handle_request(netif, pkt);
			break;
		case ARPOP_REPLY:
			ret = handle_reply(netif, pkt);
			break;
		default:
			printf("invalid arp op: 0x%02" PRIx8 "\n", arphdr->ar_op);
			ret = -EINVAL;
			break;
	}
	return ret;
}
