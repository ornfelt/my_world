#include "soft.h"

#include <stdlib.h>
#include <errno.h>

int
soft_shader_alloc(struct jkg_ctx *ctx,
                  const struct jkg_shader_create_info *create_info,
                  struct jkg_shader **shaderp)
{
	struct jkg_shader *shader;

	(void)ctx;
	shader = malloc(sizeof(*shader));
	if (!shader)
		return -ENOMEM;
	shader->vs_fn = create_info->data;
	shader->in = 4; /* XXX */
	shader->out = 4; /* XXX */
	*shaderp = shader;
	return 0;
}

void
soft_shader_free(struct jkg_ctx *ctx,
                 struct jkg_shader *shader)
{
	(void)ctx;
	free(shader);
}

int
soft_shader_state_alloc(struct jkg_ctx *ctx,
                        const struct jkg_shader_state_create_info *create_info,
                        struct jkg_shader_state **statep)
{
	struct jkg_shader_state *state;

	(void)ctx;
	state = malloc(sizeof(*state));
	if (!state)
		return -ENOMEM;
	state->vs = create_info->vs;
	state->fs = create_info->fs;
	*statep = state;
	return 0;
}

int
soft_shader_state_bind(struct jkg_ctx *ctx,
                       struct jkg_shader_state *state)
{
	ctx->shader_state = state;
	return 0;
}

void
soft_shader_state_free(struct jkg_ctx *ctx,
                       struct jkg_shader_state *state)
{
	(void)ctx;
	free(state);
}
