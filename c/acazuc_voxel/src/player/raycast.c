#include "player/raycast.h"
#include "player/player.h"

#include "entity/dropped_block.h"

#include "particle/particle.h"

#include "block/blocks.h"
#include "block/block.h"

#include "world/block.h"
#include "world/world.h"
#include "world/chunk.h"

#include "textures.h"
#include "graphics.h"
#include "shaders.h"
#include "voxel.h"
#include "log.h"

#include <gfx/device.h>
#include <gfx/window.h>

#include <stdlib.h>
#include <string.h>
#include <math.h>

#define OFFSET        0.002
#define BREAK_OFFSET  0.002
#define PICK_DISTANCE 4
#define PICK_EPSILON  0.001

static void on_left_click(struct player_raycast *raycast, struct chunk *chunk, struct block block, struct vec3f relative, struct vec3f pos);
static void on_right_click(struct player_raycast *raycast, struct chunk *chunk, struct block_def *block_def, struct vec3f relative, struct vec3f pos);
static void build_break_tex_coords(struct player_raycast *raycast);
static void build_hover_vertexes(struct player_raycast *raycast, const struct aabb *aabb);

void player_raycast_init(struct player_raycast *raycast, struct player *player)
{
	raycast->player = player;
	raycast->done_ticks = 0;
	raycast->found = false;
	raycast->hover_attributes_state = GFX_ATTRIBUTES_STATE_INIT();
	raycast->break_attributes_state = GFX_ATTRIBUTES_STATE_INIT();
	raycast->hover_vertexes_buffer = GFX_BUFFER_INIT();
	raycast->break_vertexes_buffer = GFX_BUFFER_INIT();
	raycast->hover_indices_buffer = GFX_BUFFER_INIT();
	raycast->break_indices_buffer = GFX_BUFFER_INIT();
	raycast->hover_uniform_buffer = GFX_BUFFER_INIT();
	raycast->break_uniform_buffer = GFX_BUFFER_INIT();

	static const uint16_t break_indices[36] =
	{
		0 * 4 + 0, 0 * 4 + 1, 0 * 4 + 3, 0 * 4 + 2, 0 * 4 + 3, 0 * 4 + 1, /* front */
		1 * 4 + 0, 1 * 4 + 3, 1 * 4 + 1, 1 * 4 + 2, 1 * 4 + 1, 1 * 4 + 3, /* back */
		2 * 4 + 0, 2 * 4 + 3, 2 * 4 + 1, 2 * 4 + 2, 2 * 4 + 1, 2 * 4 + 3, /* left */
		3 * 4 + 0, 3 * 4 + 1, 3 * 4 + 3, 3 * 4 + 2, 3 * 4 + 3, 3 * 4 + 1, /* right */
		4 * 4 + 0, 4 * 4 + 1, 4 * 4 + 3, 4 * 4 + 2, 4 * 4 + 3, 4 * 4 + 1, /* up */
		4 * 4 + 0, 4 * 4 + 3, 4 * 4 + 1, 4 * 4 + 2, 4 * 4 + 1, 4 * 4 + 3, /* down */
	};
	gfx_create_buffer(g_voxel->device, &raycast->break_vertexes_buffer, GFX_BUFFER_VERTEXES, NULL, 24 * sizeof(struct shader_breaking_vertex), GFX_BUFFER_STATIC);
	gfx_create_buffer(g_voxel->device, &raycast->break_indices_buffer, GFX_BUFFER_INDICES, break_indices, sizeof(break_indices), GFX_BUFFER_IMMUTABLE);
	gfx_create_buffer(g_voxel->device, &raycast->break_uniform_buffer, GFX_BUFFER_UNIFORM, NULL, sizeof(struct shader_breaking_model_block), GFX_BUFFER_DYNAMIC);
	struct gfx_attribute_bind break_binds[] =
	{
		{&raycast->break_vertexes_buffer},
	};
	gfx_create_attributes_state(g_voxel->device, &raycast->break_attributes_state, break_binds, sizeof(break_binds) / sizeof(*break_binds), &raycast->break_indices_buffer, GFX_INDEX_UINT16);

	static const uint16_t hover_indices[24] =
	{
		0, 1, 0, 2, 1, 3, 2, 3, /* bottom */
		4, 5, 4, 6, 5, 7, 6, 7, /* top */
		0, 4, 1, 5, 2, 6, 3, 7, /* sides */
	};
	gfx_create_buffer(g_voxel->device, &raycast->hover_vertexes_buffer, GFX_BUFFER_VERTEXES, NULL, 8 * sizeof(struct shader_focused_vertex), GFX_BUFFER_STATIC);
	gfx_create_buffer(g_voxel->device, &raycast->hover_indices_buffer, GFX_BUFFER_INDICES, hover_indices, sizeof(hover_indices), GFX_BUFFER_IMMUTABLE);
	gfx_create_buffer(g_voxel->device, &raycast->hover_uniform_buffer, GFX_BUFFER_UNIFORM, NULL, sizeof(struct shader_focused_model_block), GFX_BUFFER_DYNAMIC);
	struct gfx_attribute_bind hover_binds[] =
	{
		{&raycast->hover_vertexes_buffer},
	};
	gfx_create_attributes_state(g_voxel->device, &raycast->hover_attributes_state, hover_binds, sizeof(hover_binds) / sizeof(*hover_binds), &raycast->hover_indices_buffer, GFX_INDEX_UINT16);
}

void player_raycast_destroy(struct player_raycast *raycast)
{
	gfx_delete_attributes_state(g_voxel->device, &raycast->hover_attributes_state);
	gfx_delete_attributes_state(g_voxel->device, &raycast->break_attributes_state);
	gfx_delete_buffer(g_voxel->device, &raycast->hover_vertexes_buffer);
	gfx_delete_buffer(g_voxel->device, &raycast->break_vertexes_buffer);
	gfx_delete_buffer(g_voxel->device, &raycast->hover_indices_buffer);
	gfx_delete_buffer(g_voxel->device, &raycast->break_indices_buffer);
	gfx_delete_buffer(g_voxel->device, &raycast->hover_uniform_buffer);
	gfx_delete_buffer(g_voxel->device, &raycast->break_uniform_buffer);
}

void player_raycast_tick(struct player_raycast *raycast)
{
	if (raycast->found)
		raycast->done_ticks++;
}

void player_raycast_draw(struct player_raycast *raycast)
{
	if (!raycast->found)
		return;
	{
		struct shader_focused_model_block model_block;
		MAT4_TRANSLATE(model_block.mvp, raycast->player->mat_vp, raycast->pos);
		gfx_set_buffer_data(&raycast->hover_uniform_buffer, &model_block, sizeof(model_block), 0);
		gfx_bind_pipeline_state(g_voxel->device, &g_voxel->graphics->focused.pipeline_state);
		gfx_bind_attributes_state(g_voxel->device, &raycast->hover_attributes_state, &g_voxel->graphics->focused.input_layout);
		gfx_bind_constant(g_voxel->device, 1, &raycast->hover_uniform_buffer, sizeof(model_block), 0);
		gfx_set_line_width(g_voxel->device, 2);
		gfx_draw_indexed(g_voxel->device, 24, 0);
	}
	if (raycast->found && raycast->done_ticks != 0)
	{
		struct shader_breaking_model_block model_block;
		MAT4_TRANSLATE(model_block.mvp, raycast->player->mat_vp, raycast->pos);
		MAT4_TRANSLATE(model_block.mv, raycast->player->mat_v, raycast->pos);
		VEC4_CPY(model_block.fog_color, raycast->player->entity.world->fog_color);
		model_block.fog_distance = raycast->player->entity.world->fog_distance;
		model_block.fog_density = raycast->player->entity.world->fog_density;
		gfx_set_buffer_data(&raycast->break_uniform_buffer, &model_block, sizeof(model_block), 0);
		gfx_bind_pipeline_state(g_voxel->device, &g_voxel->graphics->breaking.pipeline_state);
		gfx_bind_attributes_state(g_voxel->device, &raycast->break_attributes_state, &g_voxel->graphics->breaking.input_layout);
		gfx_bind_constant(g_voxel->device, 1, &raycast->hover_uniform_buffer, sizeof(model_block), 0);
		texture_bind(&g_voxel->textures->terrain);
		gfx_draw_indexed(g_voxel->device, 36, 0);
	}
}

static float signum(float val)
{
	if (val < 0)
		return -1;
	if (val > 0)
		return 1;
	return 0;
}

static float intbound(float pos, float dir)
{
	if (dir < 0 && dir == roundf(dir))
		return 0;
	if (dir > 0)
	{
		if (pos == 0)
			return 1 / dir;
		return (ceilf(pos) - pos) / dir;
	}
	return (pos - floorf(pos)) / -dir;
}

static bool test_block(struct player_raycast *raycast, struct vec3f pos,
                       struct vec3f org, struct vec3f dir,
                       int32_t *old_chunk_x, int32_t *old_chunk_z,
                       struct chunk **chunk, uint8_t face)
{
	if (pos.y < 0 || pos.y > CHUNK_HEIGHT)
		return false;
	int32_t chunk_x = chunk_get_coord(pos.x);
	int32_t chunk_z = chunk_get_coord(pos.z);
	if (!*chunk || chunk_x != *old_chunk_x || chunk_z != *old_chunk_z)
	{
		*old_chunk_x = chunk_x;
		*old_chunk_z = chunk_z;
		*chunk = world_get_chunk(raycast->player->entity.world, chunk_x, chunk_z);
	}
	if (!*chunk)
		return false;
	struct vec3f relative;
	VEC3_SET(relative, pos.x - chunk_x, pos.y, pos.z - chunk_z);
	struct block block;
	if (!chunk_get_block(*chunk, relative.x, relative.y, relative.z, &block)
	 || !block.type)
		return false;
	struct block_def *block_def = g_voxel->blocks->blocks[block.type];
	if (!block_def || !(block_def->flags & BLOCK_FLAG_FOCUSABLE))
		return false;
	float t = 0;
	struct aabb aabb = block_def->aabb;
	aabb_move(&aabb, pos);
	if (!aabb_intersect(&aabb, org, dir, &t))
		return false;
	if (!VEC3_EQ(raycast->pos, pos))
		raycast->done_ticks = 0;
	struct vec3f inter_pos;
	VEC3_MULV(inter_pos, dir, t);
	VEC3_ADD(inter_pos, inter_pos, org);
	float dst[6];
	dst[0] = fabsf(inter_pos.x - aabb.p0.x);
	dst[1] = fabsf(inter_pos.x - aabb.p1.x);
	dst[2] = fabsf(inter_pos.y - aabb.p0.y);
	dst[3] = fabsf(inter_pos.y - aabb.p1.y);
	dst[4] = fabsf(inter_pos.z - aabb.p0.z);
	dst[5] = fabsf(inter_pos.z - aabb.p1.z);
	float min = dst[0];
	if (dst[1] < min)
		min = dst[1];
	if (dst[2] < min)
		min = dst[2];
	if (dst[3] < min)
		min = dst[3];
	if (dst[4] < min)
		min = dst[4];
	if (dst[5] < min)
		min = dst[5];
	if (min == dst[0])
		raycast->face = BLOCK_FACE_LEFT;
	else if (min == dst[1])
		raycast->face = BLOCK_FACE_RIGHT;
	else if (min == dst[2])
		raycast->face = BLOCK_FACE_BACK;
	else if (min == dst[3])
		raycast->face = BLOCK_FACE_FRONT;
	else if (min == dst[4])
		raycast->face = BLOCK_FACE_TOP;
	else if (min == dst[5])
		raycast->face = BLOCK_FACE_BOTTOM;
	else
		return false;
	build_hover_vertexes(raycast, &block_def->aabb);
	raycast->face = face;
	raycast->pos = pos;
	raycast->found = true;
	if (gfx_is_mouse_button_down(g_voxel->window, GFX_MOUSE_BUTTON_LEFT))
	{
		on_left_click(raycast, *chunk, block, relative, pos);
		return true;
	}
	else
	{
		raycast->done_ticks = 0;
	}
	if (gfx_is_mouse_button_down(g_voxel->window, GFX_MOUSE_BUTTON_RIGHT))
	{
		on_right_click(raycast, *chunk, block_def, relative, pos);
		return true;
	}
	return true;
}

void player_raycast_update(struct player_raycast *raycast)
{
	raycast->found = false;
	struct vec3f org = raycast->player->entity.pos;
	org.y += 0.72;
	struct vec3f pos;
	VEC3_SET(pos,
	         floorf(raycast->player->entity.pos.x),
	         floorf(raycast->player->entity.pos.y + 0.72),
	         floorf(raycast->player->entity.pos.z));
	struct vec4f dir_tmp;
	struct vec3f dir;
	struct vec4f tmp;
	VEC4_SET(tmp, 0, 0, -1, 0);
	VEC4_MAT4_MUL(dir_tmp, raycast->player->mat_v, tmp);
	VEC3_CPY(dir, dir_tmp);
	struct vec3f step;
	VEC3_FN1(step, dir, signum);
	struct vec3f max;
	VEC3_SET(max,
	         intbound(raycast->player->entity.pos.x, dir.x),
	         intbound(raycast->player->entity.pos.y + 0.72, dir.y),
	         intbound(raycast->player->entity.pos.z, dir.z));
	struct vec3f delta;
	VEC3_DIV(delta, step, dir);
	float radius = PICK_DISTANCE;
	uint8_t face = 0;
	int32_t old_chunk_x = 0;
	int32_t old_chunk_z = 0;
	struct chunk *chunk = NULL;
	while (true)
	{
		if (test_block(raycast, pos, org, dir, &old_chunk_x, &old_chunk_z, &chunk, face))
			return;
		if (max.x < max.y)
		{
			if (max.x < max.z)
			{
				if (max.x > radius)
					break;
				pos.x += step.x;
				max.x += delta.x;
				face = step.x < 0 ? BLOCK_FACE_RIGHT : BLOCK_FACE_LEFT;
			}
			else
			{
				if (max.z > radius)
					break;
				pos.z += step.z;
				max.z += delta.z;
				face = step.z < 0 ? BLOCK_FACE_FRONT : BLOCK_FACE_BACK;
			}
		}
		else
		{
			if (max.y < max.z)
			{
				if (max.y > radius)
					break;
				pos.y += step.y;
				max.y += delta.y;
				face = step.y < 0 ? BLOCK_FACE_TOP : BLOCK_FACE_BOTTOM;
			}
			else
			{
				if (max.z > radius)
					break;
				pos.z += step.z;
				max.z += delta.z;
				face = step.z < 0 ? BLOCK_FACE_FRONT : BLOCK_FACE_BACK;
			}
		}
	}
	raycast->done_ticks = 0;
}

static void on_left_click(struct player_raycast *raycast, struct chunk *chunk, struct block block, struct vec3f relative, struct vec3f pos)
{
	struct block_def *block_def = g_voxel->blocks->blocks[block.type];
	if (!block_def)
		return;
	raycast->todo_ticks = 20 * block_def->hardness;
	build_break_tex_coords(raycast);
	uint8_t light = chunk_get_light(chunk, relative.x, relative.y, relative.z);
	uint8_t tmpl = world_get_light(chunk->world, pos.x - 1, pos.y, pos.z);
	if (tmpl > light)
		light = tmpl;
	tmpl = world_get_light(chunk->world, pos.x + 1, pos.y, pos.z);
	if (tmpl > light)
		light = tmpl;
	tmpl = world_get_light(chunk->world, pos.x, pos.y - 1, pos.z);
	if (tmpl > light)
		light = tmpl;
	tmpl = world_get_light(chunk->world, pos.x, pos.y + 1, pos.z);
	if (tmpl > light)
		light = tmpl;
	tmpl = world_get_light(chunk->world, pos.x, pos.y, pos.z - 1);
	if (tmpl > light)
		light = tmpl;
	tmpl = world_get_light(chunk->world, pos.x, pos.y, pos.z + 1);
	if (tmpl > light)
		light = tmpl;
	if (raycast->todo_ticks >= 0 && raycast->done_ticks > raycast->todo_ticks)
	{
		raycast->found = false;
		struct vec2f uv_base;
		struct vec4f color;
		block_def->vtable->get_destroy_values(block_def, &uv_base, &color);
		int32_t nb = 4;
		for (int32_t x = 0; x < nb; ++x)
		{
			for (int32_t y = 0; y < nb; ++y)
			{
				for (int32_t z = 0; z < nb; ++z)
				{
					struct vec3f tmp;
					VEC3_SET(tmp, (x + 0.5) / nb, (y + 0.5) / nb, (z + 0.5) / nb);
					struct vec3f pos2;
					VEC3_ADD(pos2, pos, tmp);
					pos.y += rand() * 0.1 / RAND_MAX - 0.05;
					pos.z += rand() * 0.1 / RAND_MAX - 0.05;
					struct vec2f size;
					VEC2_SET(size, (rand() * 0.5 / RAND_MAX + 0.5) * 0.2, 0);
					size.y = size.x;
					struct vec3f dir;
					VEC3_SUBV(dir, tmp, 0.5);
					dir.x += (rand() * 2.0 / RAND_MAX - 1.0);
					dir.y += (rand() * 2.0 / RAND_MAX - 1.0);
					dir.z += (rand() * 2.0 / RAND_MAX - 1.0);
					VEC3_NORMALIZE(float, dir, dir);
					float factor = (rand() / (float)RAND_MAX + rand() / (float)RAND_MAX + 1) * 0.3 * 0.4 * 0.4;
					VEC3_MULV(dir, dir, factor);
					dir.y += .1f;
					struct vec2f uv = uv_base;
					uv.x += rand() * 1.0 / 16 * 14 / 16 / RAND_MAX;
					uv.y += rand() * 1.0 / 16 * 14 / 16 / RAND_MAX;
					struct vec2f uv_size;
					VEC2_SET(uv_size, 1.0 / 16 / 8, 1.0 / 16 / 8);
					particle_new(raycast->player->entity.world, chunk, pos2, size, dir, uv, uv_size, color, light);
				}
			}
		}
		struct vec3f position;
		VEC3_ADDV(position, pos, 0.5);
		struct vec3f velocity;
		VEC3_SET(velocity, rand() * 0.2 / RAND_MAX - 0.1, 0.25 + rand() * 0.1 / RAND_MAX, rand() * 0.2 / RAND_MAX - 0.1);
		struct entity *tmp = dropped_block_new(raycast->player->entity.world, chunk, block.type, position, velocity);
		entities_add(&chunk->entities, tmp);
		chunk_destroy_block(chunk, relative.x, relative.y, relative.z);
	}
}

static void on_right_click(struct player_raycast *raycast, struct chunk *chunk, struct block_def *block_def, struct vec3f relative, struct vec3f pos)
{
	if (block_def->vtable->on_right_click && block_def->vtable->on_right_click(block_def, chunk, pos))
		return;
	if (raycast->face == BLOCK_FACE_LEFT)
		relative.x -= 1;
	else if (raycast->face == BLOCK_FACE_RIGHT)
		relative.x += 1;
	else if (raycast->face == BLOCK_FACE_FRONT)
		relative.z += 1;
	else if (raycast->face == BLOCK_FACE_BACK)
		relative.z -= 1;
	else if (raycast->face == BLOCK_FACE_TOP)
		relative.y += 1;
	else if (raycast->face == BLOCK_FACE_BOTTOM)
		relative.y -= 1;
	struct chunk *new_chunk = chunk;
	if (relative.x < 0)
	{
		new_chunk = chunk->chunk_x_less;
		relative.x = CHUNK_WIDTH - 1;
	}
	else if (relative.x >= CHUNK_WIDTH)
	{
		new_chunk = chunk->chunk_x_more;
		relative.x = 0;
	}
	else if (relative.y < 0)
	{
		return;
	}
	else if (relative.y >= CHUNK_HEIGHT)
	{
		return;
	}
	else if (relative.z < 0)
	{
		new_chunk = chunk->chunk_z_less;
		relative.z = CHUNK_WIDTH - 1;
	}
	else if (relative.z >= CHUNK_WIDTH)
	{
		new_chunk = chunk->chunk_z_more;
		relative.z = 0;
	}
	chunk_set_block_if_replaceable(new_chunk, relative.x, relative.y, relative.z, 2);
}

static void build_break_tex_coords(struct player_raycast *raycast)
{
	struct shader_breaking_vertex vertexes[24];
	uint8_t pos;
	if (raycast->todo_ticks > 0)
		pos = raycast->done_ticks * 9.0 / raycast->todo_ticks;
	else
		pos = 0;
	struct vec2f uv_org;
	struct vec2f uv_dst;
	VEC2_SET(uv_org, pos / 16.0, 15.0 / 16);
	VEC2_ADDV(uv_dst, uv_org, 1.0 / 16);
	/* front */
	VEC3_SET(vertexes[0].position , 0         , 0         , BLOCK_SIZE + BREAK_OFFSET);
	VEC3_SET(vertexes[1].position , BLOCK_SIZE, 0         , BLOCK_SIZE + BREAK_OFFSET);
	VEC3_SET(vertexes[2].position , BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE + BREAK_OFFSET);
	VEC3_SET(vertexes[3].position , 0         , BLOCK_SIZE, BLOCK_SIZE + BREAK_OFFSET);
	VEC2_SET(vertexes[0].uv , uv_org.x, uv_dst.y);
	VEC2_SET(vertexes[1].uv , uv_dst.x, uv_dst.y);
	VEC2_SET(vertexes[2].uv , uv_dst.x, uv_org.y);
	VEC2_SET(vertexes[3].uv , uv_org.x, uv_org.y);
	/* back */
	VEC3_SET(vertexes[4].position , 0         , 0         ,             -BREAK_OFFSET);
	VEC3_SET(vertexes[5].position , BLOCK_SIZE, 0         ,             -BREAK_OFFSET);
	VEC3_SET(vertexes[6].position , BLOCK_SIZE, BLOCK_SIZE,             -BREAK_OFFSET);
	VEC3_SET(vertexes[7].position , 0         , BLOCK_SIZE,             -BREAK_OFFSET);
	VEC2_SET(vertexes[4].uv , uv_dst.x, uv_dst.y);
	VEC2_SET(vertexes[5].uv , uv_org.x, uv_dst.y);
	VEC2_SET(vertexes[6].uv , uv_org.x, uv_org.y);
	VEC2_SET(vertexes[7].uv , uv_dst.x, uv_org.y);
	/* left */
	VEC3_SET(vertexes[8].position , 0           -BREAK_OFFSET, 0         , 0         );
	VEC3_SET(vertexes[9].position , 0           -BREAK_OFFSET, BLOCK_SIZE, 0         );
	VEC3_SET(vertexes[10].position, 0           -BREAK_OFFSET, BLOCK_SIZE, BLOCK_SIZE);
	VEC3_SET(vertexes[11].position, 0           -BREAK_OFFSET, 0         , BLOCK_SIZE);
	VEC2_SET(vertexes[8].uv , uv_org.x, uv_dst.y);
	VEC2_SET(vertexes[9].uv , uv_org.x, uv_org.y);
	VEC2_SET(vertexes[10].uv, uv_dst.x, uv_org.y);
	VEC2_SET(vertexes[11].uv, uv_dst.x, uv_dst.y);
	/* right */
	VEC3_SET(vertexes[12].position, BLOCK_SIZE + BREAK_OFFSET, 0         , 0         );
	VEC3_SET(vertexes[13].position, BLOCK_SIZE + BREAK_OFFSET, BLOCK_SIZE, 0         );
	VEC3_SET(vertexes[14].position, BLOCK_SIZE + BREAK_OFFSET, BLOCK_SIZE, BLOCK_SIZE);
	VEC3_SET(vertexes[15].position, BLOCK_SIZE + BREAK_OFFSET, 0         , BLOCK_SIZE);
	VEC2_SET(vertexes[12].uv, uv_dst.x, uv_dst.y);
	VEC2_SET(vertexes[13].uv, uv_dst.x, uv_org.y);
	VEC2_SET(vertexes[14].uv, uv_org.x, uv_org.y);
	VEC2_SET(vertexes[15].uv, uv_org.x, uv_dst.y);
	/* up */
	VEC3_SET(vertexes[16].position, 0         , BLOCK_SIZE + BREAK_OFFSET, 0         );
	VEC3_SET(vertexes[17].position, BLOCK_SIZE, BLOCK_SIZE + BREAK_OFFSET, 0         );
	VEC3_SET(vertexes[18].position, BLOCK_SIZE, BLOCK_SIZE + BREAK_OFFSET, BLOCK_SIZE);
	VEC3_SET(vertexes[19].position, 0         , BLOCK_SIZE + BREAK_OFFSET, BLOCK_SIZE);
	VEC2_SET(vertexes[16].uv, uv_org.x, uv_dst.y);
	VEC2_SET(vertexes[17].uv, uv_dst.x, uv_dst.y);
	VEC2_SET(vertexes[18].uv, uv_dst.x, uv_org.y);
	VEC2_SET(vertexes[19].uv, uv_org.x, uv_org.y);
	/* down */
	VEC3_SET(vertexes[20].position, 0         ,             -BREAK_OFFSET, 0         );
	VEC3_SET(vertexes[21].position, BLOCK_SIZE,             -BREAK_OFFSET, 0         );
	VEC3_SET(vertexes[22].position, BLOCK_SIZE,             -BREAK_OFFSET, BLOCK_SIZE);
	VEC3_SET(vertexes[23].position, 0         ,             -BREAK_OFFSET, BLOCK_SIZE);
	VEC2_SET(vertexes[20].uv, uv_org.x, uv_org.y);
	VEC2_SET(vertexes[21].uv, uv_dst.x, uv_org.y);
	VEC2_SET(vertexes[22].uv, uv_dst.x, uv_dst.y);
	VEC2_SET(vertexes[23].uv, uv_org.x, uv_dst.y);
	gfx_set_buffer_data(&raycast->break_vertexes_buffer, vertexes, sizeof(vertexes), 0);
}

static void build_hover_vertexes(struct player_raycast *raycast, const struct aabb *aabb)
{
	struct vec3f positions[8];
	VEC3_SET(positions[0] , aabb->p0.x - OFFSET, aabb->p0.y - OFFSET, aabb->p0.z - OFFSET);
	VEC3_SET(positions[1] , aabb->p1.x + OFFSET, aabb->p0.y - OFFSET, aabb->p0.z - OFFSET);
	VEC3_SET(positions[2] , aabb->p0.x - OFFSET, aabb->p0.y - OFFSET, aabb->p1.z + OFFSET);
	VEC3_SET(positions[3] , aabb->p1.x + OFFSET, aabb->p0.y - OFFSET, aabb->p1.z + OFFSET);
	VEC3_SET(positions[4] , aabb->p0.x - OFFSET, aabb->p1.y + OFFSET, aabb->p0.z - OFFSET);
	VEC3_SET(positions[5] , aabb->p1.x + OFFSET, aabb->p1.y + OFFSET, aabb->p0.z - OFFSET);
	VEC3_SET(positions[6] , aabb->p0.x - OFFSET, aabb->p1.y + OFFSET, aabb->p1.z + OFFSET);
	VEC3_SET(positions[7] , aabb->p1.x + OFFSET, aabb->p1.y + OFFSET, aabb->p1.z + OFFSET);
	gfx_set_buffer_data(&raycast->hover_vertexes_buffer, positions, sizeof(positions), 0);
}
