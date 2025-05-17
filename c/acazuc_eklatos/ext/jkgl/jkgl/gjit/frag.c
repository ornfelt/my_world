#include "rast/rast.h"

#include "utils.h"
#include "gjit.h"

#include <assert.h>
#include <stdlib.h>

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
          float * restrict color)
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
          float * restrict color)
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

static uint32_t
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
            uint32_t stencil)
{
	jkg_set_stencil(surface_ptr(surface, x, y),
	                stencil,
	                surface->image->format);
}

void
gjit_fragment(struct rast_ctx *rast_ctx,
              struct rast_vert *vert)
{
	struct gjit_draw_ctx *draw_ctx = rast_ctx->userdata;
	struct jkg_ctx *ctx = draw_ctx->ctx;
	float color_in[4];
	float color_out[4];
	float color[4];
	float cur_depth;
	int32_t x;
	int32_t y;
	float z;

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
	cur_depth = depth_get(ctx->depth_surface, x, y);
	if (ctx->depth_stencil_state->depth_test_enable)
	{
		if (!depth_test_fn(z, cur_depth, ctx->depth_stencil_state->depth_compare))
			return;
	}
	if (ctx->shader_state->fs->fs_fn(vert, ctx->uniform_data, color))
		return;
	color[0] = clampf(color[0], 0, 1);
	color[1] = clampf(color[1], 0, 1);
	color[2] = clampf(color[2], 0, 1);
	color[3] = clampf(color[3], 0, 1);
	color_get(ctx->color_surfaces[0], x, y, color_in);
	ctx->blend_state->fn(color_out, color, color_in, ctx->blend_color);
	color_set(ctx->color_surfaces[0],
	          &ctx->blend_state->attachments[0],
	          x,
	          y,
	          color_out);
	if (ctx->depth_stencil_state->depth_write_enable)
		depth_set(ctx->depth_surface, x, y, z);
}
