#include "virgl.h"

#include <stdlib.h>
#include <assert.h>
#include <errno.h>

int
virgl_surface_alloc(struct jkg_ctx *ctx,
                    const struct jkg_surface_create_info *create_info,
                    struct jkg_surface **surfacep)
{
	struct jkg_surface *surface = NULL;
	uint32_t *request;
	int ret;

	surface = calloc(1, sizeof(*surface));
	if (!surface)
		return -ENOMEM;
	ret = virgl_alloc_id(ctx, &surface->id);
	if (ret)
		goto err;
	ret = virgl_req_alloc(ctx,
	                      VIRGL_CCMD_CREATE_OBJECT,
	                      VIRGL_OBJECT_SURFACE,
	                      VIRGL_OBJ_SURFACE_SIZE,
	                      &request);
	if (ret)
		goto err;
	request[VIRGL_OBJ_SURFACE_HANDLE] = surface->id;
	request[VIRGL_OBJ_SURFACE_RES_HANDLE] = create_info->image->res->id;
	request[VIRGL_OBJ_SURFACE_FORMAT] = virgl_get_format(create_info->image->format);
	request[VIRGL_OBJ_SURFACE_TEXTURE_LEVEL] = create_info->level;
	request[VIRGL_OBJ_SURFACE_TEXTURE_LAYERS] = (create_info->min_layer)
	                                          | ((create_info->min_layer + create_info->num_layers - 1) << 16);
	surface->image = create_info->image;
	*surfacep = surface;
	return 0;

err:
	if (surface->id)
		virgl_free_id(ctx, surface->id);
	free(surface);
	return ret;
}

int
virgl_surface_bind(struct jkg_ctx *ctx,
                   struct jkg_surface **colors,
                   uint32_t count,
                   struct jkg_surface *depth,
                   struct jkg_surface *stencil)
{
	uint32_t *request;
	int ret;

	if (depth && stencil && depth != stencil)
		return -EINVAL;
	ret = virgl_req_alloc(ctx,
	                      VIRGL_CCMD_SET_FRAMEBUFFER_STATE,
	                      VIRGL_OBJECT_NULL,
	                      VIRGL_SET_FRAMEBUFFER_STATE_SIZE(count),
	                      &request);
	if (ret)
		return ret;
	request[VIRGL_SET_FRAMEBUFFER_STATE_NR_CBUFS] = count;
	request[VIRGL_SET_FRAMEBUFFER_STATE_NR_ZSURF_HANDLE] = depth ? depth->id : 0;
	for (uint32_t i = 0; i < count; ++i)
		request[VIRGL_SET_FRAMEBUFFER_STATE_CBUF_HANDLE(i)] = colors[i] ? colors[i]->id : 0;
	return 0;
}

int
virgl_surface_clear(struct jkg_ctx *ctx,
                    struct jkg_surface *surface,
                    const struct jkg_clear_cmd *cmd)
{
	uint32_t *request;
	uint32_t buffers;
	int ret;

	ret = virgl_req_alloc(ctx,
	                       VIRGL_CCMD_CLEAR_SURFACE,
	                       VIRGL_OBJECT_NULL,
	                       VIRGL_CLEAR_SURFACE_SIZE,
	                       &request);
	if (ret)
		return ret;
	if (cmd->mask & JKG_CLEAR_COLOR)
	{
		buffers = PIPE_CLEAR_COLOR0;
	}
	else
	{
		buffers = 0;
		if (cmd->mask & JKG_CLEAR_DEPTH)
			buffers |= PIPE_CLEAR_DEPTH;
		if (cmd->mask & JKG_CLEAR_STENCIL)
			buffers |= PIPE_CLEAR_STENCIL;
	}
	request[VIRGL_CLEAR_SURFACE_S0] = VIRGL_CLEAR_SURFACE_S0_BUFFERS(buffers);
	request[VIRGL_CLEAR_SURFACE_HANDLE] = surface->id;
	if (cmd->mask & JKG_CLEAR_COLOR)
	{
		request[VIRGL_CLEAR_SURFACE_COLOR_0] = fconv(cmd->color[0]);
		request[VIRGL_CLEAR_SURFACE_COLOR_1] = fconv(cmd->color[1]);
		request[VIRGL_CLEAR_SURFACE_COLOR_2] = fconv(cmd->color[2]);
		request[VIRGL_CLEAR_SURFACE_COLOR_3] = fconv(cmd->color[3]);
	}
	else
	{
		if (cmd->mask & JKG_CLEAR_DEPTH)
		{
			request[VIRGL_CLEAR_SURFACE_COLOR_0] = dconv(cmd->depth);
			request[VIRGL_CLEAR_SURFACE_COLOR_1] = dconv(cmd->depth) >> 32;
		}
		request[VIRGL_CLEAR_SURFACE_COLOR_2] = 0;
		if (cmd->mask & JKG_CLEAR_STENCIL)
			request[VIRGL_CLEAR_SURFACE_COLOR_3] = cmd->stencil;
	}
	request[VIRGL_CLEAR_SURFACE_DST_X] = 0;
	request[VIRGL_CLEAR_SURFACE_DST_Y] = 0;
	request[VIRGL_CLEAR_SURFACE_WIDTH] = surface->image->res->info.size.x;
	request[VIRGL_CLEAR_SURFACE_HEIGHT] = surface->image->res->info.size.y;
	return 0;
}

void
virgl_surface_free(struct jkg_ctx *ctx,
                   struct jkg_surface *surface)
{
	if (virgl_destroy_object(ctx, VIRGL_OBJECT_SURFACE, surface->id))
		assert(!"failed to destroy surface");
	virgl_free_id(ctx, surface->id);
	free(surface);
}
