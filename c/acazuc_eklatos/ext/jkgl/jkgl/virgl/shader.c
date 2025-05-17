#include "virgl.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

static enum pipe_shader_type
get_shader_type(enum jkg_shader_type type)
{
	switch (type)
	{
		default:
			assert(!"unknown shader type");
			/* FALLTHROUGH */
		case JKG_SHADER_VERTEX:
			return PIPE_SHADER_VERTEX;
		case JKG_SHADER_FRAGMENT:
			return PIPE_SHADER_FRAGMENT;
	}
}

int
virgl_shader_alloc(struct jkg_ctx *ctx,
                   const struct jkg_shader_create_info *create_info,
                   struct jkg_shader **shaderp)
{
	struct jkg_shader *shader = NULL;
	uint32_t hdr_size = VIRGL_OBJ_SHADER_HDR_SIZE(0);
	uint32_t *request;
	size_t pad;
	int ret;

	shader = calloc(1, sizeof(*shader));
	if (!shader)
		return -ENOMEM;
	shader->type = create_info->type;
	ret = virgl_alloc_id(ctx, &shader->id);
	if (ret)
		goto err;
	if (create_info->size % 4)
		pad = 4 - (create_info->size % 4);
	else
		pad = 4;
	ret = virgl_req_alloc(ctx,
	                      VIRGL_CCMD_CREATE_OBJECT,
	                      VIRGL_OBJECT_SHADER,
	                      hdr_size + (create_info->size + pad) / 4,
	                      &request);
	if (ret)
		return ret;
	request[VIRGL_OBJ_SHADER_HANDLE] = shader->id;
	request[VIRGL_OBJ_SHADER_TYPE] = get_shader_type(create_info->type);
	request[VIRGL_OBJ_SHADER_OFFSET] = create_info->size + pad;
	request[VIRGL_OBJ_SHADER_NUM_TOKENS] = 200; /* XXX less arbitrary */
	request[VIRGL_OBJ_SHADER_SO_NUM_OUTPUTS] = 0;
	memcpy(&request[hdr_size + 1], create_info->data, create_info->size);
	memset(&((uint8_t*)&request[hdr_size + 1])[create_info->size], 0, pad);
	*shaderp = shader;
	return 0;

err:
	if (shader->id)
		virgl_free_id(ctx, shader->id);
	free(shader);
	return ret;
}

void
virgl_shader_free(struct jkg_ctx *ctx,
                  struct jkg_shader *shader)
{
	if (virgl_destroy_object(ctx, VIRGL_OBJECT_SHADER, shader->id))
		assert(!"failed to destroy shader");
	virgl_free_id(ctx, shader->id);
	free(shader);
}

int
virgl_shader_state_alloc(struct jkg_ctx *ctx,
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

static int
bind_shader(struct jkg_ctx *ctx,
            struct jkg_shader *shader)
{
	uint32_t *request;
	int ret;

	ret = virgl_req_alloc(ctx,
	                      VIRGL_CCMD_BIND_SHADER,
	                      VIRGL_OBJECT_NULL,
	                      VIRGL_BIND_SHADER_SIZE,
	                      &request);
	if (ret)
		return ret;
	request[VIRGL_BIND_SHADER_HANDLE] = shader->id;
	request[VIRGL_BIND_SHADER_TYPE] = get_shader_type(shader->type);
	return 0;
}

int
virgl_shader_state_bind(struct jkg_ctx *ctx,
                        struct jkg_shader_state *state)
{
	uint32_t *request;
	int ret;

	ret = bind_shader(ctx, state->vs);
	if (ret)
		return ret;
	ret = bind_shader(ctx, state->fs);
	if (ret)
		return ret;
	ret = virgl_req_alloc(ctx,
	                      VIRGL_CCMD_LINK_SHADER,
	                      VIRGL_OBJECT_NULL,
	                      VIRGL_LINK_SHADER_SIZE,
	                      &request);
	if (ret)
		return ret;
	request[VIRGL_LINK_SHADER_VERTEX_HANDLE] = state->vs->id;
	request[VIRGL_LINK_SHADER_FRAGMENT_HANDLE] = state->fs->id;
	request[VIRGL_LINK_SHADER_GEOMETRY_HANDLE] = 0;
	request[VIRGL_LINK_SHADER_TESS_CTRL_HANDLE] = 0;
	request[VIRGL_LINK_SHADER_TESS_EVAL_HANDLE] = 0;
	request[VIRGL_LINK_SHADER_COMPUTE_HANDLE] = 0;
	return 0;
}

void
virgl_shader_state_free(struct jkg_ctx *ctx,
                        struct jkg_shader_state *state)
{
	(void)ctx;
	free(state);
}
