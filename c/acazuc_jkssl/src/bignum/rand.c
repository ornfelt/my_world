#include "bignum/bignum.h"

#include <jkssl/rand.h>

static void do_top_bottom(struct bignum *bignum, uint32_t bits, int top,
                          int bottom)
{
	if (bottom == BIGNUM_RAND_BOT_ODD)
		bignum->data[0] |= 1;
	if (top == BIGNUM_RAND_TOP_TWO || top == BIGNUM_RAND_TOP_ONE)
		bignum->data[(bits - 1) / 8 / sizeof(*bignum->data)] |= bignum_word_one() << ((bits - 1) % (8 * sizeof(*bignum->data)));
	if (top == BIGNUM_RAND_TOP_TWO && bits > 1)
		bignum->data[(bits - 2) / 8 / sizeof(*bignum->data)] |= bignum_word_one() << ((bits - 2) % (8 * sizeof(*bignum->data)));
}

int bignum_rand(struct bignum *bignum, uint32_t bits, int top,
                int bottom)
{
	size_t nwords = (bits + BIGNUM_BITS_PER_WORD - 1) / BIGNUM_BITS_PER_WORD;

	if (!bits)
	{
		__bignum_zero(bignum);
		return 1;
	}
	if (!bignum_resize(bignum, nwords))
		return 0;
	if (rand_bytes(bignum->data, nwords * sizeof(*bignum->data)) != 1)
		return 0;
	if (bits % BIGNUM_BITS_PER_WORD)
		bignum->data[bits / BIGNUM_BITS_PER_WORD] &= ((bignum_word_one() << (bits % BIGNUM_BITS_PER_WORD)) - 1);
	do_top_bottom(bignum, bits, top, bottom);
	return 1;
}

int bignum_rand_range(struct bignum *bignum, const struct bignum *range,
                      int top, int bottom)
{
	struct bignum_ctx *ctx;

	if (!bignum_rand(bignum, range->len * 8 * sizeof(*bignum->data), top,
	                 bottom))
		return 0;
	ctx = bignum_ctx_new();
	if (!ctx)
		return 0;
	/* XXX this modulo makes invalid random because
	 * the cut part is not a multiple of the range
	 */
	if (!bignum_mod(bignum, bignum, range, ctx))
	{
		bignum_ctx_free(ctx);
		return 0;
	}
	bignum_ctx_free(ctx);
	bignum_trunc(bignum);
	return 1;
}
