#include "entity/entities.h"
#include "entity/entity.h"

#include "world/chunk.h"
#include "world/world.h"

#include "graphics.h"
#include "shaders.h"
#include "voxel.h"
#include "log.h"

#include <gfx/device.h>

bool entities_init(struct entities *entities, struct chunk *chunk)
{
	entities->chunk = chunk;
	TAILQ_INIT(&entities->entities);
	entities->uniform_buffer = GFX_BUFFER_INIT();
	return true;
}

void entities_destroy(struct entities *entities)
{
	struct entity *entity;
	while ((entity = TAILQ_FIRST(&entities->entities)))
	{
		TAILQ_REMOVE(&entities->entities, entity, chain);
		entity_delete(entity);
	}
	gfx_delete_buffer(g_voxel->device, &entities->uniform_buffer);
}

void entities_tick(struct entities *entities)
{
	struct entity *entity = TAILQ_FIRST(&entities->entities);
	while (entity)
	{
		struct entity *next = TAILQ_NEXT(entity, chain);
		entity_tick(entity);
		if (entity->deleted)
		{
			TAILQ_REMOVE(&entities->entities, entity, chain);
			entity_delete(entity);
		}
		entity = next;
	}
}

void entities_draw(struct entities *entities)
{
	if (TAILQ_EMPTY(&entities->entities))
	{
		static int a = 0;
		if (!a && entities->chunk->x == 16 && entities->chunk->z == 0)
		{
			struct vec3f pos;
			a = 1;

			VEC3_SET(pos, entities->chunk->x + .5, 128, entities->chunk->z + .5);
			struct entity *creeper = creeper_new(entities->chunk->world, entities->chunk);
			entity_set_pos(creeper, pos);
			entities_add(entities, creeper);

			VEC3_SET(pos, entities->chunk->x + 5.5, 128, entities->chunk->z + .5);
			struct entity *human = human_new(entities->chunk->world, entities->chunk);
			entity_set_pos(human, pos);
			entities_add(entities, human);

			VEC3_SET(pos, entities->chunk->x + 10.5, 128, entities->chunk->z + .5);
			struct entity *pig = pig_new(entities->chunk->world, entities->chunk);
			entity_set_pos(pig, pos);
			entities_add(entities, pig);

			VEC3_SET(pos, entities->chunk->x + 15.5, 128, entities->chunk->z + .5);
			struct entity *pigman = pigman_new(entities->chunk->world, entities->chunk);
			entity_set_pos(pigman, pos);
			entities_add(entities, pigman);

			VEC3_SET(pos, entities->chunk->x + .5, 128, entities->chunk->z + 5.5); 
			struct entity *pigzombie = pigzombie_new(entities->chunk->world, entities->chunk);
			entity_set_pos(pigzombie, pos);
			entities_add(entities, pigzombie);

			VEC3_SET(pos, entities->chunk->x + 5.5, 128, entities->chunk->z + 5.5);
			struct entity *sheep = sheep_new(entities->chunk->world, entities->chunk);
			entity_set_pos(sheep, pos);
			entities_add(entities, sheep);

			VEC3_SET(pos, entities->chunk->x + 10.5, 128, entities->chunk->z + 5.5);
			struct entity *skeleton = skeleton_new(entities->chunk->world, entities->chunk);
			entity_set_pos(skeleton, pos);
			entities_add(entities, skeleton);

			VEC3_SET(pos, entities->chunk->x + 15.5, 128, entities->chunk->z + 5.5);
			struct entity *slime = slime_new(entities->chunk->world, entities->chunk);
			entity_set_pos(slime, pos);
			entities_add(entities, slime);

			VEC3_SET(pos, entities->chunk->x + .5, 128, entities->chunk->z + 10.5);
			struct entity *zombie = zombie_new(entities->chunk->world, entities->chunk);
			entity_set_pos(zombie, pos);
			entities_add(entities, zombie);

			VEC3_SET(pos, entities->chunk->x + 5.5, 128, entities->chunk->z + 10.5);
			struct entity *cow = cow_new(entities->chunk->world, entities->chunk);
			entity_set_pos(cow, pos);
			entities_add(entities, cow);
		}
		else
			return;
	}
	if (!entities->uniform_buffer.handle.ptr)
		gfx_create_buffer(g_voxel->device, &entities->uniform_buffer, GFX_BUFFER_UNIFORM, NULL, sizeof(struct shader_entity_scene_block), GFX_BUFFER_DYNAMIC);
	struct shader_entity_scene_block scene_block;
	scene_block.fog_color = entities->chunk->world->fog_color;
	scene_block.fog_distance = entities->chunk->world->fog_distance;
	scene_block.fog_density = entities->chunk->world->fog_density;
	gfx_set_buffer_data(&entities->uniform_buffer, &scene_block, sizeof(scene_block), 0);
	gfx_bind_constant(g_voxel->device, 2, &entities->uniform_buffer, sizeof(scene_block), 0);
	gfx_bind_pipeline_state(g_voxel->device, &g_voxel->graphics->entity.pipeline_state);
	struct entity *entity;
	TAILQ_FOREACH(entity, &entities->entities, chain)
		entity_draw(entity);
}

void entities_add(struct entities *entities, struct entity *entity)
{
	TAILQ_INSERT_TAIL(&entities->entities, entity, chain);
}

void entities_remove(struct entities *entities, struct entity *entity)
{
	TAILQ_REMOVE(&entities->entities, entity, chain);
}
