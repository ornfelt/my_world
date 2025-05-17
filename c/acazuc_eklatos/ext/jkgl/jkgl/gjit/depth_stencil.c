#include "gjit.h"

#include <stdlib.h>
#include <errno.h>

int
gjit_depth_stencil_state_alloc(struct jkg_ctx *ctx,
                               const struct jkg_depth_stencil_state_create_info *create_info,
                               struct jkg_depth_stencil_state **statep)
{
	struct jkg_depth_stencil_state *state;

	(void)ctx;
	state = malloc(sizeof(*state));
	if (!state)
		return -ENOMEM;
	state->depth_test_enable = create_info->depth_test_enable;
	state->depth_write_enable = create_info->depth_write_enable;
	state->depth_compare = create_info->depth_compare;
	state->stencil_enable = create_info->stencil_enable;
	state->front = create_info->front;
	state->back = create_info->back;
	*statep = state;
	return 0;
}

int
gjit_depth_stencil_state_bind(struct jkg_ctx *ctx,
                              struct jkg_depth_stencil_state *state)
{
	ctx->depth_stencil_state = state;
	return 0;
}

void
gjit_depth_stencil_state_free(struct jkg_ctx *ctx,
                              struct jkg_depth_stencil_state *state)
{
	(void)ctx;
	free(state);
}

int
gjit_set_stencil_ref(struct jkg_ctx *ctx, uint8_t back, uint8_t front)
{
	ctx->stencil_ref[0] = back;
	ctx->stencil_ref[1] = front;
	return 0;
}
