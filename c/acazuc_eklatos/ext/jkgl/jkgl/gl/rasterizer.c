#include "internal.h"
#include "fnv.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>

void
glLineWidth(GLfloat width)
{
	struct gl_ctx *ctx = g_ctx;

	if (width <= 0)
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	if (width > ctx->jkg_caps->max_line_width)
		width = ctx->jkg_caps->max_line_width;
	if (ctx->rasterizer_state.line_width != width)
	{
		ctx->rasterizer_state.line_width = width;
		ctx->dirty |= GL_CTX_DIRTY_RASTERIZER_STATE;
	}
}

void
glPointSize(GLfloat size)
{
	struct gl_ctx *ctx = g_ctx;

	if (size <= 0)
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	if (size > ctx->jkg_caps->max_point_size)
		size = ctx->jkg_caps->max_point_size;
	if (ctx->rasterizer_state.point_size != size)
	{
		ctx->rasterizer_state.point_size = size;
		ctx->dirty |= GL_CTX_DIRTY_RASTERIZER_STATE;
	}
}

void
glFrontFace(GLenum mode)
{
	struct gl_ctx *ctx = g_ctx;

	switch (mode)
	{
		case GL_CW:
		case GL_CCW:
			if (ctx->rasterizer_state.front_face != mode)
			{
				ctx->rasterizer_state.front_face = mode;
				ctx->dirty |= GL_CTX_DIRTY_RASTERIZER_STATE;
			}
			break;
		default:
			GL_SET_ERR(ctx, GL_INVALID_VALUE);
			return;
	}
}

void
glCullFace(GLenum mode)
{
	struct gl_ctx *ctx = g_ctx;

	switch (mode)
	{
		case GL_FRONT:
		case GL_BACK:
		case GL_FRONT_AND_BACK:
			if (ctx->rasterizer_state.cull_face != mode)
			{
				ctx->rasterizer_state.cull_face = mode;
				ctx->dirty |= GL_CTX_DIRTY_RASTERIZER_STATE;
			}
			break;
		default:
			GL_SET_ERR(ctx, GL_INVALID_VALUE);
			return;
	}
}

static void
polygon_mode(struct gl_ctx *ctx, GLenum face, GLenum mode)
{
	switch (face)
	{
		case GL_FRONT:
			if (ctx->rasterizer_state.fill_front != mode)
			{
				ctx->rasterizer_state.fill_front = mode;
				ctx->dirty |= GL_CTX_DIRTY_RASTERIZER_STATE;
			}
			break;
		case GL_BACK:
			if (ctx->rasterizer_state.fill_back != mode)
			{
				ctx->rasterizer_state.fill_back = mode;
				ctx->dirty |= GL_CTX_DIRTY_RASTERIZER_STATE;
			}
			break;
	}
}

void
glPolygonMode(GLenum face, GLenum mode)
{
	struct gl_ctx *ctx = g_ctx;

	switch (mode)
	{
		case GL_POINT:
		case GL_LINE:
		case GL_FILL:
			break;
		default:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return;
	}
	switch (face)
	{
		case GL_FRONT:
		case GL_BACK:
			polygon_mode(ctx, face, mode);
			break;
		case GL_FRONT_AND_BACK:
			polygon_mode(ctx, GL_FRONT, mode);
			polygon_mode(ctx, GL_BACK, mode);
			break;
	}
}

static enum jkg_cull_face
get_cull_face(const struct rasterizer_state *state)
{
	if (!state->cull_enable)
		return 0;
	switch (state->cull_face)
	{
		case GL_FRONT:
			return JKG_CULL_FRONT;
		default:
			assert(!"unknown cull face");
			/* FALLTHROUGH */
		case GL_BACK:
			return JKG_CULL_BACK;
		case GL_FRONT_AND_BACK:
			return JKG_CULL_FRONT | JKG_CULL_BACK;
	}
}

static enum jkg_fill_mode
get_fill_mode(GLenum mode)
{
	switch (mode)
	{
		case GL_POINT:
			return JKG_FILL_POINT;
		case GL_LINE:
			return JKG_FILL_LINE;
		default:
			assert(!"unknown fill mode");
			/* FALLTHROUGH */
		case GL_FILL:
			return JKG_FILL_SOLID;
	}
}

static uint32_t
rasterizer_state_hash(struct gl_ctx *ctx,
                      const void *st)
{
	const struct rasterizer_state *state = st;
	uint32_t hash;

	(void)ctx;
	hash = FNV_BASIS32;
	hash = fnv32v(hash, &state->point_size, sizeof(state->point_size));
	hash = fnv32v(hash, &state->line_width, sizeof(state->line_width));
	hash = fnv32v(hash, &state->point_smooth_enable, sizeof(state->point_smooth_enable));
	hash = fnv32v(hash, &state->line_smooth_enable, sizeof(state->line_smooth_enable));
	hash = fnv32v(hash, &state->scissor_enable, sizeof(state->scissor_enable));
	hash = fnv32v(hash, &state->depth_clamp_enable, sizeof(state->depth_clamp_enable));
	hash = fnv32v(hash, &state->rasterizer_discard_enable, sizeof(state->rasterizer_discard_enable));
	hash = fnv32v(hash, &state->multisample_enable, sizeof(state->multisample_enable));
	hash = fnv32v(hash, &state->front_face, sizeof(state->front_face));
	hash = fnv32v(hash, &state->cull_enable, sizeof(state->cull_enable));
	hash = fnv32v(hash, &state->fill_front, sizeof(state->fill_front));
	hash = fnv32v(hash, &state->fill_back, sizeof(state->fill_back));
	if (state->cull_enable)
		hash = fnv32v(hash, &state->cull_face, sizeof(state->cull_face));
	return hash;
}

static bool
rasterizer_state_eq(struct gl_ctx *ctx,
                    const struct cache_entry *entry,
                    const void *state)
{
	const struct rasterizer_state *a = &((struct rasterizer_entry*)entry)->state;
	const struct rasterizer_state *b = state;

	(void)ctx;
	if (a->point_size != b->point_size
	 || a->line_width != b->line_width
	 || a->point_smooth_enable != b->point_smooth_enable
	 || a->line_smooth_enable != b->line_smooth_enable
	 || a->scissor_enable != b->scissor_enable
	 || a->depth_clamp_enable != b->depth_clamp_enable
	 || a->rasterizer_discard_enable != b->rasterizer_discard_enable
	 || a->multisample_enable != b->multisample_enable
	 || a->front_face != b->front_face
	 || a->cull_enable != b->cull_enable
	 || a->fill_front != b->fill_front
	 || a->fill_back != b->fill_back)
		return false;
	if (a->cull_enable && a->cull_face != b->cull_face)
		return false;
	return true;
}

static struct cache_entry *
rasterizer_entry_alloc(struct gl_ctx *ctx,
                       uint32_t hash,
                       const void *st)
{
	struct jkg_rasterizer_state_create_info create_info;
	const struct rasterizer_state *state = st;
	struct rasterizer_entry *entry;
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
	create_info.point_size = state->point_size;
	create_info.line_width = state->line_width;
	create_info.point_smooth_enable = state->point_smooth_enable;
	create_info.line_smooth_enable = state->line_smooth_enable;
	create_info.scissor_enable = state->scissor_enable;
	create_info.depth_clamp_enable = state->depth_clamp_enable;
	create_info.rasterizer_discard_enable = state->rasterizer_discard_enable;
	create_info.multisample_enable = state->multisample_enable;
	create_info.front_ccw = state->front_face == GL_CCW;
	create_info.cull_face = get_cull_face(state);
	create_info.fill_front = get_fill_mode(state->fill_front);
	create_info.fill_back = get_fill_mode(state->fill_back);
	ret = ctx->jkg_op->rasterizer_state_alloc(ctx->jkg_ctx,
	                                          &create_info,
	                                          &entry->rasterizer_state);
	if (ret)
	{
		GL_SET_ERR(ctx, get_jkg_error(ret));
		free(entry);
		return NULL;
	}
	return &entry->cache;
}

static void
rasterizer_entry_free(struct gl_ctx *ctx, struct cache_entry *entry)
{
	ctx->jkg_op->rasterizer_state_free(ctx->jkg_ctx,
	                                   ((struct rasterizer_entry*)entry)->rasterizer_state);
}

const struct cache_op
rasterizer_cache_op =
{
	.alloc = rasterizer_entry_alloc,
	.free = rasterizer_entry_free,
	.hash = rasterizer_state_hash,
	.eq = rasterizer_state_eq,
};
