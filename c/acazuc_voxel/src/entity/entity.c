#include "entity/entity.h"

#include "player/player.h"

#include "world/block.h"
#include "world/world.h"
#include "world/chunk.h"

#include "shaders.h"
#include "voxel.h"

#include <gfx/device.h>

#include <stdlib.h>
#include <math.h>

static void update_parent_chunk(struct entity *entity);

void entity_init(struct entity *entity, struct world *world, struct chunk *chunk)
{
	entity->world = world;
	entity->chunk = chunk;
	entity->gravity = 0.08;
	entity->is_on_floor = false;
	entity->deleted = false;
	entity->flying = false;
	VEC3_SET(entity->sliperness, .91, .98, .91);
	VEC3_SET(entity->pos_org, 0, 0, 0);
	VEC3_SET(entity->pos_dst, 0, 0, 0);
	VEC3_SET(entity->size, 1, 1, 1);
	VEC3_SET(entity->pos, 0, 0, 0);
	VEC3_SET(entity->rot, 0, 0, 0);
	VEC3_SET(entity->aabb.p0, 0, 0, 0);
	VEC3_SET(entity->aabb.p1, 0, 0, 0);
	entity->uniform_buffer = GFX_BUFFER_INIT();
}

void entity_delete(struct entity *entity)
{
	if (entity->vtable->destroy)
		entity->vtable->destroy(entity);
	gfx_delete_buffer(g_voxel->device, &entity->uniform_buffer);
	free(entity);
}

void entity_priv_tick(struct entity *entity)
{
	if (entity != &entity->world->player->entity && !entity->chunk)
		entity->vtable->update_parent_chunk(entity);
	entity->pos_org = entity->pos;
	if (!entity->flying)
		entity->pos_dst.y -= entity->gravity;
	entity_move(entity, entity->pos_dst);
	VEC3_MUL(entity->pos_dst, entity->pos_dst, entity->sliperness);
	if (entity->flying)
	{
		entity->pos_dst.x *= 0.7;
		entity->pos_dst.y *= 0.7;
		entity->pos_dst.z *= 0.7;
	}
	else if (entity->is_on_floor)
	{
		entity->pos_dst.x *= 0.8;
		entity->pos_dst.z *= 0.8;
	}
}

void entity_tick(struct entity *entity)
{
	if (entity->vtable->tick)
		entity->vtable->tick(entity);
	else
		entity_priv_tick(entity);
}

void entity_priv_draw(struct entity *entity)
{
	if (!entity->uniform_buffer.handle.ptr)
		gfx_create_buffer(g_voxel->device, &entity->uniform_buffer, GFX_BUFFER_UNIFORM, NULL, sizeof(struct shader_entity_model_block), GFX_BUFFER_DYNAMIC);
	struct shader_entity_model_block model_block;
	float light_value = voxel_light_value(world_get_light(entity->world, entity->pos.x, entity->pos.y, entity->pos.z));
	VEC3_SETV(model_block.color, light_value);
	model_block.color.w = 1;
	gfx_set_buffer_data(&entity->uniform_buffer, &model_block, sizeof(model_block), 0);
	gfx_bind_constant(g_voxel->device, 1, &entity->uniform_buffer, sizeof(model_block), 0);
}

void entity_draw(struct entity *entity)
{
	entity->vtable->draw(entity);
}

void entity_jump(struct entity *entity)
{
	entity->is_on_floor = false;
	entity->pos_dst.y += 0.5;
}

void entity_set_pos(struct entity *entity, struct vec3f pos)
{
	entity->pos_org = pos;
	entity->pos = pos;
	struct vec3f semi_size;
	VEC3_MULV(semi_size, entity->size, 0.5);
	VEC3_SUB(entity->aabb.p0, pos, semi_size);
	VEC3_ADD(entity->aabb.p1, pos, semi_size);
	update_parent_chunk(entity);
}

static void update_gravity_sliperness(struct entity *entity)
{
	struct vec3f pos = entity->world->player->entity.pos;
	pos.y -= entity->size.y / 2 - 0.4;
	struct block block;
	if (world_get_block(entity->world, ceilf(pos.x), ceilf(pos.y), ceilf(pos.z), &block))
		entity->in_water = (block.type == 8 || block.type == 9);
	else
		entity->in_water = false;
	if (entity->in_water && !entity->flying)
		VEC3_SETV(entity->sliperness, 0.8);
	else
		VEC3_SET(entity->sliperness, 0.91, 0.98, 0.91);
}

void entity_move(struct entity *entity, struct vec3f dir)
{
	update_gravity_sliperness(entity);
	struct vec3f org = dir;
	struct aabb new_aabb = aabb_expand(&entity->aabb, dir);
	{
		struct jks_array aabbs; /* struct aabb */
		jks_array_init(&aabbs, sizeof(struct aabb), NULL, NULL);
		world_get_aabbs(entity->world, &new_aabb, &aabbs);
		for (uint32_t i = 0; i < aabbs.size; ++i)
			dir.y = aabb_collide_y(JKS_ARRAY_GET(&aabbs, i, struct aabb), &entity->aabb, dir.y);
		entity->aabb.p0.y += dir.y;
		entity->aabb.p1.y += dir.y;
		for (uint32_t i = 0; i < aabbs.size; ++i)
			dir.x = aabb_collide_x(JKS_ARRAY_GET(&aabbs, i, struct aabb), &entity->aabb, dir.x);
		entity->aabb.p0.x += dir.x;
		entity->aabb.p1.x += dir.x;
		for (uint32_t i = 0; i < aabbs.size; ++i)
			dir.z = aabb_collide_z(JKS_ARRAY_GET(&aabbs, i, struct aabb), &entity->aabb, dir.z);
		entity->aabb.p0.z += dir.z;
		entity->aabb.p1.z += dir.z;
		jks_array_destroy(&aabbs);
	}
	VEC3_ADD(entity->pos, entity->aabb.p0, entity->aabb.p1);
	VEC3_MULV(entity->pos, entity->pos, .5);
	if (dir.x != org.x)
		entity->pos_dst.x = 0;
	if (dir.y != org.y)
		entity->pos_dst.y = 0;
	if (dir.z != org.z)
		entity->pos_dst.z = 0;
	if (!entity->flying && entity->in_water && (dir.x != org.x || dir.z != org.z))
	{
		struct aabb tmp;
		struct vec3f offset;
		VEC3_SET(offset, entity->pos_dst.x, entity->pos_dst.y + .6f, entity->pos_dst.z);
		VEC3_ADD(tmp.p0, entity->aabb.p0, offset);
		VEC3_ADD(tmp.p1, entity->aabb.p1, offset);
		struct jks_array tmp2; /* struct aabb */
		jks_array_init(&tmp2, sizeof(struct aabb), NULL, NULL);
		world_get_aabbs(entity->world, &tmp, &tmp2);
		if (!tmp2.size)
			entity->pos_dst.y = 0.3;
		jks_array_destroy(&tmp2);
	}
	entity->is_on_floor = org.y < 0 && dir.y != org.y;
	if (entity->pos.y < -100)
	{
		entity->deleted = true;
		return;
	}
	if (dir.x != org.x || dir.y != org.y || dir.z != org.z)
		update_parent_chunk(entity);
}

void entity_set_size(struct entity *entity, struct vec3f size)
{
	entity->size = size;
	struct vec3f semi_size;
	VEC3_MULV(semi_size, size, 0.5);
	VEC3_SUB(entity->aabb.p0, entity->pos, semi_size);
	//org.y = entity->pos.y;
	VEC3_ADD(entity->aabb.p1, entity->pos, semi_size);
	//dst.y = entity->pos.y + size.y;
}

struct vec3f entity_get_real_pos(struct entity *entity)
{
	struct vec3f delta;
	VEC3_SUB(delta, entity->pos, entity->pos_org);
	VEC3_MULV(delta, delta, g_voxel->delta);
	VEC3_ADD(delta, delta, entity->pos_org);
	return delta;
}

void entity_priv_update_parent_chunk(struct entity *entity)
{
	if (entity->deleted)
		return;
	int32_t chunk_x = chunk_get_coord(floorf(entity->pos.x));
	int32_t chunk_z = chunk_get_coord(floorf(entity->pos.z));
	if (!entity->chunk || entity->chunk->x != chunk_x || entity->chunk->z != chunk_z)
	{
		struct chunk *chunk = world_get_chunk(entity->world, chunk_x, chunk_z);
		if (!chunk)
		{
			entity->deleted = true;
			return;
		}
		if (entity->chunk)
			entities_remove(&entity->chunk->entities, entity);
		entity->chunk = chunk;
		entities_add(&entity->chunk->entities, entity);
	}
}

static void update_parent_chunk(struct entity *entity)
{
	if (entity->vtable->update_parent_chunk)
		entity->vtable->update_parent_chunk(entity);
	else
		entity_priv_update_parent_chunk(entity);
}

struct mat4f entity_get_mat(struct entity *entity)
{
	struct vec3f pos = entity_get_real_pos(entity);
	struct mat4f model;
	struct mat4f tmp;
	struct vec3f scale;
	VEC3_SETV(scale, 0.0625);
	MAT4_IDENTITY(tmp);
	MAT4_TRANSLATE(model, tmp, pos);
	MAT4_ROTATEZ(float, tmp, model, entity->rot.z);
	MAT4_ROTATEY(float, model, tmp, entity->rot.y);
	MAT4_ROTATEX(float, tmp, model, entity->rot.x);
	MAT4_SCALE(model, tmp, scale);
	return model;
}
