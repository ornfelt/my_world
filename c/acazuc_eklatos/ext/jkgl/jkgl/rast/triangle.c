#include "utils.h"
#include "rast.h"

#include <assert.h>
#include <math.h>

struct rast_triangle
{
	float d0[3];
	float d1[3];
	float d[3];
	struct rast_vert *v[3];
	struct rast_vert *first;
	bool front_face;
};

static void
compute_ctx(struct rast_triangle *triangle)
{
	float e0[2];
	float e1[2];
	float e2[2];
	float d;

	e0[0] = triangle->v[0]->x - triangle->v[2]->x;
	e0[1] = triangle->v[0]->y - triangle->v[2]->y;
	e1[0] = triangle->v[1]->x - triangle->v[0]->x;
	e1[1] = triangle->v[1]->y - triangle->v[0]->y;
	e2[0] = triangle->v[2]->x - triangle->v[1]->x;
	e2[1] = triangle->v[2]->y - triangle->v[1]->y;
	d = 1.0f / (e0[0] * e1[1] - e1[0] * e0[1]);
	triangle->d0[0] = d * -e2[1];
	triangle->d0[1] = d * -e0[1];
	triangle->d0[2] = d * -e1[1];
	triangle->d1[0] = d *  e2[0];
	triangle->d1[1] = d *  e0[0];
	triangle->d1[2] = d *  e1[0];
	triangle->d[0] = d * (triangle->v[1]->x * triangle->v[2]->y - triangle->v[2]->x * triangle->v[1]->y);
	triangle->d[1] = d * (triangle->v[2]->x * triangle->v[0]->y - triangle->v[0]->x * triangle->v[2]->y);
	triangle->d[2] = d * (triangle->v[0]->x * triangle->v[1]->y - triangle->v[1]->x * triangle->v[0]->y);
}

static bool
truncate(struct rast_ctx *ctx,
         struct rast_vert *p1,
         struct rast_vert *p2,
         struct rast_vert *p3)
{
	if (p1->w < 0
	 && p2->w < 0
	 && p3->w < 0)
		return true;
	if (p1->x < ctx->min_x
	 && p2->x < ctx->min_x
	 && p3->x < ctx->min_x)
		return true;
	if (p1->y < ctx->min_y
	 && p2->y < ctx->min_y
	 && p3->y < ctx->min_y)
		return true;
	if (p1->x > ctx->max_x
	 && p2->x > ctx->max_x
	 && p3->x > ctx->max_x)
		return true;
	if (p1->y > ctx->max_y
	 && p2->y > ctx->max_y
	 && p3->y > ctx->max_y)
		return true;
	return false;
}

static void
draw_smooth(struct rast_ctx *ctx,
            struct rast_triangle *triangle,
            int32_t x,
            int32_t y)
{
	struct rast_vert tmp;
	float bary[3];
	float q[3];
	float qd;

	tmp.front_face = triangle->front_face;
	tmp.x = x + 0.5f;
	tmp.y = y + 0.5f;
	bary[0] = tmp.x * triangle->d0[0] + tmp.y * triangle->d1[0] + triangle->d[0];
	bary[1] = tmp.x * triangle->d0[1] + tmp.y * triangle->d1[1] + triangle->d[1];
	bary[2] = tmp.x * triangle->d0[2] + tmp.y * triangle->d1[2] + triangle->d[2];
	tmp.z = bary[0] * triangle->v[0]->z
	      + bary[1] * triangle->v[1]->z
	      + bary[2] * triangle->v[2]->z;
	tmp.w = bary[0] * triangle->v[0]->w
	      + bary[1] * triangle->v[1]->w
	      + bary[2] * triangle->v[2]->w;
	q[0] = bary[0] / triangle->v[0]->w;
	q[1] = bary[1] / triangle->v[1]->w;
	q[2] = bary[2] / triangle->v[2]->w;
	qd = 1.0f / (q[0] + q[1] + q[2]);
	q[0] *= qd;
	q[1] *= qd;
	q[2] *= qd;
	for (uint32_t i = 0; i < ctx->nvarying; ++i)
	{
		tmp.varying[i].x = q[0] * triangle->v[0]->varying[i].x
		                 + q[1] * triangle->v[1]->varying[i].x
		                 + q[2] * triangle->v[2]->varying[i].x;
		tmp.varying[i].y = q[0] * triangle->v[0]->varying[i].y
		                 + q[1] * triangle->v[1]->varying[i].y
		                 + q[2] * triangle->v[2]->varying[i].y;
		tmp.varying[i].z = q[0] * triangle->v[0]->varying[i].z
		                 + q[1] * triangle->v[1]->varying[i].z
		                 + q[2] * triangle->v[2]->varying[i].z;
		tmp.varying[i].w = q[0] * triangle->v[0]->varying[i].w
		                 + q[1] * triangle->v[1]->varying[i].w
		                 + q[2] * triangle->v[2]->varying[i].w;
	}
	ctx->fragment(ctx, &tmp);
}

static void
render_line(struct rast_ctx *ctx,
            struct rast_triangle *triangle,
            int32_t y,
            float xl,
            float xr)
{
	int32_t min_x;
	int32_t max_x;

	min_x = lroundf(xl);
	if (min_x < ctx->min_x)
		min_x = ctx->min_x;
	max_x = lroundf(xr);
	if (max_x > ctx->max_x)
		max_x = ctx->max_x;
	for (int32_t x = min_x; x < max_x; ++x)
		draw_smooth(ctx, triangle, x, y);
}

static void
render_span(struct rast_ctx *ctx,
            struct rast_triangle *triangle,
            struct rast_vert *v1,
            struct rast_vert *v2,
            struct rast_vert *v3,
            float y0,
            float y1)
{
	int32_t min_y;
	int32_t max_y;
	float dxl;
	float dxr;
	float dyl;
	float dyr;

	if (v2->x > v3->x)
	{
		struct rast_vert *tmpv = v2;
		v2 = v3;
		v3 = tmpv;
	}
	min_y = lroundf(y0);
	if (min_y < ctx->min_y)
		min_y = ctx->min_y;
	max_y = lroundf(y1);
	if (max_y > ctx->max_y)
		max_y = ctx->max_y;
	dxl = v2->x - v1->x;
	dxr = v3->x - v1->x;
	dyl = v2->y - v1->y;
	dyr = v3->y - v1->y;
	for (int32_t y = min_y; y < max_y; ++y)
	{
		float dy = y - v1->y;
		float xl = v1->x + dxl * (dy / dyl);
		float xr = v1->x + dxr * (dy / dyr);
		render_line(ctx, triangle, y, xl, xr);
	}
}

static void
render_not_flat(struct rast_ctx *ctx,
                struct rast_triangle *triangle,
                struct rast_vert *v1,
                struct rast_vert *v2,
                struct rast_vert *v3)
{
	struct rast_vert new;
	float f;

	f = (v2->y - v1->y) / (v3->y - v1->y);
	new.x = mixf(v1->x, v3->x, f);
	new.y = mixf(v1->y, v3->y, f);
	render_span(ctx, triangle, v3, &new, v2, v2->y, v3->y);
	render_span(ctx, triangle, v1, v2, &new, v1->y, v2->y);
}

static void
get_vertices_sorted(struct rast_triangle *triangle)
{
	struct rast_vert *tmp;

	if (triangle->v[0]->y > triangle->v[1]->y)
	{
		tmp = triangle->v[0];
		triangle->v[0] = triangle->v[1];
		triangle->v[1] = tmp;
	}
	if (triangle->v[0]->y > triangle->v[2]->y)
	{
		tmp =triangle->v[0];
		triangle->v[0] = triangle->v[2];
		triangle->v[2] = tmp;
	}
	if (triangle->v[1]->y > triangle->v[2]->y)
	{
		tmp = triangle->v[1];
		triangle->v[1] = triangle->v[2];
		triangle->v[2] = tmp;
	}
}

void
rast_triangle(struct rast_ctx *ctx,
              struct rast_vert *v1,
              struct rast_vert *v2,
              struct rast_vert *v3)
{
	struct rast_triangle triangle;

	if (truncate(ctx, v1, v2, v3))
		return;
	triangle.front_face = (v2->x - v1->x) * (v3->y - v2->y) - (v2->y - v1->y) * (v3->x - v2->x) > 0;
	if (ctx->front_ccw)
		triangle.front_face = !triangle.front_face;
	if (ctx->cull_face & (1 << triangle.front_face))
		return;
	/* XXX
	 * this is not valid, we should render before truncation
	 */
	switch (triangle.front_face ? ctx->fill_front : ctx->fill_back)
	{
		case RAST_FILL_POINT:
			rast_point(ctx, v1);
			rast_point(ctx, v2);
			rast_point(ctx, v3);
			return;
		case RAST_FILL_LINE:
			rast_line(ctx, v1, v2);
			rast_line(ctx, v2, v3);
			rast_line(ctx, v3, v1);
			return;
		default:
		case RAST_FILL_SOLID:
			break;
	}
	triangle.first = v1;
	triangle.v[0] = v1;
	triangle.v[1] = v2;
	triangle.v[2] = v3;
	get_vertices_sorted(&triangle);
	compute_ctx(&triangle);
	if (triangle.v[1]->y == triangle.v[2]->y)
		render_span(ctx, &triangle, triangle.v[0], triangle.v[1], triangle.v[2], triangle.v[0]->y, triangle.v[1]->y);
	else if (triangle.v[0]->y == triangle.v[1]->y)
		render_span(ctx, &triangle, triangle.v[2], triangle.v[1], triangle.v[0], triangle.v[0]->y, triangle.v[2]->y);
	else
		render_not_flat(ctx, &triangle, triangle.v[0], triangle.v[1], triangle.v[2]);
}
