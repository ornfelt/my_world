#include "internal.h"
#include "fnv.h"

#include <assert.h>
#include <stdlib.h>

static gcc_jit_function *jit_depth_test(struct gl_ctx *gl_ctx, gcc_jit_context *ctx)
{
	struct jit_ctx *jit_ctx = &gl_ctx->jit;
	gcc_jit_param *params[] =
	{
		gcc_jit_context_new_param(ctx, NULL, jit_ctx->type_float, "depth"),
		gcc_jit_context_new_param(ctx, NULL, jit_ctx->type_floatp, "range"),
		gcc_jit_context_new_param(ctx, NULL, jit_ctx->type_float, "cur"),
	};
	gcc_jit_function *fn = gcc_jit_context_new_function(ctx, NULL, GCC_JIT_FUNCTION_EXPORTED, jit_ctx->type_uint8, "depth_test", sizeof(params) / sizeof(*params), params, 0);
	gcc_jit_rvalue *depth = gcc_jit_param_as_rvalue(params[0]);
	gcc_jit_rvalue *range = gcc_jit_param_as_rvalue(params[1]);
	gcc_jit_rvalue *cur = gcc_jit_param_as_rvalue(params[2]);
	gcc_jit_block *range_min_block = gcc_jit_function_new_block(fn, NULL);
	gcc_jit_block *true_block = gcc_jit_function_new_block(fn, NULL);
	gcc_jit_block_end_with_return(true_block, NULL, gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_uint8, 1));
	gcc_jit_block *false_block = gcc_jit_function_new_block(fn, NULL);
	gcc_jit_block_end_with_return(false_block, NULL, gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_uint8, 0));
	gcc_jit_rvalue *range_min = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, range, gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_int, 0)));
	gcc_jit_rvalue *range_min_cmp = gcc_jit_context_new_comparison(ctx, NULL, GCC_JIT_COMPARISON_GE, depth, range_min);
	gcc_jit_block *range_max_block = gcc_jit_function_new_block(fn, NULL);
	gcc_jit_block_end_with_conditional(range_min_block, NULL, range_min_cmp, range_max_block, false_block);
	gcc_jit_rvalue *range_max = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, range, gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_int, 1)));
	gcc_jit_rvalue *range_max_cmp = gcc_jit_context_new_comparison(ctx, NULL, GCC_JIT_COMPARISON_LE, depth, range_max);
	gcc_jit_block *block = gcc_jit_function_new_block(fn, NULL);
	gcc_jit_block_end_with_conditional(range_max_block, NULL, range_max_cmp, block, false_block);
	if (!gl_ctx->depth_test)
	{
		gcc_jit_block_end_with_return(block, NULL, gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_uint8, 1));
		return fn;
	}
	switch (gl_ctx->depth_func)
	{
		case GL_ALWAYS:
			gcc_jit_block_end_with_return(block, NULL, gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_uint8, 1));
			break;
		case GL_LESS:
		{
			gcc_jit_rvalue *cmp = gcc_jit_context_new_comparison(ctx, NULL, GCC_JIT_COMPARISON_LT, depth, cur);
			gcc_jit_block_end_with_conditional(block, NULL, cmp, true_block, false_block);
			break;
		}
		case GL_LEQUAL:
		{
			gcc_jit_rvalue *cmp = gcc_jit_context_new_comparison(ctx, NULL, GCC_JIT_COMPARISON_LE, depth, cur);
			gcc_jit_block_end_with_conditional(block, NULL, cmp, true_block, false_block);
			break;
		}
		case GL_EQUAL:
		{
			gcc_jit_rvalue *cmp = gcc_jit_context_new_comparison(ctx, NULL, GCC_JIT_COMPARISON_EQ, depth, cur);
			gcc_jit_block_end_with_conditional(block, NULL, cmp, true_block, false_block);
			break;
		}
		case GL_GEQUAL:
		{
			gcc_jit_rvalue *cmp = gcc_jit_context_new_comparison(ctx, NULL, GCC_JIT_COMPARISON_GE, depth, cur);
			gcc_jit_block_end_with_conditional(block, NULL, cmp, true_block, false_block);
			break;
		}
		case GL_GREATER:
		{
			gcc_jit_rvalue *cmp = gcc_jit_context_new_comparison(ctx, NULL, GCC_JIT_COMPARISON_GT, depth, cur);
			gcc_jit_block_end_with_conditional(block, NULL, cmp, true_block, false_block);
			break;
		}
		case GL_NEVER:
			gcc_jit_block_end_with_return(block, NULL, gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_uint8, 0));
			break;
		case GL_NOTEQUAL:
		{
			gcc_jit_rvalue *cmp = gcc_jit_context_new_comparison(ctx, NULL, GCC_JIT_COMPARISON_NE, depth, cur);
			gcc_jit_block_end_with_conditional(block, NULL, cmp, true_block, false_block);
			break;
		}
		default:
			assert(!"unknown depth test");
			return GL_FALSE;
	}
	return fn;
}

static GLuint depth_test_state_hash(struct gl_ctx *gl_ctx)
{
	GLuint hash = FNV_BASIS32;
	hash = fnv32v(hash, &gl_ctx->depth_range[0], sizeof(gl_ctx->depth_range));
	hash = fnv32v(hash, &gl_ctx->depth_test, sizeof(gl_ctx->depth_test));
	hash = fnv32v(hash, &gl_ctx->depth_func, sizeof(gl_ctx->depth_func));
	return hash;
}

static struct depth_test_head *depth_test_head(struct gl_ctx *gl_ctx, GLuint hash)
{
	return &gl_ctx->jit.depth_test_hash[hash % sizeof(gl_ctx->jit.depth_test_hash) / sizeof(*gl_ctx->jit.depth_test_hash)];
}

static struct depth_test_state *get_depth_test_state(struct gl_ctx *gl_ctx)
{
	GLuint hash = depth_test_state_hash(gl_ctx);
	struct depth_test_head *hash_head = depth_test_head(gl_ctx, hash);
	if (TAILQ_EMPTY(hash_head))
		return NULL;
	struct depth_test_state *state;
	TAILQ_FOREACH(state, hash_head, hash_chain)
	{
		if (state->hash == hash
		 && state->depth_test == gl_ctx->depth_test
		 && state->depth_func == gl_ctx->depth_func
		 && state->depth_range[0] == gl_ctx->depth_range[0]
		 && state->depth_range[1] == gl_ctx->depth_range[1])
			return state;
	}
	return NULL;
}

static struct depth_test_state *create_depth_test_state(struct gl_ctx *gl_ctx)
{
	struct depth_test_state *state = malloc(sizeof(*state));
	if (!state)
		return NULL;
	state->hash = depth_test_state_hash(gl_ctx);
	state->depth_range[0] = gl_ctx->depth_range[0];
	state->depth_range[1] = gl_ctx->depth_range[1];
	state->depth_test = gl_ctx->depth_test;
	state->depth_func = gl_ctx->depth_func;
	gcc_jit_context *ctx = gcc_jit_context_new_child_context(gl_ctx->jit.ctx);
	jit_depth_test(gl_ctx, ctx);
	state->jit_res = gcc_jit_context_compile(ctx);
	gcc_jit_context_release(ctx);
	state->fn = gcc_jit_result_get_code(state->jit_res, "depth_test");
	return state;
}

static void delete_depth_test_state(struct gl_ctx *gl_ctx, struct depth_test_state *state)
{
	TAILQ_REMOVE(&gl_ctx->jit.depth_test_fifo, state, fifo_chain);
	TAILQ_REMOVE(depth_test_head(gl_ctx, state->hash), state, hash_chain);
	gcc_jit_result_release(state->jit_res);
	free(state);
	gl_ctx->jit.depth_test_count--;
}

GLboolean jit_update_depth_test(struct gl_ctx *gl_ctx)
{
	static const GLuint dirty_mask = GL_CTX_DIRTY_DEPTH_RANGE_MIN
	                               | GL_CTX_DIRTY_DEPTH_RANGE_MAX
	                               | GL_CTX_DIRTY_DEPTH_TEST
	                               | GL_CTX_DIRTY_DEPTH_FUNC;
	if (!(gl_ctx->dirty & dirty_mask))
		return GL_TRUE;
	gl_ctx->dirty &= ~dirty_mask;
	struct depth_test_state *state = get_depth_test_state(gl_ctx);
	if (!state)
	{
		state = create_depth_test_state(gl_ctx);
		if (!state)
			return GL_FALSE;
		TAILQ_INSERT_TAIL(depth_test_head(gl_ctx, state->hash), state, hash_chain);
		gl_ctx->jit.depth_test_count++;
		if (gl_ctx->jit.depth_test_count > 1024)
			delete_depth_test_state(gl_ctx, TAILQ_FIRST(&gl_ctx->jit.depth_test_fifo));
	}
	else
	{
		TAILQ_REMOVE(&gl_ctx->jit.depth_test_fifo, state, fifo_chain);
	}
	TAILQ_INSERT_TAIL(&gl_ctx->jit.depth_test_fifo, state, fifo_chain);
	gl_ctx->jit.depth_test_state = state;
	return GL_TRUE;
}
