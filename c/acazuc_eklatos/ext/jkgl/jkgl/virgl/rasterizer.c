#include "virgl.h"

#include <stdlib.h>
#include <assert.h>
#include <errno.h>

static uint32_t
get_cull_face(enum jkg_cull_face face)
{
	uint32_t ret = 0;

	if (face & JKG_CULL_FRONT)
		ret |= PIPE_FACE_FRONT;
	if (face & JKG_CULL_BACK)
		ret |= PIPE_FACE_BACK;
	return ret;
}

static enum pipe_polygon_mode
get_fill_mode(enum jkg_fill_mode mode)
{
	switch (mode)
	{
		default:
			assert(!"unknown fill mode");
			/* FALLTHROUGH */
		case JKG_FILL_SOLID:
			return PIPE_POLYGON_MODE_FILL;
		case JKG_FILL_LINE:
			return PIPE_POLYGON_MODE_LINE;
		case JKG_FILL_POINT:
			return PIPE_POLYGON_MODE_POINT;
	}
}

int
virgl_rasterizer_state_alloc(struct jkg_ctx *ctx,
                             const struct jkg_rasterizer_state_create_info *create_info,
                             struct jkg_rasterizer_state **statep)
{
	struct jkg_rasterizer_state *state = NULL;
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
	                      VIRGL_OBJECT_RASTERIZER,
	                      VIRGL_OBJ_RS_SIZE,
	                      &request);
	if (ret)
		goto err;
	request[VIRGL_OBJ_RS_HANDLE] = state->id;
	request[VIRGL_OBJ_RS_S0] = VIRGL_OBJ_RS_S0_FLATSHADE(0) /* XXX */
	                         | VIRGL_OBJ_RS_S0_DEPTH_CLIP(create_info->depth_clamp_enable)
	                         | VIRGL_OBJ_RS_S0_CLIP_HALFZ(0) /* XXX */
	                         | VIRGL_OBJ_RS_S0_RASTERIZER_DISCARD(create_info->rasterizer_discard_enable)
	                         | VIRGL_OBJ_RS_S0_FLATSHADE_FIRST(0) /* XXX */
	                         | VIRGL_OBJ_RS_S0_LIGHT_TWOSIZE(0) /* XXX */
	                         | VIRGL_OBJ_RS_S0_SPRITE_COORD_MODE(0) /* XXX */
	                         | VIRGL_OBJ_RS_S0_POINT_QUAD_RASTERIZATION(0) /* XXX */
	                         | VIRGL_OBJ_RS_S0_CULL_FACE(get_cull_face(create_info->cull_face))
	                         | VIRGL_OBJ_RS_S0_FILL_FRONT(get_fill_mode(create_info->fill_front))
	                         | VIRGL_OBJ_RS_S0_FILL_BACK(get_fill_mode(create_info->fill_back))
	                         | VIRGL_OBJ_RS_S0_SCISSOR(create_info->scissor_enable)
	                         | VIRGL_OBJ_RS_S0_FRONT_CCW(create_info->front_ccw)
	                         | VIRGL_OBJ_RS_S0_CLAMP_VERTEX_COLOR(0) /* XXX */
	                         | VIRGL_OBJ_RS_S0_CLAMP_FRAGMENT_COLOR(0) /* XXX */
	                         | VIRGL_OBJ_RS_S0_OFFSET_LINE(0) /* XXX */
	                         | VIRGL_OBJ_RS_S0_OFFSET_POINT(0) /* XXX */
	                         | VIRGL_OBJ_RS_S0_OFFSET_TRI(0) /* XXX */
	                         | VIRGL_OBJ_RS_S0_POLY_SMOOTH(0) /* XXX */
	                         | VIRGL_OBJ_RS_S0_POLY_STIPPLE_ENABLE(0) /* XXX */
	                         | VIRGL_OBJ_RS_S0_POINT_SMOOTH(create_info->point_smooth_enable)
	                         | VIRGL_OBJ_RS_S0_POINT_SIZE_PER_VERTEX(0) /* XXX */
	                         | VIRGL_OBJ_RS_S0_MULTISAMPLE(create_info->multisample_enable)
	                         | VIRGL_OBJ_RS_S0_LINE_SMOOTH(create_info->line_smooth_enable)
	                         | VIRGL_OBJ_RS_S0_LINE_LAST_PIXEL(0) /* XXX */
	                         | VIRGL_OBJ_RS_S0_HALF_PIXEL_CENTER(0) /* XXX */
	                         | VIRGL_OBJ_RS_S0_BOTTOM_EDGE_RULE(0) /* XXX */
	                         | VIRGL_OBJ_RS_S0_FORCE_PERSAMPLE_INTERP(0); /* XXX */
	request[VIRGL_OBJ_RS_POINT_SIZE] = fconv(create_info->point_size);
	request[VIRGL_OBJ_RS_SPRITE_COORD_ENABLE] = 0; /* XXX */
	request[VIRGL_OBJ_RS_S3] = VIRGL_OBJ_RS_S3_LINE_STIPPLE_PATTERN(0)
	                         | VIRGL_OBJ_RS_S3_LINE_STIPPLE_FACTOR(0)
	                         | VIRGL_OBJ_RS_S3_CLIP_PLANE_ENABLE(0);
	request[VIRGL_OBJ_RS_LINE_WIDTH] = fconv(create_info->line_width);
	request[VIRGL_OBJ_RS_OFFSET_UNITS] = fconv(0); /* XXX */
	request[VIRGL_OBJ_RS_OFFSET_SCALE] = fconv(0); /* XXX */
	request[VIRGL_OBJ_RS_OFFSET_CLAMP] = fconv(0); /* XXX */
	*statep = state;
	return 0;

err:
	if (state->id)
		virgl_free_id(ctx, state->id);
	free(state);
	return ret;
}

int
virgl_rasterizer_state_bind(struct jkg_ctx *ctx,
                            struct jkg_rasterizer_state *state)
{
	return virgl_bind_object(ctx, VIRGL_OBJECT_RASTERIZER, state->id);
}

void
virgl_rasterizer_state_free(struct jkg_ctx *ctx,
                            struct jkg_rasterizer_state *state)
{
	if (virgl_destroy_object(ctx, VIRGL_OBJECT_RASTERIZER, state->id))
		assert(!"failed to destroy rasterizer state");
	virgl_free_id(ctx, state->id);
	free(state);
}
