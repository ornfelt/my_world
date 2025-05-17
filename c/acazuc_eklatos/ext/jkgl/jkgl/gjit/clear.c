#include "utils.h"
#include "gjit.h"

static int
get_rect(struct jkg_ctx *ctx,
         const struct jkg_surface *surface,
         int32_t *rect)
{
	rect[0] = maxf(ctx->viewport[0], 0);
	rect[1] = minf(ctx->viewport[3], surface->image->size.x);
	rect[2] = maxf(ctx->viewport[1], 0);
	rect[3] = minf(ctx->viewport[4], surface->image->size.y);
	if (ctx->rasterizer_state->scissor_enable)
	{
		rect[0] = maxf(rect[0], ctx->viewport[0]);
		rect[1] = maxf(rect[1], ctx->viewport[1]);
		rect[2] = maxf(rect[2], ctx->viewport[2]);
		rect[3] = maxf(rect[3], ctx->viewport[3]);
	}
	if (rect[0] > rect[1]
	 || rect[2] > rect[3])
		return 1;
	return 0;
}

static void
clear_color(struct jkg_ctx *ctx, const float * restrict value)
{
	enum jkg_color_mask color_mask;
	int32_t rect[4];

	/* XXX */
	if (!ctx->blend_state || !ctx->blend_state->nattachments)
		return;
	color_mask = ctx->blend_state->attachments[0].color_mask;
	if (!(color_mask & (JKG_COLOR_MASK_R | JKG_COLOR_MASK_G | JKG_COLOR_MASK_B | JKG_COLOR_MASK_A)))
		return;
	if (!ctx->color_surfaces[0])
		return;
	if (get_rect(ctx, ctx->color_surfaces[0], rect))
		return;
	jkg_clear_vec4f(ctx->color_surfaces[0]->image->data,
	                ctx->color_surfaces[0]->image->size.x,
	                ctx->color_surfaces[0]->image->size.y,
	                ctx->color_surfaces[0]->image->format,
	                color_mask,
	                rect,
	                value);
}

static void
clear_depth(struct jkg_ctx *ctx, float value)
{
	int32_t rect[4];

	if (!ctx->depth_stencil_state->depth_write_enable)
		return;
	if (!ctx->depth_surface)
		return;
	if (get_rect(ctx, ctx->depth_surface, rect))
		return;
	jkg_clear_depth(ctx->depth_surface->image->data,
	                ctx->depth_surface->image->size.x,
	                ctx->depth_surface->image->size.y,
	                ctx->depth_surface->image->format,
	                rect,
	                value);
}

static void
clear_stencil(struct jkg_ctx *ctx, uint32_t value)
{
	int32_t rect[4];

	if (!ctx->depth_stencil_state->stencil_enable)
		return;
	if (!ctx->stencil_surface)
		return;
	if (get_rect(ctx, ctx->stencil_surface, rect))
		return;
	jkg_clear_stencil(ctx->stencil_surface->image->data,
	                  ctx->stencil_surface->image->size.x,
	                  ctx->stencil_surface->image->size.y,
	                  ctx->stencil_surface->image->format,
	                  rect,
	                  value);
}

static void
clear_depth_stencil(struct jkg_ctx *ctx, float depth, uint32_t stencil)
{
	int32_t rect[4];

	if (ctx->depth_surface != ctx->stencil_surface
	 || !ctx->depth_stencil_state->stencil_enable
	 || !ctx->depth_stencil_state->depth_write_enable)
	{
		clear_depth(ctx, depth);
		clear_stencil(ctx, stencil);
		return;
	}
	if (!ctx->depth_surface)
		return;
	if (get_rect(ctx, ctx->depth_surface, rect))
		return;
	jkg_clear_depth_stencil(ctx->depth_surface->image->data,
	                        ctx->depth_surface->image->size.x,
	                        ctx->depth_surface->image->size.y,
	                        ctx->depth_surface->image->format,
	                        rect,
	                        depth,
	                        stencil);
}

int
gjit_clear(struct jkg_ctx *ctx, const struct jkg_clear_cmd *cmd)
{
	if (cmd->mask & JKG_CLEAR_COLOR)
		clear_color(ctx, cmd->color);
	if ((cmd->mask & (JKG_CLEAR_DEPTH | JKG_CLEAR_STENCIL)) == (JKG_CLEAR_DEPTH | JKG_CLEAR_STENCIL))
	{
		clear_depth_stencil(ctx, cmd->depth, cmd->stencil);
	}
	else
	{
		if (cmd->mask & JKG_CLEAR_DEPTH)
			clear_depth(ctx, cmd->depth);
		if (cmd->mask & JKG_CLEAR_STENCIL)
			clear_stencil(ctx, cmd->stencil);
	}
	return 0;
}
