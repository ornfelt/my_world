#include "gjit.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

static gcc_jit_function *
jit_input_layout(struct jkg_ctx *ctx,
                 gcc_jit_context *jit_ctx,
                 const struct jkg_input_layout_create_info *create_info);

int
gjit_input_layout_alloc(struct jkg_ctx *ctx,
                        const struct jkg_input_layout_create_info *create_info,
                        struct jkg_input_layout **input_layoutp)
{
	struct jkg_input_layout *input_layout;
	gcc_jit_context *jit_ctx;

	input_layout = malloc(sizeof(*input_layout));
	if (!input_layout)
		return -ENOMEM;
	jit_ctx = gcc_jit_context_new_child_context(ctx->jit.ctx);
	jit_input_layout(ctx, jit_ctx, create_info);
	input_layout->res = gcc_jit_context_compile(jit_ctx);
	gcc_jit_context_release(jit_ctx);
	input_layout->fn = gcc_jit_result_get_code(input_layout->res, "input_layout");
	*input_layoutp = input_layout;
	return 0;
}

int
gjit_input_layout_bind(struct jkg_ctx *ctx,
                       struct jkg_input_layout *input_layout)
{
	ctx->input_layout = input_layout;
	return 0;
}

void
gjit_input_layout_free(struct jkg_ctx *ctx,
                       struct jkg_input_layout *input_layout)
{
	(void)ctx;
	gcc_jit_result_release(input_layout->res);
	free(input_layout);
}

static gcc_jit_function *
jit_input_layout(struct jkg_ctx *ctx,
                 gcc_jit_context *jit_ctx,
                 const struct jkg_input_layout_create_info *create_info)
{
	gcc_jit_param *params[] =
	{
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_restrict_floatp, "vert"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_vertex_bufferp, "buffers"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_uint32, "index"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_uint32, "instance"),
	};
	gcc_jit_function *fn = gcc_jit_context_new_function(jit_ctx, NULL, GCC_JIT_FUNCTION_EXPORTED, ctx->jit.type_void, "input_layout", sizeof(params) / sizeof(*params), params, 0);
	gcc_jit_rvalue *vert = gcc_jit_param_as_rvalue(params[0]);
	gcc_jit_rvalue *buffers = gcc_jit_param_as_rvalue(params[1]);
	gcc_jit_rvalue *index = gcc_jit_param_as_rvalue(params[2]);
	gcc_jit_rvalue *instance = gcc_jit_param_as_rvalue(params[3]);
	gcc_jit_lvalue *data_indexed = gcc_jit_function_new_local(fn, NULL, ctx->jit.type_voidp, "data_indexed");
	gcc_jit_block *block = gcc_jit_function_new_block(fn, NULL);
	for (size_t i = 0; i < create_info->nattribs; ++i)
	{
		const struct jkg_input_layout_attrib *attrib = &create_info->attribs[i];
		if (!attrib->enable)
		{
			for (size_t j = 0; j < 4; ++j)
			{
				gcc_jit_rvalue *jdx = gcc_jit_context_new_rvalue_from_int(jit_ctx, ctx->jit.type_int, i * 4 + j);
				gcc_jit_lvalue *lvalue = gcc_jit_context_new_array_access(jit_ctx, NULL, vert, jdx);
				gcc_jit_rvalue *expr = j == 3 ? ctx->jit.value_f[1] : ctx->jit.value_f[0];
				gcc_jit_block_add_assignment(block, NULL, lvalue, expr);
			}
			continue;
		}
		gcc_jit_rvalue *idx = gcc_jit_context_new_rvalue_from_int(jit_ctx, ctx->jit.type_int, attrib->binding);
		gcc_jit_rvalue *vertex_buffer = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(jit_ctx, NULL, buffers, idx));
		gcc_jit_rvalue *offset;
		{
			gcc_jit_rvalue *tmp_offset;
			if (attrib->divisor)
			{
				gcc_jit_rvalue *divisor = gcc_jit_context_new_rvalue_from_int(jit_ctx, ctx->jit.type_uint32, attrib->divisor);
				gcc_jit_rvalue *expr = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_DIVIDE, ctx->jit.type_uint32, instance, divisor);
				tmp_offset = expr;
			}
			else
			{
				tmp_offset = index;
			}
			gcc_jit_rvalue *vertex_buffer_stride = gcc_jit_rvalue_access_field(vertex_buffer, NULL, ctx->jit.type_vertex_buffer_field_stride);
			gcc_jit_rvalue *vertex_buffer_offset = gcc_jit_rvalue_access_field(vertex_buffer, NULL, ctx->jit.type_vertex_buffer_field_offset);
			gcc_jit_rvalue *stride_expr = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MULT, ctx->jit.type_uint32, tmp_offset, vertex_buffer_stride);
			offset = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_PLUS, ctx->jit.type_uint32, stride_expr, vertex_buffer_offset);
		}
		{
			gcc_jit_rvalue *buffer = gcc_jit_rvalue_access_field(vertex_buffer, NULL, ctx->jit.type_vertex_buffer_field_buffer);
			gcc_jit_rvalue *buffer_data = gcc_jit_lvalue_as_rvalue(gcc_jit_rvalue_dereference_field(buffer, NULL, ctx->jit.type_buffer_field_data));
			gcc_jit_rvalue *attrib_offset = gcc_jit_context_new_rvalue_from_int(jit_ctx, ctx->jit.type_uint32, attrib->offset);
			gcc_jit_rvalue *data_index = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_PLUS, ctx->jit.type_uint32, offset, attrib_offset);
			gcc_jit_block_add_assignment(block, NULL, data_indexed, gcc_jit_lvalue_get_address(gcc_jit_context_new_array_access(jit_ctx, NULL, buffer_data, data_index), NULL));
		}
		gcc_jit_rvalue *values[4];
		gjit_get_vec4f(ctx, jit_ctx, values, gcc_jit_lvalue_as_rvalue(data_indexed), attrib->format);
		for (size_t j = 0; j < 4; ++j)
		{
			gcc_jit_rvalue *jdx = gcc_jit_context_new_rvalue_from_int(jit_ctx, ctx->jit.type_int, i * 4 + j);
			gcc_jit_lvalue *lvalue = gcc_jit_context_new_array_access(jit_ctx, NULL, vert, jdx);
			gcc_jit_block_add_assignment(block, NULL, lvalue, values[j]);
		}
	}
	gcc_jit_block_end_with_void_return(block, NULL);
	return fn;
}
