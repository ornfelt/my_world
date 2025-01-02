#include "internal.h"

#include <assert.h>
#include <limits.h>

void glFogf(GLenum pname, GLfloat param)
{
	if (pname == GL_FOG_COLOR)
	{
		g_ctx->errno = GL_INVALID_VALUE;
		return;
	}
	glFogfv(pname, &param);
}

void glFogi(GLenum pname, GLint param)
{
	if (pname == GL_FOG_COLOR)
	{
		g_ctx->errno = GL_INVALID_VALUE;
		return;
	}
	glFogiv(pname, &param);
}

void glFogfv(GLenum pname, GLfloat *param)
{
	switch (pname)
	{
		case GL_FOG_DENSITY:
			if (*param < 0)
			{
				g_ctx->errno = GL_INVALID_VALUE;
				return;
			}
			g_ctx->fog_density = *param;
			break;
		case GL_FOG_START:
			g_ctx->fog_start = *param;
			break;
		case GL_FOG_END:
			g_ctx->fog_end = *param;
			break;
		case GL_FOG_INDEX:
			g_ctx->fog_index = *param;
			break;
		case GL_FOG_MODE:
		{
			GLenum v = *param;
			switch (v)
			{
				case GL_EXP:
				case GL_EXP2:
				case GL_LINEAR:
					g_ctx->fog_mode = v;
					break;
				default:
					g_ctx->errno = GL_INVALID_ENUM;
					return;
			}
			break;
		}
		case GL_FOG_COLOR:
			g_ctx->fog_color[0] = clampf(param[0], 0, 1);
			g_ctx->fog_color[1] = clampf(param[1], 0, 1);
			g_ctx->fog_color[2] = clampf(param[2], 0, 1);
			g_ctx->fog_color[3] = clampf(param[3], 0, 1);
			break;
		case GL_FOG_COORD_SRC:
		{
			GLenum v = *param;
			switch (v)
			{
				case GL_FOG_COORD:
				case GL_FRAGMENT_DEPTH:
					g_ctx->fog_coord_src = v;
					break;
				default:
					g_ctx->errno = GL_INVALID_ENUM;
					return;
			}
			break;
		}
		default:
			g_ctx->errno = GL_INVALID_ENUM;
			return;
	}
}

void glFogiv(GLenum pname, GLint *param)
{
	switch (pname)
	{
		case GL_FOG_DENSITY:
			if (*param < 0)
			{
				g_ctx->errno = GL_INVALID_VALUE;
				return;
			}
			g_ctx->fog_density = *param;
			break;
		case GL_FOG_START:
			g_ctx->fog_start = *param;
			break;
		case GL_FOG_END:
			g_ctx->fog_end = *param;
			break;
		case GL_FOG_INDEX:
			g_ctx->fog_index = *param;
			break;
		case GL_FOG_MODE:
			switch (*param)
			{
				case GL_EXP:
				case GL_EXP2:
				case GL_LINEAR:
					g_ctx->fog_mode = *param;
					break;
				default:
					g_ctx->errno = GL_INVALID_ENUM;
					return;
			}
			break;
		case GL_FOG_COLOR:
			g_ctx->fog_color[0] = clampf(param[0] / (GLfloat)INT_MAX, 0, 1);
			g_ctx->fog_color[1] = clampf(param[1] / (GLfloat)INT_MAX, 0, 1);
			g_ctx->fog_color[2] = clampf(param[2] / (GLfloat)INT_MAX, 0, 1);
			g_ctx->fog_color[3] = clampf(param[3] / (GLfloat)INT_MAX, 0, 1);
			break;
		case GL_FOG_COORD_SRC:
			switch (*param)
			{
				case GL_FOG_COORD:
				case GL_FRAGMENT_DEPTH:
					g_ctx->fog_coord_src = *param;
					break;
				default:
					g_ctx->errno = GL_INVALID_ENUM;
					return;
			}
			break;
		default:
			g_ctx->errno = GL_INVALID_ENUM;
			return;
	}
}
