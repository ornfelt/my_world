#include <net/ip4.h>
#include <net/if.h>

#include <sock.h>
#include <std.h>

static int loopback_emit(struct netif *netif, struct netpkt *pkt)
{
	/* XXX hum.... */
	int ret = ip4_input(netif, pkt);
	if (!ret)
		netpkt_free(pkt);
	return ret;
}

static const struct netif_op loopback_nop =
{
	.emit = loopback_emit,
};

void net_loopback_init(void)
{
	struct netif *netif;
	int ret = netif_alloc("lo", &loopback_nop, &netif);
	if (ret)
		panic("failed to create loopback\n");
	netif->flags = IFF_UP | IFF_LOOPBACK;
	struct netif_addr *addr = netif_addr_alloc();
	if (!addr)
		panic("loopback: netif addr allocation failed\n");
	struct sockaddr_in *sin_addr = (struct sockaddr_in*)&addr->addr;
	struct sockaddr_in *sin_mask = (struct sockaddr_in*)&addr->mask;
	sin_addr->sin_family = AF_INET;
	sin_addr->sin_port = 0;
	sin_addr->sin_addr.s_addr = 0x0100007F;
	sin_mask->sin_family = AF_INET;
	sin_mask->sin_port = 0;
	sin_mask->sin_addr.s_addr = 0x000000FF;
	TAILQ_INSERT_TAIL(&netif->addrs, addr, chain);
}
