#include "rast.h"

static bool
get_vert(struct rast_ctx *ctx,
         struct rast_vert *vert,
         uint32_t instance,
         uint32_t *index)
{
	vert->front_face = true; /* XXX */
	return ctx->get_vert(ctx, instance, (*index)++, vert);
}

static void
points(struct rast_ctx *ctx)
{
	struct rast_vert v[1];
	uint32_t index;
	uint32_t last;

	for (uint32_t instance = ctx->instance_start;
	     instance < ctx->instance_start + ctx->instance_count;
	     instance++)
	{
		last = ctx->start + ctx->count;
		index = ctx->start;
restart:
		while (last - index >= 1)
		{
			if (get_vert(ctx, &v[0], instance, &index))
				goto restart;
			rast_point(ctx, &v[0]);
		}
	}
}

static void
line_strip(struct rast_ctx *ctx)
{
	struct rast_vert v[2];
	uint32_t index;
	uint32_t last;

	for (uint32_t instance = ctx->instance_start;
	     instance < ctx->instance_start + ctx->instance_count;
	     instance++)
	{
		last = ctx->start + ctx->count;
		index = ctx->start;
restart:
		if (last - index < 2)
			continue;
		if (get_vert(ctx, &v[0], instance, &index))
			goto restart;
		while (last - index >= 1)
		{
			if (get_vert(ctx, &v[1], instance, &index))
				goto restart;
			rast_line(ctx, &v[0], &v[1]);
			v[0] = v[1];
		}
	}
}

static void
line_loop(struct rast_ctx *ctx)
{
	struct rast_vert v[3];
	uint32_t index;
	uint32_t last;
	bool loop;

	for (uint32_t instance = ctx->instance_start;
	     instance < ctx->instance_start + ctx->instance_count;
	     instance++)
	{
		last = ctx->start + ctx->count;
		index = ctx->start;
restart:
		if (last - index < 2)
			continue;
		loop = last - index > 2;
		if (get_vert(ctx, &v[1], instance, &index))
			goto restart;
		v[0] = v[1];
		while (last - index >= 1)
		{
			if (get_vert(ctx, &v[2], instance, &index))
				goto restart;
			rast_line(ctx, &v[1], &v[2]);
			v[1] = v[2];
		}
		if (loop)
			rast_line(ctx, &v[1], &v[0]);
	}
}

static void
lines(struct rast_ctx *ctx)
{
	struct rast_vert v[2];
	uint32_t index;
	uint32_t last;

	for (uint32_t instance = ctx->instance_start;
	     instance < ctx->instance_start + ctx->instance_count;
	     instance++)
	{
		last = ctx->start + ctx->count;
		index = ctx->start;
restart:
		while (last - index >= 2)
		{
			if (get_vert(ctx, &v[0], instance, &index)
			 || get_vert(ctx, &v[1], instance, &index))
				goto restart;
			rast_line(ctx, &v[0], &v[1]);
		}
	}
}

static void
triangle_strip(struct rast_ctx *ctx)
{
	struct rast_vert v[3];
	uint32_t index;
	uint32_t last;

	for (uint32_t instance = ctx->instance_start;
	     instance < ctx->instance_start + ctx->instance_count;
	     instance++)
	{
		last = ctx->start + ctx->count;
		index = ctx->start;
restart:
		if (last - index < 3)
			continue;
		if (get_vert(ctx, &v[0], instance, &index)
		 || get_vert(ctx, &v[1], instance, &index))
			goto restart;
		while (last - index >= 1)
		{
			if (get_vert(ctx, &v[2], instance, &index))
				goto restart;
			rast_triangle(ctx, &v[0], &v[1], &v[2]);
			v[0] = v[1];
			v[1] = v[2];
		}
	}
}

static void
triangle_fan(struct rast_ctx *ctx)
{
	struct rast_vert v[3];
	uint32_t index;
	uint32_t last;

	for (uint32_t instance = ctx->instance_start;
	     instance < ctx->instance_start + ctx->instance_count;
	     instance++)
	{
		last = ctx->start + ctx->count;
		index = ctx->start;
restart:
		if (last - index < 3)
			continue;
		if (get_vert(ctx, &v[0], instance, &index)
		 || get_vert(ctx, &v[1], instance, &index))
			goto restart;
		while (last - index >= 1)
		{
			if (get_vert(ctx, &v[2], instance, &index))
				goto restart;
			rast_triangle(ctx, &v[0], &v[1], &v[2]);
			v[1] = v[2];
		}
	}
}

static void
triangles(struct rast_ctx *ctx)
{
	struct rast_vert v[3];
	uint32_t index;
	uint32_t last;

	for (uint32_t instance = ctx->instance_start;
	     instance < ctx->instance_start + ctx->instance_count;
	     instance++)
	{
		last = ctx->start + ctx->count;
		index = ctx->start;
restart:
		while (last - index >= 3)
		{
			if (get_vert(ctx, &v[0], instance, &index)
			 || get_vert(ctx, &v[1], instance, &index)
			 || get_vert(ctx, &v[2], instance, &index))
				goto restart;
			rast_triangle(ctx, &v[0], &v[1], &v[2]);
		}
	}
}

static void
quads(struct rast_ctx *ctx)
{
	struct rast_vert v[4];
	uint32_t index;
	uint32_t last;

	for (uint32_t instance = ctx->instance_start;
	     instance < ctx->instance_start + ctx->instance_count;
	     instance++)
	{
		last = ctx->start + ctx->count;
		index = ctx->start;
restart:
		while (last - index >= 4)
		{
			if (get_vert(ctx, &v[0], instance, &index)
			 || get_vert(ctx, &v[1], instance, &index)
			 || get_vert(ctx, &v[2], instance, &index)
			 || get_vert(ctx, &v[3], instance, &index))
				goto restart;
			rast_triangle(ctx, &v[0], &v[1], &v[2]);
			rast_triangle(ctx, &v[2], &v[3], &v[0]);
		}
	}
}

static void
quad_strip(struct rast_ctx *ctx)
{
	struct rast_vert v[4];
	uint32_t index;
	uint32_t last;

	for (uint32_t instance = ctx->instance_start;
	     instance < ctx->instance_start + ctx->instance_count;
	     instance++)
	{
		last = ctx->start + ctx->count;
		index = ctx->start;
restart:
		if (last - index < 4)
			continue;
		if (get_vert(ctx, &v[0], instance, &index)
		 || get_vert(ctx, &v[1], instance, &index))
			goto restart;
		while (last - index >= 2)
		{
			if (get_vert(ctx, &v[2], instance, &index)
			 || get_vert(ctx, &v[3], instance, &index))
				goto restart;
			rast_triangle(ctx, &v[0], &v[1], &v[3]);
			rast_triangle(ctx, &v[3], &v[2], &v[0]);
			v[0] = v[2];
			v[1] = v[3];
		}
	}
}

void
rast_draw(struct rast_ctx *ctx)
{
	if (!ctx->instance_count)
		ctx->instance_count = 1;
	switch (ctx->primitive)
	{
		case RAST_PRIMITIVE_POINTS:
			points(ctx);
			break;
		case RAST_PRIMITIVE_LINE_STRIP:
			line_strip(ctx);
			break;
		case RAST_PRIMITIVE_LINE_LOOP:
			line_loop(ctx);
			break;
		case RAST_PRIMITIVE_LINES:
			lines(ctx);
			break;
		case RAST_PRIMITIVE_TRIANGLE_STRIP:
			triangle_strip(ctx);
			break;
		case RAST_PRIMITIVE_TRIANGLE_FAN:
			triangle_fan(ctx);
			break;
		case RAST_PRIMITIVE_TRIANGLES:
			triangles(ctx);
			break;
		case RAST_PRIMITIVE_QUADS:
			quads(ctx);
			break;
		case RAST_PRIMITIVE_QUAD_STRIP:
			quad_strip(ctx);
			break;
	}
}
