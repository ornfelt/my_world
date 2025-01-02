#define ENABLE_TRACE

#include "udp.h"

#include <net/ip6.h>
#include <net/ip4.h>
#include <net/net.h>
#include <net/if.h>

#include <proc.h>
#include <sock.h>
#include <kmod.h>
#include <sma.h>
#include <std.h>
#include <mem.h>

struct sock_udp_pkt
{
	size_t len;
	TAILQ_ENTRY(sock_udp_pkt) chain;
	union sockaddr_union addr;
	socklen_t addrlen;
	uint8_t data[];
};

struct sock_udp
{
	struct sock *sock;
	sa_family_t family;
	TAILQ_HEAD(, sock_udp_pkt) packets;
	TAILQ_ENTRY(sock_udp) chain;
};

struct udp4_pseudohdr
{
	struct in_addr src;
	struct in_addr dst;
	uint8_t zero;
	uint8_t proto;
	uint16_t len;
} __attribute__ ((packed));

struct udp6_pseudohdr
{
	struct in6_addr src;
	struct in6_addr dst;
	uint32_t len;
	uint8_t zero[3];
	uint8_t proto;
} __attribute__ ((packed));

struct sock_udp_list
{
	struct spinlock lock; /* XXX rwlock */
	TAILQ_HEAD(, sock_udp) socks;
};

static struct sock_udp_list ip4_udp_socks;
static struct sock_udp_list ip6_udp_socks;

static struct sma sock_udp_sma;

static uint16_t ephemeral_start = 49152;
static uint16_t ephemeral_end = 65535;
static uint16_t ephemeral_cur = 0;

static uint16_t udp_checksum(const struct netpkt *pkt,
                             const struct sockaddr *src,
                             const struct sockaddr *dst);
static int find_ephemeral_port(struct sock *sock);
static int has_matching_sock(struct sock_udp_list *list, struct sockaddr *addr);

static void sock_list_init(struct sock_udp_list *list)
{
	spinlock_init(&list->lock);
	TAILQ_INIT(&list->socks);
}

static struct sock_udp_list *get_sock_list(int domain)
{
	switch (domain)
	{
		case AF_INET:
			return &ip4_udp_socks;
		case AF_INET6:
			return &ip6_udp_socks;
	}
	return NULL;
}

static int udp4_get_send_addresses(struct sock *sock, struct msghdr *msg,
                                   struct sockaddr_in *src,
                                   struct sockaddr_in *dst,
                                   struct netif **netif)
{
	int ret;
	if (msg->msg_name)
	{
		if (sock->dst_addrlen)
			return -EISCONN;
		if (msg->msg_namelen != sizeof(struct sockaddr_in))
			return -EAFNOSUPPORT;
		ret = vm_copyin(curcpu()->thread->proc->vm_space, dst,
		                msg->msg_name, sizeof(*dst));
		if (ret)
			return ret;
		if (dst->sin_family != AF_INET)
			return -EAFNOSUPPORT;
	}
	else if (sock->dst_addrlen)
	{
		if (sock->dst_addrlen != sizeof(struct sockaddr_in)
		 || sock->dst_addr.su_family != AF_INET)
			return -EAFNOSUPPORT;
		memcpy(dst, &sock->dst_addr, sizeof(*dst));
	}
	else
	{
		return -EDESTADDRREQ;
	}
	if (sock->src_addrlen)
	{
		if (sock->src_addrlen != sizeof(struct sockaddr_in)
		 || sock->src_addr.su_family != AF_INET)
			return -EAFNOSUPPORT;
		if (((struct sockaddr_in*)&sock->src_addr)->sin_addr.s_addr == INADDR_ANY)
		{
			struct netif_addr *addr;
			*netif = netif_from_net((struct sockaddr*)dst, &addr);
			if (!*netif)
				return -EADDRNOTAVAIL;
			src->sin_family = AF_INET;
			src->sin_port = ((struct sockaddr_in*)&sock->src_addr)->sin_port;
			src->sin_addr = ((struct sockaddr_in*)&addr->addr)->sin_addr;
		}
		else
		{
			memcpy(src, &sock->src_addr, sizeof(*src));
			*netif = netif_from_addr((struct sockaddr*)&src, NULL);
			if (!*netif)
				return -EADDRNOTAVAIL;
		}
	}
	else
	{
		struct netif_addr *netif_addr;
		sock->src_addr.sin.sin_family = AF_INET;
		sock->src_addr.sin.sin_addr = dst->sin_addr;
		*netif = ip4_get_dst_netif(&sock->src_addr.sin.sin_addr, &netif_addr);
		if (!*netif)
			return -EADDRNOTAVAIL;
		sock->src_addr.sin.sin_addr = ((struct sockaddr_in*)&netif_addr->addr)->sin_addr;
		sock->src_addr.sin.sin_family = AF_INET;
		ret = find_ephemeral_port(sock);
		if (ret)
			return ret;
		*src = sock->src_addr.sin;
	}
	return 0;
}

static int udp6_get_send_addresses(struct sock *sock, struct msghdr *msg,
                                   struct sockaddr_in6 *src,
                                   struct sockaddr_in6 *dst,
                                   struct netif **netif)
{
	(void)sock;
	(void)msg;
	(void)src;
	(void)dst;
	(void)netif;
	/* XXX */
	return -EAFNOSUPPORT;
}

ssize_t udp_send(struct sock *sock, struct msghdr *msg, int flags)
{
	struct netpkt *pkt = NULL;
	union sockaddr_union src;
	union sockaddr_union dst;
	struct udphdr *udphdr;
	struct netif *netif = NULL;
	struct uio uio;
	size_t bytes;
	size_t pre_alloc;
	ssize_t ret;

	(void)flags;
	sock_lock(sock);
	uio_from_msghdr(&uio, msg);
	/* XXX check max length ? */
	bytes = uio.count;
	switch (sock->domain)
	{
		case AF_INET:
			ret = udp4_get_send_addresses(sock, msg, &src.sin, &dst.sin, &netif);
			if (ret)
				goto end;
			pre_alloc = sizeof(struct ip);
			break;
		case AF_INET6:
			ret = udp6_get_send_addresses(sock, msg, &src.sin6, &dst.sin6, &netif);
			if (ret)
				goto end;
			pre_alloc = sizeof(struct ip6);
			break;
		default:
			ret = -EAFNOSUPPORT;
			goto end;
	}
	pkt = netpkt_alloc(uio.count + pre_alloc + sizeof(struct udphdr));
	if (!pkt)
	{
		ret = -ENOMEM;
		goto end;
	}
	netpkt_advance(pkt, pre_alloc + sizeof(struct udphdr));
	ret = uio_copyout(pkt->data, &uio, uio.count);
	if (ret < 0)
		goto end;
	udphdr = netpkt_grow_front(pkt, sizeof(struct udphdr));
	if (!udphdr)
	{
		ret = -ENOMEM;
		goto end;
	}
	switch (sock->domain)
	{
		case AF_INET:
			udphdr->uh_sport = src.sin.sin_port;
			udphdr->uh_dport = dst.sin.sin_port;
			break;
		case AF_INET6:
			udphdr->uh_sport = src.sin6.sin6_port;
			udphdr->uh_dport = dst.sin6.sin6_port;
			break;
		default:
			ret = -EAFNOSUPPORT;
			goto end;
	}
	udphdr->uh_ulen = ntohs(pkt->len);
	udphdr->uh_sum = 0;
	udphdr->uh_sum = udp_checksum(pkt, &src.sa, &dst.sa);
	/* XXX this switch should be done outside of the udp lock */
	switch (sock->domain)
	{
		case AF_INET:
			ret = ip4_output(sock, pkt, netif, src.sin.sin_addr,
			                 dst.sin.sin_addr, IPPROTO_UDP);
			break;
		case AF_INET6:
			ret = ip6_output(sock, pkt, netif, &src.sin6.sin6_addr,
			                 &dst.sin6.sin6_addr, IPPROTO_UDP);
			break;
		default:
			ret = -EAFNOSUPPORT;
			break;
	}
	if (ret)
		goto end;
	ret = bytes;

end:
	if (ret < 0)
		netpkt_free(pkt);
	netif_free(netif);
	sock_unlock(sock);
	return ret;
}

ssize_t udp_recv(struct sock *sock, struct msghdr *msg, int flags)
{
	struct sock_udp *sock_udp = sock->userdata;
	struct sock_udp_pkt *pkt;
	struct uio uio;
	ssize_t ret;

	sock_lock(sock);
	if (!sock->src_addrlen)
	{
		/* XXX is it really the case ? */
		ret = -ENOTCONN; /* XXX */
		goto end;
	}
	pkt = TAILQ_FIRST(&sock_udp->packets);
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
			pkt = TAILQ_FIRST(&sock_udp->packets);
			/* XXX on socket close, we should return 0 (can happen in multi-threaded proc)
			 * but there is a race on mutex because it will already be
			 * destroyed before the waitq return
			 */
		} while (!pkt);
	}
	if (msg->msg_name)
	{
		ret = vm_copyout(curcpu()->thread->proc->vm_space,
		                 msg->msg_name, &pkt->addr,
		                 msg->msg_namelen < pkt->addrlen
		               ? msg->msg_namelen
		               : pkt->addrlen);
		if (ret)
			goto end;
		msg->msg_namelen = pkt->addrlen;
	}
	TAILQ_REMOVE(&sock_udp->packets, pkt, chain);
	uio_from_msghdr(&uio, msg);
	ret = uio_copyin(&uio, pkt->data, pkt->len);
	free(pkt);

end:
	sock_unlock(sock);
	return ret;
}

int udp_poll(struct sock *sock, struct poll_entry *entry)
{
	struct sock_udp *sock_udp = sock->userdata;
	int ret = 0;

	sock_lock(sock);
	if (entry->events & POLLIN)
	{
		if (!TAILQ_EMPTY(&sock_udp->packets))
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

int udp_connect(struct sock *sock, const struct sockaddr *addr,
                socklen_t addrlen)
{
	int ret;

	(void)addrlen;
	sock_lock(sock);
	switch (sock->domain)
	{
		case AF_INET:
		{
			const struct sockaddr_in *sin = (const struct sockaddr_in*)addr;
			if (!sin->sin_port)
			{
				ret = -EINVAL;
				goto end;
			}
			/* XXX more ip check */
			sock->dst_addrlen = sizeof(struct sockaddr_in);
			sock->dst_addr.sin = *sin;
			ret = 0;
			break;
		}
		case AF_INET6:
		{
			const struct sockaddr_in6 *sin6 = (const struct sockaddr_in6*)addr;
			if (!sin6->sin6_port)
			{
				ret = -EINVAL;
				goto end;
			}
			/* XXX more ip check */
			sock->dst_addrlen = sizeof(struct sockaddr_in6);
			sock->dst_addr.sin6 = *sin6;
			ret = 0;
			break;
		}
		default:
			ret = -EAFNOSUPPORT;
			break;
	}

end:
	sock_unlock(sock);
	return ret;
}

int udp_bind(struct sock *sock, const struct sockaddr *addr,
             socklen_t addrlen)
{
	struct sock_udp_list *list;
	uint16_t port;
	int ret;

	sock_lock(sock);
	switch (sock->domain)
	{
		case AF_INET:
		{
			const struct sockaddr_in *sin = (const struct sockaddr_in*)addr;
			if (addrlen < sizeof(*sin))
			{
				ret = -EINVAL;
				goto end;
			}
			addrlen = sizeof(*sin);
			/* XXX more ip check */
			sock->src_addr.sin = *sin;
			port = sin->sin_port;
			break;
		}
		case AF_INET6:
		{
			const struct sockaddr_in6 *sin6 = (const struct sockaddr_in6*)addr;
			if (addrlen < sizeof(*sin6))
			{
				ret = -EINVAL;
				goto end;
			}
			addrlen = sizeof(*sin6);
			/* XXX more ip check */
			sock->src_addr.sin6 = *sin6;
			port = sin6->sin6_port;
			break;
		}
		default:
			ret = -EAFNOSUPPORT;
			goto end;
	}
	if (!port)
	{
		ret = find_ephemeral_port(sock);
		if (ret)
			goto end;
	}
	else
	{
		list = get_sock_list(sock->domain);
		if (!list)
			panic("unknown domain\n");
		if (has_matching_sock(list, &sock->src_addr.sa))
		{
			ret = -EADDRINUSE;
			goto end;
		}
	}
	sock->src_addrlen = addrlen;
	ret = 0;

end:
	sock_unlock(sock);
	return ret;
}

int udp_release(struct sock *sock)
{
	struct sock_udp *sock_udp = sock->userdata;
	struct sock_udp_list *list;
	struct sock_udp_pkt *pkt;

	list = get_sock_list(sock->domain);
	if (!list)
		panic("unknown domain\n");
	spinlock_lock(&list->lock);
	TAILQ_REMOVE(&list->socks, sock_udp, chain);
	spinlock_unlock(&list->lock);
	pkt = TAILQ_FIRST(&sock_udp->packets);
	while (pkt)
	{
		TAILQ_REMOVE(&sock_udp->packets, pkt, chain);
		free(pkt);
		pkt = TAILQ_FIRST(&sock_udp->packets);
	}
	sma_free(&sock_udp_sma, sock_udp);
	return 0;
}

int udp_setopt(struct sock *sock, int level, int opt, const void *uval,
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

int udp_getopt(struct sock *sock, int level, int opt, void *uval,
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

int udp_ioctl(struct sock *sock, unsigned long request, uintptr_t data)
{
	return sock_sol_ioctl(sock, request, data);
}

int udp_shutdown(struct sock *sock, int how)
{
	(void)sock;
	(void)how;
	/* XXX */
	return 0;
}

int udp_open(int domain, int type, int protocol, struct sock **sock);
int udp_input(struct netif *netif, struct netpkt *pkt,
              struct sockaddr *src, struct sockaddr *dst);

static const struct sock_op udp_op =
{
	.open = udp_open,
	.release = udp_release,
	.send = udp_send,
	.recv = udp_recv,
	.poll = udp_poll,
	.connect = udp_connect,
	.bind = udp_bind,
	.setopt = udp_setopt,
	.getopt = udp_getopt,
	.ioctl = udp_ioctl,
	.shutdown = udp_shutdown,
	.input = udp_input,
};

int udp_open(int domain, int type, int protocol, struct sock **sock)
{
	struct sock_udp_list *list;
	struct sock_udp *sock_udp;
	int ret;

	if (type != SOCK_DGRAM
	 || protocol != IPPROTO_UDP)
		return -EINVAL;
	sock_udp = sma_alloc(&sock_udp_sma, M_ZERO);
	if (!sock_udp)
		return -ENOMEM;
	TAILQ_INIT(&sock_udp->packets);
	ret = sock_new(domain, type, protocol, &udp_op, sock);
	if (ret)
	{
		sma_free(&sock_udp_sma, sock_udp);
		return ret;
	}
	sock_udp->sock = *sock;
	(*sock)->userdata = sock_udp;
	list = get_sock_list(domain);
	if (!list)
		panic("unknown domain\n");
	spinlock_lock(&list->lock);
	TAILQ_INSERT_TAIL(&list->socks, sock_udp, chain);
	spinlock_unlock(&list->lock);
	return 0;
}

static uint16_t udp4_checksum(const struct netpkt *pkt,
                              const struct in_addr src,
                              const struct in_addr dst)
{
	struct udp4_pseudohdr phdr;
	uint32_t result;

	phdr.src = src;
	phdr.dst = dst;
	phdr.zero = 0;
	phdr.proto = IPPROTO_UDP;
	phdr.len = ntohs(pkt->len);

	result  = ((uint16_t*)&phdr)[0];
	result += ((uint16_t*)&phdr)[1];
	result += ((uint16_t*)&phdr)[2];
	result += ((uint16_t*)&phdr)[3];
	result += ((uint16_t*)&phdr)[4];
	result += ((uint16_t*)&phdr)[5];

	return ip_checksum(pkt->data, pkt->len, result);
}

static uint16_t udp6_checksum(const struct netpkt *pkt,
                              const struct in6_addr *src,
                              const struct in6_addr *dst)
{
	struct udp6_pseudohdr phdr;
	uint32_t result;

	phdr.src = *src;
	phdr.dst = *dst;
	phdr.len = ntohl(pkt->len);
	phdr.zero[0] = 0;
	phdr.zero[1] = 0;
	phdr.zero[2] = 0;
	phdr.proto = IPPROTO_UDP;

	result = 0;
	for (size_t i = 0; i < sizeof(phdr) / 2; ++i)
		result += ((uint16_t*)&phdr)[i];

	return ip_checksum(pkt->data, pkt->len, result);
}

static uint16_t udp_checksum(const struct netpkt *netpkt,
                             const struct sockaddr *src,
                             const struct sockaddr *dst)
{
	switch (src->sa_family)
	{
		case AF_INET:
			return udp4_checksum(netpkt,
			                     ((struct sockaddr_in*)src)->sin_addr,
			                     ((struct sockaddr_in*)dst)->sin_addr);
		case AF_INET6:
			return udp6_checksum(netpkt,
			                     &((struct sockaddr_in6*)src)->sin6_addr,
			                     &((struct sockaddr_in6*)dst)->sin6_addr);
		default:
			panic("unknown family\n");
	}
}

static int udp_pkt_queue(struct sock_udp *sock_udp, struct netpkt *pkt,
                         struct sockaddr *src)
{
	struct udphdr *udphdr;
	struct sock *sock;
	struct sock_udp_pkt *udp_pkt;
	int ret = 0;

	udphdr = pkt->data;
	sock = sock_udp->sock;
	udp_pkt = malloc(sizeof(*udp_pkt) + pkt->len, 0);
	if (!udp_pkt)
	{
		ret = -ENOMEM;
		goto end;
	}
	udp_pkt->len = pkt->len;
	switch (src->sa_family)
	{
		case AF_INET:
		{
			struct sockaddr_in *udp_sin = (struct sockaddr_in*)&udp_pkt->addr;
			udp_pkt->addrlen = sizeof(struct sockaddr_in);
			udp_sin->sin_family = AF_INET;
			udp_sin->sin_port = udphdr->uh_sport;
			udp_sin->sin_addr = ((struct sockaddr_in*)src)->sin_addr;
			break;
		}
		case AF_INET6:
		{
			struct sockaddr_in6 *udp_sin6 = (struct sockaddr_in6*)&udp_pkt->addr;
			udp_pkt->addrlen = sizeof(struct sockaddr_in6);
			udp_sin6->sin6_family = AF_INET6;
			udp_sin6->sin6_port = udphdr->uh_sport;
			udp_sin6->sin6_addr = ((struct sockaddr_in6*)src)->sin6_addr;
			break;
		}
		default:
			free(udp_pkt);
			return -EAFNOSUPPORT;
	}
	netpkt_advance(pkt, sizeof(*udphdr));
	memcpy(udp_pkt->data, pkt->data, pkt->len);
	sock_lock(sock); /* XXX sleepable lock on interrupt is NOT a good idea */
	TAILQ_INSERT_TAIL(&sock_udp->packets, udp_pkt, chain);
	waitq_signal(&sock->rwaitq, 0);
	poller_broadcast(&sock->poll_entries, POLLIN);
	sock_unlock(sock);

end:
	return ret;
}

static void udp4_find_input_sockets(struct sock_udp **both_match,
                                    struct sock_udp **dst_match,
                                    const struct sockaddr *src,
                                    const struct sockaddr *dst,
                                    const struct udphdr *udphdr)
{
	struct sock_udp_list *list;
	struct sock_udp *sock;

	list = &ip4_udp_socks;
	spinlock_lock(&list->lock);
	TAILQ_FOREACH(sock, &list->socks, chain)
	{
		struct sockaddr_in *src_sin = &sock->sock->src_addr.sin;
		struct sockaddr_in *dst_sin = &sock->sock->dst_addr.sin;
		if (!sock->sock->src_addrlen
		 || src_sin->sin_family != AF_INET
		 || (src_sin->sin_addr.s_addr != INADDR_ANY
		  && src_sin->sin_addr.s_addr != ((struct sockaddr_in*)dst)->sin_addr.s_addr)
		 || src_sin->sin_port != udphdr->uh_dport)
			continue;
		if (!sock->sock->dst_addrlen
		 || (dst_sin->sin_family == AF_INET
		  && dst_sin->sin_addr.s_addr == ((struct sockaddr_in*)src)->sin_addr.s_addr
		  && dst_sin->sin_port == udphdr->uh_sport))
		{
			*both_match = sock;
			sock_ref(sock->sock);
			break;
		}
		if (!*dst_match)
		{
			*dst_match = sock;
			sock_ref(sock->sock);
		}
	}
	spinlock_unlock(&list->lock);
}

static void udp6_find_input_sockets(struct sock_udp **both_match,
                                    struct sock_udp **dst_match,
                                    const struct sockaddr *src,
                                    const struct sockaddr *dst,
                                    const struct udphdr *udphdr)
{
	static const struct in6_addr in6_any = IN6ADDR_ANY_INIT;
	struct sock_udp_list *list;
	struct sock_udp *sock;

	list = &ip6_udp_socks;
	spinlock_lock(&list->lock);
	TAILQ_FOREACH(sock, &list->socks, chain)
	{
		struct sockaddr_in6 *src_sin6 = &sock->sock->src_addr.sin6;
		struct sockaddr_in6 *dst_sin6 = &sock->sock->dst_addr.sin6;
		if (!sock->sock->src_addrlen
		 || src_sin6->sin6_family != AF_INET6
		 || (memcmp(&src_sin6->sin6_addr, &in6_any, sizeof(src_sin6->sin6_addr))
		  && memcmp(&src_sin6->sin6_addr, &((struct sockaddr_in6*)dst)->sin6_addr, sizeof(src_sin6->sin6_addr)))
		 || src_sin6->sin6_port != udphdr->uh_dport)
			continue;
		if (!sock->sock->dst_addrlen
		 || (dst_sin6->sin6_family == AF_INET6
		  && !memcmp(&dst_sin6->sin6_addr, &((struct sockaddr_in6*)src)->sin6_addr, sizeof(dst_sin6->sin6_addr))
		  && dst_sin6->sin6_port == udphdr->uh_sport))
		{
			*both_match = sock;
			sock_ref(sock->sock);
			break;
		}
		if (!*dst_match)
		{
			*dst_match = sock;
			sock_ref(sock->sock);
		}
	}
	spinlock_unlock(&list->lock);
}

static int find_input_socket(const struct sockaddr *src,
                             const struct sockaddr *dst,
                             const struct udphdr *udphdr,
                             struct sock_udp **sock_udp)
{
	struct sock_udp *both_match = NULL;
	struct sock_udp *dst_match = NULL;

	switch (src->sa_family)
	{
		case AF_INET:
			udp4_find_input_sockets(&both_match, &dst_match,
			                        src, dst, udphdr);
			break;
		case AF_INET6:
			udp6_find_input_sockets(&both_match, &dst_match,
			                        src, dst, udphdr);
			break;
		default:
			return -EAFNOSUPPORT;
	}
	if (both_match)
	{
		*sock_udp = both_match;
		if (dst_match)
			sock_free(dst_match->sock);
	}
	else if (dst_match)
	{
		*sock_udp = dst_match;
	}
	else
	{
		*sock_udp = NULL;
	}
	return 0;
}

int udp_input(struct netif *netif, struct netpkt *pkt,
              struct sockaddr *src, struct sockaddr *dst)
{
	struct sock_udp *sock_udp;
	struct udphdr *udphdr;
	uint16_t chk_cksum;
	uint16_t udplen;
	uint16_t cksum;
	int ret;

	(void)netif;
	if (pkt->len < sizeof(*udphdr))
	{
		TRACE("udp: packet too short (no udphdr)");
		return -EINVAL;
	}
	udphdr = pkt->data;
	udplen = ntohs(udphdr->uh_ulen);
	if (pkt->len < udplen)
	{
		TRACE("udp: packet too short (%" PRIu16 " < %" PRIu16 ")",
		      (uint16_t)pkt->len, udplen);
		return -EINVAL;
	}
	if (pkt->len > udplen)
	{
		ret = netpkt_shrink_tail(pkt, pkt->len - udplen);
		if (ret)
			return ret;
	}
	cksum = udphdr->uh_sum;
	udphdr->uh_sum = 0;
	chk_cksum = udp_checksum(pkt, src, dst);
	if (cksum != chk_cksum)
	{
		TRACE("udp: invalid checksum: got %04" PRIx16
		      ", expected %04" PRIx16,
		      cksum, chk_cksum);
		return -EINVAL;
	}
	ret = find_input_socket(src, dst, udphdr, &sock_udp);
	if (ret)
		return ret;
	if (!sock_udp)
		return 0;
	ret = udp_pkt_queue(sock_udp, pkt, src);
	sock_free(sock_udp->sock);
	return ret;
}

static int has_matching_sock_locked(struct sock_udp_list *list,
                                    struct sockaddr *addr)
{
	struct sock_udp *sock;

	switch (addr->sa_family)
	{
		case AF_INET:
			TAILQ_FOREACH(sock, &list->socks, chain)
			{
				struct sockaddr_in *sin = (struct sockaddr_in*)addr;
				struct sockaddr_in *src_sin = &sock->sock->src_addr.sin;
				if (sock->sock->src_addrlen
				 && src_sin->sin_family == AF_INET
				 && (src_sin->sin_addr.s_addr == INADDR_ANY
				  || src_sin->sin_addr.s_addr == sin->sin_addr.s_addr)
				 && src_sin->sin_port == sin->sin_port)
					break;
			}
			break;
		case AF_INET6:
		{
			static const struct in6_addr in6_any = IN6ADDR_ANY_INIT;
			TAILQ_FOREACH(sock, &list->socks, chain)
			{
				struct sockaddr_in6 *sin6 = (struct sockaddr_in6*)addr;
				struct sockaddr_in6 *src_sin6 = &sock->sock->src_addr.sin6;
				if (sock->sock->src_addrlen
				 && src_sin6->sin6_family == AF_INET6
				 && (!memcmp(&src_sin6->sin6_addr, &in6_any, sizeof(sin6->sin6_addr))
				  || !memcmp(&src_sin6->sin6_addr, &sin6->sin6_addr, sizeof(sin6->sin6_addr)))
				 && src_sin6->sin6_port == sin6->sin6_port)
					break;
			}
			break;
		}
		default:
			panic("unknown domain\n");
	}
	if (sock)
		return 1;
	return 0;
}

static int has_matching_sock(struct sock_udp_list *list, struct sockaddr *addr)
{
	spinlock_lock(&list->lock);
	int ret = has_matching_sock_locked(list, addr);
	spinlock_unlock(&list->lock);
	return ret;
}

static int find_ephemeral_port(struct sock *sock)
{
	struct sock_udp_list *list;
	uint16_t ephemeral_count = ephemeral_end - ephemeral_start;
	uint16_t *port_ptr;

	switch (sock->domain)
	{
		case AF_INET:
			port_ptr = &sock->src_addr.sin.sin_port;
			break;
		case AF_INET6:
			port_ptr = &sock->src_addr.sin6.sin6_port;
			break;
		default:
			return -EAFNOSUPPORT;
	}
	list = get_sock_list(sock->domain);
	if (!list)
		panic("unknown domain\n");
	for (size_t i = 0; i < ephemeral_count; ++i)
	{
		*port_ptr = htons(ephemeral_start + ephemeral_cur);
		spinlock_lock(&list->lock);
		int ret = has_matching_sock_locked(list, &sock->src_addr.sa);
		ephemeral_cur = ephemeral_cur + 1;
		ephemeral_cur %= ephemeral_count;
		if (!ret)
		{
			switch (sock->domain)
			{
				case AF_INET:
					sock->src_addrlen = sizeof(struct sockaddr_in);
					break;
				case AF_INET6:
					sock->src_addrlen = sizeof(struct sockaddr_in6);
					break;
				default:
					panic("unknown domain\n");
			}
			spinlock_unlock(&list->lock);
			return 0;
		}
		spinlock_unlock(&list->lock);
	}
	return -EADDRINUSE;
}

int init(void)
{
	int ret;

	sma_init(&sock_udp_sma, sizeof(struct sock_udp), NULL, NULL, "sock_udp");
	sock_list_init(&ip4_udp_socks);
	sock_list_init(&ip6_udp_socks);
	ret = ip4_register_proto(IPPROTO_UDP, &udp_op);
	if (ret)
		TRACE("net_udp: failed to register ip4 proto");
	ret = ip6_register_proto(IPPROTO_UDP, &udp_op);
	if (ret)
		TRACE("net_udp: faield to register ip6 proto");
	return 0;
}

void fini(void)
{
}

struct kmod_info kmod =
{
	.magic = KMOD_MAGIC,
	.version = 1,
	.name = "net/udp",
	.init = init,
	.fini = fini,
};
