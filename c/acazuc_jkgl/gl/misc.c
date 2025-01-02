#include "internal.h"

#include <stdlib.h>
#include <assert.h>

void glLineWidth(GLfloat width)
{
	if (width <= 0)
	{
		g_ctx->errno = GL_INVALID_VALUE;
		return;
	}
	g_ctx->line_width = width;
}

void glPointSize(GLfloat size)
{
	if (size <= 0)
	{
		g_ctx->errno = GL_INVALID_VALUE;
		return;
	}
	g_ctx->point_size = size;
}

void glColorMask(GLboolean red, GLboolean green, GLboolean blue,
                 GLboolean alpha)
{
	if (g_ctx->color_mask[0] != red)
	{
		g_ctx->color_mask[0] = red;
		g_ctx->dirty |= GL_CTX_DIRTY_COLOR_MASK_R;
	}
	if (g_ctx->color_mask[1] != green)
	{
		g_ctx->color_mask[1] = green;
		g_ctx->dirty |= GL_CTX_DIRTY_COLOR_MASK_G;
	}
	if (g_ctx->color_mask[2] != blue)
	{
		g_ctx->color_mask[2] = blue;
		g_ctx->dirty |= GL_CTX_DIRTY_COLOR_MASK_B;
	}
	if (g_ctx->color_mask[3] != alpha)
	{
		g_ctx->color_mask[3] = alpha;
		g_ctx->dirty |= GL_CTX_DIRTY_COLOR_MASK_A;
	}
}

void glViewport(GLsizei width, GLsizei height)
{
	if (width < 0 || height < 0)
	{
		g_ctx->errno = GL_INVALID_VALUE;
		return;
	}
	if (width == g_ctx->width
	 && height == g_ctx->height)
		return;
	free(g_ctx->color_buffer);
	free(g_ctx->depth_buffer);
	free(g_ctx->stencil_buffer);
	g_ctx->width = width;
	g_ctx->height = height;
	if (width == 0 && height == 0)
	{
		g_ctx->color_buffer = malloc(1);
		assert(g_ctx->color_buffer);
		g_ctx->depth_buffer = malloc(1);
		assert(g_ctx->depth_buffer);
		g_ctx->stencil_buffer = malloc(1);
		assert(g_ctx->stencil_buffer);
	}
	else
	{
		g_ctx->color_buffer = malloc(sizeof(*g_ctx->color_buffer) * g_ctx->height * g_ctx->width * 4);
		assert(g_ctx->color_buffer);
		g_ctx->depth_buffer = malloc(sizeof(*g_ctx->depth_buffer) * g_ctx->height * g_ctx->width);
		assert(g_ctx->depth_buffer);
		g_ctx->stencil_buffer = malloc(sizeof(*g_ctx->stencil_buffer) * g_ctx->height * g_ctx->width);
		assert(g_ctx->stencil_buffer);
	}
}

GLsizei glSizeof(GLenum type)
{
	switch (type)
	{
		case GL_BYTE:
			return sizeof(GLbyte);
		case GL_UNSIGNED_BYTE:
			return sizeof(GLubyte);
		case GL_SHORT:
			return sizeof(GLshort);
		case GL_UNSIGNED_SHORT:
			return sizeof(GLushort);
		case GL_INT:
			return sizeof(GLint);
		case GL_UNSIGNED_INT:
			return sizeof(GLuint);
		case GL_FLOAT:
			return sizeof(GLfloat);
		case GL_DOUBLE:
			return sizeof(GLdouble);
		default:
			return 0;
	}
}

void glShadeModel(GLenum mode)
{
	switch (mode)
	{
		case GL_SMOOTH:
		case GL_FLAT:
			g_ctx->shade_model = mode;
			break;
		default:
			g_ctx->errno = GL_INVALID_VALUE;
			return;
	}
}

GLenum glGetError(void)
{
	GLenum error;

	error = g_ctx->errno;
	g_ctx->errno = GL_NO_ERROR;
	return error;
}

void glFrontFace(GLenum mode)
{
	switch (mode)
	{
		case GL_CW:
		case GL_CCW:
			g_ctx->front_face = mode;
			break;
		default:
			g_ctx->errno = GL_INVALID_VALUE;
			return;
	}
}

void glCullFace(GLenum mode)
{
	switch (mode)
	{
		case GL_FRONT:
		case GL_BACK:
		case GL_FRONT_AND_BACK:
			g_ctx->cull_face = mode;
			break;
		default:
			g_ctx->errno = GL_INVALID_VALUE;
			return;
	}
}

void glScissor(GLint x, GLint y, GLsizei width, GLsizei height)
{
	if (width < 0 || height < 0)
	{
		g_ctx->errno = GL_INVALID_VALUE;
		return;
	}
	g_ctx->scissor_x = x;
	g_ctx->scissor_y = y;
	g_ctx->scissor_width = width;
	g_ctx->scissor_height = height;
}
