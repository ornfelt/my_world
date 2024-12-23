#include <jkssl/bignum.h>
#include <jkssl/dsa.h>

int dsa_generate_key(struct dsa_ctx *ctx)
{
	struct bignum_ctx *bn_ctx;

	bignum_free(ctx->x);
	bignum_free(ctx->y);
	bn_ctx = bignum_ctx_new();
	ctx->x = bignum_new();
	ctx->y = bignum_new();
	if (!bn_ctx
	 || !ctx->x
	 || !ctx->y
	 || !bignum_rand_range(ctx->x, ctx->q, BIGNUM_RAND_TOP_TWO,
	                       BIGNUM_RAND_BOT_ODD)
	 || !bignum_umod_exp(ctx->y, ctx->g, ctx->x, ctx->p, bn_ctx))
		goto err;
	bignum_ctx_free(bn_ctx);
	return 1;

err:
	bignum_free(ctx->x);
	ctx->x = NULL;
	bignum_free(ctx->y);
	ctx->y = NULL;
	bignum_ctx_free(bn_ctx);
	return 0;
}
