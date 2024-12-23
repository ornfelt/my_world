#include "ec/ec.h"

#include <jkssl/bignum.h>

#include <stdlib.h>
#include <string.h>

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
	dst->infinity = src->infinity;
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
	if (bignum_is_negative(a->x) != bignum_is_negative(b->x)
	 && !bignum_ucmp(a->x, b->x))
	{
		ec_point_set_to_infinity(group, r);
		return 1;
	}
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
	 || !bignum_mod_sub(dx, b->x, a->x, group->p, ctx)
	 || !bignum_mod_sub(dy, b->y, a->y, group->p, ctx)
	 || !bignum_mod_inverse(dxinv, dx, group->p, ctx)
	 || !bignum_mod_mul(s, dy, dxinv, group->p, ctx)
	 || !bignum_mod_mul(x, s, s, group->p, ctx)
	 || !bignum_mod_sub(x, x, a->x, group->p, ctx)
	 || !bignum_mod_sub(x, x, b->x, group->p, ctx)
	 || !bignum_mod_sub(y, a->x, x, group->p, ctx)
	 || !bignum_mod_mul(y, y, s, group->p, ctx)
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
	 || !bignum_mod_add(yinv, a->y, a->y, group->p, ctx)
	 || !bignum_mod_inverse(yinv, yinv, group->p, ctx)
	 || !bignum_mod_mul(s, a->x, a->x, group->p, ctx)
	 || !bignum_mul_word(s, s, 3)
	 || !bignum_mod_add(s, s, group->a, group->p, ctx)
	 || !bignum_mod_mul(s, s, yinv, group->p, ctx)
	 || !bignum_mod_mul(x, s, s, group->p, ctx)
	 || !bignum_mod_sub(x, x, a->x, group->p, ctx)
	 || !bignum_mod_sub(x, x, a->x, group->p, ctx)
	 || !bignum_mod_sub(y, a->x, x, group->p, ctx)
	 || !bignum_mod_mul(y, y, s, group->p, ctx)
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
	bignum_zero(p->x);
	bignum_zero(p->y);
	p->infinity = 1;
	return 1;
}

int ec_point_is_on_curve(const struct ec_group *group, const struct ec_point *p,
                         struct bignum_ctx *ctx)
{
	struct bignum *yp2;
	struct bignum *xp3;
	struct bignum *ax;
	int ret = -1;

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
	 || !bignum_mod_mul(ax, p->x, group->a, group->p, ctx)
	 || !bignum_mod_add(xp3, xp3, ax, group->p, ctx)
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
	r->infinity = 0;
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

int ec_point_set_affine_coordinates(const struct ec_group *group,
                                    struct ec_point *point,
                                    const struct bignum *x,
                                    const struct bignum *y,
                                    struct bignum_ctx *bn_ctx)
{
	if (!group
	 || !point
	 || !x
	 || !y
	 || !bn_ctx)
		return 0;
	point->infinity = 0;
	if (!bignum_copy(point->x, x)
	 || !bignum_copy(point->y, y))
		return 0;
	return 1;
}

int ec_point_get_affine_coordinates(const struct ec_group *group,
                                    const struct ec_point *point,
                                    struct bignum *x,
                                    struct bignum *y,
                                    struct bignum_ctx *bn_ctx)
{
	if (!group
	 || !point
	 || !bn_ctx)
		return 0;
	if (x)
	{
		if (!bignum_copy(x, point->x))
			return 0;
	}
	if (y)
	{
		if (!bignum_copy(y, point->y))
			return 0;
	}
	return 1;
}

static size_t point2oct_uncompressed(const struct ec_group *group,
                                     const struct ec_point *point,
                                     uint8_t *buf, size_t size,
                                     struct bignum_ctx *bn_ctx)
{
	(void)bn_ctx;
	size_t degree = (ec_group_get_degree(group) + 7) / 8;
	size_t ret = 1 + degree * 2;
	if (!buf || !size)
		return ret;
	struct bignum *x = bignum_ctx_get(bn_ctx);
	struct bignum *y = bignum_ctx_get(bn_ctx);
	if (!x || !y)
	{
		ret = 0;
		goto end;
	}
	if (!ec_point_get_affine_coordinates(group, point, x, y, bn_ctx))
	{
		ret = 0;
		goto end;
	}
	size_t x_size = bignum_num_bytes(x);
	size_t y_size = bignum_num_bytes(y);
	if (x_size > degree
	 || y_size > degree)
	{
		ret = 0;
		goto end;
	}
	buf[0] = 0x04;
	buf++;
	size--;
	if (!size)
		goto end;
	if (size < x_size)
	{
		uint8_t *data = malloc(x_size);
		if (!data)
		{
			ret = 0;
			goto end;
		}
		if (!bignum_bignum2bin(x, data))
		{
			free(data);
			ret = 0;
			goto end;
		}
		size_t prefix = degree - x_size;
		memset(buf, 0, prefix);
		memcpy(&buf[prefix], data, size - prefix);
		free(data);
		goto end;
	}
	if (!bignum_bignum2bin(x, &buf[degree - x_size]))
	{
		ret = 0;
		goto end;
	}
	buf += x_size;
	size -= x_size;
	if (!size)
		goto end;
	if (size < y_size)
	{
		uint8_t *data = malloc(y_size);
		if (!data)
			return 0;
		if (!bignum_bignum2bin(y, data))
		{
			free(data);
			ret = 0;
			goto end;
		}
		size_t prefix = degree - y_size;
		memset(buf, 0, prefix);
		memcpy(&buf[prefix], data, size - prefix);
		free(data);
		goto end;
	}
	if (!bignum_bignum2bin(y, &buf[degree - y_size]))
	{
		ret = 0;
		goto end;
	}

end:
	bignum_ctx_release(bn_ctx, x);
	bignum_ctx_release(bn_ctx, y);
	return ret;
}

static size_t point2oct_compressed(const struct ec_group *group,
                                   const struct ec_point *point,
                                   uint8_t *buf, size_t size,
                                   struct bignum_ctx *bn_ctx)
{
	/* XXX */
	(void)group;
	(void)point;
	(void)buf;
	(void)size;
	(void)bn_ctx;
	return 0;
}

static size_t point2oct_hybrid(const struct ec_group *group,
                               const struct ec_point *point,
                               uint8_t *buf, size_t size,
                               struct bignum_ctx *bn_ctx)
{
	/* XXX */
	(void)group;
	(void)point;
	(void)buf;
	(void)size;
	(void)bn_ctx;
	return 0;
}

size_t ec_point_point2oct(const struct ec_group *group,
                          const struct ec_point *point,
                          enum point_conversion_form form,
                          uint8_t *buf, size_t size,
                          struct bignum_ctx *bn_ctx)
{
	switch (form)
	{
		case POINT_CONVERSION_UNCOMPRESSED:
			return point2oct_uncompressed(group, point,
			                              buf, size,
			                              bn_ctx);
		case POINT_CONVERSION_COMPRESSED:
			return point2oct_compressed(group, point,
			                            buf, size,
			                            bn_ctx);
		case POINT_CONVERSION_HYBRID:
			return point2oct_hybrid(group, point,
			                        buf, size,
			                        bn_ctx);
		default:
			return 0;
	}
}

size_t ec_point_point2buf(const struct ec_group *group,
                          const struct ec_point *point,
                          enum point_conversion_form form,
                          uint8_t **buf,
                          struct bignum_ctx *bn_ctx)
{
	size_t size = ec_point_point2oct(group, point, form, NULL, 0, bn_ctx);
	if (!size)
		return 0;
	*buf = malloc(size);
	if (!*buf)
		return 0;
	if (!ec_point_point2oct(group, point, form, *buf, size, bn_ctx))
	{
		free(*buf);
		*buf = NULL;
		return 0;
	}
	return size;
}

static int oct2point_uncompressed(const struct ec_group *group,
                                  struct ec_point *point,
                                  const uint8_t *buf, size_t size,
                                  struct bignum_ctx *bn_ctx)
{
	int ret = 0;
	size_t degree = (ec_group_get_degree(group) + 7) / 8;
	if (size != 1 + degree * 2)
		return 0;
	buf++;
	size--;
	struct bignum *x = bignum_bin2bignum(buf, degree, NULL);
	struct bignum *y = bignum_bin2bignum(&buf[degree], degree, NULL);
	if (!x
	 || !y)
		goto end;
	if (!ec_point_set_affine_coordinates(group, point, x, y, bn_ctx))
		goto end;
	x = NULL;
	y = NULL;
	ret = 1;

end:
	bignum_free(x);
	bignum_free(y);
	return ret;
}

int ec_point_oct2point(const struct ec_group *group,
                       struct ec_point *point,
                       const uint8_t *buf, size_t size,
                       struct bignum_ctx *bn_ctx)
{
	if (!size)
		return 0;
	if (buf[0] == 0x04)
		return oct2point_uncompressed(group, point, buf, size, bn_ctx);
	/* XXX */
	return 0;
}
