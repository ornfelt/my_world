#include "rast.h"

#include <string.h>
#include <math.h>

void
rast_point(struct rast_ctx *ctx,
           struct rast_vert *vert)
{
	float min_x;
	float max_x;
	float min_y;
	float max_y;

	if (ctx->point_size == 1)
	{
		struct rast_vert tmp;

		memcpy(&tmp, vert, sizeof(tmp));
		tmp.front_face = true;
		ctx->fragment(ctx, &tmp);
		return;
	}

	min_y = vert->y - ctx->point_size * 0.5f;
	max_y = vert->y + ctx->point_size * 0.5f;
	min_x = vert->x - ctx->point_size * 0.5f;
	max_x = vert->x + ctx->point_size * 0.5f;
	if (min_y < ctx->min_y)
		min_y = ctx->min_y;
	else if (min_y > ctx->max_y)
		return;
	if (max_y > ctx->max_y)
		max_y = ctx->max_y;
	else if (max_y < ctx->min_y)
		return;
	if (min_x < ctx->min_x)
		min_x = ctx->min_x;
	else if (min_x >= ctx->max_x)
		return;
	if (max_x > ctx->max_x)
		max_x = ctx->max_x;
	else if (max_x < ctx->min_x)
		return;
	for (float y = min_y; y < max_y; y++)
	{
		for (float x = min_x; x < max_x; x++)
		{
			struct rast_vert tmp;

			if (ctx->point_smooth_enable)
			{
				float dx = x - vert->x;
				float dy = y - vert->y;
				if (sqrtf(dx * dx + dy * dy) >= ctx->point_size * 0.5f)
					continue;
			}
			memcpy(&tmp, vert, sizeof(tmp));
			tmp.front_face = true;
			tmp.x = x;
			tmp.y = y;
			ctx->fragment(ctx, &tmp);
		}
	}
}
