#include "ec/ec.h"

#include <jkssl/bignum.h>

#include <stdlib.h>

struct ec_point *ec_point_new(const struct ec_group *group)
{
	(void)group;
	struct ec_point *point = malloc(sizeof(*point));
	if (!point)
		return NULL;
	point->infinity = 1;
	point->x = bignum_new();
	point->y = bignum_new();
	if (!point->x || !point->y)
	{
		ec_point_free(point);
		return NULL;
	}
	return point;
}

void ec_point_free(struct ec_point *point)
{
	if (!point)
		return;
	bignum_free(point->x);
	bignum_free(point->y);
	free(point);
}

int ec_point_copy(struct ec_point *dst, const struct ec_point *src)
{
	if (!bignum_copy(dst->x, src->x)
	 || !bignum_copy(dst->y, src->y))
		return 0;
	return 1;
}

struct ec_point *ec_point_dup(const struct ec_point *point)
{
	struct ec_point *dup = malloc(sizeof(*dup));
	if (!dup)
		return NULL;
	dup->infinity = point->infinity;
	dup->x = bignum_dup(point->x);
	dup->y = bignum_dup(point->y);
	if (!dup->x || !dup->y)
	{
		ec_point_free(dup);
		return NULL;
	}
	return dup;
}

int ec_point_add(const struct ec_group *group, struct ec_point *r,
                 const struct ec_point *a, const struct ec_point *b,
                 struct bignum_ctx *ctx)
{
	struct bignum *dx;
	struct bignum *dy;
	struct bignum *dxinv;
	struct bignum *s;
	struct bignum *x;
	struct bignum *y;
	int ret = 0;

	if (!ec_point_cmp(group, a, b, ctx))
		return ec_point_dbl(group, r, a, ctx);
	if (ec_point_is_at_infinity(group, a))
		return ec_point_copy(r, b);
	if (ec_point_is_at_infinity(group, b))
		return ec_point_copy(r, a);
	/* XXX test a == -b */
	dx = bignum_ctx_get(ctx);
	dy = bignum_ctx_get(ctx);
	dxinv = bignum_ctx_get(ctx);
	s = bignum_ctx_get(ctx);
	x = bignum_ctx_get(ctx);
	y = bignum_ctx_get(ctx);
	if (!dx
	 || !dy
	 || !dxinv
	 || !s
	 || !x
	 || !y
	 || !bignum_sub(dx, a->x, b->x, ctx)
	 || !bignum_sub(dy, a->y, b->y, ctx)
	 || !bignum_mod_inverse(dxinv, dx, group->p, ctx)
	 || !bignum_mod_mul(s, dy, dxinv, group->p, ctx)
	 || !bignum_mod_mul(x, s, s, group->p, ctx)
	 || !bignum_mod_sub(x, x, a->x, group->p, ctx)
	 || !bignum_mod_sub(x, x, b->x, group->p, ctx)
	 || !bignum_sub(y, a->x, x, ctx)
	 || !bignum_mul(y, y, s, ctx)
	 || !bignum_mod_sub(y, y, a->y, group->p, ctx))
		goto end;
	bignum_swap(r->x, x);
	bignum_swap(r->y, y);
	ret = 1;

end:
	bignum_ctx_release(ctx, dx);
	bignum_ctx_release(ctx, dy);
	bignum_ctx_release(ctx, dxinv);
	bignum_ctx_release(ctx, s);
	bignum_ctx_release(ctx, x);
	bignum_ctx_release(ctx, y);
	return ret;
}

int ec_point_dbl(const struct ec_group *group, struct ec_point *r,
                 const struct ec_point *a, struct bignum_ctx *ctx)
{
	struct bignum *yinv;
	struct bignum *s;
	struct bignum *x;
	struct bignum *y;
	int ret = 0;

	if (ec_point_is_at_infinity(group, a))
		return ec_point_copy(r, a);
	if (bignum_is_zero(a->y))
	{
		ec_point_set_to_infinity(group, r);
		return 1;
	}
	yinv = bignum_ctx_get(ctx);
	s = bignum_ctx_get(ctx);
	x = bignum_ctx_get(ctx);
	y = bignum_ctx_get(ctx);
	if (!yinv
	 || !s
	 || !x
	 || !y
	 || !bignum_add(yinv, a->y, a->y, ctx)
	 || !bignum_mod_inverse(yinv, yinv, group->p, ctx)
	 || !bignum_mod_mul(s, a->x, a->x, group->p, ctx)
	 || !bignum_mul_word(s, s, 3)
	 || !bignum_add(s, s, group->a, ctx)
	 || !bignum_mod_mul(s, s, yinv, group->p, ctx)
	 || !bignum_mod_mul(x, s, s, group->p, ctx)
	 || !bignum_mod_sub(x, x, a->x, group->p, ctx)
	 || !bignum_mod_sub(x, x, a->x, group->p, ctx)
	 || !bignum_sub(y, a->x, x, ctx)
	 || !bignum_mul(y, y, s, ctx)
	 || !bignum_mod_sub(y, y, a->y, group->p, ctx))
		goto end;
	bignum_swap(r->x, x);
	bignum_swap(r->y, y);
	ret = 1;

end:
	bignum_ctx_release(ctx, yinv);
	bignum_ctx_release(ctx, s);
	bignum_ctx_release(ctx, x);
	bignum_ctx_release(ctx, y);
	return ret;
}

int ec_point_invert(const struct ec_group *group, struct ec_point *r,
                    const struct ec_point *a, struct bignum_ctx *ctx)
{
	if (ec_point_is_at_infinity(group, a))
	{
		ec_point_set_to_infinity(group, r);
		return 1;
	}
	if (!bignum_copy(r->x, a->x)
	 || !bignum_copy(r->y, a->y))
		return 0;
	bignum_set_negative(r->y, !bignum_is_negative(r->y));
	if (!bignum_nnmod(r->y, r->y, group->p, ctx))
		return 0;
	return 1;
}

int ec_point_is_at_infinity(const struct ec_group *group,
                            const struct ec_point *p)
{
	(void)group;
	return p->infinity;
}

int ec_point_set_to_infinity(const struct ec_group *group, struct ec_point *p)
{
	(void)group;
	p->infinity = 1;
	return 1;
}

int ec_point_is_on_curve(const struct ec_group *group, const struct ec_point *p,
                         struct bignum_ctx *ctx)
{
	struct bignum *yp2;
	struct bignum *xp3;
	struct bignum *ax;
	int ret =-10;

	if (ec_point_is_at_infinity(group, p))
		return 0;
	yp2 = bignum_ctx_get(ctx);
	xp3 = bignum_ctx_get(ctx);
	ax = bignum_ctx_get(ctx);
	if (!yp2
	 || !xp3
	 || !ax
	 || !bignum_mod_mul(yp2, p->y, p->y, group->p, ctx)
	 || !bignum_mod_mul(xp3, p->x, p->x, group->p, ctx)
	 || !bignum_mod_mul(xp3, xp3, p->x, group->p, ctx)
	 || !bignum_mul(ax, p->x, group->a, ctx)
	 || !bignum_add(xp3, xp3, ax, ctx)
	 || !bignum_mod_add(xp3, xp3, group->b, group->p, ctx))
		goto end;
	ret = !bignum_cmp(xp3, yp2);

end:
	bignum_ctx_release(ctx, yp2);
	bignum_ctx_release(ctx, xp3);
	bignum_ctx_release(ctx, ax);
	return ret;
}

int ec_point_cmp(const struct ec_group *group, const struct ec_point *a,
                 const struct ec_point *b, struct bignum_ctx *ctx)
{
	(void)ctx;
	if (ec_point_is_at_infinity(group, a))
	{
		if (ec_point_is_at_infinity(group, b))
			return 0;
		return 1;
	}
	if (ec_point_is_at_infinity(group, b))
		return 1;
	return !!bignum_cmp(a->x, b->x); /* XXX really ? */
}

static int mul(const struct ec_group *group, struct ec_point *r,
               const struct ec_point *p, const struct bignum *n,
               struct bignum_ctx *ctx)
{
	struct ec_point *a;
	struct ec_point *x;
	size_t nbits;
	int ret = 0;

	if (ec_point_is_at_infinity(group, p))
		return ec_point_set_to_infinity(group, r);
	a = ec_point_new(group);
	x = ec_point_new(group);
	if (!a
	 || !x
	 || !ec_point_copy(a, p))
		goto end;
	nbits = bignum_num_bits(n);
	for (size_t i = 0; i < nbits; ++i)
	{
		if (bignum_is_bit_set(n, i))
		{
			if (!ec_point_add(group, x, x, a, ctx))
				goto end;
		}
		if (!ec_point_dbl(group, a, a, ctx))
			goto end;
	}
	ret = ec_point_copy(r, x);

end:
	ec_point_free(a);
	ec_point_free(x);
	return ret;
}

int ec_point_mul(const struct ec_group *group, struct ec_point *r,
                 const struct bignum *n, const struct ec_point *p,
                 const struct bignum *m, struct bignum_ctx *ctx)
{
	struct ec_point *gn = NULL;
	struct ec_point *pm = NULL;
	int ret = 0;

	if (n)
	{
		gn = ec_point_new(group);
		if (!gn
		 || !mul(group, gn, group->g, n, ctx))
			goto end;
	}
	if (p && m)
	{
		pm = ec_point_new(group);
		if (!pm
		 || !mul(group, pm, p, m, ctx))
		 	goto end;
	}
	if (gn)
	{
		if (pm)
			ret = ec_point_add(group, r, gn, pm, ctx);
		else
			ret = ec_point_copy(r, gn);
	}
	else if (pm)
	{
		ret = ec_point_copy(r, pm);
	}
	ret = 1;

end:
	ec_point_free(gn);
	ec_point_free(pm);
	return ret;
}
