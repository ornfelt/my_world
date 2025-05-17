#include "utils.h"
#include "gjit.h"

#include <stddef.h>
#include <assert.h>
#include <errno.h>

static uint32_t
get_indice(struct jkg_ctx *ctx,
           uint32_t n)
{
	void *data;

	data = &((uint8_t*)ctx->index_buffer->data)[ctx->index_offset];
	switch (ctx->index_type)
	{
		case JKG_INDEX_UINT8:
			return ((const uint8_t*)data)[n];
		case JKG_INDEX_UINT16:
			return ((const uint16_t*)data)[n];
		case JKG_INDEX_UINT32:
			return ((const uint32_t*)data)[n];
		default:
			assert(!"unknown indice type");
			return 0;
	}
}

static bool
get_vert(struct rast_ctx *rast,
         uint32_t instance,
         uint32_t index,
         struct rast_vert *vert)
{
	struct gjit_draw_ctx *ctx = rast->userdata;

	if (ctx->cmd->indexed)
	{
		index = get_indice(ctx->ctx, index);
		if (ctx->cmd->primitive_restart_enable
		 && index == ctx->cmd->primitive_restart_index)
			return true;
		index += ctx->cmd->base_vertex;
	}
	ctx->ctx->input_layout->fn(&vert->attribs[0].x, ctx->ctx->vertex_buffers, index, instance);
	ctx->ctx->shader_state->vs->vs_fn(vert, ctx->ctx->uniform_data);
	rast_vert_normalize(vert,
	                    ctx->ctx->viewport[0],
	                    ctx->ctx->viewport[1],
	                    ctx->ctx->viewport[3],
	                    ctx->ctx->viewport[4]);
	return false;
}

static enum rast_fill_mode
get_fill_mode(enum jkg_fill_mode mode)
{
	switch (mode)
	{
		case JKG_FILL_POINT:
			return RAST_FILL_POINT;
		case JKG_FILL_LINE:
			return RAST_FILL_LINE;
		default:
			assert(!"unknown fill mode");
			/* FALLTHROUGH */
		case JKG_FILL_SOLID:
			return RAST_FILL_SOLID;
	}
}

static enum rast_cull_face
get_cull_face(enum jkg_cull_face face)
{
	enum rast_cull_face ret = 0;

	if (face & JKG_CULL_FRONT)
		ret |= RAST_CULL_FRONT;
	if (face & JKG_CULL_BACK)
		ret |= RAST_CULL_BACK;
	return ret;
}

static enum rast_primitive
get_primitive(enum jkg_primitive primitive)
{
	switch (primitive)
	{
		default:
			assert(!"unknown primitive");
			/* FALLTHROUGH */
		case JKG_PRIMITIVE_POINTS:
			return RAST_PRIMITIVE_POINTS;
		case JKG_PRIMITIVE_LINES:
			return RAST_PRIMITIVE_LINES;
		case JKG_PRIMITIVE_LINE_LOOP:
			return RAST_PRIMITIVE_LINE_LOOP;
		case JKG_PRIMITIVE_LINE_STRIP:
			return RAST_PRIMITIVE_LINE_STRIP;
		case JKG_PRIMITIVE_TRIANGLES:
			return RAST_PRIMITIVE_TRIANGLES;
		case JKG_PRIMITIVE_TRIANGLE_FAN:
			return RAST_PRIMITIVE_TRIANGLE_FAN;
		case JKG_PRIMITIVE_TRIANGLE_STRIP:
			return RAST_PRIMITIVE_TRIANGLE_STRIP;
		case JKG_PRIMITIVE_QUADS:
			return RAST_PRIMITIVE_QUADS;
		case JKG_PRIMITIVE_QUAD_STRIP:
			return RAST_PRIMITIVE_QUAD_STRIP;
	}
}

int
gjit_draw(struct jkg_ctx *ctx, const struct jkg_draw_cmd *cmd)
{
	struct gjit_draw_ctx draw;

	for (size_t i = 0; i < GJIT_MAX_UNIFORM_BLOCKS; ++i)
		ctx->uniform_data[i] = ctx->uniform_blocks[i].buffer ? ctx->uniform_blocks[i].buffer->data : NULL;
	draw.ctx = ctx;
	draw.cmd = cmd;
	draw.rast.fragment = gjit_fragment;
	draw.rast.get_vert = get_vert;
	draw.rast.userdata = &draw;
	draw.rast.min_x = maxf(ctx->viewport[0], 0);
	draw.rast.min_y = maxf(ctx->viewport[1], 0);
	draw.rast.min_z = ctx->viewport[2];
	draw.rast.max_x = minf(ctx->viewport[3], ctx->color_surfaces[0]->image->size.x);
	draw.rast.max_y = minf(ctx->viewport[4], ctx->color_surfaces[0]->image->size.y);
	draw.rast.max_z = ctx->viewport[5];
	if (ctx->rasterizer_state->scissor_enable)
	{
		draw.rast.min_x = maxf(draw.rast.min_x, ctx->scissor[0]);
		draw.rast.min_y = maxf(draw.rast.min_y, ctx->scissor[1]);
		draw.rast.max_x = minf(draw.rast.max_x, ctx->scissor[2]);
		draw.rast.max_y = minf(draw.rast.max_y, ctx->scissor[3]);
	}
	if (draw.rast.min_x > draw.rast.max_x
	 || draw.rast.min_y > draw.rast.max_y)
		return 0;
	draw.rast.line_width = ctx->rasterizer_state->line_width;
	draw.rast.line_smooth_enable = ctx->rasterizer_state->line_smooth_enable;
	draw.rast.point_size = ctx->rasterizer_state->point_size;
	draw.rast.point_smooth_enable = ctx->rasterizer_state->point_smooth_enable;
	draw.rast.nvarying = ctx->shader_state->vs->out;
	draw.rast.nattribs = ctx->shader_state->vs->in;
	draw.rast.front_ccw = ctx->rasterizer_state->front_ccw;
	draw.rast.cull_face = get_cull_face(ctx->rasterizer_state->cull_face);
	draw.rast.fill_front = get_fill_mode(ctx->rasterizer_state->fill_front);
	draw.rast.fill_back = get_fill_mode(ctx->rasterizer_state->fill_back);
	draw.rast.primitive = get_primitive(cmd->primitive);
	draw.rast.start = cmd->start;
	draw.rast.count = cmd->count;
	draw.rast.instance_start = cmd->instance_start;
	draw.rast.instance_count = cmd->instance_count;
	rast_draw(&draw.rast);
	return 0;
}

int
gjit_set_viewport(struct jkg_ctx *ctx, const float * restrict viewport)
{
	for (size_t i = 0; i < 6; ++i)
		ctx->viewport[i] = viewport[i];
	return 0;
}

int
gjit_set_scissor(struct jkg_ctx *ctx, const float * restrict scissor)
{
	for (size_t i = 0; i < 4; ++i)
		ctx->scissor[i] = scissor[i];
	return 0;
}
