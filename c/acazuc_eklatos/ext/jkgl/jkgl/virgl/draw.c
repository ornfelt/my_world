#include "virgl.h"

#include <assert.h>

static enum pipe_prim_type
get_primitive_type(enum jkg_primitive primitive)
{
	switch (primitive)
	{
		default:
			assert(!"unknown primitive type");
			/* FALLTHROUGH */
		case JKG_PRIMITIVE_POINTS:
			return PIPE_PRIM_POINTS;
		case JKG_PRIMITIVE_LINES:
			return PIPE_PRIM_LINES;
		case JKG_PRIMITIVE_LINE_LOOP:
			return PIPE_PRIM_LINE_LOOP;
		case JKG_PRIMITIVE_LINE_STRIP:
			return PIPE_PRIM_LINE_STRIP;
		case JKG_PRIMITIVE_TRIANGLES:
			return PIPE_PRIM_TRIANGLES;
		case JKG_PRIMITIVE_TRIANGLE_FAN:
			return PIPE_PRIM_TRIANGLE_FAN;
		case JKG_PRIMITIVE_TRIANGLE_STRIP:
			return PIPE_PRIM_TRIANGLE_STRIP;
		case JKG_PRIMITIVE_QUADS:
			return PIPE_PRIM_QUADS;
		case JKG_PRIMITIVE_QUAD_STRIP:
			return PIPE_PRIM_QUAD_STRIP;
	}
}

int
virgl_draw(struct jkg_ctx *ctx, const struct jkg_draw_cmd *cmd)
{
	uint32_t *request;
	int ret;

	ret = virgl_req_alloc(ctx,
	                      VIRGL_CCMD_DRAW_VBO,
	                      VIRGL_OBJECT_NULL,
	                      VIRGL_DRAW_VBO_SIZE,
	                      &request);
	if (ret)
		return ret;
	request[VIRGL_DRAW_VBO_START] = cmd->start;
	request[VIRGL_DRAW_VBO_COUNT] = cmd->count;
	request[VIRGL_DRAW_VBO_MODE] = get_primitive_type(cmd->primitive);
	request[VIRGL_DRAW_VBO_INDEXED] = cmd->indexed;
	request[VIRGL_DRAW_VBO_INSTANCE_COUNT] = cmd->instance_count;
	request[VIRGL_DRAW_VBO_INDEX_BIAS] = cmd->base_vertex;
	request[VIRGL_DRAW_VBO_START_INSTANCE] = cmd->instance_start;
	request[VIRGL_DRAW_VBO_PRIMITIVE_RESTART] = cmd->primitive_restart_enable;
	request[VIRGL_DRAW_VBO_RESTART_INDEX] = cmd->primitive_restart_index;
	request[VIRGL_DRAW_VBO_MIN_INDEX] = 0;
	request[VIRGL_DRAW_VBO_MAX_INDEX] = (uint32_t)-1;
	request[VIRGL_DRAW_VBO_COUNT_FROM_SO] = 0;
	return 0;
}

int
virgl_set_viewport(struct jkg_ctx *ctx, const float *viewport)
{
	uint32_t *request;
	int ret;

	ret = virgl_req_alloc(ctx,
	                      VIRGL_CCMD_SET_VIEWPORT_STATE,
	                      VIRGL_OBJECT_NULL,
	                      VIRGL_SET_VIEWPORT_STATE_SIZE(1), /* XXX */
	                      &request);
	if (ret)
		return ret;
	request[VIRGL_SET_VIEWPORT_START_SLOT] = 0; /* XXX */
	request[VIRGL_SET_VIEWPORT_STATE_SCALE_0(0)] = fconv(viewport[3] / 2);
	request[VIRGL_SET_VIEWPORT_STATE_SCALE_1(0)] = fconv(viewport[4] / 2);
	request[VIRGL_SET_VIEWPORT_STATE_SCALE_2(0)] = fconv(viewport[5] - viewport[2]);
	request[VIRGL_SET_VIEWPORT_STATE_TRANSLATE_0(0)] = fconv(viewport[0] + viewport[3] / 2);
	request[VIRGL_SET_VIEWPORT_STATE_TRANSLATE_1(0)] = fconv(viewport[1] + viewport[4] / 2);
	request[VIRGL_SET_VIEWPORT_STATE_TRANSLATE_2(0)] = fconv(viewport[2]);
	return 0;
}

int
virgl_set_scissor(struct jkg_ctx *ctx, const float *scissor)
{
	uint32_t *request;
	int ret;

	ret = virgl_req_alloc(ctx,
	                      VIRGL_CCMD_SET_SCISSOR_STATE,
	                      VIRGL_OBJECT_NULL,
	                      VIRGL_SET_SCISSOR_STATE_SIZE(1), /* XXX */
	                      &request);
	if (ret)
		return ret;
	request[VIRGL_SET_SCISSOR_START_SLOT] = 0; /* XXX */
	request[VIRGL_SET_SCISSOR_MINX_MINY(0)] = (((uint16_t)scissor[0] & 0xFFFF) << 0)
	                                        | (((uint16_t)scissor[1] & 0xFFFF) << 16);
	request[VIRGL_SET_SCISSOR_MAXX_MAXY(0)] = (((uint16_t)(scissor[0] + scissor[2]) & 0xFFFF) << 0)
	                                        | (((uint16_t)(scissor[1] + scissor[3]) & 0xFFFF) << 16);
	return 0;
}
