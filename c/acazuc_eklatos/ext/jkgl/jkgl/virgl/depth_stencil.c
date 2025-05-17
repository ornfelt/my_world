#include "virgl.h"

#include <stdlib.h>
#include <assert.h>
#include <errno.h>

static enum pipe_compare_func
get_compare_op(enum jkg_compare_op op)
{
	switch (op)
	{
		default:
			assert(!"unknown compare op");
			/* FALLTHROUGH */
		case JKG_COMPARE_NEVER:
			return PIPE_FUNC_NEVER;
		case JKG_COMPARE_LOWER:
			return PIPE_FUNC_LESS;
		case JKG_COMPARE_LEQUAL:
			return PIPE_FUNC_LEQUAL;
		case JKG_COMPARE_EQUAL:
			return PIPE_FUNC_EQUAL;
		case JKG_COMPARE_GREATER:
			return PIPE_FUNC_GREATER;
		case JKG_COMPARE_NOTEQUAL:
			return PIPE_FUNC_NOTEQUAL;
		case JKG_COMPARE_ALWAYS:
			return PIPE_FUNC_ALWAYS;
	}
}

static enum pipe_stencil_op
get_stencil_op(enum jkg_stencil_op op)
{
	switch (op)
	{
		default:
			assert(!"unknown stencil op");
			/* FALLTHROUGH */
		case JKG_STENCIL_KEEP:
			return PIPE_STENCIL_OP_KEEP;
		case JKG_STENCIL_ZERO:
			return PIPE_STENCIL_OP_ZERO;
		case JKG_STENCIL_REPLACE:
			return PIPE_STENCIL_OP_REPLACE;
		case JKG_STENCIL_INC:
			return PIPE_STENCIL_OP_INCR;
		case JKG_STENCIL_INC_WRAP:
			return PIPE_STENCIL_OP_INCR_WRAP;
		case JKG_STENCIL_DEC:
			return PIPE_STENCIL_OP_DECR;
		case JKG_STENCIL_DEC_WRAP:
			return PIPE_STENCIL_OP_DECR_WRAP;
		case JKG_STENCIL_INV:
			return PIPE_STENCIL_OP_INVERT;
	}
}

int
virgl_depth_stencil_state_alloc(struct jkg_ctx *ctx,
                                const struct jkg_depth_stencil_state_create_info *create_info,
                                struct jkg_depth_stencil_state **statep)
{
	struct jkg_depth_stencil_state *state = NULL;
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
	                      VIRGL_OBJECT_DSA,
	                      VIRGL_OBJ_DSA_SIZE,
	                      &request);
	if (ret)
		goto err;
	request[VIRGL_OBJ_DSA_HANDLE] = state->id;
	request[VIRGL_OBJ_DSA_S0] = VIRGL_OBJ_DSA_S0_DEPTH_ENABLE(create_info->depth_test_enable)
	                          | VIRGL_OBJ_DSA_S0_DEPTH_WRITEMASK(create_info->depth_write_enable)
	                          | VIRGL_OBJ_DSA_S0_DEPTH_FUNC(get_compare_op(create_info->depth_compare))
	                          | VIRGL_OBJ_DSA_S0_ALPHA_ENABLED(0);
	request[VIRGL_OBJ_DSA_S1] = VIRGL_OBJ_DSA_S1_STENCIL_ENABLED(create_info->stencil_enable)
	                          | VIRGL_OBJ_DSA_S1_STENCIL_FUNC(get_compare_op(create_info->front.compare_op))
	                          | VIRGL_OBJ_DSA_S1_STENCIL_FAIL_OP(get_stencil_op(create_info->front.fail_op))
	                          | VIRGL_OBJ_DSA_S1_STENCIL_ZPASS_OP(get_stencil_op(create_info->front.pass_op))
	                          | VIRGL_OBJ_DSA_S1_STENCIL_ZFAIL_OP(get_stencil_op(create_info->front.zfail_op))
	                          | VIRGL_OBJ_DSA_S1_STENCIL_VALUEMASK(create_info->front.compare_mask)
	                          | VIRGL_OBJ_DSA_S1_STENCIL_WRITEMASK(create_info->front.write_mask);
	request[VIRGL_OBJ_DSA_S2] = VIRGL_OBJ_DSA_S1_STENCIL_ENABLED(create_info->stencil_enable)
	                          | VIRGL_OBJ_DSA_S1_STENCIL_FUNC(get_compare_op(create_info->back.compare_op))
	                          | VIRGL_OBJ_DSA_S1_STENCIL_FAIL_OP(get_stencil_op(create_info->back.fail_op))
	                          | VIRGL_OBJ_DSA_S1_STENCIL_ZPASS_OP(get_stencil_op(create_info->back.pass_op))
	                          | VIRGL_OBJ_DSA_S1_STENCIL_ZFAIL_OP(get_stencil_op(create_info->back.zfail_op))
	                          | VIRGL_OBJ_DSA_S1_STENCIL_VALUEMASK(create_info->back.compare_mask)
	                          | VIRGL_OBJ_DSA_S1_STENCIL_WRITEMASK(create_info->back.write_mask);
	request[VIRGL_OBJ_DSA_ALPHA_REF] = fconv(0); /* XXX ? */
	*statep = state;
	return 0;

err:
	if (state->id)
		virgl_free_id(ctx, state->id);
	free(state);
	return ret;
}

int
virgl_depth_stencil_state_bind(struct jkg_ctx *ctx,
                               struct jkg_depth_stencil_state *state)
{
	return virgl_bind_object(ctx, VIRGL_OBJECT_DSA, state->id);
}

void
virgl_depth_stencil_state_free(struct jkg_ctx *ctx,
                               struct jkg_depth_stencil_state *state)
{
	if (virgl_destroy_object(ctx, VIRGL_OBJECT_DSA, state->id))
		assert(!"failed to destroy depth stencil state");
	virgl_free_id(ctx, state->id);
	free(state);
}

int
virgl_set_stencil_ref(struct jkg_ctx *ctx, uint8_t front, uint8_t back)
{
	uint32_t *request;
	int ret;

	ret = virgl_req_alloc(ctx,
	                      VIRGL_CCMD_SET_STENCIL_REF,
	                      VIRGL_OBJECT_NULL,
	                      VIRGL_SET_STENCIL_REF_SIZE,
	                      &request);
	if (ret)
		return ret;
	request[VIRGL_SET_STENCIL_REF] = VIRGL_STENCIL_REF_VAL(front, back);
	return 0;
}
