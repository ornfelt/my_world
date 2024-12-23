#include "evp/md.h"

#include <strings.h>
#include <stdlib.h>

struct evp_md_ctx *evp_md_ctx_new(void)
{
	return calloc(sizeof(struct evp_md_ctx), 1);
}

void evp_md_ctx_free(struct evp_md_ctx *ctx)
{
	if (!ctx)
		return;
	free(ctx->ctx);
	free(ctx);
}

int evp_md_ctx_reset(struct evp_md_ctx *ctx)
{
	if (!ctx->ctx)
		return 0;
	return ctx->evp_md->init(ctx->ctx);
}

int evp_digest_init(struct evp_md_ctx *ctx, const struct evp_md *evp_md)
{
	if (ctx->evp_md != evp_md)
	{
		void *new_ctx = malloc(evp_md->ctx_size);
		if (!new_ctx)
			return 0;
		ctx->evp_md = evp_md;
		free(ctx->ctx);
		ctx->ctx = new_ctx;
	}
	return ctx->evp_md->init(ctx->ctx);
}

int evp_digest_update(struct evp_md_ctx *ctx, const uint8_t *data, size_t size)
{
	return ctx->evp_md->update(ctx->ctx, data, size);
}

int evp_digest_final(struct evp_md_ctx *ctx, uint8_t *md)
{
	return ctx->evp_md->final(md, ctx->ctx);
}

int evp_digest(const void *data, size_t count, uint8_t *md,
               unsigned *size, const struct evp_md *evp_md)
{
	struct evp_md_ctx *ctx = evp_md_ctx_new();
	if (!ctx)
		return 0;
	if (!evp_digest_init(ctx, evp_md)
	 || !evp_digest_update(ctx, data, count)
	 || !evp_digest_final(ctx, md))
	{
		evp_md_ctx_free(ctx);
		return 0;
	}
	if (size)
		*size = evp_md->digest_size;
	evp_md_ctx_free(ctx);
	return 1;
}

void evp_foreach_digest(int (*cb)(const struct evp_md *md, void *data),
                        void *data)
{
#define DIGEST_FOREACH(c) \
do \
{ \
	if (!cb(evp_##c(), data)) \
		return; \
} while (0)

	DIGEST_FOREACH(adler32);
	DIGEST_FOREACH(crc32);
	DIGEST_FOREACH(md2);
	DIGEST_FOREACH(md4);
	DIGEST_FOREACH(md5);
	DIGEST_FOREACH(ripemd128);
	DIGEST_FOREACH(ripemd160);
	DIGEST_FOREACH(ripemd256);
	DIGEST_FOREACH(ripemd320);
	DIGEST_FOREACH(sha0);
	DIGEST_FOREACH(sha1);
	DIGEST_FOREACH(sha224);
	DIGEST_FOREACH(sha256);
	DIGEST_FOREACH(sha384);
	DIGEST_FOREACH(sha512);
	DIGEST_FOREACH(sm3);

#undef DIGEST_FOREACH
}

struct digest_get_state
{
	const char *name;
	const struct evp_md *md;
};

static int digest_get_foreach(const struct evp_md *md, void *data)
{
	struct digest_get_state *state = data;
	if (!strcasecmp(md->name, state->name))
	{
		state->md = md;
		return 0;
	}
	return 1;
}

const struct evp_md *evp_get_digestbyname(const char *name)
{
	struct digest_get_state state;
	state.name = name;
	state.md = NULL;
	evp_foreach_digest(digest_get_foreach, &state);
	return state.md;
}

size_t evp_md_get_size(const struct evp_md *evp_md)
{
	return evp_md->digest_size;
}

size_t evp_md_get_block_size(const struct evp_md *evp_md)
{
	return evp_md->block_size;
}

const char *evp_md_get0_name(const struct evp_md *evp_md)
{
	return evp_md->name;
}
