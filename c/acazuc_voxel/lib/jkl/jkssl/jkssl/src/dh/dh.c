#include <jkssl/bignum.h>
#include <jkssl/dh.h>

#include <stdlib.h>

struct dh_ctx *dh_new(void)
{
	struct dh_ctx *ctx = calloc(sizeof(*ctx), 1);
	if (!ctx)
		return NULL;
	refcount_init(&ctx->refcount, 1);
	return ctx;
}

void dh_free(struct dh_ctx *ctx)
{
	if (!ctx)
		return;
	if (refcount_dec(&ctx->refcount))
		return;
	bignum_free(ctx->p);
	bignum_free(ctx->q);
	bignum_free(ctx->g);
	bignum_free(ctx->x);
	bignum_free(ctx->y);
	free(ctx);
}

int dh_up_ref(struct dh_ctx *ctx)
{
	return refcount_inc(&ctx->refcount);
}

int dh_generate_parameters(struct dh_ctx *ctx, uint32_t bits,
                           size_t generator, struct bignum_gencb *gencb)
{
	struct bignum_ctx *bn_ctx;

	if (generator <= 1)
		return 0;
	bignum_free(ctx->p);
	bignum_free(ctx->q);
	bignum_free(ctx->g);
	ctx->p = bignum_new();
	ctx->q = NULL;
	ctx->g = bignum_new();
	bn_ctx = bignum_ctx_new();
	if (!ctx->p
	 || !ctx->g
	 || !bn_ctx
	 || !bignum_set_word(ctx->g, generator)
	 || bignum_num_bits(ctx->g) >= (int)bits
	 || !bignum_generate_prime(ctx->p, bits, 0, NULL, NULL, gencb, bn_ctx))
		goto err;
	bignum_ctx_free(bn_ctx);
	return 1;

err:
	bignum_free(ctx->p);
	ctx->p = NULL;
	bignum_free(ctx->g);
	ctx->g = NULL;
	bignum_ctx_free(bn_ctx);
	return 0;
}

int dh_generate_key(struct dh_ctx *ctx)
{
	return 0;
}

ssize_t dh_size(struct dh_ctx *ctx)
{
	ssize_t bits = dh_bits(ctx);
	if (bits == -1)
		return bits;
	return bits / 8;
}

ssize_t dh_bits(struct dh_ctx *ctx)
{
	if (!ctx || !ctx->p)
		return -1;
	return bignum_num_bits(ctx->p);
}
