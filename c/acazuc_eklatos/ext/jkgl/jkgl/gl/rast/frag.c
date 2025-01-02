#include "internal.h"
#include "fnv.h"

#include <assert.h>
#include <stdlib.h>

#ifndef ENABLE_GCCJIT

static void set_blend_func_rgb(GLfloat *color, const GLfloat *src,
                               const GLfloat *dst, GLenum mode)
{
	switch (mode)
	{
		case GL_ZERO:
			color[0] = 0;
			color[1] = 0;
			color[2] = 0;
			break;
		case GL_ONE:
			break;
		case GL_SRC_COLOR:
			color[0] *= src[0];
			color[1] *= src[1];
			color[2] *= src[2];
			break;
		case GL_ONE_MINUS_SRC_COLOR:
			color[0] *= 1 - src[0];
			color[1] *= 1 - src[1];
			color[2] *= 1 - src[2];
			break;
		case GL_DST_COLOR:
			color[0] *= dst[0];
			color[1] *= dst[1];
			color[2] *= dst[2];
			break;
		case GL_ONE_MINUS_DST_COLOR:
			color[0] *= 1 - dst[0];
			color[1] *= 1 - dst[1];
			color[2] *= 1 - dst[2];
			break;
		case GL_SRC_ALPHA:
			color[0] *= src[3];
			color[1] *= src[3];
			color[2] *= src[3];
			break;
		case GL_ONE_MINUS_SRC_ALPHA:
			color[0] *= 1 - src[3];
			color[1] *= 1 - src[3];
			color[2] *= 1 - src[3];
			break;
		case GL_DST_ALPHA:
			color[0] *= dst[3];
			color[1] *= dst[3];
			color[2] *= dst[3];
			break;
		case GL_ONE_MINUS_DST_ALPHA:
			color[0] *= 1 - dst[3];
			color[1] *= 1 - dst[3];
			color[2] *= 1 - dst[3];
			break;
		case GL_CONSTANT_COLOR:
			color[0] *= g_ctx->blend_color[0];
			color[1] *= g_ctx->blend_color[1];
			color[2] *= g_ctx->blend_color[2];
			break;
		case GL_ONE_MINUS_CONSTANT_COLOR:
			color[0] *= 1 - g_ctx->blend_color[0];
			color[1] *= 1 - g_ctx->blend_color[1];
			color[2] *= 1 - g_ctx->blend_color[2];
			break;
		case GL_CONSTANT_ALPHA:
			color[0] *= g_ctx->blend_color[3];
			color[1] *= g_ctx->blend_color[3];
			color[2] *= g_ctx->blend_color[3];
			break;
		case GL_ONE_MINUS_CONSTANT_ALPHA:
			color[0] *= 1 - g_ctx->blend_color[3];
			color[1] *= 1 - g_ctx->blend_color[3];
			color[2] *= 1 - g_ctx->blend_color[3];
			break;
		case GL_SRC_ALPHA_SATURATE:
			color[0] *= minf(src[3], 1 - dst[3]);
			color[1] *= minf(src[3], 1 - dst[3]);
			color[2] *= minf(src[3], 1 - dst[3]);
			break;
		default:
			assert(!"unknown blend func rgb");
			break;
	}
}

static void set_blend_func_alpha(GLfloat *color, const GLfloat *src,
                                 const GLfloat *dst, GLenum mode)
{
	switch (mode)
	{
		case GL_ZERO:
			color[3] = 0;
			break;
		case GL_ONE:
			break;
		case GL_SRC_COLOR:
			color[3] *= src[3];
			break;
		case GL_ONE_MINUS_SRC_COLOR:
			color[3] *= 1 - src[3];
			break;
		case GL_DST_COLOR:
			color[3] *= dst[3];
			break;
		case GL_ONE_MINUS_DST_COLOR:
			color[3] *= 1 - dst[3];
			break;
		case GL_SRC_ALPHA:
			color[3] *= src[3];
			break;
		case GL_ONE_MINUS_SRC_ALPHA:
			color[3] *= 1 - src[3];
			break;
		case GL_DST_ALPHA:
			color[3] *= dst[3];
			break;
		case GL_ONE_MINUS_DST_ALPHA:
			color[3] *= 1 - dst[3];
			break;
		case GL_CONSTANT_COLOR:
			color[3] *= g_ctx->blend_color[3];
			break;
		case GL_ONE_MINUS_CONSTANT_COLOR:
			color[3] *= 1 - g_ctx->blend_color[3];
			break;
		case GL_CONSTANT_ALPHA:
			color[3] *= g_ctx->blend_color[3];
			break;
		case GL_ONE_MINUS_CONSTANT_ALPHA:
			color[3] *= 1 - g_ctx->blend_color[3];
			break;
		case GL_SRC_ALPHA_SATURATE:
			color[3] *= minf(src[3], 1 - dst[3]);
			break;
		default:
			assert(!"unknown blend func alpha");
			break;
	}
}

static void set_blend_equation_rgb(GLfloat *tmp, const GLfloat *src,
                                   const GLfloat *dst)
{
	switch (g_ctx->blend_equation_rgb)
	{
		case GL_FUNC_ADD:
			tmp[0] = src[0] + dst[0];
			tmp[1] = src[1] + dst[1];
			tmp[2] = src[2] + dst[2];
			break;
		case GL_FUNC_SUBSTRACT:
			tmp[0] = src[0] - dst[0];
			tmp[1] = src[1] - dst[1];
			tmp[2] = src[2] - dst[2];
			break;
		case GL_FUNC_REVERSE_SUBSTRACT:
			tmp[0] = dst[0] - src[0];
			tmp[1] = dst[1] - src[1];
			tmp[2] = dst[2] - src[2];
			break;
		case GL_MIN:
			tmp[0] = minf(src[0], dst[0]);
			tmp[1] = minf(src[1], dst[1]);
			tmp[2] = minf(src[2], dst[2]);
			break;
		case GL_MAX:
			tmp[0] = maxf(src[0], dst[0]);
			tmp[1] = maxf(src[1], dst[1]);
			tmp[2] = maxf(src[2], dst[2]);
			break;
		default:
			assert(!"unknown blend equation rgb");
			break;
	}
}

static void set_blend_equation_alpha(GLfloat *tmp, const GLfloat *src,
                                     const GLfloat *dst)
{
	switch (g_ctx->blend_equation_alpha)
	{
		case GL_FUNC_ADD:
			tmp[3] = src[3] + dst[3];
			break;
		case GL_FUNC_SUBSTRACT:
			tmp[3] = src[3] - dst[3];
			break;
		case GL_FUNC_REVERSE_SUBSTRACT:
			tmp[3] = dst[3] - src[3];
			break;
		case GL_MIN:
			tmp[3] = minf(src[3], dst[3]);
			break;
		case GL_MAX:
			tmp[3] = maxf(src[3], dst[3]);
			break;
		default:
			assert(!"unknown blend equation alpha");
			break;
	}
}

static GLboolean depth_test_fn(GLfloat z, GLfloat cur)
{
	switch (g_ctx->depth_func)
	{
		case GL_ALWAYS:
			return GL_TRUE;
		case GL_LESS:
			return z < cur;
		case GL_LEQUAL:
			return z <= cur;
		case GL_EQUAL:
			return z == cur;
		case GL_GEQUAL:
			return z >= cur;
		case GL_GREATER:
			return z > cur;
		case GL_NEVER:
			return GL_FALSE;
		case GL_NOTEQUAL:
			return z != cur;
		default:
			assert(!"unknown depth test");
			return GL_FALSE;
	}
}

#endif

static void fragment_set(GLfloat *color_buf, GLfloat *depth_buf, GLfloat *color, GLfloat depth)
{
#ifdef ENABLE_GCCJIT
	g_ctx->jit.fragment_set_state->fn(color_buf, depth_buf, color, depth, g_ctx->blend_color);
#else
	if (!g_ctx->blend)
	{
		if (g_ctx->color_mask[0])
			color_buf[0] = color[0];
		if (g_ctx->color_mask[1])
			color_buf[1] = color[1];
		if (g_ctx->color_mask[2])
			color_buf[2] = color[2];
		if (g_ctx->color_mask[3])
			color_buf[3] = color[3];
		return;
	}
	GLfloat src[4];
	GLfloat dst[4];
	GLfloat tmp[4];
	dst[0] = color_buf[0];
	dst[1] = color_buf[1];
	dst[2] = color_buf[2];
	dst[3] = color_buf[3];
	src[0] = color[0];
	src[1] = color[1];
	src[2] = color[2];
	src[3] = color[3];
	set_blend_func_rgb(src, color, dst, g_ctx->blend_src_rgb);
	set_blend_func_rgb(dst, color, dst, g_ctx->blend_dst_rgb);
	set_blend_func_alpha(src, color, dst, g_ctx->blend_src_alpha);
	set_blend_func_alpha(dst, color, dst, g_ctx->blend_dst_alpha);
	set_blend_equation_rgb(tmp, src, dst);
	set_blend_equation_alpha(tmp, src, dst);
	if (g_ctx->color_mask[0])
		color_buf[0] = tmp[0];
	if (g_ctx->color_mask[1])
		color_buf[1] = tmp[1];
	if (g_ctx->color_mask[2])
		color_buf[2] = tmp[2];
	if (g_ctx->color_mask[3])
		color_buf[3] = tmp[3];
	if (g_ctx->depth_write)
		*depth_buf = depth;
#endif
}

GLboolean rast_depth_test(GLint x, GLint y, GLfloat z)
{
#ifdef ENABLE_GCCJIT
	return g_ctx->jit.depth_test_state->fn(z, &g_ctx->depth_range[0], g_ctx->depth_buffer[y * g_ctx->width + x]);
#else
	if (z < g_ctx->depth_range[0]
	 || z > g_ctx->depth_range[1])
		return GL_FALSE;
	if (!g_ctx->depth_test)
		return GL_TRUE;
	return depth_test_fn(z, g_ctx->depth_buffer[y * g_ctx->width + x]);
#endif
}

void rast_fragment(struct vert *vert)
{
	GLfloat color[4];
	GLint x;
	GLint y;

	x = vert->x;
	y = vert->y;
	if (g_ctx->fs.fn(vert, color))
		return;
	fragment_set(&g_ctx->color_buffer[(y * g_ctx->width + x) * 4], &g_ctx->depth_buffer[y * g_ctx->width + x], color, vert->z);
}
