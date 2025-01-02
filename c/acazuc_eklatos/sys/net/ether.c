#include <net/packet.h>
#include <net/ip4.h>
#include <net/net.h>
#include <net/arp.h>
#include <net/if.h>

#include <errno.h>
#include <sock.h>
#include <std.h>

int ether_output(struct netif *netif, struct netpkt *pkt,
                 const struct ether_addr *dst, uint16_t ether_type)
{
	struct ether_header *hdr = netpkt_grow_front(pkt, sizeof(*hdr));
	if (!hdr)
		return -ENOMEM;
	memcpy(hdr->ether_dhost, dst->addr, ETHER_ADDR_LEN);
	memcpy(hdr->ether_shost, netif->ether.addr, ETHER_ADDR_LEN);
	hdr->ether_type = htons(ether_type);
#if 0
	printf("ether output %lu bytes: " MAC_FMT " - " MAC_FMT " - %" PRIu16 "\n",
	       pkt->len,
	       MAC_PRINTF(hdr->ether_shost),
	       MAC_PRINTF(hdr->ether_dhost),
	       ntohs(hdr->ether_type));
#endif
	int ret = netif->op->emit(netif, pkt);
	if (!ret)
		netpkt_free(pkt);
	return ret;
}

int ether_input(struct netif *netif, struct netpkt *pkt)
{
	if (pkt->len < sizeof(struct ether_header))
		return -EINVAL;
	const struct ether_header *hdr = (struct ether_header*)pkt->data;
#if 0
	printf("ether input %lu bytes: " MAC_FMT " - " MAC_FMT " - %" PRIu16 "\n",
	       pkt->len,
	       MAC_PRINTF(hdr->ether_shost),
	       MAC_PRINTF(hdr->ether_dhost),
	       ntohs(hdr->ether_type));
#endif
	net_raw_queue(AF_PACKET, pkt);
	/* XXX CRC ? */
	int ret;
	switch (ntohs(hdr->ether_type))
	{
		case ETHERTYPE_IP:
			/* XXX verify dst mac is netif mac */
			/* XXX verify mac -> ip from arp entries */
			netpkt_advance(pkt, sizeof(*hdr));
			ret = ip4_input(netif, pkt);
			break;
		case ETHERTYPE_ARP:
			netpkt_advance(pkt, sizeof(*hdr));
			ret = arp_input(netif, pkt);
			break;
		case ETHERTYPE_IP6:
			netpkt_advance(pkt, sizeof(*hdr));
			ret = ip6_input(netif, pkt);
			break;
		default:
			printf("unknown ether type: 0x%x\n", hdr->ether_type);
			ret = -EINVAL;
			break;
	}
	return ret;
}
