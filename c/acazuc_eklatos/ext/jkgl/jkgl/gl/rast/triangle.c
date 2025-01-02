#include "internal.h"

#include <assert.h>
#include <math.h>

struct triangle_ctx;

struct triangle_ctx
{
	GLfloat d0[3];
	GLfloat d1[3];
	GLfloat d[3];
	struct vert *v[3];
	struct vert *first;
	GLboolean front_face;
};

static void compute_ctx(struct triangle_ctx *ctx)
{
	GLfloat e0[2];
	GLfloat e1[2];
	GLfloat e2[2];
	e0[0] = ctx->v[0]->x - ctx->v[2]->x;
	e0[1] = ctx->v[0]->y - ctx->v[2]->y;
	e1[0] = ctx->v[1]->x - ctx->v[0]->x;
	e1[1] = ctx->v[1]->y - ctx->v[0]->y;
	e2[0] = ctx->v[2]->x - ctx->v[1]->x;
	e2[1] = ctx->v[2]->y - ctx->v[1]->y;
	GLfloat d = 1.f / (e0[0] * e1[1] - e1[0] * e0[1]);
	ctx->d0[0] = d * -e2[1];
	ctx->d0[1] = d * -e0[1];
	ctx->d0[2] = d * -e1[1];
	ctx->d1[0] = d *  e2[0];
	ctx->d1[1] = d *  e0[0];
	ctx->d1[2] = d *  e1[0];
	ctx->d[0] = d * (ctx->v[1]->x * ctx->v[2]->y - ctx->v[2]->x * ctx->v[1]->y);
	ctx->d[1] = d * (ctx->v[2]->x * ctx->v[0]->y - ctx->v[0]->x * ctx->v[2]->y);
	ctx->d[2] = d * (ctx->v[0]->x * ctx->v[1]->y - ctx->v[1]->x * ctx->v[0]->y);
}

static GLboolean truncate(struct vert *p1, struct vert *p2, struct vert *p3)
{
	if (p1->w < 0 && p2->w < 0 && p3->w < 0)
		return GL_TRUE;
	if (p1->x < 0 && p2->x < 0 && p3->x < 0)
		return GL_TRUE;
	if (p1->y < 0 && p2->y < 0 && p3->y < 0)
		return GL_TRUE;
	if (p1->x >= g_ctx->width && p2->x >= g_ctx->width && p3->x >= g_ctx->width)
		return GL_TRUE;
	if (p1->y >= g_ctx->height && p2->y >= g_ctx->height && p3->y >= g_ctx->height)
		return GL_TRUE;
	return GL_FALSE;
}

static void draw_smooth(struct triangle_ctx *ctx, GLint x, GLint y)
{
	struct vert tmp;
	GLfloat bary[3];
	GLfloat q[3];
	GLfloat qd;

	tmp.front_face = ctx->front_face;
	tmp.x = x + .5f;
	tmp.y = y + .5f;
	bary[0] = tmp.x * ctx->d0[0] + tmp.y * ctx->d1[0] + ctx->d[0];
	bary[1] = tmp.x * ctx->d0[1] + tmp.y * ctx->d1[1] + ctx->d[1];
	bary[2] = tmp.x * ctx->d0[2] + tmp.y * ctx->d1[2] + ctx->d[2];
	tmp.z = bary[0] * ctx->v[0]->z + bary[1] * ctx->v[1]->z + bary[2] * ctx->v[2]->z;
	if (!rast_depth_test(x, y, tmp.z))
		return;
	tmp.w = bary[0] * ctx->v[0]->w + bary[1] * ctx->v[1]->w + bary[2] * ctx->v[2]->w;
	q[0] = bary[0] / ctx->v[0]->w;
	q[1] = bary[1] / ctx->v[1]->w;
	q[2] = bary[2] / ctx->v[2]->w;
	qd = 1.f / (q[0] + q[1] + q[2]);
	for (GLuint i = 0; i < g_ctx->vs.varying_nb; ++i)
		tmp.varying[i] = (q[0] * ctx->v[0]->varying[i]
		                + q[1] * ctx->v[1]->varying[i]
		                + q[2] * ctx->v[2]->varying[i]) * qd;
	rast_fragment(&tmp);
}

static void render_line(struct triangle_ctx *ctx, GLint y, GLfloat xl, GLfloat xr)
{
	GLint minx = lroundf(xl);
	GLint maxx = lroundf(xr);
	if (minx < 0)
		minx = 0;
	if (maxx > g_ctx->width)
		maxx = g_ctx->width;
	for (GLint x = minx; x < maxx; ++x)
		draw_smooth(ctx, x, y);
}

static void render_span(struct triangle_ctx *ctx, struct vert *v1,
                        struct vert *v2, struct vert *v3,
                        GLfloat y0, GLfloat y1)
{
	if (v2->x > v3->x)
	{
		struct vert *tmpv = v2;
		v2 = v3;
		v3 = tmpv;
	}
	GLint miny = lroundf(y0);
	GLint maxy = lroundf(y1);
	if (miny < 0)
		miny = 0;
	if (maxy > g_ctx->height)
		maxy = g_ctx->height;
	GLfloat dxl = v2->x - v1->x;
	GLfloat dxr = v3->x - v1->x;
	GLfloat dyl = v2->y - v1->y;
	GLfloat dyr = v3->y - v1->y;
	for (GLint y = miny; y < maxy; ++y)
	{
		GLfloat dy = y - v1->y;
		GLfloat xl = v1->x + dxl * (dy / dyl);
		GLfloat xr = v1->x + dxr * (dy / dyr);
		render_line(ctx, y, xl, xr);
	}
}

static void render_not_flat(struct vert *v1, struct vert *v2, struct vert *v3, struct triangle_ctx *ctx)
{
	struct vert new;
	GLfloat f = (v2->y - v1->y) / (v3->y - v1->y);
	new.x = mixf(v1->x, v3->x, f);
	new.y = mixf(v1->y, v3->y, f);
	render_span(ctx, v3, &new, v2, v2->y, v3->y);
	render_span(ctx, v1, v2, &new, v1->y, v2->y);
}

static void get_vertices_sorted(struct triangle_ctx *ctx)
{
	struct vert *tmp;

	if (ctx->v[0]->y > ctx->v[1]->y)
	{
		tmp = ctx->v[0];
		ctx->v[0] = ctx->v[1];
		ctx->v[1] = tmp;
	}
	if (ctx->v[0]->y > ctx->v[2]->y)
	{
		tmp = ctx->v[0];
		ctx->v[0] = ctx->v[2];
		ctx->v[2] = tmp;
	}
	if (ctx->v[1]->y > ctx->v[2]->y)
	{
		tmp = ctx->v[1];
		ctx->v[1] = ctx->v[2];
		ctx->v[2] = tmp;
	}
}

void rast_triangle(struct vert *v1, struct vert *v2, struct vert *v3)
{
	struct triangle_ctx ctx;

	if (truncate(v1, v2, v3))
		return;
	ctx.front_face = (v2->x - v1->x) * (v3->y - v2->y) - (v2->y - v1->y) * (v3->x - v2->x) > 0;
	if (g_ctx->front_face == GL_CCW)
		ctx.front_face = !ctx.front_face;
	if (g_ctx->enable_cull)
	{
		if (g_ctx->cull_face == GL_FRONT_AND_BACK)
			return;
		if (ctx.front_face == (g_ctx->cull_face == GL_FRONT))
			return;
	}
	ctx.first = v1;
	ctx.v[0] = v1;
	ctx.v[1] = v2;
	ctx.v[2] = v3;
	get_vertices_sorted(&ctx);
	compute_ctx(&ctx);
	if (ctx.v[1]->y == ctx.v[2]->y)
		render_span(&ctx, ctx.v[0], ctx.v[1], ctx.v[2], ctx.v[0]->y, ctx.v[1]->y);
	else if (ctx.v[0]->y == ctx.v[1]->y)
		render_span(&ctx, ctx.v[2], ctx.v[1], ctx.v[0], ctx.v[0]->y, ctx.v[2]->y);
	else
		render_not_flat(ctx.v[0], ctx.v[1], ctx.v[2], &ctx);
}
