#include "internal.h"

#include <string.h>

void glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height,
                  GLenum format, GLenum type, GLvoid *data)
{
	if (type == GL_FLOAT)
	{
		for (GLsizei yy = 0; yy < height; ++yy)
		{
			for (GLsizei xx = 0; xx < width; ++xx)
			{
				GLfloat *src = &g_ctx->color_buffer[(x + xx + (y + yy) * g_ctx->width) * 4];
				GLfloat *dst = &((GLfloat*)data)[(xx + yy * width) * 4];
				memcpy(dst, src, 16);
			}
		}
	}
	else if (type == GL_UNSIGNED_BYTE)
	{
		for (GLsizei yy = 0; yy < height; ++yy)
		{
			for (GLsizei xx = 0; xx < width; ++xx)
			{
				GLfloat *src = &g_ctx->color_buffer[(x + xx + (y + yy) * g_ctx->width) * 4];
				GLubyte *dst = &((GLubyte*)data)[(xx + yy * width) * 4];
				dst[0] = clampf(src[0], 0, 1) * 0xFF;
				dst[1] = clampf(src[1], 0, 1) * 0xFF;
				dst[2] = clampf(src[2], 0, 1) * 0xFF;
				dst[3] = clampf(src[3], 0, 1) * 0xFF;
			}
		}
	}
}
