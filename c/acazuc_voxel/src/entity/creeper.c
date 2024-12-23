#include "entity/body_part.h"
#include "entity/entity.h"
#include "entity/def.h"

#include "world/world.h"

#include "textures.h"
#include "voxel.h"

#include <stdlib.h>

struct creeper
{
	struct entity entity;
};

bool creeper_def_init(struct creeper_def *def)
{
	struct vec3f org;
	struct vec3f size;
	struct vec2f uv;
	struct vec3f pos;
	VEC3_SET(org, -4, 0, -4);
	VEC3_SET(size, 8, 8, 8);
	VEC2_SET(uv, 0, 0);
	VEC3_SET(pos, 0, 4, 0);
	def->head = body_part_new(org, size, uv, pos);
	if (!def->head)
		return false;
	VEC3_SET(org, -4, -3, -2);
	VEC3_SET(size, 8, 12, 4);
	VEC2_SET(uv, 16, 16);
	VEC3_SET(pos, 0, -5, 0);
	def->body = body_part_new(org, size, uv, pos);
	if (!def->body)
		return false;
	VEC3_SET(org, -2, -6, -2);
	VEC3_SET(size, 4, 6, 4);
	VEC2_SET(uv, 0, 16);
	VEC3_SET(pos, -2, -8, 4);
	def->leg_fl = body_part_new(org, size, uv, pos);
	if (!def->leg_fl)
		return false;
	VEC3_SET(org, -2, -6, -2);
	VEC3_SET(size, 4, 6, 4);
	VEC2_SET(uv, 0, 16);
	VEC3_SET(pos, 2, -8, 4);
	def->leg_fr = body_part_new(org, size, uv, pos);
	if (!def->leg_fr)
		return false;
	VEC3_SET(org, -2, -6, -2);
	VEC3_SET(size, 4, 6, 4);
	VEC2_SET(uv, 0, 16);
	VEC3_SET(pos, -2, -8, -4);
	def->leg_bl = body_part_new(org, size, uv, pos);
	if (!def->leg_bl)
		return false;
	VEC3_SET(org, -2, -6, -2);
	VEC3_SET(size, 4, 6, 4);
	VEC2_SET(uv, 0, 16);
	VEC3_SET(pos, 2, -8, -4);
	def->leg_br = body_part_new(org, size, uv, pos);
	if (!def->leg_br)
		return false;
	return true;
}

void creeper_def_destroy(struct creeper_def *def)
{
	body_part_delete(def->head);
	body_part_delete(def->body);
	body_part_delete(def->leg_fl);
	body_part_delete(def->leg_fr);
	body_part_delete(def->leg_bl);
	body_part_delete(def->leg_br);
}

static void creeper_draw(struct entity *entity)
{
	entity_priv_draw(entity);
	texture_bind(&g_voxel->textures->creeper);
	struct mat4f model = entity_get_mat(entity);
	struct creeper_def *def = &g_voxel->entities->creeper;
	body_part_draw(def->head, entity->world, &model);
	body_part_draw(def->body, entity->world, &model);
	body_part_draw(def->leg_fl, entity->world, &model);
	body_part_draw(def->leg_fr, entity->world, &model);
	body_part_draw(def->leg_bl, entity->world, &model);
	body_part_draw(def->leg_br, entity->world, &model);
}

static const struct entity_vtable creeper_vtable =
{
	.draw = creeper_draw,
};

struct entity *creeper_new(struct world *world, struct chunk *chunk)
{
	struct creeper *creeper = malloc(sizeof(*creeper));
	if (!creeper)
		return NULL;
	entity_init(&creeper->entity, world, chunk);
	creeper->entity.vtable = &creeper_vtable;
	entity_set_size(&creeper->entity, (struct vec3f){0.6, 1.7, 0.6});
	return &creeper->entity;
}
