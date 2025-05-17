#include "utils.h"
#include "gjit.h"

#include <stdlib.h>
#include <assert.h>
#if defined(__i386__) || defined(__x86_64__)
#include <cpuid.h>
#endif

static const struct jkg_op
op =
{
	.destroy = gjit_destroy,
	.flush = gjit_flush,
	.resize = gjit_resize,
	.clear = gjit_clear,
	.draw = gjit_draw,
	.get_default_images = gjit_get_default_images,
	.set_index_buffer = gjit_set_index_buffer,
	.set_vertex_buffers = gjit_set_vertex_buffers,
	.set_uniform_blocks = gjit_set_uniform_blocks,
	.set_blend_color = gjit_set_blend_color,
	.set_viewport = gjit_set_viewport,
	.set_scissor = gjit_set_scissor,
	.set_stencil_ref = gjit_set_stencil_ref,
	.sample = gjit_sample,

	.buffer_alloc = gjit_buffer_alloc,
	.buffer_read = gjit_buffer_read,
	.buffer_write = gjit_buffer_write,
	.buffer_copy = gjit_buffer_copy,
	.buffer_free = gjit_buffer_free,

	.shader_alloc = gjit_shader_alloc,
	.shader_free = gjit_shader_free,

	.shader_state_alloc = gjit_shader_state_alloc,
	.shader_state_bind = gjit_shader_state_bind,
	.shader_state_free = gjit_shader_state_free,

	.depth_stencil_state_alloc = gjit_depth_stencil_state_alloc,
	.depth_stencil_state_bind = gjit_depth_stencil_state_bind,
	.depth_stencil_state_free = gjit_depth_stencil_state_free,

	.rasterizer_state_alloc = gjit_rasterizer_state_alloc,
	.rasterizer_state_free = gjit_rasterizer_state_free,
	.rasterizer_state_bind = gjit_rasterizer_state_bind,

	.blend_state_alloc = gjit_blend_state_alloc,
	.blend_state_bind = gjit_blend_state_bind,
	.blend_state_free = gjit_blend_state_free,

	.input_layout_alloc = gjit_input_layout_alloc,
	.input_layout_free = gjit_input_layout_free,
	.input_layout_bind = gjit_input_layout_bind,

	.sampler_alloc = gjit_sampler_alloc,
	.sampler_bind = gjit_sampler_bind,
	.sampler_free = gjit_sampler_free,

	.image_view_alloc = gjit_image_view_alloc,
	.image_view_bind = gjit_image_view_bind,
	.image_view_free = gjit_image_view_free,

	.image_alloc = gjit_image_alloc,
	.image_read = gjit_image_read,
	.image_write = gjit_image_write,
	.image_copy = gjit_image_copy,
	.image_free = gjit_image_free,

	.surface_alloc = gjit_surface_alloc,
	.surface_bind = gjit_surface_bind,
	.surface_clear = gjit_surface_clear,
	.surface_free = gjit_surface_free,
};

static const struct jkg_caps
caps =
{
	.vendor = "jkg",
	.renderer = "gjit",
	.version = "1.0",
	.max_vertex_attrib_relative_offset = 4095,
	.max_vertex_attrib_bindings = GJIT_MAX_VERTEX_BUFFERS,
	.max_vertex_attrib_stride = 4096,
	.max_vertex_attribs = GJIT_MAX_ATTRIBS,
	.max_draw_buffers = GJIT_MAX_ATTACHMENTS,
	.max_texture_image_units = GJIT_MAX_TEXTURE_IMAGE_UNITS,
	.max_texture_2d_size = 65536,
	.max_texture_3d_size = 65536,
	.max_texture_layers = 65536,
	.max_anisotropy = 16,
	.max_color_attachments = 32,
	.max_renderbuffer_size = 65536,
	.max_framebuffer_width = 65536,
	.max_framebuffer_height = 65536,
	.max_uniform_blocks = GJIT_MAX_UNIFORM_BLOCKS,
	.max_point_size = 128.0,
	.max_line_width = 128.0,
	.caps1 = JKG_CAPS1_SURFACE_CLEAR
	       | JKG_CAPS1_DRAW_QUADS,
};

#ifdef LIBGCCJIT_HAVE_gcc_jit_context_add_command_line_option
#if defined(__i386__) || defined(__x86_64__)
static void
set_compiler_options(gcc_jit_context *ctx)
{
	uint32_t eax, ebx, ecx, edx;
	if (!__get_cpuid(1, &eax, &ebx, &ecx, &edx))
		return;
	/* XXX only if GenuineIntel */
	switch (((eax >> 4) & 0xF) | ((eax >> 0xC) & 0xF0))
	{
		case 0x8E:
		case 0x9E:
			gcc_jit_context_add_command_line_option(ctx, "-march=skylake");
			gcc_jit_context_add_command_line_option(ctx, "-mtune=native");
			return;
	}
	if (edx & (1 << 25))
		gcc_jit_context_add_command_line_option(ctx, "-msse");
	if (edx & (1 << 26))
		gcc_jit_context_add_command_line_option(ctx, "-msse2");
	if (ecx & (1 << 0))
		gcc_jit_context_add_command_line_option(ctx, "-msse3");
	if (ecx & (1 << 9))
		gcc_jit_context_add_command_line_option(ctx, "-mssse3");
	if (ecx & (1 << 12))
		gcc_jit_context_add_command_line_option(ctx, "-mfma");
	if (ecx & (1 << 19))
		gcc_jit_context_add_command_line_option(ctx, "-msse4.1");
	if (ecx & (1 << 20))
		gcc_jit_context_add_command_line_option(ctx, "-msse4.2");
	if (ecx & (1 << 22))
		gcc_jit_context_add_command_line_option(ctx, "-mmovbe");
	if (ecx & (1 << 23))
		gcc_jit_context_add_command_line_option(ctx, "-mpopcnt");
	if (ecx & (1 << 28))
		gcc_jit_context_add_command_line_option(ctx, "-mavx");
	if (__get_cpuid_count(7, 0, &eax, &ebx, &ecx, &edx))
	{
		if (ebx & (1 << 5))
			gcc_jit_context_add_command_line_option(ctx, "-mavx2");
		if (ebx & (1 << 16))
			gcc_jit_context_add_command_line_option(ctx, "-mavx512f");
		if (ebx & (1 << 17))
			gcc_jit_context_add_command_line_option(ctx, "-mavx512dq");
		if (ebx & (1 << 21))
			gcc_jit_context_add_command_line_option(ctx, "-mavx512ifma");
		if (ebx & (1 << 26))
			gcc_jit_context_add_command_line_option(ctx, "-mavx512pf");
		if (ebx & (1 << 27))
			gcc_jit_context_add_command_line_option(ctx, "-mavx512er");
		if (ebx & (1 << 28))
			gcc_jit_context_add_command_line_option(ctx, "-mavx512cd");
		if (ebx & (1 << 30))
			gcc_jit_context_add_command_line_option(ctx, "-mavx512bw");
		if (ebx & (1 << 31))
			gcc_jit_context_add_command_line_option(ctx, "-mavx512vl");
		if (ecx & (1 << 1))
			gcc_jit_context_add_command_line_option(ctx, "-mavx512vbmi");
		if (ecx & (1 << 6))
			gcc_jit_context_add_command_line_option(ctx, "-mavx512vbmi2");
		if (ecx & (1 << 10))
			gcc_jit_context_add_command_line_option(ctx, "-mvpclmulqdq");
		if (ecx & (1 << 11))
			gcc_jit_context_add_command_line_option(ctx, "-mavx512vnni");
		if (ecx & (1 << 12))
			gcc_jit_context_add_command_line_option(ctx, "-mavx512bitalg");
		if (ecx & (1 << 14))
			gcc_jit_context_add_command_line_option(ctx, "-mavx512vpopcntdq");
	}
}
#endif
#endif

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
	ctx->jit.ctx = gcc_jit_context_acquire();
	if (!ctx->jit.ctx)
	{
		free(ctx);
		return NULL;
	}
#ifdef LIBGCCJIT_HAVE_gcc_jit_context_set_bool_allow_unreachable_blocks
	gcc_jit_context_set_bool_allow_unreachable_blocks(ctx->jit.ctx, 1);
#endif
	gcc_jit_context_set_int_option(ctx->jit.ctx,
	                               GCC_JIT_INT_OPTION_OPTIMIZATION_LEVEL,
	                               3);
	gcc_jit_context_set_bool_option(ctx->jit.ctx,
	                                GCC_JIT_BOOL_OPTION_DUMP_GENERATED_CODE,
	                                1);
	gcc_jit_context_set_bool_option(ctx->jit.ctx,
	                                GCC_JIT_BOOL_OPTION_DUMP_INITIAL_GIMPLE,
	                                1);
#ifdef LIBGCCJIT_HAVE_gcc_jit_context_add_command_line_option
#if defined(__i386__) || defined(__x86_64__)
	set_compiler_options(ctx->jit.ctx);
	gcc_jit_context_add_command_line_option(ctx->jit.ctx, "-ffast-math");
#endif
#endif
	ctx->jit.type_void = gcc_jit_context_get_type(ctx->jit.ctx, GCC_JIT_TYPE_VOID);
	ctx->jit.type_voidp = gcc_jit_context_get_type(ctx->jit.ctx, GCC_JIT_TYPE_VOID_PTR);
	ctx->jit.type_float = gcc_jit_context_get_type(ctx->jit.ctx, GCC_JIT_TYPE_FLOAT);
	ctx->jit.type_floatp = gcc_jit_type_get_pointer(ctx->jit.type_float);
	ctx->jit.type_const_floatp = gcc_jit_type_get_pointer(gcc_jit_type_get_const(ctx->jit.type_float));
	ctx->jit.type_float_3 = gcc_jit_context_new_array_type(ctx->jit.ctx, NULL, ctx->jit.type_float, 3);
	ctx->jit.type_float_4 = gcc_jit_context_new_array_type(ctx->jit.ctx, NULL, ctx->jit.type_float, 4);
	ctx->jit.type_float_8 = gcc_jit_context_new_array_type(ctx->jit.ctx, NULL, ctx->jit.type_float, 8);
	ctx->jit.type_float_16 = gcc_jit_context_new_array_type(ctx->jit.ctx, NULL, ctx->jit.type_float, 16);
	ctx->jit.type_double = gcc_jit_context_get_type(ctx->jit.ctx, GCC_JIT_TYPE_DOUBLE);
	ctx->jit.type_doublep = gcc_jit_type_get_pointer(ctx->jit.type_double);
	ctx->jit.type_const_doublep = gcc_jit_type_get_pointer(gcc_jit_type_get_const(ctx->jit.type_double));
	ctx->jit.type_int = gcc_jit_context_get_type(ctx->jit.ctx, GCC_JIT_TYPE_INT);
#ifdef LIBGCCJIT_HAVE_SIZED_INTEGERS
	ctx->jit.type_int8 = gcc_jit_context_get_type(ctx->jit.ctx, GCC_JIT_TYPE_INT8_T);
	ctx->jit.type_uint8 = gcc_jit_context_get_type(ctx->jit.ctx, GCC_JIT_TYPE_UINT8_T);
	ctx->jit.type_int16 = gcc_jit_context_get_type(ctx->jit.ctx, GCC_JIT_TYPE_INT16_T);
	ctx->jit.type_uint16 = gcc_jit_context_get_type(ctx->jit.ctx, GCC_JIT_TYPE_UINT16_T);
	ctx->jit.type_int32 = gcc_jit_context_get_type(ctx->jit.ctx, GCC_JIT_TYPE_INT32_T);
	ctx->jit.type_uint32 = gcc_jit_context_get_type(ctx->jit.ctx, GCC_JIT_TYPE_UINT32_T);
#else
	ctx->jit.type_int8 = gcc_jit_context_get_type(ctx->jit.ctx, GCC_JIT_TYPE_SIGNED_CHAR);
	ctx->jit.type_uint8 = gcc_jit_context_get_type(ctx->jit.ctx, GCC_JIT_TYPE_UNSIGNED_CHAR);
	ctx->jit.type_int16 = gcc_jit_context_get_type(ctx->jit.ctx, GCC_JIT_TYPE_SHORT);
	ctx->jit.type_uint16 = gcc_jit_context_get_type(ctx->jit.ctx, GCC_JIT_TYPE_UNSIGNED_SHORT);
	ctx->jit.type_int32 = gcc_jit_context_get_type(ctx->jit.ctx, GCC_JIT_TYPE_INT);
	ctx->jit.type_uint32 = gcc_jit_context_get_type(ctx->jit.ctx, GCC_JIT_TYPE_UNSIGNED_INT);
#endif
	ctx->jit.type_int8p = gcc_jit_type_get_pointer(ctx->jit.type_int8);
	ctx->jit.type_const_int8p = gcc_jit_type_get_pointer(gcc_jit_type_get_const(ctx->jit.type_int8));
	ctx->jit.type_uint8p = gcc_jit_type_get_pointer(ctx->jit.type_uint8);
	ctx->jit.type_const_uint8p = gcc_jit_type_get_pointer(gcc_jit_type_get_const(ctx->jit.type_uint8));
	ctx->jit.type_int16p = gcc_jit_type_get_pointer(ctx->jit.type_int16);
	ctx->jit.type_const_int16p = gcc_jit_type_get_pointer(gcc_jit_type_get_const(ctx->jit.type_int16));
	ctx->jit.type_uint16p = gcc_jit_type_get_pointer(ctx->jit.type_uint16);
	ctx->jit.type_const_uint16p = gcc_jit_type_get_pointer(gcc_jit_type_get_const(ctx->jit.type_uint16));
	ctx->jit.type_int32p = gcc_jit_type_get_pointer(ctx->jit.type_int32);
	ctx->jit.type_const_int32p = gcc_jit_type_get_pointer(gcc_jit_type_get_const(ctx->jit.type_int32));
	ctx->jit.type_int32_1 = gcc_jit_context_new_array_type(ctx->jit.ctx, NULL, ctx->jit.type_int32, 1);
	ctx->jit.type_int32_2 = gcc_jit_context_new_array_type(ctx->jit.ctx, NULL, ctx->jit.type_int32, 2);
	ctx->jit.type_int32_3 = gcc_jit_context_new_array_type(ctx->jit.ctx, NULL, ctx->jit.type_int32, 3);
	ctx->jit.type_int32_4 = gcc_jit_context_new_array_type(ctx->jit.ctx, NULL, ctx->jit.type_int32, 4);
	ctx->jit.type_int32_6 = gcc_jit_context_new_array_type(ctx->jit.ctx, NULL, ctx->jit.type_int32, 6);
	ctx->jit.type_int32_12 = gcc_jit_context_new_array_type(ctx->jit.ctx, NULL, ctx->jit.type_int32, 12);
	ctx->jit.type_uint32p = gcc_jit_type_get_pointer(ctx->jit.type_uint32);
	ctx->jit.type_const_uint32p = gcc_jit_type_get_pointer(gcc_jit_type_get_const(ctx->jit.type_uint32));
	ctx->jit.type_size_t = gcc_jit_context_get_type(ctx->jit.ctx, GCC_JIT_TYPE_SIZE_T);
#ifdef LIBGCCJIT_HAVE_gcc_jit_type_get_restrict
	ctx->jit.type_restrict_floatp = gcc_jit_type_get_restrict(ctx->jit.type_floatp);
	ctx->jit.type_const_restrict_floatp = gcc_jit_type_get_restrict(ctx->jit.type_const_floatp);
	ctx->jit.type_restrict_doublep = gcc_jit_type_get_restrict(ctx->jit.type_doublep);
	ctx->jit.type_const_restrict_doublep = gcc_jit_type_get_restrict(ctx->jit.type_const_doublep);
	ctx->jit.type_const_restrict_int8p = gcc_jit_type_get_restrict(ctx->jit.type_const_int8p);
	ctx->jit.type_const_restrict_uint8p = gcc_jit_type_get_restrict(ctx->jit.type_const_uint8p);
	ctx->jit.type_const_restrict_int16p = gcc_jit_type_get_restrict(ctx->jit.type_const_int16p);
	ctx->jit.type_const_restrict_uint16p = gcc_jit_type_get_restrict(ctx->jit.type_const_uint16p);
	ctx->jit.type_const_restrict_int32p = gcc_jit_type_get_restrict(ctx->jit.type_const_int32p);
	ctx->jit.type_const_restrict_uint32p = gcc_jit_type_get_restrict(ctx->jit.type_const_uint32p);
#else
	ctx->jit.type_restrict_floatp = ctx->jit.type_floatp;
	ctx->jit.type_const_restrict_floatp = ctx->jit.type_const_floatp;
	ctx->jit.type_restrict_doublep = ctx->jit.type_doublep;
	ctx->jit.type_const_restrict_doublep = ctx->jit.type_const_doublep;
	ctx->jit.type_const_restrict_int8p = ctx->jit.type_const_int8p;
	ctx->jit.type_const_restrict_uint8p = ctx->jit.type_const_uint8p;
	ctx->jit.type_const_restrict_int16p = ctx->jit.type_const_int16p;
	ctx->jit.type_const_restrict_uint16p = ctx->jit.type_const_uint16p;
	ctx->jit.type_const_restrict_int32p = ctx->jit.type_const_int32p;
	ctx->jit.type_const_restrict_uint32p = ctx->jit.type_const_uint32p;
#endif
	ctx->jit.type_buffer_field_data = gcc_jit_context_new_field(ctx->jit.ctx, NULL, ctx->jit.type_uint8p, "data");
	ctx->jit.type_buffer_field_size = gcc_jit_context_new_field(ctx->jit.ctx, NULL, ctx->jit.type_uint32, "size");
	gcc_jit_field *buffer_fields[] =
	{
		ctx->jit.type_buffer_field_data,
		ctx->jit.type_buffer_field_size,
	};
	ctx->jit.type_buffer = gcc_jit_context_new_struct_type(ctx->jit.ctx, NULL, "buffer", 2, buffer_fields);
	ctx->jit.type_bufferp = gcc_jit_type_get_pointer(gcc_jit_struct_as_type(ctx->jit.type_buffer));
	ctx->jit.type_vertex_buffer_field_stride = gcc_jit_context_new_field(ctx->jit.ctx, NULL, ctx->jit.type_uint32, "stride");
	ctx->jit.type_vertex_buffer_field_offset = gcc_jit_context_new_field(ctx->jit.ctx, NULL, ctx->jit.type_uint32, "offset");
	ctx->jit.type_vertex_buffer_field_buffer = gcc_jit_context_new_field(ctx->jit.ctx, NULL, ctx->jit.type_bufferp, "buffer");
	gcc_jit_field *vertex_buffer_fields[] =
	{
		ctx->jit.type_vertex_buffer_field_stride,
		ctx->jit.type_vertex_buffer_field_offset,
		ctx->jit.type_vertex_buffer_field_buffer,
	};
	ctx->jit.type_vertex_buffer = gcc_jit_context_new_struct_type(ctx->jit.ctx, NULL, "vertex_buffer", 3, vertex_buffer_fields);
	ctx->jit.type_vertex_bufferp = gcc_jit_type_get_pointer(gcc_jit_struct_as_type(ctx->jit.type_vertex_buffer));
	gcc_jit_type *image_view_fn_params[] =
	{
		ctx->jit.type_floatp,
		ctx->jit.type_const_int32p,
	};
	ctx->jit.type_image_view_fn = gcc_jit_context_new_function_ptr_type(ctx->jit.ctx, NULL, ctx->jit.type_void, 2, image_view_fn_params, 0);
	ctx->jit.value_inv_65535f = gcc_jit_context_new_rvalue_from_double(ctx->jit.ctx, ctx->jit.type_float, 1.0f / 65535.0f);
	ctx->jit.value_inv_32767f = gcc_jit_context_new_rvalue_from_double(ctx->jit.ctx, ctx->jit.type_float, 1.0f / 32767.0f);
	ctx->jit.value_inv_255f = gcc_jit_context_new_rvalue_from_double(ctx->jit.ctx, ctx->jit.type_float, 1.0f / 255.0f);
	ctx->jit.value_inv_127f = gcc_jit_context_new_rvalue_from_double(ctx->jit.ctx, ctx->jit.type_float, 1.0f / 127.0f);
	ctx->jit.value_inv_63f = gcc_jit_context_new_rvalue_from_double(ctx->jit.ctx, ctx->jit.type_float, 1.0f / 63.0f);
	ctx->jit.value_inv_31f = gcc_jit_context_new_rvalue_from_double(ctx->jit.ctx, ctx->jit.type_float, 1.0f / 31.0f);
	ctx->jit.value_inv_15f = gcc_jit_context_new_rvalue_from_double(ctx->jit.ctx, ctx->jit.type_float, 1.0f / 15.0f);
	ctx->jit.value_minus_one = gcc_jit_context_new_rvalue_from_int(ctx->jit.ctx, ctx->jit.type_int32, -1);
	for (size_t i = 0; i < sizeof(ctx->jit.value_f) / sizeof(*ctx->jit.value_f); ++i)
		ctx->jit.value_f[i] = gcc_jit_context_new_rvalue_from_int(ctx->jit.ctx, ctx->jit.type_float, i);
	for (size_t i = 0; i < sizeof(ctx->jit.value_i32) / sizeof(*ctx->jit.value_i32); ++i)
		ctx->jit.value_i32[i] = gcc_jit_context_new_rvalue_from_int(ctx->jit.ctx, ctx->jit.type_int32, i);
	gjit_blend_init(ctx);
	gjit_sampler_init(ctx);
	ret = gjit_resize(ctx, 640, 480);
	if (ret)
		goto err;
	return ctx;

err:
	gjit_destroy(ctx);
	return NULL;
}

void
gjit_destroy(struct jkg_ctx *ctx)
{
	/* XXX */
	if (ctx->default_color_images[0])
		gjit_image_free(ctx, ctx->default_color_images[0]);
	if (ctx->default_color_images[1])
		gjit_image_free(ctx, ctx->default_color_images[1]);
	if (ctx->default_depth_stencil_image)
		gjit_image_free(ctx, ctx->default_depth_stencil_image);
	free(ctx);
}

int
gjit_resize(struct jkg_ctx *ctx, uint32_t width, uint32_t height)
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
	ret = gjit_image_alloc(ctx, &image_create_info, &color_images[0]);
	if (ret)
		goto err;
	ret = gjit_image_alloc(ctx, &image_create_info, &color_images[1]);
	if (ret)
		goto err;
	image_create_info.format = JKG_FORMAT_D24_UNORM_S8_UINT;
	ret = gjit_image_alloc(ctx, &image_create_info, &depth_stencil_image);
	if (ret)
		goto err;
	if (ctx->default_color_images[0])
		gjit_image_free(ctx, ctx->default_color_images[0]);
	if (ctx->default_color_images[1])
		gjit_image_free(ctx, ctx->default_color_images[1]);
	if (ctx->default_depth_stencil_image)
		gjit_image_free(ctx, ctx->default_depth_stencil_image);
	ctx->default_color_images[0] = color_images[0];
	ctx->default_color_images[1] = color_images[1];
	ctx->default_depth_stencil_image = depth_stencil_image;
	ctx->width = width;
	ctx->height = height;
	return 0;

err:
	if (color_images[0])
		gjit_image_free(ctx, color_images[0]);
	if (color_images[1])
		gjit_image_free(ctx, color_images[1]);
	if (depth_stencil_image)
		gjit_image_free(ctx, depth_stencil_image);
	return ret;
}

int
gjit_flush(struct jkg_ctx *ctx)
{
	(void)ctx;
	return 0;
}

int
gjit_get_default_images(struct jkg_ctx *ctx,
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

static size_t
get_norm(struct jkg_ctx *ctx,
         struct gcc_jit_context *jit_ctx,
         gcc_jit_rvalue **dst,
         gcc_jit_rvalue *datav,
         gcc_jit_type *type,
         gcc_jit_rvalue *scale,
         size_t n)
{
	gcc_jit_rvalue *data = gcc_jit_context_new_cast(jit_ctx, NULL, datav, type);
	for (size_t i = 0; i < n; ++i)
	{
		gcc_jit_rvalue *comp = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(jit_ctx, NULL, data, ctx->jit.value_i32[i]));
		gcc_jit_rvalue *compf = gcc_jit_context_new_cast(jit_ctx, NULL, comp, ctx->jit.type_float);
		dst[i] = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MULT, ctx->jit.type_float, compf, scale);
	}
	return n;
}

static size_t
get_scaled(struct jkg_ctx *ctx,
           struct gcc_jit_context *jit_ctx,
           gcc_jit_rvalue **dst,
           gcc_jit_rvalue *datav,
           gcc_jit_type *type,
           size_t n)
{
	gcc_jit_rvalue *data = gcc_jit_context_new_cast(jit_ctx, NULL, datav, type);
	for (size_t i = 0; i < n; ++i)
	{
		gcc_jit_rvalue *comp = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(jit_ctx, NULL, data, ctx->jit.value_i32[i]));
		dst[i] = gcc_jit_context_new_cast(jit_ctx, NULL, comp, ctx->jit.type_float);
	}
	return n;
}

static size_t
get_8_unorm(struct jkg_ctx *ctx,
            struct gcc_jit_context *jit_ctx,
            gcc_jit_rvalue **dst,
            gcc_jit_rvalue *datav,
            size_t n)
{
	return get_norm(ctx, jit_ctx, dst, datav, ctx->jit.type_const_restrict_uint8p, ctx->jit.value_inv_255f, n);
}

static size_t
get_8_snorm(struct jkg_ctx *ctx,
            struct gcc_jit_context *jit_ctx,
            gcc_jit_rvalue **dst,
            gcc_jit_rvalue *datav,
            size_t n)
{
	return get_norm(ctx, jit_ctx, dst, datav, ctx->jit.type_const_restrict_int8p, ctx->jit.value_inv_127f, n);
}

static size_t
get_8_uscaled(struct jkg_ctx *ctx,
              struct gcc_jit_context *jit_ctx,
              gcc_jit_rvalue **dst,
              gcc_jit_rvalue *datav,
              size_t n)
{
	return get_scaled(ctx, jit_ctx, dst, datav, ctx->jit.type_const_restrict_uint8p, n);
}

static size_t
get_8_sscaled(struct jkg_ctx *ctx,
              struct gcc_jit_context *jit_ctx,
              gcc_jit_rvalue **dst,
              gcc_jit_rvalue *datav,
              size_t n)
{
	return get_scaled(ctx, jit_ctx, dst, datav, ctx->jit.type_const_restrict_int8p, n);
}

static size_t
get_16_unorm(struct jkg_ctx *ctx,
             struct gcc_jit_context *jit_ctx,
             gcc_jit_rvalue **dst,
             gcc_jit_rvalue *datav,
             size_t n)
{
	return get_norm(ctx, jit_ctx, dst, datav, ctx->jit.type_const_restrict_uint16p, ctx->jit.value_inv_65535f, n);
}

static size_t
get_16_snorm(struct jkg_ctx *ctx,
             struct gcc_jit_context *jit_ctx,
             gcc_jit_rvalue **dst,
             gcc_jit_rvalue *datav,
             size_t n)
{
	return get_norm(ctx, jit_ctx, dst, datav, ctx->jit.type_const_restrict_int16p, ctx->jit.value_inv_32767f, n);
}

static size_t
get_16_uscaled(struct jkg_ctx *ctx,
               struct gcc_jit_context *jit_ctx,
               gcc_jit_rvalue **dst,
               gcc_jit_rvalue *datav,
               size_t n)
{
	return get_scaled(ctx, jit_ctx, dst, datav, ctx->jit.type_const_restrict_uint16p, n);
}

static size_t
get_16_sscaled(struct jkg_ctx *ctx,
               struct gcc_jit_context *jit_ctx,
               gcc_jit_rvalue **dst,
               gcc_jit_rvalue *datav,
               size_t n)
{
	return get_scaled(ctx, jit_ctx, dst, datav, ctx->jit.type_const_restrict_int16p, n);
}

static size_t
get_32_sfloat(struct jkg_ctx *ctx,
              gcc_jit_context *jit_ctx,
              gcc_jit_rvalue **dst,
              gcc_jit_rvalue *datav,
              size_t n)
{
	gcc_jit_rvalue *dataf = gcc_jit_context_new_cast(jit_ctx, NULL, datav, ctx->jit.type_const_restrict_floatp);
	for (size_t i = 0; i < n; ++i)
		dst[i] = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(jit_ctx, NULL, dataf, ctx->jit.value_i32[i]));
	return n;
}

static size_t
get_64_sfloat(struct jkg_ctx *ctx,
              gcc_jit_context *jit_ctx,
              gcc_jit_rvalue **dst,
              gcc_jit_rvalue *datav,
              size_t n)
{
	gcc_jit_rvalue *datad = gcc_jit_context_new_cast(jit_ctx, NULL, datav, ctx->jit.type_const_restrict_doublep);
	for (size_t i = 0; i < n; ++i)
	{
		gcc_jit_rvalue *compd = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(jit_ctx, NULL, datad, ctx->jit.value_i32[i]));
		dst[i] = gcc_jit_context_new_cast(jit_ctx, NULL, compd, ctx->jit.type_float);
	}
	return n;
}

static size_t
get_rg_4_unorm(struct jkg_ctx *ctx,
               gcc_jit_context *jit_ctx,
               gcc_jit_rvalue **dst,
               gcc_jit_rvalue *datav)
{
	gcc_jit_rvalue *data8 = gcc_jit_context_new_cast(jit_ctx, NULL, datav, ctx->jit.type_const_restrict_uint8p);
	gcc_jit_rvalue *value = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(jit_ctx, NULL, data8, ctx->jit.value_i32[0]));
	gcc_jit_rvalue *b15 = gcc_jit_context_new_rvalue_from_int(jit_ctx, ctx->jit.type_int32, 0xF);
	for (size_t i = 0; i < 2; ++i)
	{
		gcc_jit_rvalue *shift = gcc_jit_context_new_rvalue_from_int(jit_ctx, ctx->jit.type_int32, (1 - i) * 4);
		gcc_jit_rvalue *shifted = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_RSHIFT, ctx->jit.type_uint8, value, shift);
		gcc_jit_rvalue *comp = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_BITWISE_AND, ctx->jit.type_uint8, shifted, b15);
		gcc_jit_rvalue *compf = gcc_jit_context_new_cast(jit_ctx, NULL, comp, ctx->jit.type_float);
		dst[i] = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MULT, ctx->jit.type_float, compf, ctx->jit.value_inv_15f);
	}
	return 2;
}

static size_t
get_rgba_4_unorm(struct jkg_ctx *ctx,
                 gcc_jit_context *jit_ctx,
                 gcc_jit_rvalue **dst,
                 gcc_jit_rvalue *datav)
{
	gcc_jit_rvalue *data16 = gcc_jit_context_new_cast(jit_ctx, NULL, datav, ctx->jit.type_const_restrict_uint16p);
	gcc_jit_rvalue *value = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(jit_ctx, NULL, data16, ctx->jit.value_i32[0]));
	gcc_jit_rvalue *b15 = gcc_jit_context_new_rvalue_from_int(jit_ctx, ctx->jit.type_int32, 0xF);
	for (size_t i = 0; i < 4; ++i)
	{
		gcc_jit_rvalue *shift = gcc_jit_context_new_rvalue_from_int(jit_ctx, ctx->jit.type_int32, (3 - i) * 4);
		gcc_jit_rvalue *shifted = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_RSHIFT, ctx->jit.type_uint16, value, shift);
		gcc_jit_rvalue *comp = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_BITWISE_AND, ctx->jit.type_uint16, shifted, b15);
		gcc_jit_rvalue *compf = gcc_jit_context_new_cast(jit_ctx, NULL, comp, ctx->jit.type_float);
		dst[i] = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MULT, ctx->jit.type_float, compf, ctx->jit.value_inv_15f);
	}
	return 4;
}

static size_t
get_rgb_565_unorm(struct jkg_ctx *ctx,
                  gcc_jit_context *jit_ctx,
                  gcc_jit_rvalue **dst,
                  gcc_jit_rvalue *datav)
{
	gcc_jit_rvalue *data16 = gcc_jit_context_new_cast(jit_ctx, NULL, datav, ctx->jit.type_const_restrict_uint16p);
	gcc_jit_rvalue *value = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(jit_ctx, NULL, data16, ctx->jit.value_i32[0]));
	gcc_jit_rvalue *b31 = gcc_jit_context_new_rvalue_from_int(jit_ctx, ctx->jit.type_int32, 0x1F);
	gcc_jit_rvalue *b63 = gcc_jit_context_new_rvalue_from_int(jit_ctx, ctx->jit.type_int32, 0x3F);
	static const int shifts[3] =
	{
		11,
		5,
		0,
	};
	for (size_t i = 0; i < 3; ++i)
	{
		gcc_jit_rvalue *shift = gcc_jit_context_new_rvalue_from_int(jit_ctx, ctx->jit.type_int32, shifts[i]);
		gcc_jit_rvalue *shifted = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_RSHIFT, ctx->jit.type_uint16, value, shift);
		gcc_jit_rvalue *comp = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_BITWISE_AND, ctx->jit.type_uint16, shifted, i == 1 ? b63 : b31);
		gcc_jit_rvalue *compf = gcc_jit_context_new_cast(jit_ctx, NULL, comp, ctx->jit.type_float);
		dst[i] = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MULT, ctx->jit.type_float, compf, i == 1 ? ctx->jit.value_inv_63f : ctx->jit.value_inv_31f);
	}
	return 3;
}

static size_t
get_rgba_5551_unorm(struct jkg_ctx *ctx,
                    gcc_jit_context *jit_ctx,
                    gcc_jit_rvalue **dst,
                    gcc_jit_rvalue *datav)
{
	gcc_jit_rvalue *data16 = gcc_jit_context_new_cast(jit_ctx, NULL, datav, ctx->jit.type_const_restrict_uint16p);
	gcc_jit_rvalue *value = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(jit_ctx, NULL, data16, ctx->jit.value_i32[0]));
	gcc_jit_rvalue *b31 = gcc_jit_context_new_rvalue_from_int(jit_ctx, ctx->jit.type_int32, 0x1F);
	static const int shifts[4] =
	{
		11,
		6,
		1,
		0,
	};
	for (size_t i = 0; i < 4; ++i)
	{
		gcc_jit_rvalue *shift = gcc_jit_context_new_rvalue_from_int(jit_ctx, ctx->jit.type_int32, shifts[i]);
		gcc_jit_rvalue *shifted = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_RSHIFT, ctx->jit.type_uint16, value, shift);
		gcc_jit_rvalue *comp = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_BITWISE_AND, ctx->jit.type_uint16, shifted, i == 3 ? ctx->jit.value_i32[1] : b31);
		gcc_jit_rvalue *compf = gcc_jit_context_new_cast(jit_ctx, NULL, comp, ctx->jit.type_float);
		dst[i] = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MULT, ctx->jit.type_float, compf, i == 3 ? ctx->jit.value_f[1] : ctx->jit.value_inv_31f);
	}
	return 4;
}

static void
swap_bgr(gcc_jit_rvalue **dst)
{
	gcc_jit_rvalue *tmp;

	tmp = dst[0];
	dst[0] = dst[2];
	dst[2] = tmp;
}

static void
swap_argb(gcc_jit_rvalue **dst)
{
	gcc_jit_rvalue *tmp[4];

	tmp[0] = dst[0];
	tmp[1] = dst[1];
	tmp[2] = dst[2];
	tmp[3] = dst[3];
	dst[0] = tmp[1];
	dst[1] = tmp[2];
	dst[2] = tmp[3];
	dst[3] = tmp[0];
}

void
gjit_get_vec4f(struct jkg_ctx *ctx,
               gcc_jit_context *jit_ctx,
               gcc_jit_rvalue **dst,
               gcc_jit_rvalue *datav,
               enum jkg_format format)
{
	size_t n = 0;
	switch (format)
	{
		case JKG_FORMAT_R8_UNORM:
			n = get_8_unorm(ctx, jit_ctx, dst, datav, 1);
			break;
		case JKG_FORMAT_R8_SNORM:
			n = get_8_snorm(ctx, jit_ctx, dst, datav, 1);
			break;
		case JKG_FORMAT_R8_USCALED:
			n = get_8_uscaled(ctx, jit_ctx, dst, datav, 1);
			break;
		case JKG_FORMAT_R8_SSCALED:
			n = get_8_sscaled(ctx, jit_ctx, dst, datav, 1);
			break;
		case JKG_FORMAT_R8G8_UNORM:
			n = get_8_unorm(ctx, jit_ctx, dst, datav, 2);
			break;
		case JKG_FORMAT_R8G8_SNORM:
			n = get_8_snorm(ctx, jit_ctx, dst, datav, 2);
			break;
		case JKG_FORMAT_R8G8_USCALED:
			n = get_8_uscaled(ctx, jit_ctx, dst, datav, 2);
			break;
		case JKG_FORMAT_R8G8_SSCALED:
			n = get_8_sscaled(ctx, jit_ctx, dst, datav, 2);
			break;
		case JKG_FORMAT_R8G8B8_UNORM:
			n = get_8_unorm(ctx, jit_ctx, dst, datav, 3);
			break;
		case JKG_FORMAT_R8G8B8_SNORM:
			n = get_8_snorm(ctx, jit_ctx, dst, datav, 3);
			break;
		case JKG_FORMAT_R8G8B8_USCALED:
			n = get_8_uscaled(ctx, jit_ctx, dst, datav, 3);
			break;
		case JKG_FORMAT_R8G8B8_SSCALED:
			n = get_8_sscaled(ctx, jit_ctx, dst, datav, 3);
			break;
		case JKG_FORMAT_B8G8R8_UNORM:
			n = get_8_unorm(ctx, jit_ctx, dst, datav, 3);
			swap_bgr(dst);
			break;
		case JKG_FORMAT_B8G8R8_SNORM:
			n = get_8_snorm(ctx, jit_ctx, dst, datav, 3);
			swap_bgr(dst);
			break;
		case JKG_FORMAT_B8G8R8_USCALED:
			n = get_8_uscaled(ctx, jit_ctx, dst, datav, 3);
			swap_bgr(dst);
			break;
		case JKG_FORMAT_B8G8R8_SSCALED:
			n = get_8_sscaled(ctx, jit_ctx, dst, datav, 3);
			swap_bgr(dst);
			break;
		case JKG_FORMAT_R8G8B8A8_UNORM:
			n = get_8_unorm(ctx, jit_ctx, dst, datav, 4);
			break;
		case JKG_FORMAT_R8G8B8A8_SNORM:
			n = get_8_snorm(ctx, jit_ctx, dst, datav, 4);
			break;
		case JKG_FORMAT_R8G8B8A8_USCALED:
			n = get_8_uscaled(ctx, jit_ctx, dst, datav, 4);
			break;
		case JKG_FORMAT_R8G8B8A8_SSCALED:
			n = get_8_sscaled(ctx, jit_ctx, dst, datav, 4);
			break;
		case JKG_FORMAT_B8G8R8A8_UNORM:
			n = get_8_unorm(ctx, jit_ctx, dst, datav, 4);
			swap_bgr(dst);
			break;
		case JKG_FORMAT_B8G8R8A8_SNORM:
			n = get_8_snorm(ctx, jit_ctx, dst, datav, 4);
			swap_bgr(dst);
			break;
		case JKG_FORMAT_B8G8R8A8_USCALED:
			n = get_8_uscaled(ctx, jit_ctx, dst, datav, 4);
			swap_bgr(dst);
			break;
		case JKG_FORMAT_B8G8R8A8_SSCALED:
			n = get_8_sscaled(ctx, jit_ctx, dst, datav, 4);
			swap_bgr(dst);
			break;
		case JKG_FORMAT_R16_UNORM:
			n = get_16_unorm(ctx, jit_ctx, dst, datav, 1);
			break;
		case JKG_FORMAT_R16_SNORM:
			n = get_16_snorm(ctx, jit_ctx, dst, datav, 1);
			break;
		case JKG_FORMAT_R16_USCALED:
			n = get_16_uscaled(ctx, jit_ctx, dst, datav, 1);
			break;
		case JKG_FORMAT_R16_SSCALED:
			n = get_16_sscaled(ctx, jit_ctx, dst, datav, 1);
			break;
		case JKG_FORMAT_R16G16_UNORM:
			n = get_16_unorm(ctx, jit_ctx, dst, datav, 2);
			break;
		case JKG_FORMAT_R16G16_SNORM:
			n = get_16_snorm(ctx, jit_ctx, dst, datav, 2);
			break;
		case JKG_FORMAT_R16G16_USCALED:
			n = get_16_uscaled(ctx, jit_ctx, dst, datav, 2);
			break;
		case JKG_FORMAT_R16G16_SSCALED:
			n = get_16_sscaled(ctx, jit_ctx, dst, datav, 2);
			break;
		case JKG_FORMAT_R16G16B16_UNORM:
			n = get_16_unorm(ctx, jit_ctx, dst, datav, 3);
			break;
		case JKG_FORMAT_R16G16B16_SNORM:
			n = get_16_snorm(ctx, jit_ctx, dst, datav, 3);
			break;
		case JKG_FORMAT_R16G16B16_USCALED:
			n = get_16_uscaled(ctx, jit_ctx, dst, datav, 3);
			break;
		case JKG_FORMAT_R16G16B16_SSCALED:
			n = get_16_sscaled(ctx, jit_ctx, dst, datav, 3);
			break;
		case JKG_FORMAT_R16G16B16A16_UNORM:
			n = get_16_unorm(ctx, jit_ctx, dst, datav, 4);
			break;
		case JKG_FORMAT_R16G16B16A16_SNORM:
			n = get_16_snorm(ctx, jit_ctx, dst, datav, 4);
			break;
		case JKG_FORMAT_R16G16B16A16_USCALED:
			n = get_16_uscaled(ctx, jit_ctx, dst, datav, 4);
			break;
		case JKG_FORMAT_R16G16B16A16_SSCALED:
			n = get_16_sscaled(ctx, jit_ctx, dst, datav, 4);
			break;
		case JKG_FORMAT_R32_SFLOAT:
			n = get_32_sfloat(ctx, jit_ctx, dst, datav, 1);
			break;
		case JKG_FORMAT_R32G32_SFLOAT:
			n = get_32_sfloat(ctx, jit_ctx, dst, datav, 2);
			break;
		case JKG_FORMAT_R32G32B32_SFLOAT:
			n = get_32_sfloat(ctx, jit_ctx, dst, datav, 3);
			break;
		case JKG_FORMAT_R32G32B32A32_SFLOAT:
			n = get_32_sfloat(ctx, jit_ctx, dst, datav, 4);
			break;
		case JKG_FORMAT_R64_SFLOAT:
			n = get_64_sfloat(ctx, jit_ctx, dst, datav, 1);
			break;
		case JKG_FORMAT_R64G64_SFLOAT:
			n = get_64_sfloat(ctx, jit_ctx, dst, datav, 2);
			break;
		case JKG_FORMAT_R64G64B64_SFLOAT:
			n = get_64_sfloat(ctx, jit_ctx, dst, datav, 3);
			break;
		case JKG_FORMAT_R64G64B64A64_SFLOAT:
			n = get_64_sfloat(ctx, jit_ctx, dst, datav, 4);
			break;
		case JKG_FORMAT_R4G4_UNORM_PACK8:
			n = get_rg_4_unorm(ctx, jit_ctx, dst, datav);
			break;
		case JKG_FORMAT_R4G4B4A4_UNORM_PACK16:
			n = get_rgba_4_unorm(ctx, jit_ctx, dst, datav);
			break;
		case JKG_FORMAT_B4G4R4A4_UNORM_PACK16:
			n = get_rgba_4_unorm(ctx, jit_ctx, dst, datav);
			swap_bgr(dst);
			break;
		case JKG_FORMAT_A4R4G4B4_UNORM_PACK16:
			n = get_rgba_4_unorm(ctx, jit_ctx, dst, datav);
			swap_argb(dst);
			break;
		case JKG_FORMAT_A4B4G4R4_UNORM_PACK16:
			n = get_rgba_4_unorm(ctx, jit_ctx, dst, datav);
			swap_argb(dst);
			swap_bgr(dst);
			break;
		case JKG_FORMAT_R5G6B5_UNORM_PACK16:
			n = get_rgb_565_unorm(ctx, jit_ctx, dst, datav);
			break;
		case JKG_FORMAT_B5G6R5_UNORM_PACK16:
			n = get_rgb_565_unorm(ctx, jit_ctx, dst, datav);
			swap_bgr(dst);
			break;
		case JKG_FORMAT_R5G5B5A1_UNORM_PACK16:
			n = get_rgba_5551_unorm(ctx, jit_ctx, dst, datav);
			break;
		case JKG_FORMAT_B5G5R5A1_UNORM_PACK16:
			n = get_rgba_5551_unorm(ctx, jit_ctx, dst, datav);
			swap_bgr(dst);
			break;
		case JKG_FORMAT_A1R5G5B5_UNORM_PACK16:
			n = get_rgba_5551_unorm(ctx, jit_ctx, dst, datav);
			swap_argb(dst);
			break;
		case JKG_FORMAT_A1B5G5R5_UNORM_PACK16:
			n = get_rgba_5551_unorm(ctx, jit_ctx, dst, datav);
			swap_argb(dst);
			swap_bgr(dst);
			break;
		case JKG_FORMAT_UNKNOWN:
		case JKG_FORMAT_R8_UINT:
		case JKG_FORMAT_R8_SINT:
		case JKG_FORMAT_R8G8_UINT:
		case JKG_FORMAT_R8G8_SINT:
		case JKG_FORMAT_R8G8B8_UINT:
		case JKG_FORMAT_R8G8B8_SINT:
		case JKG_FORMAT_B8G8R8_UINT:
		case JKG_FORMAT_B8G8R8_SINT:
		case JKG_FORMAT_R8G8B8A8_UINT:
		case JKG_FORMAT_R8G8B8A8_SINT:
		case JKG_FORMAT_B8G8R8A8_UINT:
		case JKG_FORMAT_B8G8R8A8_SINT:
		case JKG_FORMAT_R16_UINT:
		case JKG_FORMAT_R16_SINT:
		case JKG_FORMAT_R16G16_UINT:
		case JKG_FORMAT_R16G16_SINT:
		case JKG_FORMAT_R16G16B16_UINT:
		case JKG_FORMAT_R16G16B16_SINT:
		case JKG_FORMAT_R16G16B16A16_UINT:
		case JKG_FORMAT_R16G16B16A16_SINT:
		case JKG_FORMAT_R32_UINT:
		case JKG_FORMAT_R32_SINT:
		case JKG_FORMAT_R32G32_UINT:
		case JKG_FORMAT_R32G32_SINT:
		case JKG_FORMAT_R32G32B32_UINT:
		case JKG_FORMAT_R32G32B32_SINT:
		case JKG_FORMAT_R32G32B32A32_UINT:
		case JKG_FORMAT_R32G32B32A32_SINT:
		case JKG_FORMAT_R64_UINT:
		case JKG_FORMAT_R64_SINT:
		case JKG_FORMAT_R64G64_UINT:
		case JKG_FORMAT_R64G64_SINT:
		case JKG_FORMAT_R64G64B64_UINT:
		case JKG_FORMAT_R64G64B64_SINT:
		case JKG_FORMAT_R64G64B64A64_UINT:
		case JKG_FORMAT_R64G64B64A64_SINT:
		case JKG_FORMAT_D24_UNORM_S8_UINT:
		case JKG_FORMAT_D32_SFLOAT:
		case JKG_FORMAT_S8_UINT:
		case JKG_FORMAT_BC1_RGB_UNORM_BLOCK:
		case JKG_FORMAT_BC1_RGBA_UNORM_BLOCK:
		case JKG_FORMAT_BC2_UNORM_BLOCK:
		case JKG_FORMAT_BC3_UNORM_BLOCK:
		case JKG_FORMAT_BC4_UNORM_BLOCK:
		case JKG_FORMAT_BC4_SNORM_BLOCK:
		case JKG_FORMAT_BC5_UNORM_BLOCK:
		case JKG_FORMAT_BC5_SNORM_BLOCK:
			break;
	}
	for (size_t i = n; i < 4; ++i)
		dst[i] = i == 3 ? ctx->jit.value_f[1] : ctx->jit.value_f[0];
}
