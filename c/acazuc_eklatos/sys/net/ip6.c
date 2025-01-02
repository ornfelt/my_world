#include <net/ip6.h>
#include <net/net.h>
#include <net/if.h>

#include <errno.h>
#include <sock.h>

static struct in6_addr ip6_gateway;
static const struct sock_op *ip6_protos[256];

int ip6_input(struct netif *netif, struct netpkt *pkt)
{
	struct ip6 *ip6hdr = (struct ip6*)pkt->data;
	if (pkt->len < sizeof(*ip6hdr))
		return -EINVAL;
	net_raw_queue(AF_INET6, pkt);
	/* XXX */
	(void)netif;
	return -EAFNOSUPPORT;
}

int ip6_output(struct sock *sock, struct netpkt *pkt, struct netif *netif,
               struct in6_addr *src, struct in6_addr *dst, uint16_t proto)
{
	(void)sock;
	(void)pkt;
	(void)netif;
	(void)src;
	(void)dst;
	(void)proto;
	/* XXX */
	return -EAFNOSUPPORT;
}

int ip6_setopt(struct sock *sock, int level, int opt, const void *uval,
               socklen_t len)
{
	(void)sock;
	(void)uval;
	(void)len;
	if (level != IPPROTO_IPV6)
		return -EINVAL;
	switch (opt)
	{
		case IPV6_HDRINCL:
			/* XXX */
			return 0;
	}
	return -EINVAL;
}

int ip6_getopt(struct sock *sock, int level, int opt, void *uval,
               socklen_t *ulen)
{
	(void)sock;
	(void)uval;
	(void)ulen;
	if (level != IPPROTO_IPV6)
		return -EINVAL;
	switch (opt)
	{
		case IPV6_HDRINCL:
			/* XXX */
			return 0;
	}
	return -EINVAL;
}

static int open_proto(int domain, int type, int protocol, struct sock **sock)
{
	if (protocol < 0
	 || (unsigned)protocol >= sizeof(ip6_protos) / sizeof(*ip6_protos))
		return -EINVAL;
	if (!ip6_protos[protocol])
		return -EINVAL;
	return ip6_protos[protocol]->open(domain, type, protocol, sock);
}

int ip6_open(int domain, int type, int protocol, struct sock **sock)
{
	if (domain != AF_INET6)
		return -EINVAL;
	switch (type)
	{
		case SOCK_RAW:
			return net_raw_open(AF_INET6, protocol, sock);
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

struct in6_addr ip6_get_gateway(void)
{
	return ip6_gateway;
}

void ip6_set_gateway(const struct in6_addr addr)
{
	ip6_gateway = addr;
}

int ip6_register_proto(uint8_t proto, const struct sock_op *op)
{
	if (ip6_protos[proto])
		return -EALREADY;
	ip6_protos[proto] = op;
	return 0;
}
