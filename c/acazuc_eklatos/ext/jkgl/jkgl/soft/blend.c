#include "soft.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>

int
soft_blend_state_alloc(struct jkg_ctx *ctx,
                       const struct jkg_blend_state_create_info *create_info,
                       struct jkg_blend_state **statep)
{
	struct jkg_blend_state *state = NULL;

	(void)ctx;
	state = malloc(sizeof(*state));
	if (!state)
		return -ENOMEM;
	state->logic_enable = create_info->logic_enable;
	state->logic_op = create_info->logic_op;
	state->nattachments = create_info->nattachments;
	memcpy(state->attachments,
	       create_info->attachments,
	       sizeof(*state->attachments) * state->nattachments);
	*statep = state;
	return 0;
}

int
soft_blend_state_bind(struct jkg_ctx *ctx,
                      struct jkg_blend_state *state)
{
	ctx->blend_state = state;
	return 0;
}

void
soft_blend_state_free(struct jkg_ctx *ctx,
                      struct jkg_blend_state *state)
{
	(void)ctx;
	free(state);
}

int
soft_set_blend_color(struct jkg_ctx *ctx,
                     const float *color)
{
	for (size_t i = 0; i < 4; ++i)
		ctx->blend_color[i] = color[i];
	return 0;
}
