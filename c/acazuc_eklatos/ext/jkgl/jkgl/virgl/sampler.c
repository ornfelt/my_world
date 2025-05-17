#include "virgl.h"

#include <stdlib.h>
#include <assert.h>
#include <errno.h>

static enum pipe_tex_wrap
get_wrap_mode(enum jkg_wrap_mode mode)
{
	switch (mode)
	{
		default:
			assert(!"unknown wrap mode");
			/* FALLTHROUGH */
		case JKG_WRAP_REPEAT:
			return PIPE_TEX_WRAP_REPEAT;
		case JKG_WRAP_MIRRORED_REPEAT:
			return PIPE_TEX_WRAP_MIRROR_REPEAT;
		case JKG_WRAP_CLAMP_TO_EDGE:
			return PIPE_TEX_WRAP_CLAMP_TO_EDGE;
		case JKG_WRAP_CLAMP_TO_BORDER:
			return PIPE_TEX_WRAP_CLAMP_TO_BORDER;
		case JKG_WRAP_MIRROR_CLAMP_TO_EDGE:
			return PIPE_TEX_WRAP_MIRROR_CLAMP_TO_EDGE;
	}
}

static enum pipe_tex_filter
get_min_filter(enum jkg_filter_mode mode)
{
	switch (mode)
	{
		default:
			assert(!"unknown min filter");
			/* FALLTHROUGH */
		case JKG_FILTER_NEAREST:
			return PIPE_TEX_FILTER_NEAREST;
		case JKG_FILTER_LINEAR:
		case JKG_FILTER_CUBIC:
			return PIPE_TEX_FILTER_LINEAR;
	}
}

static enum pipe_tex_mipfilter
get_mip_filter(enum jkg_filter_mode mode)
{
	switch (mode)
	{
		default:
			assert(!"unknown mip filter");
			/* FALLTHROUGH */
		case JKG_FILTER_NONE:
			return PIPE_TEX_MIPFILTER_NONE;
		case JKG_FILTER_NEAREST:
			return PIPE_TEX_MIPFILTER_NEAREST;
		case JKG_FILTER_LINEAR:
			return PIPE_TEX_MIPFILTER_LINEAR;
	}
}

static enum pipe_tex_filter
get_mag_filter(enum jkg_filter_mode mode)
{
	switch (mode)
	{
		default:
			assert(!"unknown mag filter");
			/* FALLTHROUGH */
		case JKG_FILTER_NEAREST:
			return PIPE_TEX_FILTER_NEAREST;
		case JKG_FILTER_LINEAR:
		case JKG_FILTER_CUBIC:
			return PIPE_TEX_FILTER_LINEAR;
	}
}

static enum pipe_compare_func
get_compare_func(enum jkg_compare_op op)
{
	switch (op)
	{
		default:
			assert(!"unknown compare op");
			/* FALLTHROUGH */
		case JKG_COMPARE_NEVER:
			return PIPE_FUNC_NEVER;
		case JKG_COMPARE_LOWER:
			return PIPE_FUNC_LESS;
		case JKG_COMPARE_LEQUAL:
			return PIPE_FUNC_LEQUAL;
		case JKG_COMPARE_EQUAL:
			return PIPE_FUNC_EQUAL;
		case JKG_COMPARE_GEQUAL:
			return PIPE_FUNC_GEQUAL;
		case JKG_COMPARE_GREATER:
			return PIPE_FUNC_GREATER;
		case JKG_COMPARE_NOTEQUAL:
			return PIPE_FUNC_NOTEQUAL;
		case JKG_COMPARE_ALWAYS:
			return PIPE_FUNC_ALWAYS;
	}
}

int
virgl_sampler_alloc(struct jkg_ctx *ctx,
                    const struct jkg_sampler_create_info *create_info,
                    struct jkg_sampler **samplerp)
{
	struct jkg_sampler *sampler = NULL;
	uint32_t *request;
	int ret;

	(void)ctx;
	sampler = calloc(1, sizeof(*sampler));
	if (!sampler)
		return -ENOMEM;
	ret = virgl_alloc_id(ctx, &sampler->id);
	if (ret)
		goto err;
	ret = virgl_req_alloc(ctx,
	                      VIRGL_CCMD_CREATE_OBJECT,
	                      VIRGL_OBJECT_SAMPLER_STATE,
	                      VIRGL_OBJ_SAMPLER_STATE_SIZE,
	                      &request);
	if (ret)
		goto err;
	request[VIRGL_OBJ_SAMPLER_STATE_HANDLE] = sampler->id;
	request[VIRGL_OBJ_SAMPLER_STATE_S0] = VIRGL_OBJ_SAMPLE_STATE_S0_WRAP_S(get_wrap_mode(create_info->wrap[0]))
	                                    | VIRGL_OBJ_SAMPLE_STATE_S0_WRAP_T(get_wrap_mode(create_info->wrap[1]))
	                                    | VIRGL_OBJ_SAMPLE_STATE_S0_WRAP_R(get_wrap_mode(create_info->wrap[2]))
	                                    | VIRGL_OBJ_SAMPLE_STATE_S0_MIN_IMG_FILTER(get_min_filter(create_info->min_filter))
	                                    | VIRGL_OBJ_SAMPLE_STATE_S0_MIN_MIP_FILTER(get_mip_filter(create_info->mip_filter))
	                                    | VIRGL_OBJ_SAMPLE_STATE_S0_MAG_IMG_FILTER(get_mag_filter(create_info->mag_filter))
	                                    | VIRGL_OBJ_SAMPLE_STATE_S0_COMPARE_MODE(create_info->compare_enable ? PIPE_TEX_COMPARE_R_TO_TEXTURE : PIPE_TEX_COMPARE_NONE)
	                                    | VIRGL_OBJ_SAMPLE_STATE_S0_COMPARE_FUNC(get_compare_func(create_info->compare_func))
	                                    | VIRGL_OBJ_SAMPLE_STATE_S0_SEAMLESS_CUBE_MAP(0) /* XXX */
	                                    | VIRGL_OBJ_SAMPLE_STATE_S0_MAX_ANISOTROPY(create_info->max_anisotropy),
	request[VIRGL_OBJ_SAMPLER_STATE_LOD_BIAS] = fconv(create_info->lod_bias);
	request[VIRGL_OBJ_SAMPLER_STATE_MIN_LOD] = create_info->min_lod;
	request[VIRGL_OBJ_SAMPLER_STATE_MAX_LOD] = create_info->max_lod;
	request[VIRGL_OBJ_SAMPLER_STATE_BORDER_COLOR(0)] = fconv(create_info->border_color[0]);
	request[VIRGL_OBJ_SAMPLER_STATE_BORDER_COLOR(1)] = fconv(create_info->border_color[1]);
	request[VIRGL_OBJ_SAMPLER_STATE_BORDER_COLOR(2)] = fconv(create_info->border_color[2]);
	request[VIRGL_OBJ_SAMPLER_STATE_BORDER_COLOR(3)] = fconv(create_info->border_color[3]);
	*samplerp = sampler;
	return  0;

err:
	if (sampler->id)
		virgl_free_id(ctx, sampler->id);
	free(sampler);
	return ret;
}

int
virgl_sampler_bind(struct jkg_ctx *ctx,
                   struct jkg_sampler **samplers,
                   uint32_t first,
                   uint32_t count)
{
	uint32_t *request;
	int ret;

	ret = virgl_req_alloc(ctx,
	                      VIRGL_CCMD_BIND_SAMPLER_STATES,
	                      VIRGL_OBJECT_NULL,
	                      VIRGL_BIND_SAMPLER_STATES(count),
	                      &request);
	if (ret)
		return ret;
	request[VIRGL_BIND_SAMPLER_STATES_SHADER_TYPE] = PIPE_SHADER_VERTEX | PIPE_SHADER_FRAGMENT; /* XXX */
	request[VIRGL_BIND_SAMPLER_STATES_START_SLOT] = first;
	for (uint32_t i = 0; i < count; ++i)
		request[VIRGL_BIND_SAMPLER_STATES_S0_HANDLE + i] = samplers[i] ? samplers[i]->id : 0;
	return 0;
}

void
virgl_sampler_free(struct jkg_ctx *ctx,
                   struct jkg_sampler *sampler)
{
	if (virgl_destroy_object(ctx, VIRGL_OBJECT_SAMPLER_STATE, sampler->id))
		assert(!"failed to destroy sampler");
	virgl_free_id(ctx, sampler->id);
	free(sampler);
}
