#include "soft.h"

#include <stdlib.h>
#include <errno.h>

int
soft_image_view_alloc(struct jkg_ctx *ctx,
                      const struct jkg_image_view_create_info *create_info,
                      struct jkg_image_view **image_viewp)
{
	struct jkg_image_view *image_view = NULL;

	(void)ctx;
	image_view = malloc(sizeof(*image_view));
	if (!image_view)
		return -ENOMEM;
	image_view->image = create_info->image;
	image_view->min_level = create_info->min_level;
	image_view->num_levels = create_info->num_levels;
	image_view->min_layer = create_info->min_layer;
	image_view->num_layers = create_info->num_layers;
	image_view->format = create_info->format;
	image_view->swizzle[0] = create_info->swizzle[0];
	image_view->swizzle[1] = create_info->swizzle[1];
	image_view->swizzle[2] = create_info->swizzle[2];
	image_view->swizzle[3] = create_info->swizzle[3];
	*image_viewp = image_view;
	return 0;
}

int
soft_image_view_bind(struct jkg_ctx *ctx,
                     struct jkg_image_view **image_view,
                     uint32_t first,
                     uint32_t count)
{
	for (uint32_t i = 0; i < count; ++i)
		ctx->image_views[first + i] = image_view[i];
	return 0;
}

void
soft_image_view_free(struct jkg_ctx *ctx,
                     struct jkg_image_view *image_view)
{
	(void)ctx;
	free(image_view);
}
