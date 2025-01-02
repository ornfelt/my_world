#include "internal.h"

#include <string.h>
#include <math.h>

void rast_point(struct vert *vert)
{
	if (g_ctx->point_size == 1)
	{
		struct vert tmp;

		memcpy(&tmp, vert, sizeof(tmp));
		tmp.front_face = GL_TRUE;
		rast_fragment(&tmp);
		return;
	}
	GLfloat minx;
	GLfloat maxx;
	GLfloat miny;
	GLfloat maxy;
	miny = vert->y - g_ctx->point_size * .5f;
	maxy = vert->y + g_ctx->point_size * .5f;
	minx = vert->x - g_ctx->point_size * .5f;
	maxx = vert->x + g_ctx->point_size * .5f;
	if (miny < 0)
		miny = 0;
	else if (miny >= g_ctx->height)
		return;
	if (maxy > g_ctx->height)
		maxy = g_ctx->height;
	else if (maxy < 0)
		return;
	if (minx < 0)
		minx = 0;
	else if (minx >= g_ctx->width)
		return;
	if (maxx > g_ctx->width)
		maxx = g_ctx->width;
	else if (maxx < 0)
		return;
	for (GLfloat y = miny; y < maxy; y++)
	{
		for (GLfloat x = minx; x < maxx; x++)
		{
			struct vert tmp;

			if (g_ctx->point_smooth)
			{
				GLfloat dx = x - vert->x;
				GLfloat dy = y - vert->y;
				if (sqrtf(dx * dx + dy * dy) >= g_ctx->point_size * .5f)
					continue;
			}
			if (!rast_depth_test(x, y, vert->z))
				return;
			memcpy(&tmp, vert, sizeof(tmp));
			tmp.front_face = GL_TRUE;
			tmp.x = x;
			tmp.y = y;
			rast_fragment(&tmp);
		}
	}
}
