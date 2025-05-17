#include "soft.h"

#include <stdlib.h>
#include <errno.h>

int
soft_sampler_alloc(struct jkg_ctx *ctx,
                   const struct jkg_sampler_create_info *create_info,
                   struct jkg_sampler **samplerp)
{
	struct jkg_sampler *sampler = NULL;

	(void)ctx;
	sampler = malloc(sizeof(*sampler));
	if (!sampler)
		return -ENOMEM;
	sampler->wrap[0] = create_info->wrap[0];
	sampler->wrap[1] = create_info->wrap[1];
	sampler->wrap[2] = create_info->wrap[2];
	sampler->min_filter = create_info->min_filter;
	sampler->mag_filter = create_info->mag_filter;
	sampler->mip_filter = create_info->mip_filter;
	sampler->compare_enable = create_info->compare_enable;
	sampler->compare_func = create_info->compare_func;
	sampler->lod_bias = create_info->lod_bias;
	sampler->min_lod = create_info->min_lod;
	sampler->max_lod = create_info->max_lod;
	sampler->max_anisotropy = create_info->max_anisotropy;
	sampler->border_color[0] = create_info->border_color[0];
	sampler->border_color[1] = create_info->border_color[1];
	sampler->border_color[2] = create_info->border_color[2];
	sampler->border_color[3] = create_info->border_color[3];
	*samplerp = sampler;
	return  0;
}

int
soft_sampler_bind(struct jkg_ctx *ctx,
                  struct jkg_sampler **samplers,
                  uint32_t first,
                  uint32_t count)
{
	for (uint32_t i = 0; i < count; ++i)
		ctx->samplers[first + i] = samplers[i];
	return 0;
}

void
soft_sampler_free(struct jkg_ctx *ctx,
                  struct jkg_sampler *sampler)
{
	(void)ctx;
	free(sampler);
}
