#include "bio/bio.h"
#include "ssl/ssl.h"

#include <stdlib.h>
#include <string.h>

struct bio_ssl
{
	struct ssl_ctx *ssl_ctx;
	struct ssl *ssl;
	char *hostname;
	int client;
};

struct bio *bio_new_ssl(struct ssl_ctx *ctx, int client)
{
	struct bio *bio = bio_new(bio_s_ssl());
	if (!bio)
		return NULL;
	struct bio_ssl *bio_ssl = bio->ctx;
	bio_ssl->ssl_ctx = ctx;
	ctx->bio = bio;
	bio_ssl->client = client;
	bio_ssl->ssl = ssl_new(ctx);
	if (!bio_ssl->ssl)
	{
		bio_vfree(bio);
		return NULL;
	}
	return bio;
}

struct bio *bio_new_ssl_connect(struct ssl_ctx *ctx)
{
	struct bio *bio_ssl = bio_new_ssl(ctx, 1);
	if (!bio_ssl)
		return NULL;
	struct bio *bio_tcp = bio_new(bio_s_connect());
	if (!bio_tcp)
	{
		bio_free(bio_ssl);
		return NULL;
	}
	bio_push(bio_ssl, bio_tcp);
	return bio_ssl;
}

static void bio_ssl_dtr(struct bio *bio)
{
	struct bio_ssl *bio_ssl = bio->ctx;
	ssl_ctx_free(bio_ssl->ssl_ctx);
	ssl_free(bio_ssl->ssl);
	free(bio_ssl->hostname);
}

static long bio_ssl_ctrl(struct bio *bio, enum bio_ctrl cmd, long arg1,
                         void *arg2)
{
	(void)arg1;
	struct bio_ssl *bio_ssl = bio->ctx;
	switch (cmd)
	{
		case BIO_CTRL_RESET:
			/* XXX shutdown */
			return -1;
		case BIO_CTRL_SET_CONN_HOSTNAME:
			if (bio->next && !bio_set_conn_hostname(bio->next, arg2))
				return -1;
			free(bio_ssl->hostname);
			bio_ssl->hostname = strdup(arg2);
			if (!bio_ssl->hostname)
				return -1;
			return 1;
		case BIO_CTRL_CONNECT:
			if (bio->next && !bio_do_connect(bio->next))
				return -1;
			return ssl_connect(bio_ssl->ssl);
		case BIO_CTRL_GET_SSL:
			*(struct ssl**)arg2 = bio_ssl->ssl;
			return 1;
		default:
			return -1;
	}
}

static ssize_t bio_ssl_read(struct bio *bio, void *data, size_t size)
{
	/* XXX */
	(void)bio;
	(void)data;
	(void)size;
	return -1;
}

static ssize_t bio_ssl_write(struct bio *bio, const void *data, size_t size)
{
	struct bio *msg;
	struct buf_mem *mem;
	int ret = 0;

	if (!bio->next)
		return 0;
	msg = bio_new(bio_s_mem());
	if (!msg)
		goto end;
	if (!ssl_ctx_generate_layer_record(msg, data, size))
		goto end;
	bio_get_mem_ptr(msg, &mem);
	if (bio_write(bio->next, mem->data, mem->size) != (ssize_t)mem->size)
		goto end;
	ret = size;

end:
	if (msg)
		bio_vfree(msg);
	return ret;
}

static ssize_t bio_ssl_gets(struct bio *bio, char *data, size_t size)
{
	/* XXX */
	(void)bio;
	(void)data;
	(void)size;
	return -1;
}

static const struct bio_method g_bio_s_ssl =
{
	.ctx_size = sizeof(struct bio_ssl),
	.op_dtr = bio_ssl_dtr,
	.op_ctrl = bio_ssl_ctrl,
	.op_read = bio_ssl_read,
	.op_write = bio_ssl_write,
	.op_gets = bio_ssl_gets,
};

const struct bio_method *bio_s_ssl(void)
{
	return &g_bio_s_ssl;
}
