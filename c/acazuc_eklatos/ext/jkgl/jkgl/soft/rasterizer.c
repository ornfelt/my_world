#include "soft.h"

#include <stdlib.h>
#include <errno.h>

int
soft_rasterizer_state_alloc(struct jkg_ctx *ctx,
                            const struct jkg_rasterizer_state_create_info *create_info,
                            struct jkg_rasterizer_state **statep)
{
	struct jkg_rasterizer_state *state;

	(void)ctx;
	state = malloc(sizeof(*state));
	if (!state)
		return -ENOMEM;
	state->point_size = create_info->point_size;
	state->line_width = create_info->line_width;
	state->point_smooth_enable = create_info->point_smooth_enable;
	state->line_smooth_enable = create_info->line_smooth_enable;
	state->scissor_enable = create_info->scissor_enable;
	state->depth_clamp_enable = create_info->depth_clamp_enable;
	state->rasterizer_discard_enable = create_info->rasterizer_discard_enable;
	state->multisample_enable = create_info->multisample_enable;
	state->front_ccw = create_info->front_ccw;
	state->cull_face = create_info->cull_face;
	state->fill_front = create_info->fill_front;
	state->fill_back = create_info->fill_back;
	*statep = state;
	return 0;
}

int
soft_rasterizer_state_bind(struct jkg_ctx *ctx,
                           struct jkg_rasterizer_state *state)
{
	ctx->rasterizer_state = state;
	return 0;
}

void
soft_rasterizer_state_free(struct jkg_ctx *ctx,
                           struct jkg_rasterizer_state *state)
{
	(void)ctx;
	free(state);
}
