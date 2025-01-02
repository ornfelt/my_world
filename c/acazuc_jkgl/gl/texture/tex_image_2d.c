#include "internal.h"

#include <string.h>
#include <assert.h>
#include <stdlib.h>

void glTexImage2D(GLenum target, GLint level, GLint internalformat,
                  GLsizei width, GLsizei height, GLint border,
                  GLenum format, GLenum type, const GLvoid *data)
{
	struct texture *tex;
	GLuint texture;

	switch (target)
	{
		case GL_TEXTURE_1D:
			texture = g_ctx->texture_binding_1d;
			break;
		case GL_TEXTURE_2D:
			texture = g_ctx->texture_binding_2d;
			break;
		case GL_TEXTURE_3D:
			texture = g_ctx->texture_binding_3d;
			break;
		default:
			g_ctx->errno = GL_INVALID_ENUM;
			return;
	}
	if (width < 0 || height < 0 || border != 0)
	{
		g_ctx->errno = GL_INVALID_VALUE;
		return;
	}
	if (!texture || texture >= g_ctx->textures_capacity)
	{
		g_ctx->errno = GL_INVALID_VALUE;
		return;
	}
	if (!g_ctx->textures[texture])
	{
		g_ctx->errno = GL_INVALID_VALUE;
		return;
	}
	tex = g_ctx->textures[texture];
	if (!tex->initialized)
	{
		tex->initialized = GL_TRUE;
		tex->target = target;
	}
	else if (tex->target != target)
	{
		g_ctx->errno = GL_INVALID_OPERATION;
		return;
	}
	if (tex->width != (GLuint)width)
	{
		tex->width = width;
		tex->dirty |= GL_TEXTURE_DIRTY_WIDTH;
	}
	if (tex->height != (GLuint)height)
	{
		tex->height = height;
		tex->dirty |= GL_TEXTURE_DIRTY_HEIGTH;
	}
	if (tex->depth != 1)
	{
		tex->depth = 1;
		tex->dirty |= GL_TEXTURE_DIRTY_DEPTH;
	}
	if (tex->format != (GLenum)internalformat)
	{
		tex->format = internalformat;
		tex->dirty |= GL_TEXTURE_DIRTY_FORMAT;
	}
	tex->data = malloc(width * height * 4);
	assert(tex->data);
	memcpy(tex->data, data, width * height * 4);
}
