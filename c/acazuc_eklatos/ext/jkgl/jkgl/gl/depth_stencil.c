#include "internal.h"
#include "fnv.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

static bool
is_valid_depth_func(GLenum func)
{
	switch (func)
	{
		case GL_NEVER:
		case GL_LESS:
		case GL_EQUAL:
		case GL_LEQUAL:
		case GL_GREATER:
		case GL_NOTEQUAL:
		case GL_GEQUAL:
		case GL_ALWAYS:
			return true;
		default:
			return false;
	}
}

void
glDepthFunc(GLenum func)
{
	struct gl_ctx *ctx = g_ctx;

	if (!is_valid_depth_func(func))
	{
		GL_SET_ERR(ctx, GL_INVALID_ENUM);
		return;
	}
	if (ctx->depth_stencil_state.depth_compare != func)
	{
		ctx->depth_stencil_state.depth_compare = func;
		ctx->dirty |= GL_CTX_DIRTY_DEPTH_STENCIL_STATE;
	}
}

void
glDepthMask(GLboolean flag)
{
	struct gl_ctx *ctx = g_ctx;

	flag = !!flag;
	if (ctx->depth_stencil_state.depth_write_enable != flag)
	{
		ctx->depth_stencil_state.depth_write_enable = flag;
		ctx->dirty |= GL_CTX_DIRTY_DEPTH_STENCIL_STATE;
	}
}

static void
depth_range(struct gl_ctx *ctx,
            GLfloat near_val,
            GLfloat far_val)
{
	near_val = clampf(near_val, 0, 1);
	far_val = clampf(far_val, 0, 1);
	if (ctx->depth_range[0] != near_val)
	{
		ctx->depth_range[0] = near_val;
		ctx->dirty |= GL_CTX_DIRTY_VIEWPORT;
	}
	if (ctx->depth_range[1] != far_val)
	{
		ctx->depth_range[1] = far_val;
		ctx->dirty |= GL_CTX_DIRTY_VIEWPORT;
	}
}

void
glDepthRange(GLdouble near_val, GLdouble far_val)
{
	depth_range(g_ctx, near_val, far_val);
}

void
glDepthRangef(GLfloat near_val, GLfloat far_val)
{
	depth_range(g_ctx, near_val, far_val);
}

static void
stencil_mask_separate(struct gl_ctx *ctx,
                      GLenum face,
                      GLuint mask)
{
	switch (face)
	{
		case GL_BACK:
			if (ctx->depth_stencil_state.back.write_mask != mask)
			{
				ctx->depth_stencil_state.back.write_mask = mask;
				ctx->dirty |= GL_CTX_DIRTY_DEPTH_STENCIL_STATE;
			}
			break;
		case GL_FRONT:
			if (ctx->depth_stencil_state.front.write_mask != mask)
			{
				ctx->depth_stencil_state.front.write_mask = mask;
				ctx->dirty |= GL_CTX_DIRTY_DEPTH_STENCIL_STATE;
			}
			break;
		case GL_FRONT_AND_BACK:
			stencil_mask_separate(ctx, GL_BACK, mask);
			stencil_mask_separate(ctx, GL_FRONT, mask);
			break;
		default:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return;
	}
}

void
glStencilMask(GLuint mask)
{
	stencil_mask_separate(g_ctx, GL_FRONT_AND_BACK, mask);
}

void
glStencilMaskSeparate(GLenum face, GLuint mask)
{
	stencil_mask_separate(g_ctx, face, mask);
}

static void
stencil_func_separate(struct gl_ctx *ctx,
                      GLenum face,
                      GLenum func,
                      GLint ref,
                      GLuint mask)
{
	switch (func)
	{
		case GL_NEVER:
		case GL_LESS:
		case GL_LEQUAL:
		case GL_GREATER:
		case GL_GEQUAL:
		case GL_EQUAL:
		case GL_NOTEQUAL:
		case GL_ALWAYS:
			break;
		default:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return;
	}
	ref = clampi(ref, 0, 0xFF);
	switch (face)
	{
		case GL_BACK:
			if (ctx->stencil_ref[0] != ref)
			{
				ctx->stencil_ref[0] = ref;
				ctx->dirty |= GL_CTX_DIRTY_STENCIL_REF;
			}
			if (ctx->depth_stencil_state.back.compare_op != func)
			{
				ctx->depth_stencil_state.back.compare_op = func;
				ctx->dirty |= GL_CTX_DIRTY_DEPTH_STENCIL_STATE;
			}
			if (ctx->depth_stencil_state.back.compare_mask != mask)
			{
				ctx->depth_stencil_state.back.compare_mask = mask;
				ctx->dirty |= GL_CTX_DIRTY_DEPTH_STENCIL_STATE;
			}
			break;
		case GL_FRONT:
			if (ctx->stencil_ref[1] != ref)
			{
				ctx->stencil_ref[1] = ref;
				ctx->dirty |= GL_CTX_DIRTY_STENCIL_REF;
			}
			if (ctx->depth_stencil_state.front.compare_op != func)
			{
				ctx->depth_stencil_state.front.compare_op = func;
				ctx->dirty |= GL_CTX_DIRTY_DEPTH_STENCIL_STATE;
			}
			if (ctx->depth_stencil_state.front.compare_mask != mask)
			{
				ctx->depth_stencil_state.front.compare_mask = mask;
				ctx->dirty |= GL_CTX_DIRTY_DEPTH_STENCIL_STATE;
			}
			break;
		case GL_FRONT_AND_BACK:
			stencil_func_separate(ctx, GL_BACK, func, ref, mask);
			stencil_func_separate(ctx, GL_FRONT, func, ref, mask);
			break;
		default:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return;
	}
}

void
glStencilFunc(GLenum func, GLint ref, GLuint mask)
{
	stencil_func_separate(g_ctx, GL_FRONT_AND_BACK, func, ref, mask);
}

void
glStencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask)
{
	stencil_func_separate(g_ctx, face, func, ref, mask);
}

static bool
is_valid_op(GLenum op)
{
	switch (op)
	{
		case GL_KEEP:
		case GL_ZERO:
		case GL_REPLACE:
		case GL_INCR:
		case GL_INCR_WRAP:
		case GL_DECR:
		case GL_DECR_WRAP:
		case GL_INVERT:
			return true;
		default:
			return false;
	}
}

static void
stencil_op_separate(struct gl_ctx *ctx,
                    GLenum face,
                    GLenum sfail,
                    GLenum dpfail,
                    GLenum dppass)
{
	if (!is_valid_op(sfail)
	 || !is_valid_op(dpfail)
	 || !is_valid_op(dppass))
	{
		GL_SET_ERR(ctx, GL_INVALID_ENUM);
		return;
	}
	switch (face)
	{
		case GL_BACK:
			if (ctx->depth_stencil_state.back.fail_op != sfail)
			{
				ctx->depth_stencil_state.back.fail_op = sfail;
				ctx->dirty |= GL_CTX_DIRTY_DEPTH_STENCIL_STATE;
			}
			if (ctx->depth_stencil_state.back.zfail_op != dpfail)
			{
				ctx->depth_stencil_state.back.zfail_op = dpfail;
				ctx->dirty |= GL_CTX_DIRTY_DEPTH_STENCIL_STATE;
			}
			if (ctx->depth_stencil_state.back.pass_op != dppass)
			{
				ctx->depth_stencil_state.back.pass_op = dppass;
				ctx->dirty |= GL_CTX_DIRTY_DEPTH_STENCIL_STATE;
			}
			break;
		case GL_FRONT:
			if (ctx->depth_stencil_state.front.fail_op != sfail)
			{
				ctx->depth_stencil_state.front.fail_op = sfail;
				ctx->dirty |= GL_CTX_DIRTY_DEPTH_STENCIL_STATE;
			}
			if (ctx->depth_stencil_state.front.zfail_op != dpfail)
			{
				ctx->depth_stencil_state.front.zfail_op = dpfail;
				ctx->dirty |= GL_CTX_DIRTY_DEPTH_STENCIL_STATE;
			}
			if (ctx->depth_stencil_state.front.pass_op != dppass)
			{
				ctx->depth_stencil_state.front.pass_op = dppass;
				ctx->dirty |= GL_CTX_DIRTY_DEPTH_STENCIL_STATE;
			}
			break;
		case GL_FRONT_AND_BACK:
			stencil_op_separate(ctx, GL_BACK, sfail, dpfail, dppass);
			stencil_op_separate(ctx, GL_FRONT, sfail, dpfail, dppass);
			break;
		default:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return;
	}
}

void
glStencilOp(GLenum sfail, GLenum dpfail, GLenum dppass)
{
	stencil_op_separate(g_ctx, GL_FRONT_AND_BACK, sfail, dpfail, dppass);
}

void
glStencilOpSeparate(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass)
{
	stencil_op_separate(g_ctx, face, sfail, dpfail, dppass);
}

static enum jkg_compare_op
get_compare_op(GLenum op)
{
	switch (op)
	{
		default:
			assert(!"unknown compare op");
			/* FALLTHROUGH */
		case GL_NEVER:
			return JKG_COMPARE_NEVER;
		case GL_LESS:
			return JKG_COMPARE_LOWER;
		case GL_LEQUAL:
			return JKG_COMPARE_LEQUAL;
		case GL_EQUAL:
			return JKG_COMPARE_EQUAL;
		case GL_GEQUAL:
			return JKG_COMPARE_GEQUAL;
		case GL_GREATER:
			return JKG_COMPARE_GREATER;
		case GL_NOTEQUAL:
			return JKG_COMPARE_NOTEQUAL;
		case GL_ALWAYS:
			return JKG_COMPARE_ALWAYS;
	}
}

static enum jkg_stencil_op
get_stencil_op(GLenum op)
{
	switch (op)
	{
		default:
			assert(!"unknown stencil op");
			/* FALLTHROUGH */
		case GL_KEEP:
			return JKG_STENCIL_KEEP;
		case GL_ZERO:
			return JKG_STENCIL_ZERO;
		case GL_REPLACE:
			return JKG_STENCIL_REPLACE;
		case GL_INCR:
			return JKG_STENCIL_INC;
		case GL_INCR_WRAP:
			return JKG_STENCIL_INC_WRAP;
		case GL_DECR:
			return JKG_STENCIL_DEC;
		case GL_DECR_WRAP:
			return JKG_STENCIL_DEC_WRAP;
		case GL_INVERT:
			return JKG_STENCIL_INV;
	}
}

static uint32_t
stencil_state_hash(uint32_t hash,
                   const struct stencil_op_state *state)
{
	hash = fnv32v(hash, &state->fail_op, sizeof(state->fail_op));
	hash = fnv32v(hash, &state->pass_op, sizeof(state->pass_op));
	hash = fnv32v(hash, &state->zfail_op, sizeof(state->zfail_op));
	hash = fnv32v(hash, &state->compare_op, sizeof(state->compare_op));
	hash = fnv32v(hash, &state->compare_mask, sizeof(state->compare_mask));
	hash = fnv32v(hash, &state->write_mask, sizeof(state->write_mask));
	return hash;
}

static uint32_t
depth_stencil_state_hash(struct gl_ctx *ctx,
                         const void *st)
{
	const struct depth_stencil_state *state = st;
	uint32_t hash;

	(void)ctx;
	hash = FNV_BASIS32;
	hash = fnv32v(hash, &state->depth_test_enable, sizeof(state->depth_test_enable));
	hash = fnv32v(hash, &state->depth_write_enable, sizeof(state->depth_write_enable));
	hash = fnv32v(hash, &state->stencil_enable, sizeof(state->stencil_enable));
	if (state->depth_test_enable)
		hash = fnv32v(hash, &state->depth_compare, sizeof(state->depth_compare));
	if (state->stencil_enable)
	{
		hash = stencil_state_hash(hash, &state->front);
		hash = stencil_state_hash(hash, &state->back);
	}
	return hash;
}

static bool
stencil_state_eq(const struct stencil_op_state *a,
                 const struct stencil_op_state *b)
{
	if (a->fail_op != b->fail_op
	 || a->zfail_op != b->zfail_op
	 || a->pass_op != b->pass_op
	 || a->compare_op != b->compare_op
	 || a->compare_mask != b->compare_mask
	 || a->write_mask != b->write_mask)
		return false;
	return true;
}

static bool
depth_stencil_state_eq(struct gl_ctx *ctx,
                       const struct cache_entry *entry,
                       const void *state)
{
	const struct depth_stencil_state *a = &((struct depth_stencil_entry*)entry)->state;
	const struct depth_stencil_state *b = state;

	(void)ctx;
	if (a->depth_test_enable != b->depth_test_enable
	 || a->depth_write_enable != b->depth_write_enable
	 || a->stencil_enable != b->stencil_enable)
		return false;
	if (a->depth_test_enable
	 && (a->depth_compare != b->depth_compare))
		return false;
	if (a->stencil_enable
	 && (!stencil_state_eq(&a->front, &b->front)
	  || !stencil_state_eq(&a->back, &b->back)))
		return false;
	return true;
}

static void
convert_stencil_state(struct jkg_stencil_op_state *jkg_state,
                      const struct stencil_op_state *state)
{
	jkg_state->fail_op = get_stencil_op(state->fail_op);
	jkg_state->zfail_op = get_stencil_op(state->zfail_op);
	jkg_state->pass_op = get_stencil_op(state->pass_op);
	jkg_state->compare_op = get_compare_op(state->compare_op);
	jkg_state->compare_mask = state->compare_mask;
	jkg_state->write_mask = state->write_mask;
}

static struct cache_entry *
depth_stencil_entry_alloc(struct gl_ctx *ctx,
                          uint32_t hash,
                          const void *st)
{
	struct jkg_depth_stencil_state_create_info create_info;
	const struct depth_stencil_state *state = st;
	struct depth_stencil_entry *entry;
	int ret;

	entry = malloc(sizeof(*entry));
	if (!entry)
	{
		GL_SET_ERR(ctx, GL_OUT_OF_MEMORY);
		return NULL;
	}
	memcpy(&entry->state, state, sizeof(*state));
	entry->cache.hash = hash;
	entry->cache.ref = 0;
	create_info.depth_test_enable = state->depth_test_enable;
	create_info.depth_write_enable = state->depth_write_enable;
	create_info.stencil_enable = state->stencil_enable;
	create_info.depth_compare = get_compare_op(state->depth_compare);
	convert_stencil_state(&create_info.front, &state->front);
	convert_stencil_state(&create_info.back, &state->back);
	ret = ctx->jkg_op->depth_stencil_state_alloc(ctx->jkg_ctx,
	                                             &create_info,
	                                             &entry->depth_stencil_state);
	if (ret)
	{
		GL_SET_ERR(ctx, get_jkg_error(ret));
		free(entry);
		return NULL;
	}
	return &entry->cache;
}

static void
depth_stencil_entry_free(struct gl_ctx *ctx,
                         struct cache_entry *entry)
{
	ctx->jkg_op->depth_stencil_state_free(ctx->jkg_ctx,
	                                      ((struct depth_stencil_entry*)entry)->depth_stencil_state);
}

const struct cache_op
depth_stencil_cache_op =
{
	.alloc = depth_stencil_entry_alloc,
	.free = depth_stencil_entry_free,
	.hash = depth_stencil_state_hash,
	.eq = depth_stencil_state_eq,
};
