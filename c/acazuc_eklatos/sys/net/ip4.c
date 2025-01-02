#define ENABLE_TRACE

#include <net/ip4.h>
#include <net/net.h>
#include <net/arp.h>
#include <net/if.h>

#include <errno.h>
#include <sock.h>
#include <std.h>

static struct in_addr ip4_gateway;
static uint16_t ip_id;
static const struct sock_op *ip4_protos[256];

static inline void print_iphdr(const struct ip *iphdr)
{
	printf("version: %" PRIu8 "\n", iphdr->ip_v);
	printf("ihl: %" PRIu8 "\n", iphdr->ip_hl);
	printf("tos: %" PRIu8 "\n", iphdr->ip_tos);
	printf("length: %" PRId16 "\n", ntohs(iphdr->ip_len));
	printf("ident: %" PRIu16 "\n", ntohs(iphdr->ip_id));
	printf("offset: %" PRId16 "\n", ntohs(iphdr->ip_off));
	printf("ttl: %" PRIu8 "\n", iphdr->ip_ttl);
	printf("proto: %" PRIu8 "\n", iphdr->ip_p);
	printf("checksum: %" PRIx16 "\n", ntohs(iphdr->ip_sum));
	printf("src: %u.%u.%u.%u\n", IN_ADDR_PRINTF(&iphdr->ip_src));
	printf("dst: %u.%u.%u.%u\n", IN_ADDR_PRINTF(&iphdr->ip_dst));
}

int ip4_input(struct netif *netif, struct netpkt *pkt)
{
	struct ip *iphdr = (struct ip*)pkt->data;
	uint16_t iplen;

	if (pkt->len < sizeof(*iphdr))
	{
		TRACE("ip4: packet too short (no iphdr)");
		return -EINVAL;
	}
#if 0
	printf("ip input:\n");
	print_iphdr(iphdr);
#endif
	net_raw_queue(AF_INET, pkt);
	/* XXX do some checks */
	if (iphdr->ip_off & IP_RF)
	{
		TRACE("ip4: unhandlded IP_RF");
		return -EINVAL;
	}
	if (iphdr->ip_off & IP_DF)
	{
		TRACE("ip4: unhandled IP_DF");
		return -EINVAL;
	}
	if (iphdr->ip_off & IP_MF)
	{
		TRACE("ip4: unhandled IP_MF");
		return -EINVAL;
	}
	if (iphdr->ip_off & IP_OFFMASK)
	{
		TRACE("ip4: unhandled fragmentation");
		return -EINVAL;
	}
	iplen = ntohs(iphdr->ip_len);
	if (pkt->len < iplen)
	{
		TRACE("ip4: packet too short (%" PRIu16 " < %" PRIu16 ")",
		     (uint16_t)pkt->len, iplen);
		return -EINVAL;
	}
	if (pkt->len > iplen)
	{
		int ret = netpkt_shrink_tail(pkt, pkt->len - iplen);
		if (ret)
			return ret;
	}
	netpkt_advance(pkt, sizeof(*iphdr));
	struct sockaddr_in src;
	struct sockaddr_in dst;
	src.sin_family = AF_INET;
	src.sin_port = 0;
	src.sin_addr = iphdr->ip_src;
	dst.sin_family = AF_INET;
	dst.sin_port = 0;
	dst.sin_addr = iphdr->ip_dst;
	if (!ip4_protos[iphdr->ip_p])
	{
#if 0
		TRACE("unknown ip proto: %" PRIu8, iphdr->ip_p);
#endif
		return 0;
	}
	ip4_protos[iphdr->ip_p]->input(netif, pkt,
	                                (struct sockaddr*)&src,
	                                (struct sockaddr*)&dst);
	return 0;
}

int ip4_output(struct sock *sock, struct netpkt *pkt, struct netif *netif,
               struct in_addr src, struct in_addr dst, uint16_t proto)
{
	struct arp_entry *arp_entry = NULL;
	struct netif_addr *netif_addr;
	struct ip *iphdr;
	int ret;

	(void)sock;
	iphdr = netpkt_grow_front(pkt, sizeof(struct ip));
	if (!iphdr)
	{
		ret = -ENOMEM;
		goto end;
	}
	iphdr->ip_v = 4;
	iphdr->ip_hl = 5;
	iphdr->ip_tos = 0;
	iphdr->ip_len = htons(pkt->len);
	iphdr->ip_id = __atomic_add_fetch(&ip_id, 1, __ATOMIC_SEQ_CST);
	iphdr->ip_off = 0;
	iphdr->ip_ttl = 64;
	iphdr->ip_p = proto;
	iphdr->ip_sum = 0;
	iphdr->ip_src = src;
	iphdr->ip_dst = dst;
	iphdr->ip_sum = ip_checksum(iphdr, sizeof(*iphdr), 0);
#if 0
	printf("ip output:\n");
	print_iphdr(iphdr);
#endif
	if (netif->flags & IFF_LOOPBACK)
	{
		ret = netif->op->emit(netif, pkt);
		goto end;
	}
	/* use arp of gateway if dst isn't in netif
	 * XXX it shouldn't be done that way I guess
	 */
	TAILQ_FOREACH(netif_addr, &netif->addrs, chain)
	{
		if (netif_addr->addr.sa_family != AF_INET)
			continue;
		struct sockaddr_in *sin_addr = (struct sockaddr_in*)&netif_addr->addr;
		struct sockaddr_in *sin_mask = (struct sockaddr_in*)&netif_addr->mask;
		if ((sin_addr->sin_addr.s_addr & sin_mask->sin_addr.s_addr)
		 == (dst.s_addr & sin_mask->sin_addr.s_addr))
			break;
	}
	arp_entry = arp_fetch(netif_addr ? dst : ip4_gateway);
	if (!arp_entry)
	{
		ret = -ENOMEM;
		goto end;
	}
	if (arp_entry->state != ARP_ENTRY_RESOLVED)
	{
		arp_resolve(arp_entry, netif, pkt);
		ret = 0;
		goto end;
	}
	ret = ether_output(netif, pkt, &arp_entry->ether, ETHERTYPE_IP);

end:
	arp_free(arp_entry);
	return ret;
}

int ip4_setopt(struct sock *sock, int level, int opt, const void *uval,
               socklen_t len)
{
	(void)sock;
	(void)uval;
	(void)len;
	if (level != IPPROTO_IP)
		return -EINVAL;
	switch (opt)
	{
		case IP_HDRINCL:
			/* XXX */
			return 0;
	}
	return -EINVAL;
}

int ip4_getopt(struct sock *sock, int level, int opt, void *uval,
               socklen_t *ulen)
{
	(void)sock;
	(void)uval;
	(void)ulen;
	if (level != IPPROTO_IP)
		return -EINVAL;
	switch (opt)
	{
		case IP_HDRINCL:
			/* XXX */
			return 0;
	}
	return -EINVAL;
}

uint16_t ip_checksum(const void *data, size_t size, uint32_t init)
{
	uint32_t result;
	uint16_t *tmp;

	tmp = (uint16_t*)data;
	result = init;
	while (size > 1)
	{
		result += *(tmp++);
		size -= 2;
	}
	if (size)
		result += *((uint8_t*)tmp);
	while (result > 0xFFFF)
		result = ((result >> 16) & 0xFFFF) + (result & 0xFFFF);
	return (~((uint16_t)result));
}

struct netif *ip4_get_dst_netif(struct in_addr *dst,
                                struct netif_addr **netif_addr)
{
	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_addr = *dst;
	struct netif *netif = netif_from_net((struct sockaddr*)&sin, netif_addr);
	if (netif)
		return netif;
	sin.sin_addr = ip4_gateway;
	netif = netif_from_net((struct sockaddr*)&sin, netif_addr);
	if (!netif)
		return NULL;
	*dst = ip4_gateway;
	return netif;
}

static int open_proto(int domain, int type, int protocol, struct sock **sock)
{
	if (protocol < 0
	 || (unsigned)protocol >= sizeof(ip4_protos) / sizeof(*ip4_protos))
		return -EINVAL;
	if (!ip4_protos[protocol])
		return -EINVAL;
	return ip4_protos[protocol]->open(domain, type, protocol, sock);
}

int ip4_open(int domain, int type, int protocol, struct sock **sock)
{
	if (domain != AF_INET)
		return -EINVAL;
	switch (type)
	{
		case SOCK_RAW:
			return net_raw_open(domain, protocol, sock);
		case SOCK_DGRAM:
			if (!protocol)
				protocol = IPPROTO_UDP;
			return open_proto(domain, type, protocol, sock);
		case SOCK_STREAM:
			if (!protocol)
				protocol = IPPROTO_TCP;
			return open_proto(domain, type, protocol, sock);
		default:
			return -EINVAL;
	}
}

struct in_addr ip4_get_gateway(void)
{
	return ip4_gateway;
}

void ip4_set_gateway(const struct in_addr addr)
{
	ip4_gateway = addr;
}

int ip4_register_proto(uint8_t proto, const struct sock_op *op)
{
	if (ip4_protos[proto])
		return -EALREADY;
	ip4_protos[proto] = op;
	return 0;
}
