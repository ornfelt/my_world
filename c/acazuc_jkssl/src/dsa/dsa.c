#include "utils/utils.h"
#include "dsa/dsa.h"

#include <jkssl/bignum.h>
#include <jkssl/dsa.h>
#include <jkssl/der.h>

#include <stdlib.h>
#include <string.h>

int dsa_generate_key(struct dsa *dsa)
{
	struct bignum_ctx *bn_ctx;

	bignum_free(dsa->x);
	bignum_free(dsa->y);
	bn_ctx = bignum_ctx_new();
	dsa->x = bignum_new();
	dsa->y = bignum_new();
	if (!bn_ctx
	 || !dsa->x
	 || !dsa->y
	 || !bignum_rand_range(dsa->x, dsa->q, BIGNUM_RAND_TOP_TWO,
	                       BIGNUM_RAND_BOT_ODD)
	 || !bignum_umod_exp(dsa->y, dsa->g, dsa->x, dsa->p, bn_ctx))
		goto err;
	bignum_ctx_free(bn_ctx);
	return 1;

err:
	bignum_free(dsa->x);
	dsa->x = NULL;
	bignum_free(dsa->y);
	dsa->y = NULL;
	bignum_ctx_free(bn_ctx);
	return 0;
}

struct dsa_sig *dsa_do_sign(const uint8_t *tbs, size_t tbslen,
                            struct dsa *dsa)
{
	struct bignum_ctx *bn_ctx;
	struct dsa_sig *dsa_sig = NULL;
	struct bignum *kinv;
	struct bignum *qmt;
	struct bignum *k;
	struct bignum *r;
	struct bignum *s;
	struct bignum *m;
	size_t qbytes;

	bn_ctx = bignum_ctx_new();
	if (!bn_ctx)
		return NULL;
	kinv = bignum_ctx_get(bn_ctx);
	qmt = bignum_ctx_get(bn_ctx);
	k = bignum_ctx_get(bn_ctx);
	r = bignum_new();
	s = bignum_new();
	m = bignum_ctx_get(bn_ctx);
	if (!kinv
	 || !qmt
	 || !k
	 || !r
	 || !s
	 || !m)
		goto end;
	qbytes = bignum_num_bytes(dsa->q);
	if (tbslen > qbytes)
		tbslen = qbytes;
	if (!bn_ctx
	 || !bignum_bin2bignum(tbs, tbslen, m)
	 || !bignum_set_word(qmt, 2)
	 || !bignum_sub(qmt, dsa->q, qmt, bn_ctx))
		goto end;
	while (1)
	{
		if (!bignum_rand_range(k, dsa->q, BIGNUM_RAND_TOP_TWO, BIGNUM_RAND_BOT_ODD)
		 || !bignum_mod_exp(r, dsa->g, k, dsa->p, bn_ctx)
		 || !bignum_mod(r, r, dsa->q, bn_ctx))
			goto end;
		if (bignum_is_zero(r))
			continue;
		if (!bignum_mod_exp(kinv, k, qmt, dsa->q, bn_ctx)
		 || !bignum_mul(s, r, dsa->x, bn_ctx)
		 || !bignum_add(s, s, m, bn_ctx)
		 || !bignum_mod_mul(s, s, kinv, dsa->q, bn_ctx))
			goto end;
		if (bignum_is_zero(s))
			continue;
		break;
	}
	dsa_sig = dsa_sig_new();
	if (!dsa_sig)
		goto end;
	if (!dsa_sig_set0(dsa_sig, r, s))
	{
		dsa_sig_free(dsa_sig);
		dsa_sig = NULL;
		goto end;
	}
	r = NULL;
	s = NULL;

end:
	bignum_free(r);
	bignum_free(s);
	bignum_ctx_release(bn_ctx, m);
	bignum_ctx_release(bn_ctx, k);
	bignum_ctx_release(bn_ctx, qmt);
	bignum_ctx_release(bn_ctx, kinv);
	bignum_ctx_free(bn_ctx);
	return dsa_sig;
}

int dsa_sign(struct dsa *dsa, const uint8_t *tbs, size_t tbslen,
             uint8_t *sig, size_t *siglen)
{
	struct dsa_sig *dsa_sig = dsa_do_sign(tbs, tbslen, dsa);
	uint8_t *sigbytes = NULL;
	int ret = 0;

	if (!dsa_sig)
		goto end;
	ret = i2d_dsa_sig(dsa_sig, &sigbytes);
	if (ret <= 0)
		goto end;
	memcpy(sig, sigbytes, ret);
	*siglen = ret;
	ret = 1;

end:
	dsa_sig_free(dsa_sig);
	free(sigbytes);
	return ret;
}

int dsa_do_verify(const uint8_t *tbs, size_t tbslen, struct dsa_sig *dsa_sig,
                  struct dsa *dsa)
{
	struct bignum_ctx *bn_ctx;
	const struct bignum *s;
	const struct bignum *r;
	struct bignum *w;
	struct bignum *m;
	struct bignum *u1;
	struct bignum *u2;
	struct bignum *v;
	size_t qbytes;
	int ret = -1;

	bn_ctx = bignum_ctx_new();
	if (!bn_ctx)
		return 0;
	w = bignum_ctx_get(bn_ctx);
	m = bignum_ctx_get(bn_ctx);
	u1 = bignum_ctx_get(bn_ctx);
	u2 = bignum_ctx_get(bn_ctx);
	v = bignum_ctx_get(bn_ctx);
	if (!w
	 || !m
	 || !u1
	 || !u2
	 || !v)
		goto end;
	qbytes = bignum_num_bytes(dsa->q);
	if (tbslen > qbytes)
		tbslen = qbytes;
	if (!bignum_bin2bignum(tbs, tbslen, m))
		goto end;
	dsa_sig_get0(dsa_sig, &r, &s);
	if (bignum_is_zero(r)
	 || bignum_is_zero(s)
	 || bignum_is_negative(r)
	 || bignum_is_negative(s)
	 || bignum_cmp(r, dsa->q) >= 0
	 || bignum_cmp(s, dsa->q) >= 0)
	{
		ret = 0;
		goto end;
	}
	if (!bignum_mod_inverse(w, s, dsa->q, bn_ctx)
	 || !bignum_mod_mul(u1, m, w, dsa->q, bn_ctx)
	 || !bignum_mod_mul(u2, r, w, dsa->q, bn_ctx)
	 || !bignum_mod_exp(u1, dsa->g, u1, dsa->p, bn_ctx)
	 || !bignum_mod_exp(u2, dsa->y, u2, dsa->p, bn_ctx)
	 || !bignum_mod_mul(v, u1, u2, dsa->p, bn_ctx)
	 || !bignum_mod(v, v, dsa->q, bn_ctx))
		goto end;
	if (!bignum_cmp(r, v))
		ret = 1;
	else
		ret = 0;

end:
	bignum_ctx_release(bn_ctx, v);
	bignum_ctx_release(bn_ctx, u1);
	bignum_ctx_release(bn_ctx, u2);
	bignum_ctx_release(bn_ctx, m);
	bignum_ctx_release(bn_ctx, w);
	bignum_ctx_free(bn_ctx);
	return ret;
}

int dsa_verify(struct dsa *dsa, const uint8_t *tbs, size_t tbslen,
               const uint8_t *sig, size_t siglen)
{
	struct dsa_sig *dsa_sig = dsa_sig_new();
	int ret = -1;

	if (!dsa_sig)
		return -1;
	if (!d2i_dsa_sig(&dsa_sig, (uint8_t**)&sig, siglen))
		goto end;
	ret = dsa_do_verify(tbs, tbslen, dsa_sig, dsa);

end:
	dsa_sig_free(dsa_sig);
	return ret;
}

ssize_t dsa_size(struct dsa *dsa)
{
	ssize_t bits = dsa_bits(dsa);
	if (bits == -1)
		return -1;
	return bits / 8 + 20; /* XXX be more precise about DER overhead */
}

ssize_t dsa_bits(struct dsa *dsa)
{
	if (!dsa || !dsa->p)
		return -1;
	return bignum_num_bits(dsa->p);
}

struct dsa *dsa_new(void)
{
	struct dsa *dsa = calloc(sizeof(*dsa), 1);
	if (!dsa)
		return NULL;
	refcount_init(&dsa->refcount, 1);
	return dsa;
}

void dsa_free(struct dsa *dsa)
{
	if (!dsa)
		return;
	if (refcount_dec(&dsa->refcount))
		return;
	bignum_free(dsa->p);
	bignum_free(dsa->q);
	bignum_free(dsa->g);
	bignum_free(dsa->y);
	bignum_free(dsa->x);
	free(dsa);
}

void dsa_get0_pqg(const struct dsa *dsa, const struct bignum **p,
                  const struct bignum **q, const struct bignum **g)
{
	if (p)
		*p = dsa->p;
	if (q)
		*q = dsa->q;
	if (g)
		*g = dsa->g;
}

int dsa_set0_pqg(struct dsa *dsa, struct bignum *p, struct bignum *q,
                 struct bignum *g)
{
	if (!p || !q || !g)
		return 0;
	bignum_free(dsa->p);
	dsa->p = p;
	bignum_free(dsa->q);
	dsa->q = q;
	bignum_free(dsa->g);
	dsa->g = g;
	return 1;
}

void dsa_get0_key(const struct dsa *dsa, const struct bignum **y,
                  const struct bignum **x)
{
	if (y)
		*y = dsa->y;
	if (x)
		*x = dsa->x;
}

int dsa_set0_key(struct dsa *dsa, struct bignum *y, struct bignum *x)
{
	if (!y)
		return 0;
	bignum_free(dsa->y);
	dsa->y = y;
	bignum_free(dsa->x);
	dsa->x = x;
	return 1;
}

const struct bignum *dsa_get0_p(const struct dsa *dsa)
{
	return dsa->p;
}

const struct bignum *dsa_get0_q(const struct dsa *dsa)
{
	return dsa->q;
}

const struct bignum *dsa_get0_g(const struct dsa *dsa)
{
	return dsa->g;
}

const struct bignum *dsa_get0_pub_key(const struct dsa *dsa)
{
	return dsa->y;
}

const struct bignum *dsa_get0_priv_key(const struct dsa *dsa)
{
	return dsa->x;
}

int dsa_up_ref(struct dsa *dsa)
{
	return refcount_inc(&dsa->refcount);
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
