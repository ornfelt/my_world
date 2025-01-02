#include <net/local.h>

#include <pipebuf.h>
#include <proc.h>
#include <sock.h>
#include <file.h>
#include <vfs.h>
#include <sma.h>
#include <std.h>
#include <cpu.h>

#define PAIR(sps) ((sps)->client.pair)
#define PAIR_INPUT(sps) (&PAIR(sps)->data[(sps)->client.pair_idx])
#define PAIR_OUTPUT(sps) (&PAIR(sps)->data[(sps)->client.pair_idx ^ 1])

struct sock_pfl_stream_pair
{
	struct
	{
		struct pipebuf pipebuf;
		struct mutex mutex;
		struct waitq rwaitq;
		struct waitq wwaitq;
	} data[2];
	refcount_t refcount;
};

struct sock_pfl_stream
{
	struct sock *sock;
	struct pfls *pfls;
	union
	{
		struct
		{
			struct sock_pfl_stream **queue; /* protected by pfls->mutex */
			uint32_t queue_size;
			uint32_t queue_len;
		} server;
		struct
		{
			struct sock_pfl_stream_pair *pair;
			unsigned pair_idx; /* input pipebuf index in pair */
			struct sock_pfl_stream *peer; /* XXX remove: move poll entries to pipebuf */
		} client;
	};
};

static struct sma sock_pfl_stream_sma;
static struct sma sock_pfl_stream_pair_sma;

void sock_pfl_stream_init(void)
{
	sma_init(&sock_pfl_stream_sma, sizeof(struct sock_pfl_stream), NULL, NULL, "sock_pfl_stream");
	sma_init(&sock_pfl_stream_pair_sma, sizeof(struct sock_pfl_stream_pair), NULL, NULL, "sock_pfl_stream_pair");
}

int pfl_stream_poll(struct sock *sock, struct poll_entry *entry)
{
	struct sock_pfl_stream *pfl_stream = sock->userdata;
	int ret;

	/* XXX should be done under sock lock */
	switch (sock->state)
	{
		case SOCK_ST_LISTENING:
			ret = 0;
			mutex_lock(&pfl_stream->pfls->mutex);
			if (entry->events & POLLIN)
			{
				if (pfl_stream->server.queue_len)
					ret |= POLLIN;
			}
			mutex_unlock(&pfl_stream->pfls->mutex);
			break;
		case SOCK_ST_CONNECTED:
			ret = 0;
			ret |= pipebuf_poll(&PAIR_INPUT(pfl_stream)->pipebuf,
			                    entry->events & ~POLLOUT); /* XXX better mask */
			ret |= pipebuf_poll(&PAIR_OUTPUT(pfl_stream)->pipebuf,
			                    entry->events & ~POLLIN); /* XXX better mask */
			break;
		default:
			ret = -ENOTCONN;
			break;
	}
	if (ret)
		return ret;
	entry->file_head = &sock->poll_entries;
	return poller_add(entry);
}

int pfl_stream_ioctl(struct sock *sock, unsigned long request, uintptr_t data)
{
	return sock_sol_ioctl(sock, request, data);
}

ssize_t pfl_stream_recv(struct sock *sock, struct msghdr *msg, int flags)
{
	struct sock_pfl_stream *pfl_stream = sock->userdata;
	struct uio uio;
	ssize_t ret;

	sock_lock(sock);
	if (sock->state != SOCK_ST_CONNECTED)
	{
		ret = -ENOTCONN;
		goto end;
	}
	uio_from_msghdr(&uio, msg);
	ret = pipebuf_read(&PAIR_INPUT(pfl_stream)->pipebuf, &uio,
	                   (flags & MSG_DONTWAIT) ? 0 : 1,
	                   (sock->rcv_timeo.tv_sec || sock->rcv_timeo.tv_nsec)
	                  ? &sock->rcv_timeo : NULL);
	if (ret < 0)
		goto end;
	if (pfl_stream->client.peer)
		poller_broadcast(&pfl_stream->client.peer->sock->poll_entries, POLLOUT);

end:
	sock_unlock(sock);
	return ret;
}

ssize_t pfl_stream_send(struct sock *sock, struct msghdr *msg, int flags)
{
	struct sock_pfl_stream *pfl_stream = sock->userdata;
	struct uio uio;
	ssize_t ret;

	sock_lock(sock);
	if (sock->state != SOCK_ST_CONNECTED)
	{
		ret = -ENOTCONN;
		goto end;
	}
	uio_from_msghdr(&uio, msg);
	ret = pipebuf_write(&PAIR_OUTPUT(pfl_stream)->pipebuf,
	                    &uio, (flags & MSG_DONTWAIT) ? 0 : 1,
	                   (sock->snd_timeo.tv_sec || sock->snd_timeo.tv_nsec)
	                  ? &sock->rcv_timeo : NULL);
	if (ret < 0)
		goto end;
	if (pfl_stream->client.peer)
		poller_broadcast(&pfl_stream->client.peer->sock->poll_entries, POLLIN);

end:
	sock_unlock(sock);
	return ret;
}

int pfl_stream_bind(struct sock *sock, const struct sockaddr *addr,
                    socklen_t addrlen)
{
	struct sock_pfl_stream *pfl_stream = sock->userdata;
	if (addrlen < sizeof(struct sockaddr_un))
		return -EINVAL;
	/* XXX sock lock */
	if (sock->state != SOCK_ST_NONE)
		return -EINVAL;
	struct thread *thread = curcpu()->thread;
	struct sockaddr_un *sun = (struct sockaddr_un*)addr;
	if (!memchr(sun->sun_path, '\0', sizeof(sun->sun_path)))
		return -EINVAL;
	struct node *dir;
	char *end_fn;
	int ret = vfs_getdir(thread->proc->cwd, sun->sun_path, 0, &dir, &end_fn);
	if (ret)
		return ret;
	if (!*end_fn)
	{
		node_free(dir);
		return -EISDIR;
	}
	if (dir->sb->flags & ST_RDONLY)
	{
		node_free(dir);
		return -EROFS;
	}
	struct node *node;
	ret = node_lookup(dir, end_fn, strlen(end_fn), &node);
	if (!ret)
	{
		node_free(node);
		node_free(dir);
		return -EADDRINUSE;
	}
	if (ret != -ENOENT)
	{
		node_free(dir);
		return ret;
	}
	size_t end_fn_len = strlen(end_fn);
	fs_attr_mask_t mask = FS_ATTR_UID | FS_ATTR_GID | FS_ATTR_MODE;
	struct fs_attr attr;
	attr.mode = S_IFSOCK | (0777 & ~thread->proc->umask);
	attr.uid = thread->proc->cred.euid;
	attr.gid = thread->proc->cred.egid;
	ret = node_mknode(dir, end_fn, end_fn_len, mask, &attr, 0);
	if (ret)
	{
		if (ret == -EEXIST)
			ret = -EADDRINUSE;
		node_free(dir);
		return ret;
	}
	ret = node_lookup(dir, end_fn, end_fn_len, &node);
	node_free(dir);
	if (ret)
		return ret;
	ret = pfls_alloc(&node->pfls, node);
	if (ret)
	{
		node_free(node);
		return ret;
	}
	node->pfls->type = sock->type; /* XXX race condition */
	pfl_stream->pfls = node->pfls;
	pfls_ref(pfl_stream->pfls);
	node_free(node);
	return 0;
}

static void pair_free(struct sock_pfl_stream_pair *pair)
{
	if (refcount_dec(&pair->refcount))
		return;
	for (size_t i = 0; i < 2; ++i)
	{
		pipebuf_destroy(&pair->data[i].pipebuf);
		waitq_destroy(&pair->data[i].rwaitq);
		waitq_destroy(&pair->data[i].wwaitq);
		mutex_destroy(&pair->data[i].mutex);
	}
	sma_free(&sock_pfl_stream_pair_sma, pair);
}

static int pair_alloc(struct sock_pfl_stream_pair **pairp)
{
	struct sock_pfl_stream_pair *pair = sma_alloc(&sock_pfl_stream_pair_sma, 0);
	if (!pair)
		return -ENOMEM;
	refcount_init(&pair->refcount, 1);
	for (size_t i = 0; i < 2; ++i)
	{
		mutex_init(&pair->data[i].mutex, 0);
		waitq_init(&pair->data[i].rwaitq);
		waitq_init(&pair->data[i].wwaitq);
		int ret = pipebuf_init(&pair->data[i].pipebuf, PIPE_BUF * 2,
		                       &pair->data[i].mutex,
		                       &pair->data[i].rwaitq,
		                       &pair->data[i].wwaitq);
		if (ret)
		{
			pair_free(pair);
			return ret;
		}
	}
	*pairp = pair;
	return 0;
}

int pfl_stream_connect(struct sock *sock, const struct sockaddr *addr,
                       socklen_t addrlen)
{
	struct sock_pfl_stream *pfl_stream = sock->userdata;

	(void)addrlen;
	/* XXX sock lock */
	if (sock->state != SOCK_ST_NONE)
		return -EISCONN;
	struct thread *thread = curcpu()->thread;
	struct sockaddr_un *sun = (struct sockaddr_un*)addr;
	if (!memchr(sun->sun_path, '\0', sizeof(sun->sun_path)))
		return -EINVAL;
	struct node *node;
	int ret = vfs_getnode(thread->proc->cwd, sun->sun_path, 0, &node);
	if (ret)
		return ret;
	if (!S_ISSOCK(node->attr.mode))
	{
		node_free(node);
		return -ECONNREFUSED;
	}
	if (!node->pfls)
	{
		node_free(node);
		return -ECONNREFUSED;
	}
	struct pfls *pfls = node->pfls;
	mutex_lock(&pfls->mutex);
	if (!pfls->listen)
	{
		mutex_unlock(&pfls->mutex);
		node_free(node);
		return -ECONNREFUSED;
	}
	struct sock_pfl_stream *listener = pfls->listen->userdata;
	if (listener->server.queue_len >= listener->server.queue_size)
	{
		mutex_unlock(&pfls->mutex);
		node_free(node);
		return -ECONNREFUSED;
	}
	pfl_stream->client.pair_idx = 0;
	ret = pair_alloc(&pfl_stream->client.pair);
	if (ret)
	{
		mutex_unlock(&pfls->mutex);
		node_free(node);
		return ret;
	}
	PAIR_INPUT(pfl_stream)->pipebuf.nreaders = 1;
	PAIR_OUTPUT(pfl_stream)->pipebuf.nwriters = 1;
	listener->server.queue[listener->server.queue_len++] = pfl_stream;
	waitq_signal(&listener->sock->rwaitq, 0);
	poller_broadcast(&listener->sock->poll_entries, POLLIN);
	ret = waitq_wait_head_mutex(&PAIR_INPUT(pfl_stream)->wwaitq,
	                            &pfls->mutex, NULL);
	if (ret)
	{
		pair_free(pfl_stream->client.pair);
		mutex_unlock(&pfls->mutex);
		return ret;
	}
	sock->state = SOCK_ST_CONNECTED;
	pfl_stream->pfls = pfls;
	pfls_ref(pfls);
	mutex_unlock(&pfls->mutex);
	node_free(node);
	return 0;
}

int pfl_stream_accept(struct sock *sock, struct sock **child)
{
	struct sock_pfl_stream *pfl_stream = sock->userdata;
	/* XXX sock lock */
	if (sock->state != SOCK_ST_LISTENING)
		return -EINVAL;
	mutex_lock(&pfl_stream->pfls->mutex);
	while (!pfl_stream->server.queue_len)
	{
		int ret = waitq_wait_head_mutex(&pfl_stream->sock->rwaitq,
		                                &pfl_stream->pfls->mutex, NULL);
		if (ret)
		{
			mutex_unlock(&pfl_stream->pfls->mutex);
			return ret;
		}
	}
	struct sock_pfl_stream *peersock = pfl_stream->server.queue[0];
	pfl_stream->server.queue_len--;
	memmove(&pfl_stream->server.queue[0], &pfl_stream->server.queue[1],
	        sizeof(*pfl_stream->server.queue) * pfl_stream->server.queue_len);
	struct sock *localsock_st;
	int ret = sock_open(AF_UNIX, sock->type, sock->protocol,
	                    &localsock_st);
	if (ret)
	{
		waitq_signal(&PAIR_INPUT(peersock)->wwaitq,
		             -ECONNREFUSED);
		mutex_unlock(&pfl_stream->pfls->mutex);
		return ret;
	}
	struct sock_pfl_stream *localsock = localsock_st->userdata;
	localsock->client.pair_idx = 1;
	localsock->client.pair = peersock->client.pair;
	refcount_inc(&localsock->client.pair->refcount);
	PAIR_INPUT(localsock)->pipebuf.nreaders = 1;
	PAIR_OUTPUT(localsock)->pipebuf.nwriters = 1;
	localsock->client.peer = peersock;
	peersock->client.peer = localsock;
	localsock->sock->state = SOCK_ST_CONNECTED;
	waitq_signal(&PAIR_INPUT(peersock)->wwaitq, 0);
	localsock->pfls = pfl_stream->pfls;
	mutex_unlock(&pfl_stream->pfls->mutex);
	*child = localsock->sock;
	return 0;
}

static uint32_t npot32(uint32_t val)
{
	val--;
	val |= val >> 1;
	val |= val >> 2;
	val |= val >> 4;
	val |= val >> 8;
	val |= val >> 16;
	return ++val;
}

int pfl_stream_listen(struct sock *sock, int backlog)
{
	struct sock_pfl_stream *pfl_stream = sock->userdata;
	if (backlog <= 0)
		return -EINVAL;
	if (!pfl_stream->pfls)
		return -EINVAL;
	mutex_lock(&pfl_stream->pfls->mutex);
	if (pfl_stream->pfls->listen)
	{
		mutex_unlock(&pfl_stream->pfls->mutex);
		return -EADDRINUSE;
	}
	pfl_stream->server.queue_size = npot32(backlog);
	if (pfl_stream->server.queue_size > 1024) /* XXX less arbitrary ? */
		pfl_stream->server.queue_size = 1024;
	pfl_stream->server.queue = malloc(pfl_stream->server.queue_size, 0);
	if (!pfl_stream->server.queue)
	{
		mutex_unlock(&pfl_stream->pfls->mutex);
		return -ENOMEM;
	}
	pfl_stream->server.queue_len = 0;
	pfl_stream->pfls->listen = sock;
	sock->state = SOCK_ST_LISTENING;
	mutex_unlock(&pfl_stream->pfls->mutex);
	return 0;
}

int pfl_stream_release(struct sock *sock)
{
	struct sock_pfl_stream *pfl_stream = sock->userdata;
	/* XXX sock lock */
	if (sock->state == SOCK_ST_LISTENING)
	{
		mutex_lock(&pfl_stream->pfls->mutex);
		pfl_stream->pfls->listen = NULL;
		mutex_unlock(&pfl_stream->pfls->mutex);
		for (size_t i = 0; i < pfl_stream->server.queue_len; ++i)
		{
			waitq_signal(&PAIR_INPUT(pfl_stream->server.queue[i])->wwaitq,
			             -ECONNREFUSED);
		}
		free(pfl_stream->server.queue);
		sma_free(&sock_pfl_stream_sma, pfl_stream);
		pfls_free(pfl_stream->pfls);
		return 0;
	}
	if (sock->state != SOCK_ST_CONNECTED)
	{
		sma_free(&sock_pfl_stream_sma, pfl_stream);
		pfls_free(pfl_stream->pfls);
		return 0;
	}

	pipebuf_lock(&PAIR_INPUT(pfl_stream)->pipebuf);
	PAIR_INPUT(pfl_stream)->pipebuf.nreaders--;
	waitq_broadcast(&PAIR_INPUT(pfl_stream)->wwaitq, 0);
	pipebuf_unlock(&PAIR_INPUT(pfl_stream)->pipebuf);

	pipebuf_lock(&PAIR_OUTPUT(pfl_stream)->pipebuf);
	PAIR_OUTPUT(pfl_stream)->pipebuf.nwriters--;
	waitq_broadcast(&PAIR_OUTPUT(pfl_stream)->rwaitq, 0);
	pipebuf_unlock(&PAIR_OUTPUT(pfl_stream)->pipebuf);

	if (pfl_stream->client.peer)
	{
		poller_broadcast(&pfl_stream->client.peer->sock->poll_entries, POLLHUP);
		pfl_stream->client.peer->client.peer = NULL; /* XXX racy, that's why a better state machine is needed */
	}
	pair_free(pfl_stream->client.pair);
	sma_free(&sock_pfl_stream_sma, pfl_stream);
	pfls_free(pfl_stream->pfls);
	return 0;
}

int pfl_stream_shutdown(struct sock *sock, int how)
{
	(void)sock;
	(void)how;
	/* XXX */
	return 0;
}

static const struct sock_op pfl_stream_op =
{
	.release = pfl_stream_release,
	.bind = pfl_stream_bind,
	.accept = pfl_stream_accept,
	.connect = pfl_stream_connect,
	.listen = pfl_stream_listen,
	.ioctl = pfl_stream_ioctl,
	.recv = pfl_stream_recv,
	.send = pfl_stream_send,
	.poll = pfl_stream_poll,
	.shutdown = pfl_stream_shutdown,
};

int pfl_stream_open(int domain, int type, int protocol, struct sock **sock)
{
	struct sock_pfl_stream *pfl_stream;
	int ret;

	if (domain != AF_LOCAL
	 || type != SOCK_STREAM
	 || protocol)
		return -EINVAL;
	pfl_stream = sma_alloc(&sock_pfl_stream_sma, M_ZERO);
	if (!pfl_stream)
		return -ENOMEM;
	ret = sock_new(domain, type, protocol, &pfl_stream_op, sock);
	if (ret)
	{
		sma_free(&sock_pfl_stream_sma, pfl_stream);
		return ret;
	}
	pfl_stream->sock = *sock;
	(*sock)->userdata = pfl_stream;
	return 0;
}

int pfl_stream_openpair(int domain, int type, int protocol, struct sock **socksp)
{
	struct sock_pfl_stream *pfl_streams[2];
	struct sock_pfl_stream_pair *pair;
	struct sock *socks[2] = {NULL, NULL};
	int ret;

	if (domain != AF_LOCAL
	 || type != SOCK_STREAM
	 || protocol)
		return -EINVAL;
	ret = pair_alloc(&pair);
	if (ret)
		return ret;
	ret = pfl_stream_open(domain, type, protocol, &socks[0]);
	if (ret)
		goto err;
	ret = pfl_stream_open(domain, type, protocol, &socks[1]);
	if (ret)
		goto err;
	for (size_t i = 0; i < 2; ++i)
	{
		pfl_streams[i] = socks[i]->userdata;
		pfl_streams[i]->client.pair = pair;
		pfl_streams[i]->client.pair_idx = i;
		pfl_streams[i]->client.peer = socks[(i + 1) & 1]->userdata;
		pair->data[i].pipebuf.nreaders = 1;
		pair->data[i].pipebuf.nwriters = 1;
		socks[i]->state = SOCK_ST_CONNECTED;
		socksp[i] = socks[i];
	}
	refcount_inc(&pair->refcount);
	return 0;

err:
	sock_free(socks[0]);
	sock_free(socks[1]);
	pair_free(pair);
	return ret;
}
