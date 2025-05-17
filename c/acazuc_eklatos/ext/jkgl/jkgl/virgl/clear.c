#include "virgl.h"

static uint32_t
get_buffers(enum jkg_clear_mask mask)
{
	uint32_t ret = 0;

	if (mask & JKG_CLEAR_COLOR)
		ret |= PIPE_CLEAR_COLOR0;
	if (mask & JKG_CLEAR_DEPTH)
		ret |= PIPE_CLEAR_DEPTH;
	if (mask & JKG_CLEAR_STENCIL)
		ret |= PIPE_CLEAR_STENCIL;
	return ret;
}

int
virgl_clear(struct jkg_ctx *ctx, const struct jkg_clear_cmd *cmd)
{
	uint32_t *request;
	int ret;

	ret = virgl_req_alloc(ctx,
	                      VIRGL_CCMD_CLEAR,
	                      VIRGL_OBJECT_NULL,
	                      VIRGL_OBJ_CLEAR_SIZE,
	                      &request);
	if (ret)
		return ret;
	request[VIRGL_OBJ_CLEAR_BUFFERS] = get_buffers(cmd->mask);
	request[VIRGL_OBJ_CLEAR_COLOR_0] = fconv(cmd->color[0]);
	request[VIRGL_OBJ_CLEAR_COLOR_1] = fconv(cmd->color[1]);
	request[VIRGL_OBJ_CLEAR_COLOR_2] = fconv(cmd->color[2]);
	request[VIRGL_OBJ_CLEAR_COLOR_3] = fconv(cmd->color[3]);
	request[VIRGL_OBJ_CLEAR_DEPTH_0] = dconv(cmd->depth);
	request[VIRGL_OBJ_CLEAR_DEPTH_1] = dconv(cmd->depth) >> 32;
	request[VIRGL_OBJ_CLEAR_STENCIL] = cmd->stencil;
	return 0;
}
