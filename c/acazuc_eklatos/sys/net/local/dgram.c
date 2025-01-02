#include <net/local.h>

#include <errno.h>
#include <sock.h>
#include <sma.h>
#include <std.h>

struct sock_pfl_dgram
{
	struct sock *sock;
};

static struct sma sock_pfl_dgram_sma;

void sock_pfl_dgram_init(void)
{
	sma_init(&sock_pfl_dgram_sma, sizeof(struct sock_pfl_dgram), NULL, NULL, "sock_pfl_dgram");
}

static int pfl_dgram_ioctl(struct sock *sock, unsigned long request,
                           uintptr_t data)
{
	(void)sock;
	(void)request;
	(void)data;
	/* XXX */
	return -EINVAL;
}

static ssize_t pfl_dgram_recv(struct sock *sock, struct msghdr *msg, int flags)
{
	(void)sock;
	(void)msg;
	(void)flags;
	/* XXX */
	return -EINVAL;
}

static ssize_t pfl_dgram_send(struct sock *sock, struct msghdr *msg, int flags)
{
	(void)sock;
	(void)msg;
	(void)flags;
	/* XXX */
	return -EINVAL;
}

static int pfl_dgram_bind(struct sock *sock, const struct sockaddr *addr,
                          socklen_t addrlen)
{
	(void)sock;
	(void)addr;
	(void)addrlen;
	/* XXX */
	return -EINVAL;
}

static int pfl_dgram_connect(struct sock *sock, const struct sockaddr *addr,
                             socklen_t addrlen)
{
	(void)sock;
	(void)addr;
	(void)addrlen;
	/* XXX */
	return -EINVAL;
}

static int pfl_dgram_release(struct sock *sock)
{
	struct sock_pfl_dgram *pfl_dgram = sock->userdata;
	sma_free(&sock_pfl_dgram_sma, pfl_dgram);
	return 0;
}

static int pfl_dgram_shutdown(struct sock *sock, int how)
{
	(void)sock;
	(void)how;
	/* XXX */
	return 0;
}

static const struct sock_op pfl_dgram_op =
{
	.release = pfl_dgram_release,
	.bind = pfl_dgram_bind,
	.connect = pfl_dgram_connect,
	.ioctl = pfl_dgram_ioctl,
	.recv = pfl_dgram_recv,
	.send = pfl_dgram_send,
	.shutdown = pfl_dgram_shutdown,
};

int pfl_dgram_open(int domain, int type, int protocol, struct sock **sock)
{
	struct sock_pfl_dgram *pfl_dgram;
	int ret;

	if (domain != AF_LOCAL
	 || type != SOCK_DGRAM
	 || protocol)
		return -EINVAL;
	pfl_dgram = sma_alloc(&sock_pfl_dgram_sma, M_ZERO);
	if (!pfl_dgram)
		return -ENOMEM;
	ret = sock_new(AF_LOCAL, SOCK_DGRAM, protocol, &pfl_dgram_op, sock);
	if (ret)
	{
		sma_free(&sock_pfl_dgram_sma, pfl_dgram);
		return ret;
	}
	pfl_dgram->sock = *sock;
	(*sock)->userdata = pfl_dgram;
	return 0;
}

int pfl_dgram_openpair(int domain, int type, int protocol, struct sock **socks)
{
	(void)domain;
	(void)type;
	(void)protocol;
	(void)socks;
	return -EOPNOTSUPP;
}
