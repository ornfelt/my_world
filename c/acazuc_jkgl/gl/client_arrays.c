#include "internal.h"

void glColorPointer(GLint size, GLenum type, GLsizei stride,
                    const GLvoid *pointer)
{
	switch (size)
	{
		case 3:
		case 4:
			break;
		default:
			g_ctx->errno = GL_INVALID_VALUE;
			return;
	}
	switch (type)
	{
		case GL_BYTE:
		case GL_UNSIGNED_BYTE:
		case GL_SHORT:
		case GL_UNSIGNED_SHORT:
		case GL_INT:
		case GL_UNSIGNED_INT:
		case GL_FLOAT:
		case GL_DOUBLE:
			break;
		default:
			g_ctx->errno = GL_INVALID_ENUM;
			return;
	}
	if (stride < 0)
	{
		g_ctx->errno = GL_INVALID_VALUE;
		return;
	}
	if (stride == 0)
		stride = size * glSizeof(type);
	g_ctx->color_array.size = size;
	g_ctx->color_array.type = type;
	g_ctx->color_array.stride = stride;
	g_ctx->color_array.pointer = pointer;
}

void glVertexPointer(GLint size, GLenum type, GLsizei stride,
                     const GLvoid *pointer)
{
	switch (size)
	{
		case 2:
		case 3:
		case 4:
			break;
		default:
			g_ctx->errno = GL_INVALID_VALUE;
			return;
	}
	switch (type)
	{
		case GL_SHORT:
		case GL_INT:
		case GL_FLOAT:
		case GL_DOUBLE:
			break;
		default:
			g_ctx->errno = GL_INVALID_ENUM;
			return;
	}
	if (stride < 0)
	{
		g_ctx->errno = GL_INVALID_VALUE;
		return;
	}
	if (stride == 0)
		stride = size * glSizeof(type);
	g_ctx->vertex_array.size = size;
	g_ctx->vertex_array.type = type;
	g_ctx->vertex_array.stride = stride;
	g_ctx->vertex_array.pointer = pointer;
}

void glTexCoordPointer(GLint size, GLenum type, GLsizei stride,
                       const GLvoid *pointer)
{
	switch (size)
	{
		case 1:
		case 2:
		case 3:
		case 4:
			break;
		default:
			g_ctx->errno = GL_INVALID_VALUE;
			return;
	}
	switch (type)
	{
		case GL_SHORT:
		case GL_INT:
		case GL_FLOAT:
		case GL_DOUBLE:
			break;
		default:
			g_ctx->errno = GL_INVALID_ENUM;
			return;
	}
	if (stride < 0)
	{
		g_ctx->errno = GL_INVALID_VALUE;
		return;
	}
	if (stride == 0)
		stride = size * glSizeof(type);
	g_ctx->texture_coord_array.size = size;
	g_ctx->texture_coord_array.type = type;
	g_ctx->texture_coord_array.stride = stride;
	g_ctx->texture_coord_array.pointer = pointer;
}

void glNormalPointer(GLenum type, GLsizei stride, const GLvoid *pointer)
{
	switch (type)
	{
		case GL_BYTE:
		case GL_SHORT:
		case GL_INT:
		case GL_FLOAT:
		case GL_DOUBLE:
			break;
		default:
			g_ctx->errno = GL_INVALID_ENUM;
			return;
	}
	if (stride < 0)
	{
		g_ctx->errno = GL_INVALID_VALUE;
		return;
	}
	if (stride == 0)
		stride = glSizeof(type);
	g_ctx->normal_array.type = type;
	g_ctx->normal_array.stride = stride;
	g_ctx->normal_array.pointer = pointer;
}

void glEnableClientState(GLenum cap)
{
	switch (cap)
	{
		case GL_COLOR_ARRAY:
			g_ctx->color_array.enabled = GL_TRUE;
			break;
		case GL_VERTEX_ARRAY:
			g_ctx->vertex_array.enabled = GL_TRUE;
			break;
		case GL_TEXTURE_COORD_ARRAY:
			g_ctx->texture_coord_array.enabled = GL_TRUE;
			break;
		case GL_NORMAL_ARRAY:
			g_ctx->normal_array.enabled = GL_TRUE;
			break;
		default:
			g_ctx->errno = GL_INVALID_ENUM;
			break;
	}
}

void glDisableClientState(GLenum cap)
{
	switch (cap)
	{
		case GL_COLOR_ARRAY:
			g_ctx->color_array.enabled = GL_FALSE;
			break;
		case GL_VERTEX_ARRAY:
			g_ctx->vertex_array.enabled = GL_FALSE;
			break;
		case GL_TEXTURE_COORD_ARRAY:
			g_ctx->texture_coord_array.enabled = GL_FALSE;
			break;
		case GL_NORMAL_ARRAY:
			g_ctx->normal_array.enabled = GL_FALSE;
			break;
		default:
			g_ctx->errno = GL_INVALID_ENUM;
			break;
	}
}
