#include "adler32/adler32.h"
#include "utils/utils.h"

int adler32_init(struct adler32_ctx *ctx)
{
	ctx->v1 = 1;
	ctx->v2 = 0;
	return 1;
}

int adler32_update(struct adler32_ctx *ctx, const void *data, size_t len)
{
	for (size_t i = 0; i < len; ++i)
	{
		ctx->v1 += ((uint8_t*)data)[i];
		ctx->v2 += ctx->v1;
	}
	return 1;
}

int adler32_final(uint8_t *md, struct adler32_ctx *ctx)
{
	*(uint32_t*)md = bswap32((ctx->v2 << 16) | ctx->v1);
	return 1;
}

#if 0
int adler32(const uint8_t *data, size_t len, uint8_t *md)
{
	struct adler32_ctx ctx;
	if (!adler32_init(&ctx)
	 || !adler32_update(&ctx, data, len)
	 || !adler32_final(md, &ctx))
		return 0;
	return 1;
}
#endif
