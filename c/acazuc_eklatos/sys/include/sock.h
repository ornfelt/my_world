#ifndef SOCK_H
#define SOCK_H

#include <net/packet.h>
#include <net/local.h>
#include <net/ip6.h>
#include <net/ip4.h>
#include <net/net.h>

#include <refcount.h>
#include <mutex.h>
#include <types.h>
#include <time.h>
#include <poll.h>
#include <uio.h>

#define PF_UNSPEC 0
#define PF_UNIX   1
#define PF_LOCAL  PF_UNIX
#define PF_INET   2
#define PF_INET6  10
#define PF_PACKET 17

#define AF_UNSPEC PF_UNSPEC
#define AF_UNIX   PF_UNIX
#define AF_LOCAL  PF_LOCAL
#define AF_INET   PF_INET
#define AF_INET6  PF_INET6
#define AF_PACKET PF_PACKET

#define MSG_DONTWAIT (1 << 0)

#define SOCK_STREAM 1
#define SOCK_DGRAM  2
#define SOCK_RAW    3

#define SOL_SOCKET 1

#define SO_RCVTIMEO 1
#define SO_SNDTIMEO 2

#define SHUT_RD   0
#define SHUT_WR   1
#define SHUT_RDWR 2

struct poll_entry;
struct sockaddr;
struct file;
struct sock;

union sockaddr_union
{
	sa_family_t su_family;
	struct sockaddr sa;
	struct sockaddr_in sin;
	struct sockaddr_in6 sin6;
	struct sockaddr_un sun;
	struct sockaddr_ll sll;
};

struct msghdr
{
	void *msg_name;
	socklen_t msg_namelen;
	struct iovec *msg_iov;
	size_t msg_iovlen;
	void *msg_control;
	size_t msg_controllen;
	int msg_flags;
};

struct sock_op
{
	int (*open)(int domain, int type, int protocol, struct sock **sock);
	int (*release)(struct sock *sock);
	int (*bind)(struct sock *sock, const struct sockaddr *addr,
	            socklen_t addrlen);
	int (*accept)(struct sock *sock, struct sock **child);
	int (*connect)(struct sock *sock, const struct sockaddr *addr,
	               socklen_t addrlen);
	int (*listen)(struct sock *sock, int backlog);
	int (*ioctl)(struct sock *sock, unsigned long request, uintptr_t data);
	ssize_t (*recv)(struct sock *sock, struct msghdr *msg, int flags);
	ssize_t (*send)(struct sock *sock, struct msghdr *msg, int flags);
	int (*poll)(struct sock *sock, struct poll_entry *entry);
	int (*getopt)(struct sock *sock, int level, int opt, void *uval,
	              socklen_t *ulen);
	int (*setopt)(struct sock *sock, int level, int opt, const void *uval,
	              socklen_t len);
	int (*shutdown)(struct sock *sock, int how);
	int (*input)(struct netif *netif, struct netpkt *pkt,
	             struct sockaddr *src, struct sockaddr *dst);
};

enum sock_state
{
	SOCK_ST_NONE,
	SOCK_ST_LISTENING,
	SOCK_ST_CONNECTING,
	SOCK_ST_CONNECTED,
	SOCK_ST_CLOSING,
	SOCK_ST_CLOSED,
};

struct sock
{
	const struct sock_op *op;
	refcount_t refcount;
	struct mutex mutex;
	struct waitq rwaitq;
	struct waitq wwaitq;
	struct poller_head poll_entries;
	enum sock_state state;
	struct timespec rcv_timeo;
	struct timespec snd_timeo;
	union sockaddr_union src_addr;
	socklen_t src_addrlen;
	union sockaddr_union dst_addr;
	socklen_t dst_addrlen;
	int domain;
	int protocol;
	int type;
	void *userdata;
};

int sock_open(int domain, int type, int protocol, struct sock **sock);
int sock_openpair(int domain, int type, int protocol, struct sock **socks);
int sock_release(struct sock *sock);

int sock_new(int domain, int type, int protocol, const struct sock_op *op,
             struct sock **sockp);
void sock_ref(struct sock *sock);
void sock_free(struct sock *sock);

int sock_bind(struct sock *sock, const struct sockaddr *addr,
              socklen_t addrlen);
int sock_accept(struct sock *sock, struct sock **child);
int sock_connect(struct sock *sock, const struct sockaddr *addr,
                 socklen_t addrlen);
int sock_listen(struct sock *sock, int backlog);
ssize_t sock_recv(struct sock *sock, struct msghdr *msg, int flags);
ssize_t sock_send(struct sock *sock, struct msghdr *msg, int flags);
int sock_getopt(struct sock *sock, int level, int opt, void *uval,
                socklen_t *ulen);
int sock_setopt(struct sock *sock, int level, int opt, const void *uval,
                socklen_t len);
int sock_shutdown(struct sock *sock, int how);

int sock_sol_setopt(struct sock *sock, int level, int opt, const void *uval,
                    socklen_t len);
int sock_sol_getopt(struct sock *sock, int level, int opt, void *uval,
                    socklen_t *ulen);
int sock_sol_ioctl(struct sock *sock, unsigned long request, uintptr_t data);

/* XXX move somewhere else */
static inline void uio_from_msghdr(struct uio *uio, const struct msghdr *msg)
{
	uio->iov = msg->msg_iov;
	uio->iovcnt = msg->msg_iovlen;
	uio->count = 0;
	for (size_t i = 0; i < uio->iovcnt; ++i)
		uio->count += uio->iov[i].iov_len;
	uio->off = 0;
	uio->userbuf = 1;
}

static inline void sock_lock(struct sock *sock)
{
	mutex_lock(&sock->mutex);
}

static inline void sock_unlock(struct sock *sock)
{
	mutex_unlock(&sock->mutex);
}

extern const struct file_op g_sock_fop;

#endif
