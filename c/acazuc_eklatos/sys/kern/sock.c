#include <net/packet.h>
#include <net/local.h>
#include <net/ip6.h>
#include <net/ip4.h>
#include <net/if.h>

#include <errno.h>
#include <ioctl.h>
#include <file.h>
#include <sock.h>
#include <proc.h>
#include <cpu.h>
#include <vfs.h>
#include <sma.h>
#include <std.h>
#include <mem.h>

static struct sma sock_sma;

void sock_init(void)
{
	sma_init(&sock_sma, sizeof(struct sock), NULL, NULL, "sock");
}

static struct sock *getsock(struct file *file)
{
	return file->sock;
}

static ssize_t sock_read(struct file *file, struct uio *uio)
{
	struct sock *sock = getsock(file);
	if (!sock)
		return -EINVAL;
	struct msghdr msg;
	msg.msg_name = NULL;
	msg.msg_namelen = 0;
	msg.msg_iov = uio->iov;
	msg.msg_iovlen = uio->iovcnt;
	msg.msg_control = NULL;
	msg.msg_controllen = 0;
	msg.msg_flags = 0;
	return sock_recv(sock, &msg, 0);
}

static ssize_t sock_write(struct file *file, struct uio *uio)
{
	struct sock *sock = getsock(file);
	if (!sock)
		return -EINVAL;
	struct msghdr msg;
	msg.msg_name = NULL;
	msg.msg_namelen = 0;
	msg.msg_iov = uio->iov;
	msg.msg_iovlen = uio->iovcnt;
	msg.msg_control = NULL;
	msg.msg_controllen = 0;
	msg.msg_flags = 0;
	return sock_send(sock, &msg, 0);
}

static off_t sock_seek(struct file *file, off_t off, int whence)
{
	(void)file;
	(void)off;
	(void)whence;
	return -ESPIPE;
}

static int sock_ioctl(struct file *file, unsigned long request, uintptr_t data)
{
	struct sock *sock = getsock(file);
	if (!sock)
		return -EINVAL; /* XXX */
	if (!sock->op || !sock->op->ioctl)
		return -EOPNOTSUPP;
	return sock->op->ioctl(sock, request, data);
}

static int sock_poll(struct file *file, struct poll_entry *entry)
{
	struct sock *sock = getsock(file);
	if (!sock)
		return -EINVAL; /* XXX */
	if (!sock->op || !sock->op->poll)
		return -EOPNOTSUPP; /* XXX */
	return sock->op->poll(sock, entry);
}

const struct file_op g_sock_fop =
{
	.read = sock_read,
	.write = sock_write,
	.seek = sock_seek,
	.ioctl = sock_ioctl,
	.poll = sock_poll,
};

int sock_open(int domain, int type, int protocol, struct sock **sock)
{
	switch (domain)
	{
		case PF_LOCAL:
			return pfl_open(domain, type, protocol, sock);
		case PF_INET:
			return ip4_open(domain, type, protocol, sock);
		case PF_PACKET:
			if (curcpu()->thread->proc->cred.euid)
				return -EACCES;
			switch (type)
			{
				case SOCK_RAW:
					return net_raw_open(AF_PACKET, protocol, sock);
				default:
					return -EINVAL;
			}
			break;
		case AF_INET6:
			return ip6_open(domain, type, protocol, sock);
		default:
			return -EAFNOSUPPORT;
	}
}

int sock_openpair(int domain, int type, int protocol, struct sock **socks)
{
	switch (domain)
	{
		case PF_LOCAL:
			switch (type)
			{
				case SOCK_STREAM:
					return pfl_stream_openpair(domain, type, protocol, socks);
				case SOCK_DGRAM:
					return pfl_dgram_openpair(domain, type, protocol, socks);
				default:
					return -EINVAL;
			}
			break;
		case PF_INET:
		case AF_INET6:
		case AF_PACKET:
			return -EOPNOTSUPP;
		default:
			return -EAFNOSUPPORT;
	}
}

int sock_new(int domain, int type, int protocol, const struct sock_op *op,
             struct sock **sockp)
{
	struct sock *sock = sma_alloc(&sock_sma, M_ZERO);
	if (!sock)
		return -ENOMEM;
	TAILQ_INIT(&sock->poll_entries);
	mutex_init(&sock->mutex, 0);
	waitq_init(&sock->rwaitq);
	waitq_init(&sock->wwaitq);
	sock->domain = domain;
	sock->type = type;
	sock->protocol = protocol;
	sock->op = op;
	sock->state = SOCK_ST_NONE;
	refcount_init(&sock->refcount, 1);
	*sockp = sock;
	return 0;
}

void sock_ref(struct sock *sock)
{
	refcount_inc(&sock->refcount);
}

void sock_free(struct sock *sock)
{
	if (refcount_dec(&sock->refcount))
		return;
	sock_release(sock);
	waitq_destroy(&sock->rwaitq);
	waitq_destroy(&sock->wwaitq);
	poller_remove(&sock->poll_entries);
	mutex_destroy(&sock->mutex);
	sma_free(&sock_sma, sock);
}

int sock_release(struct sock *sock)
{
	sock_shutdown(sock, SHUT_RDWR); /* XXX somehow check for return value ? */
	if (!sock->op || !sock->op->release)
		return 0;
	return sock->op->release(sock);
}

int sock_bind(struct sock *sock, const struct sockaddr *addr, socklen_t addrlen)
{
	if (!sock->op || !sock->op->bind)
		return -EOPNOTSUPP;
	return sock->op->bind(sock, addr, addrlen);
}

int sock_accept(struct sock *sock, struct sock **child)
{
	if (!sock->op || !sock->op->accept)
		return -EOPNOTSUPP;
	return sock->op->accept(sock, child);
}

int sock_connect(struct sock *sock, const struct sockaddr *addr,
                 socklen_t addrlen)
{
	if (!sock->op || !sock->op->connect)
		return -EOPNOTSUPP;
	return sock->op->connect(sock, addr, addrlen);
}

int sock_listen(struct sock *sock, int backlog)
{
	if (!sock->op || !sock->op->listen)
		return -EOPNOTSUPP;
	return sock->op->listen(sock, backlog);
}

ssize_t sock_recv(struct sock *sock, struct msghdr *msg, int flags)
{
	if (!sock->op || !sock->op->recv)
		return -EOPNOTSUPP;
	return sock->op->recv(sock, msg, flags);
}

ssize_t sock_send(struct sock *sock, struct msghdr *msg, int flags)
{
	if (!sock->op || !sock->op->send)
		return -EOPNOTSUPP;
	return sock->op->send(sock, msg, flags);
}

int sock_getopt(struct sock *sock, int level, int opt, void *uval,
                socklen_t *ulen)
{
	if (!sock->op || !sock->op->getopt)
		return -EOPNOTSUPP;
	return sock->op->getopt(sock, level, opt, uval, ulen);
}

int sock_setopt(struct sock *sock, int level, int opt, const void *uval,
                socklen_t len)
{
	if (!sock->op || !sock->op->setopt)
		return -EOPNOTSUPP;
	return sock->op->setopt(sock, level, opt, uval, len);
}

int sock_shutdown(struct sock *sock, int how)
{
	if (!sock->op || !sock->op->shutdown)
		return -EOPNOTSUPP;
	return sock->op->shutdown(sock, how);
}

int sock_sol_setopt(struct sock *sock, int level, int opt, const void *uval,
                    socklen_t len)
{
	if (level != SOL_SOCKET)
		return -EINVAL;
	switch (opt)
	{
		case SO_RCVTIMEO:
		{
			struct timeval tv;
			if (len != sizeof(struct timeval))
				return -EINVAL;
			struct thread *thread = curcpu()->thread;
			int ret = vm_copyin(thread->proc->vm_space,
			                    &tv, uval, len);
			if (ret)
				return ret;
			ret = timeval_validate(&tv);
			if (ret)
				return ret;
			sock->rcv_timeo.tv_sec = tv.tv_sec;
			sock->rcv_timeo.tv_nsec = tv.tv_usec * 1000;
			return 0;
		}
		case SO_SNDTIMEO:
		{
			struct timeval tv;
			if (len != sizeof(struct timeval))
				return -EINVAL;
			struct thread *thread = curcpu()->thread;
			int ret = vm_copyin(thread->proc->vm_space,
			                    &tv, uval, len);
			if (ret)
				return ret;
			ret = timeval_validate(&tv);
			if (ret)
				return ret;
			timespec_from_timeval(&sock->snd_timeo, &tv);
			return 0;
		}
		default:
			return -EINVAL;
	}
}

static int getopt_out(void *uval, socklen_t *ulen, const void *kval,
                      socklen_t klen)
{
	struct thread *thread = curcpu()->thread;
	socklen_t len;
	int ret;

	ret = vm_copyin(thread->proc->vm_space, &len, ulen, sizeof(len));
	if (ret)
		return ret;
	ret = vm_copyout(thread->proc->vm_space, ulen, &klen, sizeof(klen));
	if (ret)
		return ret;
	if (len < klen)
		return -EINVAL;
	ret = vm_copyout(thread->proc->vm_space, uval, kval, klen);
	if (ret)
		return ret;
	return 0;
}

int sock_sol_getopt(struct sock *sock, int level, int opt, void *uval,
                    socklen_t *ulen)
{
	if (level != SOL_SOCKET)
		return -EINVAL;
	switch (opt)
	{
		case SO_RCVTIMEO:
		{
			struct timeval tv;
			timeval_from_timespec(&tv, &sock->rcv_timeo);
			return getopt_out(uval, ulen, &tv, sizeof(tv));
		}
		case SO_SNDTIMEO:
		{
			struct timeval tv;
			timeval_from_timespec(&tv, &sock->snd_timeo);
			return getopt_out(uval, ulen, &tv, sizeof(tv));
		}
	}
	return -EINVAL;
}

int sock_sol_ioctl(struct sock *sock, unsigned long request, uintptr_t data)
{
	struct thread *thread = curcpu()->thread;

	(void)sock;
	switch (request)
	{
		case SIOCGIFHWADDR:
		{
			struct ifreq ifreq;
			int ret = vm_copyin(thread->proc->vm_space, &ifreq,
			                    (void*)data, sizeof(ifreq));
			if (ret)
				return ret;
			if (!memchr(ifreq.ifr_name, 0, IFNAMSIZ))
				return -EINVAL;
			struct netif *netif = netif_from_name(ifreq.ifr_name);
			if (!netif)
				return -EINVAL;
			if (netif->flags & IFF_LOOPBACK) /* XXX I guess ? */
			{
				netif_free(netif);
				return -ENOENT;
			}
			ifreq.ifr_hwaddr.sa_family = 0;
			memcpy(ifreq.ifr_hwaddr.sa_data, netif->ether.addr,
			       ETHER_ADDR_LEN);
			netif_free(netif);
			ret = vm_copyout(thread->proc->vm_space, (void*)data,
			                 &ifreq, sizeof(ifreq));
			return ret;
		}
		case SIOCSIFADDR:
		{
			struct ifreq ifreq;
			int ret = vm_copyin(thread->proc->vm_space, &ifreq,
			                    (void*)data, sizeof(ifreq));
			if (ret)
				return ret;
			if (ifreq.ifr_addr.sa_family != AF_INET)
				return -EINVAL;
			if (!memchr(ifreq.ifr_name, 0, IFNAMSIZ))
				return -EINVAL;
			struct netif *netif = netif_from_name(ifreq.ifr_name);
			if (!netif)
				return -EINVAL;
			if (TAILQ_EMPTY(&netif->addrs))
			{
				struct netif_addr *addr = netif_addr_alloc();
				if (!addr)
				{
					netif_free(netif);
					return -ENOMEM;
				}
				addr->addr = ifreq.ifr_addr;
				struct sockaddr_in *sin_mask = (struct sockaddr_in*)&addr->mask;
				sin_mask->sin_family = AF_INET;
				sin_mask->sin_port = 0;
				sin_mask->sin_addr.s_addr = 0xFFFFFFFF;
				TAILQ_INSERT_HEAD(&netif->addrs, addr, chain);
				netif_free(netif);
				return 0;
			}
			struct netif_addr *addr = TAILQ_FIRST(&netif->addrs);
			addr->addr = ifreq.ifr_addr;
			netif_free(netif);
			return 0;
		}
		case SIOCGIFADDR:
		{
			struct ifreq ifreq;
			int ret = vm_copyin(thread->proc->vm_space, &ifreq,
			                    (void*)data, sizeof(ifreq));
			if (ret)
				return ret;
			if (!memchr(ifreq.ifr_name, 0, IFNAMSIZ))
				return -EINVAL;
			struct netif *netif = netif_from_name(ifreq.ifr_name);
			if (!netif)
				return -EINVAL;
			if (TAILQ_EMPTY(&netif->addrs))
			{
				netif_free(netif);
				return -EINVAL;
			}
			memcpy(&ifreq.ifr_addr,
			       &TAILQ_FIRST(&netif->addrs)->addr,
			       sizeof(ifreq.ifr_addr));
			ret = vm_copyout(thread->proc->vm_space, (void*)data,
			                 &ifreq, sizeof(ifreq));
			netif_free(netif);
			return ret;
		}
		case SIOCSIFNETMASK:
		{
			struct ifreq ifreq;
			int ret = vm_copyin(thread->proc->vm_space, &ifreq,
			                    (void*)data, sizeof(ifreq));
			if (ret)
				return ret;
			if (ifreq.ifr_netmask.sa_family != AF_INET)
				return -EINVAL;
			if (!memchr(ifreq.ifr_name, 0, IFNAMSIZ))
				return -EINVAL;
			struct netif *netif = netif_from_name(ifreq.ifr_name);
			if (!netif)
				return -EINVAL;
			if (TAILQ_EMPTY(&netif->addrs)) /* can't create an addr without an ip */
				return -EINVAL;
			struct netif_addr *addr = TAILQ_FIRST(&netif->addrs);
			addr->mask = ifreq.ifr_netmask;
			netif_free(netif);
			return 0;
		}
		case SIOCGIFNETMASK:
		{
			struct ifreq ifreq;
			int ret = vm_copyin(thread->proc->vm_space, &ifreq,
			                    (void*)data, sizeof(ifreq));
			if (ret)
				return ret;
			if (!memchr(ifreq.ifr_name, 0, IFNAMSIZ))
				return -EINVAL;
			struct netif *netif = netif_from_name(ifreq.ifr_name);
			if (!netif)
				return -EINVAL;
			if (TAILQ_EMPTY(&netif->addrs))
			{
				netif_free(netif);
				return -EINVAL;
			}
			memcpy(&ifreq.ifr_netmask,
			       &TAILQ_FIRST(&netif->addrs)->mask,
			       sizeof(ifreq.ifr_netmask));
			ret = vm_copyout(thread->proc->vm_space, (void*)data,
			                 &ifreq, sizeof(ifreq));
			netif_free(netif);
			return ret;
		}
		case SIOCSGATEWAY:
		{
			struct sockaddr sa;
			int ret = vm_copyin(thread->proc->vm_space, &sa,
			                    (void*)data, sizeof(sa));
			if (ret)
				return ret;
			if (sa.sa_family != AF_INET)
				return -EINVAL;
			ip4_set_gateway(((struct sockaddr_in*)&sa)->sin_addr);
			return 0;
		}
		case SIOCGGATEWAY:
		{
			struct sockaddr_in sin;
			sin.sin_family = AF_INET;
			sin.sin_port = 0;
			sin.sin_addr = ip4_get_gateway();
			memset(&sin.sin_zero[0], 0, sizeof(sin.sin_zero));
			return vm_copyout(thread->proc->vm_space, (void*)data,
			                  &sin, sizeof(sin));
		}
		case SIOCGIFCONF:
		{
			struct ifconf ifconf;
			int ret = vm_copyin(thread->proc->vm_space, &ifconf,
			                    (void*)data, sizeof(ifconf));
			if (ret)
				return ret;
			if (!ifconf.ifc_buf)
			{
				ifconf.ifc_len = netif_count()
				               * sizeof(struct ifreq);
				return vm_copyout(thread->proc->vm_space,
				                  (void*)data, &ifconf,
				                  sizeof(ifconf));
			}
			ret = netif_fill_ifconf(&ifconf);
			if (ret)
				return ret;
			return vm_copyout(thread->proc->vm_space, (void*)data,
			                   &ifconf, sizeof(ifconf));
		}
		case SIOCGIFFLAGS:
		{
			struct ifreq ifreq;
			int ret = vm_copyin(thread->proc->vm_space, &ifreq,
			                    (void*)data, sizeof(ifreq));
			if (ret)
				return ret;
			if (!memchr(ifreq.ifr_name, 0, IFNAMSIZ))
				return -EINVAL;
			struct netif *netif = netif_from_name(ifreq.ifr_name);
			if (!netif)
				return -EINVAL;
			ifreq.ifr_flags = netif->flags;
			ret = vm_copyout(thread->proc->vm_space, (void*)data,
			                 &ifreq, sizeof(ifreq));
			netif_free(netif);
			return ret;
		}
		case SIOCSIFFLAGS:
		{
			struct ifreq ifreq;
			int ret = vm_copyin(thread->proc->vm_space, &ifreq,
			                    (void*)data, sizeof(ifreq));
			if (ret)
				return ret;
			if (!memchr(ifreq.ifr_name, 0, IFNAMSIZ))
				return -EINVAL;
			struct netif *netif = netif_from_name(ifreq.ifr_name);
			if (!netif)
				return -EINVAL;
			if (ifreq.ifr_flags & ~(IFF_LOOPBACK | IFF_UP | IFF_BROADCAST | IFF_PROMISC | IFF_MULTICAST | IFF_ALLMULTI))
			{
				netif_free(netif);
				return -EINVAL;
			}
			int x = ifreq.ifr_flags ^ netif->flags;
			if (x & IFF_LOOPBACK)
			{
				netif_free(netif);
				return -EINVAL;
			}
			netif->flags = ifreq.ifr_flags;
			if (x & (IFF_UP | IFF_BROADCAST | IFF_MULTICAST | IFF_PROMISC | IFF_ALLMULTI))
			{
				if (netif->op->update_rx_filter)
					netif->op->update_rx_filter(netif);
			}
			netif_free(netif);
			return ret;
		}
	}
	return -EINVAL;
}
