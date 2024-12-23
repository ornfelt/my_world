#include "ssl/ssl.h"

#include <stdlib.h>

struct ssl_ctx *ssl_ctx_new(const struct ssl_method *method)
{
	(void)method; /* XXX */
	struct ssl_ctx *ctx = calloc(1, sizeof(*ctx));
	if (!ctx)
		return NULL;
	refcount_init(&ctx->refcount, 1);
	return ctx;
}

void ssl_ctx_free(struct ssl_ctx *ctx)
{
	if (!ctx)
		return;
	if (refcount_dec(&ctx->refcount))
		return;
	free(ctx);
}

int ssl_ctx_up_ref(struct ssl_ctx *ctx)
{
	return refcount_inc(&ctx->refcount);
}

int ssl_ctx_generate_layer_record(struct bio *bio, const void *data, size_t size)
{
	if (size >= (1 << 14))
		return 0;
	if (!ssl_write_u8(bio, HANDSHAKE))
		goto err;
	if (!ssl_write_u16(bio, TLS_VERSION_1_2))
		goto err;
	if (!ssl_write_u16(bio, size))
		goto err;
	if (bio_write(bio, data, size) != (ssize_t)size)
		goto err;
	return 1;

err:
	return 0;
}
