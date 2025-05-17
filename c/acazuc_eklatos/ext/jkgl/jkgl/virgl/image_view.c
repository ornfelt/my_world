#include "virgl.h"

#include <stdlib.h>
#include <assert.h>
#include <errno.h>

static enum pipe_swizzle
get_swizzle(enum jkg_swizzle swizzle)
{
	switch (swizzle)
	{
		default:
			assert(!"unknown swizzle");
			/* FALLTHROUGH */
		case JKG_SWIZZLE_R:
			return PIPE_SWIZZLE_RED;
		case JKG_SWIZZLE_G:
			return PIPE_SWIZZLE_GREEN;
		case JKG_SWIZZLE_B:
			return PIPE_SWIZZLE_BLUE;
		case JKG_SWIZZLE_A:
			return PIPE_SWIZZLE_ALPHA;
		case JKG_SWIZZLE_ZERO:
			return PIPE_SWIZZLE_ZERO;
		case JKG_SWIZZLE_ONE:
			return PIPE_SWIZZLE_ONE;
	}
}

int
virgl_image_view_alloc(struct jkg_ctx *ctx,
                       const struct jkg_image_view_create_info *create_info,
                       struct jkg_image_view **image_viewp)
{
	struct jkg_image_view *image_view = NULL;
	uint32_t *request;
	int ret;

	image_view = calloc(1, sizeof(*image_view));
	if (!image_view)
		return -ENOMEM;
	ret = virgl_alloc_id(ctx, &image_view->id);
	if (ret)
		goto err;
	ret = virgl_req_alloc(ctx,
	                      VIRGL_CCMD_CREATE_OBJECT,
	                      VIRGL_OBJECT_SAMPLER_VIEW,
	                      VIRGL_OBJ_SAMPLER_VIEW_SIZE,
	                      &request);
	if (ret)
		goto err;
	request[VIRGL_OBJ_SAMPLER_VIEW_HANDLE] = image_view->id;
	request[VIRGL_OBJ_SAMPLER_VIEW_RES_HANDLE] = create_info->image->res->id;
	request[VIRGL_OBJ_SAMPLER_VIEW_FORMAT] = virgl_get_format(create_info->format);
	request[VIRGL_OBJ_SAMPLER_VIEW_TEXTURE_LAYER] = (create_info->min_layer)
	                                              | ((create_info->min_layer + create_info->num_layers) << 16);
	request[VIRGL_OBJ_SAMPLER_VIEW_TEXTURE_LEVEL] = (create_info->min_level)
	                                              | ((create_info->min_level + create_info->num_levels) << 16);
	request[VIRGL_OBJ_SAMPLER_VIEW_SWIZZLE] = VIRGL_OBJ_SAMPLER_VIEW_SWIZZLE_R(get_swizzle(create_info->swizzle[0]))
	                                        | VIRGL_OBJ_SAMPLER_VIEW_SWIZZLE_G(get_swizzle(create_info->swizzle[1]))
	                                        | VIRGL_OBJ_SAMPLER_VIEW_SWIZZLE_B(get_swizzle(create_info->swizzle[2]))
	                                        | VIRGL_OBJ_SAMPLER_VIEW_SWIZZLE_A(get_swizzle(create_info->swizzle[3]));
	*image_viewp = image_view;
	return 0;

err:
	if (image_view->id)
		virgl_free_id(ctx, image_view->id);
	free(image_view);
	return ret;
}

int
virgl_image_view_bind(struct jkg_ctx *ctx,
                      struct jkg_image_view **image_views,
                      uint32_t first,
                      uint32_t count)
{
	uint32_t *request;
	int ret;

	ret = virgl_req_alloc(ctx,
	                      VIRGL_CCMD_SET_SAMPLER_VIEWS,
	                      VIRGL_OBJECT_NULL,
	                      VIRGL_SET_SAMPLER_VIEWS_SIZE(count),
	                      &request);
	if (ret)
		return ret;
	request[VIRGL_SET_SAMPLER_VIEWS_SHADER_TYPE] = PIPE_SHADER_VERTEX | PIPE_SHADER_FRAGMENT; /* XXX */
	request[VIRGL_SET_SAMPLER_VIEWS_START_SLOT] = first;
	for (size_t i = 0; i < count; ++i)
		request[VIRGL_SET_SAMPLER_VIEWS_V0_HANDLE + i] = image_views[i] ? image_views[i]->id : 0;
	return 0;
}

void
virgl_image_view_free(struct jkg_ctx *ctx,
                      struct jkg_image_view *image_view)
{
	if (virgl_destroy_object(ctx, VIRGL_OBJECT_SAMPLER_VIEW, image_view->id))
		assert(!"failed to destroy image view");
	virgl_free_id(ctx, image_view->id);
	free(image_view);
}
