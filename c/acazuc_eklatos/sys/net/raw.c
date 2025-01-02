#include <net/ip6.h>
#include <net/ip4.h>
#include <net/net.h>
#include <net/arp.h>
#include <net/if.h>

#include <proc.h>
#include <sock.h>
#include <sma.h>
#include <std.h>
#include <cpu.h>

struct sock_raw_pkt
{
	size_t len;
	TAILQ_ENTRY(sock_raw_pkt) chain;
	uint8_t data[];
};

struct sock_raw
{
	struct sock *sock;
	TAILQ_HEAD(, sock_raw_pkt) packets;
	TAILQ_ENTRY(sock_raw) chain;
};

struct sock_raw_list
{
	struct spinlock lock; /* XXX rwlock */
	TAILQ_HEAD(, sock_raw) socks;
};

static struct sock_raw_list ip4_raw_socks;
static struct sock_raw_list ip6_raw_socks;
static struct sock_raw_list pkt_raw_socks;

static struct sma sock_raw_sma;

static void sock_list_init(struct sock_raw_list *list)
{
	spinlock_init(&list->lock);
	TAILQ_INIT(&list->socks);
}

void sock_raw_init(void)
{
	sma_init(&sock_raw_sma, sizeof(struct sock_raw), NULL, NULL, "sock_raw");
	sock_list_init(&ip4_raw_socks);
	sock_list_init(&ip6_raw_socks);
	sock_list_init(&pkt_raw_socks);
}

static struct sock_raw_list *get_sock_list(int domain)
{
	switch (domain)
	{
		case AF_INET:
			return &ip4_raw_socks;
		case AF_INET6:
			return &ip6_raw_socks;
		case AF_PACKET:
			return &pkt_raw_socks;
	}
	return NULL;
}

static int raw_pkt_queue(struct sock_raw *sock_raw, struct netpkt *pkt)
{
	struct sock_raw_pkt *raw_pkt;
	struct sock *sock = sock_raw->sock;
	int ret;

	switch (sock->domain)
	{
		case AF_INET:
		{
			struct ip *iphdr;
			if (pkt->len < sizeof(*iphdr))
				return -EINVAL;
			iphdr = pkt->data;
			if (sock->protocol && iphdr->ip_p != sock->protocol)
				return 0;
			break;
		}
		case AF_INET6:
		{
			struct ip6 *iphdr;
			if (pkt->len < sizeof(*iphdr))
				return -EINVAL;
			iphdr = pkt->data;
			if (sock->protocol && iphdr->ip6_nxt != sock->protocol)
				return 0;
			break;
		}
		case AF_PACKET:
		{
			struct ether_header *ether;
			if (pkt->len < sizeof(*ether))
				return -EINVAL;
			ether = pkt->data;
			if (sock->protocol && ether->ether_type != sock->protocol)
				return 0;
			break;
		}
		default:
			panic("unknown domain\n");
	}
	raw_pkt = malloc(sizeof(*raw_pkt) + pkt->len, 0);
	if (!raw_pkt)
	{
		ret = -ENOMEM;
		goto end;
	}
	raw_pkt->len = pkt->len;
	memcpy(raw_pkt->data, pkt->data, pkt->len);
	sock_lock(sock);
	TAILQ_INSERT_TAIL(&sock_raw->packets, raw_pkt, chain);
	waitq_signal(&sock->rwaitq, 0);
	poller_broadcast(&sock->poll_entries, POLLIN);
	sock_unlock(sock);

end:
	return ret;
}

void net_raw_queue(int domain, struct netpkt *pkt)
{
	struct sock_raw_list *list;
	struct sock_raw *sock_raw;

	list = get_sock_list(domain);
	if (!list)
		panic("unknown domain\n");
	spinlock_lock(&list->lock);
	TAILQ_FOREACH(sock_raw, &list->socks, chain)
		raw_pkt_queue(sock_raw, pkt);
	spinlock_unlock(&list->lock);
}

ssize_t raw4_send(struct sock *sock, struct msghdr *msg, int flags)
{
	struct uio uio;
	ssize_t ret;
	struct netif *netif = NULL;
	struct netpkt *pkt = NULL;
	struct netif_addr *netif_addr = NULL;
	struct arp_entry *arp_entry = NULL;
	struct ip *iphdr;
	struct in_addr dst_ip;

	(void)flags;
	sock_lock(sock);
	uio_from_msghdr(&uio, msg);
	if (uio.count < sizeof(struct ip))
	{
		ret = -EINVAL;
		goto end;
	}
	pkt = netpkt_alloc(uio.count);
	if (!pkt)
	{
		ret = -ENOMEM;
		goto end;
	}
	/* XXX check max length ? */
	ret = uio_copyout(pkt->data, &uio, uio.count);
	if (ret < 0)
		goto end;
	iphdr = pkt->data;
	iphdr->ip_len = htons(iphdr->ip_len);
	iphdr->ip_id = htons(iphdr->ip_id);
	iphdr->ip_off = htons(iphdr->ip_off);
	iphdr->ip_sum = htons(iphdr->ip_sum);
	dst_ip = iphdr->ip_dst;
	netif = ip4_get_dst_netif(&dst_ip, &netif_addr);
	if (!netif)
	{
		ret = -EINVAL; /* XXX */
		goto end;
	}
	iphdr->ip_src.s_addr = ((struct sockaddr_in*)&netif_addr->addr)->sin_addr.s_addr;
	iphdr->ip_dst.s_addr = iphdr->ip_dst.s_addr;
	if (!iphdr->ip_sum)
		iphdr->ip_sum = ip_checksum(iphdr, sizeof(*iphdr), 0);
#if 0
	printf("ip output:\n");
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
#endif
	arp_entry = arp_fetch(dst_ip);
	if (!arp_entry)
	{
		ret = -ENOMEM;
		goto end;
	}
	if (arp_entry->state != ARP_ENTRY_RESOLVED)
	{
		arp_resolve(arp_entry, netif, pkt);
		ret = uio.count;
		goto end;
	}
	/* XXX should be done outside sock lock */
	ret = ether_output(netif, pkt, &arp_entry->ether, ETHERTYPE_IP);
	if (ret)
		goto end;
	ret = uio.count;

end:
	arp_free(arp_entry);
	if (ret < 0)
		netpkt_free(pkt);
	netif_free(netif);
	sock_unlock(sock);
	return ret;
}

ssize_t raw6_send(struct sock *sock, struct msghdr *msg, int flags)
{
	(void)sock;
	(void)msg;
	(void)flags;
	/* XXX */
	return -EAFNOSUPPORT;
}

ssize_t rawp_send(struct sock *sock, struct msghdr *msg, int flags)
{
	struct uio uio;
	ssize_t ret;
	struct netif *netif = NULL;
	struct netpkt *pkt = NULL;
	struct ether_header *ether;

	(void)flags;
	sock_lock(sock);
	uio_from_msghdr(&uio, msg);
	if (uio.count < sizeof(struct ether_header))
	{
		ret = -EINVAL;
		goto end;
	}
	pkt = netpkt_alloc(uio.count);
	if (!pkt)
	{
		ret = -ENOMEM;
		goto end;
	}
	/* XXX check max length ? */
	ret = uio_copyout(pkt->data, &uio, uio.count);
	if (ret < 0)
		goto end;
	ether = pkt->data;
	netif = netif_from_ether((struct ether_addr*)&ether->ether_shost);
	if (!netif)
	{
		ret = -EINVAL;
		goto end;
	}
	ret = netif->op->emit(netif, pkt);
	if (ret)
		goto end;
	ret = uio.count;

end:
	if (ret < 0)
		netpkt_free(pkt);
	if (netif)
		netif_free(netif);
	sock_unlock(sock);
	return ret;
}

ssize_t raw_recv(struct sock *sock, struct msghdr *msg, int flags)
{
	struct sock_raw *sock_raw = sock->userdata;
	struct sock_raw_pkt *pkt;
	ssize_t ret;
	struct uio uio;

	sock_lock(sock);
	pkt = TAILQ_FIRST(&sock_raw->packets);
	if (!pkt)
	{
		if (flags & MSG_DONTWAIT)
		{
			ret = -EAGAIN;
			goto end;
		}
		do
		{
			ret = waitq_wait_tail_mutex(&sock->rwaitq,
			                            &sock->mutex,
			                            (sock->rcv_timeo.tv_sec
			                          || sock->rcv_timeo.tv_nsec)
			                           ? &sock->rcv_timeo : NULL);
			if (ret)
				goto end;
			pkt = TAILQ_FIRST(&sock_raw->packets);
			/* XXX on socket close, we should return 0 (can happen in multi-threaded proc)
			 * but there is a race on mutex because it will already be
			 * destroyed before the waitq return
			 */
		} while (!pkt);
	}
	TAILQ_REMOVE(&sock_raw->packets, pkt, chain);
	uio_from_msghdr(&uio, msg);
	ret = uio_copyin(&uio, pkt->data, pkt->len);
	free(pkt);

end:
	sock_unlock(sock);
	return ret;
}

int raw_poll(struct sock *sock, struct poll_entry *entry)
{
	struct sock_raw *sock_raw = sock->userdata;
	int ret = 0;

	sock_lock(sock);
	if (entry->events & POLLIN)
	{
		if (!TAILQ_EMPTY(&sock_raw->packets))
			ret |= POLLIN;
	}
	if (entry->events & POLLOUT)
		ret |= POLLOUT;
	if (ret)
		goto end;
	entry->file_head = &sock->poll_entries;
	ret = poller_add(entry);

end:
	sock_unlock(sock);
	return ret;
}

int raw_release(struct sock *sock)
{
	struct sock_raw *sock_raw = sock->userdata;
	struct sock_raw_list *list;
	struct sock_raw_pkt *pkt;

	list = get_sock_list(sock->domain);
	if (!list)
		panic("unknown domain\n");
	spinlock_lock(&list->lock);
	TAILQ_REMOVE(&list->socks, sock_raw, chain);
	spinlock_unlock(&list->lock);
	pkt = TAILQ_FIRST(&sock_raw->packets);
	while (pkt)
	{
		TAILQ_REMOVE(&sock_raw->packets, pkt, chain);
		free(pkt);
		pkt = TAILQ_FIRST(&sock_raw->packets);
	}
	sma_free(&sock_raw_sma, sock_raw);
	return 0;
}

int raw_setopt(struct sock *sock, int level, int opt, const void *uval,
               socklen_t len)
{
	int ret;

	sock_lock(sock);
	switch (level)
	{
		case SOL_SOCKET:
			ret = sock_sol_setopt(sock, level, opt, uval, len);
			break;
		case IPPROTO_IP:
			if (sock->domain == AF_INET)
				ret = ip4_setopt(sock, level, opt, uval, len);
			else
				ret = -EINVAL;
			break;
		case IPPROTO_IPV6:
			if (sock->domain == AF_INET6)
				ret = ip6_setopt(sock, level, opt, uval, len);
			else
				ret = -EINVAL;
			break;
		default:
			ret = -EINVAL;
			break;
	}
	sock_unlock(sock);
	return ret;
}

int raw_getopt(struct sock *sock, int level, int opt, void *uval,
               socklen_t *ulen)
{
	int ret;

	sock_lock(sock);
	switch (level)
	{
		case SOL_SOCKET:
			ret = sock_sol_getopt(sock, level, opt, uval, ulen);
			break;
		case IPPROTO_IP:
			if (sock->domain == AF_INET)
				ret = ip4_getopt(sock, level, opt, uval, ulen);
			else
				ret = -EINVAL;
			break;
		case IPPROTO_IPV6:
			if (sock->domain == AF_INET6)
				ret = ip6_getopt(sock, level, opt, uval, ulen);
			else
				ret = -EINVAL;
			break;
		default:
			ret = -EINVAL;
			break;
	}
	sock_unlock(sock);
	return ret;
}

int raw_ioctl(struct sock *sock, unsigned long request, uintptr_t data)
{
	return sock_sol_ioctl(sock, request, data);
}

int raw_shutdown(struct sock *sock, int how)
{
	(void)sock;
	(void)how;
	/* XXX */
	return 0;
}

static const struct sock_op raw4_op =
{
	.release = raw_release,
	.send = raw4_send,
	.recv = raw_recv,
	.poll = raw_poll,
	.setopt = raw_setopt,
	.getopt = raw_getopt,
	.ioctl = raw_ioctl,
	.shutdown = raw_shutdown,
};

static const struct sock_op raw6_op =
{
	.release = raw_release,
	.send = raw6_send,
	.recv = raw_recv,
	.poll = raw_poll,
	.setopt = raw_setopt,
	.getopt = raw_getopt,
	.ioctl = raw_ioctl,
	.shutdown = raw_shutdown,
};

static const struct sock_op rawp_op =
{
	.release = raw_release,
	.send = rawp_send,
	.recv = raw_recv,
	.poll = raw_poll,
	.setopt = raw_setopt,
	.getopt = raw_getopt,
	.ioctl = raw_ioctl,
	.shutdown = raw_shutdown,
};

int net_raw_open(int domain, int protocol, struct sock **sock)
{
	struct sock_raw_list *list;
	struct sock_raw *sock_raw;
	const struct sock_op *op;
	int ret;

	if (curcpu()->thread->proc->cred.euid)
		return -EACCES;
	sock_raw = sma_alloc(&sock_raw_sma, M_ZERO);
	if (!sock_raw)
		return -ENOMEM;
	TAILQ_INIT(&sock_raw->packets);
	switch (domain)
	{
		case AF_INET:
			op = &raw4_op;
			break;
		case AF_INET6:
			op = &raw6_op;
			break;
		case AF_PACKET:
			op = &rawp_op;
			break;
		default:
			panic("unknown domain\n");
	}
	ret = sock_new(domain, SOCK_RAW, protocol, op, sock);
	if (ret)
	{
		sma_free(&sock_raw_sma, sock_raw);
		return ret;
	}
	sock_raw->sock = *sock;
	(*sock)->userdata = sock_raw;
	list = get_sock_list(domain);
	if (!list)
		panic("invalid domain\n");
	spinlock_lock(&list->lock);
	TAILQ_INSERT_TAIL(&list->socks, sock_raw, chain);
	spinlock_unlock(&list->lock);
	return 0;
}
