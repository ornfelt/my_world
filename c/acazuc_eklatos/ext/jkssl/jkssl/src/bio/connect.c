#include "bio/bio.h"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct bio_connect
{
	char *hostname;
	int fd;
};

struct bio *bio_new_connect(const char *name)
{
	struct bio *bio = bio_new(bio_s_connect());
	if (!bio)
		return NULL;
	if (!bio_set_conn_hostname(bio, name))
	{
		bio_free(bio);
		return NULL;
	}
	return bio;
}

static int bio_connect_ctr(struct bio *bio)
{
	struct bio_connect *bio_connect = bio->ctx;
	bio_connect->fd = -1;
	return 1;
}

static void bio_connect_dtr(struct bio *bio)
{
	struct bio_connect *bio_connect = bio->ctx;
	if (bio_connect->fd != -1)
		close(bio_connect->fd);
	free(bio_connect->hostname);
}

static int do_connect(struct bio *bio)
{
	struct bio_connect *bio_connect = bio->ctx;
	struct addrinfo *addrs = NULL;
	char *node = NULL;
	char *service = NULL;
	char *sc;
	int ret = 0;
	int res;

	if (!bio_connect->hostname)
		goto end;
	sc = strchr(bio_connect->hostname, ':');
	if (sc)
	{
		if (sc != bio_connect->hostname)
		{
			node = malloc(sc - bio_connect->hostname + 1);
			if (!node)
				goto end;
			memcpy(node, bio_connect->hostname, sc - bio_connect->hostname);
			node[sc - bio_connect->hostname] = '\0';
		}
		else if (!sc[1])
		{
			goto end;
		}
		if (sc[1])
		{
			service = strdup(&sc[1]);
			if (!service)
				goto end;
		}
	}
	else
	{
		node = strdup(bio_connect->hostname);
		if (!node)
			goto end;
		service = strdup("https");
		if (!service)
			goto end;
	}
	bio_connect->fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (bio_connect->fd == -1)
		goto end;
	res = getaddrinfo(node, service, NULL, &addrs);
	if (res)
		goto end;
	if (connect(bio_connect->fd, addrs->ai_addr, addrs->ai_addrlen) == -1)
		goto end;
	ret = 1;

end:
	if (!ret)
	{
		if (bio_connect->fd != -1)
		{
			close(bio_connect->fd);
			bio_connect->fd = -1;
		}
	}
	freeaddrinfo(addrs);
	free(node);
	free(service);
	return ret;
}

static long bio_connect_ctrl(struct bio *bio, enum bio_ctrl cmd, long arg1,
                             void *arg2)
{
	(void)arg1;
	struct bio_connect *bio_connect = bio->ctx;
	switch (cmd)
	{
		case BIO_CTRL_RESET:
			/* XXX shutdown */
			break;
		case BIO_CTRL_SET_CONN_HOSTNAME:
			free(bio_connect->hostname);
			bio_connect->hostname = strdup(arg2);
			if (!bio_connect->hostname)
				return -1;
			break;
		case BIO_CTRL_CONNECT:
			if (bio_connect->fd != -1)
				return 1;
			return do_connect(bio);
		default:
			return -1;
	}
	return 0;
}

static ssize_t bio_connect_read(struct bio *bio, void *data, size_t size)
{
	struct bio_connect *bio_connect = bio->ctx;
	return recv(bio_connect->fd, data, size, 0);
}

static ssize_t bio_connect_write(struct bio *bio, const void *data, size_t size)
{
	struct bio_connect *bio_connect = bio->ctx;
	return send(bio_connect->fd, data, size, 0);
}

static ssize_t bio_connect_gets(struct bio *bio, char *data, size_t size)
{
	(void)bio;
	(void)data;
	(void)size;
	return -2;
}

static const struct bio_method g_bio_s_connect =
{
	.ctx_size = sizeof(struct bio_connect),
	.op_ctr = bio_connect_ctr,
	.op_dtr = bio_connect_dtr,
	.op_ctrl = bio_connect_ctrl,
	.op_read = bio_connect_read,
	.op_write = bio_connect_write,
	.op_gets = bio_connect_gets,
};

const struct bio_method *bio_s_connect(void)
{
	return &g_bio_s_connect;
}
