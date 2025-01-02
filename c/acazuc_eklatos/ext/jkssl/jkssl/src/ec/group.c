#include "oid/oid.h"
#include "ec/ec.h"

#include <jkssl/bignum.h>
#include <jkssl/asn1.h>

#include <stdlib.h>
#include <string.h>

struct ec_group *ec_group_new(void)
{
	struct ec_group *group = calloc(1, sizeof(*group));
	if (!group)
		return NULL;
	group->asn1_flag = JKSSL_EC_NAMED_CURVE;
	group->conv_form = POINT_CONVERSION_UNCOMPRESSED;
	return group;
}

struct ec_group *ec_group_new_curve_gfp(const struct bignum *p,
                                        const struct bignum *a,
                                        const struct bignum *b,
                                        struct bignum_ctx *bn_ctx)
{
	struct ec_group *group = calloc(1, sizeof(*group));
	if (!group)
		return NULL;
	(void)bn_ctx;
	group->asn1_flag = JKSSL_EC_NAMED_CURVE;
	group->conv_form = POINT_CONVERSION_UNCOMPRESSED;
	group->p = bignum_dup(p);
	group->a = bignum_dup(a);
	group->b = bignum_dup(b);
	if (!group->p
	 || !group->a
	 || !group->b)
	{
		ec_group_free(group);
		return NULL;
	}
	return group;
}

void ec_group_free(struct ec_group *group)
{
	if (!group)
		return;
	asn1_oid_free(group->oid);
	ec_point_free(group->g);
	bignum_free(group->p);
	bignum_free(group->a);
	bignum_free(group->b);
	bignum_free(group->n);
	bignum_free(group->h);
	free(group);
}

int ec_group_copy(struct ec_group *dst, const struct ec_group *src)
{
	dst->asn1_flag = src->asn1_flag;
	dst->conv_form = src->conv_form;
	asn1_oid_free(dst->oid);
	if (src->oid)
	{
		dst->oid = asn1_oid_dup(src->oid);
		if (!dst->oid)
			return 0;
	}
	else
	{
		dst->oid = NULL;
	}
	if (src->g)
	{
		if (!dst->g)
		{
			dst->g = ec_point_new(dst);
			if (!dst->g)
				return 0;
		}
		if (!ec_point_copy(dst->g, src->g))
			return 0;
	}
	else
	{
		dst->g = NULL;
	}
	if (src->p)
	{
		if (!dst->p)
		{
			dst->p = bignum_new();
			if (!dst->p)
				return 0;
		}
		if (!bignum_copy(dst->p, src->p))
			return 0;
	}
	else
	{
		dst->p = NULL;
	}
	if (src->a)
	{
		if (!dst->a)
		{
			dst->a = bignum_new();
			if (!dst->a)
				return 0;
		}
		if (!bignum_copy(dst->a, src->a))
			return 0;
	}
	else
	{
		dst->a = NULL;
	}
	if (src->b)
	{
		if (!dst->b)
		{
			dst->b = bignum_new();
			if (!dst->b)
				return 0;
		}
		if (!bignum_copy(dst->b, src->b))
			return 0;
	}
	else
	{
		dst->b = NULL;
	}
	if (src->n)
	{
		if (!dst->n)
		{
			dst->n = bignum_new();
			if (!dst->n)
				return 0;
		}
		if (!bignum_copy(dst->n, src->n))
			return 0;
	}
	else
	{
		dst->n = NULL;
	}
	if (src->h)
	{
		if (!dst->h)
		{
			dst->h = bignum_new();
			if (!dst->h)
				return 0;
		}
		if (!bignum_copy(dst->h, src->h))
			return 0;
	}
	else
	{
		dst->h = NULL;
	}
	return 1;
}

struct ec_group *ec_group_dup(const struct ec_group *group)
{
	struct ec_group *dup = calloc(1, sizeof(*dup));
	if (!dup)
		return NULL;
	if (!ec_group_copy(dup, group))
	{
		ec_group_free(dup);
		return NULL;
	}
	return dup;
}

int ec_group_set_curve(struct ec_group *group, const struct bignum *p,
                       const struct bignum *a, const struct bignum *b,
                       struct bignum_ctx *bn_ctx)
{
	(void)bn_ctx;
	if (!p || !a || !b)
		return 0;
	struct bignum *dup_p = bignum_dup(p);
	struct bignum *dup_a = bignum_dup(a);
	struct bignum *dup_b = bignum_dup(b);
	if (!dup_p || !dup_a || !dup_b)
	{
		bignum_free(dup_p);
		bignum_free(dup_a);
		bignum_free(dup_b);
		return 0;
	}
	bignum_free(group->p);
	group->p = dup_p;
	bignum_free(group->a);
	group->a = dup_a;
	bignum_free(group->b);
	group->b = dup_b;
	return 1;
}

int ec_group_get_curve(const struct ec_group *group, struct bignum *p,
                       struct bignum *a, struct bignum *b,
                       struct bignum_ctx *bn_ctx)
{
	(void)bn_ctx;
	if (p)
	{
		if (!bignum_copy(p, group->p))
			return 0;
	}
	if (a)
	{
		if (!bignum_copy(a, group->a))
			return 0;
	}
	if (b)
	{
		if (!bignum_copy(b, group->b))
			return 0;
	}
	return 1;
}

struct asn1_oid *ec_group_get_curve_name(const struct ec_group *group)
{
	if (!group->oid)
		return NULL;
	return asn1_oid_dup(group->oid);
}

int ec_group_set_curve_name(struct ec_group *group, const struct asn1_oid *oid)
{
	struct asn1_oid *dup = asn1_oid_dup(oid);
	if (!dup)
		return 0;
	asn1_oid_free(group->oid);
	group->oid = dup;
	return 1;
}

int ec_group_set_generator(struct ec_group *group, const struct ec_point *g,
                           const struct bignum *n, const struct bignum *h)
{
	if (!g || !n || !h)
		return 0;
	struct ec_point *dup_g = ec_point_dup(g);
	struct bignum *dup_n = bignum_dup(n);
	struct bignum *dup_h = bignum_dup(h);
	if (!dup_g || !dup_n || !dup_h)
	{
		ec_point_free(dup_g);
		bignum_free(dup_n);
		bignum_free(dup_h);
		return 0;
	}
	ec_point_free(group->g);
	group->g = dup_g;
	bignum_free(group->n);
	group->n = dup_n;
	bignum_free(group->h);
	group->h = dup_h;
	return 1;
}

const struct bignum *ec_group_get0_cofactor(const struct ec_group *group)
{
	return group->h;
}

const struct bignum *ec_group_get0_field(const struct ec_group *group)
{
	return group->p;
}

const struct bignum *ec_group_get0_order(const struct ec_group *group)
{
	return group->n;
}

const struct ec_point *ec_group_get0_generator(const struct ec_group *group)
{
	return group->g;
}

static const struct gfp_curve *gfp_curves[] =
{
	&prime192v1,
	&prime192v2,
	&prime192v3,
	&prime239v1,
	&prime239v2,
	&prime239v3,
	&prime256v1,
	&secp112r1,
	&secp112r2,
	&secp128r1,
	&secp128r2,
	&secp160r1,
	&secp160r2,
	&secp160k1,
	&secp192k1,
	&secp224r1,
	&secp224k1,
	&secp256k1,
	&secp384r1,
	&secp521r1,
	&frp256v1,
	&sm2,
	&gost3410_12_256,
	&gost3410_12_512,
	&brainpoolP160r1,
	&brainpoolP160t1,
	&brainpoolP192r1,
	&brainpoolP192t1,
	&brainpoolP224r1,
	&brainpoolP224t1,
	&brainpoolP256r1,
	&brainpoolP256t1,
	&brainpoolP320r1,
	&brainpoolP320t1,
	&brainpoolP384r1,
	&brainpoolP384t1,
	&brainpoolP512r1,
	&brainpoolP512t1,
};

static int set_curve(struct ec_group *group,
                     const struct asn1_oid *oid,
                     const struct gfp_curve *curve)
{
	struct bignum *p = bignum_bin2bignum(curve->p_data, curve->p_size, NULL);
	struct bignum *a = bignum_bin2bignum(curve->a_data, curve->a_size, NULL);
	struct bignum *b = bignum_bin2bignum(curve->b_data, curve->b_size, NULL);
	struct bignum *n = bignum_bin2bignum(curve->n_data, curve->n_size, NULL);
	struct bignum *h = bignum_bin2bignum(curve->h_data, curve->h_size, NULL);
	struct bignum *gx = bignum_bin2bignum(curve->gx_data, curve->gx_size, NULL);
	struct bignum *gy = bignum_bin2bignum(curve->gy_data, curve->gy_size, NULL);
	struct ec_point *g = ec_point_new(group);
	struct bignum_ctx *bn_ctx = bignum_ctx_new();
	int ret = 0;

	if (!p
	 || !a
	 || !b
	 || !n
	 || !h
	 || !gx
	 || !gy
	 || !g
	 || !bn_ctx
	 || !ec_point_set_affine_coordinates(group, g, gx, gy, bn_ctx)
	 || !ec_group_set_curve(group, p, a, b, bn_ctx)
	 || !ec_group_set_generator(group, g, n, h)
	 || !ec_group_set_curve_name(group, oid))
		goto end;
	ret = 1;

end:
	bignum_free(p);
	bignum_free(a);
	bignum_free(b);
	bignum_free(n);
	bignum_free(h);
	bignum_free(gx);
	bignum_free(gy);
	ec_point_free(g);
	bignum_ctx_free(bn_ctx);
	return ret;
}

int ec_group_set_curve_by_name(struct ec_group *group,
                               const struct asn1_oid *oid)
{
	for (size_t i = 0; i < sizeof(gfp_curves) / sizeof(*gfp_curves); ++i)
	{
		const struct gfp_curve *curve = gfp_curves[i];
		if (!asn1_oid_cmp_uint32(oid, curve->oid_data, curve->oid_size))
			return set_curve(group, oid, curve);
	}
	return 0;
}

void ec_foreach_curve(int (*cb)(const char *name, const uint32_t *oid,
                                size_t oid_size, void *data),
                      void *data)
{
	for (size_t i = 0; i < sizeof(gfp_curves) / sizeof(*gfp_curves); ++i)
	{
		const struct gfp_curve *curve = gfp_curves[i];
		if (!cb(curve->name, curve->oid_data, curve->oid_size, data))
			return;
	}
}

struct curve_get_state
{
	const char *name;
	struct ec_group *group;
};

static int curve_get_foreach(const char *name, const uint32_t *oid_data,
                             size_t oid_size, void *data)
{
	struct curve_get_state *state = data;
	struct asn1_oid *oid = NULL;
	if (strcmp(state->name, name))
		return 1;
	state->group = ec_group_new();
	if (!state->group)
		goto err;
	oid = asn1_oid_new();
	if (!oid)
		goto err;
	if (!asn1_oid_set1(oid, oid_data, oid_size))
		goto err;
	if (!ec_group_set_curve_by_name(state->group, oid))
		goto err;
	asn1_oid_free(oid);
	return 0;

err:
	asn1_oid_free(oid);
	ec_group_free(state->group);
	state->group = NULL;
	return 0;
}

struct ec_group *ec_get_curvebyname(const char *name)
{
	struct curve_get_state state;
	state.name = name;
	state.group = NULL;
	ec_foreach_curve(curve_get_foreach, &state);
	return state.group;
}

int ec_group_get_degree(const struct ec_group *group)
{
	return bignum_num_bits(group->p);
}

void ec_group_set_asn1_flag(struct ec_group *group, int flag)
{
	group->asn1_flag = flag;
}

int ec_group_get_asn1_flag(const struct ec_group *group)
{
	return group->asn1_flag;
}

void ec_group_set_point_conversion_form(struct ec_group *group,
                                        enum point_conversion_form form)
{
	group->conv_form = form;
}

enum point_conversion_form ec_group_get_point_conversion_form(const struct ec_group *group)
{
	return group->conv_form;
}

int ec_group_check(const struct ec_group *group, struct bignum_ctx *bn_ctx)
{
	if (!group->n
	 || !group->p
	 || !group->g
	 || !ec_point_is_on_curve(group, group->g, bn_ctx))
		return 0;
	/* XXX check if named curve is ok */
	return 1;
}
