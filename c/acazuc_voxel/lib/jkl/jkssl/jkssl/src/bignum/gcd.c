#include <jkssl/bignum.h>

int bignum_gcd(struct bignum *r, const struct bignum *a,
               const struct bignum *b, struct bignum_ctx *ctx)
{
	struct bignum *tmp1;
	struct bignum *tmp2;
	struct bignum *tmp3;
	int ret = 0;

	tmp1 = bignum_ctx_get(ctx);
	tmp2 = bignum_ctx_get(ctx);
	tmp3 = bignum_ctx_get(ctx);
	if (!bignum_copy(tmp1, a))
		goto end;
	if (!bignum_copy(tmp2, b))
		goto end;
	while (!bignum_is_zero(tmp2))
	{
		if (!bignum_mod(tmp3, tmp1, tmp2, ctx))
			goto end;
		if (!bignum_copy(tmp1, tmp2))
			goto end;
		if (!bignum_copy(tmp2, tmp3))
			goto end;
	}
	bignum_trunc(tmp1);
	bignum_swap(r, tmp1);
	ret = 1;

end:
	bignum_ctx_release(ctx, tmp1);
	bignum_ctx_release(ctx, tmp2);
	bignum_ctx_release(ctx, tmp3);
	return ret;
}
