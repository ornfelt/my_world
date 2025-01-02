#include <net/net.h>
#include <net/if.h>

#include <errno.h>
#include <proc.h>
#include <sock.h>
#include <file.h>
#include <sma.h>
#include <std.h>
#include <vfs.h>
#include <cpu.h>
#include <mem.h>

static struct spinlock netifs_lock = SPINLOCK_INITIALIZER(); /* XXX rwlock */
static struct netif_head netifs = TAILQ_HEAD_INITIALIZER(netifs);

static struct sma netif_sma;
static struct sma netif_addr_sma;

static int netif_sys_open(struct file *file, struct node *node);
static ssize_t netif_sys_read(struct file *file, struct uio *uio);

static const struct file_op netif_fop =
{
	.open = netif_sys_open,
	.read = netif_sys_read,
};

void net_init(void)
{
	sma_init(&netif_sma, sizeof(struct netif), NULL, NULL, "netif");
	sma_init(&netif_addr_sma, sizeof(struct netif_addr), NULL, NULL, "netif_addr");
}

int netif_alloc(const char *name, const struct netif_op *op, struct netif **netifp)
{
	struct netif *netif = sma_alloc(&netif_sma, M_ZERO);
	if (!netif)
		return -ENOMEM;
	for (size_t i = 0; ; ++i)
	{
		if (i == 4096)
		{
			sma_free(&netif_sma, netif);
			return -ENOMEM;
		}
		snprintf(netif->name, sizeof(netif->name), "%s%zu", name, i);
		struct netif *tmp = netif_from_name(netif->name);
		if (!tmp)
			break;
		netif_free(tmp);
	}
	TAILQ_INIT(&netif->addrs);
	refcount_init(&netif->refcount, 1);
	netif->op = op;
	*netifp = netif;
	spinlock_lock(&netifs_lock);
	TAILQ_INSERT_TAIL(&netifs, netif, chain);
	spinlock_unlock(&netifs_lock);

	char path[1024];
	snprintf(path, sizeof(path), "net/%s", netif->name);
	int ret = sysfs_mknode(path, 0, 0, 0400, &netif_fop, &netif->sysfs_node);
	if (!ret)
		netif->sysfs_node->userdata = netif;
	return 0;
}

void netif_free(struct netif *netif)
{
	if (!netif)
		return;
	if (refcount_dec(&netif->refcount))
		return;
	spinlock_lock(&netifs_lock);
	TAILQ_REMOVE(&netifs, netif, chain);
	spinlock_unlock(&netifs_lock);
	struct netif_addr *addr;
	TAILQ_FOREACH(addr, &netif->addrs, chain)
		free(addr);
	sma_free(&netif_sma, netif);
}

void netif_ref(struct netif *netif)
{
	refcount_inc(&netif->refcount);
}

size_t netif_count(void)
{
	size_t count = 0;
	spinlock_lock(&netifs_lock);
	struct netif *netif;
	TAILQ_FOREACH(netif, &netifs, chain)
		count++;
	spinlock_unlock(&netifs_lock);
	return count;
}

int netif_fill_ifconf(struct ifconf *ifconf)
{
	size_t max = ifconf->ifc_len / sizeof(struct ifreq);
	ifconf->ifc_len = 0;
	if (!max)
		return 0;
	struct thread *thread = curcpu()->thread;
	spinlock_lock(&netifs_lock);
	size_t i = 0;
	struct netif *netif;
	int ret = 0;
	TAILQ_FOREACH(netif, &netifs, chain)
	{
		struct ifreq ifreq;
		memset(&ifreq, 0, sizeof(ifreq));
		strlcpy(ifreq.ifr_name, netif->name, sizeof(ifreq.ifr_name));
		if (!TAILQ_EMPTY(&netif->addrs))
		{
			memcpy(&ifreq.ifr_addr,
			       &TAILQ_FIRST(&netif->addrs)->addr,
			       sizeof(ifreq.ifr_addr));
		}
		/* XXX not optimal */
		ret = vm_copyout(thread->proc->vm_space, &ifconf->ifc_req[i],
		                 &ifreq, sizeof(ifreq));
		if (ret)
			break;
		ifconf->ifc_len += sizeof(struct ifreq);
		if (++i == max)
			break;
	}
	spinlock_unlock(&netifs_lock);
	return ret;
}

struct netif *netif_from_name(const char *name)
{
	spinlock_lock(&netifs_lock);
	struct netif *netif;
	TAILQ_FOREACH(netif, &netifs, chain)
	{
		if (strcmp(name, netif->name))
			continue;
		netif_ref(netif);
		break;
	}
	spinlock_unlock(&netifs_lock);
	return netif;
}

static int addr_cmp(const struct sockaddr *addr,
                    const struct netif_addr *netif_addr)
{
	if (addr->sa_family != netif_addr->addr.sa_family)
		return 1;
	switch (addr->sa_family)
	{
		case AF_INET:
		{
			struct sockaddr_in *sin = (struct sockaddr_in*)addr;
			struct sockaddr_in *sin_addr = (struct sockaddr_in*)&netif_addr->addr;
			return sin->sin_addr.s_addr != sin_addr->sin_addr.s_addr;
		}
		default:
			return 1;
	}
	return 1;
}

struct netif *netif_from_addr(const struct sockaddr *addr,
                              struct netif_addr **netif_addrp)
{
	spinlock_lock(&netifs_lock);
	struct netif *netif;
	TAILQ_FOREACH(netif, &netifs, chain)
	{
		struct netif_addr *netif_addr;
		TAILQ_FOREACH(netif_addr, &netif->addrs, chain)
		{
			if (addr_cmp(addr, netif_addr))
				continue;
			netif_ref(netif);
			spinlock_unlock(&netifs_lock);
			if (netif_addrp)
				*netif_addrp = netif_addr;
			return netif;
		}
	}
	spinlock_unlock(&netifs_lock);
	return NULL;
}

static int net_cmp(const struct sockaddr *addr,
                    const struct netif_addr *netif_addr)
{
	if (addr->sa_family != netif_addr->addr.sa_family)
		return 1;
	switch (addr->sa_family)
	{
		case AF_INET:
		{
			struct sockaddr_in *sin = (struct sockaddr_in*)addr;
			struct sockaddr_in *sin_addr = (struct sockaddr_in*)&netif_addr->addr;
			struct sockaddr_in *sin_mask = (struct sockaddr_in*)&netif_addr->mask;
			return (sin_addr->sin_addr.s_addr & sin_mask->sin_addr.s_addr)
			    != (sin->sin_addr.s_addr & sin_mask->sin_addr.s_addr);
		}
		default:
			return 1;
	}
	return 1;
}

struct netif *netif_from_net(const struct sockaddr *addr,
                             struct netif_addr **netif_addrp)
{
	spinlock_lock(&netifs_lock);
	struct netif *netif;
	TAILQ_FOREACH(netif, &netifs, chain)
	{
		struct netif_addr *netif_addr;
		TAILQ_FOREACH(netif_addr, &netif->addrs, chain)
		{
			if (net_cmp(addr, netif_addr))
				continue;
			netif_ref(netif);
			spinlock_unlock(&netifs_lock);
			if (netif_addrp)
				*netif_addrp = netif_addr;
			return netif;
		}
	}
	spinlock_unlock(&netifs_lock);
	return NULL;
}

struct netif *netif_from_ether(const struct ether_addr *addr)
{
	spinlock_lock(&netifs_lock);
	struct netif *netif;
	TAILQ_FOREACH(netif, &netifs, chain)
	{
		if (netif->flags & IFF_LOOPBACK) /* XXX I guess ? */
			continue;
		if (memcmp(netif->ether.addr, addr->addr, ETHER_ADDR_LEN))
			continue;
		netif_ref(netif);
		spinlock_unlock(&netifs_lock);
		return netif;
	}
	spinlock_unlock(&netifs_lock);
	return NULL;
}

struct netif_addr *netif_addr_alloc(void)
{
	return sma_alloc(&netif_addr_sma, M_ZERO);
}

void netif_addr_free(struct netif_addr *netif_addr)
{
	sma_free(&netif_addr_sma, netif_addr);
}

static int netif_sys_open(struct file *file, struct node *node)
{
	file->userdata = node->userdata;
	return 0;
}

static ssize_t netif_sys_read(struct file *file, struct uio *uio)
{
	struct netif *netif = file->userdata;
	size_t count = uio->count;
	off_t off = uio->off;
	uprintf(uio, "name: %s\n", netif->name);
	uprintf(uio, "rx_packets: %" PRIu64 "\n"
	             "rx_bytes:   %" PRIu64 "\n"
	             "rx_errors:  %" PRIu64 "\n"
	             "tx_packets: %" PRIu64 "\n"
	             "tx_bytes:   %" PRIu64 "\n"
	             "tx_errors:  %" PRIu64 "\n",
	             netif->stats.rx_packets,
	             netif->stats.rx_bytes,
	             netif->stats.rx_errors,
	             netif->stats.tx_packets,
	             netif->stats.tx_bytes,
	             netif->stats.tx_errors);
	uio->off = off + count - uio->count;
	return count - uio->count;
}
