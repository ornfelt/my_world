#include "bignum/bignum.h"

#include <string.h>

static int do_part(struct bignum *tmp, struct bignum *q, struct bignum *v,
                   struct bignum *old_v, struct bignum_ctx *ctx)
{
	return bignum_mul(tmp, q, v, ctx)
	    && bignum_sub(tmp, old_v, tmp, ctx)
	    && bignum_copy(old_v, v)
	    && bignum_copy(v, tmp);
}

int bignum_mod_inverse(struct bignum *res, const struct bignum *a,
                       const struct bignum *b, struct bignum_ctx *ctx)
{
	struct bignum *q;
	struct bignum *r;
	struct bignum *s;
	struct bignum *old_r;
	struct bignum *old_s;
	struct bignum *tmp;
	int ret = 0;

	q = bignum_ctx_get(ctx);
	r = bignum_ctx_get(ctx);
	s = bignum_ctx_get(ctx);
	old_r = bignum_ctx_get(ctx);
	old_s = bignum_ctx_get(ctx);
	tmp = bignum_ctx_get(ctx);
	if (!q
	 || !r
	 || !s
	 || !old_r
	 || !old_s
	 || !tmp
	 || !bignum_copy(r, b)
	 || !bignum_copy(old_r, a)
	 || !__bignum_one(old_s))
		goto end;
	while (!__bignum_is_zero(r))
	{
		if (!bignum_div(q, old_r, r, ctx)
		 || !do_part(tmp, q, r, old_r, ctx)
		 || !do_part(tmp, q, s, old_s, ctx))
			goto end;
	}
	if (__bignum_is_negative(old_s)
	 && !bignum_add(old_s, old_s, s, ctx))
		goto end;
	bignum_trunc(old_s);
	bignum_swap(res, old_s);
	ret = 1;

end:
	bignum_ctx_release(ctx, q);
	bignum_ctx_release(ctx, r);
	bignum_ctx_release(ctx, s);
	bignum_ctx_release(ctx, old_r);
	bignum_ctx_release(ctx, old_s);
	bignum_ctx_release(ctx, tmp);
	return ret;
}
