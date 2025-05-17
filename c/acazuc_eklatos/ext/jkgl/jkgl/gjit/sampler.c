#include "gjit.h"

#include <assert.h>
#include <stdlib.h>
#include <errno.h>

static gcc_jit_function *
jit_sample(struct jkg_ctx *ctx,
           gcc_jit_context *jit_ctx,
           const struct jkg_sampler_create_info *create_info);

int
gjit_sampler_alloc(struct jkg_ctx *ctx,
                   const struct jkg_sampler_create_info *create_info,
                   struct jkg_sampler **samplerp)
{
	struct jkg_sampler *sampler = NULL;
	gcc_jit_context *jit_ctx;

	(void)ctx;
	sampler = malloc(sizeof(*sampler));
	if (!sampler)
		return -ENOMEM;
	jit_ctx = gcc_jit_context_new_child_context(ctx->jit.sampler.ctx);
	jit_sample(ctx, jit_ctx, create_info);
	sampler->res = gcc_jit_context_compile(jit_ctx);
	gcc_jit_context_release(jit_ctx);
	sampler->fn = gcc_jit_result_get_code(sampler->res, "sample");
	sampler->min_filter = create_info->min_filter;
	sampler->mag_filter = create_info->mag_filter;
	sampler->mip_filter = create_info->mip_filter;
	sampler->compare_enable = create_info->compare_enable;
	sampler->compare_func = create_info->compare_func;
	sampler->lod_bias = create_info->lod_bias;
	sampler->min_lod = create_info->min_lod;
	sampler->max_lod = create_info->max_lod;
	sampler->max_anisotropy = create_info->max_anisotropy;
	*samplerp = sampler;
	return  0;
}

int
gjit_sampler_bind(struct jkg_ctx *ctx,
                  struct jkg_sampler **samplers,
                  uint32_t first,
                  uint32_t count)
{
	for (uint32_t i = 0; i < count; ++i)
		ctx->samplers[first + i] = samplers[i];
	return 0;
}

void
gjit_sampler_free(struct jkg_ctx *ctx,
                  struct jkg_sampler *sampler)
{
	(void)ctx;
	gcc_jit_result_release(sampler->res);
	free(sampler);
}

static gcc_jit_function *
jit_wrap(struct jkg_ctx *ctx,
         gcc_jit_context *jit_ctx,
         enum jkg_wrap_mode mode)
{
	static const char *names[] =
	{
		"wrap_repeat",
		"wrap_mirrored_repeat",
		"wrap_clamp_to_edge",
		"wrap_clamp_to_border",
		"wrap_mirror_clamp_to_edge",
	};
	gcc_jit_param *params[] =
	{
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_int32, "max"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_int32, "value"),
	};
	gcc_jit_function *fn = gcc_jit_context_new_function(jit_ctx, NULL, GCC_JIT_FUNCTION_INTERNAL, ctx->jit.type_int32, names[mode], sizeof(params) / sizeof(*params), params, 0);
	gcc_jit_rvalue *max = gcc_jit_param_as_rvalue(params[0]);
	gcc_jit_rvalue *value = gcc_jit_param_as_rvalue(params[1]);
	gcc_jit_block *block = gcc_jit_function_new_block(fn, "main");
	switch (mode)
	{
		case JKG_WRAP_CLAMP_TO_EDGE:
		{
			gcc_jit_block *neg_block = gcc_jit_function_new_block(fn, "neg_block");
			gcc_jit_block_end_with_return(neg_block, NULL, ctx->jit.value_i32[0]);

			gcc_jit_block *max_block = gcc_jit_function_new_block(fn, "max_block");
			gcc_jit_rvalue *max_minus_one = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MINUS, ctx->jit.type_int32, max, ctx->jit.value_i32[1]);
			gcc_jit_block_end_with_return(max_block, NULL, max_minus_one);

			gcc_jit_block *ranged_block = gcc_jit_function_new_block(fn, "ranged_block");
			gcc_jit_block_end_with_return(ranged_block, NULL, value);

			gcc_jit_block *pos_block = gcc_jit_function_new_block(fn, "pos_block");
			gcc_jit_rvalue *max_cmp = gcc_jit_context_new_comparison(jit_ctx, NULL, GCC_JIT_COMPARISON_GE, value, max);
			gcc_jit_block_end_with_conditional(pos_block, NULL, max_cmp, max_block, ranged_block);

			gcc_jit_rvalue *neg_cmp = gcc_jit_context_new_comparison(jit_ctx, NULL, GCC_JIT_COMPARISON_LT, value, ctx->jit.value_i32[0]);
			gcc_jit_block_end_with_conditional(block, NULL, neg_cmp, neg_block, pos_block);
			break;
		}
		case JKG_WRAP_CLAMP_TO_BORDER:
		{
			gcc_jit_block *neg_block = gcc_jit_function_new_block(fn, "neg_block");
			gcc_jit_block_end_with_return(neg_block, NULL, ctx->jit.value_minus_one);

			gcc_jit_block *max_block = gcc_jit_function_new_block(fn, "max_block");
			gcc_jit_block_end_with_return(max_block, NULL, ctx->jit.value_minus_one);

			gcc_jit_block *ranged_block = gcc_jit_function_new_block(fn, "ranged_block");
			gcc_jit_block_end_with_return(ranged_block, NULL, value);

			gcc_jit_block *pos_block = gcc_jit_function_new_block(fn, "pos_block");
			gcc_jit_rvalue *max_cmp = gcc_jit_context_new_comparison(jit_ctx, NULL, GCC_JIT_COMPARISON_GE, value, max);
			gcc_jit_block_end_with_conditional(pos_block, NULL, max_cmp, max_block, ranged_block);

			gcc_jit_rvalue *neg_cmp = gcc_jit_context_new_comparison(jit_ctx, NULL, GCC_JIT_COMPARISON_LT, value, ctx->jit.value_i32[0]);
			gcc_jit_block_end_with_conditional(block, NULL, neg_cmp, neg_block, pos_block);
			break;
		}
		case JKG_WRAP_REPEAT:
		{
			gcc_jit_lvalue *value_mod_max = gcc_jit_function_new_local(fn, NULL, ctx->jit.type_int32, "value_mod_max");
			gcc_jit_block_add_assignment(block, NULL, value_mod_max, gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MODULO, ctx->jit.type_int32, value, max));

			gcc_jit_block *pos_block = gcc_jit_function_new_block(fn, "pos_block");
			gcc_jit_block_end_with_return(pos_block, NULL, gcc_jit_lvalue_as_rvalue(value_mod_max));

			gcc_jit_block *neg_block = gcc_jit_function_new_block(fn, "neg_block");
			gcc_jit_rvalue *mod_positive = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_PLUS, ctx->jit.type_int32, gcc_jit_lvalue_as_rvalue(value_mod_max), max);
			gcc_jit_block_end_with_return(neg_block, NULL, mod_positive);

			gcc_jit_rvalue *neg_cmp = gcc_jit_context_new_comparison(jit_ctx, NULL, GCC_JIT_COMPARISON_LT, gcc_jit_lvalue_as_rvalue(value_mod_max), ctx->jit.value_i32[0]);
			gcc_jit_block_end_with_conditional(block, NULL, neg_cmp, neg_block, pos_block);
			break;
		}
		case JKG_WRAP_MIRRORED_REPEAT:
		{
			gcc_jit_rvalue *max2 = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_PLUS, ctx->jit.type_int32, max, ctx->jit.value_i32[2]);
			gcc_jit_rvalue *value_mod_max2 = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MODULO, ctx->jit.type_int32, value, max2);
			gcc_jit_rvalue *neg_max = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MINUS, ctx->jit.type_int32, ctx->jit.value_i32[0], max);
			gcc_jit_rvalue *neg_value = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MINUS, ctx->jit.type_int32, ctx->jit.value_i32[0], value);

			gcc_jit_block *min_block = gcc_jit_function_new_block(fn, "min_block");
			gcc_jit_rvalue *neg_value_minus_max = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MINUS, ctx->jit.type_int32, neg_value, max);
			gcc_jit_block_end_with_return(min_block, NULL, neg_value_minus_max);

			gcc_jit_block *neg_block = gcc_jit_function_new_block(fn, "neg_block");
			gcc_jit_block_end_with_return(neg_block, NULL, neg_value);

			gcc_jit_block *max_block = gcc_jit_function_new_block(fn, "max_block");
			gcc_jit_rvalue *max_minus_one = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MODULO, ctx->jit.type_int32, max, ctx->jit.value_i32[1]);
			gcc_jit_rvalue *value_minus_max = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MINUS, ctx->jit.type_int32, value, max);
			gcc_jit_rvalue *ret = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MINUS, ctx->jit.type_int32, max_minus_one, value_minus_max);
			gcc_jit_block_end_with_return(max_block, NULL, ret);

			gcc_jit_block *ranged_block = gcc_jit_function_new_block(fn, "ranged_block");
			gcc_jit_block_end_with_return(ranged_block, NULL, value);

			gcc_jit_block *pos_block = gcc_jit_function_new_block(fn, "pos_block");
			gcc_jit_rvalue *max_cmp = gcc_jit_context_new_comparison(jit_ctx, NULL, GCC_JIT_COMPARISON_GE, value, max);
			gcc_jit_block_end_with_conditional(pos_block, NULL, max_cmp, max_block, ranged_block);

			gcc_jit_block *neg_cmp_block = gcc_jit_function_new_block(fn, "neg_cmp_block");
			gcc_jit_rvalue *neg_cmp = gcc_jit_context_new_comparison(jit_ctx, NULL, GCC_JIT_COMPARISON_LT, value, ctx->jit.value_i32[0]);
			gcc_jit_block_end_with_conditional(neg_cmp_block, NULL, neg_cmp, neg_block, pos_block);

			gcc_jit_rvalue *min_cmp = gcc_jit_context_new_comparison(jit_ctx, NULL, GCC_JIT_COMPARISON_LE, value_mod_max2, neg_max);
			gcc_jit_block_end_with_conditional(block, NULL, min_cmp, min_block, neg_cmp_block);
			break;
		}
		case JKG_WRAP_MIRROR_CLAMP_TO_EDGE:
		{
			gcc_jit_rvalue *neg_max = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MINUS, ctx->jit.type_int32, ctx->jit.value_i32[0], max);
			gcc_jit_rvalue *max_minus_one = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MINUS, ctx->jit.type_int32, max, ctx->jit.value_i32[1]);

			gcc_jit_block *min_block = gcc_jit_function_new_block(fn, "min_block");
			gcc_jit_block_end_with_return(min_block, NULL, max_minus_one);

			gcc_jit_block *neg_block = gcc_jit_function_new_block(fn, "neg_block");
			gcc_jit_rvalue *neg_value = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MINUS, ctx->jit.type_int32, ctx->jit.value_i32[0], value);
			gcc_jit_block_end_with_return(neg_block, NULL, neg_value);

			gcc_jit_block *max_block = gcc_jit_function_new_block(fn, "max_block");
			gcc_jit_block_end_with_return(max_block, NULL, max_minus_one);

			gcc_jit_block *ranged_block = gcc_jit_function_new_block(fn, "ranged_block");
			gcc_jit_block_end_with_return(ranged_block, NULL, value);

			gcc_jit_block *pos_block = gcc_jit_function_new_block(fn, "pos_block");
			gcc_jit_rvalue *max_cmp = gcc_jit_context_new_comparison(jit_ctx, NULL, GCC_JIT_COMPARISON_GE, value, max);
			gcc_jit_block_end_with_conditional(pos_block, NULL, max_cmp, max_block, ranged_block);

			gcc_jit_block *test_neg_block = gcc_jit_function_new_block(fn, "test_neg_block");
			gcc_jit_rvalue *neg_cmp = gcc_jit_context_new_comparison(jit_ctx, NULL, GCC_JIT_COMPARISON_LT, value, ctx->jit.value_i32[0]);
			gcc_jit_block_end_with_conditional(test_neg_block, NULL, neg_cmp, neg_block, pos_block);

			gcc_jit_rvalue *neg_max_cmp = gcc_jit_context_new_comparison(jit_ctx, NULL, GCC_JIT_COMPARISON_LE, value, neg_max);
			gcc_jit_block_end_with_conditional(block, NULL, neg_max_cmp, min_block, test_neg_block);
			break;
		}
		default:
			assert(!"unknown wrap mode");
			break;
	}
	return fn;
}

static gcc_jit_function *
jit_get_filter_coord(struct jkg_ctx *ctx,
                     gcc_jit_context *jit_ctx)
{
	gcc_jit_param *params[] =
	{
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_float, "value"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_int32p, "blockp"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_floatp, "fractp"),
	};
	gcc_jit_function *fn = gcc_jit_context_new_function(jit_ctx, NULL, GCC_JIT_FUNCTION_INTERNAL, ctx->jit.type_void, "get_filter_coord", sizeof(params) / sizeof(*params), params, 0);
	gcc_jit_rvalue *value = gcc_jit_param_as_rvalue(params[0]);
	gcc_jit_rvalue *blockp = gcc_jit_param_as_rvalue(params[1]);
	gcc_jit_rvalue *fractp = gcc_jit_param_as_rvalue(params[2]);
	gcc_jit_block *block = gcc_jit_function_new_block(fn, "main");
	gcc_jit_rvalue *half = gcc_jit_context_new_rvalue_from_double(jit_ctx, ctx->jit.type_float, 0.5);
	gcc_jit_function *builtin_fmodf = gcc_jit_context_get_builtin_function(jit_ctx, "__builtin_fmodf");

	gcc_jit_rvalue *value_minus_half = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MINUS, ctx->jit.type_float, value, half);
	gcc_jit_rvalue *fmodf_params[] =
	{
		value_minus_half,
		ctx->jit.value_f[1],
	};
	gcc_jit_rvalue *fract_rvalue = gcc_jit_context_new_call(jit_ctx, NULL, builtin_fmodf, sizeof(fmodf_params) / sizeof(*fmodf_params), fmodf_params);
	gcc_jit_lvalue *fract_lvalue = gcc_jit_context_new_array_access(jit_ctx, NULL, fractp, ctx->jit.value_i32[0]);
	gcc_jit_block_add_assignment(block, NULL, fract_lvalue, fract_rvalue);
	gcc_jit_rvalue *int_rvalue = gcc_jit_context_new_cast(jit_ctx, NULL, value_minus_half, ctx->jit.type_int32);
	gcc_jit_lvalue *int_lvalue = gcc_jit_context_new_array_access(jit_ctx, NULL, blockp, ctx->jit.value_i32[0]);
	gcc_jit_block_add_assignment(block, NULL, int_lvalue, int_rvalue);

	gcc_jit_block *pos_block = gcc_jit_function_new_block(fn, "pos_block");
	gcc_jit_block_end_with_void_return(pos_block, NULL);

	gcc_jit_block *neg_block = gcc_jit_function_new_block(fn, "neg_block");
	gcc_jit_block_add_assignment_op(neg_block, NULL, fract_lvalue, GCC_JIT_BINARY_OP_PLUS, ctx->jit.value_f[1]);
	gcc_jit_block_add_assignment_op(neg_block, NULL, int_lvalue, GCC_JIT_BINARY_OP_MINUS, ctx->jit.value_i32[1]);
	gcc_jit_block_end_with_void_return(neg_block, NULL);

	gcc_jit_rvalue *neg_cmp = gcc_jit_context_new_comparison(jit_ctx, NULL, GCC_JIT_COMPARISON_LT, gcc_jit_lvalue_as_rvalue(fract_lvalue), ctx->jit.value_f[0]);
	gcc_jit_block_end_with_conditional(block, NULL, neg_cmp, neg_block, pos_block);
	return fn;
}

static gcc_jit_function *
jit_linear_interpolate(struct jkg_ctx *ctx,
                       gcc_jit_context *jit_ctx)
{
	gcc_jit_param *params[] =
	{
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_restrict_floatp, "res"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_const_restrict_floatp, "c0"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_const_restrict_floatp, "c1"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_float, "f"),
	};
	gcc_jit_function *fn = gcc_jit_context_new_function(jit_ctx, NULL, GCC_JIT_FUNCTION_INTERNAL, ctx->jit.type_void, "linear_interpolate", sizeof(params) / sizeof(*params), params, 0);
	gcc_jit_rvalue *res = gcc_jit_param_as_rvalue(params[0]);
	gcc_jit_rvalue *c0 = gcc_jit_param_as_rvalue(params[1]);
	gcc_jit_rvalue *c1 = gcc_jit_param_as_rvalue(params[2]);
	gcc_jit_rvalue *f = gcc_jit_param_as_rvalue(params[3]);
	gcc_jit_block *block = gcc_jit_function_new_block(fn, "main");
	gcc_jit_block *end_block = gcc_jit_function_new_block(fn, "end_block");
	gcc_jit_block *comp_block = gcc_jit_function_new_block(fn, "comp_block");

	gcc_jit_lvalue *color = gcc_jit_function_new_local(fn, NULL, ctx->jit.type_float_4, "color");
	gcc_jit_lvalue *i = gcc_jit_function_new_local(fn, NULL, ctx->jit.type_int32, "i");

	gcc_jit_block_add_assignment(block, NULL, i, ctx->jit.value_i32[0]);
	gcc_jit_block_end_with_jump(block, NULL, comp_block);

	gcc_jit_rvalue *c0_indexed = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(jit_ctx, NULL, c0, gcc_jit_lvalue_as_rvalue(i)));
	gcc_jit_rvalue *c1_indexed = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(jit_ctx, NULL, c1, gcc_jit_lvalue_as_rvalue(i)));
	gcc_jit_rvalue *diff = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MINUS, ctx->jit.type_float, c1_indexed, c0_indexed);
	gcc_jit_rvalue *diff_f = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MULT, ctx->jit.type_float, diff, f);
	gcc_jit_rvalue *rvalue = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_PLUS, ctx->jit.type_float, c0_indexed, diff_f);
	gcc_jit_lvalue *lvalue = gcc_jit_context_new_array_access(jit_ctx, NULL, gcc_jit_lvalue_as_rvalue(color), gcc_jit_lvalue_as_rvalue(i));
	gcc_jit_block_add_assignment(comp_block, NULL, lvalue, rvalue);
	gcc_jit_block_add_assignment_op(comp_block, NULL, i, GCC_JIT_BINARY_OP_PLUS, ctx->jit.value_i32[1]);
	gcc_jit_rvalue *end_cmp = gcc_jit_context_new_comparison(jit_ctx, NULL, GCC_JIT_COMPARISON_LE, gcc_jit_lvalue_as_rvalue(i), ctx->jit.value_i32[3]);
	gcc_jit_block_end_with_conditional(comp_block, NULL, end_cmp, comp_block, end_block);

	for (size_t i = 0; i < 4; ++i)
	{
		gcc_jit_lvalue *lvalue = gcc_jit_context_new_array_access(jit_ctx, NULL, res, ctx->jit.value_i32[i]);
		gcc_jit_lvalue *rvalue = gcc_jit_context_new_array_access(jit_ctx, NULL, gcc_jit_lvalue_as_rvalue(color), ctx->jit.value_i32[i]);
		gcc_jit_block_add_assignment(end_block, NULL, lvalue, gcc_jit_lvalue_as_rvalue(rvalue));
	}
	gcc_jit_block_end_with_void_return(end_block, NULL);
	return fn;
}

static gcc_jit_function *
jit_cubic_interpolate(struct jkg_ctx *ctx,
                      gcc_jit_context *jit_ctx)
{
	gcc_jit_param *params[] =
	{
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_restrict_floatp, "res"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_const_restrict_floatp, "c0"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_const_restrict_floatp, "c1"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_const_restrict_floatp, "c2"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_const_restrict_floatp, "c3"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_float, "f"),
	};
	gcc_jit_function *fn = gcc_jit_context_new_function(jit_ctx, NULL, GCC_JIT_FUNCTION_INTERNAL, ctx->jit.type_void, "cubic_interpolate", sizeof(params) / sizeof(*params), params, 0);
	gcc_jit_rvalue *res = gcc_jit_param_as_rvalue(params[0]);
	gcc_jit_rvalue *c0 = gcc_jit_param_as_rvalue(params[1]);
	gcc_jit_rvalue *c1 = gcc_jit_param_as_rvalue(params[2]);
	gcc_jit_rvalue *c2 = gcc_jit_param_as_rvalue(params[3]);
	gcc_jit_rvalue *c3 = gcc_jit_param_as_rvalue(params[4]);
	gcc_jit_rvalue *f = gcc_jit_param_as_rvalue(params[5]);
	gcc_jit_block *block = gcc_jit_function_new_block(fn, "main");
	gcc_jit_block *end_block = gcc_jit_function_new_block(fn, "end_block");
	gcc_jit_block *comp_block = gcc_jit_function_new_block(fn, "comp_block");
	gcc_jit_lvalue *p0 = gcc_jit_function_new_local(fn, NULL, ctx->jit.type_float, "p0");
	gcc_jit_lvalue *p1 = gcc_jit_function_new_local(fn, NULL, ctx->jit.type_float, "p1");
	gcc_jit_lvalue *m0 = gcc_jit_function_new_local(fn, NULL, ctx->jit.type_float, "m0");
	gcc_jit_lvalue *m1 = gcc_jit_function_new_local(fn, NULL, ctx->jit.type_float, "m1");
	gcc_jit_lvalue *color = gcc_jit_function_new_local(fn, NULL, ctx->jit.type_float_4, "color");
	gcc_jit_lvalue *i = gcc_jit_function_new_local(fn, NULL, ctx->jit.type_int32, "i");

	gcc_jit_block_add_assignment(block, NULL, i, ctx->jit.value_i32[0]);
	gcc_jit_block_end_with_jump(block, NULL, comp_block);

	gcc_jit_rvalue *c0_indexed = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(jit_ctx, NULL, c0, gcc_jit_lvalue_as_rvalue(i)));
	gcc_jit_rvalue *c1_indexed = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(jit_ctx, NULL, c1, gcc_jit_lvalue_as_rvalue(i)));
	gcc_jit_rvalue *c2_indexed = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(jit_ctx, NULL, c2, gcc_jit_lvalue_as_rvalue(i)));
	gcc_jit_rvalue *c3_indexed = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(jit_ctx, NULL, c3, gcc_jit_lvalue_as_rvalue(i)));
	gcc_jit_block_add_assignment(comp_block, NULL, p0, c1_indexed);
	gcc_jit_block_add_assignment(comp_block, NULL, p1, c2_indexed);
	gcc_jit_block_add_assignment(comp_block, NULL, m0, gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MINUS, ctx->jit.type_float, gcc_jit_lvalue_as_rvalue(p0), c0_indexed));
	gcc_jit_block_add_assignment(comp_block, NULL, m1, gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MINUS, ctx->jit.type_float, c3_indexed, gcc_jit_lvalue_as_rvalue(p1)));
	gcc_jit_rvalue *p0_2 = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MULT, ctx->jit.type_float, ctx->jit.value_f[2], gcc_jit_lvalue_as_rvalue(p0));
	gcc_jit_rvalue *p0_3 = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MULT, ctx->jit.type_float, ctx->jit.value_f[3], gcc_jit_lvalue_as_rvalue(p0));
	gcc_jit_rvalue *p1_2 = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MULT, ctx->jit.type_float, ctx->jit.value_f[2], gcc_jit_lvalue_as_rvalue(p1));
	gcc_jit_rvalue *p1_3 = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MULT, ctx->jit.type_float, ctx->jit.value_f[3], gcc_jit_lvalue_as_rvalue(p1));
	gcc_jit_rvalue *m0_2 = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MULT, ctx->jit.type_float, ctx->jit.value_f[2], gcc_jit_lvalue_as_rvalue(m0));

	gcc_jit_rvalue *f3;
	f3 = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_PLUS, ctx->jit.type_float, p0_2, gcc_jit_lvalue_as_rvalue(m0));
	f3 = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MINUS, ctx->jit.type_float, f3, p1_2);
	f3 = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_PLUS, ctx->jit.type_float, f3, gcc_jit_lvalue_as_rvalue(m1));

	gcc_jit_rvalue *f2;
	f2 = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MULT, ctx->jit.type_float, f3, f);
	f2 = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MINUS, ctx->jit.type_float, f2, p0_3);
	f2 = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MINUS, ctx->jit.type_float, f2, m0_2);
	f2 = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_PLUS, ctx->jit.type_float, f2, p1_3);
	f2 = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MINUS, ctx->jit.type_float, f2, gcc_jit_lvalue_as_rvalue(m1));

	gcc_jit_rvalue *f1;
	f1 = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MULT, ctx->jit.type_float, f2, f);
	f1 = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_PLUS, ctx->jit.type_float, f1, gcc_jit_lvalue_as_rvalue(m0));

	gcc_jit_rvalue *f0;
	f0 = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MULT, ctx->jit.type_float, f1, f);
	f0 = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_PLUS, ctx->jit.type_float, f0, gcc_jit_lvalue_as_rvalue(p0));

	gcc_jit_lvalue *lvalue = gcc_jit_context_new_array_access(jit_ctx, NULL, gcc_jit_lvalue_as_rvalue(color), gcc_jit_lvalue_as_rvalue(i));
	gcc_jit_block_add_assignment(comp_block, NULL, lvalue, f0);
	gcc_jit_block_add_assignment_op(comp_block, NULL, i, GCC_JIT_BINARY_OP_PLUS, ctx->jit.value_i32[1]);
	gcc_jit_rvalue *end_cmp = gcc_jit_context_new_comparison(jit_ctx, NULL, GCC_JIT_COMPARISON_LE, gcc_jit_lvalue_as_rvalue(i), ctx->jit.value_i32[3]);
	gcc_jit_block_end_with_conditional(comp_block, NULL, end_cmp, comp_block, end_block);

	for (size_t i = 0; i < 4; ++i)
	{
		gcc_jit_lvalue *lvalue = gcc_jit_context_new_array_access(jit_ctx, NULL, res, ctx->jit.value_i32[i]);
		gcc_jit_lvalue *rvalue = gcc_jit_context_new_array_access(jit_ctx, NULL, gcc_jit_lvalue_as_rvalue(color), ctx->jit.value_i32[i]);
		gcc_jit_block_add_assignment(end_block, NULL, lvalue, gcc_jit_lvalue_as_rvalue(rvalue));
	}
	gcc_jit_block_end_with_void_return(end_block, NULL);
	return fn;
}

static gcc_jit_function *
jit_fetch(struct jkg_ctx *ctx,
          gcc_jit_context *jit_ctx)
{
	gcc_jit_param *params[] =
	{
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_floatp, "color"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_int32p, "coords"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_const_int32p, "size"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_image_view_fn, "image_view_fn"),
	};
	gcc_jit_function *fn = gcc_jit_context_new_function(jit_ctx, NULL, GCC_JIT_FUNCTION_INTERNAL, ctx->jit.type_void, "fetch", sizeof(params) / sizeof(*params), params, 0);
	gcc_jit_rvalue *color = gcc_jit_param_as_rvalue(params[0]);
	gcc_jit_rvalue *coords = gcc_jit_param_as_rvalue(params[1]);
	gcc_jit_rvalue *size = gcc_jit_param_as_rvalue(params[2]);
	(void)size;
	gcc_jit_rvalue *image_view_fn = gcc_jit_param_as_rvalue(params[3]);
	gcc_jit_block *block = gcc_jit_function_new_block(fn, "main");

	gcc_jit_rvalue *texel_params[] =
	{
		color,
		gcc_jit_lvalue_get_address(gcc_jit_context_new_array_access(jit_ctx, NULL, coords, ctx->jit.value_i32[0]), NULL),
	};
	gcc_jit_block_add_eval(block, NULL, gcc_jit_context_new_call_through_ptr(jit_ctx, NULL, image_view_fn, sizeof(texel_params) / sizeof(*texel_params), texel_params));
	gcc_jit_block_end_with_void_return(block, NULL);
	return fn;
}

static gcc_jit_function *
jit_nearest(struct jkg_ctx *ctx,
            gcc_jit_context *jit_ctx,
            size_t comp)
{
	const char *names[] =
	{
		"nearest_x",
		"nearest_y",
		"nearest_z",
	};
	gcc_jit_param *params[] =
	{
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_floatp, "color"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_int32p, "fetch_coords"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_const_int32p, "wrapped_coords"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_const_floatp, "coord_fract"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_const_floatp, "border_color"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_const_int32p, "size"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_image_view_fn, "image_view_fn"),
	};
	gcc_jit_function *fn = gcc_jit_context_new_function(jit_ctx, NULL, GCC_JIT_FUNCTION_INTERNAL, ctx->jit.type_void, names[comp], sizeof(params) / sizeof(*params), params, 0);
	gcc_jit_rvalue *color = gcc_jit_param_as_rvalue(params[0]);
	gcc_jit_rvalue *fetch_coords = gcc_jit_param_as_rvalue(params[1]);
	gcc_jit_rvalue *wrapped_coords = gcc_jit_param_as_rvalue(params[2]);
	gcc_jit_rvalue *coord_fract = gcc_jit_param_as_rvalue(params[3]);
	gcc_jit_rvalue *border_color = gcc_jit_param_as_rvalue(params[4]);
	gcc_jit_rvalue *size = gcc_jit_param_as_rvalue(params[5]);
	gcc_jit_rvalue *image_view_fn = gcc_jit_param_as_rvalue(params[6]);
	gcc_jit_block *block = gcc_jit_function_new_block(fn, "main");
	gcc_jit_lvalue *lvalue = gcc_jit_context_new_array_access(jit_ctx, NULL, fetch_coords, ctx->jit.value_i32[comp]);
	gcc_jit_lvalue *rvalue = gcc_jit_context_new_array_access(jit_ctx, NULL, wrapped_coords, gcc_jit_context_new_rvalue_from_int(ctx->jit.ctx, ctx->jit.type_int32, comp));

	gcc_jit_block *fetch_block = gcc_jit_function_new_block(fn, "fetch_block");
	gcc_jit_block_add_assignment(fetch_block, NULL, lvalue, gcc_jit_lvalue_as_rvalue(rvalue));
	if (comp)
	{
		gcc_jit_rvalue *fetch_params[] =
		{
			color,
			fetch_coords,
			wrapped_coords,
			coord_fract,
			border_color,
			size,
			image_view_fn,
		};
		gcc_jit_block_add_eval(fetch_block, NULL, gcc_jit_context_new_call(jit_ctx, NULL, ctx->jit.sampler.nearest[comp - 1], sizeof(fetch_params) / sizeof(*fetch_params), fetch_params));
	}
	else
	{
		gcc_jit_rvalue *fetch_params[] =
		{
			color,
			gcc_jit_lvalue_get_address(gcc_jit_context_new_array_access(jit_ctx, NULL, fetch_coords, ctx->jit.value_i32[0]), NULL),
			size,
			image_view_fn,
		};
		gcc_jit_block_add_eval(fetch_block, NULL, gcc_jit_context_new_call(jit_ctx, NULL, ctx->jit.sampler.fetch, sizeof(fetch_params) / sizeof(*fetch_params), fetch_params));
	}
	gcc_jit_block_end_with_void_return(fetch_block, NULL);

	gcc_jit_block *border_block = gcc_jit_function_new_block(fn, "border_block");
	for (size_t i = 0; i < 4; ++i)
	{
		gcc_jit_lvalue *border_indexed = gcc_jit_context_new_array_access(jit_ctx, NULL, border_color, ctx->jit.value_i32[i]);
		gcc_jit_lvalue *color_indexed = gcc_jit_context_new_array_access(jit_ctx, NULL, color, ctx->jit.value_i32[i]);
		gcc_jit_block_add_assignment(border_block, NULL, color_indexed, gcc_jit_lvalue_as_rvalue(border_indexed));
	}
	gcc_jit_block_end_with_void_return(border_block, NULL);

	gcc_jit_rvalue *neg_cmp = gcc_jit_context_new_comparison(jit_ctx, NULL, GCC_JIT_COMPARISON_LT, gcc_jit_lvalue_as_rvalue(rvalue), ctx->jit.value_i32[0]);
	gcc_jit_block_end_with_conditional(block, NULL, neg_cmp, border_block, fetch_block);
	return fn;
}

static gcc_jit_function *
jit_linear(struct jkg_ctx *ctx,
           gcc_jit_context *jit_ctx,
           size_t comp)
{
	const char *names[] =
	{
		"linear_x",
		"linear_y",
		"linear_z",
	};
	gcc_jit_param *params[] =
	{
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_floatp, "color"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_int32p, "fetch_coords"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_const_int32p, "wrapped_coords"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_const_floatp, "coord_fract"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_const_floatp, "border_color"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_const_int32p, "size"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_image_view_fn, "image_view_fn"),
	};
	gcc_jit_function *fn = gcc_jit_context_new_function(jit_ctx, NULL, GCC_JIT_FUNCTION_INTERNAL, ctx->jit.type_void, names[comp], sizeof(params) / sizeof(*params), params, 0);
	gcc_jit_rvalue *color = gcc_jit_param_as_rvalue(params[0]);
	gcc_jit_rvalue *fetch_coords = gcc_jit_param_as_rvalue(params[1]);
	gcc_jit_rvalue *wrapped_coords = gcc_jit_param_as_rvalue(params[2]);
	gcc_jit_rvalue *coord_fract = gcc_jit_param_as_rvalue(params[3]);
	gcc_jit_rvalue *border_color = gcc_jit_param_as_rvalue(params[4]);
	gcc_jit_rvalue *size = gcc_jit_param_as_rvalue(params[5]);
	gcc_jit_rvalue *image_view_fn = gcc_jit_param_as_rvalue(params[6]);
	gcc_jit_block *block = gcc_jit_function_new_block(fn, "main");
	gcc_jit_block *comp_block = gcc_jit_function_new_block(fn, "comp_block");
	gcc_jit_block *cond_block = gcc_jit_function_new_block(fn, "cond_block");
	gcc_jit_block *end_block = gcc_jit_function_new_block(fn, "end_block");
	gcc_jit_block *test_block = gcc_jit_function_new_block(fn, "test_block");
	gcc_jit_block *border_block = gcc_jit_function_new_block(fn, "border_block");
	gcc_jit_lvalue *tmp = gcc_jit_function_new_local(fn, NULL, ctx->jit.type_float_8, "tmp");
	gcc_jit_lvalue *i = gcc_jit_function_new_local(fn, NULL, ctx->jit.type_int32, "i");

	gcc_jit_block_add_assignment(block, NULL, i, ctx->jit.value_i32[0]);
	gcc_jit_block_end_with_jump(block, NULL, test_block);

	gcc_jit_rvalue *index = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_PLUS, ctx->jit.type_int32, gcc_jit_lvalue_as_rvalue(i), gcc_jit_context_new_rvalue_from_int(ctx->jit.ctx, ctx->jit.type_int32, comp * 2));
	gcc_jit_lvalue *rvalue = gcc_jit_context_new_array_access(jit_ctx, NULL, wrapped_coords, index);

	gcc_jit_rvalue *border_cmp = gcc_jit_context_new_comparison(jit_ctx, NULL, GCC_JIT_COMPARISON_LT, gcc_jit_lvalue_as_rvalue(rvalue), ctx->jit.value_i32[0]);
	gcc_jit_block_end_with_conditional(test_block, NULL, border_cmp, border_block, comp_block);

	for (size_t i = 0; i < 4; ++i)
	{
		gcc_jit_lvalue *border_indexed = gcc_jit_context_new_array_access(jit_ctx, NULL, border_color, ctx->jit.value_i32[i]);
		gcc_jit_lvalue *color_indexed = gcc_jit_context_new_array_access(jit_ctx, NULL, color, ctx->jit.value_i32[i]);
		gcc_jit_block_add_assignment(border_block, NULL, color_indexed, gcc_jit_lvalue_as_rvalue(border_indexed));
	}
	gcc_jit_block_end_with_jump(border_block, NULL, cond_block);

	gcc_jit_lvalue *lvalue = gcc_jit_context_new_array_access(jit_ctx, NULL, fetch_coords, ctx->jit.value_i32[comp]);
	gcc_jit_block_add_assignment(comp_block, NULL, lvalue, gcc_jit_lvalue_as_rvalue(rvalue));
	gcc_jit_rvalue *tmp_index = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MULT, ctx->jit.type_int32, gcc_jit_lvalue_as_rvalue(i), ctx->jit.value_i32[4]);
	if (comp)
	{
		gcc_jit_rvalue *fetch_params[] =
		{
			gcc_jit_lvalue_get_address(gcc_jit_context_new_array_access(jit_ctx, NULL, gcc_jit_lvalue_as_rvalue(tmp), tmp_index), NULL),
			fetch_coords,
			wrapped_coords,
			coord_fract,
			border_color,
			size,
			image_view_fn,
		};
		gcc_jit_block_add_eval(comp_block, NULL, gcc_jit_context_new_call(jit_ctx, NULL, ctx->jit.sampler.linear[comp - 1], sizeof(fetch_params) / sizeof(*fetch_params), fetch_params));
	}
	else
	{
		gcc_jit_rvalue *fetch_params[] =
		{
			gcc_jit_lvalue_get_address(gcc_jit_context_new_array_access(jit_ctx, NULL, gcc_jit_lvalue_as_rvalue(tmp), tmp_index), NULL),
			gcc_jit_lvalue_get_address(gcc_jit_context_new_array_access(jit_ctx, NULL, fetch_coords, ctx->jit.value_i32[0]), NULL),
			size,
			image_view_fn,
		};
		gcc_jit_block_add_eval(comp_block, NULL, gcc_jit_context_new_call(jit_ctx, NULL, ctx->jit.sampler.fetch, sizeof(fetch_params) / sizeof(*fetch_params), fetch_params));
	}
	gcc_jit_block_end_with_jump(comp_block, NULL, cond_block);

	gcc_jit_block_add_assignment_op(cond_block, NULL, i, GCC_JIT_BINARY_OP_PLUS, ctx->jit.value_i32[1]);
	gcc_jit_rvalue *end_cmp = gcc_jit_context_new_comparison(jit_ctx, NULL, GCC_JIT_COMPARISON_LT, gcc_jit_lvalue_as_rvalue(i), ctx->jit.value_i32[2]);
	gcc_jit_block_end_with_conditional(cond_block, NULL, end_cmp, comp_block, end_block);

	gcc_jit_rvalue *interp_params[] =
	{
		color,
		gcc_jit_lvalue_get_address(gcc_jit_context_new_array_access(jit_ctx, NULL, gcc_jit_lvalue_as_rvalue(tmp), ctx->jit.value_i32[0]), NULL),
		gcc_jit_lvalue_get_address(gcc_jit_context_new_array_access(jit_ctx, NULL, gcc_jit_lvalue_as_rvalue(tmp), ctx->jit.value_i32[4]), NULL),
		gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(jit_ctx, NULL, coord_fract, ctx->jit.value_i32[comp])),
	};
	gcc_jit_block_add_eval(end_block, NULL, gcc_jit_context_new_call(jit_ctx, NULL, ctx->jit.sampler.linear_interpolate, sizeof(interp_params) / sizeof(*interp_params), interp_params));
	gcc_jit_block_end_with_void_return(end_block, NULL);
	return fn;
}

static gcc_jit_function *
jit_cubic(struct jkg_ctx *ctx,
          gcc_jit_context *jit_ctx,
          size_t comp)
{
	const char *names[] =
	{
		"cubic_x",
		"cubic_y",
		"cubic_z",
	};
	gcc_jit_param *params[] =
	{
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_floatp, "color"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_int32p, "fetch_coords"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_const_int32p, "wrapped_coords"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_const_floatp, "coord_fract"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_const_floatp, "border_color"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_const_int32p, "size"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_image_view_fn, "image_view_fn"),
	};
	gcc_jit_function *fn = gcc_jit_context_new_function(jit_ctx, NULL, GCC_JIT_FUNCTION_INTERNAL, ctx->jit.type_void, names[comp], sizeof(params) / sizeof(*params), params, 0);
	gcc_jit_rvalue *color = gcc_jit_param_as_rvalue(params[0]);
	gcc_jit_rvalue *fetch_coords = gcc_jit_param_as_rvalue(params[1]);
	gcc_jit_rvalue *wrapped_coords = gcc_jit_param_as_rvalue(params[2]);
	gcc_jit_rvalue *coord_fract = gcc_jit_param_as_rvalue(params[3]);
	gcc_jit_rvalue *border_color = gcc_jit_param_as_rvalue(params[4]);
	gcc_jit_rvalue *size = gcc_jit_param_as_rvalue(params[5]);
	gcc_jit_rvalue *image_view_fn = gcc_jit_param_as_rvalue(params[6]);
	gcc_jit_block *block = gcc_jit_function_new_block(fn, "main");
	gcc_jit_block *comp_block = gcc_jit_function_new_block(fn, "comp_block");
	gcc_jit_block *cond_block = gcc_jit_function_new_block(fn, "cond_block");
	gcc_jit_block *end_block = gcc_jit_function_new_block(fn, "end_block");
	gcc_jit_block *test_block = gcc_jit_function_new_block(fn, "test_block");
	gcc_jit_block *border_block = gcc_jit_function_new_block(fn, "border_block");
	gcc_jit_lvalue *tmp = gcc_jit_function_new_local(fn, NULL, ctx->jit.type_float_16, "tmp");
	gcc_jit_lvalue *i = gcc_jit_function_new_local(fn, NULL, ctx->jit.type_int32, "i");

	gcc_jit_block_add_assignment(block, NULL, i, ctx->jit.value_i32[0]);
	gcc_jit_block_end_with_jump(block, NULL, test_block);

	gcc_jit_rvalue *index = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_PLUS, ctx->jit.type_int32, gcc_jit_lvalue_as_rvalue(i), gcc_jit_context_new_rvalue_from_int(ctx->jit.ctx, ctx->jit.type_int32, comp * 4));
	gcc_jit_lvalue *rvalue = gcc_jit_context_new_array_access(jit_ctx, NULL, wrapped_coords, index);

	gcc_jit_rvalue *border_cmp = gcc_jit_context_new_comparison(jit_ctx, NULL, GCC_JIT_COMPARISON_LT, gcc_jit_lvalue_as_rvalue(rvalue), ctx->jit.value_i32[0]);
	gcc_jit_block_end_with_conditional(test_block, NULL, border_cmp, border_block, comp_block);

	for (size_t i = 0; i < 4; ++i)
	{
		gcc_jit_lvalue *border_indexed = gcc_jit_context_new_array_access(jit_ctx, NULL, border_color, ctx->jit.value_i32[i]);
		gcc_jit_lvalue *color_indexed = gcc_jit_context_new_array_access(jit_ctx, NULL, color, ctx->jit.value_i32[i]);
		gcc_jit_block_add_assignment(border_block, NULL, color_indexed, gcc_jit_lvalue_as_rvalue(border_indexed));
	}
	gcc_jit_block_end_with_jump(border_block, NULL, cond_block);

	gcc_jit_lvalue *lvalue = gcc_jit_context_new_array_access(jit_ctx, NULL, fetch_coords, ctx->jit.value_i32[comp]);
	gcc_jit_block_add_assignment(comp_block, NULL, lvalue, gcc_jit_lvalue_as_rvalue(rvalue));
	gcc_jit_rvalue *tmp_index = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_MULT, ctx->jit.type_int32, gcc_jit_lvalue_as_rvalue(i), ctx->jit.value_i32[4]);
	if (comp)
	{
		gcc_jit_rvalue *fetch_params[] =
		{
			gcc_jit_lvalue_get_address(gcc_jit_context_new_array_access(jit_ctx, NULL, gcc_jit_lvalue_as_rvalue(tmp), tmp_index), NULL),
			fetch_coords,
			wrapped_coords,
			coord_fract,
			border_color,
			size,
			image_view_fn,
		};
		gcc_jit_block_add_eval(comp_block, NULL, gcc_jit_context_new_call(jit_ctx, NULL, ctx->jit.sampler.cubic[comp - 1], sizeof(fetch_params) / sizeof(*fetch_params), fetch_params));
	}
	else
	{
		gcc_jit_rvalue *fetch_params[] =
		{
			gcc_jit_lvalue_get_address(gcc_jit_context_new_array_access(jit_ctx, NULL, gcc_jit_lvalue_as_rvalue(tmp), tmp_index), NULL),
			gcc_jit_lvalue_get_address(gcc_jit_context_new_array_access(jit_ctx, NULL, fetch_coords, ctx->jit.value_i32[0]), NULL),
			size,
			image_view_fn,
		};
		gcc_jit_block_add_eval(comp_block, NULL, gcc_jit_context_new_call(jit_ctx, NULL, ctx->jit.sampler.fetch, sizeof(fetch_params) / sizeof(*fetch_params), fetch_params));
	}
	gcc_jit_block_end_with_jump(comp_block, NULL, cond_block);

	gcc_jit_block_add_assignment_op(cond_block, NULL, i, GCC_JIT_BINARY_OP_PLUS, ctx->jit.value_i32[1]);
	gcc_jit_rvalue *end_cmp = gcc_jit_context_new_comparison(jit_ctx, NULL, GCC_JIT_COMPARISON_LT, gcc_jit_lvalue_as_rvalue(i), ctx->jit.value_i32[4]);
	gcc_jit_block_end_with_conditional(cond_block, NULL, end_cmp, comp_block, end_block);


	gcc_jit_rvalue *interp_params[] =
	{
		color,
		gcc_jit_lvalue_get_address(gcc_jit_context_new_array_access(jit_ctx, NULL, gcc_jit_lvalue_as_rvalue(tmp), gcc_jit_context_new_rvalue_from_int(ctx->jit.ctx, ctx->jit.type_int32, 0)), NULL),
		gcc_jit_lvalue_get_address(gcc_jit_context_new_array_access(jit_ctx, NULL, gcc_jit_lvalue_as_rvalue(tmp), gcc_jit_context_new_rvalue_from_int(ctx->jit.ctx, ctx->jit.type_int32, 4)), NULL),
		gcc_jit_lvalue_get_address(gcc_jit_context_new_array_access(jit_ctx, NULL, gcc_jit_lvalue_as_rvalue(tmp), gcc_jit_context_new_rvalue_from_int(ctx->jit.ctx, ctx->jit.type_int32, 8)), NULL),
		gcc_jit_lvalue_get_address(gcc_jit_context_new_array_access(jit_ctx, NULL, gcc_jit_lvalue_as_rvalue(tmp), gcc_jit_context_new_rvalue_from_int(ctx->jit.ctx, ctx->jit.type_int32, 12)), NULL),
		gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(jit_ctx, NULL, coord_fract, ctx->jit.value_i32[comp])),
	};
	gcc_jit_block_add_eval(end_block, NULL, gcc_jit_context_new_call(jit_ctx, NULL, ctx->jit.sampler.cubic_interpolate, sizeof(interp_params) / sizeof(*interp_params), interp_params));
	gcc_jit_block_end_with_void_return(end_block, NULL);
	return fn;
}

void
gjit_sampler_init(struct jkg_ctx *ctx)
{
	ctx->jit.sampler.ctx = gcc_jit_context_new_child_context(ctx->jit.ctx);
	for (size_t i = 0; i < sizeof(ctx->jit.sampler.wrap) / sizeof(*ctx->jit.sampler.wrap); ++i)
		ctx->jit.sampler.wrap[i] = jit_wrap(ctx, ctx->jit.sampler.ctx, i);
	ctx->jit.sampler.get_filter_coord = jit_get_filter_coord(ctx, ctx->jit.sampler.ctx);
	ctx->jit.sampler.linear_interpolate = jit_linear_interpolate(ctx, ctx->jit.sampler.ctx);
	ctx->jit.sampler.cubic_interpolate = jit_cubic_interpolate(ctx, ctx->jit.sampler.ctx);
	ctx->jit.sampler.fetch = jit_fetch(ctx, ctx->jit.sampler.ctx);
	for (size_t i = 0; i < 3; ++i)
		ctx->jit.sampler.nearest[i] = jit_nearest(ctx, ctx->jit.sampler.ctx, i);
	for (size_t i = 0; i < 3; ++i)
		ctx->jit.sampler.linear[i] = jit_linear(ctx, ctx->jit.sampler.ctx, i);
	for (size_t i = 0; i < 3; ++i)
		ctx->jit.sampler.cubic[i] = jit_cubic(ctx, ctx->jit.sampler.ctx, i);
}

static gcc_jit_function *
jit_wrap_coord(struct jkg_ctx *ctx,
               gcc_jit_context *jit_ctx,
               size_t comp,
               gcc_jit_function *wrap_fn,
               int32_t base,
               int32_t count)
{
	static const char *names[] =
	{
		"wrap_coord_x",
		"wrap_coord_y",
		"wrap_coord_z",
	};
	gcc_jit_param *params[] =
	{
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_int32p, "wrapped"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_int32, "block"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_int32, "max"),
	};
	gcc_jit_function *fn = gcc_jit_context_new_function(jit_ctx, NULL, GCC_JIT_FUNCTION_INTERNAL, ctx->jit.type_void, names[comp], sizeof(params) / sizeof(*params), params, 0);
	gcc_jit_rvalue *wrapped = gcc_jit_param_as_rvalue(params[0]);
	gcc_jit_rvalue *blockv = gcc_jit_param_as_rvalue(params[1]);
	gcc_jit_rvalue *max = gcc_jit_param_as_rvalue(params[2]);
	gcc_jit_block *block = gcc_jit_function_new_block(fn, "main");
	gcc_jit_block *end_block = gcc_jit_function_new_block(fn, "end_block");
	gcc_jit_block *wrap_block = gcc_jit_function_new_block(fn, "wrap_block");
	gcc_jit_lvalue *i = gcc_jit_function_new_local(fn, NULL, ctx->jit.type_int32, "i");
	gcc_jit_rvalue *count_rvalue = gcc_jit_context_new_rvalue_from_int(ctx->jit.ctx, ctx->jit.type_int32, count);
	gcc_jit_rvalue *base_rvalue = gcc_jit_context_new_rvalue_from_int(ctx->jit.ctx, ctx->jit.type_int32, base);
	gcc_jit_block_add_assignment(block, NULL, i, ctx->jit.value_i32[0]);
	gcc_jit_block_end_with_jump(block, NULL, wrap_block);
	gcc_jit_block_end_with_void_return(end_block, NULL);
	gcc_jit_rvalue *block_base = gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_PLUS, ctx->jit.type_int32, blockv, base_rvalue);
	gcc_jit_rvalue *wrap_params[] =
	{
		max,
		gcc_jit_context_new_binary_op(jit_ctx, NULL, GCC_JIT_BINARY_OP_PLUS, ctx->jit.type_int32, block_base, gcc_jit_lvalue_as_rvalue(i)),
	};
	gcc_jit_rvalue *rvalue = gcc_jit_context_new_call(jit_ctx, NULL, wrap_fn, sizeof(wrap_params) / sizeof(*wrap_params), wrap_params);
	gcc_jit_lvalue *lvalue = gcc_jit_context_new_array_access(jit_ctx, NULL, wrapped, gcc_jit_lvalue_as_rvalue(i));
	gcc_jit_block_add_assignment(wrap_block, NULL, lvalue, rvalue);
	gcc_jit_block_add_assignment_op(wrap_block, NULL, i, GCC_JIT_BINARY_OP_PLUS, ctx->jit.value_i32[1]);
	gcc_jit_rvalue *end_cmp = gcc_jit_context_new_comparison(jit_ctx, NULL, GCC_JIT_COMPARISON_LT, gcc_jit_lvalue_as_rvalue(i), count_rvalue);
	gcc_jit_block_end_with_conditional(wrap_block, NULL, end_cmp, wrap_block, end_block);
	return fn;
}

static gcc_jit_function *
jit_filter_nearest(struct jkg_ctx *ctx,
                   gcc_jit_context *jit_ctx,
                   const struct jkg_sampler_create_info *create_info,
                   size_t ncomp)
{
	gcc_jit_param *params[] =
	{
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_floatp, "color"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_const_floatp, "coords"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_const_int32p, "size"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_image_view_fn, "image_view_fn"),
	};
	gcc_jit_function *fn = gcc_jit_context_new_function(jit_ctx, NULL, GCC_JIT_FUNCTION_INTERNAL, ctx->jit.type_void, "filter_nearest", sizeof(params) / sizeof(*params), params, 0);
	gcc_jit_rvalue *color = gcc_jit_param_as_rvalue(params[0]);
	gcc_jit_rvalue *coords = gcc_jit_param_as_rvalue(params[1]);
	gcc_jit_rvalue *size = gcc_jit_param_as_rvalue(params[2]);
	gcc_jit_rvalue *image_view_fn = gcc_jit_param_as_rvalue(params[3]);
	gcc_jit_block *block = gcc_jit_function_new_block(fn, "main");
	gcc_jit_function *wrap_coord_fn[3];
	gcc_jit_lvalue *coord_int = gcc_jit_function_new_local(fn, NULL, ctx->jit.type_int32_3, "coord_int");
	gcc_jit_lvalue *coord_fract = gcc_jit_function_new_local(fn, NULL, ctx->jit.type_float_3, "coord_fract");
	gcc_jit_lvalue *fetch_coords = gcc_jit_function_new_local(fn, NULL, ctx->jit.type_int32_3, "fetch_coords");
	gcc_jit_lvalue *wrapped_coords = gcc_jit_function_new_local(fn, NULL, ctx->jit.type_int32_3, "wrapped_coords");
	gcc_jit_lvalue *border_color = gcc_jit_function_new_local(fn, NULL, ctx->jit.type_float_4, "border_color");
	for (size_t i = 0; i < ncomp; ++i)
	{
		wrap_coord_fn[i] = jit_wrap_coord(ctx, jit_ctx, i, ctx->jit.sampler.wrap[create_info->wrap[0]], 0, 1);
		gcc_jit_rvalue *get_filter_coord_params[] =
		{
			gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(jit_ctx, NULL, coords, ctx->jit.value_i32[i])),
			gcc_jit_lvalue_get_address(gcc_jit_context_new_array_access(jit_ctx, NULL, gcc_jit_lvalue_as_rvalue(coord_int), ctx->jit.value_i32[i]), NULL),
			gcc_jit_lvalue_get_address(gcc_jit_context_new_array_access(jit_ctx, NULL, gcc_jit_lvalue_as_rvalue(coord_fract), ctx->jit.value_i32[i]), NULL),
		};
		gcc_jit_block_add_eval(block, NULL, gcc_jit_context_new_call(jit_ctx, NULL, ctx->jit.sampler.get_filter_coord, sizeof(get_filter_coord_params) / sizeof(*get_filter_coord_params), get_filter_coord_params));
		gcc_jit_rvalue *wrap_coord_params[] =
		{
			gcc_jit_lvalue_get_address(gcc_jit_context_new_array_access(jit_ctx, NULL, gcc_jit_lvalue_as_rvalue(wrapped_coords), ctx->jit.value_i32[i]), NULL),
			gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(jit_ctx, NULL, gcc_jit_lvalue_as_rvalue(coord_int), ctx->jit.value_i32[i])),
			gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(jit_ctx, NULL, size, ctx->jit.value_i32[i])),
		};
		gcc_jit_block_add_eval(block, NULL, gcc_jit_context_new_call(jit_ctx, NULL, wrap_coord_fn[i], sizeof(wrap_coord_params) / sizeof(*wrap_coord_params), wrap_coord_params));
	}
	for (size_t i = 0; i < 4; ++i)
	{
		gcc_jit_rvalue *rvalue = gcc_jit_context_new_rvalue_from_double(jit_ctx, ctx->jit.type_float, create_info->border_color[i]);
		gcc_jit_lvalue *lvalue = gcc_jit_context_new_array_access(jit_ctx, NULL, gcc_jit_lvalue_as_rvalue(border_color), ctx->jit.value_i32[i]);
		gcc_jit_block_add_assignment(block, NULL, lvalue, rvalue);
	}
	gcc_jit_rvalue *fetch_params[] =
	{
		color,
		gcc_jit_lvalue_get_address(gcc_jit_context_new_array_access(jit_ctx, NULL, gcc_jit_lvalue_as_rvalue(fetch_coords), ctx->jit.value_i32[0]), NULL),
		gcc_jit_lvalue_get_address(gcc_jit_context_new_array_access(jit_ctx, NULL, gcc_jit_lvalue_as_rvalue(wrapped_coords), ctx->jit.value_i32[0]), NULL),
		gcc_jit_lvalue_get_address(gcc_jit_context_new_array_access(jit_ctx, NULL, gcc_jit_lvalue_as_rvalue(coord_fract), ctx->jit.value_i32[0]), NULL),
		gcc_jit_lvalue_get_address(gcc_jit_context_new_array_access(jit_ctx, NULL, gcc_jit_lvalue_as_rvalue(border_color), ctx->jit.value_i32[0]), NULL),
		size,
		image_view_fn,
	};
	gcc_jit_block_add_eval(block, NULL, gcc_jit_context_new_call(jit_ctx, NULL, ctx->jit.sampler.nearest[ncomp - 1], sizeof(fetch_params) / sizeof(*fetch_params), fetch_params));
	gcc_jit_block_end_with_void_return(block, NULL);
	return fn;
}

static gcc_jit_function *
jit_filter_linear(struct jkg_ctx *ctx,
                  gcc_jit_context *jit_ctx,
                  const struct jkg_sampler_create_info *create_info,
                  size_t ncomp)
{
	gcc_jit_param *params[] =
	{
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_floatp, "color"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_const_floatp, "coords"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_const_int32p, "size"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_image_view_fn, "image_view_fn"),
	};
	gcc_jit_function *fn = gcc_jit_context_new_function(jit_ctx, NULL, GCC_JIT_FUNCTION_INTERNAL, ctx->jit.type_void, "filter_linear", sizeof(params) / sizeof(*params), params, 0);
	gcc_jit_rvalue *color = gcc_jit_param_as_rvalue(params[0]);
	gcc_jit_rvalue *coords = gcc_jit_param_as_rvalue(params[1]);
	gcc_jit_rvalue *size = gcc_jit_param_as_rvalue(params[2]);
	gcc_jit_rvalue *image_view_fn = gcc_jit_param_as_rvalue(params[3]);
	gcc_jit_block *block = gcc_jit_function_new_block(fn, "main");
	gcc_jit_lvalue *coord_int = gcc_jit_function_new_local(fn, NULL, ctx->jit.type_int32_3, "coord_int");
	gcc_jit_lvalue *coord_fract = gcc_jit_function_new_local(fn, NULL, ctx->jit.type_float_3, "coord_fract");
	gcc_jit_lvalue *fetch_coords = gcc_jit_function_new_local(fn, NULL, ctx->jit.type_int32_3, "fetch_coords");
	gcc_jit_lvalue *wrapped_coords = gcc_jit_function_new_local(fn, NULL, ctx->jit.type_int32_6, "wrapped_coords");
	gcc_jit_lvalue *border_color = gcc_jit_function_new_local(fn, NULL, ctx->jit.type_float_4, "border_color");
	for (size_t i = 0; i < ncomp; ++i)
	{
		gcc_jit_rvalue *get_filter_coord_params[] =
		{
			gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(jit_ctx, NULL, coords, ctx->jit.value_i32[i])),
			gcc_jit_lvalue_get_address(gcc_jit_context_new_array_access(jit_ctx, NULL, gcc_jit_lvalue_as_rvalue(coord_int), ctx->jit.value_i32[i]), NULL),
			gcc_jit_lvalue_get_address(gcc_jit_context_new_array_access(jit_ctx, NULL, gcc_jit_lvalue_as_rvalue(coord_fract), ctx->jit.value_i32[i]), NULL),
		};
		gcc_jit_block_add_eval(block, NULL, gcc_jit_context_new_call(jit_ctx, NULL, ctx->jit.sampler.get_filter_coord, sizeof(get_filter_coord_params) / sizeof(*get_filter_coord_params), get_filter_coord_params));
		gcc_jit_function *wrap_coord_fn = jit_wrap_coord(ctx, jit_ctx, i, ctx->jit.sampler.wrap[create_info->wrap[i]], 0, 2);
		gcc_jit_rvalue *wrap_coord_params[] =
		{
			gcc_jit_lvalue_get_address(gcc_jit_context_new_array_access(jit_ctx, NULL, gcc_jit_lvalue_as_rvalue(wrapped_coords), gcc_jit_context_new_rvalue_from_int(jit_ctx, ctx->jit.type_int32, i * 2)), NULL),
			gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(jit_ctx, NULL, gcc_jit_lvalue_as_rvalue(coord_int), ctx->jit.value_i32[i])),
			gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(jit_ctx, NULL, size, ctx->jit.value_i32[i])),
		};
		gcc_jit_block_add_eval(block, NULL, gcc_jit_context_new_call(jit_ctx, NULL, wrap_coord_fn, sizeof(wrap_coord_params) / sizeof(*wrap_coord_params), wrap_coord_params));
	}
	for (size_t i = 0; i < 4; ++i)
	{
		gcc_jit_rvalue *rvalue = gcc_jit_context_new_rvalue_from_double(jit_ctx, ctx->jit.type_float, create_info->border_color[i]);
		gcc_jit_lvalue *lvalue = gcc_jit_context_new_array_access(jit_ctx, NULL, gcc_jit_lvalue_as_rvalue(border_color), ctx->jit.value_i32[i]);
		gcc_jit_block_add_assignment(block, NULL, lvalue, rvalue);
	}
	gcc_jit_rvalue *fetch_params[] =
	{
		color,
		gcc_jit_lvalue_get_address(gcc_jit_context_new_array_access(jit_ctx, NULL, gcc_jit_lvalue_as_rvalue(fetch_coords), ctx->jit.value_i32[0]), NULL),
		gcc_jit_lvalue_get_address(gcc_jit_context_new_array_access(jit_ctx, NULL, gcc_jit_lvalue_as_rvalue(wrapped_coords), ctx->jit.value_i32[0]), NULL),
		gcc_jit_lvalue_get_address(gcc_jit_context_new_array_access(jit_ctx, NULL, gcc_jit_lvalue_as_rvalue(coord_fract), ctx->jit.value_i32[0]), NULL),
		gcc_jit_lvalue_get_address(gcc_jit_context_new_array_access(jit_ctx, NULL, gcc_jit_lvalue_as_rvalue(border_color), ctx->jit.value_i32[0]), NULL),
		size,
		image_view_fn,
	};
	gcc_jit_block_add_eval(block, NULL, gcc_jit_context_new_call(jit_ctx, NULL, ctx->jit.sampler.linear[ncomp - 1], sizeof(fetch_params) / sizeof(*fetch_params), fetch_params));
	gcc_jit_block_end_with_void_return(block, NULL);
	return fn;
}

static gcc_jit_function *
jit_filter_cubic(struct jkg_ctx *ctx,
                 gcc_jit_context *jit_ctx,
                 const struct jkg_sampler_create_info *create_info,
                 size_t ncomp)
{
	gcc_jit_param *params[] =
	{
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_floatp, "color"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_const_floatp, "coords"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_const_int32p, "size"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_image_view_fn, "image_view_fn"),
	};
	gcc_jit_function *fn = gcc_jit_context_new_function(jit_ctx, NULL, GCC_JIT_FUNCTION_INTERNAL, ctx->jit.type_void, "filter_cubic", sizeof(params) / sizeof(*params), params, 0);
	gcc_jit_rvalue *color = gcc_jit_param_as_rvalue(params[0]);
	gcc_jit_rvalue *coords = gcc_jit_param_as_rvalue(params[1]);
	gcc_jit_rvalue *size = gcc_jit_param_as_rvalue(params[2]);
	gcc_jit_rvalue *image_view_fn = gcc_jit_param_as_rvalue(params[3]);
	gcc_jit_block *block = gcc_jit_function_new_block(fn, "main");
	gcc_jit_lvalue *coord_int = gcc_jit_function_new_local(fn, NULL, ctx->jit.type_int32_3, "coord_int");
	gcc_jit_lvalue *coord_fract = gcc_jit_function_new_local(fn, NULL, ctx->jit.type_float_3, "coord_fract");
	gcc_jit_lvalue *fetch_coords = gcc_jit_function_new_local(fn, NULL, ctx->jit.type_int32_3, "fetch_coords");
	gcc_jit_lvalue *wrapped_coords = gcc_jit_function_new_local(fn, NULL, ctx->jit.type_int32_12, "wrapped_coords");
	gcc_jit_lvalue *border_color = gcc_jit_function_new_local(fn, NULL, ctx->jit.type_float_4, "border_color");
	for (size_t i = 0; i < ncomp; ++i)
	{
		gcc_jit_rvalue *get_filter_coord_params[] =
		{
			gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(jit_ctx, NULL, coords, ctx->jit.value_i32[i])),
			gcc_jit_lvalue_get_address(gcc_jit_context_new_array_access(jit_ctx, NULL, gcc_jit_lvalue_as_rvalue(coord_int), ctx->jit.value_i32[i]), NULL),
			gcc_jit_lvalue_get_address(gcc_jit_context_new_array_access(jit_ctx, NULL, gcc_jit_lvalue_as_rvalue(coord_fract), ctx->jit.value_i32[i]), NULL),
		};
		gcc_jit_block_add_eval(block, NULL, gcc_jit_context_new_call(jit_ctx, NULL, ctx->jit.sampler.get_filter_coord, sizeof(get_filter_coord_params) / sizeof(*get_filter_coord_params), get_filter_coord_params));
		gcc_jit_function *wrap_coord_fn = jit_wrap_coord(ctx, jit_ctx, i, ctx->jit.sampler.wrap[create_info->wrap[i]], -1, 4);
		gcc_jit_rvalue *wrap_coord_params[] =
		{
			gcc_jit_lvalue_get_address(gcc_jit_context_new_array_access(jit_ctx, NULL, gcc_jit_lvalue_as_rvalue(wrapped_coords), gcc_jit_context_new_rvalue_from_int(jit_ctx, ctx->jit.type_int32, i * 4)), NULL),
			gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(jit_ctx, NULL, gcc_jit_lvalue_as_rvalue(coord_int), ctx->jit.value_i32[i])),
			gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(jit_ctx, NULL, size, ctx->jit.value_i32[i])),
		};
		gcc_jit_block_add_eval(block, NULL, gcc_jit_context_new_call(jit_ctx, NULL, wrap_coord_fn, sizeof(wrap_coord_params) / sizeof(*wrap_coord_params), wrap_coord_params));
	}
	for (size_t i = 0; i < 4; ++i)
	{
		gcc_jit_rvalue *rvalue = gcc_jit_context_new_rvalue_from_double(jit_ctx, ctx->jit.type_float, create_info->border_color[i]);
		gcc_jit_lvalue *lvalue = gcc_jit_context_new_array_access(jit_ctx, NULL, gcc_jit_lvalue_as_rvalue(border_color), ctx->jit.value_i32[i]);
		gcc_jit_block_add_assignment(block, NULL, lvalue, rvalue);
	}
	gcc_jit_rvalue *fetch_params[] =
	{
		color,
		gcc_jit_lvalue_get_address(gcc_jit_context_new_array_access(jit_ctx, NULL, gcc_jit_lvalue_as_rvalue(fetch_coords), ctx->jit.value_i32[0]), NULL),
		gcc_jit_lvalue_get_address(gcc_jit_context_new_array_access(jit_ctx, NULL, gcc_jit_lvalue_as_rvalue(wrapped_coords), ctx->jit.value_i32[0]), NULL),
		gcc_jit_lvalue_get_address(gcc_jit_context_new_array_access(jit_ctx, NULL, gcc_jit_lvalue_as_rvalue(coord_fract), ctx->jit.value_i32[0]), NULL),
		gcc_jit_lvalue_get_address(gcc_jit_context_new_array_access(jit_ctx, NULL, gcc_jit_lvalue_as_rvalue(border_color), ctx->jit.value_i32[0]), NULL),
		size,
		image_view_fn,
	};
	gcc_jit_block_add_eval(block, NULL, gcc_jit_context_new_call(jit_ctx, NULL, ctx->jit.sampler.cubic[ncomp - 1], sizeof(fetch_params) / sizeof(*fetch_params), fetch_params));
	gcc_jit_block_end_with_void_return(block, NULL);
	return fn;
}

static gcc_jit_function *
jit_sample(struct jkg_ctx *ctx,
           gcc_jit_context *jit_ctx,
           const struct jkg_sampler_create_info *create_info)
{
	gcc_jit_param *params[] =
	{
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_floatp, "color"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_const_floatp, "coords"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_const_int32p, "size"),
		gcc_jit_context_new_param(jit_ctx, NULL, ctx->jit.type_image_view_fn, "image_view_fn"),
	};
	gcc_jit_function *fn = gcc_jit_context_new_function(jit_ctx, NULL, GCC_JIT_FUNCTION_EXPORTED, ctx->jit.type_void, "sample", sizeof(params) / sizeof(*params), params, 0);
	gcc_jit_rvalue *color = gcc_jit_param_as_rvalue(params[0]);
	gcc_jit_rvalue *coords = gcc_jit_param_as_rvalue(params[1]);
	gcc_jit_rvalue *size = gcc_jit_param_as_rvalue(params[2]);
	gcc_jit_rvalue *image_view_fn = gcc_jit_param_as_rvalue(params[3]);
	gcc_jit_block *block = gcc_jit_function_new_block(fn, NULL);
	gcc_jit_function *filter_fn;
	switch (create_info->mag_filter) /* XXX handle min / mag / mip */
	{
		default:
			assert(!"unknown filtering");
			/* FALLTHROUGH */
		case JKG_FILTER_NEAREST:
			filter_fn = jit_filter_nearest(ctx, jit_ctx, create_info, 2);
			break;
		case JKG_FILTER_LINEAR:
			filter_fn = jit_filter_linear(ctx, jit_ctx, create_info, 2);
			break;
		case JKG_FILTER_CUBIC:
			filter_fn = jit_filter_cubic(ctx, jit_ctx, create_info, 2);
			break;
	}
	gcc_jit_rvalue *filter_params[] =
	{
		color,
		coords,
		size,
		image_view_fn,
	};
	gcc_jit_block_add_eval(block, NULL, gcc_jit_context_new_call(jit_ctx, NULL, filter_fn, sizeof(filter_params) / sizeof(*filter_params), filter_params));
	gcc_jit_block_end_with_void_return(block, NULL);
	return fn;
}

void
gjit_sample(struct jkg_ctx *ctx,
            uint32_t id,
            const float coord[3],
            float color[4])
{
	struct jkg_image_view *image_view;
	struct jkg_sampler *sampler;
	float texel_coord[3];
	int32_t size[3];

	image_view = ctx->image_views[id];
	sampler = ctx->samplers[id];
	if (!image_view)
	{
		color[0] = 0;
		color[1] = 0;
		color[2] = 0;
		color[3] = 1;
		return;
	}
	size[0] = image_view->image->size.x;
	size[1] = image_view->image->size.y;
	size[2] = image_view->image->size.z;
	if (!size[0])
		size[0] = 1;
	if (!size[1])
		size[1] = 1;
	if (!size[2])
		size[2] = 1;
	texel_coord[0] = coord[0] * size[0];
	texel_coord[1] = coord[1] * size[1];
	texel_coord[2] = coord[2] * size[2];
	sampler->fn(color, texel_coord, size, image_view->fn);
}
