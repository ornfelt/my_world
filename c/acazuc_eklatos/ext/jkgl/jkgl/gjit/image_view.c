#include "gjit.h"

#include <stdlib.h>
#include <assert.h>
#include <errno.h>

static gcc_jit_function *
jit_image_view(struct jkg_ctx *ctx,
               gcc_jit_context *jit_ctx,
               const struct jkg_image_view_create_info *create_info);

int
gjit_image_view_alloc(struct jkg_ctx *ctx,
                      const struct jkg_image_view_create_info *create_info,
                      struct jkg_image_view **image_viewp)
{
	struct jkg_image_view *image_view = NULL;
	gcc_jit_context *jit_ctx;

	(void)ctx;
	image_view = malloc(sizeof(*image_view));
	if (!image_view)
		return -ENOMEM;
	jit_ctx = gcc_jit_context_new_child_context(ctx->jit.ctx);
	jit_image_view(ctx, jit_ctx, create_info);
	image_view->res = gcc_jit_context_compile(jit_ctx);
	gcc_jit_context_release(jit_ctx);
	image_view->fn = gcc_jit_result_get_code(image_view->res, "image_view");
	image_view->image = create_info->image;
	image_view->min_level = create_info->min_level;
	image_view->num_levels = create_info->num_levels;
	image_view->min_layer = create_info->min_layer;
	image_view->num_layers = create_info->num_layers;
	*image_viewp = image_view;
	return 0;
}

int
gjit_image_view_bind(struct jkg_ctx *ctx,
                     struct jkg_image_view **image_view,
                     uint32_t first,
                     uint32_t count)
{
	for (uint32_t i = 0; i < count; ++i)
		ctx->image_views[first + i] = image_view[i];
	return 0;
}

void
gjit_image_view_free(struct jkg_ctx *ctx,
                     struct jkg_image_view *image_view)
{
	(void)ctx;
	gcc_jit_result_release(image_view->res);
	free(image_view);
}

static gcc_jit_function *
jit_image_view(struct jkg_ctx *ctx,
               gcc_jit_context *jit_ctx,
               const struct jkg_image_view_create_info *create_info)
{
	gcc_jit_param *params[] =
	{
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_restrict_floatp, "color"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_const_int32p, "coords"),
	};
	gcc_jit_function *fn = gcc_jit_context_new_function(jit_ctx, NULL, GCC_JIT_FUNCTION_EXPORTED, ctx->jit.type_void, "image_view", sizeof(params) / sizeof(*params), params, 0);
	gcc_jit_rvalue *color = gcc_jit_param_as_rvalue(params[0]);
	gcc_jit_rvalue *coords = gcc_jit_param_as_rvalue(params[1]);
	gcc_jit_block *block = gcc_jit_function_new_block(fn, "main");
	gcc_jit_rvalue *width = gcc_jit_context_new_rvalue_from_int(jit_ctx, ctx->jit.type_int32, create_info->image->size.x);
	gcc_jit_rvalue *height = gcc_jit_context_new_rvalue_from_int(jit_ctx, ctx->jit.type_int32, create_info->image->size.y);
	gcc_jit_lvalue *index = gcc_jit_function_new_local(fn, NULL, ctx->jit.type_int32, "index");
	gcc_jit_lvalue *data_indexed = gcc_jit_function_new_local(fn, NULL, ctx->jit.type_voidp, "data_indexed");
	switch (create_info->image->type)
	{
		default:
			assert(!"unknown texture type");
			/* FALLTHROUGH */
		case JKG_IMAGE_1D:
		{
			gcc_jit_rvalue *coord_x = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(jit_ctx, NULL, coords, ctx->jit.value_i32[0]));
			gcc_jit_block_add_assignment(block, NULL, index, coord_x);
			break;
		}
		case JKG_IMAGE_1D_ARRAY:
		{
			gcc_jit_rvalue *coord_x = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(jit_ctx, NULL, coords, ctx->jit.value_i32[0]));
			gcc_jit_rvalue *coord_y = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(jit_ctx, NULL, coords, ctx->jit.value_i32[1]));
			gcc_jit_rvalue *coord_y_width = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MULT, ctx->jit.type_int32, coord_y, width);
			gcc_jit_block_add_assignment(block, NULL, index, gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_PLUS, ctx->jit.type_int32, coord_x, coord_y_width));
			break;
		}
		case JKG_IMAGE_2D:
		{
			gcc_jit_rvalue *coord_x = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(jit_ctx, NULL, coords, ctx->jit.value_i32[0]));
			gcc_jit_rvalue *coord_y = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(jit_ctx, NULL, coords, ctx->jit.value_i32[1]));
			gcc_jit_rvalue *coord_y_width = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MULT, ctx->jit.type_int32, coord_y, width);
			gcc_jit_block_add_assignment(block, NULL, index, gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_PLUS, ctx->jit.type_int32, coord_x, coord_y_width));
			break;
		}
		case JKG_IMAGE_2D_ARRAY:
		{
			gcc_jit_rvalue *coord_x = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(jit_ctx, NULL, coords, ctx->jit.value_i32[0]));
			gcc_jit_rvalue *coord_y = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(jit_ctx, NULL, coords, ctx->jit.value_i32[1]));
			gcc_jit_rvalue *coord_z = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(jit_ctx, NULL, coords, ctx->jit.value_i32[2]));
			gcc_jit_rvalue *zh = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MULT, ctx->jit.type_int32, coord_z, height);
			gcc_jit_rvalue *y_zh = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_PLUS, ctx->jit.type_int32, zh, coord_y);
			gcc_jit_rvalue *y_zh_width = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MULT, ctx->jit.type_int32, y_zh, width);
			gcc_jit_block_add_assignment(block, NULL, index, gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_PLUS, ctx->jit.type_int32, coord_x, y_zh_width));
			break;
		}
		case JKG_IMAGE_3D:
		{
			gcc_jit_rvalue *coord_x = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(jit_ctx, NULL, coords, ctx->jit.value_i32[0]));
			gcc_jit_rvalue *coord_y = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(jit_ctx, NULL, coords, ctx->jit.value_i32[1]));
			gcc_jit_rvalue *coord_z = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(jit_ctx, NULL, coords, ctx->jit.value_i32[2]));
			gcc_jit_rvalue *zh = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MULT, ctx->jit.type_int32, coord_z, height);
			gcc_jit_rvalue *y_zh = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_PLUS, ctx->jit.type_int32, zh, coord_y);
			gcc_jit_rvalue *y_zh_width = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MULT, ctx->jit.type_int32, y_zh, width);
			gcc_jit_block_add_assignment(block, NULL, index, gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_PLUS, ctx->jit.type_int32, coord_x, y_zh_width));
			break;
		}
	}
	gcc_jit_rvalue *rgba[4];
	gcc_jit_rvalue *stride = gcc_jit_context_new_rvalue_from_int(jit_ctx, ctx->jit.type_int32, jkg_get_stride(create_info->format));
	gcc_jit_rvalue *offset = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MULT, ctx->jit.type_int32, stride, gcc_jit_lvalue_as_rvalue(index));
	gcc_jit_rvalue *image_data = gcc_jit_context_new_rvalue_from_ptr(jit_ctx, ctx->jit.type_uint8p, create_info->image->data);
	gcc_jit_block_add_assignment(block, NULL, data_indexed, gcc_jit_lvalue_get_address(gcc_jit_context_new_array_access(jit_ctx, NULL, image_data, offset), NULL));
	gjit_get_vec4f(ctx, jit_ctx, rgba, gcc_jit_lvalue_as_rvalue(data_indexed), create_info->format);
	for (size_t i = 0; i < 4; ++i)
	{
		gcc_jit_rvalue *rvalue;
		switch (create_info->swizzle[i])
		{
			case JKG_SWIZZLE_R:
				rvalue = rgba[0];
				break;
			case JKG_SWIZZLE_G:
				rvalue = rgba[1];
				break;
			case JKG_SWIZZLE_B:
				rvalue = rgba[2];
				break;
			case JKG_SWIZZLE_A:
				rvalue = rgba[3];
				break;
			default:
				assert(!"unknown swizzle");
				/* FALLTHROUGH */
			case JKG_SWIZZLE_ZERO:
				rvalue = ctx->jit.value_f[0];
				break;
			case JKG_SWIZZLE_ONE:
				rvalue = ctx->jit.value_f[1];
				break;
		}
		gcc_jit_block_add_assignment(block, NULL, gcc_jit_context_new_array_access(jit_ctx, NULL, color, gcc_jit_context_new_rvalue_from_int(jit_ctx, ctx->jit.type_int, i)), rvalue);
	}
	gcc_jit_block_end_with_void_return(block, NULL);
	return fn;
}
