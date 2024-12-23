#include "utils/utils.h"

#include <jkssl/bignum.h>
#include <jkssl/dsa.h>
#include <jkssl/der.h>

#include <stdlib.h>
#include <string.h>

int dsa_sign(struct dsa_ctx *ctx, const uint8_t *tbs, size_t tbslen,
             uint8_t *sig, size_t *siglen)
{
	struct dsa_sig dsa_sig;
	struct bignum_ctx *bn_ctx;
	struct bignum kinv;
	struct bignum qmt;
	struct bignum k;
	struct bignum r;
	struct bignum s;
	struct bignum m;
	size_t qbytes;
	uint8_t *sigbytes = NULL;
	int ret = 0;

	bignum_init(&kinv);
	bignum_init(&qmt);
	bignum_init(&k);
	bignum_init(&r);
	bignum_init(&s);
	bignum_init(&m);
	bn_ctx = bignum_ctx_new();
	qbytes = bignum_num_bytes(ctx->q);
	if (tbslen > qbytes)
		tbslen = qbytes;
	if (!bn_ctx
	 || !bignum_bin2bignum(tbs, tbslen, &m)
	 || !bignum_set_word(&qmt, 2)
	 || !bignum_sub(&qmt, ctx->q, &qmt, bn_ctx))
		goto end;
	while (1)
	{
		if (!bignum_rand_range(&k, ctx->q, BIGNUM_RAND_TOP_TWO, BIGNUM_RAND_BOT_ODD)
		 || !bignum_mod_exp(&r, ctx->g, &k, ctx->p, bn_ctx)
		 || !bignum_mod(&r, &r, ctx->q, bn_ctx))
			goto end;
		if (bignum_is_zero(&r))
			continue;
		if (!bignum_mod_exp(&kinv, &k, &qmt, ctx->q, bn_ctx)
		 || !bignum_mul(&s, &r, ctx->x, bn_ctx)
		 || !bignum_add(&s, &s, &m, bn_ctx)
		 || !bignum_mod_mul(&s, &s, &kinv, ctx->q, bn_ctx))
			goto end;
		if (bignum_is_zero(&s))
			continue;
		break;
	}
	dsa_sig.r = &r;
	dsa_sig.s = &s;
	ret = i2d_dsa_sig(&dsa_sig, &sigbytes);
	if (ret <= 0)
		goto end;
	memcpy(sig, sigbytes, ret);
	*siglen = ret;
	ret = 1;

end:
	bignum_clear(&kinv);
	bignum_clear(&qmt);
	bignum_clear(&k);
	bignum_clear(&r);
	bignum_clear(&s);
	bignum_clear(&m);
	bignum_ctx_free(bn_ctx);
	free(sigbytes);
	return ret;
}

int dsa_verify(struct dsa_ctx *ctx, const uint8_t *tbs, size_t tbslen,
               const uint8_t *sig, size_t siglen)
{
	struct dsa_sig *dsa_sig;
	struct bignum_ctx *bn_ctx;
	const struct bignum *s;
	const struct bignum *r;
	struct bignum w;
	struct bignum m;
	struct bignum u1;
	struct bignum u2;
	struct bignum v;
	size_t qbytes;
	int ret = -1;

	bignum_init(&w);
	bignum_init(&m);
	bignum_init(&u1);
	bignum_init(&u2);
	bignum_init(&v);
	bn_ctx = bignum_ctx_new();
	dsa_sig = dsa_sig_new();
	qbytes = bignum_num_bytes(ctx->q);
	if (tbslen > qbytes)
		tbslen = qbytes;
	if (!dsa_sig
	 || !d2i_dsa_sig(&dsa_sig, (uint8_t**)&sig, siglen)
	 || !bn_ctx
	 || !bignum_bin2bignum(tbs, tbslen, &m))
		goto end;
	dsa_sig_get0(dsa_sig, &r, &s);
	if (bignum_is_zero(r)
	 || bignum_is_zero(s)
	 || bignum_is_negative(r)
	 || bignum_is_negative(s)
	 || bignum_cmp(r, ctx->q) >= 0
	 || bignum_cmp(s, ctx->q) >= 0)
	{
		ret = 0;
		goto end;
	}
	if (!bignum_mod_inverse(&w, s, ctx->q, bn_ctx)
	 || !bignum_mod_mul(&u1, &m, &w, ctx->q, bn_ctx)
	 || !bignum_mod_mul(&u2, r, &w, ctx->q, bn_ctx)
	 || !bignum_mod_exp(&u1, ctx->g, &u1, ctx->p, bn_ctx)
	 || !bignum_mod_exp(&u2, ctx->y, &u2, ctx->p, bn_ctx)
	 || !bignum_mod_mul(&v, &u1, &u2, ctx->p, bn_ctx)
	 || !bignum_mod(&v, &v, ctx->q, bn_ctx))
		goto end;
	if (!bignum_cmp(r, &v))
		ret = 1;
	else
		ret = 0;

end:
	bignum_clear(&w);
	bignum_clear(&m);
	bignum_clear(&u1);
	bignum_clear(&u2);
	bignum_clear(&v);
	bignum_ctx_free(bn_ctx);
	dsa_sig_free(dsa_sig);
	return ret;
}

ssize_t dsa_size(struct dsa_ctx *ctx)
{
	ssize_t bits = dsa_bits(ctx);
	if (bits == -1)
		return bits;
	return bits / 8;
}

ssize_t dsa_bits(struct dsa_ctx *ctx)
{
	if (!ctx || !ctx->p)
		return -1;
	return bignum_num_bits(ctx->p);
}

struct dsa_ctx *dsa_new(void)
{
	struct dsa_ctx *ctx = calloc(sizeof(*ctx), 1);
	if (!ctx)
		return NULL;
	refcount_init(&ctx->refcount, 1);
	return ctx;
}

void dsa_free(struct dsa_ctx *ctx)
{
	if (!ctx)
		return;
	if (refcount_dec(&ctx->refcount))
		return;
	bignum_free(ctx->p);
	bignum_free(ctx->q);
	bignum_free(ctx->g);
	bignum_free(ctx->y);
	bignum_free(ctx->x);
	free(ctx);
}

int dsa_up_ref(struct dsa_ctx *ctx)
{
	return refcount_inc(&ctx->refcount);
}

struct dsa_sig *dsa_sig_new(void)
{
	return calloc(sizeof(struct dsa_sig), 1);
}

void dsa_sig_free(struct dsa_sig *sig)
{
	if (!sig)
		return;
	bignum_free(sig->r);
	bignum_free(sig->s);
	free(sig);
}

void dsa_sig_get0(const struct dsa_sig *sig, const struct bignum **r,
                  const struct bignum **s)
{
	if (r)
		*r = sig->r;
	if (s)
		*s = sig->s;
}

int dsa_sig_set0(struct dsa_sig *sig, struct bignum *r, struct bignum *s)
{
	bignum_free(sig->r);
	sig->r = r;
	bignum_free(sig->s);
	sig->s = s;
	return 1;
}
