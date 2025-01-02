#include "internal.h"

static GLboolean is_valid_depth_func(GLenum func)
{
	switch (func)
	{
		case GL_NEVER:
		case GL_LESS:
		case GL_EQUAL:
		case GL_LEQUAL:
		case GL_GREATER:
		case GL_NOTEQUAL:
		case GL_GEQUAL:
		case GL_ALWAYS:
			return GL_TRUE;
		default:
			g_ctx->errno = GL_INVALID_ENUM;
			return GL_FALSE;
	}
}

void glDepthFunc(GLenum func)
{
	if (!is_valid_depth_func(func))
		return;
	if (g_ctx->depth_func != func)
	{
		g_ctx->depth_func = func;
		g_ctx->dirty |= GL_CTX_DIRTY_DEPTH_FUNC;
	}
}

void glDepthMask(GLboolean flag)
{
	flag = !!flag;
	if (g_ctx->depth_write != flag)
	{
		g_ctx->depth_write = flag;
		g_ctx->dirty |= GL_CTX_DIRTY_DEPTH_WRITE;
	}
}

void glDepthRange(GLclampd near_val, GLclampd far_val)
{
	near_val = clampd(near_val, 0, 1);
	far_val = clampd(far_val, 0, 1);
	if (g_ctx->depth_range[0] != near_val)
	{
		g_ctx->depth_range[0] = near_val;
		g_ctx->dirty |= GL_CTX_DIRTY_DEPTH_RANGE_MIN;
	}
	if (g_ctx->depth_range[1] != far_val)
	{
		g_ctx->depth_range[1] = far_val;
		g_ctx->dirty |= GL_CTX_DIRTY_DEPTH_RANGE_MAX;
	}
}

void glDepthRangef(GLclampf near_val, GLclampf far_val)
{
	near_val = clampf(near_val, 0, 1);
	far_val = clampf(far_val, 0, 1);
	if (g_ctx->depth_range[0] != near_val)
	{
		g_ctx->depth_range[0] = near_val;
		g_ctx->dirty |= GL_CTX_DIRTY_DEPTH_RANGE_MIN;
	}
	if (g_ctx->depth_range[1] != far_val)
	{
		g_ctx->depth_range[1] = far_val;
		g_ctx->dirty |= GL_CTX_DIRTY_DEPTH_RANGE_MAX;
	}
}
