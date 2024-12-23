#include "world/skybox.h"
#include "world/world.h"

#include "player/player.h"

#include "graphics.h"
#include "textures.h"
#include "shaders.h"
#include "voxel.h"
#include "log.h"

#include <gfx/device.h>

#include <jks/array.h>

#include <stdlib.h>
#include <math.h>

#define SKYBOX_PARTS 10
#define SKYBOX_DIST 1000

static void init_buffers(struct skybox *skybox);
static uint16_t get_skybox_index(uint32_t x, uint32_t y);
static void update_skybox_vertexes(struct skybox *skybox);

void skybox_init(struct skybox *skybox, struct world *world)
{
	skybox->world = world;
	skybox->skybox_attributes_state = GFX_ATTRIBUTES_STATE_INIT();
	skybox->moon_attributes_state = GFX_ATTRIBUTES_STATE_INIT();
	skybox->sun_attributes_state = GFX_ATTRIBUTES_STATE_INIT();
	skybox->sunmoon_uniform_buffer = GFX_BUFFER_INIT();
	skybox->skybox_uniform_buffer = GFX_BUFFER_INIT();
	skybox->skybox_vertexes_buffer = GFX_BUFFER_INIT();
	skybox->skybox_indices_buffer = GFX_BUFFER_INIT();
	skybox->moon_vertexes_buffer = GFX_BUFFER_INIT();
	skybox->moon_indices_buffer = GFX_BUFFER_INIT();
	skybox->sun_vertexes_buffer = GFX_BUFFER_INIT();
	skybox->sun_indices_buffer = GFX_BUFFER_INIT();
	gfx_create_buffer(g_voxel->device, &skybox->sunmoon_uniform_buffer, GFX_BUFFER_UNIFORM, NULL, sizeof(struct shader_sunmoon_model_block), GFX_BUFFER_DYNAMIC);
	gfx_create_buffer(g_voxel->device, &skybox->skybox_uniform_buffer, GFX_BUFFER_UNIFORM, NULL, sizeof(struct shader_skybox_model_block), GFX_BUFFER_DYNAMIC);
	init_buffers(skybox);
}

void skybox_destroy(struct skybox *skybox)
{
	gfx_delete_attributes_state(g_voxel->device, &skybox->skybox_attributes_state);
	gfx_delete_attributes_state(g_voxel->device, &skybox->moon_attributes_state);
	gfx_delete_attributes_state(g_voxel->device, &skybox->sun_attributes_state);
	gfx_delete_buffer(g_voxel->device, &skybox->sunmoon_uniform_buffer);
	gfx_delete_buffer(g_voxel->device, &skybox->skybox_uniform_buffer);
	gfx_delete_buffer(g_voxel->device, &skybox->skybox_vertexes_buffer);
	gfx_delete_buffer(g_voxel->device, &skybox->skybox_indices_buffer);
	gfx_delete_buffer(g_voxel->device, &skybox->moon_vertexes_buffer);
	gfx_delete_buffer(g_voxel->device, &skybox->moon_vertexes_buffer);
	gfx_delete_buffer(g_voxel->device, &skybox->sun_indices_buffer);
	gfx_delete_buffer(g_voxel->device, &skybox->sun_indices_buffer);
}

void skybox_draw(struct skybox *skybox)
{
	update_skybox_vertexes(skybox);
	{
		struct shader_skybox_model_block model_block;
		struct mat4f tmp;
		MAT4_IDENTITY(model_block.mvp);
		MAT4_TRANSLATE(tmp, model_block.mvp, skybox->world->player->entity.pos);
		MAT4_MUL(model_block.mvp, skybox->world->player->mat_vp, tmp);
		gfx_set_buffer_data(&skybox->skybox_uniform_buffer, &model_block, sizeof(model_block), 0);
		gfx_bind_constant(g_voxel->device, 1, &skybox->skybox_uniform_buffer, sizeof(model_block), 0);
	}
	gfx_bind_pipeline_state(g_voxel->device, &g_voxel->graphics->skybox.pipeline_state);
	gfx_bind_attributes_state(g_voxel->device, &skybox->skybox_attributes_state, &g_voxel->graphics->skybox.input_layout);
	gfx_draw_indexed(g_voxel->device, 6 * ((SKYBOX_PARTS * SKYBOX_PARTS - 1) - (SKYBOX_PARTS - 1)), 0);
	{
		float rot = -g_voxel->frametime / 1000000000. / 60 / 20 * M_PI * 2;
		struct shader_sunmoon_model_block model_block;
		struct mat4f tmp;
		MAT4_IDENTITY(tmp);
		MAT4_TRANSLATE(model_block.mvp, tmp, skybox->world->player->entity.pos);
		MAT4_ROTATEX(float, tmp, model_block.mvp, rot);
		MAT4_MUL(model_block.mvp, skybox->world->player->mat_vp, tmp);
		gfx_set_buffer_data(&skybox->sunmoon_uniform_buffer, &model_block, sizeof(model_block), 0);
		gfx_bind_constant(g_voxel->device, 1, &skybox->sunmoon_uniform_buffer, sizeof(model_block), 0);
	}
	texture_bind(&g_voxel->textures->sun);
	gfx_bind_pipeline_state(g_voxel->device, &g_voxel->graphics->sunmoon.pipeline_state);
	gfx_bind_attributes_state(g_voxel->device, &skybox->sun_attributes_state, &g_voxel->graphics->sunmoon.input_layout);
	gfx_draw_indexed(g_voxel->device, 6, 0);
	texture_bind(&g_voxel->textures->moon);
	gfx_bind_pipeline_state(g_voxel->device, &g_voxel->graphics->sunmoon.pipeline_state);
	gfx_bind_attributes_state(g_voxel->device, &skybox->moon_attributes_state, &g_voxel->graphics->sunmoon.input_layout);
	gfx_draw_indexed(g_voxel->device, 6, 0);
}

static void init_moon(struct skybox *skybox)
{
	static const struct shader_sunmoon_vertex moon_vertexes[] =
	{
		{{-100, -100, -800}, {1, 1, 1}, {0, 0}},
		{{ 100, -100, -800}, {1, 1, 1}, {1, 0}},
		{{ 100,  100, -800}, {1, 1, 1}, {1, 1}},
		{{-100,  100, -800}, {1, 1, 1}, {0, 1}},
	};
	static const uint16_t moon_indices[] = {0, 1, 2, 0, 2, 3};
	gfx_create_buffer(g_voxel->device, &skybox->moon_vertexes_buffer, GFX_BUFFER_VERTEXES, moon_vertexes, sizeof(moon_vertexes), GFX_BUFFER_IMMUTABLE);
	gfx_create_buffer(g_voxel->device, &skybox->moon_indices_buffer, GFX_BUFFER_INDICES, moon_indices, sizeof(moon_indices), GFX_BUFFER_IMMUTABLE);
	const struct gfx_attribute_bind moon_binds[] =
	{
		{&skybox->moon_vertexes_buffer},
	};
	gfx_create_attributes_state(g_voxel->device, &skybox->moon_attributes_state, moon_binds, sizeof(moon_binds) / sizeof(*moon_binds), &skybox->moon_indices_buffer, GFX_INDEX_UINT16);
}

static void init_sun(struct skybox *skybox)
{
	static const struct shader_sunmoon_vertex sun_vertexes[] =
	{
		{{-100, -100, 800}, {1, 1, 1}, {0, 0}},
		{{ 100, -100, 800}, {1, 1, 1}, {1, 0}},
		{{ 100,  100, 800}, {1, 1, 1}, {1, 1}},
		{{-100,  100, 800}, {1, 1, 1}, {0, 1}},
	};
	static const uint16_t sun_indices[] = {0, 1, 2, 0, 2, 3};
	gfx_create_buffer(g_voxel->device, &skybox->sun_vertexes_buffer, GFX_BUFFER_VERTEXES, sun_vertexes, sizeof(sun_vertexes), GFX_BUFFER_IMMUTABLE);
	gfx_create_buffer(g_voxel->device, &skybox->sun_indices_buffer, GFX_BUFFER_INDICES, sun_indices, sizeof(sun_indices), GFX_BUFFER_IMMUTABLE);
	const struct gfx_attribute_bind sun_binds[] =
	{
		{&skybox->sun_vertexes_buffer},
	};
	gfx_create_attributes_state(g_voxel->device, &skybox->sun_attributes_state, sun_binds, sizeof(sun_binds) / sizeof(*sun_binds), &skybox->sun_indices_buffer, GFX_INDEX_UINT16);
}

static void init_skybox(struct skybox *skybox)
{
	struct jks_array indices;
	jks_array_init(&indices, sizeof(uint16_t), NULL, NULL);
	uint16_t pos = 0;
	uint16_t nb_vertex = 0;
	uint16_t *indices_ptr = jks_array_grow(&indices, 6 * ((SKYBOX_PARTS * SKYBOX_PARTS - 1) - (SKYBOX_PARTS - 1)));
	if (!indices_ptr)
	{
		LOG_ERROR("can't resize skybox indices buffer");
		abort();
	}
	for (uint32_t y = 0; y < SKYBOX_PARTS; ++y)
	{
		for (uint32_t x = 0; x < SKYBOX_PARTS; ++x)
		{
			if ((y == 0 && x == 1) || (y > 0 && y < SKYBOX_PARTS - 1) || (y == SKYBOX_PARTS - 1 && x == 0))
				nb_vertex++;
			uint16_t p1 = get_skybox_index(x, y);
			uint16_t p2 = get_skybox_index((x + 1) % SKYBOX_PARTS, y);
			uint16_t p3 = get_skybox_index((x + 1) % SKYBOX_PARTS, (y + 1) % SKYBOX_PARTS);
			uint16_t p4 = get_skybox_index(x, (y + 1) % SKYBOX_PARTS);
			indices_ptr[pos++] = p1;
			indices_ptr[pos++] = p2;
			indices_ptr[pos++] = p4;
			indices_ptr[pos++] = p3;
			indices_ptr[pos++] = p4;
			indices_ptr[pos++] = p2;
		}
	}
	gfx_create_buffer(g_voxel->device, &skybox->skybox_indices_buffer, GFX_BUFFER_INDICES, indices.data, indices.size * sizeof(uint16_t), GFX_BUFFER_IMMUTABLE);
	gfx_create_buffer(g_voxel->device, &skybox->skybox_vertexes_buffer, GFX_BUFFER_VERTEXES, NULL, nb_vertex * sizeof(struct shader_skybox_vertex), GFX_BUFFER_DYNAMIC);
	{
		const struct gfx_attribute_bind skybox_binds[] =
		{
			{&skybox->skybox_vertexes_buffer},
		};
		gfx_create_attributes_state(g_voxel->device, &skybox->skybox_attributes_state, skybox_binds, sizeof(skybox_binds) / sizeof(*skybox_binds), &skybox->skybox_indices_buffer, GFX_INDEX_UINT16);
	}
	jks_array_destroy(&indices);
}

static void init_buffers(struct skybox *skybox)
{
	init_moon(skybox);
	init_sun(skybox);
	init_skybox(skybox);
}

static uint16_t get_skybox_index(uint32_t x, uint32_t y)
{
	if (y == 0)
		return 0;
	if (y == SKYBOX_PARTS - 1)
		return 1 + (SKYBOX_PARTS - 2) * SKYBOX_PARTS;
	return 1 + (y - 1) * SKYBOX_PARTS + x;
}

static void update_skybox_vertexes(struct skybox *skybox)
{
	struct jks_array vertexes; /* struct shader_skybox_vertex */
	jks_array_init(&vertexes, sizeof(struct shader_skybox_vertex), NULL, NULL);
	struct shader_skybox_vertex *vertexes_ptr = jks_array_grow(&vertexes, SKYBOX_PARTS * SKYBOX_PARTS - (SKYBOX_PARTS - 1) * 2);
	if (!vertexes_ptr)
	{
		LOG_ERROR("can't allocate colors buffer");
		abort();
	}
	for (uint32_t y = 0; y < SKYBOX_PARTS; ++y)
	{
		struct vec3f color;
		if (y <= SKYBOX_PARTS / 2)
			VEC3_SET(color, .71, .82, 1);
		else
			VEC3_SET(color, .51, .68, 1);
		for (uint32_t x = 0; x < SKYBOX_PARTS; ++x)
		{
			if ((y == 0 && x == 1) || (y > 0 && y < SKYBOX_PARTS - 1) || (y == SKYBOX_PARTS - 1 && x == 0))
			{
				uint16_t index = get_skybox_index(x, y);
				struct shader_skybox_vertex *vertex = &vertexes_ptr[index];
				float factor = M_PI * 2 / (SKYBOX_PARTS);
				float y_factor = sin(y * M_PI / (SKYBOX_PARTS - 1));
				vertex->position.x =  cos(x * factor) * SKYBOX_DIST * y_factor;
				vertex->position.y = -cos(y * factor / 2) * SKYBOX_DIST;
				vertex->position.z =  sin(x * factor) * SKYBOX_DIST * y_factor;
				VEC3_CPY(vertex->color, color);
			}
		}
	}
	gfx_set_buffer_data(&skybox->skybox_vertexes_buffer, vertexes.data, vertexes.size * sizeof(struct shader_skybox_vertex), 0);
	jks_array_destroy(&vertexes);
}
