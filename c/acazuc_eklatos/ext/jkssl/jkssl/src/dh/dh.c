#include "dh/dh.h"

#include <jkssl/bignum.h>
#include <jkssl/dh.h>

#include <stdlib.h>

struct dh *dh_new(void)
{
	struct dh *dh = calloc(sizeof(*dh), 1);
	if (!dh)
		return NULL;
	refcount_init(&dh->refcount, 1);
	return dh;
}

void dh_free(struct dh *dh)
{
	if (!dh)
		return;
	if (refcount_dec(&dh->refcount))
		return;
	bignum_free(dh->p);
	bignum_free(dh->q);
	bignum_free(dh->g);
	bignum_free(dh->x);
	bignum_free(dh->y);
	free(dh);
}

int dh_up_ref(struct dh *dh)
{
	return refcount_inc(&dh->refcount);
}

int dh_generate_parameters(struct dh *dh, uint32_t bits,
                           size_t generator, struct bignum_gencb *gencb)
{
	struct bignum_ctx *bn_ctx;

	if (generator <= 1)
		return 0;
	bignum_free(dh->p);
	bignum_free(dh->q);
	bignum_free(dh->g);
	dh->p = bignum_new();
	dh->q = NULL;
	dh->g = bignum_new();
	bn_ctx = bignum_ctx_new();
	if (!dh->p
	 || !dh->g
	 || !bn_ctx
	 || !bignum_set_word(dh->g, generator)
	 || bignum_num_bits(dh->g) >= (int)bits
	 || !bignum_generate_prime(dh->p, bits, 0, NULL, NULL, gencb, bn_ctx))
		goto err;
	/* XXX set q = (p - 1) / 2
	 * in fact, we should use random q and derive p = 2 * q + 1
	 */
	bignum_ctx_free(bn_ctx);
	return 1;

err:
	bignum_free(dh->p);
	dh->p = NULL;
	bignum_free(dh->g);
	dh->g = NULL;
	bignum_ctx_free(bn_ctx);
	return 0;
}

int dh_generate_key(struct dh *dh)
{
	struct bignum_ctx *bn_ctx;

	bignum_free(dh->x);
	bignum_free(dh->y);
	bn_ctx = bignum_ctx_new();
	dh->x = bignum_new();
	dh->y = bignum_new();
	if (!bn_ctx
	 || !dh->x
	 || !dh->y
	 || !bignum_rand_range(dh->x, dh->p, BIGNUM_RAND_TOP_TWO,
	                       BIGNUM_RAND_BOT_ODD)
	 || !bignum_umod_exp(dh->y, dh->g, dh->x, dh->p, bn_ctx))
		goto err;
	bignum_ctx_free(bn_ctx);
	return 1;

err:
	bignum_free(dh->x);
	dh->x = NULL;
	bignum_free(dh->y);
	dh->y = NULL;
	bignum_ctx_free(bn_ctx);
	return 0;
}

int dh_compute_key(uint8_t *key, const struct bignum *pub, struct dh *dh)
{
	struct bignum_ctx *bn_ctx = NULL;
	struct bignum *s = NULL;
	int ret = -1;

	if (!dh || !dh->p || !dh->x || !dh->y || !pub)
		return -1;
	bn_ctx = bignum_ctx_new();
	if (!bn_ctx)
		goto end;
	s = bignum_ctx_get(bn_ctx);
	if (!s
	 || !bignum_mod_exp(s, pub, dh->x, dh->p, bn_ctx)
	 || !bignum_bignum2bin(s, key))
		goto end;
	ret = bignum_num_bytes(s);

end:
	bignum_ctx_release(bn_ctx, s);
	bignum_ctx_free(bn_ctx);
	return ret;
}

int dh_check(struct dh *dh, int *codes)
{
	if (!codes)
		return 0;
	struct bignum_ctx *bn_ctx = bignum_ctx_new();
	if (!bn_ctx)
		return 0;
	*codes = 0;
	if (!dh->p
	 || bignum_check_prime(dh->p, bn_ctx, NULL) != 1)
		*codes |= DH_CHECK_P_NOT_PRIME;
	if (!dh->g
	 || bignum_is_negative(dh->g)
	 || bignum_is_zero(dh->g)
	 || bignum_is_one(dh->g))
		*codes |= DH_NOT_SUITABLE_GENERATOR;
	/* XXX p safe prime */
	/* XXX q prime */
	/* XXX p = q * 2 + 1 */
	bignum_ctx_free(bn_ctx);
	return 1;
}

int dh_check_params(struct dh *dh, int *codes)
{
	if (!codes)
		return 0;
	struct bignum_ctx *bn_ctx = bignum_ctx_new();
	if (!bn_ctx)
		return 0;
	*codes = 0;
	if (!dh->p
	 || bignum_check_prime(dh->p, bn_ctx, NULL) != 1)
		*codes |= DH_CHECK_P_NOT_PRIME;
	if (!dh->g
	 || bignum_is_negative(dh->g)
	 || bignum_is_zero(dh->g)
	 || bignum_is_one(dh->g))
		*codes |= DH_NOT_SUITABLE_GENERATOR;
	bignum_ctx_free(bn_ctx);
	return 1;
}

ssize_t dh_size(struct dh *dh)
{
	ssize_t bits = dh_bits(dh);
	if (bits == -1)
		return bits;
	return bits / 8;
}

ssize_t dh_bits(struct dh *dh)
{
	if (!dh || !dh->p)
		return -1;
	return bignum_num_bits(dh->p);
}

void dh_get0_pqg(const struct dh *dh, const struct bignum **p,
                 const struct bignum **q, const struct bignum **g)
{
	if (p)
		*p = dh->p;
	if (q)
		*q = dh->q;
	if (g)
		*g = dh->g;
}

int dh_set0_pqg(struct dh *dh, struct bignum *p,
                struct bignum *q, struct bignum *g)
{
	if (!p || !q || !g)
		return 0;
	bignum_free(dh->p);
	dh->p = p;
	bignum_free(dh->q);
	dh->q = q;
	bignum_free(dh->g);
	dh->g = g;
	return 1;
}

void dh_get0_key(const struct dh *dh, const struct bignum **y,
                 struct bignum **x)
{
	if (y)
		*y = dh->y;
	if (x)
		*x = dh->x;
}

int dh_set0_key(struct dh *dh, struct bignum *y, struct bignum *x)
{
	if (!y)
		return 0;
	bignum_free(dh->y);
	dh->y = y;
	bignum_free(dh->x);
	dh->x = x;
	return 1;
}

const struct bignum *dh_get0_p(const struct dh *dh)
{
	return dh->p;
}

const struct bignum *dh_get0_q(const struct dh *dh)
{
	return dh->q;
}

const struct bignum *dh_get0_g(const struct dh *dh)
{
	return dh->g;
}

const struct bignum *dh_get0_priv_key(const struct dh *dh)
{
	return dh->x;
}

const struct bignum *dh_get0_pub_key(const struct dh *dh)
{
	return dh->y;
}
