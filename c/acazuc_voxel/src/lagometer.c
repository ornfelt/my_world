#include "lagometer.h"
#include "shaders.h"
#include "memory.h"
#include "voxel.h"
#include "log.h"

#include <gfx/window.h>
#include <gfx/device.h>

#include <stdlib.h>

#define WIDTH 500

static const struct gfx_input_layout_bind g_binds[] =
{
	{0, GFX_ATTR_R32G32_FLOAT      , sizeof(struct shader_gui_vertex), offsetof(struct shader_gui_vertex, position)},
	{0, GFX_ATTR_R32G32B32A32_FLOAT, sizeof(struct shader_gui_vertex), offsetof(struct shader_gui_vertex, color)},
	{0, GFX_ATTR_R32G32_FLOAT      , sizeof(struct shader_gui_vertex), offsetof(struct shader_gui_vertex, uv)},
};

struct lagometer *lagometer_new(void)
{
	struct lagometer *lagometer = calloc(sizeof(*lagometer), 1);
	if (!lagometer)
		return NULL;
	const struct gfx_attribute_bind binds[] =
	{
		{&lagometer->vertexes_buffer},
	};
	lagometer->pos = 0;
	lagometer->vertexes_data = calloc(sizeof(*lagometer->vertexes_data), WIDTH * 4);
	if (!lagometer->vertexes_data)
	{
		LOG_ERROR("failed to alloc lagometer vertexes");
		goto err;
	}
	lagometer->pipeline_state = GFX_PIPELINE_STATE_INIT();
	lagometer->vertexes_buffer = GFX_BUFFER_INIT();
	lagometer->input_layout = GFX_INPUT_LAYOUT_INIT();
	gfx_create_buffer(g_voxel->device, &lagometer->vertexes_buffer, GFX_BUFFER_VERTEXES, NULL, sizeof(struct shader_gui_vertex) * WIDTH * 4, GFX_BUFFER_STREAM);
	lagometer->uniform_buffer = GFX_BUFFER_INIT();
	gfx_create_buffer(g_voxel->device, &lagometer->uniform_buffer, GFX_BUFFER_UNIFORM, NULL, sizeof(struct shader_gui_model_block), GFX_BUFFER_STREAM);
	gfx_create_attributes_state(g_voxel->device, &lagometer->attributes_state, binds, sizeof(binds) / sizeof(*binds), NULL, 0);
	gfx_create_input_layout(g_voxel->device, &lagometer->input_layout, g_binds, sizeof(g_binds) / sizeof(*g_binds), &g_voxel->shaders->gui);
	lagometer->blend_state = GFX_BLEND_STATE_INIT();
	lagometer->depth_stencil_state = GFX_DEPTH_STENCIL_STATE_INIT();
	lagometer->rasterizer_state = GFX_RASTERIZER_STATE_INIT();
	gfx_create_depth_stencil_state(g_voxel->device, &lagometer->depth_stencil_state, false, false, GFX_CMP_ALWAYS, false, 0, GFX_CMP_NEVER, 0, 0, GFX_STENCIL_KEEP, GFX_STENCIL_KEEP, GFX_STENCIL_KEEP);
	gfx_create_rasterizer_state(g_voxel->device, &lagometer->rasterizer_state, GFX_FILL_SOLID, GFX_CULL_NONE, GFX_FRONT_CCW, true);
	gfx_create_blend_state(g_voxel->device, &lagometer->blend_state, true, GFX_BLEND_SRC_ALPHA, GFX_BLEND_ONE_MINUS_SRC_ALPHA, GFX_BLEND_SRC_ALPHA, GFX_BLEND_ONE_MINUS_SRC_ALPHA, GFX_EQUATION_ADD, GFX_EQUATION_ADD, GFX_COLOR_MASK_ALL);
	gfx_create_pipeline_state(g_voxel->device, &lagometer->pipeline_state, &g_voxel->shaders->gui, &lagometer->rasterizer_state, &lagometer->depth_stencil_state, &lagometer->blend_state, &lagometer->input_layout, GFX_PRIMITIVE_LINES);
	lagometer->white_pixel = GFX_TEXTURE_INIT();
	gfx_create_texture(g_voxel->device, &lagometer->white_pixel, GFX_TEXTURE_2D, GFX_R8G8B8A8, 1, 1, 1, 0);
	static const uint8_t white_pixel[] = {0xFF, 0xFF, 0xFF, 0xFF};
	gfx_set_texture_data(&lagometer->white_pixel, 0, 0, 1, 1, 0, 4, white_pixel);
	gfx_set_texture_addressing(&lagometer->white_pixel, GFX_TEXTURE_ADDRESSING_CLAMP, GFX_TEXTURE_ADDRESSING_CLAMP, GFX_TEXTURE_ADDRESSING_CLAMP);
	gfx_set_texture_filtering(&lagometer->white_pixel, GFX_FILTERING_NEAREST, GFX_FILTERING_NEAREST, GFX_FILTERING_NONE);
	gfx_set_texture_levels(&lagometer->white_pixel, 0, 0);
	return lagometer;

err:
	free(lagometer->vertexes_data);
	free(lagometer);
	return NULL;
}

void lagometer_delete(struct lagometer *lagometer)
{
	if (!lagometer)
		return;
	gfx_delete_blend_state(g_voxel->device, &lagometer->blend_state);
	gfx_delete_depth_stencil_state(g_voxel->device, &lagometer->depth_stencil_state);
	gfx_delete_rasterizer_state(g_voxel->device, &lagometer->rasterizer_state);
	gfx_delete_input_layout(g_voxel->device, &lagometer->input_layout);
	gfx_delete_attributes_state(g_voxel->device, &lagometer->attributes_state);
	gfx_delete_pipeline_state(g_voxel->device, &lagometer->pipeline_state);
	gfx_delete_buffer(g_voxel->device, &lagometer->vertexes_buffer);
	gfx_delete_buffer(g_voxel->device, &lagometer->uniform_buffer);
	gfx_delete_texture(g_voxel->device, &lagometer->white_pixel);
	free(lagometer->vertexes_data);
	free(lagometer);
}

void lagometer_draw(struct lagometer *lagometer)
{
	MAT4_ORTHO(float, lagometer->mat, 0, (float)g_voxel->window->width, (float)g_voxel->window->height, 0, -2, 2);
	float alpha = .5;
	for (size_t i = 0; i < WIDTH * 4; ++i)
		lagometer->vertexes_data[i].position.x--;
	float divisor = 100000;
	float y = g_voxel->window->height;
	VEC2_SET(lagometer->vertexes_data[lagometer->pos * 4 + 0].position, WIDTH - 1, y);
	y -= g_voxel->last_frame_draw_duration / divisor;
	VEC2_SET(lagometer->vertexes_data[lagometer->pos * 4 + 1].position, WIDTH - 1, y);
	uint64_t duration = g_voxel->last_frame_draw_duration;
	if (duration > 16666666)
	{
		VEC4_SET(lagometer->vertexes_data[lagometer->pos * 4 + 0].color, 0, 0, 1, alpha);
		VEC4_SET(lagometer->vertexes_data[lagometer->pos * 4 + 1].color, 0, 0, 1, alpha);
	}
	else
	{
		if (duration < 16666666 / 2)
		{
			float tmp = duration / 100000.f / 167.f * 2;
			if (tmp < 0)
				tmp = 0;
			if (tmp > 1)
				tmp = 1;
			VEC4_SET(lagometer->vertexes_data[lagometer->pos * 4 + 0].color, tmp, 1, 0, alpha);
			VEC4_SET(lagometer->vertexes_data[lagometer->pos * 4 + 1].color, tmp, 1, 0, alpha);
		}
		else
		{
			float tmp = (duration - 16666666 / 2) / 100000.f / 167.f * 2;
			if (tmp < 0)
				tmp = 0;
			if (tmp > 1)
				tmp = 1;
			VEC4_SET(lagometer->vertexes_data[lagometer->pos * 4 + 0].color, 1, 1 - tmp, 0, alpha);
			VEC4_SET(lagometer->vertexes_data[lagometer->pos * 4 + 1].color, 1, 1 - tmp, 0, alpha);
		}
	}
	VEC2_SET(lagometer->vertexes_data[lagometer->pos * 4 + 2].position, WIDTH - 1, y);
	y -= g_voxel->last_frame_update_duration / divisor;
	VEC2_SET(lagometer->vertexes_data[lagometer->pos * 4 + 3].position, WIDTH - 1, y);
	VEC4_SET(lagometer->vertexes_data[lagometer->pos * 4 + 2].color, 1, 1, 1, alpha);
	VEC4_SET(lagometer->vertexes_data[lagometer->pos * 4 + 3].color, 1, 1, 1, alpha);
	lagometer->pos = (lagometer->pos + 1) % WIDTH;
	gfx_set_buffer_data(&lagometer->vertexes_buffer, lagometer->vertexes_data, sizeof(*lagometer->vertexes_data) * WIDTH * 4, 0);
	struct shader_gui_model_block model_block;
	model_block.mvp = lagometer->mat;
	gfx_set_buffer_data(&lagometer->uniform_buffer, &model_block, sizeof(model_block), 0);
	gfx_bind_attributes_state(g_voxel->device, &lagometer->attributes_state, &lagometer->input_layout);
	gfx_bind_pipeline_state(g_voxel->device, &lagometer->pipeline_state);
	gfx_bind_constant(g_voxel->device, 1, &lagometer->uniform_buffer, sizeof(model_block), 0);
	const gfx_texture_t *textures[] =
	{
		&lagometer->white_pixel,
	};
	gfx_bind_samplers(g_voxel->device, 0, 1, &textures[0]);
	gfx_set_line_width(g_voxel->device, 1);
	gfx_draw(g_voxel->device, WIDTH * 4, 0);
}
