#include "internal.h"

void glBindTexture(GLenum target, GLuint texture)
{
	struct texture *tex;

	if (texture >= g_ctx->textures_capacity)
	{
		g_ctx->errno = GL_INVALID_VALUE;
		return;
	}
	if (!texture)
	{
		switch (target)
		{
			case GL_TEXTURE_1D:
				g_ctx->texture_binding_1d = texture;
				break;
			case GL_TEXTURE_2D:
				g_ctx->texture_binding_2d = texture;
				break;
			case GL_TEXTURE_3D:
				g_ctx->texture_binding_3d = texture;
				break;
			default:
				g_ctx->errno = GL_INVALID_ENUM;
				return;
		}
		return;
	}
	if (!g_ctx->textures[texture])
	{
		g_ctx->errno = GL_INVALID_VALUE;
		return;
	}
	tex = g_ctx->textures[texture];
	switch (target)
	{
		case GL_TEXTURE_1D:
			if (tex->initialized)
			{
				if (tex->target != GL_TEXTURE_1D)
				{
					g_ctx->errno = GL_INVALID_OPERATION;
					return;
				}
			}
			else
			{
				tex->initialized = GL_TRUE;
				tex->target = GL_TEXTURE_1D;
			}
			g_ctx->texture_binding_1d = texture;
			break;
		case GL_TEXTURE_2D:
			if (tex->initialized)
			{
				if (tex->target != GL_TEXTURE_2D)
				{
					g_ctx->errno = GL_INVALID_OPERATION;
					return;
				}
			}
			else
			{
				tex->initialized = GL_TRUE;
				tex->target = GL_TEXTURE_2D;
			}
			g_ctx->texture_binding_2d = texture;
			break;
		case GL_TEXTURE_3D:
			if (tex->initialized)
			{
				if (tex->target != GL_TEXTURE_3D)
				{
					g_ctx->errno = GL_INVALID_OPERATION;
					return;
				}
			}
			else
			{
				tex->initialized = GL_TRUE;
				tex->target = GL_TEXTURE_3D;
			}
			g_ctx->texture_binding_3d = texture;
			break;
		default:
			g_ctx->errno = GL_INVALID_ENUM;
			return;
	}
}
