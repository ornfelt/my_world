#include "internal.h"
#include "fnv.h"

#include <assert.h>
#include <stdlib.h>

static gcc_jit_function *jit_wrap(struct gl_ctx *gl_ctx, struct texture *texture, gcc_jit_context *ctx, const char *name, GLenum mode, GLuint max)
{
	struct jit_ctx *jit_ctx = &gl_ctx->jit;
	gcc_jit_param *params[] =
	{
		gcc_jit_context_new_param(ctx, NULL, jit_ctx->type_int32, "value"),
	};
	gcc_jit_function *fn = gcc_jit_context_new_function(ctx, NULL, GCC_JIT_FUNCTION_INTERNAL, jit_ctx->type_int32, name, sizeof(params) / sizeof(*params), params, 0);
	gcc_jit_rvalue *value = gcc_jit_param_as_rvalue(params[0]);
	gcc_jit_block *block = gcc_jit_function_new_block(fn, NULL);
	switch (mode)
	{
		case GL_CLAMP:
		case GL_CLAMP_TO_BORDER:
		{
			gcc_jit_block *negative_block = gcc_jit_function_new_block(fn, NULL);
			gcc_jit_block_end_with_return(negative_block, NULL, gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_int32, 0));
			gcc_jit_block *max_block = gcc_jit_function_new_block(fn, NULL);
			gcc_jit_block_end_with_return(max_block, NULL, gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_int32, max - 1));
			gcc_jit_block *ranged_block = gcc_jit_function_new_block(fn, NULL);
			gcc_jit_block_end_with_return(ranged_block, NULL, value);
			gcc_jit_block *positive_block = gcc_jit_function_new_block(fn, NULL);
			gcc_jit_rvalue *max_cmp = gcc_jit_context_new_comparison(ctx, NULL, GCC_JIT_COMPARISON_GE, value, gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_int32, max));
			gcc_jit_block_end_with_conditional(positive_block, NULL, max_cmp, max_block, ranged_block);
			gcc_jit_rvalue *negative_cmp = gcc_jit_context_new_comparison(ctx, NULL, GCC_JIT_COMPARISON_LT, value, gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_int32, 0));
			gcc_jit_block_end_with_conditional(block, NULL, negative_cmp, negative_block, positive_block);
			break;
		}
		case GL_REPEAT:
		{
			gcc_jit_block *positive_block = gcc_jit_function_new_block(fn, NULL);
			gcc_jit_block *negative_block = gcc_jit_function_new_block(fn, NULL);
			gcc_jit_rvalue *mod = gcc_jit_context_new_binary_op(ctx, NULL, GCC_JIT_BINARY_OP_MODULO, jit_ctx->type_int32, value, gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_int32, max));
			gcc_jit_rvalue *negative_cmp = gcc_jit_context_new_comparison(ctx, NULL, GCC_JIT_COMPARISON_LT, value, gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_int32, 0));
			gcc_jit_block_end_with_conditional(block, NULL, negative_cmp, negative_block, positive_block);
			gcc_jit_rvalue *mod_positive = gcc_jit_context_new_binary_op(ctx, NULL, GCC_JIT_BINARY_OP_PLUS, jit_ctx->type_int32, mod, gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_int32, max));
			gcc_jit_block_end_with_return(negative_block, NULL, mod_positive);
			gcc_jit_block_end_with_return(positive_block, NULL, mod);
			break;
		}
		case GL_MIRRORED_REPEAT:
		{
			gcc_jit_lvalue *v = gcc_jit_function_new_local(fn, NULL, jit_ctx->type_int32, "v");
			gcc_jit_rvalue *max2 = gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_int32, max * 2);
			gcc_jit_block_add_assignment(block, NULL, v, gcc_jit_context_new_binary_op(ctx, NULL, GCC_JIT_BINARY_OP_MODULO, jit_ctx->type_int32, value, max2));
			gcc_jit_block *positive_block = gcc_jit_function_new_block(fn, NULL);
			gcc_jit_block *negative_block = gcc_jit_function_new_block(fn, NULL);
			gcc_jit_rvalue *negative_cmp = gcc_jit_context_new_comparison(ctx, NULL, GCC_JIT_COMPARISON_LT, gcc_jit_lvalue_as_rvalue(v), gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_int32, 0));
			gcc_jit_block_end_with_conditional(block, NULL, negative_cmp, negative_block, positive_block);
			gcc_jit_block_add_assignment_op(negative_block, NULL, v, GCC_JIT_BINARY_OP_PLUS, max2);
			gcc_jit_block_end_with_jump(negative_block, NULL, positive_block);
			gcc_jit_block *max_block = gcc_jit_function_new_block(fn, NULL);
			gcc_jit_rvalue *maxm1 = gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_int32, max - 1);
			gcc_jit_rvalue *vmmax = gcc_jit_context_new_binary_op(ctx, NULL, GCC_JIT_BINARY_OP_MINUS, jit_ctx->type_int32, gcc_jit_lvalue_as_rvalue(v), gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_int32, max));
			gcc_jit_block_end_with_return(max_block, NULL, gcc_jit_context_new_binary_op(ctx, NULL, GCC_JIT_BINARY_OP_MINUS, jit_ctx->type_int32, maxm1, vmmax));
			gcc_jit_block *ranged_block = gcc_jit_function_new_block(fn, NULL);
			gcc_jit_block_end_with_return(ranged_block, NULL, gcc_jit_lvalue_as_rvalue(v));
			gcc_jit_rvalue *max_cmp = gcc_jit_context_new_comparison(ctx, NULL, GCC_JIT_COMPARISON_GE, gcc_jit_lvalue_as_rvalue(v), gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_int32, max));
			gcc_jit_block_end_with_conditional(positive_block, NULL, max_cmp, max_block, ranged_block);
			break;
		}
		/*case GL_CLAMP_TO_EDGE:
			break;*/
		default:
			assert(!"unknown wrap mode");
			break;
	}
	return fn;
}

static gcc_jit_function *jit_texture_fetch(struct gl_ctx *gl_ctx, struct texture *texture, gcc_jit_context *ctx)
{
	struct jit_ctx *jit_ctx = &gl_ctx->jit;
	gcc_jit_param *params[] =
	{
		gcc_jit_context_new_param(ctx, NULL, jit_ctx->type_floatp, "color"),
		gcc_jit_context_new_param(ctx, NULL, jit_ctx->type_voidp, "data"),
		gcc_jit_context_new_param(ctx, NULL, jit_ctx->type_int32, "s"),
		gcc_jit_context_new_param(ctx, NULL, jit_ctx->type_int32, "t"),
		gcc_jit_context_new_param(ctx, NULL, jit_ctx->type_int32, "p"),
		gcc_jit_context_new_param(ctx, NULL, jit_ctx->type_int32, "q"),
	};
	gcc_jit_function *fn = gcc_jit_context_new_function(ctx, NULL, GCC_JIT_FUNCTION_EXPORTED, jit_ctx->type_void, "texture_fetch", sizeof(params) / sizeof(*params), params, 0);
	gcc_jit_rvalue *color = gcc_jit_param_as_rvalue(params[0]);
	gcc_jit_rvalue *data = gcc_jit_param_as_rvalue(params[1]);
	gcc_jit_rvalue *s = gcc_jit_param_as_rvalue(params[2]);
	gcc_jit_rvalue *t = gcc_jit_param_as_rvalue(params[3]);
	gcc_jit_rvalue *p = gcc_jit_param_as_rvalue(params[4]);
	gcc_jit_rvalue *q = gcc_jit_param_as_rvalue(params[5]);
	gcc_jit_block *block = gcc_jit_function_new_block(fn, NULL);
	gcc_jit_lvalue *idx = gcc_jit_function_new_local(fn, NULL, jit_ctx->type_int32, "idx");
	switch (texture->target)
	{
		case GL_TEXTURE_2D:
		{
			gcc_jit_function *wrap_s_fn = jit_wrap(gl_ctx, texture, ctx, "wrap_s", texture->wrap_s, texture->width);
			gcc_jit_rvalue *wrap_s_args[] = {s};
			gcc_jit_rvalue *s_rvalue = gcc_jit_context_new_call(ctx, NULL, wrap_s_fn, sizeof(wrap_s_args) / sizeof(*wrap_s_args), wrap_s_args);
			gcc_jit_function *wrap_t_fn = jit_wrap(gl_ctx, texture, ctx, "wrap_t", texture->wrap_t, texture->height);
			gcc_jit_rvalue *wrap_t_args[] = {t};
			gcc_jit_rvalue *t_rvalue = gcc_jit_context_new_call(ctx, NULL, wrap_t_fn, sizeof(wrap_t_args) / sizeof(*wrap_t_args), wrap_t_args);
			gcc_jit_rvalue *width = gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_int32, texture->width);
			gcc_jit_rvalue *t_width_rvalue = gcc_jit_context_new_binary_op(ctx, NULL, GCC_JIT_BINARY_OP_MULT, jit_ctx->type_int32, t_rvalue, width);
			gcc_jit_rvalue *expr = gcc_jit_context_new_binary_op(ctx, NULL, GCC_JIT_BINARY_OP_PLUS, jit_ctx->type_int32, s_rvalue, t_width_rvalue);
			gcc_jit_block_add_assignment(block, NULL, idx, expr);
			break;
		}
		default:
			assert(!"unknown texture target");
	}
	gcc_jit_rvalue *rgba[4];
	switch (texture->format)
	{
		case GL_RGBA8:
		{
			gcc_jit_rvalue *base_idx = gcc_jit_context_new_binary_op(ctx, NULL, GCC_JIT_BINARY_OP_MULT, jit_ctx->type_int32, gcc_jit_lvalue_as_rvalue(idx), gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_int, 4));
			gcc_jit_rvalue *rvalue_255 = gcc_jit_context_new_rvalue_from_double(ctx, jit_ctx->type_float, 255.f);
			gcc_jit_rvalue *base_uint8 = gcc_jit_context_new_cast(ctx, NULL, data, jit_ctx->type_uint8p);
			gcc_jit_rvalue *base_ptr = gcc_jit_lvalue_get_address(gcc_jit_context_new_array_access(ctx, NULL, base_uint8, base_idx), NULL);
			for (GLint i = 0; i < 4; ++i)
			{
				gcc_jit_rvalue *comp = gcc_jit_context_new_cast(ctx, NULL, gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, NULL, base_ptr, gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_int, i))), jit_ctx->type_float);
				rgba[i] = gcc_jit_context_new_binary_op(ctx, NULL, GCC_JIT_BINARY_OP_DIVIDE, jit_ctx->type_float, comp, rvalue_255);
			}
			break;
		}
		default:
			assert(!"unknown texture format");
			rgba[0] = gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_float, 1);
			rgba[1] = gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_float, 1);
			rgba[2] = gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_float, 1);
			rgba[3] = gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_float, 1);
			break;
	}
	for (GLint i = 0; i < 4; ++i)
		gcc_jit_block_add_assignment(block, NULL, gcc_jit_context_new_array_access(ctx, NULL, color, gcc_jit_context_new_rvalue_from_int(ctx, jit_ctx->type_int, i)), rgba[i]);
	gcc_jit_block_end_with_void_return(block, NULL);
	return fn;
}

static GLuint texture_fetch_state_hash(struct texture *texture)
{
	GLuint hash = FNV_BASIS32;
	hash = fnv32v(hash, &texture->width, sizeof(texture->width));
	hash = fnv32v(hash, &texture->height, sizeof(texture->height));
	hash = fnv32v(hash, &texture->depth, sizeof(texture->depth));
	hash = fnv32v(hash, &texture->format, sizeof(texture->format));
	hash = fnv32v(hash, &texture->target, sizeof(texture->target));
	hash = fnv32v(hash, &texture->wrap_s, sizeof(texture->wrap_s));
	hash = fnv32v(hash, &texture->wrap_t, sizeof(texture->wrap_t));
	hash = fnv32v(hash, &texture->wrap_r, sizeof(texture->wrap_r));
	return hash;
}

static struct texture_fetch_head *texture_fetch_head(struct gl_ctx *gl_ctx, GLuint hash)
{
	return &gl_ctx->jit.texture_fetch_hash[hash % sizeof(gl_ctx->jit.texture_fetch_hash) / sizeof(*gl_ctx->jit.texture_fetch_hash)];
}

static struct texture_fetch_state *get_texture_fetch_state(struct gl_ctx *gl_ctx, struct texture *texture)
{
	GLuint hash = texture_fetch_state_hash(texture);
	struct texture_fetch_head *hash_head = texture_fetch_head(gl_ctx, hash);
	if (TAILQ_EMPTY(hash_head))
		return NULL;
	struct texture_fetch_state *state;
	TAILQ_FOREACH(state, hash_head, hash_chain)
	{
		if (state->hash == hash
		 && state->width == texture->width
		 && state->height == texture->height
		 && state->depth == texture->depth
		 && state->format == texture->format
		 && state->target == texture->target
		 && state->wrap_s == texture->wrap_s
		 && state->wrap_t == texture->wrap_t
		 && state->wrap_r == texture->wrap_r)
			return state;
	}
	return NULL;
}

static struct texture_fetch_state *create_texture_fetch_state(struct gl_ctx *gl_ctx, struct texture *texture)
{
	struct texture_fetch_state *state = malloc(sizeof(*state));
	if (!state)
		return NULL;
	state->hash = texture_fetch_state_hash(texture);
	state->width = texture->width;
	state->height = texture->height;
	state->depth = texture->depth;
	state->format = texture->format;
	state->target = texture->target;
	state->wrap_s = texture->wrap_s;
	state->wrap_t = texture->wrap_t;
	state->wrap_r = texture->wrap_r;
	gcc_jit_context *ctx = gcc_jit_context_new_child_context(gl_ctx->jit.ctx);
	jit_texture_fetch(gl_ctx, texture, ctx);
	state->jit_res = gcc_jit_context_compile(ctx);
	gcc_jit_context_release(ctx);
	state->fn = gcc_jit_result_get_code(state->jit_res, "texture_fetch");
	return state;
}

static void delete_texture_fetch_state(struct gl_ctx *gl_ctx, struct texture_fetch_state *state)
{
	TAILQ_REMOVE(&gl_ctx->jit.texture_fetch_fifo, state, fifo_chain);
	TAILQ_REMOVE(texture_fetch_head(gl_ctx, state->hash), state, hash_chain);
	gcc_jit_result_release(state->jit_res);
	free(state);
	gl_ctx->jit.texture_fetch_count--;
}

GLboolean jit_update_texture_fetch(struct gl_ctx *gl_ctx, struct texture *texture)
{
	static const GLuint dirty_mask = GL_TEXTURE_DIRTY_WIDTH
	                               | GL_TEXTURE_DIRTY_HEIGTH
	                               | GL_TEXTURE_DIRTY_DEPTH
	                               | GL_TEXTURE_DIRTY_FORMAT
	                               | GL_TEXTURE_DIRTY_TARGET
	                               | GL_TEXTURE_DIRTY_WRAP_S
	                               | GL_TEXTURE_DIRTY_WRAP_T
	                               | GL_TEXTURE_DIRTY_WRAP_R
	                               | GL_TEXTURE_DIRTY_MIN_FILTER
	                               | GL_TEXTURE_DIRTY_MAG_FILTER;
	if (!(texture->dirty & dirty_mask))
		return GL_TRUE;
	texture->dirty &= ~dirty_mask;
	struct texture_fetch_state *state = get_texture_fetch_state(gl_ctx, texture);
	if (!state)
	{
		state = create_texture_fetch_state(gl_ctx, texture);
		if (!state)
			return GL_FALSE;
		TAILQ_INSERT_TAIL(texture_fetch_head(gl_ctx, state->hash), state, hash_chain);
		gl_ctx->jit.texture_fetch_count++;
		if (gl_ctx->jit.texture_fetch_count > 1024)
			delete_texture_fetch_state(gl_ctx, TAILQ_FIRST(&gl_ctx->jit.texture_fetch_fifo));
	}
	else
	{
		TAILQ_REMOVE(&gl_ctx->jit.texture_fetch_fifo, state, fifo_chain);
	}
	TAILQ_INSERT_TAIL(&gl_ctx->jit.texture_fetch_fifo, state, fifo_chain);
	texture->texture_fetch_state = state;
	return GL_TRUE;
}
