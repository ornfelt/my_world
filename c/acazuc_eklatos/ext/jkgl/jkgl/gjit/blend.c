#include "gjit.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>

static gcc_jit_function *
jit_blend(struct jkg_ctx *ctx,
          gcc_jit_context *jit_ctx,
          const struct jkg_blend_state_create_info *create_info);

int
gjit_blend_state_alloc(struct jkg_ctx *ctx,
                       const struct jkg_blend_state_create_info *create_info,
                       struct jkg_blend_state **statep)
{
	struct jkg_blend_state *state;
	gcc_jit_context *jit_ctx;

	state = malloc(sizeof(*state));
	if (!state)
		return -ENOMEM;
	state->logic_enable = create_info->logic_enable;
	state->logic_op = create_info->logic_op;
	state->nattachments = create_info->nattachments;
	memcpy(state->attachments,
	       create_info->attachments,
	       sizeof(*state->attachments) * state->nattachments);
	jit_ctx = gcc_jit_context_new_child_context(ctx->jit.blend.ctx);
	jit_blend(ctx, jit_ctx, create_info);
	state->res = gcc_jit_context_compile(jit_ctx);
	gcc_jit_context_release(jit_ctx);
	state->fn = gcc_jit_result_get_code(state->res, "blend");
	*statep = state;
	return 0;
}

int
gjit_blend_state_bind(struct jkg_ctx *ctx,
                      struct jkg_blend_state *state)
{
	ctx->blend_state = state;
	return 0;
}

void
gjit_blend_state_free(struct jkg_ctx *ctx,
                      struct jkg_blend_state *state)
{
	(void)ctx;
	gcc_jit_result_release(state->res);
	free(state);
}

int
gjit_set_blend_color(struct jkg_ctx *ctx,
                     const float *color)
{
	for (size_t i = 0; i < 4; ++i)
		ctx->blend_color[i] = color[i];
	return 0;
}

static gcc_jit_function *
jit_blend_fn_rgb(struct jkg_ctx *ctx,
                 gcc_jit_context *jit_ctx,
                 enum jkg_blend_fn func)
{
	static const char *names[] =
	{
		[JKG_BLEND_ZERO]                     = "fn_rgb_zero",
		[JKG_BLEND_ONE]                      = "fn_rgb_one",
		[JKG_BLEND_SRC_COLOR]                = "fn_rgb_src_color",
		[JKG_BLEND_ONE_MINUS_SRC_COLOR]      = "fn_rgb_one_minus_src_color",
		[JKG_BLEND_DST_COLOR]                = "fn_rgb_dst_color",
		[JKG_BLEND_ONE_MINUS_DST_COLOR]      = "fn_rgb_one_minus_dst_color",
		[JKG_BLEND_SRC_ALPHA]                = "fn_rgb_src_alpha",
		[JKG_BLEND_ONE_MINUS_SRC_ALPHA]      = "fn_rgb_one_minus_src_alpha",
		[JKG_BLEND_DST_ALPHA]                = "fn_rgb_dst_alpha",
		[JKG_BLEND_ONE_MINUS_DST_ALPHA]      = "fn_rgb_one_minus_dst_alpha",
		[JKG_BLEND_CONSTANT_COLOR]           = "fn_rgb_constant_color",
		[JKG_BLEND_ONE_MINUS_CONSTANT_COLOR] = "fn_rgb_one_minus_constant_color",
		[JKG_BLEND_CONSTANT_ALPHA]           = "fn_rgb_constant_alpha",
		[JKG_BLEND_ONE_MINUS_CONSTANT_ALPHA] = "fn_rgb_one_minus_constant_alpha",
		[JKG_BLEND_SRC_ALPHA_SATURATE]       = "fn_rgb_src_alpha_saturate",
		[JKG_BLEND_SRC1_COLOR]               = "fn_rgb_src1_color",
		[JKG_BLEND_ONE_MINUS_SRC1_COLOR]     = "fn_rgb_one_minus_src1_color",
		[JKG_BLEND_SRC1_ALPHA]               = "fn_rgb_src1_alpha",
		[JKG_BLEND_ONE_MINUS_SRC1_ALPHA]     = "fn_rgb_one_minus_src1_alpha",
	};
	gcc_jit_param *params[] =
	{
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_restrict_floatp, "res"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_const_restrict_floatp, "org"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_const_restrict_floatp, "src"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_const_restrict_floatp, "dst"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_const_restrict_floatp, "constant"),
	};
	gcc_jit_function *fn = gcc_jit_context_new_function(jit_ctx, NULL, GCC_JIT_FUNCTION_INTERNAL, ctx->jit.type_void, names[func], sizeof(params) / sizeof(*params), params, 0);
	gcc_jit_rvalue *res = gcc_jit_param_as_rvalue(params[0]);
	gcc_jit_rvalue *org = gcc_jit_param_as_rvalue(params[1]);
	gcc_jit_rvalue *src = gcc_jit_param_as_rvalue(params[2]);
	gcc_jit_rvalue *dst = gcc_jit_param_as_rvalue(params[3]);
	gcc_jit_rvalue *constant = gcc_jit_param_as_rvalue(params[4]);
	gcc_jit_block *block = gcc_jit_function_new_block(fn, "main");
	gcc_jit_lvalue *res_lvalue[3] =
	{
		gcc_jit_context_new_array_access(jit_ctx, NULL, res, ctx->jit.value_i32[0]),
		gcc_jit_context_new_array_access(jit_ctx, NULL, res, ctx->jit.value_i32[1]),
		gcc_jit_context_new_array_access(jit_ctx, NULL, res, ctx->jit.value_i32[2]),
	};
	gcc_jit_rvalue *org_rvalue[3] =
	{
		gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(jit_ctx, NULL, org, ctx->jit.value_i32[0])),
		gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(jit_ctx, NULL, org, ctx->jit.value_i32[1])),
		gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(jit_ctx, NULL, org, ctx->jit.value_i32[2])),
	};
	switch (func)
	{
		case JKG_BLEND_SRC1_COLOR: /* XXX */
		case JKG_BLEND_ONE_MINUS_SRC1_COLOR: /* XXX */
		case JKG_BLEND_SRC1_ALPHA: /* XXX */
		case JKG_BLEND_ONE_MINUS_SRC1_ALPHA: /* XXX */
		case JKG_BLEND_ZERO:
		{
			for (size_t i = 0; i < 3; ++i)
				gcc_jit_block_add_assignment(block, NULL, res_lvalue[i], gcc_jit_context_new_rvalue_from_double(jit_ctx, ctx->jit.type_float, 0));
			break;
		}
		case JKG_BLEND_ONE:
		{
			for (size_t i = 0; i < 3; ++i)
				gcc_jit_block_add_assignment(block, NULL, res_lvalue[i], org_rvalue[i]);
			break;
		}
		case JKG_BLEND_SRC_COLOR:
		{
			for (size_t i = 0; i < 3; ++i)
			{
				gcc_jit_rvalue *src_rvalue = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(jit_ctx, NULL, src, ctx->jit.value_i32[i]));
				gcc_jit_rvalue *expr = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MULT, ctx->jit.type_float, src_rvalue, org_rvalue[i]);
				gcc_jit_block_add_assignment(block, NULL, res_lvalue[i], expr);
			}
			break;
		}
		case JKG_BLEND_ONE_MINUS_SRC_COLOR:
		{
			for (size_t i = 0; i < 3; ++i)
			{
				gcc_jit_rvalue *src_rvalue = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(jit_ctx, NULL, src, ctx->jit.value_i32[i]));
				gcc_jit_rvalue *one_minus_src = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MINUS, ctx->jit.type_float, ctx->jit.value_f[1], src_rvalue);
				gcc_jit_rvalue *expr = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MULT, ctx->jit.type_float, one_minus_src, org_rvalue[i]);
				gcc_jit_block_add_assignment(block, NULL, res_lvalue[i], expr);
			}
			break;
		}
		case JKG_BLEND_DST_COLOR:
		{
			for (size_t i = 0; i < 3; ++i)
			{
				gcc_jit_rvalue *dst_rvalue = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(jit_ctx, NULL, dst, ctx->jit.value_i32[i]));
				gcc_jit_rvalue *expr = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MULT, ctx->jit.type_float, dst_rvalue, org_rvalue[i]);
				gcc_jit_block_add_assignment(block, NULL, res_lvalue[i], expr);
			}
			break;
		}
		case JKG_BLEND_ONE_MINUS_DST_COLOR:
		{
			for (size_t i = 0; i < 3; ++i)
			{
				gcc_jit_rvalue *dst_rvalue = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(jit_ctx, NULL, dst, ctx->jit.value_i32[i]));
				gcc_jit_rvalue *one_minus_dst = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MINUS, ctx->jit.type_float, ctx->jit.value_f[1], dst_rvalue);
				gcc_jit_rvalue *expr = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MULT, ctx->jit.type_float, one_minus_dst, org_rvalue[i]);
				gcc_jit_block_add_assignment(block, NULL, res_lvalue[i], expr);
			}
			break;
		}
		case JKG_BLEND_SRC_ALPHA:
		{
			gcc_jit_rvalue *alpha = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(jit_ctx, NULL, src, ctx->jit.value_i32[3]));
			for (size_t i = 0; i < 3; ++i)
			{
				gcc_jit_rvalue *expr = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MULT, ctx->jit.type_float, org_rvalue[i], alpha);
				gcc_jit_block_add_assignment(block, NULL, res_lvalue[i], expr);
			}
			break;
		}
		case JKG_BLEND_ONE_MINUS_SRC_ALPHA:
		{
			gcc_jit_rvalue *alpha = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(jit_ctx, NULL, src, ctx->jit.value_i32[3]));
			gcc_jit_rvalue *one_minus_alpha = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MINUS, ctx->jit.type_float, ctx->jit.value_f[1], alpha);
			for (size_t i = 0; i < 3; ++i)
			{
				gcc_jit_rvalue *expr = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MULT, ctx->jit.type_float, org_rvalue[i], one_minus_alpha);
				gcc_jit_block_add_assignment(block, NULL, res_lvalue[i], expr);
			}
			break;
		}
		case JKG_BLEND_DST_ALPHA:
		{
			gcc_jit_rvalue *alpha = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(jit_ctx, NULL, dst, ctx->jit.value_i32[3]));
			for (size_t i = 0; i < 3; ++i)
			{
				gcc_jit_rvalue *expr = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MULT, ctx->jit.type_float, org_rvalue[i], alpha);
				gcc_jit_block_add_assignment(block, NULL, res_lvalue[i], expr);
			}
			break;
		}
		case JKG_BLEND_ONE_MINUS_DST_ALPHA:
		{
			gcc_jit_rvalue *alpha = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(jit_ctx, NULL, dst, ctx->jit.value_i32[3]));
			gcc_jit_rvalue *one_minus_alpha = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MINUS, ctx->jit.type_float, ctx->jit.value_f[1], alpha);
			for (size_t i = 0; i < 3; ++i)
			{
				gcc_jit_rvalue *expr = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MULT, ctx->jit.type_float, org_rvalue[i], one_minus_alpha);
				gcc_jit_block_add_assignment(block, NULL, res_lvalue[i], expr);
			}
			break;
		}
		case JKG_BLEND_CONSTANT_COLOR:
		{
			for (size_t i = 0; i < 3; ++i)
			{
				gcc_jit_rvalue *constant_rvalue = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(jit_ctx, NULL, constant, ctx->jit.value_i32[i]));
				gcc_jit_rvalue *expr = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MULT, ctx->jit.type_float, constant_rvalue, org_rvalue[i]);
				gcc_jit_block_add_assignment(block, NULL, res_lvalue[i], expr);
			}
			break;
		}
		case JKG_BLEND_ONE_MINUS_CONSTANT_COLOR:
		{
			for (size_t i = 0; i < 3; ++i)
			{
				gcc_jit_rvalue *constant_rvalue = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(jit_ctx, NULL, constant, ctx->jit.value_i32[i]));
				gcc_jit_rvalue *one_minus_dst = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MINUS, ctx->jit.type_float, ctx->jit.value_f[1], constant_rvalue);
				gcc_jit_rvalue *expr = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MULT, ctx->jit.type_float, one_minus_dst, org_rvalue[i]);
				gcc_jit_block_add_assignment(block, NULL, res_lvalue[i], expr);
			}
			break;
		}
		case JKG_BLEND_CONSTANT_ALPHA:
		{
			gcc_jit_rvalue *alpha = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(jit_ctx, NULL, constant, ctx->jit.value_i32[3]));
			for (size_t i = 0; i < 3; ++i)
			{
				gcc_jit_rvalue *expr = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MULT, ctx->jit.type_float, org_rvalue[i], alpha);
				gcc_jit_block_add_assignment(block, NULL, res_lvalue[i], expr);
			}
			break;
		}
		case JKG_BLEND_ONE_MINUS_CONSTANT_ALPHA:
		{
			gcc_jit_rvalue *alpha = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(jit_ctx, NULL, constant, ctx->jit.value_i32[3]));
			gcc_jit_rvalue *one_minus_alpha = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MINUS, ctx->jit.type_float, ctx->jit.value_f[1], alpha);
			for (size_t i = 0; i < 3; ++i)
			{
				gcc_jit_rvalue *expr = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MULT, ctx->jit.type_float, org_rvalue[i], one_minus_alpha);
				gcc_jit_block_add_assignment(block, NULL, res_lvalue[i], expr);
			}
			break;
		}
		case JKG_BLEND_SRC_ALPHA_SATURATE:
		{
			gcc_jit_rvalue *src_alpha = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(jit_ctx, NULL, src, ctx->jit.value_i32[3]));
			gcc_jit_rvalue *dst_alpha = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(jit_ctx, NULL, dst, ctx->jit.value_i32[3]));
			gcc_jit_rvalue *one_minus_dst_alpha = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MINUS, ctx->jit.type_float, ctx->jit.value_f[1], dst_alpha);
			gcc_jit_lvalue *alpha_saturate = gcc_jit_function_new_local(fn, NULL, ctx->jit.type_float, "alpha_saturate");
			gcc_jit_block *next_block = gcc_jit_function_new_block(fn, "next_block");
			gcc_jit_block *true_block = gcc_jit_function_new_block(fn, "true_block");
			gcc_jit_block_add_assignment(true_block, NULL, alpha_saturate, src_alpha);
			gcc_jit_block_end_with_jump(true_block, NULL, next_block);
			gcc_jit_block *false_block = gcc_jit_function_new_block(fn, "false_block");
			gcc_jit_block_add_assignment(false_block, NULL, alpha_saturate, one_minus_dst_alpha);
			gcc_jit_block_end_with_jump(false_block, NULL, next_block);
			gcc_jit_rvalue *cmp = gcc_jit_context_new_comparison(jit_ctx, NULL, GCC_JIT_COMPARISON_LE, src_alpha, one_minus_dst_alpha);
			gcc_jit_block_end_with_conditional(block, NULL, cmp, true_block, false_block);
			block = next_block;
			for (size_t i = 0; i < 3; ++i)
			{
				gcc_jit_rvalue *expr = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MULT, ctx->jit.type_float, org_rvalue[i], gcc_jit_lvalue_as_rvalue(alpha_saturate));
				gcc_jit_block_add_assignment(block, NULL, res_lvalue[i], expr);
			}
			break;
		}
	}
	gcc_jit_block_end_with_void_return(block, NULL);
	return fn;
}

static gcc_jit_function *
jit_blend_fn_alpha(struct jkg_ctx *ctx,
                   gcc_jit_context *jit_ctx,
                   enum jkg_blend_fn func)
{
	static const char *names[] =
	{
		[JKG_BLEND_ZERO]                     = "fn_alpha_zero",
		[JKG_BLEND_ONE]                      = "fn_alpha_one",
		[JKG_BLEND_SRC_COLOR]                = "fn_alpha_src_color",
		[JKG_BLEND_ONE_MINUS_SRC_COLOR]      = "fn_alpha_one_minus_src_color",
		[JKG_BLEND_DST_COLOR]                = "fn_alpha_dst_color",
		[JKG_BLEND_ONE_MINUS_DST_COLOR]      = "fn_alpha_one_minus_dst_color",
		[JKG_BLEND_SRC_ALPHA]                = "fn_alpha_src_alpha",
		[JKG_BLEND_ONE_MINUS_SRC_ALPHA]      = "fn_alpha_one_minus_src_alpha",
		[JKG_BLEND_DST_ALPHA]                = "fn_alpha_dst_alpha",
		[JKG_BLEND_ONE_MINUS_DST_ALPHA]      = "fn_alpha_one_minus_dst_alpha",
		[JKG_BLEND_CONSTANT_COLOR]           = "fn_alpha_constant_color",
		[JKG_BLEND_ONE_MINUS_CONSTANT_COLOR] = "fn_alpha_one_minus_constant_color",
		[JKG_BLEND_CONSTANT_ALPHA]           = "fn_alpha_constant_alpha",
		[JKG_BLEND_ONE_MINUS_CONSTANT_ALPHA] = "fn_alpha_one_minus_constant_alpha",
		[JKG_BLEND_SRC_ALPHA_SATURATE]       = "fn_alpha_src_alpha_saturate",
		[JKG_BLEND_SRC1_COLOR]               = "fn_alpha_src1_color",
		[JKG_BLEND_ONE_MINUS_SRC1_COLOR]     = "fn_alpha_one_minus_src1_color",
		[JKG_BLEND_SRC1_ALPHA]               = "fn_alpha_src1_alpha",
		[JKG_BLEND_ONE_MINUS_SRC1_ALPHA]     = "fn_alpha_one_minus_src1_alpha",
	};
	gcc_jit_param *params[] =
	{
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_restrict_floatp, "res"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_const_restrict_floatp, "org"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_const_restrict_floatp, "src"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_const_restrict_floatp, "dst"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_const_restrict_floatp, "constant"),
	};
	gcc_jit_function *fn = gcc_jit_context_new_function(jit_ctx, NULL, GCC_JIT_FUNCTION_INTERNAL, ctx->jit.type_void, names[func], sizeof(params) / sizeof(*params), params, 0);
	gcc_jit_rvalue *res = gcc_jit_param_as_rvalue(params[0]);
	gcc_jit_rvalue *org = gcc_jit_param_as_rvalue(params[1]);
	gcc_jit_rvalue *src = gcc_jit_param_as_rvalue(params[2]);
	gcc_jit_rvalue *dst = gcc_jit_param_as_rvalue(params[3]);
	gcc_jit_rvalue *constant = gcc_jit_param_as_rvalue(params[4]);
	gcc_jit_block *block = gcc_jit_function_new_block(fn, "main");
	gcc_jit_lvalue *res_lvalue = gcc_jit_context_new_array_access(jit_ctx, NULL, res, ctx->jit.value_i32[3]);
	gcc_jit_rvalue *org_rvalue = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(jit_ctx, NULL, org, ctx->jit.value_i32[3]));
	gcc_jit_rvalue *src_alpha = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(jit_ctx, NULL, src, ctx->jit.value_i32[3]));
	gcc_jit_rvalue *dst_alpha = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(jit_ctx, NULL, dst, ctx->jit.value_i32[3]));
	gcc_jit_rvalue *constant_alpha = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(jit_ctx, NULL, constant, ctx->jit.value_i32[3]));
	switch (func)
	{
		case JKG_BLEND_SRC1_COLOR: /* XXX */
		case JKG_BLEND_ONE_MINUS_SRC1_COLOR: /* XXX */
		case JKG_BLEND_SRC1_ALPHA: /* XXX */
		case JKG_BLEND_ONE_MINUS_SRC1_ALPHA: /* XXX */
		case JKG_BLEND_ZERO:
		{
			gcc_jit_block_add_assignment(block, NULL, res_lvalue, gcc_jit_context_new_rvalue_from_double(jit_ctx, ctx->jit.type_float, 0));
			break;
		}
		case JKG_BLEND_ONE:
		{
			gcc_jit_block_add_assignment(block, NULL, res_lvalue, org_rvalue);
			break;
		}
		case JKG_BLEND_SRC_COLOR:
		case JKG_BLEND_SRC_ALPHA:
		{
			gcc_jit_rvalue *expr = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MULT, ctx->jit.type_float, org_rvalue, src_alpha);
			gcc_jit_block_add_assignment(block, NULL, res_lvalue, expr);
			break;
		}
		case JKG_BLEND_ONE_MINUS_SRC_COLOR:
		case JKG_BLEND_ONE_MINUS_SRC_ALPHA:
		{
			gcc_jit_rvalue *one_minus_alpha = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MINUS, ctx->jit.type_float, ctx->jit.value_f[1], src_alpha);
			gcc_jit_rvalue *expr = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MULT, ctx->jit.type_float, org_rvalue, one_minus_alpha);
			gcc_jit_block_add_assignment(block, NULL, res_lvalue, expr);
			break;
		}
		case JKG_BLEND_DST_COLOR:
		case JKG_BLEND_DST_ALPHA:
		{
			gcc_jit_rvalue *expr = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MULT, ctx->jit.type_float, org_rvalue, dst_alpha);
			gcc_jit_block_add_assignment(block, NULL, res_lvalue, expr);
			break;
		}
		case JKG_BLEND_ONE_MINUS_DST_COLOR:
		case JKG_BLEND_ONE_MINUS_DST_ALPHA:
		{
			gcc_jit_rvalue *one_minus_alpha = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MINUS, ctx->jit.type_float, ctx->jit.value_f[1], dst_alpha);
			gcc_jit_rvalue *expr = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MULT, ctx->jit.type_float, org_rvalue, one_minus_alpha);
			gcc_jit_block_add_assignment(block, NULL, res_lvalue, expr);
			break;
		}
		case JKG_BLEND_CONSTANT_COLOR:
		case JKG_BLEND_CONSTANT_ALPHA:
		{
			gcc_jit_rvalue *expr = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MULT, ctx->jit.type_float, org_rvalue, constant_alpha);
			gcc_jit_block_add_assignment(block, NULL, res_lvalue, expr);
			break;
		}
		case JKG_BLEND_ONE_MINUS_CONSTANT_COLOR:
		case JKG_BLEND_ONE_MINUS_CONSTANT_ALPHA:
		{
			gcc_jit_rvalue *one_minus_alpha = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MINUS, ctx->jit.type_float, ctx->jit.value_f[1], constant_alpha);
			gcc_jit_rvalue *expr = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MULT, ctx->jit.type_float, org_rvalue, one_minus_alpha);
			gcc_jit_block_add_assignment(block, NULL, res_lvalue, expr);
			break;
		}
		case JKG_BLEND_SRC_ALPHA_SATURATE:
		{
			gcc_jit_rvalue *one_minus_dst_alpha = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MINUS, ctx->jit.type_float, ctx->jit.value_f[1], dst_alpha);
			gcc_jit_lvalue *alpha_saturate = gcc_jit_function_new_local(fn, NULL, ctx->jit.type_float, "alpha_saturate");
			gcc_jit_block *next_block = gcc_jit_function_new_block(fn, "next_block");
			gcc_jit_block *true_block = gcc_jit_function_new_block(fn, "true_block");
			gcc_jit_block_add_assignment(true_block, NULL, alpha_saturate, src_alpha);
			gcc_jit_block_end_with_jump(true_block, NULL, next_block);
			gcc_jit_block *false_block = gcc_jit_function_new_block(fn, "false_block");
			gcc_jit_block_add_assignment(false_block, NULL, alpha_saturate, one_minus_dst_alpha);
			gcc_jit_block_end_with_jump(false_block, NULL, next_block);
			gcc_jit_rvalue *cmp = gcc_jit_context_new_comparison(jit_ctx, NULL, GCC_JIT_COMPARISON_LE, src_alpha, one_minus_dst_alpha);
			gcc_jit_block_end_with_conditional(block, NULL, cmp, true_block, false_block);
			block = next_block;
			gcc_jit_rvalue *expr = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MULT, ctx->jit.type_float, org_rvalue, gcc_jit_lvalue_as_rvalue(alpha_saturate));
			gcc_jit_block_add_assignment(block, NULL, res_lvalue, expr);
			break;
		}
	}
	gcc_jit_block_end_with_void_return(block, NULL);
	return fn;
}

static gcc_jit_function *
jit_blend_eq_rgb(struct jkg_ctx *ctx,
                 gcc_jit_context *jit_ctx,
                 enum jkg_blend_eq eq)
{
	static const char *names[] =
	{
		[JKG_EQUATION_ADD]          = "eq_rgb_add",
		[JKG_EQUATION_SUBTRACT]     = "eq_rgb_subtract",
		[JKG_EQUATION_REV_SUBTRACT] = "eq_rgb_rev_subtract",
		[JKG_EQUATION_MIN]          = "eq_rgb_min",
		[JKG_EQUATION_MAX]          = "eq_rgb_max",
	};
	gcc_jit_param *params[] =
	{
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_restrict_floatp, "res"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_const_restrict_floatp, "src"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_const_restrict_floatp, "org"),
	};
	gcc_jit_function *fn = gcc_jit_context_new_function(jit_ctx, NULL, GCC_JIT_FUNCTION_INTERNAL, ctx->jit.type_void, names[eq], sizeof(params) / sizeof(*params), params, 0);
	gcc_jit_rvalue *res = gcc_jit_param_as_rvalue(params[0]);
	gcc_jit_rvalue *src = gcc_jit_param_as_rvalue(params[1]);
	gcc_jit_rvalue *dst = gcc_jit_param_as_rvalue(params[2]);
	gcc_jit_block *block = gcc_jit_function_new_block(fn, "main");
	gcc_jit_lvalue *res_lvalue[3] =
	{
		gcc_jit_context_new_array_access(jit_ctx, NULL, res, ctx->jit.value_i32[0]),
		gcc_jit_context_new_array_access(jit_ctx, NULL, res, ctx->jit.value_i32[1]),
		gcc_jit_context_new_array_access(jit_ctx, NULL, res, ctx->jit.value_i32[2]),
	};
	gcc_jit_rvalue *src_rvalue[3] =
	{
		gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(jit_ctx, NULL, src, ctx->jit.value_i32[0])),
		gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(jit_ctx, NULL, src, ctx->jit.value_i32[1])),
		gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(jit_ctx, NULL, src, ctx->jit.value_i32[2])),
	};
	gcc_jit_rvalue *dst_rvalue[3] =
	{
		gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(jit_ctx, NULL, dst, ctx->jit.value_i32[0])),
		gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(jit_ctx, NULL, dst, ctx->jit.value_i32[1])),
		gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(jit_ctx, NULL, dst, ctx->jit.value_i32[2])),
	};
	switch (eq)
	{
		case JKG_EQUATION_ADD:
		{
			for (size_t i = 0; i < 3; ++i)
			{
				gcc_jit_rvalue *expr = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_PLUS, ctx->jit.type_float, src_rvalue[i], dst_rvalue[i]);
				gcc_jit_block_add_assignment(block, NULL, res_lvalue[i], expr);
			}
			break;
		}
		case JKG_EQUATION_SUBTRACT:
		{
			for (size_t i = 0; i < 3; ++i)
			{
				gcc_jit_rvalue *expr = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MINUS, ctx->jit.type_float, src_rvalue[i], dst_rvalue[i]);
				gcc_jit_block_add_assignment(block, NULL, res_lvalue[i], expr);
			}
			break;
		}
		case JKG_EQUATION_REV_SUBTRACT:
		{
			for (size_t i = 0; i < 3; ++i)
			{
				gcc_jit_rvalue *expr = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MINUS, ctx->jit.type_float, dst_rvalue[i], src_rvalue[i]);
				gcc_jit_block_add_assignment(block, NULL, res_lvalue[i], expr);
			}
			break;
		}
		case JKG_EQUATION_MIN:
		{
			gcc_jit_block *next_block;
			for (size_t i = 0; i < 3; ++i)
			{
				if (i != 2)
					next_block = gcc_jit_function_new_block(fn, "comp_block");
				gcc_jit_block *true_block = gcc_jit_function_new_block(fn, "true_block");
				gcc_jit_block_add_assignment(true_block, NULL, res_lvalue[i], src_rvalue[i]);
				if (i != 2)
					gcc_jit_block_end_with_jump(true_block, NULL, next_block);
				else
					gcc_jit_block_end_with_void_return(true_block, NULL);
				gcc_jit_block *false_block = gcc_jit_function_new_block(fn, "false_block");
				gcc_jit_block_add_assignment(false_block, NULL, res_lvalue[i], dst_rvalue[i]);
				if (i != 2)
					gcc_jit_block_end_with_jump(false_block, NULL, next_block);
				else
					gcc_jit_block_end_with_void_return(false_block, NULL);
				gcc_jit_rvalue *cmp = gcc_jit_context_new_comparison(jit_ctx, NULL, GCC_JIT_COMPARISON_LE, src_rvalue[i], dst_rvalue[i]);
				gcc_jit_block_end_with_conditional(block, NULL, cmp, true_block, false_block);
				block = next_block;
			}
			return fn;
		}
		case JKG_EQUATION_MAX:
		{
			gcc_jit_block *next_block;
			for (size_t i = 0; i < 3; ++i)
			{
				if (i != 2)
					next_block = gcc_jit_function_new_block(fn, "comp_block");
				gcc_jit_block *true_block = gcc_jit_function_new_block(fn, "true_block");
				gcc_jit_block_add_assignment(true_block, NULL, res_lvalue[i], src_rvalue[i]);
				if (i != 2)
					gcc_jit_block_end_with_jump(true_block, NULL, next_block);
				else
					gcc_jit_block_end_with_void_return(true_block, NULL);
				gcc_jit_block *false_block = gcc_jit_function_new_block(fn, "false_block");
				gcc_jit_block_add_assignment(false_block, NULL, res_lvalue[i], dst_rvalue[i]);
				if (i != 2)
					gcc_jit_block_end_with_jump(false_block, NULL, next_block);
				else
					gcc_jit_block_end_with_void_return(false_block, NULL);
				gcc_jit_rvalue *cmp = gcc_jit_context_new_comparison(jit_ctx, NULL, GCC_JIT_COMPARISON_GE, src_rvalue[i], dst_rvalue[i]);
				gcc_jit_block_end_with_conditional(block, NULL, cmp, true_block, false_block);
				block = next_block;
			}
			return fn;
		}
		default:
			assert(!"unknown blend equation rgb");
			break;
	}
	gcc_jit_block_end_with_void_return(block, NULL);
	return fn;
}

static gcc_jit_function *
jit_blend_eq_alpha(struct jkg_ctx *ctx,
                   gcc_jit_context *jit_ctx,
                   enum jkg_blend_eq eq)
{
	static const char *names[] =
	{
		[JKG_EQUATION_ADD]          = "eq_alpha_add",
		[JKG_EQUATION_SUBTRACT]     = "eq_alpha_subtract",
		[JKG_EQUATION_REV_SUBTRACT] = "eq_alpha_rev_subtract",
		[JKG_EQUATION_MIN]          = "eq_alpha_min",
		[JKG_EQUATION_MAX]          = "eq_alpha_max",
	};
	gcc_jit_param *params[] =
	{
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_restrict_floatp, "res"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_const_restrict_floatp, "src"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_const_restrict_floatp, "dst"),
	};
	gcc_jit_function *fn = gcc_jit_context_new_function(jit_ctx, NULL, GCC_JIT_FUNCTION_INTERNAL, ctx->jit.type_void, names[eq], sizeof(params) / sizeof(*params), params, 0);
	gcc_jit_rvalue *res = gcc_jit_param_as_rvalue(params[0]);
	gcc_jit_rvalue *src = gcc_jit_param_as_rvalue(params[1]);
	gcc_jit_rvalue *dst = gcc_jit_param_as_rvalue(params[2]);
	gcc_jit_block *block = gcc_jit_function_new_block(fn, "main");
	gcc_jit_lvalue *res_lvalue = gcc_jit_context_new_array_access(jit_ctx, NULL, res, ctx->jit.value_i32[3]);
	gcc_jit_rvalue *src_rvalue = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(jit_ctx, NULL, src, ctx->jit.value_i32[3]));
	gcc_jit_rvalue *dst_rvalue = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(jit_ctx, NULL, dst, ctx->jit.value_i32[3]));
	switch (eq)
	{
		case JKG_EQUATION_ADD:
		{
			gcc_jit_rvalue *expr = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_PLUS, ctx->jit.type_float, src_rvalue, dst_rvalue);
			gcc_jit_block_add_assignment(block, NULL, res_lvalue, expr);
			break;
		}
		case JKG_EQUATION_SUBTRACT:
		{
			gcc_jit_rvalue *expr = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MINUS, ctx->jit.type_float, src_rvalue, dst_rvalue);
			gcc_jit_block_add_assignment(block, NULL, res_lvalue, expr);
			break;
		}
		case JKG_EQUATION_REV_SUBTRACT:
		{
			gcc_jit_rvalue *expr = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MINUS, ctx->jit.type_float, dst_rvalue, src_rvalue);
			gcc_jit_block_add_assignment(block, NULL, res_lvalue, expr);
			break;
		}
		case JKG_EQUATION_MIN:
		{
			gcc_jit_block *true_block = gcc_jit_function_new_block(fn, "true_block");
			gcc_jit_block_add_assignment(true_block, NULL, res_lvalue, src_rvalue);
			gcc_jit_block_end_with_void_return(true_block, NULL);
			gcc_jit_block *false_block = gcc_jit_function_new_block(fn, "false_block");
			gcc_jit_block_add_assignment(false_block, NULL, res_lvalue, dst_rvalue);
			gcc_jit_block_end_with_void_return(false_block, NULL);
			gcc_jit_rvalue *cmp = gcc_jit_context_new_comparison(jit_ctx, NULL, GCC_JIT_COMPARISON_LE, src_rvalue, dst_rvalue);
			gcc_jit_block_end_with_conditional(block, NULL, cmp, true_block, false_block);
			return fn;
		}
		case JKG_EQUATION_MAX:
		{
			gcc_jit_block *true_block = gcc_jit_function_new_block(fn, "true_block");
			gcc_jit_block_add_assignment(true_block, NULL, res_lvalue, src_rvalue);
			gcc_jit_block_end_with_void_return(true_block, NULL);
			gcc_jit_block *false_block = gcc_jit_function_new_block(fn, "false_block");
			gcc_jit_block_add_assignment(false_block, NULL, res_lvalue, dst_rvalue);
			gcc_jit_block_end_with_void_return(false_block, NULL);
			gcc_jit_rvalue *cmp = gcc_jit_context_new_comparison(jit_ctx, NULL, GCC_JIT_COMPARISON_GE, src_rvalue, dst_rvalue);
			gcc_jit_block_end_with_conditional(block, NULL, cmp, true_block, false_block);
			return fn;
		}
		default:
			assert(!"unknown blend equation rgb");
			break;
	}
	gcc_jit_block_end_with_void_return(block, NULL);
	return fn;
}

void
gjit_blend_init(struct jkg_ctx *ctx)
{
	ctx->jit.blend.ctx = gcc_jit_context_new_child_context(ctx->jit.ctx);
	for (size_t i = 0; i < sizeof(ctx->jit.blend.fn_rgb) / sizeof(*ctx->jit.blend.fn_rgb); ++i)
		ctx->jit.blend.fn_rgb[i] = jit_blend_fn_rgb(ctx, ctx->jit.blend.ctx, i);
	for (size_t i = 0; i < sizeof(ctx->jit.blend.fn_alpha) / sizeof(*ctx->jit.blend.fn_alpha); ++i)
		ctx->jit.blend.fn_alpha[i] = jit_blend_fn_alpha(ctx, ctx->jit.blend.ctx, i);
	for (size_t i = 0; i < sizeof(ctx->jit.blend.eq_rgb) / sizeof(*ctx->jit.blend.eq_rgb); ++i)
		ctx->jit.blend.eq_rgb[i] = jit_blend_eq_rgb(ctx, ctx->jit.blend.ctx, i);
	for (size_t i = 0; i < sizeof(ctx->jit.blend.eq_alpha) / sizeof(*ctx->jit.blend.eq_alpha); ++i)
		ctx->jit.blend.eq_alpha[i] = jit_blend_eq_alpha(ctx, ctx->jit.blend.ctx, i);
}

static gcc_jit_function *
jit_color_mask(struct jkg_ctx *ctx,
               gcc_jit_context *jit_ctx,
               const struct jkg_blend_attachment *attachment)
{
	gcc_jit_param *params[] =
	{
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_restrict_floatp, "dst"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_const_restrict_floatp, "src"),
	};
	gcc_jit_function *fn = gcc_jit_context_new_function(jit_ctx, NULL, GCC_JIT_FUNCTION_INTERNAL, ctx->jit.type_void, "color_mask", sizeof(params) / sizeof(*params), params, 0);
	gcc_jit_rvalue *dst = gcc_jit_param_as_rvalue(params[0]);
	gcc_jit_rvalue *src = gcc_jit_param_as_rvalue(params[1]);
	gcc_jit_block *block = gcc_jit_function_new_block(fn, "main");
	for (size_t i = 0; i < 4; ++i)
	{
		if (!(attachment->color_mask & (1 << i)))
			continue;
		gcc_jit_lvalue *dst_lvalue = gcc_jit_context_new_array_access(jit_ctx, NULL, dst, ctx->jit.value_i32[i]);
		gcc_jit_rvalue *src_rvalue = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(jit_ctx, NULL, src, ctx->jit.value_i32[i]));
		gcc_jit_block_add_assignment(block, NULL, dst_lvalue, src_rvalue);
	}
	gcc_jit_block_end_with_void_return(block, NULL);
	return fn;
}

static gcc_jit_function *
jit_blend_disabled(struct jkg_ctx *ctx,
                   gcc_jit_context *jit_ctx,
                   const struct jkg_blend_attachment *attachment)
{
	gcc_jit_function *color_mask_fn = jit_color_mask(ctx, jit_ctx, attachment);
	gcc_jit_param *params[] =
	{
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_restrict_floatp, "res"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_const_restrict_floatp, "src"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_const_restrict_floatp, "dst"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_const_restrict_floatp, "constant"),
	};
	gcc_jit_function *fn = gcc_jit_context_new_function(jit_ctx, NULL, GCC_JIT_FUNCTION_EXPORTED, ctx->jit.type_void, "blend", sizeof(params) / sizeof(*params), params, 0);
	gcc_jit_rvalue *res = gcc_jit_param_as_rvalue(params[0]);
	gcc_jit_rvalue *src = gcc_jit_param_as_rvalue(params[1]);
	gcc_jit_block *block = gcc_jit_function_new_block(fn, "main");
	gcc_jit_rvalue *color_mask_params[] =
	{
		res,
		src,
	};
	gcc_jit_block_add_eval(block, NULL, gcc_jit_context_new_call(jit_ctx, NULL, color_mask_fn, sizeof(color_mask_params) / sizeof(*color_mask_params), color_mask_params));
	gcc_jit_block_end_with_void_return(block, NULL);
	return fn;
}

static gcc_jit_function *
jit_blend_enabled(struct jkg_ctx *ctx,
                  gcc_jit_context *jit_ctx,
                  const struct jkg_blend_attachment *attachment)
{
	gcc_jit_function *blend_src_rgb_fn = ctx->jit.blend.fn_rgb[attachment->rgb_src];
	gcc_jit_function *blend_dst_rgb_fn = ctx->jit.blend.fn_rgb[attachment->rgb_dst];
	gcc_jit_function *blend_src_alpha_fn = ctx->jit.blend.fn_alpha[attachment->alpha_src];
	gcc_jit_function *blend_dst_alpha_fn = ctx->jit.blend.fn_alpha[attachment->alpha_dst];
	gcc_jit_function *blend_eq_rgb_fn = ctx->jit.blend.eq_rgb[attachment->rgb_eq];
	gcc_jit_function *blend_eq_alpha_fn = ctx->jit.blend.eq_alpha[attachment->alpha_eq];
	gcc_jit_function *color_mask_fn = jit_color_mask(ctx, jit_ctx, attachment);
	gcc_jit_param *params[] =
	{
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_restrict_floatp, "res"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_const_restrict_floatp, "src"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_const_restrict_floatp, "dst"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_const_restrict_floatp, "constant"),
	};
	gcc_jit_function *fn = gcc_jit_context_new_function(jit_ctx, NULL, GCC_JIT_FUNCTION_EXPORTED, ctx->jit.type_void, "blend", sizeof(params) / sizeof(*params), params, 0);
	gcc_jit_rvalue *res = gcc_jit_param_as_rvalue(params[0]);
	gcc_jit_rvalue *src = gcc_jit_param_as_rvalue(params[1]);
	gcc_jit_rvalue *dst = gcc_jit_param_as_rvalue(params[2]);
	gcc_jit_rvalue *constant = gcc_jit_param_as_rvalue(params[3]);
	gcc_jit_lvalue *tmp_src_array = gcc_jit_function_new_local(fn, NULL, ctx->jit.type_float_4, "tmp_src");
	gcc_jit_rvalue *tmp_src = gcc_jit_lvalue_get_address(gcc_jit_context_new_array_access(jit_ctx, NULL, gcc_jit_lvalue_as_rvalue(tmp_src_array), gcc_jit_context_new_rvalue_from_int(jit_ctx, ctx->jit.type_int, 0)), NULL);
	gcc_jit_lvalue *tmp_dst_array = gcc_jit_function_new_local(fn, NULL, ctx->jit.type_float_4, "tmp_dst");
	gcc_jit_rvalue *tmp_dst = gcc_jit_lvalue_get_address(gcc_jit_context_new_array_access(jit_ctx, NULL, gcc_jit_lvalue_as_rvalue(tmp_dst_array), gcc_jit_context_new_rvalue_from_int(jit_ctx, ctx->jit.type_int, 0)), NULL);
	gcc_jit_lvalue *tmp_res_array = gcc_jit_function_new_local(fn, NULL, ctx->jit.type_float_4, "tmp_res");
	gcc_jit_rvalue *tmp_res = gcc_jit_lvalue_get_address(gcc_jit_context_new_array_access(jit_ctx, NULL, gcc_jit_lvalue_as_rvalue(tmp_res_array), gcc_jit_context_new_rvalue_from_int(jit_ctx, ctx->jit.type_int, 0)), NULL);
	gcc_jit_block *block = gcc_jit_function_new_block(fn, "main");
	gcc_jit_rvalue *blend_src_rgb_args[] = {tmp_src, src, src, dst, constant};
	gcc_jit_block_add_eval(block, NULL, gcc_jit_context_new_call(jit_ctx, NULL, blend_src_rgb_fn, sizeof(blend_src_rgb_args) / sizeof(*blend_src_rgb_args), blend_src_rgb_args));
	gcc_jit_rvalue *blend_dst_rgb_args[] = {tmp_dst, dst, src, dst, constant};
	gcc_jit_block_add_eval(block, NULL, gcc_jit_context_new_call(jit_ctx, NULL, blend_dst_rgb_fn, sizeof(blend_dst_rgb_args) / sizeof(*blend_dst_rgb_args), blend_dst_rgb_args));
	gcc_jit_rvalue *blend_src_alpha_args[] = {tmp_src, src, src, dst, constant};
	gcc_jit_block_add_eval(block, NULL, gcc_jit_context_new_call(jit_ctx, NULL, blend_src_alpha_fn, sizeof(blend_src_alpha_args) / sizeof(*blend_src_alpha_args), blend_src_alpha_args));
	gcc_jit_rvalue *blend_dst_alpha_args[] = {tmp_dst, dst, src, dst, constant};
	gcc_jit_block_add_eval(block, NULL, gcc_jit_context_new_call(jit_ctx, NULL, blend_dst_alpha_fn, sizeof(blend_dst_alpha_args) / sizeof(*blend_dst_alpha_args), blend_dst_alpha_args));
	gcc_jit_rvalue *blend_eq_rgb_args[] = {tmp_res, tmp_src, tmp_dst};
	gcc_jit_block_add_eval(block, NULL, gcc_jit_context_new_call(jit_ctx, NULL, blend_eq_rgb_fn, sizeof(blend_eq_rgb_args) / sizeof(*blend_eq_rgb_args), blend_eq_rgb_args));
	gcc_jit_rvalue *blend_eq_alpha_args[] = {tmp_res, tmp_src, tmp_dst};
	gcc_jit_block_add_eval(block, NULL, gcc_jit_context_new_call(jit_ctx, NULL, blend_eq_alpha_fn, sizeof(blend_eq_alpha_args) / sizeof(*blend_eq_alpha_args), blend_eq_alpha_args));
	gcc_jit_rvalue *color_mask_args[] = {res, tmp_res};
	gcc_jit_block_add_eval(block, NULL, gcc_jit_context_new_call(jit_ctx, NULL, color_mask_fn, sizeof(color_mask_args) / sizeof(*color_mask_args), color_mask_args));
	gcc_jit_block_end_with_void_return(block, NULL);
	return fn;
}

static gcc_jit_function *
jit_blend(struct jkg_ctx *ctx,
          gcc_jit_context *jit_ctx,
          const struct jkg_blend_state_create_info *create_info)
{
	if (create_info->attachments[0].enable)
		return jit_blend_enabled(ctx, jit_ctx, &create_info->attachments[0]);
	return jit_blend_disabled(ctx, jit_ctx, &create_info->attachments[0]);
}
