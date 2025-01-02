#include "internal.h"

void glBlendColor(GLclampf red, GLclampf green, GLclampf blue,
                  GLclampf alpha)
{
	g_ctx->blend_color[0] = clampf(red, 0, 1);
	g_ctx->blend_color[1] = clampf(green, 0, 1);
	g_ctx->blend_color[2] = clampf(blue, 0, 1);
	g_ctx->blend_color[3] = clampf(alpha, 0, 1);
}

void glBlendEquation(GLenum mode)
{
	glBlendEquationSeparate(mode, mode);
}

static GLboolean equation_is_valid(GLenum mode)
{
	switch (mode)
	{
		case GL_FUNC_ADD:
		case GL_FUNC_SUBSTRACT:
		case GL_FUNC_REVERSE_SUBSTRACT:
		case GL_MIN:
		case GL_MAX:
			return GL_TRUE;
		default:
			return GL_FALSE;
	}
}

void glBlendEquationSeparate(GLenum mode_rgb, GLenum mode_alpha)
{
	if (!equation_is_valid(mode_rgb)
	 || !equation_is_valid(mode_alpha))
	{
		g_ctx->errno = GL_INVALID_ENUM;
		return;
	}
	if (g_ctx->blend_equation_rgb != mode_rgb)
	{
		g_ctx->blend_equation_rgb = mode_rgb;
		g_ctx->dirty |= GL_CTX_DIRTY_BLEND_EQUATION_RGB;
	}
	if (g_ctx->blend_equation_alpha != mode_alpha)
	{
		g_ctx->blend_equation_alpha = mode_alpha;
		g_ctx->dirty |= GL_CTX_DIRTY_BLEND_EQUATION_ALPHA;
	}
}

void glBlendFunc(GLenum s_factor, GLenum d_factor)
{
	glBlendFuncSeparate(s_factor, d_factor, s_factor, d_factor);
}

static GLboolean factor_is_valid(GLenum s_factor, GLenum d_factor)
{
	switch (s_factor)
	{
		case GL_ZERO:
		case GL_ONE:
		case GL_SRC_COLOR:
		case GL_ONE_MINUS_SRC_COLOR:
		case GL_DST_COLOR:
		case GL_ONE_MINUS_DST_COLOR:
		case GL_SRC_ALPHA:
		case GL_ONE_MINUS_SRC_ALPHA:
		case GL_DST_ALPHA:
		case GL_ONE_MINUS_DST_ALPHA:
		case GL_CONSTANT_COLOR:
		case GL_ONE_MINUS_CONSTANT_COLOR:
		case GL_CONSTANT_ALPHA:
		case GL_ONE_MINUS_CONSTANT_ALPHA:
		case GL_SRC_ALPHA_SATURATE:
			return GL_TRUE;
		default:
			return GL_FALSE;
	}
	switch (d_factor)
	{
		case GL_ZERO:
		case GL_ONE:
		case GL_SRC_COLOR:
		case GL_ONE_MINUS_SRC_COLOR:
		case GL_DST_COLOR:
		case GL_ONE_MINUS_DST_COLOR:
		case GL_SRC_ALPHA:
		case GL_ONE_MINUS_SRC_ALPHA:
		case GL_DST_ALPHA:
		case GL_ONE_MINUS_DST_ALPHA:
		case GL_CONSTANT_COLOR:
		case GL_ONE_MINUS_CONSTANT_COLOR:
		case GL_CONSTANT_ALPHA:
		case GL_ONE_MINUS_CONSTANT_ALPHA:
			return GL_TRUE;
		default:
			return GL_FALSE;
	}
}

void glBlendFuncSeparate(GLenum src_rgb, GLenum dst_rgb,
                         GLenum src_alpha, GLenum dst_alpha)
{
	if (!factor_is_valid(src_rgb, dst_rgb)
	 || !factor_is_valid(src_alpha, dst_alpha))
	{
		g_ctx->errno = GL_INVALID_ENUM;
		return;
	}
	if (g_ctx->blend_src_rgb != src_rgb)
	{
		g_ctx->blend_src_rgb = src_rgb;
		g_ctx->dirty |= GL_CTX_DIRTY_BLEND_SRC_RGB;
	}
	if (g_ctx->blend_src_alpha != src_alpha)
	{
		g_ctx->blend_src_alpha = src_alpha;
		g_ctx->dirty |= GL_CTX_DIRTY_BLEND_SRC_ALPHA;
	}
	if (g_ctx->blend_dst_rgb != dst_rgb)
	{
		g_ctx->blend_dst_rgb = dst_rgb;
		g_ctx->dirty |= GL_CTX_DIRTY_BLEND_DST_RGB;
	}
	if (g_ctx->blend_dst_alpha != dst_alpha)
	{
		g_ctx->blend_dst_alpha = dst_alpha;
		g_ctx->dirty |= GL_CTX_DIRTY_BLEND_DST_ALPHA;
	}
}
