#include "virgl.h"

#include <sys/mman.h>

#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>

static const struct jkg_op
op =
{
	.destroy = virgl_destroy,
	.flush = virgl_flush,
	.resize = virgl_resize,
	.clear = virgl_clear,
	.draw = virgl_draw,
	.get_default_images = virgl_get_default_images,
	.set_index_buffer = virgl_set_index_buffer,
	.set_vertex_buffers = virgl_set_vertex_buffers,
	.set_uniform_blocks = virgl_set_uniform_blocks,
	.set_blend_color = virgl_set_blend_color,
	.set_viewport = virgl_set_viewport,
	.set_scissor = virgl_set_scissor,
	.set_stencil_ref = virgl_set_stencil_ref,

	.buffer_alloc = virgl_buffer_alloc,
	.buffer_read = virgl_buffer_read,
	.buffer_write = virgl_buffer_write,
	.buffer_copy = virgl_buffer_copy,
	.buffer_free = virgl_buffer_free,

	.shader_alloc = virgl_shader_alloc,
	.shader_free = virgl_shader_free,

	.shader_state_alloc = virgl_shader_state_alloc,
	.shader_state_bind = virgl_shader_state_bind,
	.shader_state_free = virgl_shader_state_free,

	.depth_stencil_state_alloc = virgl_depth_stencil_state_alloc,
	.depth_stencil_state_bind = virgl_depth_stencil_state_bind,
	.depth_stencil_state_free = virgl_depth_stencil_state_free,

	.rasterizer_state_alloc = virgl_rasterizer_state_alloc,
	.rasterizer_state_bind = virgl_rasterizer_state_bind,
	.rasterizer_state_free = virgl_rasterizer_state_free,

	.blend_state_alloc = virgl_blend_state_alloc,
	.blend_state_bind = virgl_blend_state_bind,
	.blend_state_free = virgl_blend_state_free,

	.input_layout_alloc = virgl_input_layout_alloc,
	.input_layout_bind = virgl_input_layout_bind,
	.input_layout_free = virgl_input_layout_free,

	.sampler_alloc = virgl_sampler_alloc,
	.sampler_bind = virgl_sampler_bind,
	.sampler_free = virgl_sampler_free,

	.image_view_alloc = virgl_image_view_alloc,
	.image_view_bind = virgl_image_view_bind,
	.image_view_free = virgl_image_view_free,

	.image_alloc = virgl_image_alloc,
	.image_read = virgl_image_read,
	.image_write = virgl_image_write,
	.image_copy = virgl_image_copy,
	.image_free = virgl_image_free,

	.surface_alloc = virgl_surface_alloc,
	.surface_bind = virgl_surface_bind,
	.surface_clear = virgl_surface_clear,
	.surface_free = virgl_surface_free,
};

struct jkg_ctx *
jkg_ctx_alloc(void)
{
	struct jkg_ctx *ctx;
	int ret;

	ctx = calloc(1, sizeof(*ctx));
	if (!ctx)
		return NULL;
	if (!jkg_id_init(&ctx->id_list, 1, UINT32_MAX - 1))
		goto err;
	ctx->op = &op;
	ctx->fd = open("/dev/virgl", O_RDONLY);
	if (ctx->fd == -1)
		goto err;
	if (vgl_get_capset_info(ctx,
	                        &ctx->capset_id,
	                        &ctx->capset_version,
	                        &ctx->capset_size))
		goto err;
	/* XXX assert capset id */
	switch (ctx->capset_version)
	{
		case 1:
			if (ctx->capset_size != sizeof(ctx->capset_v1))
				goto err;
			break;
		case 2:
			if (ctx->capset_size != sizeof(ctx->capset_v2))
				goto err;
			break;
		default:
			goto err;
	}
	if (vgl_get_capset(ctx, &ctx->capset, ctx->capset_size))
		goto err;
	ctx->caps_ptr = &ctx->caps;
	ctx->caps.vendor = "jkg";
	ctx->caps.version = "1.0";
	if (ctx->capset_version >= 2)
	{
		ctx->caps.max_vertex_attrib_stride = ctx->capset_v2.max_vertex_attrib_stride;
		ctx->caps.max_texture_2d_size = ctx->capset_v2.max_texture_2d_size;
		ctx->caps.max_texture_3d_size = ctx->capset_v2.max_texture_3d_size;
		ctx->caps.max_point_size = ctx->capset_v2.max_smooth_point_size;
		ctx->caps.max_line_width = ctx->capset_v2.max_smooth_line_width;
		ctx->caps.max_anisotropy = ctx->capset_v2.max_anisotropy;
		ctx->caps.max_renderbuffer_size = ctx->capset_v2.max_texture_2d_size;
		ctx->caps.max_framebuffer_width = ctx->capset_v2.max_texture_2d_size; /* XXX */
		ctx->caps.max_framebuffer_height = ctx->capset_v2.max_texture_2d_size; /* XXX */
		ctx->caps.max_vertex_attribs = ctx->capset_v2.max_vertex_attribs;
		ctx->caps.renderer = ctx->capset_v2.renderer;
		if (ctx->capset_v2.host_feature_check_version >= 21)
			ctx->caps.caps1 |= JKG_CAPS1_SURFACE_CLEAR;
	}
	else
	{
		ctx->caps.max_vertex_attrib_stride = 4096; /* XXX */
		ctx->caps.max_texture_2d_size = 65536; /* XXX */
		ctx->caps.max_texture_3d_size = 65536; /* XXX */
		ctx->caps.max_point_size = 1; /* XXX */
		ctx->caps.max_line_width = 1; /* XXX */
		ctx->caps.max_anisotropy = 1; /* XXX */
		ctx->caps.max_renderbuffer_size = 65536; /* XXX */
		ctx->caps.max_framebuffer_width = 65536; /* XXX */
		ctx->caps.max_framebuffer_height = 65536; /* XXX */
		ctx->caps.max_vertex_attribs = 16; /* XXX */
		ctx->caps.renderer = "virgl";
	}
	ctx->caps.max_texture_layers = ctx->capset_v1.max_texture_array_layers;
	ctx->caps.max_vertex_attrib_relative_offset = ctx->caps.max_vertex_attrib_stride - 1;
	ctx->caps.max_vertex_attrib_bindings = ctx->caps.max_vertex_attribs;
	ctx->caps.max_draw_buffers = ctx->capset_v1.max_render_targets;
	ctx->caps.max_texture_image_units = 16; /* XXX */
	ctx->caps.max_color_attachments = ctx->capset_v1.max_render_targets;
	ctx->caps.max_uniform_blocks = ctx->capset_v1.max_uniform_blocks;
	ctx->cmd_buf.size = 4096; /* XXX less arbitrary? */
	ctx->cmd_buf.data = malloc(ctx->cmd_buf.size); /* XXX mmap from virgl? */
	if (!ctx->cmd_buf.data)
		goto err;
	ret = vgl_create_context(ctx);
	if (ret)
		goto err;
	ret = virgl_resize(ctx, 640, 480);
	if (ret)
		goto err;
	return ctx;

err:
	virgl_destroy(ctx);
	return NULL;
}

void
virgl_destroy(struct jkg_ctx *ctx)
{
	/* XXX */
	if (ctx->default_color_images[0])
		virgl_image_free(ctx, ctx->default_color_images[0]);
	if (ctx->default_color_images[1])
		virgl_image_free(ctx, ctx->default_color_images[1]);
	if (ctx->default_depth_stencil_image)
		virgl_image_free(ctx, ctx->default_depth_stencil_image);
	if (ctx->ctx_fd != -1)
		close(ctx->ctx_fd);
	if (ctx->fd != -1)
		close(ctx->fd);
	free(ctx);
}

int
virgl_resize(struct jkg_ctx *ctx, uint32_t width, uint32_t height)
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
	ret = virgl_image_alloc(ctx, &image_create_info, &color_images[0]);
	if (ret)
		goto err;
	ret = virgl_image_alloc(ctx, &image_create_info, &color_images[1]);
	if (ret)
		goto err;
	image_create_info.format = JKG_FORMAT_D24_UNORM_S8_UINT;
	ret = virgl_image_alloc(ctx, &image_create_info, &depth_stencil_image);
	if (ret)
		goto err;
	if (ctx->default_color_images[0])
		virgl_image_free(ctx, ctx->default_color_images[0]);
	if (ctx->default_color_images[1])
		virgl_image_free(ctx, ctx->default_color_images[1]);
	if (ctx->default_depth_stencil_image)
		virgl_image_free(ctx, ctx->default_depth_stencil_image);
	ctx->default_color_images[0] = color_images[0];
	ctx->default_color_images[1] = color_images[1];
	ctx->default_depth_stencil_image = depth_stencil_image;
	ctx->width = width;
	ctx->height = height;
	return 0;

err:
	if (color_images[0])
		virgl_image_free(ctx, color_images[0]);
	if (color_images[1])
		virgl_image_free(ctx, color_images[1]);
	if (depth_stencil_image)
		virgl_image_free(ctx, depth_stencil_image);
	return ret;
}

int
virgl_flush(struct jkg_ctx *ctx)
{
	if (!ctx->cmd_buf.pos)
		return 0;
	return vgl_submit(ctx);
}

int
virgl_get_default_images(struct jkg_ctx *ctx,
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

int
virgl_alloc_id(struct jkg_ctx *ctx,
               uint32_t *id)
{
	if (!jkg_id_alloc(&ctx->id_list, id))
		return -ENOMEM;
	return 0;
}

int
virgl_free_id(struct jkg_ctx *ctx,
              uint32_t id)
{
	if (!jkg_id_free(&ctx->id_list, id))
		return -ENOMEM;
	return 0;
}

int
virgl_req_alloc(struct jkg_ctx *ctx,
                enum virgl_context_cmd cmd,
                enum virgl_object_type type,
                uint32_t size,
                uint32_t **requestp)
{
	uint32_t *request;
	uint32_t bytes;
	int ret;

	/* XXX should be ringbuf to allow asynchronous submission */
	bytes = 4 * (1 + size);
	if (ctx->cmd_buf.pos + bytes > ctx->cmd_buf.size)
	{
		ret = vgl_submit(ctx);
		if (ret)
			return ret;
		if (bytes > ctx->cmd_buf.size)
			return -EINVAL;
	}
	request = (uint32_t*)&ctx->cmd_buf.data[ctx->cmd_buf.pos];
	*request = VIRGL_CMD0(cmd, type, size);
	ctx->cmd_buf.pos += bytes;
	*requestp = request;
	return 0;
}

int
virgl_bind_object(struct jkg_ctx *ctx,
                  enum virgl_object_type type,
                  uint32_t id)
{
	uint32_t *request;
	int ret;

	ret = virgl_req_alloc(ctx,
	                      VIRGL_CCMD_BIND_OBJECT,
	                      type,
	                      VIRGL_OBJ_BIND_HANDLE,
	                      &request);
	if (ret)
		return ret;
	request[VIRGL_OBJ_BIND_HANDLE] = id;
	return 0;
}

int
virgl_destroy_object(struct jkg_ctx *ctx,
                     enum virgl_object_type type,
                     uint32_t id)
{
	uint32_t *request;
	int ret;

	ret = virgl_req_alloc(ctx,
	                      VIRGL_CCMD_DESTROY_OBJECT,
	                      type,
	                      VIRGL_OBJ_DESTROY_HANDLE,
	                      &request);
	if (ret)
		return ret;
	request[VIRGL_OBJ_DESTROY_HANDLE] = id;
	return 0;
}

enum virgl_formats
virgl_get_format(enum jkg_format format)
{
	switch (format)
	{
		case JKG_FORMAT_UNKNOWN:
			return VIRGL_FORMAT_NONE;
		case JKG_FORMAT_R8_UINT:
			return VIRGL_FORMAT_R8_UINT;
		case JKG_FORMAT_R8_SINT:
			return VIRGL_FORMAT_R8_SINT;
		case JKG_FORMAT_R8_UNORM:
			return VIRGL_FORMAT_R8_UNORM;
		case JKG_FORMAT_R8_SNORM:
			return VIRGL_FORMAT_R8_SNORM;
		case JKG_FORMAT_R8_USCALED:
			return VIRGL_FORMAT_R8_USCALED;
		case JKG_FORMAT_R8_SSCALED:
			return VIRGL_FORMAT_R8_SSCALED;
		case JKG_FORMAT_R8G8_UINT:
			return VIRGL_FORMAT_R8G8_UINT;
		case JKG_FORMAT_R8G8_SINT:
			return VIRGL_FORMAT_R8G8_SINT;
		case JKG_FORMAT_R8G8_UNORM:
			return VIRGL_FORMAT_R8G8_UNORM;
		case JKG_FORMAT_R8G8_SNORM:
			return VIRGL_FORMAT_R8G8_SNORM;
		case JKG_FORMAT_R8G8_USCALED:
			return VIRGL_FORMAT_R8G8_USCALED;
		case JKG_FORMAT_R8G8_SSCALED:
			return VIRGL_FORMAT_R8G8_SSCALED;
		case JKG_FORMAT_R8G8B8_UINT:
			return VIRGL_FORMAT_R8G8B8_UINT;
		case JKG_FORMAT_R8G8B8_SINT:
			return VIRGL_FORMAT_R8G8B8_SINT;
		case JKG_FORMAT_R8G8B8_UNORM:
			return VIRGL_FORMAT_R8G8B8_UNORM;
		case JKG_FORMAT_R8G8B8_SNORM:
			return VIRGL_FORMAT_R8G8B8_SNORM;
		case JKG_FORMAT_R8G8B8_USCALED:
			return VIRGL_FORMAT_R8G8B8_USCALED;
		case JKG_FORMAT_R8G8B8_SSCALED:
			return VIRGL_FORMAT_R8G8B8_SSCALED;
		case JKG_FORMAT_B8G8R8_UINT:
			return VIRGL_FORMAT_B8G8R8_UINT;
		case JKG_FORMAT_B8G8R8_SINT:
			return VIRGL_FORMAT_B8G8R8_SINT;
		case JKG_FORMAT_B8G8R8_UNORM:
			return VIRGL_FORMAT_B8G8R8_UNORM;
		case JKG_FORMAT_B8G8R8_SNORM:
			return VIRGL_FORMAT_B8G8R8_SNORM;
		case JKG_FORMAT_B8G8R8_USCALED:
			return VIRGL_FORMAT_B8G8R8_USCALED;
		case JKG_FORMAT_B8G8R8_SSCALED:
			return VIRGL_FORMAT_B8G8R8_SSCALED;
		case JKG_FORMAT_R8G8B8A8_UINT:
			return VIRGL_FORMAT_R8G8B8A8_UINT;
		case JKG_FORMAT_R8G8B8A8_SINT:
			return VIRGL_FORMAT_R8G8B8A8_SINT;
		case JKG_FORMAT_R8G8B8A8_UNORM:
			return VIRGL_FORMAT_R8G8B8A8_UNORM;
		case JKG_FORMAT_R8G8B8A8_SNORM:
			return VIRGL_FORMAT_R8G8B8A8_SNORM;
		case JKG_FORMAT_R8G8B8A8_USCALED:
			return VIRGL_FORMAT_R8G8B8A8_USCALED;
		case JKG_FORMAT_R8G8B8A8_SSCALED:
			return VIRGL_FORMAT_R8G8B8A8_SSCALED;
		case JKG_FORMAT_B8G8R8A8_UINT:
			return VIRGL_FORMAT_B8G8R8A8_UINT;
		case JKG_FORMAT_B8G8R8A8_SINT:
			return VIRGL_FORMAT_B8G8R8A8_SINT;
		case JKG_FORMAT_B8G8R8A8_UNORM:
			return VIRGL_FORMAT_B8G8R8A8_UNORM;
		case JKG_FORMAT_B8G8R8A8_SNORM:
			return VIRGL_FORMAT_B8G8R8A8_SNORM;
		case JKG_FORMAT_B8G8R8A8_USCALED:
			return VIRGL_FORMAT_B8G8R8A8_USCALED;
		case JKG_FORMAT_B8G8R8A8_SSCALED:
			return VIRGL_FORMAT_B8G8R8A8_SSCALED;
		case JKG_FORMAT_R16_UINT:
			return VIRGL_FORMAT_R16_UINT;
		case JKG_FORMAT_R16_SINT:
			return VIRGL_FORMAT_R16_SINT;
		case JKG_FORMAT_R16_UNORM:
			return VIRGL_FORMAT_R16_UNORM;
		case JKG_FORMAT_R16_SNORM:
			return VIRGL_FORMAT_R16_SNORM;
		case JKG_FORMAT_R16_USCALED:
			return VIRGL_FORMAT_R16_USCALED;
		case JKG_FORMAT_R16_SSCALED:
			return VIRGL_FORMAT_R16_SSCALED;
		case JKG_FORMAT_R16_SFLOAT:
			return VIRGL_FORMAT_R16_FLOAT;
		case JKG_FORMAT_R16G16_UINT:
			return VIRGL_FORMAT_R16G16_UINT;
		case JKG_FORMAT_R16G16_SINT:
			return VIRGL_FORMAT_R16G16_SINT;
		case JKG_FORMAT_R16G16_UNORM:
			return VIRGL_FORMAT_R16G16_UNORM;
		case JKG_FORMAT_R16G16_SNORM:
			return VIRGL_FORMAT_R16G16_SNORM;
		case JKG_FORMAT_R16G16_USCALED:
			return VIRGL_FORMAT_R16G16_USCALED;
		case JKG_FORMAT_R16G16_SSCALED:
			return VIRGL_FORMAT_R16G16_SSCALED;
		case JKG_FORMAT_R16G16_SFLOAT:
			return VIRGL_FORMAT_R16G16_FLOAT;
		case JKG_FORMAT_R16G16B16_UINT:
			return VIRGL_FORMAT_R16G16B16_UINT;
		case JKG_FORMAT_R16G16B16_SINT:
			return VIRGL_FORMAT_R16G16B16_SINT;
		case JKG_FORMAT_R16G16B16_UNORM:
			return VIRGL_FORMAT_R16G16B16_UNORM;
		case JKG_FORMAT_R16G16B16_SNORM:
			return VIRGL_FORMAT_R16G16B16_SNORM;
		case JKG_FORMAT_R16G16B16_USCALED:
			return VIRGL_FORMAT_R16G16B16_USCALED;
		case JKG_FORMAT_R16G16B16_SSCALED:
			return VIRGL_FORMAT_R16G16B16_SSCALED;
		case JKG_FORMAT_R16G16B16_SFLOAT:
			return VIRGL_FORMAT_R16G16B16_FLOAT;
		case JKG_FORMAT_R16G16B16A16_UINT:
			return VIRGL_FORMAT_R16G16B16A16_UINT;
		case JKG_FORMAT_R16G16B16A16_SINT:
			return VIRGL_FORMAT_R16G16B16A16_SINT;
		case JKG_FORMAT_R16G16B16A16_UNORM:
			return VIRGL_FORMAT_R16G16B16A16_UNORM;
		case JKG_FORMAT_R16G16B16A16_SNORM:
			return VIRGL_FORMAT_R16G16B16A16_SNORM;
		case JKG_FORMAT_R16G16B16A16_USCALED:
			return VIRGL_FORMAT_R16G16B16A16_USCALED;
		case JKG_FORMAT_R16G16B16A16_SSCALED:
			return VIRGL_FORMAT_R16G16B16A16_SSCALED;
		case JKG_FORMAT_R16G16B16A16_SFLOAT:
			return VIRGL_FORMAT_R16G16B16A16_FLOAT;
		case JKG_FORMAT_R32_UINT:
			return VIRGL_FORMAT_R32_UINT;
		case JKG_FORMAT_R32_SINT:
			return VIRGL_FORMAT_R32_SINT;
		case JKG_FORMAT_R32_SFLOAT:
			return VIRGL_FORMAT_R32_FLOAT;
		case JKG_FORMAT_R32G32_UINT:
			return VIRGL_FORMAT_R32G32_UINT;
		case JKG_FORMAT_R32G32_SINT:
			return VIRGL_FORMAT_R32G32_SINT;
		case JKG_FORMAT_R32G32_SFLOAT:
			return VIRGL_FORMAT_R32G32_FLOAT;
		case JKG_FORMAT_R32G32B32_UINT:
			return VIRGL_FORMAT_R32G32B32_UINT;
		case JKG_FORMAT_R32G32B32_SINT:
			return VIRGL_FORMAT_R32G32B32_SINT;
		case JKG_FORMAT_R32G32B32_SFLOAT:
			return VIRGL_FORMAT_R32G32B32_FLOAT;
		case JKG_FORMAT_R32G32B32A32_UINT:
			return VIRGL_FORMAT_R32G32B32A32_UINT;
		case JKG_FORMAT_R32G32B32A32_SINT:
			return VIRGL_FORMAT_R32G32B32A32_SINT;
		case JKG_FORMAT_R32G32B32A32_SFLOAT:
			return VIRGL_FORMAT_R32G32B32A32_FLOAT;
		case JKG_FORMAT_R64_UINT:
			return VIRGL_FORMAT_R64_UINT;
		case JKG_FORMAT_R64_SINT:
			return VIRGL_FORMAT_R64_SINT;
		case JKG_FORMAT_R64_SFLOAT:
			return VIRGL_FORMAT_R64_FLOAT;
		case JKG_FORMAT_R64G64_UINT:
			return VIRGL_FORMAT_R64G64_UINT;
		case JKG_FORMAT_R64G64_SINT:
			return VIRGL_FORMAT_R64G64_SINT;
		case JKG_FORMAT_R64G64_SFLOAT:
			return VIRGL_FORMAT_R64G64_FLOAT;
		case JKG_FORMAT_R64G64B64_UINT:
			return VIRGL_FORMAT_R64G64B64_UINT;
		case JKG_FORMAT_R64G64B64_SINT:
			return VIRGL_FORMAT_R64G64B64_SINT;
		case JKG_FORMAT_R64G64B64_SFLOAT:
			return VIRGL_FORMAT_R64G64B64_FLOAT;
		case JKG_FORMAT_R64G64B64A64_UINT:
			return VIRGL_FORMAT_R64G64B64A64_UINT;
		case JKG_FORMAT_R64G64B64A64_SINT:
			return VIRGL_FORMAT_R64G64B64A64_SINT;
		case JKG_FORMAT_R64G64B64A64_SFLOAT:
			return VIRGL_FORMAT_R64G64B64A64_FLOAT;
		case JKG_FORMAT_D24_UNORM_S8_UINT:
			return VIRGL_FORMAT_S8_UINT_Z24_UNORM; /* XXX ? */
		case JKG_FORMAT_D32_SFLOAT:
			return VIRGL_FORMAT_Z32_FLOAT;
		case JKG_FORMAT_S8_UINT:
			return VIRGL_FORMAT_S8_UINT;
		case JKG_FORMAT_BC1_RGB_UNORM_BLOCK:
			return VIRGL_FORMAT_DXT1_RGB;
		case JKG_FORMAT_BC1_RGBA_UNORM_BLOCK:
			return VIRGL_FORMAT_DXT1_RGBA;
		case JKG_FORMAT_BC2_UNORM_BLOCK:
			return VIRGL_FORMAT_DXT3_RGBA;
		case JKG_FORMAT_BC3_UNORM_BLOCK:
			return VIRGL_FORMAT_DXT5_RGBA;
		case JKG_FORMAT_BC4_UNORM_BLOCK:
			return VIRGL_FORMAT_RGTC1_UNORM;
		case JKG_FORMAT_BC4_SNORM_BLOCK:
			return VIRGL_FORMAT_RGTC1_SNORM;
		case JKG_FORMAT_BC5_UNORM_BLOCK:
			return VIRGL_FORMAT_RGTC2_UNORM;
		case JKG_FORMAT_BC5_SNORM_BLOCK:
			return VIRGL_FORMAT_RGTC2_SNORM;
		case JKG_FORMAT_R4G4_UNORM_PACK8:
			return VIRGL_FORMAT_R4A4_UNORM; /* XXX */
		case JKG_FORMAT_R4G4B4A4_UNORM_PACK16:
			return VIRGL_FORMAT_R4G4B4A4_UNORM;
		case JKG_FORMAT_B4G4R4A4_UNORM_PACK16:
			return VIRGL_FORMAT_B4G4R4A4_UNORM;
		case JKG_FORMAT_A4R4G4B4_UNORM_PACK16:
			return VIRGL_FORMAT_A4R4G4B4_UNORM;
		case JKG_FORMAT_A4B4G4R4_UNORM_PACK16:
			return VIRGL_FORMAT_A4B4G4R4_UNORM;
		case JKG_FORMAT_R5G6B5_UNORM_PACK16:
			return VIRGL_FORMAT_R5G6B5_UNORM;
		case JKG_FORMAT_B5G6R5_UNORM_PACK16:
			return VIRGL_FORMAT_B5G6R5_UNORM;
		case JKG_FORMAT_R5G5B5A1_UNORM_PACK16:
			return VIRGL_FORMAT_R5G5B5A1_UNORM;
		case JKG_FORMAT_B5G5R5A1_UNORM_PACK16:
			return VIRGL_FORMAT_B5G5R5A1_UNORM;
		case JKG_FORMAT_A1R5G5B5_UNORM_PACK16:
			return VIRGL_FORMAT_A1R5G5B5_UNORM;
		case JKG_FORMAT_A1B5G5R5_UNORM_PACK16:
			return VIRGL_FORMAT_A1B5G5R5_UNORM;
	}
	return VIRGL_FORMAT_NONE;
}

int
virgl_res_alloc(struct jkg_ctx *ctx,
                const struct virgl_res_info *info,
                struct virgl_res **resp)
{
	struct virgl_res *res;
	int ret;

	res = calloc(1, sizeof(*res));
	if (!res)
		return -ENOMEM;
	res->fd = -1;
	res->data = MAP_FAILED;
	res->info = *info;
	ret = vgl_create_resource(ctx, res);
	if (ret)
		goto err;
	*resp = res;
	return 0;

err:
	virgl_res_free(ctx, res);
	return ret;
}

void
virgl_res_free(struct jkg_ctx *ctx,
               struct virgl_res *res)
{
	(void)ctx;
	if (res->data != MAP_FAILED)
		munmap(res->data, res->info.bytes);
	if (res->fd != -1)
		close(res->fd);
	free(res);
}
