#include "internal.h"

static GLboolean valid_wrap(GLenum v)
{
	switch (v)
	{
		case GL_CLAMP:
		case GL_CLAMP_TO_BORDER:
		case GL_CLAMP_TO_EDGE:
		case GL_MIRRORED_REPEAT:
		case GL_REPEAT:
			return GL_TRUE;
		default:
			g_ctx->errno = GL_INVALID_ENUM;
			return GL_FALSE;
	}
}

static GLboolean valid_min_filter(GLenum v)
{
	switch (v)
	{
		case GL_NEAREST:
		case GL_LINEAR:
		case GL_NEAREST_MIPMAP_NEAREST:
		case GL_LINEAR_MIPMAP_NEAREST:
		case GL_NEAREST_MIPMAP_LINEAR:
		case GL_LINEAR_MIPMAP_LINEAR:
		case GL_CUBIC:
			return GL_TRUE;
		default:
			g_ctx->errno = GL_INVALID_ENUM;
			return GL_FALSE;
	}
}

static GLboolean valid_mag_filter(GLenum v)
{
	switch (v)
	{
		case GL_NEAREST:
		case GL_LINEAR:
		case GL_CUBIC:
			return GL_TRUE;
		default:
			g_ctx->errno = GL_INVALID_ENUM;
			return GL_FALSE;
	}
}

void glTexParameteri(GLenum target, GLenum pname, GLint param)
{
	GLuint texid;
	switch (target)
	{
		case GL_TEXTURE_1D:
			texid = g_ctx->texture_binding_1d;
			break;
		case GL_TEXTURE_2D:
			texid = g_ctx->texture_binding_2d;
			break;
		case GL_TEXTURE_3D:
			texid = g_ctx->texture_binding_3d;
			break;
		default:
			g_ctx->errno = GL_INVALID_ENUM;
			return;
	}
	struct texture *texture;
	if (!texid || texid >= g_ctx->textures_capacity)
	{
		g_ctx->errno = GL_INVALID_VALUE;
		return;
	}
	texture = g_ctx->textures[texid];
	switch (pname)
	{
		case GL_TEXTURE_WRAP_S:
			if (!valid_wrap(param))
				return;
			if (texture->wrap_s != (GLenum)param)
			{
				texture->wrap_s = param;
				texture->dirty |= GL_TEXTURE_DIRTY_WRAP_S;
			}
			break;
		case GL_TEXTURE_WRAP_T:
			if (!valid_wrap(param))
				return;
			if (texture->wrap_t != (GLenum)param)
			{
				texture->wrap_t = param;
				texture->dirty |= GL_TEXTURE_DIRTY_WRAP_T;
			}
			break;
		case GL_TEXTURE_WRAP_R:
			if (!valid_wrap(param))
				return;
			if (texture->wrap_r != (GLenum)param)
			{
				texture->wrap_r = param;
				texture->dirty |= GL_TEXTURE_DIRTY_WRAP_R;
			}
			break;
		case GL_TEXTURE_MIN_FILTER:
			if (!valid_min_filter(param))
				return;
			if (texture->min_filter != (GLenum)param)
			{
				texture->min_filter = param;
				texture->dirty |= GL_TEXTURE_DIRTY_MIN_FILTER;
			}
			break;
		case GL_TEXTURE_MAG_FILTER:
			if (!valid_mag_filter(param))
				return;
			if (texture->mag_filter != (GLenum)param)
			{
				texture->mag_filter = param;
				texture->dirty |= GL_TEXTURE_DIRTY_MAG_FILTER;
			}
			break;
		default:
			g_ctx->errno = GL_INVALID_ENUM;
			return;
	}
}

void glTexParameterf(GLenum target, GLenum pname, GLfloat param)
{
	GLuint texid;
	switch (target)
	{
		case GL_TEXTURE_1D:
			texid = g_ctx->texture_binding_1d;
			break;
		case GL_TEXTURE_2D:
			texid = g_ctx->texture_binding_2d;
			break;
		case GL_TEXTURE_3D:
			texid = g_ctx->texture_binding_3d;
			break;
		default:
			g_ctx->errno = GL_INVALID_ENUM;
			return;
	}
	struct texture *texture;
	if (!texid || texid >= g_ctx->textures_capacity)
	{
		g_ctx->errno = GL_INVALID_VALUE;
		return;
	}
	texture = g_ctx->textures[texid];
	switch (pname)
	{
		default:
			g_ctx->errno = GL_INVALID_ENUM;
			return;
	}
}
