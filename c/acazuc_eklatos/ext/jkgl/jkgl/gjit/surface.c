#include "gjit.h"

#include <stdlib.h>
#include <errno.h>

int
gjit_surface_alloc(struct jkg_ctx *ctx,
                   const struct jkg_surface_create_info *create_info,
                   struct jkg_surface **surfacep)
{
	struct jkg_surface *surface = NULL;

	(void)ctx;
	surface = malloc(sizeof(*surface));
	if (!surface)
		return -ENOMEM;
	surface->image = create_info->image;
	surface->level = create_info->level;
	surface->min_layer = create_info->min_layer;
	surface->num_layers = create_info->num_layers;
	*surfacep = surface;
	return 0;
}

int
gjit_surface_bind(struct jkg_ctx *ctx,
                  struct jkg_surface **colors,
                  uint32_t count,
                  struct jkg_surface *depth,
                  struct jkg_surface *stencil)
{
	for (uint32_t i = 0; i < count; ++i)
		ctx->color_surfaces[i] = colors[i];
	for (uint32_t i = count; i < GJIT_MAX_ATTACHMENTS; ++i)
		ctx->color_surfaces[i] = NULL;
	ctx->depth_surface = depth;
	ctx->stencil_surface = stencil;
	return 0;
}

int
gjit_surface_clear(struct jkg_ctx *ctx,
                   struct jkg_surface *surface,
                   const struct jkg_clear_cmd *cmd)
{
	int32_t rect[4];

	(void)ctx;
	rect[0] = 0;
	rect[1] = 0;
	rect[2] = surface->image->size.x;
	rect[3] = surface->image->size.y;
	switch (jkg_get_format_type(surface->image->format))
	{
		case JKG_FORMAT_TYPE_UNKNOWN:
			break;
		case JKG_FORMAT_TYPE_COLOR_UNORM:
		case JKG_FORMAT_TYPE_COLOR_SNORM:
		case JKG_FORMAT_TYPE_COLOR_USCALED:
		case JKG_FORMAT_TYPE_COLOR_SSCALED:
		case JKG_FORMAT_TYPE_COLOR_SFLOAT:
			if (cmd->mask & JKG_CLEAR_COLOR)
				jkg_clear_vec4f(surface->image->data,
				                surface->image->size.x,
				                surface->image->size.y,
				                surface->image->format,
				                JKG_COLOR_MASK_R | JKG_COLOR_MASK_G | JKG_COLOR_MASK_B | JKG_COLOR_MASK_A,
				                rect,
				                cmd->color);
			break;
		case JKG_FORMAT_TYPE_DEPTH_SFLOAT:
			if (cmd->mask & JKG_CLEAR_DEPTH)
				jkg_clear_depth(surface->image->data,
				                surface->image->size.x,
				                surface->image->size.y,
				                surface->image->format,
				                rect,
				                cmd->depth);
			break;
		case JKG_FORMAT_TYPE_STENCIL_UINT:
			if (cmd->mask & JKG_CLEAR_STENCIL)
				jkg_clear_stencil(surface->image->data,
				                  surface->image->size.x,
				                  surface->image->size.y,
				                  surface->image->format,
				                  rect,
				                  cmd->stencil);
			break;
		case JKG_FORMAT_TYPE_DEPTH_UNORM_STENCIL_UINT:
			if ((cmd->mask & (JKG_CLEAR_DEPTH | JKG_CLEAR_STENCIL)) == (JKG_CLEAR_DEPTH | JKG_CLEAR_STENCIL))
			{
				jkg_clear_depth_stencil(surface->image->data,
				                        surface->image->size.x,
				                        surface->image->size.y,
				                        surface->image->format,
				                        rect,
				                        cmd->depth,
				                        cmd->stencil);
			}
			else
			{
				if (cmd->mask & JKG_CLEAR_DEPTH)
					jkg_clear_depth(surface->image->data,
					                surface->image->size.x,
					                surface->image->size.y,
					                surface->image->format,
					                rect,
					                cmd->depth);
				if (cmd->mask & JKG_CLEAR_STENCIL)
					jkg_clear_stencil(surface->image->data,
					                  surface->image->size.x,
					                  surface->image->size.y,
					                  surface->image->format,
					                  rect,
					                  cmd->stencil);
			}
			break;
	}
	return 0;
}

void
gjit_surface_free(struct jkg_ctx *ctx,
                  struct jkg_surface *surface)
{
	(void)ctx;
	free(surface);
}
