#include "ec/ec.h"

#include <jkssl/bignum.h>
#include <jkssl/der.h>

#include <string.h>
#include <stdlib.h>

int ecdsa_size(const struct ec_key *key)
{
	return 20 + 2 * ((ec_group_get_degree(key->group) + 7) / 8); /* XXX be more precise about DER overhead */
}

struct ecdsa_sig *ecdsa_do_sign(const uint8_t *tbs, size_t tbslen,
                                struct ec_key *key)
{
	struct ecdsa_sig *ecdsa_sig = NULL;
	struct bignum_ctx *bn_ctx;
	struct ec_group *group = key->group;
	struct ec_point *p;
	struct bignum *kinv;
	struct bignum *tmp;
	struct bignum *x;
	struct bignum *k;
	struct bignum *z;
	struct bignum *r;
	struct bignum *s;

	if (!key->priv)
		return 0;
	if (tbslen > bignum_num_bits(group->n) / 8u)
		return 0;
	bn_ctx = bignum_ctx_new();
	if (!bn_ctx)
		return 0;
	p = ec_point_new(group);
	kinv = bignum_ctx_get(bn_ctx);
	tmp = bignum_ctx_get(bn_ctx);
	k = bignum_ctx_get(bn_ctx);
	z = bignum_ctx_get(bn_ctx);
	x = bignum_ctx_get(bn_ctx);
	r = bignum_new();
	s = bignum_new();
	if (!p
	 || !kinv
	 || !tmp
	 || !k
	 || !z
	 || !x
	 || !r
	 || !s
	 || !bignum_bin2bignum(tbs, tbslen, z))
		goto end;
	while (1)
	{
		if (!bignum_rand_range(k, group->n, BIGNUM_RAND_TOP_TWO, BIGNUM_RAND_BOT_ODD)
		 || !ec_point_mul(group, p, k, NULL, NULL, bn_ctx)
		 || !ec_point_get_affine_coordinates(group, p, x, NULL, bn_ctx)
		 || !bignum_mod(r, x, group->n, bn_ctx))
			goto end;
		if (bignum_is_zero(r))
			continue;
		if (!bignum_mod_inverse(kinv, k, group->n, bn_ctx)
		 || !bignum_mul(tmp, r, key->priv, bn_ctx)
		 || !bignum_add(tmp, tmp, z, bn_ctx)
		 || !bignum_mod_mul(s, tmp, kinv, group->n, bn_ctx))
			goto end;
		if (bignum_is_zero(s))
			continue;
		break;
	}
	ecdsa_sig = ecdsa_sig_new();
	if (!ecdsa_sig)
		goto end;
	if (!ecdsa_sig_set0(ecdsa_sig, r, s))
	{
		ecdsa_sig_free(ecdsa_sig);
		ecdsa_sig = NULL;
		goto end;
	}
	r = NULL;
	s = NULL;

end:
	ec_point_free(p);
	bignum_ctx_release(bn_ctx, kinv);
	bignum_ctx_release(bn_ctx, tmp);
	bignum_ctx_release(bn_ctx, k);
	bignum_ctx_release(bn_ctx, z);
	bignum_ctx_release(bn_ctx, x);
	bignum_ctx_release(bn_ctx, r);
	bignum_ctx_release(bn_ctx, s);
	bignum_ctx_free(bn_ctx);
	return ecdsa_sig;
}

int ecdsa_sign(int type, const uint8_t *tbs, size_t tbslen,
               uint8_t *sig, size_t *siglen, struct ec_key *key)
{
	struct ecdsa_sig *ecdsa_sig = ecdsa_do_sign(tbs, tbslen, key);
	uint8_t *sigbytes = NULL;
	int ret = 0;

	(void)type;
	if (!ecdsa_sig)
		goto end;
	ret = i2d_ecdsa_sig(ecdsa_sig, &sigbytes);
	if (ret <= 0)
		goto end;
	memcpy(sig, sigbytes, ret);
	*siglen = ret;
	ret = 1;

end:
	ecdsa_sig_free(ecdsa_sig);
	free(sigbytes);
	return ret;
}

int ecdsa_do_verify(const uint8_t *tbs, size_t tbslen,
                    const struct ecdsa_sig *ecdsa_sig, struct ec_key *key)
{
	struct bignum_ctx *bn_ctx;
	struct ec_group *group = key->group;
	struct bignum *sinv = NULL;
	const struct bignum *r;
	const struct bignum *s;
	struct bignum *z = NULL;
	struct bignum *x = NULL;
	struct bignum *u = NULL;
	struct bignum *v = NULL;
	struct ec_point *p = NULL;
	int ret = -1;

	if (tbslen > bignum_num_bits(group->n) / 8u)
		return 0;
	bn_ctx = bignum_ctx_new();
	if (!bn_ctx)
		return 0;
	if (ec_point_is_at_infinity(group, key->pub)
	 || ec_point_is_on_curve(group, key->pub, bn_ctx) != 1)
		goto end;
	ecdsa_sig_get0(ecdsa_sig, &r, &s);
	sinv = bignum_ctx_get(bn_ctx);
	z = bignum_ctx_get(bn_ctx);
	x = bignum_ctx_get(bn_ctx);
	u = bignum_ctx_get(bn_ctx);
	v = bignum_ctx_get(bn_ctx);
	p = ec_point_new(group);
	if (!sinv
	 || !z
	 || !x
	 || !u
	 || !v
	 || !p)
		goto end;
	if (bignum_is_zero(r)
	 || bignum_is_negative(r)
	 || bignum_cmp(r, group->n) >= 0
	 || bignum_is_zero(s)
	 || bignum_is_negative(s)
	 || bignum_cmp(s, group->n) >= 0)
	{
		ret = 0;
		goto end;
	}
	if (!bignum_bin2bignum(tbs, tbslen, z)
	 || !bignum_mod_inverse(sinv, s, group->n, bn_ctx)
	 || !bignum_mod_mul(u, z, sinv, group->n, bn_ctx)
	 || !bignum_mod_mul(v, r, sinv, group->n, bn_ctx)
	 || !ec_point_mul(group, p, u, key->pub, v, bn_ctx)
	 || !ec_point_get_affine_coordinates(group, p, x, NULL, bn_ctx)
	 || !bignum_nnmod(z, x, group->n, bn_ctx))
		goto end;
	if (!bignum_cmp(z, r))
		ret = 1;
	else
		ret = 0;

end:
	bignum_ctx_release(bn_ctx, sinv);
	bignum_ctx_release(bn_ctx, z);
	bignum_ctx_release(bn_ctx, x);
	bignum_ctx_release(bn_ctx, u);
	bignum_ctx_release(bn_ctx, v);
	ec_point_free(p);
	bignum_ctx_free(bn_ctx);
	return ret;
}

int ecdsa_verify(int type, const uint8_t *tbs, size_t tbslen,
                 const uint8_t *sig, size_t siglen, struct ec_key *key)
{
	struct ecdsa_sig *ecdsa_sig = ecdsa_sig_new();
	int ret = -1;

	(void)type;
	if (!ecdsa_sig)
		return -1;
	if (!d2i_ecdsa_sig(&ecdsa_sig, (uint8_t**)&sig, siglen))
		goto end;
	ret = ecdsa_do_verify(tbs, tbslen, ecdsa_sig, key);

end:
	ecdsa_sig_free(ecdsa_sig);
	return ret;
}
