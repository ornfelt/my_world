#include "internal.h"
#include "fnv.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

void
glBlendColor(GLfloat red,
             GLfloat green,
             GLfloat blue,
             GLfloat alpha)
{
	struct gl_ctx *ctx = g_ctx;

	ctx->blend_color[0] = clampf(red, 0, 1);
	ctx->blend_color[1] = clampf(green, 0, 1);
	ctx->blend_color[2] = clampf(blue, 0, 1);
	ctx->blend_color[3] = clampf(alpha, 0, 1);
	ctx->dirty |= GL_CTX_DIRTY_BLEND_COLOR;
}

static bool
is_valid_equation(GLenum mode)
{
	switch (mode)
	{
		case GL_FUNC_ADD:
		case GL_FUNC_SUBTRACT:
		case GL_FUNC_REVERSE_SUBTRACT:
		case GL_MIN:
		case GL_MAX:
			return true;
		default:
			return false;
	}
}

static void
blend_equation(struct gl_ctx *ctx,
               GLuint buf,
               GLenum mode_rgb,
               GLenum mode_alpha)
{
	if (ctx->blend_state.attachments[buf].equation_rgb != mode_rgb)
	{
		ctx->blend_state.attachments[buf].equation_rgb = mode_rgb;
		ctx->dirty |= GL_CTX_DIRTY_BLEND_STATE;
	}
	if (ctx->blend_state.attachments[buf].equation_alpha != mode_alpha)
	{
		ctx->blend_state.attachments[buf].equation_alpha = mode_alpha;
		ctx->dirty |= GL_CTX_DIRTY_BLEND_STATE;
	}
}

void
glBlendEquation(GLenum mode)
{
	struct gl_ctx *ctx = g_ctx;

	if (!is_valid_equation(mode))
	{
		GL_SET_ERR(ctx, GL_INVALID_ENUM);
		return;
	}
	for (GLuint i = 0; i < ctx->jkg_caps->max_draw_buffers; ++i)
		blend_equation(ctx, i, mode, mode);
}

void
glBlendEquationi(GLuint buf, GLenum mode)
{
	struct gl_ctx *ctx = g_ctx;

	if (!is_valid_equation(mode))
	{
		GL_SET_ERR(ctx, GL_INVALID_ENUM);
		return;
	}
	if (buf >= ctx->jkg_caps->max_draw_buffers)
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	blend_equation(ctx, buf, mode, mode);
}

void
glBlendEquationSeparate(GLenum mode_rgb, GLenum mode_alpha)
{
	struct gl_ctx *ctx = g_ctx;

	if (!is_valid_equation(mode_rgb)
	 || !is_valid_equation(mode_alpha))
	{
		GL_SET_ERR(ctx, GL_INVALID_ENUM);
		return;
	}
	for (GLuint i = 0; i < ctx->jkg_caps->max_draw_buffers; ++i)
		blend_equation(ctx, i, mode_rgb, mode_alpha);
}

void
glBlendEquationSeparatei(GLuint buf, GLenum mode_rgb, GLenum mode_alpha)
{
	struct gl_ctx *ctx = g_ctx;

	if (!is_valid_equation(mode_rgb)
	 || !is_valid_equation(mode_alpha))
	{
		GL_SET_ERR(ctx, GL_INVALID_ENUM);
		return;
	}
	if (buf >= ctx->jkg_caps->max_draw_buffers)
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	blend_equation(ctx, buf, mode_rgb, mode_alpha);
}

static bool
is_valid_factor(GLenum factor)
{
	switch (factor)
	{
		case GL_ZERO:
		case GL_ONE:
		case GL_SRC_COLOR:
		case GL_ONE_MINUS_SRC_COLOR:
		case GL_DST_COLOR:
		case GL_ONE_MINUS_DST_COLOR:
		case GL_SRC_ALPHA:
		case GL_ONE_MINUS_SRC_ALPHA:
		case GL_DST_ALPHA:
		case GL_ONE_MINUS_DST_ALPHA:
		case GL_CONSTANT_COLOR:
		case GL_ONE_MINUS_CONSTANT_COLOR:
		case GL_CONSTANT_ALPHA:
		case GL_ONE_MINUS_CONSTANT_ALPHA:
		case GL_SRC_ALPHA_SATURATE:
			return true;
		default:
			return false;
	}
}

static void
blend_func(struct gl_ctx *ctx,
           GLuint buf,
           GLenum src_rgb,
           GLenum dst_rgb,
           GLenum src_alpha,
           GLenum dst_alpha)
{
	if (ctx->blend_state.attachments[buf].src_rgb != src_rgb)
	{
		ctx->blend_state.attachments[buf].src_rgb = src_rgb;
		ctx->dirty |= GL_CTX_DIRTY_BLEND_STATE;
	}
	if (ctx->blend_state.attachments[buf].src_alpha != src_alpha)
	{
		ctx->blend_state.attachments[buf].src_alpha = src_alpha;
		ctx->dirty |= GL_CTX_DIRTY_BLEND_STATE;
	}
	if (ctx->blend_state.attachments[buf].dst_rgb != dst_rgb)
	{
		ctx->blend_state.attachments[buf].dst_rgb = dst_rgb;
		ctx->dirty |= GL_CTX_DIRTY_BLEND_STATE;
	}
	if (ctx->blend_state.attachments[buf].dst_alpha != dst_alpha)
	{
		ctx->blend_state.attachments[buf].dst_alpha = dst_alpha;
		ctx->dirty |= GL_CTX_DIRTY_BLEND_STATE;
	}
}

void
glBlendFunc(GLenum s_factor, GLenum d_factor)
{
	struct gl_ctx *ctx = g_ctx;

	if (!is_valid_factor(s_factor)
	 || !is_valid_factor(d_factor))
	{
		GL_SET_ERR(ctx, GL_INVALID_ENUM);
		return;
	}
	for (GLuint i = 0; i < ctx->jkg_caps->max_draw_buffers; ++i)
		blend_func(ctx, i, s_factor, d_factor, s_factor, d_factor);
}

void
glBlendFunci(GLuint buf, GLenum s_factor, GLenum d_factor)
{
	struct gl_ctx *ctx = g_ctx;

	if (!is_valid_factor(s_factor)
	 || !is_valid_factor(d_factor))
	{
		GL_SET_ERR(ctx, GL_INVALID_ENUM);
		return;
	}
	if (buf >= ctx->jkg_caps->max_draw_buffers)
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	blend_func(ctx, buf, s_factor, d_factor, s_factor, d_factor);
}

void
glBlendFuncSeparate(GLenum src_rgb,
                    GLenum dst_rgb,
                    GLenum src_alpha,
                    GLenum dst_alpha)
{
	struct gl_ctx *ctx = g_ctx;

	if (!is_valid_factor(src_rgb)
	 || !is_valid_factor(dst_rgb)
	 || !is_valid_factor(src_alpha)
	 || !is_valid_factor(dst_alpha))
	{
		GL_SET_ERR(ctx, GL_INVALID_ENUM);
		return;
	}
	for (GLuint i = 0; i < ctx->jkg_caps->max_draw_buffers; ++i)
		blend_func(ctx, i, src_rgb, dst_rgb, src_alpha, dst_alpha);
}

void
glBlendFuncSeparatei(GLuint buf,
                     GLenum src_rgb,
                     GLenum dst_rgb,
                     GLenum src_alpha,
                     GLenum dst_alpha)
{
	struct gl_ctx *ctx = g_ctx;

	if (!is_valid_factor(src_rgb)
	 || !is_valid_factor(dst_rgb)
	 || !is_valid_factor(src_alpha)
	 || !is_valid_factor(dst_alpha))
	{
		GL_SET_ERR(ctx, GL_INVALID_ENUM);
		return;
	}
	if (buf >= ctx->jkg_caps->max_draw_buffers)
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	blend_func(ctx, buf, src_rgb, dst_rgb, src_alpha, dst_alpha);
}

void
glLogicOp(GLenum opcode)
{
	struct gl_ctx *ctx = g_ctx;

	switch (opcode)
	{
		case GL_CLEAR:
		case GL_AND:
		case GL_AND_REVERSE:
		case GL_COPY:
		case GL_AND_INVERTED:
		case GL_NOOP:
		case GL_XOR:
		case GL_OR:
		case GL_NOR:
		case GL_EQUIV:
		case GL_INVERT:
		case GL_OR_REVERSE:
		case GL_COPY_INVERTED:
		case GL_OR_INVERTED:
		case GL_NAND:
		case GL_SET:
			break;
		default:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return;
	}
	if (ctx->blend_state.logic_op != opcode)
	{
		ctx->blend_state.logic_op = opcode;
		ctx->dirty |= GL_CTX_DIRTY_BLEND_STATE;
	}
}

static void
color_mask(struct gl_ctx *ctx,
           GLuint buf,
           GLboolean red,
           GLboolean green,
           GLboolean blue,
           GLboolean alpha)
{
	if (ctx->blend_state.attachments[buf].color_mask[0] != red)
	{
		ctx->blend_state.attachments[buf].color_mask[0] = red;
		ctx->dirty |= GL_CTX_DIRTY_BLEND_STATE;
	}
	if (ctx->blend_state.attachments[buf].color_mask[1] != green)
	{
		ctx->blend_state.attachments[buf].color_mask[1] = green;
		ctx->dirty |= GL_CTX_DIRTY_BLEND_STATE;
	}
	if (ctx->blend_state.attachments[buf].color_mask[2] != blue)
	{
		ctx->blend_state.attachments[buf].color_mask[2] = blue;
		ctx->dirty |= GL_CTX_DIRTY_BLEND_STATE;
	}
	if (ctx->blend_state.attachments[buf].color_mask[3] != alpha)
	{
		ctx->blend_state.attachments[buf].color_mask[3] = alpha;
		ctx->dirty |= GL_CTX_DIRTY_BLEND_STATE;
	}
}

void
glColorMask(GLboolean red,
            GLboolean green,
            GLboolean blue,
            GLboolean alpha)
{
	struct gl_ctx *ctx = g_ctx;

	red = !!red;
	green = !!green;
	blue = !!blue;
	alpha = !!alpha;
	for (GLuint i = 0; i < ctx->jkg_caps->max_draw_buffers; ++i)
		color_mask(ctx, i, red, green, blue, alpha);
}

void
glColorMaski(GLuint buf,
             GLboolean red,
             GLboolean green,
             GLboolean blue,
             GLboolean alpha)
{
	struct gl_ctx *ctx = g_ctx;

	if (buf >= ctx->jkg_caps->max_draw_buffers)
	{
		GL_SET_ERR(ctx, GL_INVALID_VALUE);
		return;
	}
	red = !!red;
	green = !!green;
	blue = !!blue;
	alpha = !!alpha;
	color_mask(ctx, buf, red, green, blue, alpha);
}

static enum jkg_logic_op
get_logic_op(GLenum op)
{
	switch (op)
	{
		default:
			assert(!"unknown logic op");
			/* FALLTHROUGH */
		case GL_CLEAR:
			return JKG_LOGIC_CLEAR;
		case GL_SET:
			return JKG_LOGIC_SET;
		case GL_COPY:
			return JKG_LOGIC_COPY;
		case GL_COPY_INVERTED:
			return JKG_LOGIC_COPY_INVERTED;
		case GL_NOOP:
			return JKG_LOGIC_NOOP;
		case GL_INVERT:
			return JKG_LOGIC_INVERT;
		case GL_AND:
			return JKG_LOGIC_AND;
		case GL_NAND:
			return JKG_LOGIC_NAND;
		case GL_OR:
			return JKG_LOGIC_OR;
		case GL_NOR:
			return JKG_LOGIC_NOR;
		case GL_XOR:
			return JKG_LOGIC_XOR;
		case GL_EQUIV:
			return JKG_LOGIC_EQUIV;
		case GL_AND_REVERSE:
			return JKG_LOGIC_AND_REVERSE;
		case GL_AND_INVERTED:
			return JKG_LOGIC_AND_INVERTED;
		case GL_OR_REVERSE:
			return JKG_LOGIC_AND_REVERSE;
		case GL_OR_INVERTED:
			return JKG_LOGIC_AND_INVERTED;
	}
}

static enum jkg_blend_fn
get_blend_fn(GLenum fn)
{
	switch (fn)
	{
		default:
			assert(!"unknown blend fn");
			/* FALLTHROUGH */
		case GL_ZERO:
			return JKG_BLEND_ZERO;
		case GL_ONE:
			return JKG_BLEND_ONE;
		case GL_SRC_COLOR:
			return JKG_BLEND_SRC_COLOR;
		case GL_ONE_MINUS_SRC_COLOR:
			return JKG_BLEND_ONE_MINUS_SRC_COLOR;
		case GL_DST_COLOR:
			return JKG_BLEND_DST_COLOR;
		case GL_ONE_MINUS_DST_COLOR:
			return JKG_BLEND_ONE_MINUS_DST_COLOR;
		case GL_SRC_ALPHA:
			return JKG_BLEND_SRC_ALPHA;
		case GL_ONE_MINUS_SRC_ALPHA:
			return JKG_BLEND_ONE_MINUS_SRC_ALPHA;
		case GL_DST_ALPHA:
			return JKG_BLEND_DST_ALPHA;
		case GL_ONE_MINUS_DST_ALPHA:
			return JKG_BLEND_ONE_MINUS_DST_ALPHA;
		case GL_CONSTANT_COLOR:
			return JKG_BLEND_CONSTANT_COLOR;
		case GL_ONE_MINUS_CONSTANT_COLOR:
			return JKG_BLEND_ONE_MINUS_CONSTANT_COLOR;
		case GL_CONSTANT_ALPHA:
			return JKG_BLEND_CONSTANT_ALPHA;
		case GL_ONE_MINUS_CONSTANT_ALPHA:
			return JKG_BLEND_ONE_MINUS_CONSTANT_ALPHA;
		case GL_SRC_ALPHA_SATURATE:
			return JKG_BLEND_SRC_ALPHA_SATURATE;
		case GL_SRC1_COLOR:
			return JKG_BLEND_SRC1_COLOR;
		case GL_ONE_MINUS_SRC1_COLOR:
			return JKG_BLEND_ONE_MINUS_SRC1_COLOR;
		case GL_SRC1_ALPHA:
			return JKG_BLEND_SRC1_ALPHA;
		case GL_ONE_MINUS_SRC1_ALPHA:
			return JKG_BLEND_ONE_MINUS_SRC1_ALPHA;
	}
}

static enum jkg_blend_eq
get_blend_equation(GLenum eq)
{
	switch (eq)
	{
		default:
			assert(!"unknown blend equation");
		case GL_FUNC_ADD:
			return JKG_EQUATION_ADD;
		case GL_FUNC_SUBTRACT:
			return JKG_EQUATION_SUBTRACT;
		case GL_FUNC_REVERSE_SUBTRACT:
			return JKG_EQUATION_REV_SUBTRACT;
		case GL_MIN:
			return JKG_EQUATION_MIN;
		case GL_MAX:
			return JKG_EQUATION_MAX;
	}
}

static enum jkg_color_mask
get_color_mask(const GLboolean color_mask[4])
{
	enum jkg_color_mask ret = 0;

	if (color_mask[0])
		ret |= JKG_COLOR_MASK_R;
	if (color_mask[1])
		ret |= JKG_COLOR_MASK_G;
	if (color_mask[2])
		ret |= JKG_COLOR_MASK_B;
	if (color_mask[3])
		ret |= JKG_COLOR_MASK_A;
	return ret;
}

static uint32_t
blend_attachment_hash(uint32_t hash,
                      const struct blend_attachment *attachment)
{
	hash = fnv32v(hash, &attachment->enable, sizeof(attachment->enable));
	hash = fnv32v(hash, &attachment->color_mask, sizeof(attachment->color_mask));
	if (attachment->enable)
	{
		hash = fnv32v(hash, &attachment->src_rgb, sizeof(attachment->src_rgb));
		hash = fnv32v(hash, &attachment->src_alpha, sizeof(attachment->src_alpha));
		hash = fnv32v(hash, &attachment->dst_rgb, sizeof(attachment->dst_rgb));
		hash = fnv32v(hash, &attachment->dst_alpha, sizeof(attachment->dst_alpha));
		hash = fnv32v(hash, &attachment->equation_rgb, sizeof(attachment->equation_rgb));
		hash = fnv32v(hash, &attachment->equation_alpha, sizeof(attachment->equation_alpha));
	}
	return hash;
}

static uint32_t
blend_state_hash(struct gl_ctx *ctx,
                 const void *st)
{
	const struct blend_state *state = st;
	uint32_t hash;

	hash = FNV_BASIS32;
	for (GLuint i = 0; i < ctx->jkg_caps->max_draw_buffers; ++i)
		hash = blend_attachment_hash(hash, &state->attachments[i]);
	hash = fnv32v(hash, &state->logic_enable, sizeof(state->logic_enable));
	if (state->logic_enable)
		hash = fnv32v(hash, &state->logic_op, sizeof(state->logic_op));
	return hash;
}

static bool
blend_attachment_eq(const struct blend_attachment *a,
                    const struct blend_attachment *b)
{
	if (a->enable         != b->enable
	 || a->color_mask[0]  != b->color_mask[0]
	 || a->color_mask[1]  != b->color_mask[1]
	 || a->color_mask[2]  != b->color_mask[2]
	 || a->color_mask[3]  != b->color_mask[3])
		return false;
	if (a->enable
	 && (a->src_rgb        != b->src_rgb
	  || a->src_alpha      != b->src_alpha
	  || a->dst_rgb        != b->dst_rgb
	  || a->dst_alpha      != b->dst_alpha
	  || a->equation_rgb   != b->equation_rgb
	  || a->equation_alpha != b->equation_alpha))
		return false;
	return true;
}

static bool
blend_state_eq(struct gl_ctx *ctx,
               const struct cache_entry *entry,
               const void *state)
{
	const struct blend_state *a = &((struct blend_entry*)entry)->state;
	const struct blend_state *b = state;

	if (a->logic_enable != b->logic_enable)
		return false;
	if (a->logic_enable && a->logic_op != b->logic_op)
		return false;
	for (uint32_t i = 0; i < ctx->jkg_caps->max_draw_buffers; ++i)
	{
		if (!blend_attachment_eq(&a->attachments[i],
		                         &b->attachments[i]))
			return false;
	}
	return true;
}

static struct cache_entry *
blend_entry_alloc(struct gl_ctx *ctx,
                  uint32_t hash,
                  const void *st)
{
	struct jkg_blend_attachment jkg_attachments[MAX_COLOR_ATTACHMENTS];
	struct jkg_blend_state_create_info create_info;
	const struct blend_state *state = st;
	struct blend_entry *entry;
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
	create_info.logic_enable = state->logic_enable;
	create_info.logic_op = get_logic_op(state->logic_op);
	create_info.attachments = jkg_attachments;
	create_info.nattachments = ctx->jkg_caps->max_draw_buffers;
	for (GLuint i = 0; i < ctx->jkg_caps->max_draw_buffers; ++i)
	{
		const struct blend_attachment *attachment = &state->attachments[i];
		jkg_attachments[i].enable = attachment->enable;
		jkg_attachments[i].rgb_src = get_blend_fn(attachment->src_rgb);
		jkg_attachments[i].rgb_dst = get_blend_fn(attachment->dst_rgb);
		jkg_attachments[i].rgb_eq = get_blend_equation(attachment->equation_rgb);
		jkg_attachments[i].alpha_src = get_blend_fn(attachment->src_alpha);
		jkg_attachments[i].alpha_dst = get_blend_fn(attachment->dst_alpha);
		jkg_attachments[i].alpha_eq = get_blend_equation(attachment->equation_alpha);
		jkg_attachments[i].color_mask = get_color_mask(attachment->color_mask);
	}
	ret = ctx->jkg_op->blend_state_alloc(ctx->jkg_ctx,
	                                     &create_info,
	                                     &entry->blend_state);
	if (ret)
	{
		GL_SET_ERR(ctx, get_jkg_error(ret));
		free(entry);
		return NULL;
	}
	return &entry->cache;
}

static void
blend_entry_free(struct gl_ctx *ctx, struct cache_entry *entry)
{
	ctx->jkg_op->blend_state_free(ctx->jkg_ctx,
	                              ((struct blend_entry*)entry)->blend_state);
}

const struct cache_op
blend_cache_op =
{
	.alloc = blend_entry_alloc,
	.free = blend_entry_free,
	.hash = blend_state_hash,
	.eq = blend_state_eq,
};
