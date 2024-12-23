#include "world/clouds.h"
#include "world/world.h"

#include "player/player.h"

#include "graphics.h"
#include "textures.h"
#include "shaders.h"
#include "voxel.h"
#include "log.h"

#include <gfx/device.h>

#include <stdlib.h>
#include <math.h>

#define CLOUD_Y 128
#define CLOUD_WIDTH 12
#define CLOUD_HEIGHT 4
#define DISPLAY_DISTANCE (24 * CLOUD_WIDTH)
#define FRONT_COLOR 0.9f
#define BACK_COLOR  0.9f
#define LEFT_COLOR  0.8f
#define RIGHT_COLOR 0.8f
#define UP_COLOR    1.0f
#define DOWN_COLOR  0.7f

#define TIME_FACTOR (1000000000L * CLOUD_WIDTH)

static bool is_part_filled(int32_t x, int32_t y);
static void draw_face_up(struct jks_array *vertexes_array, struct jks_array *indices_array, int32_t x, int32_t y);
static void draw_face_down(struct jks_array *vertexes_array, struct jks_array *indices_array, int32_t x, int32_t y);
static void draw_face_left(struct jks_array *vertexes_array, struct jks_array *indices_array, int32_t x, int32_t y);
static void draw_face_right(struct jks_array *vertexes_array, struct jks_array *indices_array, int32_t x, int32_t y);
static void draw_face_front(struct jks_array *vertexes_array, struct jks_array *indices_array, int32_t x, int32_t y);
static void draw_face_back(struct jks_array *vertexes_array, struct jks_array *indices_array, int32_t x, int32_t y);

void clouds_init(struct clouds *clouds, struct world *world)
{
	clouds->attributes_state = GFX_ATTRIBUTES_STATE_INIT();
	clouds->vertexes_buffer = GFX_BUFFER_INIT();
	clouds->indices_buffer = GFX_BUFFER_INIT();
	clouds->uniform_buffer = GFX_BUFFER_INIT();
	gfx_create_buffer(g_voxel->device, &clouds->uniform_buffer, GFX_BUFFER_UNIFORM, NULL, sizeof(struct shader_clouds_model_block), GFX_BUFFER_DYNAMIC);
	clouds->world = world;
	clouds_rebuild(clouds);
}

void clouds_destroy(struct clouds *clouds)
{
	gfx_delete_buffer(g_voxel->device, &clouds->uniform_buffer);
	gfx_delete_buffer(g_voxel->device, &clouds->indices_buffer);
	gfx_delete_buffer(g_voxel->device, &clouds->vertexes_buffer);
	gfx_delete_attributes_state(g_voxel->device, &clouds->attributes_state);
}

void clouds_tick(struct clouds *clouds)
{
	float offset_diff = (g_voxel->frametime / TIME_FACTOR) - clouds->last_x_offset;
	float player_diff_x = clouds->world->player->entity.pos.x - clouds->last_player_x;
	float player_diff_z = clouds->world->player->entity.pos.z - clouds->last_player_z;
	float player_diff = sqrt(player_diff_x * player_diff_x + player_diff_z * player_diff_z);
	if (offset_diff + player_diff > CLOUD_WIDTH * 3 || offset_diff + player_diff < -CLOUD_WIDTH * 3)
		clouds_rebuild(clouds);
}

void clouds_draw(struct clouds *clouds)
{
	if (g_voxel->frametime / TIME_FACTOR != clouds->last_x_offset)
		clouds_rebuild(clouds);
	struct mat4f mat_m;
	struct mat4f tmp;
	struct vec3f pos;
	float offset = -(g_voxel->frametime % TIME_FACTOR) / (float)TIME_FACTOR * CLOUD_WIDTH;
	VEC3_SET(pos, offset, CLOUD_Y, 0);
	MAT4_IDENTITY(tmp);
	MAT4_TRANSLATE(mat_m, tmp, pos);
	struct shader_clouds_model_block model_block;
	MAT4_MUL(model_block.mvp, clouds->world->player->mat_vp, mat_m);
	MAT4_MUL(model_block.mv, clouds->world->player->mat_v, mat_m);
	VEC4_CPY(model_block.fog_color, g_voxel->world->fog_color);
	model_block.fog_distance = g_voxel->world->fog_distance;
	model_block.fog_density = g_voxel->world->fog_density;
	gfx_set_buffer_data(&clouds->uniform_buffer, &model_block, sizeof(model_block), 0);
	gfx_bind_constant(g_voxel->device, 1, &clouds->uniform_buffer, sizeof(model_block), 0);
	gfx_bind_attributes_state(g_voxel->device, &clouds->attributes_state, &g_voxel->graphics->clouds.input_layout);
	gfx_bind_pipeline_state(g_voxel->device, &g_voxel->graphics->clouds.first_pipeline_state);
	gfx_draw_indexed(g_voxel->device, clouds->indices_nb, 0);
	gfx_bind_pipeline_state(g_voxel->device, &g_voxel->graphics->clouds.second_pipeline_state);
	gfx_draw_indexed(g_voxel->device, clouds->indices_nb, 0);
}

void clouds_rebuild(struct clouds *clouds)
{
	clouds->last_x_offset = g_voxel->frametime / TIME_FACTOR;
	clouds->last_player_x = clouds->world->player->entity.pos.x;
	clouds->last_player_z = clouds->world->player->entity.pos.z;
	struct jks_array vertexes; /* struct shader_clouds_vertex */
	struct jks_array indices; /* uint16_t */
	jks_array_init(&vertexes, sizeof(struct shader_clouds_vertex), NULL, NULL);
	jks_array_init(&indices, sizeof(uint16_t), NULL, NULL);
	clouds->indices_nb = 0;
	int64_t offset = g_voxel->frametime / TIME_FACTOR;
	int32_t base_x = clouds->world->player->entity.pos.x - DISPLAY_DISTANCE - CLOUD_WIDTH * 2;
	int32_t end_x = clouds->world->player->entity.pos.x + DISPLAY_DISTANCE + CLOUD_WIDTH * 2;
	int32_t base_y = clouds->world->player->entity.pos.z - DISPLAY_DISTANCE - CLOUD_WIDTH * 2;
	int32_t end_y = clouds->world->player->entity.pos.z + DISPLAY_DISTANCE + CLOUD_WIDTH * 2;
	base_x -= base_x % CLOUD_WIDTH;
	end_x -= end_x % CLOUD_WIDTH;
	base_y -= base_y % CLOUD_WIDTH;
	end_y -= end_y % CLOUD_WIDTH;
	for (int32_t y = base_y; y <= end_y; y += CLOUD_WIDTH)
	{
		for (int32_t x = base_x; x <= end_x; x += CLOUD_WIDTH)
		{
			int32_t rel_x = x / CLOUD_WIDTH + offset;
			int32_t rel_y = y / CLOUD_WIDTH;
			if (!is_part_filled(rel_x, rel_y))
				continue;
			draw_face_up(&vertexes, &indices, x, y);
			draw_face_down(&vertexes, &indices, x, y);
			clouds->indices_nb += 12;
			if (!is_part_filled(rel_x - 1, rel_y))
			{
				draw_face_left(&vertexes, &indices, x, y);
				clouds->indices_nb += 6;
			}
			if (!is_part_filled(rel_x + 1, rel_y))
			{
				draw_face_right(&vertexes, &indices, x, y);
				clouds->indices_nb += 6;
			}
			if (!is_part_filled(rel_x, rel_y - 1))
			{
				draw_face_back(&vertexes, &indices, x, y);
				clouds->indices_nb += 6;
			}
			if (!is_part_filled(rel_x, rel_y + 1))
			{
				draw_face_front(&vertexes, &indices, x, y);
				clouds->indices_nb += 6;
			}
		}
	}
	gfx_delete_attributes_state(g_voxel->device, &clouds->attributes_state);
	gfx_delete_buffer(g_voxel->device, &clouds->vertexes_buffer);
	gfx_delete_buffer(g_voxel->device, &clouds->indices_buffer);
	gfx_create_buffer(g_voxel->device, &clouds->vertexes_buffer, GFX_BUFFER_VERTEXES, vertexes.data, sizeof(struct shader_clouds_vertex) * vertexes.size, GFX_BUFFER_IMMUTABLE);
	gfx_create_buffer(g_voxel->device, &clouds->indices_buffer, GFX_BUFFER_INDICES, indices.data, sizeof(uint16_t) * indices.size, GFX_BUFFER_IMMUTABLE);
	const struct gfx_attribute_bind binds[] =
	{
		{&clouds->vertexes_buffer},
	};
	gfx_create_attributes_state(g_voxel->device, &clouds->attributes_state, binds, sizeof(binds) / sizeof(*binds), &clouds->indices_buffer, GFX_INDEX_UINT16);
	jks_array_destroy(&vertexes);
	jks_array_destroy(&indices);
}

static bool is_part_filled(int32_t x, int32_t y)
{
	x %= g_voxel->textures->clouds.width;
	y %= g_voxel->textures->clouds.height;
	if (x < 0)
		x += g_voxel->textures->clouds.width;
	if (y < 0)
		y += g_voxel->textures->clouds.height;
	return g_voxel->textures->clouds.data[(y * g_voxel->textures->clouds.width + x) * 4 + 3] > 0x7F;
}

static void draw_face_up(struct jks_array *vertexes_array, struct jks_array *indices_array, int32_t x, int32_t y)
{
	uint16_t current_indice = vertexes_array->size;
	struct shader_clouds_vertex *vertexes = jks_array_grow(vertexes_array, 4);
	if (!vertexes)
	{
		LOG_ERROR("allocation failed");
		abort();
	}
	VEC3_SET(vertexes[0].position, x              , CLOUD_HEIGHT, y);
	VEC3_SET(vertexes[1].position, x              , CLOUD_HEIGHT, y + CLOUD_WIDTH);
	VEC3_SET(vertexes[2].position, x + CLOUD_WIDTH, CLOUD_HEIGHT, y);
	VEC3_SET(vertexes[3].position, x + CLOUD_WIDTH, CLOUD_HEIGHT, y + CLOUD_WIDTH);
	VEC3_SETV(vertexes[0].color, UP_COLOR);
	VEC3_SETV(vertexes[1].color, UP_COLOR);
	VEC3_SETV(vertexes[2].color, UP_COLOR);
	VEC3_SETV(vertexes[3].color, UP_COLOR);
	uint16_t *indices = jks_array_grow(indices_array, 6);
	if (!indices)
	{
		LOG_ERROR("allocation failed");
		abort();
	}
	indices[0] = current_indice + 0;
	indices[1] = current_indice + 1;
	indices[2] = current_indice + 2;
	indices[3] = current_indice + 3;
	indices[4] = current_indice + 2;
	indices[5] = current_indice + 1;
}

static void draw_face_down(struct jks_array *vertexes_array, struct jks_array *indices_array, int32_t x, int32_t y)
{
	uint16_t current_indice = vertexes_array->size;
	struct shader_clouds_vertex *vertexes = jks_array_grow(vertexes_array, 4);
	if (!vertexes)
	{
		LOG_ERROR("allocation failed");
		abort();
	}
	VEC3_SET(vertexes[0].position, x              , 0, y);
	VEC3_SET(vertexes[1].position, x + CLOUD_WIDTH, 0, y);
	VEC3_SET(vertexes[2].position, x              , 0, y + CLOUD_WIDTH);
	VEC3_SET(vertexes[3].position, x + CLOUD_WIDTH, 0, y + CLOUD_WIDTH);
	VEC3_SETV(vertexes[0].color, DOWN_COLOR);
	VEC3_SETV(vertexes[1].color, DOWN_COLOR);
	VEC3_SETV(vertexes[2].color, DOWN_COLOR);
	VEC3_SETV(vertexes[3].color, DOWN_COLOR);
	uint16_t *indices = jks_array_grow(indices_array, 6);
	if (!indices)
	{
		LOG_ERROR("allocation failed");
		abort();
	}
	indices[0] = current_indice + 0;
	indices[1] = current_indice + 1;
	indices[2] = current_indice + 2;
	indices[3] = current_indice + 3;
	indices[4] = current_indice + 2;
	indices[5] = current_indice + 1;
}

static void draw_face_left(struct jks_array *vertexes_array, struct jks_array *indices_array, int32_t x, int32_t y)
{
	uint16_t current_indice = vertexes_array->size;
	struct shader_clouds_vertex *vertexes = jks_array_grow(vertexes_array, 4);
	if (!vertexes)
	{
		LOG_ERROR("allocation failed");
		abort();
	}
	VEC3_SET(vertexes[0].position, x, 0           , y);
	VEC3_SET(vertexes[1].position, x, 0           , y + CLOUD_WIDTH);
	VEC3_SET(vertexes[2].position, x, CLOUD_HEIGHT, y);
	VEC3_SET(vertexes[3].position, x, CLOUD_HEIGHT, y + CLOUD_WIDTH);
	VEC3_SETV(vertexes[0].color, LEFT_COLOR);
	VEC3_SETV(vertexes[1].color, LEFT_COLOR);
	VEC3_SETV(vertexes[2].color, LEFT_COLOR);
	VEC3_SETV(vertexes[3].color, LEFT_COLOR);
	uint16_t *indices = jks_array_grow(indices_array, 6);
	if (!indices)
	{
		LOG_ERROR("allocation failed");
		abort();
	}
	indices[0] = current_indice + 0;
	indices[1] = current_indice + 1;
	indices[2] = current_indice + 2;
	indices[3] = current_indice + 3;
	indices[4] = current_indice + 2;
	indices[5] = current_indice + 1;
}

static void draw_face_right(struct jks_array *vertexes_array, struct jks_array *indices_array, int32_t x, int32_t y)
{
	uint16_t current_indice = vertexes_array->size;
	struct shader_clouds_vertex *vertexes = jks_array_grow(vertexes_array, 4);
	if (!vertexes)
	{
		LOG_ERROR("allocation failed");
		abort();
	}
	VEC3_SET(vertexes[0].position, x + CLOUD_WIDTH, 0           , y);
	VEC3_SET(vertexes[1].position, x + CLOUD_WIDTH, CLOUD_HEIGHT, y);
	VEC3_SET(vertexes[2].position, x + CLOUD_WIDTH, 0           , y + CLOUD_WIDTH);
	VEC3_SET(vertexes[3].position, x + CLOUD_WIDTH, CLOUD_HEIGHT, y + CLOUD_WIDTH);
	VEC3_SETV(vertexes[0].color, RIGHT_COLOR);
	VEC3_SETV(vertexes[1].color, RIGHT_COLOR);
	VEC3_SETV(vertexes[2].color, RIGHT_COLOR);
	VEC3_SETV(vertexes[3].color, RIGHT_COLOR);
	uint16_t *indices = jks_array_grow(indices_array, 6);
	if (!indices)
	{
		LOG_ERROR("allocation failed");
		abort();
	}
	indices[0] = current_indice + 0;
	indices[1] = current_indice + 1;
	indices[2] = current_indice + 2;
	indices[3] = current_indice + 3;
	indices[4] = current_indice + 2;
	indices[5] = current_indice + 1;
}

static void draw_face_front(struct jks_array *vertexes_array, struct jks_array *indices_array, int32_t x, int32_t y)
{
	uint16_t current_indice = vertexes_array->size;
	struct shader_clouds_vertex *vertexes = jks_array_grow(vertexes_array, 4);
	if (!vertexes)
	{
		LOG_ERROR("allocation failed");
		abort();
	}
	VEC3_SET(vertexes[0].position, x              , 0           , y + CLOUD_WIDTH);
	VEC3_SET(vertexes[1].position, x + CLOUD_WIDTH, 0           , y + CLOUD_WIDTH);
	VEC3_SET(vertexes[2].position, x              , CLOUD_HEIGHT, y + CLOUD_WIDTH);
	VEC3_SET(vertexes[3].position, x + CLOUD_WIDTH, CLOUD_HEIGHT, y + CLOUD_WIDTH);
	VEC3_SETV(vertexes[0].color, FRONT_COLOR);
	VEC3_SETV(vertexes[1].color, FRONT_COLOR);
	VEC3_SETV(vertexes[2].color, FRONT_COLOR);
	VEC3_SETV(vertexes[3].color, FRONT_COLOR);
	uint16_t *indices = jks_array_grow(indices_array, 6);
	if (!indices)
	{
		LOG_ERROR("allocation failed");
		abort();
	}
	indices[0] = current_indice + 0;
	indices[1] = current_indice + 1;
	indices[2] = current_indice + 2;
	indices[3] = current_indice + 3;
	indices[4] = current_indice + 2;
	indices[5] = current_indice + 1;
}

static void draw_face_back(struct jks_array *vertexes_array, struct jks_array *indices_array, int32_t x, int32_t y)
{
	uint16_t current_indice = vertexes_array->size;
	struct shader_clouds_vertex *vertexes = jks_array_grow(vertexes_array, 4);
	if (!vertexes)
	{
		LOG_ERROR("allocation failed");
		abort();
	}
	VEC3_SET(vertexes[0].position, x              , 0           , y);
	VEC3_SET(vertexes[1].position, x              , CLOUD_HEIGHT, y);
	VEC3_SET(vertexes[2].position, x + CLOUD_WIDTH, 0           , y);
	VEC3_SET(vertexes[3].position, x + CLOUD_WIDTH, CLOUD_HEIGHT, y);
	VEC3_SETV(vertexes[0].color, BACK_COLOR);
	VEC3_SETV(vertexes[1].color, BACK_COLOR);
	VEC3_SETV(vertexes[2].color, BACK_COLOR);
	VEC3_SETV(vertexes[3].color, BACK_COLOR);
	uint16_t *indices = jks_array_grow(indices_array, 6);
	if (!indices)
	{
		LOG_ERROR("allocation failed");
		abort();
	}
	indices[0] = current_indice + 0;
	indices[1] = current_indice + 1;
	indices[2] = current_indice + 2;
	indices[3] = current_indice + 3;
	indices[4] = current_indice + 2;
	indices[5] = current_indice + 1;
}
