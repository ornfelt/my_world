#include "rast.h"

#include <math.h>

struct rast_line
{
	struct rast_vert *v1;
	struct rast_vert *v2;
	int32_t orgx;
	int32_t orgy;
	int32_t x0;
	int32_t y0;
	int32_t x1;
	int32_t y1;
	int32_t dx;
	int32_t dy;
	int32_t sx;
	int32_t sy;
	float len;
	float invlen;
};

static bool
is_out_of_screen(struct rast_ctx *ctx,
                 struct rast_vert *v1,
                 struct rast_vert *v2)
{
	if (v1->z < ctx->min_z && v2->z < ctx->min_z)
		return true;
	if (v1->z > ctx->max_z || v2->z > ctx->max_z)
		return true;
	if (v1->x < ctx->min_x && v2->x < ctx->min_x)
		return true;
	if (v1->x > ctx->max_x && v2->x > ctx->max_x)
		return true;
	if (v1->y < ctx->min_y && v2->y < ctx->min_y)
		return true;
	if (v1->y > ctx->max_y && v2->y > ctx->max_y)
		return true;
	return false;
}

static bool
truncate_x(struct rast_ctx *ctx,
           struct rast_vert *v1,
           struct rast_vert *v2)
{
	if (v1->x < ctx->min_x)
	{
		float f = (ctx->min_x - v1->x) / (v2->x - v1->x);
		rast_vert_mix(ctx, v1, v1, v2, f);
		v1->x = ctx->min_x;
	}
	if (v2->x > ctx->max_x)
	{
		float f = (ctx->max_x - v1->x) / (v2->x - v1->x);
		rast_vert_mix(ctx, v2, v1, v2, f);
		v2->x = ctx->max_x;
	}
	return true;
}

static bool
truncate_y(struct rast_ctx *ctx,
           struct rast_vert *v1,
           struct rast_vert *v2)
{
	if (v1->y < ctx->min_y)
	{
		float f = (ctx->min_y - v1->y) / (v2->y - v1->y);
		rast_vert_mix(ctx, v1, v1, v2, f);
		v1->y = ctx->min_y;
	}
	if (v2->y > ctx->max_y)
	{
		float f = (ctx->max_y - v1->y) / (v2->y - v1->y);
		rast_vert_mix(ctx, v2, v1, v2, f);
		v2->y = ctx->max_y;
	}
	return true;
}

static bool
truncate(struct rast_ctx *ctx,
         struct rast_vert *v1,
         struct rast_vert *v2)
{
	if (is_out_of_screen(ctx, v1, v2))
		return true;
	if (v1->x < v2->x)
		truncate_x(ctx, v1, v2);
	else
		truncate_x(ctx, v2, v1);
	if (is_out_of_screen(ctx, v1, v2))
		return true;
	if (v1->y < v2->y)
		truncate_y(ctx, v1, v2);
	else
		truncate_y(ctx, v2, v1);
	if (is_out_of_screen(ctx, v1, v2))
		return true;
	if (v1->x < v2->x)
		truncate_x(ctx, v1, v2);
	else
		truncate_x(ctx, v2, v1);
	return is_out_of_screen(ctx, v1, v2);
}

static void
setup_line(struct rast_line *line,
           struct rast_vert *v1,
           struct rast_vert *v2)
{
	line->v1 = v1;
	line->v2 = v2;
	line->x0 = v1->x;
	line->x1 = v2->x;
	line->y0 = v1->y;
	line->y1 = v2->y;
	line->sx = line->x0 < line->x1 ? 1 : -1;
	line->sy = line->y0 < line->y1 ? 1 : -1;
	line->dx = line->x1 - line->x0;
	if (line->dx < 0)
		line->dx = -line->dx;
	line->dy = line->y1 - line->y0;
	if (line->dy > 0)
		line->dy = -line->dy;
	line->orgx = line->x0;
	line->orgy = line->y0;
	line->len = sqrtf(line->dx * line->dx + line->dy * line->dy);
	line->invlen = 1.0f / line->len;
}

static void
rast_frag(struct rast_ctx *ctx,
          struct rast_line *line,
          int32_t x,
          int32_t y,
          float a)
{
	struct rast_vert tmp;
	int32_t fx;
	int32_t fy;
	float f;

	if (a < 0 || a > 1)
		return;
	fx = line->x0 - line->orgx;
	fy = line->y0 - line->orgy;
	f = sqrtf(fx * fx + fy * fy) * line->invlen;
	if (f < 0 || f > 1)
		return;
	rast_vert_mix(ctx, &tmp, line->v1, line->v2, f);
	tmp.x = x;
	tmp.y = y;
	float f2 = 1 - f;
	for (uint32_t i = 0; i < ctx->nvarying; ++i)
	{
		tmp.varying[i].x = f * line->v2->varying[i].x + f2 * line->v1->varying[i].x;
		tmp.varying[i].y = f * line->v2->varying[i].y + f2 * line->v1->varying[i].y;
		tmp.varying[i].z = f * line->v2->varying[i].z + f2 * line->v1->varying[i].z;
		tmp.varying[i].w = f * line->v2->varying[i].w + f2 * line->v1->varying[i].w;
	}
	/* XXX alpha */
	ctx->fragment(ctx, &tmp);
}

static void
rast_aa(struct rast_ctx *ctx,
        struct rast_vert *v1,
        struct rast_vert *v2)
{
	struct rast_line line;
	int32_t err;
	int32_t d;
	int32_t e2;
	int32_t x2;
	int32_t n;

	setup_line(&line, v1, v2);
	if (!line.dx && !line.dy)
	{
		ctx->fragment(ctx, v1);
		return;
	}
	err = line.dx - line.dy;
	for (;;)
	{
		d = err - line.dx + line.dy;
		if (d < 0)
			d = -d;
		rast_frag(ctx, &line, line.x0, line.y0, d * line.invlen);
		if (line.x0 == line.x1 && line.y0 == line.y1)
			break;
		e2 = 2 * err;
		x2 = line.x0;
		if (e2 >= -line.dx)
		{
			n = err + line.dy;
			if (n < line.len)
				rast_frag(ctx, &line, line.x0, line.y0 + line.sy, n * line.invlen);
			err -= line.dy;
			line.x0 += line.sx;
		}
		if (e2 <= line.dy)
		{
			n = line.dx - err;
			if (n < line.len)
				rast_frag(ctx, &line, x2 + line.sx, line.y0, n * line.invlen);
			err += line.dx;
			line.y0 += line.sy;
		}
	}
}

static void
rast_default(struct rast_ctx *ctx,
             struct rast_vert *v1,
             struct rast_vert *v2)
{
	struct rast_line line;
	int32_t err;
	int32_t e2;

	setup_line(&line, v1, v2);
	if (!line.dx && !line.dy)
	{
		ctx->fragment(ctx, v1);
		return;
	}
	err = line.dx + line.dy;
	for (;;)
	{
		rast_frag(ctx, &line, line.x0, line.y0, 0);
		if (line.x0 == line.x1 && line.y0 == line.y1)
			break;
		e2 = 2 * err;
		if (e2 >= line.dy)
		{
			err += line.dy;
			line.x0 += line.sx;
		}
		if (e2 <= line.dx)
		{
			err += line.dx;
			line.y0 += line.sy;
		}
	}
}

void
rast_line(struct rast_ctx *ctx,
          struct rast_vert *v1,
          struct rast_vert *v2)
{
	switch (ctx->fill_front)
	{
		case RAST_FILL_POINT:
			rast_point(ctx, v1);
			rast_point(ctx, v2);
			return;
		default:
			break;
	}
	if (truncate(ctx, v1, v2))
		return;
	if (ctx->line_smooth_enable)
		rast_aa(ctx, v1, v2);
	else
		rast_default(ctx, v1, v2);
}
