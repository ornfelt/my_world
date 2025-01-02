#include "ssl/ssl.h"

#include <jkssl/bio.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

struct ssl *ssl_new(struct ssl_ctx *ctx)
{
	struct ssl *ssl = calloc(sizeof(*ssl), 1);
	if (!ssl)
		return NULL;
	refcount_init(&ssl->refcount, 1);
	ssl->state = SSL_ST_NONE;
	ssl->ctx = ctx;
	ssl_ctx_up_ref(ctx);
	return ssl;
}

void ssl_free(struct ssl *ssl)
{
	if (!ssl)
		return;
	if (refcount_dec(&ssl->refcount))
		return;
	ssl_ctx_free(ssl->ctx);
	free(ssl);
}

int ssl_up_ref(struct ssl *ssl)
{
	return refcount_inc(&ssl->refcount);
}

static int send_client_hello(struct ssl *ssl)
{
	struct bio *bio = bio_new(bio_s_mem());
	struct buf_mem *mem;
	int ret = 0;

	if (!bio)
		goto end;
	if (!ssl_write_client_hello(ssl, bio))
		goto end;
	bio_get_mem_ptr(bio, &mem);
	if (bio_write(ssl->ctx->bio, mem->data, mem->size) != (ssize_t)mem->size)
		goto end;
	ret = 1;

end:
	if (bio)
		bio_vfree(bio);
	return ret;
}

int ssl_connect(struct ssl *ssl)
{
	switch (ssl->state)
	{
		case SSL_ST_CONNECT:
			break;
		case SSL_ST_NONE:
			ssl->state = SSL_ST_CONNECT;
			if (!bio_do_connect(ssl->ctx->bio))
				return 0;
			ssl->state = SSL_ST_CLIENT_HELLO;
			/* FALLTHROUGH */
		case SSL_ST_CLIENT_HELLO:
			if (!send_client_hello(ssl))
				return 0;
			break;
	}
	return 1;
}

const struct ssl_method *sslv23_method(void)
{
	/* XXX */
	return NULL;
}
