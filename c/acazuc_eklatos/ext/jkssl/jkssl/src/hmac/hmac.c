#include <jkssl/hmac.h>
#include <jkssl/evp.h>

#include <string.h>
#include <stdlib.h>

struct hmac_ctx
{
	struct evp_md_ctx *evp_md_ctx;
	const struct evp_md *evp_md;
	uint8_t *pads[2];
	uint8_t result[EVP_MAX_MD_SIZE];
};

static int create_pads(struct hmac_ctx *ctx, const uint8_t *key, size_t key_len,
                       const struct evp_md *evp_md)
{
	size_t block_size;
	uint8_t *kp;
	int ret = 0;

	block_size = evp_md_get_block_size(evp_md);
	kp = malloc(block_size);
	if (!kp)
		return 0;
	if (key_len > block_size)
	{
		uint8_t tmp[EVP_MAX_MD_SIZE];
		unsigned len;
		if (!evp_digest(key, key_len, tmp, &len, evp_md))
			goto end;
		if (len >= block_size)
		{
			memcpy(kp, tmp, block_size);
		}
		else
		{
			memcpy(kp, tmp, len);
			memset(kp + len, 0, block_size - len);
		}
	}
	else
	{
		memcpy(kp, key, key_len);
		memset(kp + key_len, 0, block_size - key_len);
	}
	free(ctx->pads[0]);
	ctx->pads[0] = malloc(block_size);
	if (!ctx->pads[0])
		goto end;
	free(ctx->pads[1]);
	ctx->pads[1] = malloc(block_size);
	if (!ctx->pads[1])
	{
		free(ctx->pads[0]);
		ctx->pads[0] = NULL;
		goto end;
	}
	for (size_t i = 0; i < block_size; ++i)
	{
		ctx->pads[0][i] = kp[i] ^ 0x5C;
		ctx->pads[1][i] = kp[i] ^ 0x36;
	}
	ret = 1;

end:
	free(kp);
	return ret;
}

struct hmac_ctx *hmac_ctx_new(void)
{
	return calloc(1, sizeof(struct hmac_ctx));
}

void hmac_ctx_free(struct hmac_ctx *ctx)
{
	if (!ctx)
		return;
	evp_md_ctx_free(ctx->evp_md_ctx);
	free(ctx->pads[0]);
	free(ctx->pads[1]);
	free(ctx);
}

int hmac_init(struct hmac_ctx *ctx, const void *key, size_t key_len,
              const struct evp_md *evp_md)
{
	ctx->evp_md = evp_md;
	if (!create_pads(ctx, key, key_len, evp_md))
		return 0;
	evp_md_ctx_free(ctx->evp_md_ctx);
	ctx->evp_md_ctx = evp_md_ctx_new();
	if (!ctx->evp_md_ctx
	 || !evp_digest_init(ctx->evp_md_ctx, evp_md)
	 || !evp_digest_update(ctx->evp_md_ctx, ctx->pads[1],
	                       evp_md_get_block_size(ctx->evp_md)))
		return 0;
	return 1;
}

int hmac_update(struct hmac_ctx *ctx, const uint8_t *data, size_t len)
{
	return evp_digest_update(ctx->evp_md_ctx, data, len);
}

int hmac_final(struct hmac_ctx *ctx, uint8_t *md, unsigned *len)
{
	if (!evp_digest_final(ctx->evp_md_ctx, ctx->result)
	 || !evp_md_ctx_reset(ctx->evp_md_ctx)
	 || !evp_digest_update(ctx->evp_md_ctx, ctx->pads[0],
	                       evp_md_get_block_size(ctx->evp_md))
	 || !evp_digest_update(ctx->evp_md_ctx, ctx->result,
	                       evp_md_get_size(ctx->evp_md))
	 || !evp_digest_final(ctx->evp_md_ctx, md))
		return 0;
	if (len)
		*len = evp_md_get_size(ctx->evp_md);
	return 1;
}

uint8_t *hmac(const struct evp_md *evp_md, const void *key,
              size_t key_len, const uint8_t *d, size_t n, uint8_t *md,
              unsigned *md_len)
{
	static uint8_t result[EVP_MAX_MD_SIZE];
	if (!md)
		md = result;
	struct hmac_ctx *ctx = hmac_ctx_new();
	if (!ctx)
		return NULL;
	if (!hmac_init(ctx, key, key_len, evp_md)
	 || !hmac_update(ctx, d, n)
	 || !hmac_final(ctx, md, md_len))
	{
		hmac_ctx_free(ctx);
		return NULL;
	}
	hmac_ctx_free(ctx);
	return md;
}

size_t hmac_size(const struct hmac_ctx *ctx)
{
	return evp_md_get_size(ctx->evp_md);
}
