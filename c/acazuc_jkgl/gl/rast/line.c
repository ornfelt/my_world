#include "internal.h"

#include <math.h>

struct line_ctx
{
	struct vert *v1;
	struct vert *v2;
	GLint orgx;
	GLint orgy;
	GLint x0;
	GLint y0;
	GLint x1;
	GLint y1;
	GLint dx;
	GLint dy;
	GLint sx;
	GLint sy;
	GLfloat len;
	GLfloat invlen;
};

static GLboolean is_out_of_screen(struct vert *v1, struct vert *v2)
{
	if (v1->z < 0 && v2->z < 0)
		return GL_TRUE;
	if (v1->z > 1 || v2->z > 1)
		return GL_TRUE;
	if (v1->x < 0 && v2->x < 0)
		return GL_TRUE;
	if (v1->x >= g_ctx->width && v2->x >= g_ctx->width)
		return GL_TRUE;
	if (v1->y < 0 && v2->y < 0)
		return GL_TRUE;
	if (v1->y >= g_ctx->height && v2->y >= g_ctx->height)
		return GL_TRUE;
	return GL_FALSE;
}

static GLboolean truncate_x(struct vert *v1, struct vert *v2)
{
	if (v1->x < 0)
	{
		GLfloat f = -v1->x / (v2->x - v1->x);
		vert_mix(v1, v1, v2, f);
		v1->x = 0;
	}
	if (v2->x >= g_ctx->width)
	{
		GLfloat f = (g_ctx->width - 1 - v1->x) / (v2->x - v1->x);
		vert_mix(v2, v1, v2, f);
		v2->x = g_ctx->width - 1;
	}
	return GL_TRUE;
}

static GLboolean truncate_y(struct vert *v1, struct vert *v2)
{
	if (v1->y < 0)
	{
		GLfloat f = -v1->y / (v2->y - v1->y);
		vert_mix(v1, v1, v2, f);
		v1->y = 0;
	}
	if (v2->y >= g_ctx->height)
	{
		GLfloat f = (g_ctx->height - 1 - v1->y) / (v2->y - v1->y);
		vert_mix(v2, v1, v2, f);
		v2->y = g_ctx->height - 1;
	}
	return GL_TRUE;
}

static GLboolean truncate(struct vert *v1, struct vert *v2)
{
	if (is_out_of_screen(v1, v2))
		return GL_TRUE;
	if (v1->x < v2->x)
		truncate_x(v1, v2);
	else
		truncate_x(v2, v1);
	if (is_out_of_screen(v1, v2))
		return GL_TRUE;
	if (v1->y < v2->y)
		truncate_y(v1, v2);
	else
		truncate_y(v2, v1);
	if (is_out_of_screen(v1, v2))
		return GL_TRUE;
	if (v1->x < v2->x)
		truncate_x(v1, v2);
	else
		truncate_x(v2, v1);
	return is_out_of_screen(v1, v2);
}

static void setup_ctx(struct line_ctx *ctx, struct vert *v1, struct vert *v2)
{
	ctx->v1 = v1;
	ctx->v2 = v2;
	ctx->x0 = v1->x;
	ctx->x1 = v2->x;
	ctx->y0 = v1->y;
	ctx->y1 = v2->y;
	ctx->sx = ctx->x0 < ctx->x1 ? 1 : -1;
	ctx->sy = ctx->y0 < ctx->y1 ? 1 : -1;
	ctx->dx = ctx->x1 - ctx->x0;
	if (ctx->dx < 0)
		ctx->dx = -ctx->dx;
	ctx->dy = ctx->y1 - ctx->y0;
	if (ctx->dy > 0)
		ctx->dy = -ctx->dy;
	ctx->orgx = ctx->x0;
	ctx->orgy = ctx->y0;
	ctx->len = sqrtf(ctx->dx * ctx->dx + ctx->dy * ctx->dy);
	ctx->invlen = 1.f / ctx->len;
}

static void rast_frag(struct line_ctx *ctx, GLint x, GLint y, GLfloat a)
{
	if (a < 0 || a > 1)
		return;
	struct vert tmp;
	GLint fx = ctx->x0 - ctx->orgx;
	GLint fy = ctx->y0 - ctx->orgy;
	GLfloat f = sqrtf(fx * fx + fy * fy) * ctx->invlen;
	if (f < 0 || f > 1)
		return;
	vert_mix(&tmp, ctx->v1, ctx->v2, f);
	tmp.x = x;
	tmp.y = y;
	if (!rast_depth_test(tmp.x, tmp.y, tmp.z))
		return;
	GLfloat f2 = 1 - f;
	for (GLuint i = 0; i < g_ctx->vs.varying_nb; ++i)
		tmp.varying[i] = f * ctx->v2->varying[i] + f2 * ctx->v1->varying[i];
	/* XXX alpha */
	rast_fragment(&tmp);
}

static void rast_aa(struct vert *v1, struct vert *v2)
{
	struct line_ctx ctx;
	setup_ctx(&ctx, v1, v2);
	if (!ctx.dx && !ctx.dy)
	{
		if (!rast_depth_test(v1->x, v1->y, v1->z))
			return;
		rast_fragment(v1);
		return;
	}
	GLint err = ctx.dx - ctx.dy;
	for (;;)
	{
		GLint d = err - ctx.dx + ctx.dy;
		if (d < 0)
			d = -d;
		rast_frag(&ctx, ctx.x0, ctx.y0, d * ctx.invlen);
		if (ctx.x0 == ctx.x1 && ctx.y0 == ctx.y1)
			break;
		GLint e2 = 2 * err;
		GLint x2 = ctx.x0;
		if (e2 >= -ctx.dx)
		{
			GLint n = err + ctx.dy;
			if (n < ctx.len)
				rast_frag(&ctx, ctx.x0, ctx.y0 + ctx.sy, n * ctx.invlen);
			err -= ctx.dy;
			ctx.x0 += ctx.sx;
		}
		if (e2 <= ctx.dy)
		{
			GLint n = ctx.dx - err;
			if (n < ctx.len)
				rast_frag(&ctx, x2 + ctx.sx, ctx.y0, n * ctx.invlen);
			err += ctx.dx;
			ctx.y0 += ctx.sy;
		}
	}
}

static void rast_default(struct vert *v1, struct vert *v2)
{
	struct line_ctx ctx;
	setup_ctx(&ctx, v1, v2);
	if (!ctx.dx && !ctx.dy)
	{
		if (!rast_depth_test(v1->x, v1->y, v1->z))
			return;
		rast_fragment(v1);
		return;
	}
	GLint err = ctx.dx + ctx.dy;
	for (;;)
	{
		rast_frag(&ctx, ctx.x0, ctx.y0, 0);
		if (ctx.x0 == ctx.x1 && ctx.y0 == ctx.y1)
			break;
		GLint e2 = 2 * err;
		if (e2 >= ctx.dy)
		{
			err += ctx.dy;
			ctx.x0 += ctx.sx;
		}
		if (e2 <= ctx.dx)
		{
			err += ctx.dx;
			ctx.y0 += ctx.sy;
		}
	}
}

void rast_line(struct vert *v1, struct vert *v2)
{
	if (truncate(v1, v2))
		return;
	if (g_ctx->line_smooth)
		rast_aa(v1, v2);
	else
		rast_default(v1, v2);
}
