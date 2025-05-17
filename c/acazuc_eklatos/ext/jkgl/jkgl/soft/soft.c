#include "utils.h"
#include "soft.h"

#include <stdlib.h>

static const struct jkg_op
op =
{
	.destroy = soft_destroy,
	.flush = soft_flush,
	.resize = soft_resize,
	.clear = soft_clear,
	.draw = soft_draw,
	.get_default_images = soft_get_default_images,
	.set_index_buffer = soft_set_index_buffer,
	.set_vertex_buffers = soft_set_vertex_buffers,
	.set_uniform_blocks = soft_set_uniform_blocks,
	.set_blend_color = soft_set_blend_color,
	.set_viewport = soft_set_viewport,
	.set_scissor = soft_set_scissor,
	.set_stencil_ref = soft_set_stencil_ref,
	.sample = soft_sample,

	.buffer_alloc = soft_buffer_alloc,
	.buffer_read = soft_buffer_read,
	.buffer_write = soft_buffer_write,
	.buffer_copy = soft_buffer_copy,
	.buffer_free = soft_buffer_free,

	.shader_alloc = soft_shader_alloc,
	.shader_free = soft_shader_free,

	.shader_state_alloc = soft_shader_state_alloc,
	.shader_state_bind = soft_shader_state_bind,
	.shader_state_free = soft_shader_state_free,

	.depth_stencil_state_alloc = soft_depth_stencil_state_alloc,
	.depth_stencil_state_bind = soft_depth_stencil_state_bind,
	.depth_stencil_state_free = soft_depth_stencil_state_free,

	.rasterizer_state_alloc = soft_rasterizer_state_alloc,
	.rasterizer_state_bind = soft_rasterizer_state_bind,
	.rasterizer_state_free = soft_rasterizer_state_free,

	.blend_state_alloc = soft_blend_state_alloc,
	.blend_state_bind = soft_blend_state_bind,
	.blend_state_free = soft_blend_state_free,

	.input_layout_alloc = soft_input_layout_alloc,
	.input_layout_bind = soft_input_layout_bind,
	.input_layout_free = soft_input_layout_free,

	.sampler_alloc = soft_sampler_alloc,
	.sampler_bind = soft_sampler_bind,
	.sampler_free = soft_sampler_free,

	.image_view_alloc = soft_image_view_alloc,
	.image_view_bind = soft_image_view_bind,
	.image_view_free = soft_image_view_free,

	.image_alloc = soft_image_alloc,
	.image_read = soft_image_read,
	.image_write = soft_image_write,
	.image_copy = soft_image_copy,
	.image_free = soft_image_free,

	.surface_alloc = soft_surface_alloc,
	.surface_bind = soft_surface_bind,
	.surface_clear = soft_surface_clear,
	.surface_free = soft_surface_free,
};

static const struct jkg_caps
caps =
{
	.vendor = "jkg",
	.renderer = "soft",
	.version = "1.0",
	.max_vertex_attrib_relative_offset = 4095,
	.max_vertex_attrib_bindings = SOFT_MAX_VERTEX_BUFFERS,
	.max_vertex_attrib_stride = 4096,
	.max_vertex_attribs = SOFT_MAX_ATTRIBS,
	.max_draw_buffers = SOFT_MAX_ATTACHMENTS,
	.max_texture_image_units = SOFT_MAX_TEXTURE_IMAGE_UNITS,
	.max_texture_2d_size = 65536,
	.max_texture_3d_size = 65536,
	.max_texture_layers = 65536,
	.max_anisotropy = 16,
	.max_color_attachments = SOFT_MAX_ATTACHMENTS,
	.max_renderbuffer_size = 65536,
	.max_framebuffer_width = 65536,
	.max_framebuffer_height = 65536,
	.max_uniform_blocks = SOFT_MAX_UNIFORM_BLOCKS,
	.max_point_size = 128.0,
	.max_line_width = 128.0,
	.caps1 = JKG_CAPS1_SURFACE_CLEAR
	       | JKG_CAPS1_DRAW_QUADS,
};

struct jkg_ctx *
jkg_ctx_alloc(void)
{
	struct jkg_ctx *ctx;
	int ret;

	ctx = calloc(1, sizeof(*ctx));
	if (!ctx)
		return NULL;
	ctx->caps = &caps;
	ctx->op = &op;
	ret = soft_resize(ctx, 640, 480);
	if (ret)
		goto err;
	return ctx;

err:
	soft_destroy(ctx);
	return NULL;
}

void
soft_destroy(struct jkg_ctx *ctx)
{
	/* XXX */
	if (ctx->default_color_images[0])
		soft_image_free(ctx, ctx->default_color_images[0]);
	if (ctx->default_color_images[1])
		soft_image_free(ctx, ctx->default_color_images[1]);
	if (ctx->default_depth_stencil_image)
		soft_image_free(ctx, ctx->default_depth_stencil_image);
	free(ctx);
}

int
soft_resize(struct jkg_ctx *ctx, uint32_t width, uint32_t height)
{
	struct jkg_image_create_info image_create_info;
	struct jkg_image *color_images[2] = {NULL};
	struct jkg_image *depth_stencil_image = NULL;
	int ret;

	image_create_info.size.x = width;
	image_create_info.size.y = height;
	image_create_info.size.z = 0;
	image_create_info.type = JKG_IMAGE_2D;
	image_create_info.levels = 1;
	image_create_info.layers = 0;
	image_create_info.format = JKG_FORMAT_R32G32B32A32_SFLOAT;
	ret = soft_image_alloc(ctx, &image_create_info, &color_images[0]);
	if (ret)
		goto err;
	ret = soft_image_alloc(ctx, &image_create_info, &color_images[1]);
	if (ret)
		goto err;
	image_create_info.format = JKG_FORMAT_D24_UNORM_S8_UINT;
	ret = soft_image_alloc(ctx, &image_create_info, &depth_stencil_image);
	if (ret)
		goto err;
	if (ctx->default_color_images[0])
		soft_image_free(ctx, ctx->default_color_images[0]);
	if (ctx->default_color_images[1])
		soft_image_free(ctx, ctx->default_color_images[1]);
	if (ctx->default_depth_stencil_image)
		soft_image_free(ctx, ctx->default_depth_stencil_image);
	ctx->default_color_images[0] = color_images[0];
	ctx->default_color_images[1] = color_images[1];
	ctx->default_depth_stencil_image = depth_stencil_image;
	ctx->width = width;
	ctx->height = height;
	return 0;

err:
	if (color_images[0])
		soft_image_free(ctx, color_images[0]);
	if (color_images[1])
		soft_image_free(ctx, color_images[1]);
	if (depth_stencil_image)
		soft_image_free(ctx, depth_stencil_image);
	return ret;
}

int
soft_flush(struct jkg_ctx *ctx)
{
	(void)ctx;
	return 0;
}

int
soft_get_default_images(struct jkg_ctx *ctx,
                        struct jkg_image **colors,
                        enum jkg_format *color_format,
                        struct jkg_image **depth,
                        enum jkg_format *depth_format,
                        struct jkg_image **stencil,
                        enum jkg_format *stencil_format,
                        uint32_t *width,
                        uint32_t *height)
{
	colors[0] = ctx->default_color_images[0];
	colors[1] = ctx->default_color_images[1];
	*color_format = ctx->default_color_images[0]->format;
	*depth = ctx->default_depth_stencil_image;
	*depth_format = ctx->default_depth_stencil_image->format;
	*stencil = ctx->default_depth_stencil_image;
	*stencil_format = ctx->default_depth_stencil_image->format;
	*width = ctx->width;
	*height = ctx->height;
	return 0;
}
