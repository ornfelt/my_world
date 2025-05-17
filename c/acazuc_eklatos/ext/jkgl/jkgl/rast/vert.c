#include "utils.h"
#include "rast.h"

#include <math.h>

void
rast_vert_mix(struct rast_ctx *ctx,
              struct rast_vert *dst,
              struct rast_vert *v1,
              struct rast_vert *v2,
              float f)
{
	dst->front_face = v1->front_face;
	dst->x = mixf(v1->x, v2->x, f);
	dst->y = mixf(v1->y, v2->y, f);
	dst->z = mixf(v1->z, v2->z, f);
	dst->w = mixf(v1->w, v2->w, f);
	for (uint32_t i = 0; i < ctx->nattribs; ++i)
	{
		dst->attribs[i].x = mixf(v1->attribs[i].x, v2->attribs[i].x, f);
		dst->attribs[i].y = mixf(v1->attribs[i].y, v2->attribs[i].y, f);
		dst->attribs[i].z = mixf(v1->attribs[i].z, v2->attribs[i].z, f);
		dst->attribs[i].w = mixf(v1->attribs[i].w, v2->attribs[i].w, f);
	}
}

void
rast_vert_normalize(struct rast_vert *vert,
                    float min_x,
                    float min_y,
                    float max_x,
                    float max_y)
{
	if (vert->w != 0)
	{
		float f = 1.0f / vert->w;
		vert->x *= f;
		vert->y *= f;
		vert->z *= f;
	}
	vert->x = roundf((1 + vert->x) * ((max_x - min_x) * 0.5f) + min_x);
	vert->y = roundf((1 + vert->y) * ((max_y - min_y) * 0.5f) + min_y);
}
