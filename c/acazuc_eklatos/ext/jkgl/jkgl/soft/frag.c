#include "utils.h"
#include "soft.h"

#include <assert.h>
#include <stdlib.h>

static void
set_blend_func_rgb(float * restrict color,
                   const float * restrict src,
                   const float * restrict dst,
                   const float * restrict constant_color,
                   enum jkg_blend_fn fn)
{
	switch (fn)
	{
		case JKG_BLEND_ZERO:
			color[0] = 0;
			color[1] = 0;
			color[2] = 0;
			break;
		case JKG_BLEND_ONE:
			break;
		case JKG_BLEND_SRC_COLOR:
			color[0] *= src[0];
			color[1] *= src[1];
			color[2] *= src[2];
			break;
		case JKG_BLEND_ONE_MINUS_SRC_COLOR:
			color[0] *= 1 - src[0];
			color[1] *= 1 - src[1];
			color[2] *= 1 - src[2];
			break;
		case JKG_BLEND_DST_COLOR:
			color[0] *= dst[0];
			color[1] *= dst[1];
			color[2] *= dst[2];
			break;
		case JKG_BLEND_ONE_MINUS_DST_COLOR:
			color[0] *= 1 - dst[0];
			color[1] *= 1 - dst[1];
			color[2] *= 1 - dst[2];
			break;
		case JKG_BLEND_SRC_ALPHA:
			color[0] *= src[3];
			color[1] *= src[3];
			color[2] *= src[3];
			break;
		case JKG_BLEND_ONE_MINUS_SRC_ALPHA:
			color[0] *= 1 - src[3];
			color[1] *= 1 - src[3];
			color[2] *= 1 - src[3];
			break;
		case JKG_BLEND_DST_ALPHA:
			color[0] *= dst[3];
			color[1] *= dst[3];
			color[2] *= dst[3];
			break;
		case JKG_BLEND_ONE_MINUS_DST_ALPHA:
			color[0] *= 1 - dst[3];
			color[1] *= 1 - dst[3];
			color[2] *= 1 - dst[3];
			break;
		case JKG_BLEND_CONSTANT_COLOR:
			color[0] *= constant_color[0];
			color[1] *= constant_color[1];
			color[2] *= constant_color[2];
			break;
		case JKG_BLEND_ONE_MINUS_CONSTANT_COLOR:
			color[0] *= 1 - constant_color[0];
			color[1] *= 1 - constant_color[1];
			color[2] *= 1 - constant_color[2];
			break;
		case JKG_BLEND_CONSTANT_ALPHA:
			color[0] *= constant_color[3];
			color[1] *= constant_color[3];
			color[2] *= constant_color[3];
			break;
		case JKG_BLEND_ONE_MINUS_CONSTANT_ALPHA:
			color[0] *= 1 - constant_color[3];
			color[1] *= 1 - constant_color[3];
			color[2] *= 1 - constant_color[3];
			break;
		case JKG_BLEND_SRC_ALPHA_SATURATE:
			color[0] *= minf(src[3], 1 - dst[3]);
			color[1] *= minf(src[3], 1 - dst[3]);
			color[2] *= minf(src[3], 1 - dst[3]);
			break;
		default:
			assert(!"unknown blend func rgb");
			break;
	}
}

static void
set_blend_func_alpha(float * restrict color,
                     const float * restrict src,
                     const float * restrict dst,
                     const float * restrict constant_color,
                     enum jkg_blend_fn fn)
{
	switch (fn)
	{
		case JKG_BLEND_ZERO:
			color[3] = 0;
			break;
		case JKG_BLEND_ONE:
			break;
		case JKG_BLEND_SRC_COLOR:
			color[3] *= src[3];
			break;
		case JKG_BLEND_ONE_MINUS_SRC_COLOR:
			color[3] *= 1 - src[3];
			break;
		case JKG_BLEND_DST_COLOR:
			color[3] *= dst[3];
			break;
		case JKG_BLEND_ONE_MINUS_DST_COLOR:
			color[3] *= 1 - dst[3];
			break;
		case JKG_BLEND_SRC_ALPHA:
			color[3] *= src[3];
			break;
		case JKG_BLEND_ONE_MINUS_SRC_ALPHA:
			color[3] *= 1 - src[3];
			break;
		case JKG_BLEND_DST_ALPHA:
			color[3] *= dst[3];
			break;
		case JKG_BLEND_ONE_MINUS_DST_ALPHA:
			color[3] *= 1 - dst[3];
			break;
		case JKG_BLEND_CONSTANT_COLOR:
			color[3] *= constant_color[3];
			break;
		case JKG_BLEND_ONE_MINUS_CONSTANT_COLOR:
			color[3] *= 1 - constant_color[3];
			break;
		case JKG_BLEND_CONSTANT_ALPHA:
			color[3] *= constant_color[3];
			break;
		case JKG_BLEND_ONE_MINUS_CONSTANT_ALPHA:
			color[3] *= 1 - constant_color[3];
			break;
		case JKG_BLEND_SRC_ALPHA_SATURATE:
			color[3] *= minf(src[3], 1 - dst[3]);
			break;
		default:
			assert(!"unknown blend func alpha");
			break;
	}
}

static void
set_blend_equation_rgb(float * restrict tmp,
                       const float * restrict src,
                       const float * restrict dst,
                       enum jkg_blend_eq eq)
{
	switch (eq)
	{
		case JKG_EQUATION_ADD:
			tmp[0] = src[0] + dst[0];
			tmp[1] = src[1] + dst[1];
			tmp[2] = src[2] + dst[2];
			break;
		case JKG_EQUATION_SUBTRACT:
			tmp[0] = src[0] - dst[0];
			tmp[1] = src[1] - dst[1];
			tmp[2] = src[2] - dst[2];
			break;
		case JKG_EQUATION_REV_SUBTRACT:
			tmp[0] = dst[0] - src[0];
			tmp[1] = dst[1] - src[1];
			tmp[2] = dst[2] - src[2];
			break;
		case JKG_EQUATION_MIN:
			tmp[0] = minf(src[0], dst[0]);
			tmp[1] = minf(src[1], dst[1]);
			tmp[2] = minf(src[2], dst[2]);
			break;
		case JKG_EQUATION_MAX:
			tmp[0] = maxf(src[0], dst[0]);
			tmp[1] = maxf(src[1], dst[1]);
			tmp[2] = maxf(src[2], dst[2]);
			break;
		default:
			assert(!"unknown blend equation rgb");
			break;
	}
}

static void
set_blend_equation_alpha(float * restrict tmp,
                         const float * restrict src,
                         const float * restrict dst,
                         enum jkg_blend_eq eq)
{
	switch (eq)
	{
		case JKG_EQUATION_ADD:
			tmp[3] = src[3] + dst[3];
			break;
		case JKG_EQUATION_SUBTRACT:
			tmp[3] = src[3] - dst[3];
			break;
		case JKG_EQUATION_REV_SUBTRACT:
			tmp[3] = dst[3] - src[3];
			break;
		case JKG_EQUATION_MIN:
			tmp[3] = minf(src[3], dst[3]);
			break;
		case JKG_EQUATION_MAX:
			tmp[3] = maxf(src[3], dst[3]);
			break;
		default:
			assert(!"unknown blend equation alpha");
			break;
	}
}

static bool
depth_test_fn(float z,
              float cur,
              enum jkg_compare_op cmp)
{
	switch (cmp)
	{
		case JKG_COMPARE_ALWAYS:
			return true;
		case JKG_COMPARE_LOWER:
			return z < cur;
		case JKG_COMPARE_LEQUAL:
			return z <= cur;
		case JKG_COMPARE_EQUAL:
			return z == cur;
		case JKG_COMPARE_GEQUAL:
			return z >= cur;
		case JKG_COMPARE_GREATER:
			return z > cur;
		case JKG_COMPARE_NEVER:
			return false;
		case JKG_COMPARE_NOTEQUAL:
			return z != cur;
		default:
			assert(!"unknown depth test");
			return false;
	}
}

void
fragment_set(struct jkg_ctx *ctx,
             const struct jkg_blend_attachment *attachment,
             const float * restrict color_dst,
             const float * restrict color_src,
             float * restrict color_buf)
{
	float src[4];
	float dst[4];

	if (!attachment->enable)
	{
		color_buf[0] = color_src[0];
		color_buf[1] = color_src[1];
		color_buf[2] = color_src[2];
		color_buf[3] = color_src[3];
		return;
	}
	dst[0] = color_dst[0];
	dst[1] = color_dst[1];
	dst[2] = color_dst[2];
	dst[3] = color_dst[3];
	src[0] = color_src[0];
	src[1] = color_src[1];
	src[2] = color_src[2];
	src[3] = color_src[3];
	set_blend_func_rgb(src, color_src, color_dst, ctx->blend_color, attachment->rgb_src);
	set_blend_func_rgb(dst, color_src, color_dst, ctx->blend_color, attachment->rgb_dst);
	set_blend_func_alpha(src, color_src, color_dst, ctx->blend_color, attachment->alpha_src);
	set_blend_func_alpha(dst, color_src, color_dst, ctx->blend_color, attachment->alpha_dst);
	set_blend_equation_rgb(color_buf, src, dst, attachment->rgb_eq);
	set_blend_equation_alpha(color_buf, src, dst, attachment->alpha_eq);
}

static void *
surface_ptr(const struct jkg_surface *surface,
            int32_t x,
            int32_t y)
{
	uint8_t *data;
	size_t index;

	data = surface->image->data;
	index = x + (surface->image->size.y - 1 - y) * surface->image->size.x;
	return &data[index * jkg_get_stride(surface->image->format)];
}

static void
color_get(const struct jkg_surface *surface,
          int32_t x,
          int32_t y,
          float *color)
{
	jkg_get_vec4f(color,
	              surface_ptr(surface, x, y),
	              surface->image->format);
}

static void
color_set(const struct jkg_surface *surface,
          const struct jkg_blend_attachment *attachment,
          int32_t x,
          int32_t y,
          const float *color)
{
	jkg_set_vec4f(surface_ptr(surface, x, y),
	              color,
	              surface->image->format,
	              attachment->color_mask);
}

static float
depth_get(const struct jkg_surface *surface,
          int32_t x,
          int32_t y)
{
	return jkg_get_depth(surface_ptr(surface, x, y),
	                     surface->image->format);
}

static void
depth_set(const struct jkg_surface *surface,
          int32_t x,
          int32_t y,
          float z)
{
	jkg_set_depth(surface_ptr(surface, x, y),
	              z,
	              surface->image->format);
}

static uint8_t
stencil_get(const struct jkg_surface *surface,
            int32_t x,
            int32_t y)
{
	return jkg_get_stencil(surface_ptr(surface, x, y),
	                       surface->image->format);
}

static void
stencil_set(const struct jkg_surface *surface,
            int32_t x,
            int32_t y,
            uint8_t stencil)
{
	jkg_set_stencil(surface_ptr(surface, x, y),
	                stencil,
	                surface->image->format);
}

static void
stencil_op(struct jkg_ctx *ctx,
           enum jkg_stencil_op op,
           uint8_t mask,
           uint8_t value,
           int32_t x,
           int32_t y,
           bool front_face)
{
	switch (op)
	{
		default:
			assert(!"unknown stencil op");
			/* FALLTHROUGH */
		case JKG_STENCIL_KEEP:
			break;
		case JKG_STENCIL_ZERO:
			stencil_set(ctx->stencil_surface,
			            x,
			            y,
			            value & ~mask);
			break;
		case JKG_STENCIL_REPLACE:
			stencil_set(ctx->stencil_surface,
			            x,
			            y,
			            (value & ~mask) | (ctx->stencil_ref[!!front_face] & mask));
			break;
		case JKG_STENCIL_INC:
			if (value == UINT8_MAX)
				break;
			stencil_set(ctx->stencil_surface,
			            x,
			            y,
			            (value & ~mask) | ((value + 1) & mask));
			break;
		case JKG_STENCIL_INC_WRAP:
			stencil_set(ctx->stencil_surface,
			            x,
			            y,
			            (value & ~mask) | ((value + 1) & mask));
			break;
		case JKG_STENCIL_DEC:
			if (!value)
				break;
			stencil_set(ctx->stencil_surface,
			            x,
			            y,
			            (value & ~mask) | ((value - 1) & mask));
			break;
		case JKG_STENCIL_DEC_WRAP:
			stencil_set(ctx->stencil_surface,
			            x,
			            y,
			            (value & ~mask) | ((value - 1) & mask));
			break;
		case JKG_STENCIL_INV:
			stencil_set(ctx->stencil_surface,
			            x,
			            y,
			            (value & mask) | ((~value) & mask));
			break;
	}
}

static bool
stencil_test_fn(enum jkg_compare_op op,
                uint8_t ref,
                uint8_t value,
                uint8_t mask)
{
	switch (op)
	{
		default:
			assert(!"unknown compare op");
			/* XXX FALLTHROUGH */
		case JKG_COMPARE_NEVER:
			return false;
		case JKG_COMPARE_LOWER:
			return (ref & mask) < (value & mask);
		case JKG_COMPARE_LEQUAL:
			return (ref & mask) <= (value & mask);
		case JKG_COMPARE_EQUAL:
			return (ref & mask) == (value & mask);
		case JKG_COMPARE_GEQUAL:
			return (ref & mask) >= (value & mask);
		case JKG_COMPARE_GREATER:
			return (ref & mask) > (value & mask);
		case JKG_COMPARE_NOTEQUAL:
			return (ref & mask) != (value & mask);
		case JKG_COMPARE_ALWAYS:
			return true;
	}
}

void
soft_fragment(struct rast_ctx *rast_ctx,
              struct rast_vert *vert)
{
	struct soft_draw_ctx *draw_ctx = rast_ctx->userdata;
	struct jkg_ctx *ctx = draw_ctx->ctx;
	const struct jkg_stencil_op_state *stencil_state;
	float color_in[4];
	float color_out[4];
	float color[4];
	float depth_value;
	uint8_t stencil_value;
	int32_t x;
	int32_t y;
	float z;
	bool stencil_enable;

	if (ctx->rasterizer_state->rasterizer_discard_enable)
		return;
	x = vert->x;
	y = vert->y;
	z = vert->z;
	if (z < ctx->viewport[2])
	{
		if (ctx->rasterizer_state->depth_clamp_enable)
			z = ctx->viewport[2];
		else
			return;
	}
	if (z > ctx->viewport[5])
	{
		if (ctx->rasterizer_state->depth_clamp_enable)
			z = ctx->viewport[5];
		else
			return;
	}
	stencil_state = vert->front_face ? &ctx->depth_stencil_state->front : &ctx->depth_stencil_state->back;
	stencil_enable = ctx->depth_stencil_state->stencil_enable;
	stencil_value = stencil_get(ctx->stencil_surface, x, y);
	if (stencil_enable)
	{
		if (!stencil_test_fn(stencil_state->compare_op,
		                     ctx->stencil_ref[!!vert->front_face],
		                     stencil_value,
		                     stencil_state->compare_mask))
		{
			stencil_op(ctx,
			           stencil_state->fail_op,
			           stencil_state->write_mask,
			           stencil_value,
			           x,
			           y,
			           vert->front_face);
			return;
		}
	}
	if (ctx->depth_stencil_state->depth_test_enable)
	{
		depth_value = depth_get(ctx->depth_surface, x, y);
		if (!depth_test_fn(z,
		                   depth_value,
		                   ctx->depth_stencil_state->depth_compare))
		{
			if (stencil_enable)
				stencil_op(ctx,
				           stencil_state->zfail_op,
				           stencil_state->write_mask,
				           stencil_value,
				           x,
				           y,
				           vert->front_face);
			return;
		}
	}
	if (ctx->shader_state->fs->fs_fn(vert, ctx->uniform_data, color))
		return;
	color[0] = clampf(color[0], 0, 1);
	color[1] = clampf(color[1], 0, 1);
	color[2] = clampf(color[2], 0, 1);
	color[3] = clampf(color[3], 0, 1);
	color_get(ctx->color_surfaces[0], x, y, color_in);
	fragment_set(ctx,
	             &ctx->blend_state->attachments[0],
	             color_in,
	             color,
	             color_out);
	color_set(ctx->color_surfaces[0],
	          &ctx->blend_state->attachments[0],
	          x,
	          y,
	          color_out);
	if (stencil_enable)
		stencil_op(ctx,
		           stencil_state->pass_op,
		           stencil_state->write_mask,
		           stencil_value,
		           x,
		           y,
		           vert->front_face);
	if (ctx->depth_stencil_state->depth_write_enable)
		depth_set(ctx->depth_surface, x, y, z);
}
