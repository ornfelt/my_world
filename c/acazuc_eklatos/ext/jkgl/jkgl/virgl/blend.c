#include "virgl.h"

#include <stdlib.h>
#include <assert.h>
#include <errno.h>

static int
get_logic_op(enum jkg_logic_op op)
{
	switch (op)
	{
		default:
			assert(!"unknown logic op");
			/* FALLTHROUGH */
		case JKG_LOGIC_CLEAR:
			return PIPE_LOGICOP_CLEAR;
		case JKG_LOGIC_SET:
			return PIPE_LOGICOP_SET;
		case JKG_LOGIC_COPY:
			return PIPE_LOGICOP_COPY;
		case JKG_LOGIC_COPY_INVERTED:
			return PIPE_LOGICOP_COPY_INVERTED;
		case JKG_LOGIC_NOOP:
			return PIPE_LOGICOP_NOOP;
		case JKG_LOGIC_INVERT:
			return PIPE_LOGICOP_INVERT;
		case JKG_LOGIC_AND:
			return PIPE_LOGICOP_AND;
		case JKG_LOGIC_NAND:
			return PIPE_LOGICOP_NAND;
		case JKG_LOGIC_OR:
			return PIPE_LOGICOP_OR;
		case JKG_LOGIC_NOR:
			return PIPE_LOGICOP_NOR;
		case JKG_LOGIC_XOR:
			return PIPE_LOGICOP_XOR;
		case JKG_LOGIC_EQUIV:
			return PIPE_LOGICOP_EQUIV;
		case JKG_LOGIC_AND_REVERSE:
			return PIPE_LOGICOP_AND_REVERSE;
		case JKG_LOGIC_AND_INVERTED:
			return PIPE_LOGICOP_AND_INVERTED;
		case JKG_LOGIC_OR_REVERSE:
			return PIPE_LOGICOP_OR_REVERSE;
		case JKG_LOGIC_OR_INVERTED:
			return PIPE_LOGICOP_OR_INVERTED;
	}
}

static enum pipe_blendfactor
get_blend_factor(enum jkg_blend_fn fn)
{
	switch (fn)
	{
		default:
			assert(!"unknown blend factor");
			/* FALLTHROUGH */
		case JKG_BLEND_ZERO:
			return PIPE_BLENDFACTOR_ZERO;
		case JKG_BLEND_ONE:
			return PIPE_BLENDFACTOR_ONE;
		case JKG_BLEND_SRC_COLOR:
			return PIPE_BLENDFACTOR_SRC_COLOR;
		case JKG_BLEND_ONE_MINUS_SRC_COLOR:
			return PIPE_BLENDFACTOR_INV_SRC_COLOR;
		case JKG_BLEND_DST_COLOR:
			return PIPE_BLENDFACTOR_DST_COLOR;
		case JKG_BLEND_ONE_MINUS_DST_COLOR:
			return PIPE_BLENDFACTOR_INV_DST_COLOR;
		case JKG_BLEND_SRC_ALPHA:
			return PIPE_BLENDFACTOR_SRC_ALPHA;
		case JKG_BLEND_ONE_MINUS_SRC_ALPHA:
			return PIPE_BLENDFACTOR_INV_SRC_ALPHA;
		case JKG_BLEND_DST_ALPHA:
			return PIPE_BLENDFACTOR_DST_ALPHA;
		case JKG_BLEND_ONE_MINUS_DST_ALPHA:
			return PIPE_BLENDFACTOR_INV_DST_ALPHA;
		case JKG_BLEND_CONSTANT_COLOR:
			return PIPE_BLENDFACTOR_CONST_COLOR;
		case JKG_BLEND_ONE_MINUS_CONSTANT_COLOR:
			return PIPE_BLENDFACTOR_INV_CONST_COLOR;
		case JKG_BLEND_CONSTANT_ALPHA:
			return PIPE_BLENDFACTOR_CONST_ALPHA;
		case JKG_BLEND_ONE_MINUS_CONSTANT_ALPHA:
			return PIPE_BLENDFACTOR_INV_CONST_ALPHA;
		case JKG_BLEND_SRC_ALPHA_SATURATE:
			return PIPE_BLENDFACTOR_SRC_ALPHA_SATURATE;
		case JKG_BLEND_SRC1_COLOR:
			return PIPE_BLENDFACTOR_SRC1_COLOR;
		case JKG_BLEND_ONE_MINUS_SRC1_COLOR:
			return PIPE_BLENDFACTOR_INV_SRC1_COLOR;
		case JKG_BLEND_SRC1_ALPHA:
			return PIPE_BLENDFACTOR_SRC1_ALPHA;
		case JKG_BLEND_ONE_MINUS_SRC1_ALPHA:
			return PIPE_BLENDFACTOR_INV_SRC1_ALPHA;
	}
}

static enum pipe_blend_func
get_blend_eq(enum jkg_blend_eq eq)
{
	switch (eq)
	{
		default:
			assert(!"unknown blend eq");
			/* FALLTHROUGH */
		case JKG_EQUATION_ADD:
			return PIPE_BLEND_ADD;
		case JKG_EQUATION_SUBTRACT:
			return PIPE_BLEND_SUBTRACT;
		case JKG_EQUATION_REV_SUBTRACT:
			return PIPE_BLEND_REVERSE_SUBTRACT;
		case JKG_EQUATION_MIN:
			return PIPE_BLEND_MIN;
		case JKG_EQUATION_MAX:
			return PIPE_BLEND_MAX;
	}
}

static uint32_t
get_color_mask(enum jkg_color_mask mask)
{
	uint32_t ret = 0;

	if (mask & JKG_COLOR_MASK_R)
		ret |= PIPE_MASK_R;
	if (mask & JKG_COLOR_MASK_G)
		ret |= PIPE_MASK_G;
	if (mask & JKG_COLOR_MASK_B)
		ret |= PIPE_MASK_B;
	if (mask & JKG_COLOR_MASK_A)
		ret |= PIPE_MASK_A;
	return ret;
}

int
virgl_blend_state_alloc(struct jkg_ctx *ctx,
                        const struct jkg_blend_state_create_info *create_info,
                        struct jkg_blend_state **statep)
{
	struct jkg_blend_state *state = NULL;
	uint32_t *request;
	int ret;

	state = calloc(1, sizeof(*state));
	if (!state)
		return -ENOMEM;
	ret = virgl_alloc_id(ctx, &state->id);
	if (ret)
		goto err;
	ret = virgl_req_alloc(ctx,
	                      VIRGL_CCMD_CREATE_OBJECT,
	                      VIRGL_OBJECT_BLEND,
	                      VIRGL_OBJ_BLEND_SIZE,
	                      &request);
	if (ret)
		goto err;
	request[VIRGL_OBJ_BLEND_HANDLE] = state->id;
	request[VIRGL_OBJ_BLEND_S0] = VIRGL_OBJ_BLEND_S0_INDEPENDENT_BLEND_ENABLE(1)
	                            | VIRGL_OBJ_BLEND_S0_LOGICOP_ENABLE(create_info->logic_enable)
	                            | VIRGL_OBJ_BLEND_S0_DITHER(0) /* XXX */
	                            | VIRGL_OBJ_BLEND_S0_ALPHA_TO_COVERAGE(0) /* XXX */
	                            | VIRGL_OBJ_BLEND_S0_ALPHA_TO_ONE(0); /* XXX */
	request[VIRGL_OBJ_BLEND_S1] = VIRGL_OBJ_BLEND_S1_LOGICOP_FUNC(get_logic_op(create_info->logic_op));
	for (size_t i = 0; i < create_info->nattachments; ++i)
	{
		const struct jkg_blend_attachment *attachment = &create_info->attachments[i];
		request[VIRGL_OBJ_BLEND_S2(i)] = VIRGL_OBJ_BLEND_S2_RT_BLEND_ENABLE(attachment->enable)
		            | VIRGL_OBJ_BLEND_S2_RT_RGB_FUNC(get_blend_eq(attachment->rgb_eq))
		            | VIRGL_OBJ_BLEND_S2_RT_RGB_SRC_FACTOR(get_blend_factor(attachment->rgb_src))
		            | VIRGL_OBJ_BLEND_S2_RT_RGB_DST_FACTOR(get_blend_factor(attachment->rgb_dst))
		            | VIRGL_OBJ_BLEND_S2_RT_ALPHA_FUNC(get_blend_eq(attachment->alpha_eq))
		            | VIRGL_OBJ_BLEND_S2_RT_ALPHA_SRC_FACTOR(get_blend_factor(attachment->alpha_src))
		            | VIRGL_OBJ_BLEND_S2_RT_ALPHA_DST_FACTOR(get_blend_factor(attachment->alpha_dst))
		            | VIRGL_OBJ_BLEND_S2_RT_COLORMASK(get_color_mask(attachment->color_mask));
	}
	for (size_t i = create_info->nattachments; i < VIRGL_MAX_COLOR_BUFS; ++i)
	{
		request[VIRGL_OBJ_BLEND_S2(i)] = VIRGL_OBJ_BLEND_S2_RT_BLEND_ENABLE(1)
		            | VIRGL_OBJ_BLEND_S2_RT_RGB_FUNC(PIPE_BLEND_ADD)
		            | VIRGL_OBJ_BLEND_S2_RT_RGB_SRC_FACTOR(PIPE_BLENDFACTOR_SRC_ALPHA)
		            | VIRGL_OBJ_BLEND_S2_RT_RGB_DST_FACTOR(PIPE_BLENDFACTOR_INV_SRC_ALPHA)
		            | VIRGL_OBJ_BLEND_S2_RT_ALPHA_FUNC(PIPE_BLEND_ADD)
		            | VIRGL_OBJ_BLEND_S2_RT_ALPHA_SRC_FACTOR(PIPE_BLENDFACTOR_SRC_ALPHA)
		            | VIRGL_OBJ_BLEND_S2_RT_ALPHA_DST_FACTOR(PIPE_BLENDFACTOR_INV_SRC_ALPHA)
		            | VIRGL_OBJ_BLEND_S2_RT_COLORMASK(PIPE_MASK_RGBAZS);
	};
	*statep = state;
	return 0;

err:
	if (state->id)
		virgl_free_id(ctx, state->id);
	free(state);
	return ret;
}

int
virgl_blend_state_bind(struct jkg_ctx *ctx,
                       struct jkg_blend_state *state)
{
	return virgl_bind_object(ctx, VIRGL_OBJECT_BLEND, state->id);
}

void
virgl_blend_state_free(struct jkg_ctx *ctx,
                       struct jkg_blend_state *state)
{
	if (virgl_destroy_object(ctx, VIRGL_OBJECT_BLEND, state->id))
		assert(!"failed to destroy blend state");
	virgl_free_id(ctx, state->id);
	free(state);
}

int
virgl_set_blend_color(struct jkg_ctx *ctx,
                      const float *color)
{
	uint32_t *request;
	int ret;

	ret = virgl_req_alloc(ctx,
	                      VIRGL_CCMD_SET_BLEND_COLOR,
	                      VIRGL_OBJECT_NULL,
	                      VIRGL_SET_BLEND_COLOR_SIZE,
	                      &request);
	if (ret)
		return ret;
	for (size_t i = 0; i < 4; ++i)
		request[VIRGL_SET_BLEND_COLOR(i)] = fconv(color[i]);
	return 0;
}
