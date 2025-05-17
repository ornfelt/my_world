#include "internal.h"

#include <assert.h>
#include <limits.h>

static void
fogfv(struct gl_ctx *ctx,
      GLenum pname,
      GLfloat *param)
{
	switch (pname)
	{
		case GL_FOG_DENSITY:
			if (*param < 0)
			{
				GL_SET_ERR(ctx, GL_INVALID_VALUE);
				return;
			}
			ctx->fixed.block.fog_density = *param;
			break;
		case GL_FOG_START:
			ctx->fixed.block.fog_start = *param;
			break;
		case GL_FOG_END:
			ctx->fixed.block.fog_end = *param;
			break;
		case GL_FOG_INDEX:
			ctx->fixed.block.fog_index = *param;
			break;
		case GL_FOG_MODE:
		{
			GLenum v = *param;
			switch (v)
			{
				case GL_EXP:
				case GL_EXP2:
				case GL_LINEAR:
					ctx->fixed.block.fog_mode = v;
					break;
				default:
					GL_SET_ERR(ctx, GL_INVALID_ENUM);
					return;
			}
			break;
		}
		case GL_FOG_COLOR:
			ctx->fixed.block.fog_color[0] = clampf(param[0], 0, 1);
			ctx->fixed.block.fog_color[1] = clampf(param[1], 0, 1);
			ctx->fixed.block.fog_color[2] = clampf(param[2], 0, 1);
			ctx->fixed.block.fog_color[3] = clampf(param[3], 0, 1);
			break;
		case GL_FOG_COORD_SRC:
		{
			GLenum v = *param;
			switch (v)
			{
				case GL_FOG_COORD:
				case GL_FRAGMENT_DEPTH:
					ctx->fixed.block.fog_coord_src = v;
					break;
				default:
					GL_SET_ERR(ctx, GL_INVALID_ENUM);
					return;
			}
			break;
		}
		default:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return;
	}
}

static void
fogiv(struct gl_ctx *ctx,
      GLenum pname,
      GLint *param)
{
	switch (pname)
	{
		case GL_FOG_DENSITY:
			if (*param < 0)
			{
				GL_SET_ERR(ctx, GL_INVALID_VALUE);
				return;
			}
			ctx->fixed.block.fog_density = *param;
			break;
		case GL_FOG_START:
			ctx->fixed.block.fog_start = *param;
			break;
		case GL_FOG_END:
			ctx->fixed.block.fog_end = *param;
			break;
		case GL_FOG_INDEX:
			ctx->fixed.block.fog_index = *param;
			break;
		case GL_FOG_MODE:
			switch (*param)
			{
				case GL_EXP:
				case GL_EXP2:
				case GL_LINEAR:
					ctx->fixed.block.fog_mode = *param;
					break;
				default:
					GL_SET_ERR(ctx, GL_INVALID_ENUM);
					return;
			}
			break;
		case GL_FOG_COLOR:
			ctx->fixed.block.fog_color[0] = clampf(param[0] * (1.0 / INT32_MAX), 0, 1);
			ctx->fixed.block.fog_color[1] = clampf(param[1] * (1.0 / INT32_MAX), 0, 1);
			ctx->fixed.block.fog_color[2] = clampf(param[2] * (1.0 / INT32_MAX), 0, 1);
			ctx->fixed.block.fog_color[3] = clampf(param[3] * (1.0 / INT32_MAX), 0, 1);
			break;
		case GL_FOG_COORD_SRC:
			switch (*param)
			{
				case GL_FOG_COORD:
				case GL_FRAGMENT_DEPTH:
					ctx->fixed.block.fog_coord_src = *param;
					break;
				default:
					GL_SET_ERR(ctx, GL_INVALID_ENUM);
					return;
			}
			break;
		default:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return;
	}
}

void
glFogf(GLenum pname, GLfloat param)
{
	struct gl_ctx *ctx = g_ctx;

	if (pname == GL_FOG_COLOR)
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	fogfv(ctx, pname, &param);
}

void
glFogi(GLenum pname, GLint param)
{
	struct gl_ctx *ctx = g_ctx;

	if (pname == GL_FOG_COLOR)
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	fogiv(ctx, pname, &param);
}

void
glFogfv(GLenum pname, GLfloat *param)
{
	struct gl_ctx *ctx = g_ctx;

	fogfv(ctx, pname, param);
}

void
glFogiv(GLenum pname, GLint *param)
{
	struct gl_ctx *ctx = g_ctx;

	fogiv(ctx, pname, param);
}
