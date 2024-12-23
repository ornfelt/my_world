#include <jkssl/bignum.h>

int bignum_nnmod(struct bignum *r, const struct bignum *a,
                 const struct bignum *m, struct bignum_ctx *ctx)
{
	if (!bignum_mod(r, a, m, ctx))
		return 0;
	if (bignum_is_negative(r)
	 && !bignum_add(r, r, m, ctx))
		return 0;
	return 1;
}

int bignum_mod_add(struct bignum *r, const struct bignum *a,
                   const struct bignum *b, const struct bignum *m,
                   struct bignum_ctx *ctx)
{
	return bignum_add(r, a, b, ctx)
	    && bignum_nnmod(r, r, m, ctx);
}

int bignum_mod_sub(struct bignum *r, const struct bignum *a,
                   const struct bignum *b, const struct bignum *m,
                   struct bignum_ctx *ctx)
{
	return bignum_sub(r, a, b, ctx)
	    && bignum_nnmod(r, r, m, ctx);
}

int bignum_mod_mul(struct bignum *r, const struct bignum *a,
                   const struct bignum *b, const struct bignum *m,
                   struct bignum_ctx *ctx)
{
	return bignum_mul(r, a, b, ctx)
	    && bignum_nnmod(r, r, m, ctx);
}

int bignum_mod_sqr(struct bignum *r, const struct bignum *a,
                   const struct bignum *m, struct bignum_ctx *ctx)
{
	return bignum_sqr(r, a, ctx)
	    && bignum_nnmod(r, r, m, ctx);
}

int bignum_mod_word(bignum_word_t *r, const struct bignum *a,
                    bignum_word_t b)
{
	bignum_dword_t ret;
	bignum_dword_t tmp;
	uint32_t i;

	if (!b)
		return 0;
	if (bignum_is_zero(a))
	{
		*r = 0;
		return 1;
	}
	ret = 0;
	i = a->len - 1;
	tmp = ((bignum_dword_t)1 << (sizeof(*a->data) * 8)) % b;
	while (1)
	{
		ret = (ret * tmp + a->data[i] % b) % b;
		if (!i)
			break;
		--i;
	}
	*r = ret;
	return 1;
}
