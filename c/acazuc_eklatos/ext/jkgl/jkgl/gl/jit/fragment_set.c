#include "internal.h"
#include "fnv.h"

#include <assert.h>
#include <stdlib.h>

static gcc_jit_function *jit_blend_rgb(struct gl_ctx *gl_ctx, gcc_jit_context *ctx, const char *name, GLenum mode)
{
	struct jit_ctx *jit_ctx = &gl_ctx->jit;
	gcc_jit_param *params[] =
	{
		gcc_jit_context_new_param(ctx, NULL, jit_ctx->type_floatp, "res"),
		gcc_jit_context_new_param(ctx, NULL, jit_ctx->type_floatp, "org"),
		gcc_jit_context_new_param(ctx, NULL, jit_ctx->type_floatp, "src"),
		gcc_jit_context_new_param(ctx, NULL, jit_ctx->type_floatp, "dst"),
		gcc_jit_context_new_param(ctx, NULL, jit_ctx->type_floatp, "constant"),
	};
	gcc_jit_function *fn = gcc_jit_context_new_function(ctx, NULL, GCC_JIT_FUNCTION_INTERNAL, jit_ctx->type_void, name, sizeof(params) / sizeof(*params), params, 0);
	gcc_jit_rvalue *res = gcc_jit_param_as_rvalue(params[0]);
	gcc_jit_rvalue *org = gcc_jit_param_as_rvalue(params[1]);
	gcc_jit_rvalue *src = gcc_jit_param_as_rvalue(params[2]);
	gcc_jit_rvalue *dst = gcc_jit_param_as_rvalue(params[3]);
	gcc_jit_rvalue *constant = gcc_jit_param_as_rvalue(params[4]);
	gcc_jit_block *block = gcc_jit_function_new_block(fn, NULL);
	switch (mode)
	{
		case GL_ZERO:
		{
			for (GLint i = 0; i < 3; ++i)
			{
				gcc_jit_rvalue *idx = gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_int, i);
				gcc_jit_lvalue *res_lvalue = gcc_jit_context_new_array_access(ctx, NULL, res, idx);
				gcc_jit_block_add_assignment(block, NULL, res_lvalue, gcc_jit_context_new_rvalue_from_double(ctx, jit_ctx->type_float, 0));
			}
			break;
		}
		case GL_ONE:
		{
			for (GLint i = 0; i < 3; ++i)
			{
				gcc_jit_rvalue *idx = gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_int, i);
				gcc_jit_lvalue *res_lvalue = gcc_jit_context_new_array_access(ctx, NULL, res, idx);
				gcc_jit_rvalue *org_rvalue = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, org, idx));
				gcc_jit_block_add_assignment(block, NULL, res_lvalue, org_rvalue);
			}
			break;
		}
		case GL_SRC_COLOR:
		{
			for (GLint i = 0; i < 3; ++i)
			{
				gcc_jit_rvalue *idx = gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_int, i);
				gcc_jit_lvalue *res_lvalue = gcc_jit_context_new_array_access(ctx, NULL, res, idx);
				gcc_jit_rvalue *src_rvalue = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, src, idx));
				gcc_jit_rvalue *org_rvalue = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, org, idx));
				gcc_jit_rvalue *expr = gcc_jit_context_new_binary_op(ctx, NULL, GCC_JIT_BINARY_OP_MULT, jit_ctx->type_float, src_rvalue, org_rvalue);
				gcc_jit_block_add_assignment(block, NULL, res_lvalue, expr);
			}
			break;
		}
		case GL_ONE_MINUS_SRC_COLOR:
		{
			for (GLint i = 0; i < 3; ++i)
			{
				gcc_jit_rvalue *idx = gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_int, i);
				gcc_jit_lvalue *res_lvalue = gcc_jit_context_new_array_access(ctx, NULL, res, idx);
				gcc_jit_rvalue *src_rvalue = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, src, idx));
				gcc_jit_rvalue *org_rvalue = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, org, idx));
				gcc_jit_rvalue *one_minus_src = gcc_jit_context_new_binary_op(ctx, NULL, GCC_JIT_BINARY_OP_MINUS, jit_ctx->type_float, gcc_jit_context_new_rvalue_from_double(ctx, jit_ctx->type_float, 1), src_rvalue);
				gcc_jit_rvalue *expr = gcc_jit_context_new_binary_op(ctx, NULL, GCC_JIT_BINARY_OP_MULT, jit_ctx->type_float, one_minus_src, org_rvalue);
				gcc_jit_block_add_assignment(block, NULL, res_lvalue, expr);
			}
			break;
		}
		case GL_DST_COLOR:
		{
			for (GLint i = 0; i < 3; ++i)
			{
				gcc_jit_rvalue *idx = gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_int, i);
				gcc_jit_lvalue *res_lvalue = gcc_jit_context_new_array_access(ctx, NULL, res, idx);
				gcc_jit_rvalue *dst_rvalue = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, dst, idx));
				gcc_jit_rvalue *org_rvalue = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, org, idx));
				gcc_jit_rvalue *expr = gcc_jit_context_new_binary_op(ctx, NULL, GCC_JIT_BINARY_OP_MULT, jit_ctx->type_float, dst_rvalue, org_rvalue);
				gcc_jit_block_add_assignment(block, NULL, res_lvalue, expr);
			}
			break;
		}
		case GL_ONE_MINUS_DST_COLOR:
		{
			for (GLint i = 0; i < 3; ++i)
			{
				gcc_jit_rvalue *idx = gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_int, i);
				gcc_jit_lvalue *res_lvalue = gcc_jit_context_new_array_access(ctx, NULL, res, idx);
				gcc_jit_rvalue *dst_rvalue = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, dst, idx));
				gcc_jit_rvalue *org_rvalue = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, org, idx));
				gcc_jit_rvalue *one_minus_dst = gcc_jit_context_new_binary_op(ctx, NULL, GCC_JIT_BINARY_OP_MINUS, jit_ctx->type_float, gcc_jit_context_new_rvalue_from_double(ctx, jit_ctx->type_float, 1), dst_rvalue);
				gcc_jit_rvalue *expr = gcc_jit_context_new_binary_op(ctx, NULL, GCC_JIT_BINARY_OP_MULT, jit_ctx->type_float, one_minus_dst, org_rvalue);
				gcc_jit_block_add_assignment(block, NULL, res_lvalue, expr);
			}
			break;
		}
		case GL_SRC_ALPHA:
		{
			gcc_jit_rvalue *alpha = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, src, gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_int, 3)));
			for (GLint i = 0; i < 3; ++i)
			{
				gcc_jit_rvalue *idx = gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_int, i);
				gcc_jit_lvalue *res_lvalue = gcc_jit_context_new_array_access(ctx, NULL, res, idx);
				gcc_jit_rvalue *org_rvalue = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, org, idx));
				gcc_jit_rvalue *expr = gcc_jit_context_new_binary_op(ctx, NULL, GCC_JIT_BINARY_OP_MULT, jit_ctx->type_float, org_rvalue, alpha);
				gcc_jit_block_add_assignment(block, NULL, res_lvalue, expr);
			}
			break;
		}
		case GL_ONE_MINUS_SRC_ALPHA:
		{
			gcc_jit_rvalue *alpha = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, src, gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_int, 3)));
			gcc_jit_rvalue *one_minus_alpha = gcc_jit_context_new_binary_op(ctx, NULL, GCC_JIT_BINARY_OP_MINUS, jit_ctx->type_float, gcc_jit_context_new_rvalue_from_double(ctx, jit_ctx->type_float, 1), alpha);
			for (GLint i = 0; i < 3; ++i)
			{
				gcc_jit_rvalue *idx = gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_int, i);
				gcc_jit_lvalue *res_lvalue = gcc_jit_context_new_array_access(ctx, NULL, res, idx);
				gcc_jit_rvalue *org_rvalue = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, org, idx));
				gcc_jit_rvalue *expr = gcc_jit_context_new_binary_op(ctx, NULL, GCC_JIT_BINARY_OP_MULT, jit_ctx->type_float, org_rvalue, one_minus_alpha);
				gcc_jit_block_add_assignment(block, NULL, res_lvalue, expr);
			}
			break;
		}
		case GL_DST_ALPHA:
		{
			gcc_jit_rvalue *alpha = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, dst, gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_int, 3)));
			for (GLint i = 0; i < 3; ++i)
			{
				gcc_jit_rvalue *idx = gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_int, i);
				gcc_jit_lvalue *res_lvalue = gcc_jit_context_new_array_access(ctx, NULL, res, idx);
				gcc_jit_rvalue *org_rvalue = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, org, idx));
				gcc_jit_rvalue *expr = gcc_jit_context_new_binary_op(ctx, NULL, GCC_JIT_BINARY_OP_MULT, jit_ctx->type_float, org_rvalue, alpha);
				gcc_jit_block_add_assignment(block, NULL, res_lvalue, expr);
			}
			break;
		}
		case GL_ONE_MINUS_DST_ALPHA:
		{
			gcc_jit_rvalue *alpha = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, dst, gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_int, 3)));
			gcc_jit_rvalue *one_minus_alpha = gcc_jit_context_new_binary_op(ctx, NULL, GCC_JIT_BINARY_OP_MINUS, jit_ctx->type_float, gcc_jit_context_new_rvalue_from_double(ctx, jit_ctx->type_float, 1), alpha);
			for (GLint i = 0; i < 3; ++i)
			{
				gcc_jit_rvalue *idx = gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_int, i);
				gcc_jit_lvalue *res_lvalue = gcc_jit_context_new_array_access(ctx, NULL, res, idx);
				gcc_jit_rvalue *org_rvalue = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, org, idx));
				gcc_jit_rvalue *expr = gcc_jit_context_new_binary_op(ctx, NULL, GCC_JIT_BINARY_OP_MULT, jit_ctx->type_float, org_rvalue, one_minus_alpha);
				gcc_jit_block_add_assignment(block, NULL, res_lvalue, expr);
			}
			break;
		}
		case GL_CONSTANT_COLOR:
		{
			for (GLint i = 0; i < 3; ++i)
			{
				gcc_jit_rvalue *idx = gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_int, i);
				gcc_jit_lvalue *res_lvalue = gcc_jit_context_new_array_access(ctx, NULL, res, idx);
				gcc_jit_rvalue *constant_rvalue = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, constant, idx));
				gcc_jit_rvalue *org_rvalue = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, org, idx));
				gcc_jit_rvalue *expr = gcc_jit_context_new_binary_op(ctx, NULL, GCC_JIT_BINARY_OP_MULT, jit_ctx->type_float, constant_rvalue, org_rvalue);
				gcc_jit_block_add_assignment(block, NULL, res_lvalue, expr);
			}
			break;
		}
		case GL_ONE_MINUS_CONSTANT_COLOR:
		{
			for (GLint i = 0; i < 3; ++i)
			{
				gcc_jit_rvalue *idx = gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_int, i);
				gcc_jit_lvalue *res_lvalue = gcc_jit_context_new_array_access(ctx, NULL, res, idx);
				gcc_jit_rvalue *constant_rvalue = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, constant, idx));
				gcc_jit_rvalue *org_rvalue = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, org, idx));
				gcc_jit_rvalue *one_minus_dst = gcc_jit_context_new_binary_op(ctx, NULL, GCC_JIT_BINARY_OP_MINUS, jit_ctx->type_float, gcc_jit_context_new_rvalue_from_double(ctx, jit_ctx->type_float, 1), constant_rvalue);
				gcc_jit_rvalue *expr = gcc_jit_context_new_binary_op(ctx, NULL, GCC_JIT_BINARY_OP_MULT, jit_ctx->type_float, one_minus_dst, org_rvalue);
				gcc_jit_block_add_assignment(block, NULL, res_lvalue, expr);
			}
			break;
		}
		case GL_CONSTANT_ALPHA:
		{
			gcc_jit_rvalue *alpha = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, constant, gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_int, 3)));
			for (GLint i = 0; i < 3; ++i)
			{
				gcc_jit_rvalue *idx = gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_int, i);
				gcc_jit_lvalue *res_lvalue = gcc_jit_context_new_array_access(ctx, NULL, res, idx);
				gcc_jit_rvalue *org_rvalue = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, org, idx));
				gcc_jit_rvalue *expr = gcc_jit_context_new_binary_op(ctx, NULL, GCC_JIT_BINARY_OP_MULT, jit_ctx->type_float, org_rvalue, alpha);
				gcc_jit_block_add_assignment(block, NULL, res_lvalue, expr);
			}
			break;
		}
		case GL_ONE_MINUS_CONSTANT_ALPHA:
		{
			gcc_jit_rvalue *alpha = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, constant, gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_int, 3)));
			gcc_jit_rvalue *one_minus_alpha = gcc_jit_context_new_binary_op(ctx, NULL, GCC_JIT_BINARY_OP_MINUS, jit_ctx->type_float, gcc_jit_context_new_rvalue_from_double(ctx, jit_ctx->type_float, 1), alpha);
			for (GLint i = 0; i < 3; ++i)
			{
				gcc_jit_rvalue *idx = gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_int, i);
				gcc_jit_lvalue *res_lvalue = gcc_jit_context_new_array_access(ctx, NULL, res, idx);
				gcc_jit_rvalue *org_rvalue = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, org, idx));
				gcc_jit_rvalue *expr = gcc_jit_context_new_binary_op(ctx, NULL, GCC_JIT_BINARY_OP_MULT, jit_ctx->type_float, org_rvalue, one_minus_alpha);
				gcc_jit_block_add_assignment(block, NULL, res_lvalue, expr);
			}
			break;
		}
		case GL_SRC_ALPHA_SATURATE:
		{
			gcc_jit_rvalue *src_alpha = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, src, gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_int, 3)));
			gcc_jit_rvalue *dst_alpha = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, dst, gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_int, 3)));
			gcc_jit_rvalue *one_minus_dst_alpha = gcc_jit_context_new_binary_op(ctx, NULL, GCC_JIT_BINARY_OP_MINUS, jit_ctx->type_float, gcc_jit_context_new_rvalue_from_double(ctx, jit_ctx->type_float, 1), dst_alpha);
			gcc_jit_lvalue *alpha_saturate = gcc_jit_function_new_local(fn, NULL, jit_ctx->type_float, "alpha_saturate");
			gcc_jit_block *next_block = gcc_jit_function_new_block(fn, NULL);
			gcc_jit_block *true_block = gcc_jit_function_new_block(fn, NULL);
			gcc_jit_block_add_assignment(true_block, NULL, alpha_saturate, src_alpha);
			gcc_jit_block_end_with_jump(true_block, NULL, next_block);
			gcc_jit_block *false_block = gcc_jit_function_new_block(fn, NULL);
			gcc_jit_block_add_assignment(false_block, NULL, alpha_saturate, one_minus_dst_alpha);
			gcc_jit_block_end_with_jump(false_block, NULL, next_block);
			gcc_jit_rvalue *cmp = gcc_jit_context_new_comparison(ctx, NULL, GCC_JIT_COMPARISON_LE, src_alpha, one_minus_dst_alpha);
			gcc_jit_block_end_with_conditional(block, NULL, cmp, true_block, false_block);
			block = next_block;
			for (GLint i = 0; i < 3; ++i)
			{
				gcc_jit_rvalue *idx = gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_int, i);
				gcc_jit_lvalue *res_lvalue = gcc_jit_context_new_array_access(ctx, NULL, res, idx);
				gcc_jit_rvalue *org_rvalue = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, org, idx));
				gcc_jit_rvalue *expr = gcc_jit_context_new_binary_op(ctx, NULL, GCC_JIT_BINARY_OP_MULT, jit_ctx->type_float, org_rvalue, gcc_jit_lvalue_as_rvalue(alpha_saturate));
				gcc_jit_block_add_assignment(block, NULL, res_lvalue, expr);
			}
			break;
		}
	}
	gcc_jit_block_end_with_void_return(block, NULL);
	return fn;
}

static gcc_jit_function *jit_blend_alpha(struct gl_ctx *gl_ctx, gcc_jit_context *ctx, const char *name, GLenum mode)
{
	struct jit_ctx *jit_ctx = &gl_ctx->jit;
	gcc_jit_param *params[] =
	{
		gcc_jit_context_new_param(ctx, NULL, jit_ctx->type_floatp, "res"),
		gcc_jit_context_new_param(ctx, NULL, jit_ctx->type_floatp, "org"),
		gcc_jit_context_new_param(ctx, NULL, jit_ctx->type_floatp, "src"),
		gcc_jit_context_new_param(ctx, NULL, jit_ctx->type_floatp, "dst"),
		gcc_jit_context_new_param(ctx, NULL, jit_ctx->type_floatp, "constant"),
	};
	gcc_jit_function *fn = gcc_jit_context_new_function(ctx, NULL, GCC_JIT_FUNCTION_INTERNAL, jit_ctx->type_void, name, sizeof(params) / sizeof(*params), params, 0);
	gcc_jit_rvalue *res = gcc_jit_param_as_rvalue(params[0]);
	gcc_jit_rvalue *org = gcc_jit_param_as_rvalue(params[1]);
	gcc_jit_rvalue *src = gcc_jit_param_as_rvalue(params[2]);
	gcc_jit_rvalue *dst = gcc_jit_param_as_rvalue(params[3]);
	gcc_jit_rvalue *constant = gcc_jit_param_as_rvalue(params[4]);
	gcc_jit_block *block = gcc_jit_function_new_block(fn, NULL);
	switch (mode)
	{
		case GL_ZERO:
		{
			gcc_jit_rvalue *idx = gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_int, 3);
			gcc_jit_lvalue *lvalue = gcc_jit_context_new_array_access(ctx, NULL, res, idx);
			gcc_jit_block_add_assignment(block, NULL, lvalue, gcc_jit_context_new_rvalue_from_double(ctx, jit_ctx->type_float, 0));
			break;
		}
		case GL_ONE:
		{
			gcc_jit_rvalue *idx = gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_int, 3);
			gcc_jit_lvalue *res_lvalue = gcc_jit_context_new_array_access(ctx, NULL, res, idx);
			gcc_jit_rvalue *org_rvalue = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, org, idx));
			gcc_jit_block_add_assignment(block, NULL, res_lvalue, org_rvalue);
			break;
		}
		case GL_SRC_COLOR:
		case GL_SRC_ALPHA:
		{
			gcc_jit_rvalue *idx = gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_int, 3);
			gcc_jit_rvalue *alpha = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, src, idx));
			gcc_jit_lvalue *res_lvalue = gcc_jit_context_new_array_access(ctx, NULL, res, idx);
			gcc_jit_rvalue *org_rvalue = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, org, idx));
			gcc_jit_rvalue *expr = gcc_jit_context_new_binary_op(ctx, NULL, GCC_JIT_BINARY_OP_MULT, jit_ctx->type_float, org_rvalue, alpha);
			gcc_jit_block_add_assignment(block, NULL, res_lvalue, expr);
			break;
		}
		case GL_ONE_MINUS_SRC_COLOR:
		case GL_ONE_MINUS_SRC_ALPHA:
		{
			gcc_jit_rvalue *idx = gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_int, 3);
			gcc_jit_rvalue *alpha = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, src, idx));
			gcc_jit_rvalue *one_minus_alpha = gcc_jit_context_new_binary_op(ctx, NULL, GCC_JIT_BINARY_OP_MINUS, jit_ctx->type_float, gcc_jit_context_new_rvalue_from_double(ctx, jit_ctx->type_float, 1), alpha);
			gcc_jit_lvalue *res_lvalue = gcc_jit_context_new_array_access(ctx, NULL, res, gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_int, 0));
			gcc_jit_rvalue *org_rvalue = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, org, idx));
			gcc_jit_rvalue *expr = gcc_jit_context_new_binary_op(ctx, NULL, GCC_JIT_BINARY_OP_MULT, jit_ctx->type_float, org_rvalue, one_minus_alpha);
			gcc_jit_block_add_assignment(block, NULL, res_lvalue, expr);
			break;
		}
		case GL_DST_COLOR:
		case GL_DST_ALPHA:
		{
			gcc_jit_rvalue *idx = gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_int, 3);
			gcc_jit_rvalue *alpha = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, dst, idx));
			gcc_jit_lvalue *res_lvalue = gcc_jit_context_new_array_access(ctx, NULL, res, idx);
			gcc_jit_rvalue *org_rvalue = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, org, idx));
			gcc_jit_rvalue *expr = gcc_jit_context_new_binary_op(ctx, NULL, GCC_JIT_BINARY_OP_MULT, jit_ctx->type_float, org_rvalue, alpha);
			gcc_jit_block_add_assignment(block, NULL, res_lvalue, expr);
			break;
		}
		case GL_ONE_MINUS_DST_COLOR:
		case GL_ONE_MINUS_DST_ALPHA:
		{
			gcc_jit_rvalue *idx = gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_int, 3);
			gcc_jit_rvalue *alpha = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, dst, idx));
			gcc_jit_rvalue *one_minus_alpha = gcc_jit_context_new_binary_op(ctx, NULL, GCC_JIT_BINARY_OP_MINUS, jit_ctx->type_float, gcc_jit_context_new_rvalue_from_double(ctx, jit_ctx->type_float, 1), alpha);
			gcc_jit_lvalue *res_lvalue = gcc_jit_context_new_array_access(ctx, NULL, res, gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_int, 0));
			gcc_jit_rvalue *org_rvalue = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, org, idx));
			gcc_jit_rvalue *expr = gcc_jit_context_new_binary_op(ctx, NULL, GCC_JIT_BINARY_OP_MULT, jit_ctx->type_float, org_rvalue, one_minus_alpha);
			gcc_jit_block_add_assignment(block, NULL, res_lvalue, expr);
			break;
		}
		case GL_CONSTANT_COLOR:
		case GL_CONSTANT_ALPHA:
		{
			gcc_jit_rvalue *idx = gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_int, 3);
			gcc_jit_rvalue *alpha = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, constant, idx));
			gcc_jit_lvalue *res_lvalue = gcc_jit_context_new_array_access(ctx, NULL, res, idx);
			gcc_jit_rvalue *org_rvalue = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, org, idx));
			gcc_jit_rvalue *expr = gcc_jit_context_new_binary_op(ctx, NULL, GCC_JIT_BINARY_OP_MULT, jit_ctx->type_float, org_rvalue, alpha);
			gcc_jit_block_add_assignment(block, NULL, res_lvalue, expr);
			break;
		}
		case GL_ONE_MINUS_CONSTANT_COLOR:
		case GL_ONE_MINUS_CONSTANT_ALPHA:
		{
			gcc_jit_rvalue *idx = gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_int, 3);
			gcc_jit_rvalue *alpha = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, constant, idx));
			gcc_jit_rvalue *one_minus_alpha = gcc_jit_context_new_binary_op(ctx, NULL, GCC_JIT_BINARY_OP_MINUS, jit_ctx->type_float, gcc_jit_context_new_rvalue_from_double(ctx, jit_ctx->type_float, 1), alpha);
			gcc_jit_lvalue *res_lvalue = gcc_jit_context_new_array_access(ctx, NULL, res, gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_int, 0));
			gcc_jit_rvalue *org_rvalue = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, org, idx));
			gcc_jit_rvalue *expr = gcc_jit_context_new_binary_op(ctx, NULL, GCC_JIT_BINARY_OP_MULT, jit_ctx->type_float, org_rvalue, one_minus_alpha);
			gcc_jit_block_add_assignment(block, NULL, res_lvalue, expr);
			break;
		}
		case GL_SRC_ALPHA_SATURATE:
		{
			gcc_jit_rvalue *idx = gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_int, 3);
			gcc_jit_rvalue *src_alpha = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, src, idx));
			gcc_jit_rvalue *dst_alpha = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, dst, idx));
			gcc_jit_rvalue *one_minus_dst_alpha = gcc_jit_context_new_binary_op(ctx, NULL, GCC_JIT_BINARY_OP_MINUS, jit_ctx->type_float, gcc_jit_context_new_rvalue_from_double(ctx, jit_ctx->type_float, 1), dst_alpha);
			gcc_jit_lvalue *alpha_saturate = gcc_jit_function_new_local(fn, NULL, jit_ctx->type_float, "alpha_saturate");
			gcc_jit_block *next_block = gcc_jit_function_new_block(fn, NULL);
			gcc_jit_block *true_block = gcc_jit_function_new_block(fn, NULL);
			gcc_jit_block_add_assignment(true_block, NULL, alpha_saturate, src_alpha);
			gcc_jit_block_end_with_jump(true_block, NULL, next_block);
			gcc_jit_block *false_block = gcc_jit_function_new_block(fn, NULL);
			gcc_jit_block_add_assignment(false_block, NULL, alpha_saturate, one_minus_dst_alpha);
			gcc_jit_block_end_with_jump(false_block, NULL, next_block);
			gcc_jit_rvalue *cmp = gcc_jit_context_new_comparison(ctx, NULL, GCC_JIT_COMPARISON_LE, src_alpha, one_minus_dst_alpha);
			gcc_jit_block_end_with_conditional(block, NULL, cmp, true_block, false_block);
			block = next_block;
			gcc_jit_lvalue *res_lvalue = gcc_jit_context_new_array_access(ctx, NULL, res, idx);
			gcc_jit_rvalue *org_rvalue = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, org, idx));
			gcc_jit_rvalue *expr = gcc_jit_context_new_binary_op(ctx, NULL, GCC_JIT_BINARY_OP_MULT, jit_ctx->type_float, org_rvalue, gcc_jit_lvalue_as_rvalue(alpha_saturate));
			gcc_jit_block_add_assignment(block, NULL, res_lvalue, expr);
			break;
		}
	}
	gcc_jit_block_end_with_void_return(block, NULL);
	return fn;
}

static gcc_jit_function *jit_blend_eq_rgb(struct gl_ctx *gl_ctx, gcc_jit_context *ctx, GLenum mode)
{
	struct jit_ctx *jit_ctx = &gl_ctx->jit;
	gcc_jit_param *params[] =
	{
		gcc_jit_context_new_param(ctx, NULL, jit_ctx->type_floatp, "res"),
		gcc_jit_context_new_param(ctx, NULL, jit_ctx->type_floatp, "src"),
		gcc_jit_context_new_param(ctx, NULL, jit_ctx->type_floatp, "dst"),
	};
	gcc_jit_function *fn = gcc_jit_context_new_function(ctx, NULL, GCC_JIT_FUNCTION_INTERNAL, jit_ctx->type_void, "blend_eq_rgb", sizeof(params) / sizeof(*params), params, 0);
	gcc_jit_rvalue *res = gcc_jit_param_as_rvalue(params[0]);
	gcc_jit_rvalue *src = gcc_jit_param_as_rvalue(params[1]);
	gcc_jit_rvalue *dst = gcc_jit_param_as_rvalue(params[2]);
	gcc_jit_block *block = gcc_jit_function_new_block(fn, NULL);
	switch (mode)
	{
		case GL_FUNC_ADD:
		{
			for (GLint i = 0; i < 3; ++i)
			{
				gcc_jit_rvalue *idx = gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_int, i);
				gcc_jit_lvalue *res_lvalue = gcc_jit_context_new_array_access(ctx, NULL, res, idx);
				gcc_jit_rvalue *src_rvalue = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, src, idx));
				gcc_jit_rvalue *dst_rvalue = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, dst, idx));
				gcc_jit_rvalue *expr = gcc_jit_context_new_binary_op(ctx, NULL, GCC_JIT_BINARY_OP_PLUS, jit_ctx->type_float, src_rvalue, dst_rvalue);
				gcc_jit_block_add_assignment(block, NULL, res_lvalue, expr);
			}
			break;
		}
		case GL_FUNC_SUBSTRACT:
		{
			for (GLint i = 0; i < 3; ++i)
			{
				gcc_jit_rvalue *idx = gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_int, i);
				gcc_jit_lvalue *res_lvalue = gcc_jit_context_new_array_access(ctx, NULL, res, idx);
				gcc_jit_rvalue *src_rvalue = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, src, idx));
				gcc_jit_rvalue *dst_rvalue = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, dst, idx));
				gcc_jit_rvalue *expr = gcc_jit_context_new_binary_op(ctx, NULL, GCC_JIT_BINARY_OP_MINUS, jit_ctx->type_float, src_rvalue, dst_rvalue);
				gcc_jit_block_add_assignment(block, NULL, res_lvalue, expr);
			}
			break;
		}
		case GL_FUNC_REVERSE_SUBSTRACT:
		{
			for (GLint i = 0; i < 3; ++i)
			{
				gcc_jit_rvalue *idx = gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_int, i);
				gcc_jit_lvalue *res_lvalue = gcc_jit_context_new_array_access(ctx, NULL, res, idx);
				gcc_jit_rvalue *src_rvalue = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, src, idx));
				gcc_jit_rvalue *dst_rvalue = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, dst, idx));
				gcc_jit_rvalue *expr = gcc_jit_context_new_binary_op(ctx, NULL, GCC_JIT_BINARY_OP_MINUS, jit_ctx->type_float, dst_rvalue, src_rvalue);
				gcc_jit_block_add_assignment(block, NULL, res_lvalue, expr);
			}
			break;
		}
		case GL_MIN:
		{
			gcc_jit_block *next_block;
			for (GLint i = 0; i < 3; ++i)
			{
				if (i != 2)
					next_block = gcc_jit_function_new_block(fn, NULL);
				gcc_jit_rvalue *idx = gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_int, i);
				gcc_jit_lvalue *res_lvalue = gcc_jit_context_new_array_access(ctx, NULL, res, idx);
				gcc_jit_rvalue *src_rvalue = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, src, idx));
				gcc_jit_rvalue *dst_rvalue = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, dst, idx));
				gcc_jit_block *true_block = gcc_jit_function_new_block(fn, NULL);
				gcc_jit_block_add_assignment(true_block, NULL, res_lvalue, src_rvalue);
				if (i != 2)
					gcc_jit_block_end_with_jump(true_block, NULL, next_block);
				else
					gcc_jit_block_end_with_void_return(true_block, NULL);
				gcc_jit_block *false_block = gcc_jit_function_new_block(fn, NULL);
				gcc_jit_block_add_assignment(false_block, NULL, res_lvalue, dst_rvalue);
				if (i != 2)
					gcc_jit_block_end_with_jump(false_block, NULL, next_block);
				else
					gcc_jit_block_end_with_void_return(false_block, NULL);
				gcc_jit_rvalue *cmp = gcc_jit_context_new_comparison(ctx, NULL, GCC_JIT_COMPARISON_LE, src_rvalue, dst_rvalue);
				gcc_jit_block_end_with_conditional(block, NULL, cmp, true_block, false_block);
				block = next_block;
			}
			return fn;
		}
		case GL_MAX:
		{
			gcc_jit_block *next_block;
			for (GLint i = 0; i < 3; ++i)
			{
				if (i != 2)
					next_block = gcc_jit_function_new_block(fn, NULL);
				gcc_jit_rvalue *idx = gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_int, i);
				gcc_jit_lvalue *res_lvalue = gcc_jit_context_new_array_access(ctx, NULL, res, idx);
				gcc_jit_rvalue *src_rvalue = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, src, idx));
				gcc_jit_rvalue *dst_rvalue = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, dst, idx));
				gcc_jit_block *true_block = gcc_jit_function_new_block(fn, NULL);
				gcc_jit_block_add_assignment(true_block, NULL, res_lvalue, src_rvalue);
				if (i != 2)
					gcc_jit_block_end_with_jump(true_block, NULL, next_block);
				else
					gcc_jit_block_end_with_void_return(true_block, NULL);
				gcc_jit_block *false_block = gcc_jit_function_new_block(fn, NULL);
				gcc_jit_block_add_assignment(false_block, NULL, res_lvalue, dst_rvalue);
				if (i != 2)
					gcc_jit_block_end_with_jump(false_block, NULL, next_block);
				else
					gcc_jit_block_end_with_void_return(false_block, NULL);
				gcc_jit_rvalue *cmp = gcc_jit_context_new_comparison(ctx, NULL, GCC_JIT_COMPARISON_GE, src_rvalue, dst_rvalue);
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

static gcc_jit_function *jit_blend_eq_alpha(struct gl_ctx *gl_ctx, gcc_jit_context *ctx, GLenum mode)
{
	struct jit_ctx *jit_ctx = &gl_ctx->jit;
	gcc_jit_param *params[] =
	{
		gcc_jit_context_new_param(ctx, NULL, jit_ctx->type_floatp, "res"),
		gcc_jit_context_new_param(ctx, NULL, jit_ctx->type_floatp, "src"),
		gcc_jit_context_new_param(ctx, NULL, jit_ctx->type_floatp, "dst"),
	};
	gcc_jit_function *fn = gcc_jit_context_new_function(ctx, NULL, GCC_JIT_FUNCTION_INTERNAL, jit_ctx->type_void, "blend_eq_alpha", sizeof(params) / sizeof(*params), params, 0);
	gcc_jit_rvalue *res = gcc_jit_param_as_rvalue(params[0]);
	gcc_jit_rvalue *src = gcc_jit_param_as_rvalue(params[1]);
	gcc_jit_rvalue *dst = gcc_jit_param_as_rvalue(params[2]);
	gcc_jit_block *block = gcc_jit_function_new_block(fn, NULL);
	switch (mode)
	{
		case GL_FUNC_ADD:
		{
			gcc_jit_rvalue *idx = gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_int, 3);
			gcc_jit_lvalue *res_lvalue = gcc_jit_context_new_array_access(ctx, NULL, res, idx);
			gcc_jit_rvalue *src_rvalue = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, src, idx));
			gcc_jit_rvalue *dst_rvalue = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, dst, idx));
			gcc_jit_rvalue *expr = gcc_jit_context_new_binary_op(ctx, NULL, GCC_JIT_BINARY_OP_PLUS, jit_ctx->type_float, src_rvalue, dst_rvalue);
			gcc_jit_block_add_assignment(block, NULL, res_lvalue, expr);
			break;
		}
		case GL_FUNC_SUBSTRACT:
		{
			gcc_jit_rvalue *idx = gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_int, 3);
			gcc_jit_lvalue *res_lvalue = gcc_jit_context_new_array_access(ctx, NULL, res, idx);
			gcc_jit_rvalue *src_rvalue = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, src, idx));
			gcc_jit_rvalue *dst_rvalue = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, dst, idx));
			gcc_jit_rvalue *expr = gcc_jit_context_new_binary_op(ctx, NULL, GCC_JIT_BINARY_OP_MINUS, jit_ctx->type_float, src_rvalue, dst_rvalue);
			gcc_jit_block_add_assignment(block, NULL, res_lvalue, expr);
			break;
		}
		case GL_FUNC_REVERSE_SUBSTRACT:
		{
			gcc_jit_rvalue *idx = gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_int, 3);
			gcc_jit_lvalue *res_lvalue = gcc_jit_context_new_array_access(ctx, NULL, res, idx);
			gcc_jit_rvalue *src_rvalue = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, src, idx));
			gcc_jit_rvalue *dst_rvalue = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, dst, idx));
			gcc_jit_rvalue *expr = gcc_jit_context_new_binary_op(ctx, NULL, GCC_JIT_BINARY_OP_MINUS, jit_ctx->type_float, dst_rvalue, src_rvalue);
			gcc_jit_block_add_assignment(block, NULL, res_lvalue, expr);
			break;
		}
		case GL_MIN:
		{
			gcc_jit_rvalue *idx = gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_int, 3);
			gcc_jit_rvalue *src_rvalue = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, src, idx));
			gcc_jit_rvalue *dst_rvalue = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, dst, idx));
			gcc_jit_lvalue *res_lvalue = gcc_jit_context_new_array_access(ctx, NULL, res, idx);
			gcc_jit_block *true_block = gcc_jit_function_new_block(fn, NULL);
			gcc_jit_block_add_assignment(true_block, NULL, res_lvalue, src_rvalue);
			gcc_jit_block_end_with_void_return(true_block, NULL);
			gcc_jit_block *false_block = gcc_jit_function_new_block(fn, NULL);
			gcc_jit_block_add_assignment(false_block, NULL, res_lvalue, dst_rvalue);
			gcc_jit_block_end_with_void_return(false_block, NULL);
			gcc_jit_rvalue *cmp = gcc_jit_context_new_comparison(ctx, NULL, GCC_JIT_COMPARISON_LE, src_rvalue, dst_rvalue);
			gcc_jit_block_end_with_conditional(block, NULL, cmp, true_block, false_block);
			return fn;
		}
		case GL_MAX:
		{
			gcc_jit_rvalue *idx = gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_int, 3);
			gcc_jit_rvalue *src_rvalue = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, src, idx));
			gcc_jit_rvalue *dst_rvalue = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, dst, idx));
			gcc_jit_lvalue *res_lvalue = gcc_jit_context_new_array_access(ctx, NULL, res, idx);
			gcc_jit_block *true_block = gcc_jit_function_new_block(fn, NULL);
			gcc_jit_block_add_assignment(true_block, NULL, res_lvalue, src_rvalue);
			gcc_jit_block_end_with_void_return(true_block, NULL);
			gcc_jit_block *false_block = gcc_jit_function_new_block(fn, NULL);
			gcc_jit_block_add_assignment(false_block, NULL, res_lvalue, dst_rvalue);
			gcc_jit_block_end_with_void_return(false_block, NULL);
			gcc_jit_rvalue *cmp = gcc_jit_context_new_comparison(ctx, NULL, GCC_JIT_COMPARISON_GE, src_rvalue, dst_rvalue);
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

static gcc_jit_function *jit_color_mask(struct gl_ctx *gl_ctx, gcc_jit_context *ctx)
{
	struct jit_ctx *jit_ctx = &gl_ctx->jit;
	gcc_jit_param *params[] =
	{
		gcc_jit_context_new_param(ctx, NULL, jit_ctx->type_floatp, "dst"),
		gcc_jit_context_new_param(ctx, NULL, jit_ctx->type_floatp, "src"),
	};
	gcc_jit_function *fn = gcc_jit_context_new_function(ctx, NULL, GCC_JIT_FUNCTION_INTERNAL, jit_ctx->type_void, "color_mask", sizeof(params) / sizeof(*params), params, 0);
	gcc_jit_rvalue *dst = gcc_jit_param_as_rvalue(params[0]);
	gcc_jit_rvalue *src = gcc_jit_param_as_rvalue(params[1]);
	gcc_jit_block *block = gcc_jit_function_new_block(fn, NULL);
	for (GLint i = 0; i < 4; ++i)
	{
		if (!gl_ctx->color_mask[i])
			continue;
		gcc_jit_rvalue *idx = gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_int, i);
		gcc_jit_lvalue *dst_lvalue = gcc_jit_context_new_array_access(ctx, NULL, dst, idx);
		gcc_jit_rvalue *src_rvalue = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, src, idx));
		gcc_jit_block_add_assignment(block, NULL, dst_lvalue, src_rvalue);
	}
	gcc_jit_block_end_with_void_return(block, NULL);
	return fn;
}

static gcc_jit_function *jit_blend_disabled(struct gl_ctx *gl_ctx, gcc_jit_context *ctx)
{
	struct jit_ctx *jit_ctx = &gl_ctx->jit;
	gcc_jit_function *color_mask_fn = jit_color_mask(gl_ctx, ctx);
	gcc_jit_param *params[] =
	{
		gcc_jit_context_new_param(ctx, NULL, jit_ctx->type_floatp, "src"),
		gcc_jit_context_new_param(ctx, NULL, jit_ctx->type_floatp, "dst"),
		gcc_jit_context_new_param(ctx, NULL, jit_ctx->type_floatp, "constant"),
	};
	gcc_jit_function *fn = gcc_jit_context_new_function(ctx, NULL, GCC_JIT_FUNCTION_INTERNAL, jit_ctx->type_void, "blend", sizeof(params) / sizeof(*params), params, 0);
	gcc_jit_rvalue *src = gcc_jit_param_as_rvalue(params[0]);
	gcc_jit_rvalue *dst = gcc_jit_param_as_rvalue(params[1]);
	gcc_jit_block *block = gcc_jit_function_new_block(fn, NULL);
	gcc_jit_rvalue *color_mask_args[] = {dst, src};
	gcc_jit_block_add_eval(block, NULL, gcc_jit_context_new_call(ctx, NULL, color_mask_fn, sizeof(color_mask_args) / sizeof(*color_mask_args), color_mask_args));
	gcc_jit_block_end_with_void_return(block, NULL);
	return fn;
}

static gcc_jit_function *jit_blend_enabled(struct gl_ctx *gl_ctx, gcc_jit_context *ctx)
{
	struct jit_ctx *jit_ctx = &gl_ctx->jit;
	gcc_jit_function *blend_src_rgb_fn = jit_blend_rgb(gl_ctx, ctx, "blend_src_rgb", gl_ctx->blend_src_rgb);
	gcc_jit_function *blend_dst_rgb_fn = jit_blend_rgb(gl_ctx, ctx, "blend_dst_rgb", gl_ctx->blend_dst_rgb);
	gcc_jit_function *blend_src_alpha_fn = jit_blend_alpha(gl_ctx, ctx, "blend_src_alpha", gl_ctx->blend_src_alpha);
	gcc_jit_function *blend_dst_alpha_fn = jit_blend_alpha(gl_ctx, ctx, "blend_dst_alpha", gl_ctx->blend_dst_alpha);
	gcc_jit_function *blend_eq_rgb_fn = jit_blend_eq_rgb(gl_ctx, ctx, gl_ctx->blend_equation_rgb);
	gcc_jit_function *blend_eq_alpha_fn = jit_blend_eq_alpha(gl_ctx, ctx, gl_ctx->blend_equation_alpha);
	gcc_jit_function *color_mask_fn = jit_color_mask(gl_ctx, ctx);
	gcc_jit_param *params[] =
	{
		gcc_jit_context_new_param(ctx, NULL, jit_ctx->type_floatp, "src"),
		gcc_jit_context_new_param(ctx, NULL, jit_ctx->type_floatp, "dst"),
		gcc_jit_context_new_param(ctx, NULL, jit_ctx->type_floatp, "constant"),
	};
	gcc_jit_function *fn = gcc_jit_context_new_function(ctx, NULL, GCC_JIT_FUNCTION_INTERNAL, jit_ctx->type_void, "blend", sizeof(params) / sizeof(*params), params, 0);
	gcc_jit_rvalue *src = gcc_jit_param_as_rvalue(params[0]);
	gcc_jit_rvalue *dst = gcc_jit_param_as_rvalue(params[1]);
	gcc_jit_rvalue *constant = gcc_jit_param_as_rvalue(params[2]);
	gcc_jit_lvalue *tmp_src_array = gcc_jit_function_new_local(fn, NULL, jit_ctx->type_float4, "tmp_src");
	gcc_jit_rvalue *tmp_src = gcc_jit_lvalue_get_address(gcc_jit_context_new_array_access(ctx, NULL, gcc_jit_lvalue_as_rvalue(tmp_src_array), gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_int, 0)), NULL);
	gcc_jit_lvalue *tmp_dst_array = gcc_jit_function_new_local(fn, NULL, jit_ctx->type_float4, "tmp_dst");
	gcc_jit_rvalue *tmp_dst = gcc_jit_lvalue_get_address(gcc_jit_context_new_array_access(ctx, NULL, gcc_jit_lvalue_as_rvalue(tmp_dst_array), gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_int, 0)), NULL);
	gcc_jit_lvalue *tmp_res_array = gcc_jit_function_new_local(fn, NULL, jit_ctx->type_float4, "tmp_res");
	gcc_jit_rvalue *tmp_res = gcc_jit_lvalue_get_address(gcc_jit_context_new_array_access(ctx, NULL, gcc_jit_lvalue_as_rvalue(tmp_res_array), gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_int, 0)), NULL);
	gcc_jit_block *block = gcc_jit_function_new_block(fn, NULL);
	gcc_jit_rvalue *blend_src_rgb_args[] = {tmp_src, src, src, dst, constant};
	gcc_jit_block_add_eval(block, NULL, gcc_jit_context_new_call(ctx, NULL, blend_src_rgb_fn, sizeof(blend_src_rgb_args) / sizeof(*blend_src_rgb_args), blend_src_rgb_args));
	gcc_jit_rvalue *blend_dst_rgb_args[] = {tmp_dst, dst, src, dst, constant};
	gcc_jit_block_add_eval(block, NULL, gcc_jit_context_new_call(ctx, NULL, blend_dst_rgb_fn, sizeof(blend_dst_rgb_args) / sizeof(*blend_dst_rgb_args), blend_dst_rgb_args));
	gcc_jit_rvalue *blend_src_alpha_args[] = {tmp_src, src, src, dst, constant};
	gcc_jit_block_add_eval(block, NULL, gcc_jit_context_new_call(ctx, NULL, blend_src_alpha_fn, sizeof(blend_src_alpha_args) / sizeof(*blend_src_alpha_args), blend_src_alpha_args));
	gcc_jit_rvalue *blend_dst_alpha_args[] = {tmp_dst, dst, src, dst, constant};
	gcc_jit_block_add_eval(block, NULL, gcc_jit_context_new_call(ctx, NULL, blend_dst_alpha_fn, sizeof(blend_dst_alpha_args) / sizeof(*blend_dst_alpha_args), blend_dst_alpha_args));
	gcc_jit_rvalue *blend_eq_rgb_args[] = {tmp_res, tmp_src, tmp_dst};
	gcc_jit_block_add_eval(block, NULL, gcc_jit_context_new_call(ctx, NULL, blend_eq_rgb_fn, sizeof(blend_eq_rgb_args) / sizeof(*blend_eq_rgb_args), blend_eq_rgb_args));
	gcc_jit_rvalue *blend_eq_alpha_args[] = {tmp_res, tmp_src, tmp_dst};
	gcc_jit_block_add_eval(block, NULL, gcc_jit_context_new_call(ctx, NULL, blend_eq_alpha_fn, sizeof(blend_eq_alpha_args) / sizeof(*blend_eq_alpha_args), blend_eq_alpha_args));
	gcc_jit_rvalue *color_mask_args[] = {dst, tmp_res};
	gcc_jit_block_add_eval(block, NULL, gcc_jit_context_new_call(ctx, NULL, color_mask_fn, sizeof(color_mask_args) / sizeof(*color_mask_args), color_mask_args));
	gcc_jit_block_end_with_void_return(block, NULL);
	return fn;
}

static gcc_jit_function *jit_blend(struct gl_ctx *gl_ctx, gcc_jit_context *ctx)
{
	if (gl_ctx->blend)
		return jit_blend_enabled(gl_ctx, ctx);
	return jit_blend_disabled(gl_ctx, ctx);
}

static gcc_jit_function *jit_fragment_set(struct gl_ctx *gl_ctx, gcc_jit_context *ctx)
{
	struct jit_ctx *jit_ctx = &gl_ctx->jit;
	gcc_jit_function *blend_fn = jit_blend(gl_ctx, ctx);
	gcc_jit_param *params[] =
	{
		gcc_jit_context_new_param(ctx, NULL, jit_ctx->type_floatp, "color_buf"),
		gcc_jit_context_new_param(ctx, NULL, jit_ctx->type_floatp, "depth_buf"),
		gcc_jit_context_new_param(ctx, NULL, jit_ctx->type_floatp, "color"),
		gcc_jit_context_new_param(ctx, NULL, jit_ctx->type_float, "depth"),
		gcc_jit_context_new_param(ctx, NULL, jit_ctx->type_floatp, "constant"),
	};
	gcc_jit_function *fn = gcc_jit_context_new_function(ctx, NULL, GCC_JIT_FUNCTION_EXPORTED, jit_ctx->type_void, "fragment_set", sizeof(params) / sizeof(*params), params, 0);
	gcc_jit_rvalue *color_buf = gcc_jit_param_as_rvalue(params[0]);
	gcc_jit_rvalue *depth_buf = gcc_jit_param_as_rvalue(params[1]);
	gcc_jit_rvalue *color = gcc_jit_param_as_rvalue(params[2]);
	gcc_jit_rvalue *depth = gcc_jit_param_as_rvalue(params[3]);
	gcc_jit_rvalue *constant = gcc_jit_param_as_rvalue(params[4]);
	gcc_jit_block *block = gcc_jit_function_new_block(fn, NULL);
	gcc_jit_rvalue *blend_args[] = {color, color_buf, constant};
	gcc_jit_block_add_eval(block, NULL, gcc_jit_context_new_call(ctx, NULL, blend_fn, sizeof(blend_args) / sizeof(*blend_args), blend_args));
	if (gl_ctx->depth_write)
	{
		gcc_jit_lvalue *lvalue = gcc_jit_rvalue_dereference(depth_buf, NULL);
		gcc_jit_block_add_assignment(block, NULL, lvalue, depth);
	}
	gcc_jit_block_end_with_void_return(block, NULL);
	return fn;
}

static GLuint fragment_set_state_hash(struct gl_ctx *gl_ctx)
{
	GLuint hash = FNV_BASIS32;
	hash = fnv32v(hash, &gl_ctx->blend_src_rgb, sizeof(gl_ctx->blend_src_rgb));
	hash = fnv32v(hash, &gl_ctx->blend_src_alpha, sizeof(gl_ctx->blend_src_alpha));
	hash = fnv32v(hash, &gl_ctx->blend_dst_rgb, sizeof(gl_ctx->blend_dst_rgb));
	hash = fnv32v(hash, &gl_ctx->blend_dst_alpha, sizeof(gl_ctx->blend_dst_alpha));
	hash = fnv32v(hash, &gl_ctx->blend_equation_rgb, sizeof(gl_ctx->blend_equation_rgb));
	hash = fnv32v(hash, &gl_ctx->blend_equation_alpha, sizeof(gl_ctx->blend_equation_alpha));
	hash = fnv32v(hash, &gl_ctx->color_mask, sizeof(gl_ctx->color_mask));
	hash = fnv32v(hash, &gl_ctx->blend, sizeof(gl_ctx->blend));
	hash = fnv32v(hash, &gl_ctx->depth_write, sizeof(gl_ctx->depth_write));
	return hash;
}

static struct fragment_set_head *fragment_set_head(struct gl_ctx *gl_ctx, GLuint hash)
{
	return &gl_ctx->jit.fragment_set_hash[hash % sizeof(gl_ctx->jit.fragment_set_hash) / sizeof(*gl_ctx->jit.fragment_set_hash)];
}

static struct fragment_set_state *get_fragment_set_state(struct gl_ctx *gl_ctx)
{
	GLuint hash = fragment_set_state_hash(gl_ctx);
	struct fragment_set_head *hash_head = fragment_set_head(gl_ctx, hash);
	if (TAILQ_EMPTY(hash_head))
		return NULL;
	struct fragment_set_state *state;
	TAILQ_FOREACH(state, hash_head, hash_chain)
	{
		if (state->hash == hash
		 && state->blend_src_rgb == gl_ctx->blend_src_rgb
		 && state->blend_src_alpha == gl_ctx->blend_src_alpha
		 && state->blend_dst_rgb == gl_ctx->blend_dst_rgb
		 && state->blend_dst_alpha == gl_ctx->blend_dst_alpha
		 && state->blend_equation_rgb == gl_ctx->blend_equation_rgb
		 && state->blend_equation_alpha == gl_ctx->blend_equation_alpha
		 && state->color_mask[0] == gl_ctx->color_mask[0]
		 && state->color_mask[1] == gl_ctx->color_mask[1]
		 && state->color_mask[2] == gl_ctx->color_mask[2]
		 && state->color_mask[3] == gl_ctx->color_mask[3]
		 && state->blend == gl_ctx->blend
		 && state->depth_write == gl_ctx->depth_write)
			return state;
	}
	return NULL;
}

static struct fragment_set_state *create_fragment_set_state(struct gl_ctx *gl_ctx)
{
	struct fragment_set_state *state = malloc(sizeof(*state));
	if (!state)
		return NULL;
	state->hash = fragment_set_state_hash(gl_ctx);
	state->blend_src_rgb = gl_ctx->blend_src_rgb;
	state->blend_src_alpha = gl_ctx->blend_src_alpha;
	state->blend_dst_rgb = gl_ctx->blend_dst_rgb;
	state->blend_dst_alpha = gl_ctx->blend_dst_alpha;
	state->blend_equation_rgb = gl_ctx->blend_equation_rgb;
	state->blend_equation_alpha = gl_ctx->blend_equation_alpha;
	state->color_mask[0] = gl_ctx->color_mask[0];
	state->color_mask[1] = gl_ctx->color_mask[1];
	state->color_mask[2] = gl_ctx->color_mask[2];
	state->color_mask[3] = gl_ctx->color_mask[3];
	state->blend = gl_ctx->blend;
	state->depth_write = gl_ctx->depth_write;
	gcc_jit_context *ctx = gcc_jit_context_new_child_context(gl_ctx->jit.ctx);
	jit_fragment_set(gl_ctx, ctx);
	state->jit_res = gcc_jit_context_compile(ctx);
	gcc_jit_context_release(ctx);
	state->fn = gcc_jit_result_get_code(state->jit_res, "fragment_set");
	return state;
}

static void delete_fragment_set_state(struct gl_ctx *gl_ctx, struct fragment_set_state *state)
{
	TAILQ_REMOVE(&gl_ctx->jit.fragment_set_fifo, state, fifo_chain);
	TAILQ_REMOVE(fragment_set_head(gl_ctx, state->hash), state, hash_chain);
	gcc_jit_result_release(state->jit_res);
	free(state);
	gl_ctx->jit.fragment_set_count--;
}

GLboolean jit_update_fragment_set(struct gl_ctx *gl_ctx)
{
	static const GLuint dirty_mask = GL_CTX_DIRTY_BLEND_SRC_RGB
	                               | GL_CTX_DIRTY_BLEND_SRC_ALPHA
	                               | GL_CTX_DIRTY_BLEND_DST_RGB
	                               | GL_CTX_DIRTY_BLEND_DST_ALPHA
	                               | GL_CTX_DIRTY_BLEND_EQUATION_RGB
	                               | GL_CTX_DIRTY_BLEND_EQUATION_ALPHA
	                               | GL_CTX_DIRTY_COLOR_MASK_R
	                               | GL_CTX_DIRTY_COLOR_MASK_G
	                               | GL_CTX_DIRTY_COLOR_MASK_B
	                               | GL_CTX_DIRTY_COLOR_MASK_A
	                               | GL_CTX_DIRTY_BLENDING
	                               | GL_CTX_DIRTY_DEPTH_WRITE;
	if (!(gl_ctx->dirty & dirty_mask))
		return GL_TRUE;
	gl_ctx->dirty &= ~dirty_mask;
	struct fragment_set_state *state = get_fragment_set_state(gl_ctx);
	if (!state)
	{
		state = create_fragment_set_state(gl_ctx);
		if (!state)
			return GL_FALSE;
		TAILQ_INSERT_TAIL(fragment_set_head(gl_ctx, state->hash), state, hash_chain);
		gl_ctx->jit.fragment_set_count++;
		if (gl_ctx->jit.fragment_set_count > 1024)
			delete_fragment_set_state(gl_ctx, TAILQ_FIRST(&gl_ctx->jit.fragment_set_fifo));
	}
	else
	{
		TAILQ_REMOVE(&gl_ctx->jit.fragment_set_fifo, state, fifo_chain);
	}
	TAILQ_INSERT_TAIL(&gl_ctx->jit.fragment_set_fifo, state, fifo_chain);
	gl_ctx->jit.fragment_set_state = state;
	return GL_TRUE;
}
