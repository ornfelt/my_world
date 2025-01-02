#include "internal.h"

#include <math.h>

void rast_normalize_vert(struct vert *vert)
{
	if (vert->w != 0)
	{
		GLfloat f = 1.f / vert->w;
		vert->x *= f;
		vert->y *= f;
		vert->z *= f;
	}
	vert->x = roundf((1 + vert->x) * g_ctx->width * .5f);
	vert->y = roundf((1 + vert->y) * g_ctx->height * .5f);
}
