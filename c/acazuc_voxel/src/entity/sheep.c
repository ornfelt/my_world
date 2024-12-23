#include "entity/body_part.h"
#include "entity/entity.h"
#include "entity/def.h"

#include "world/world.h"

#include "textures.h"
#include "voxel.h"

#include <stdlib.h>
#include <math.h>

struct sheep
{
	struct entity entity;
};

bool sheep_def_init(struct sheep_def *def)
{
	struct vec3f org;
	struct vec3f size;
	struct vec2f uv;
	struct vec3f pos;
	struct vec3f rot;
	VEC3_SET(org, -3, -3, -4);
	VEC3_SET(size, 6, 6, 8);
	VEC2_SET(uv, 0, 0);
	VEC3_SET(pos, 0, 8, 9);
	def->head = body_part_new(org, size, uv, pos);
	if (!def->head)
		return false;
	VEC3_SET(org, -4, -8, -3);
	VEC3_SET(size, 8, 16, 6);
	VEC2_SET(uv, 28, 8);
	VEC3_SET(pos, 0, 4, 0);
	VEC3_SET(rot, M_PI / 2, 0, 0);
	def->body = body_part_new(org, size, uv, pos);
	if (!def->body)
		return false;
	def->body->rot = rot;
	VEC3_SET(org, -2, -6, -2);
	VEC3_SET(size, 4, 12, 4);
	VEC2_SET(uv, 0, 16);
	VEC3_SET(pos, -3, -5, 5);
	def->leg_fl = body_part_new(org, size, uv, pos);
	if (!def->leg_fl)
		return false;
	VEC3_SET(org, -2, -6, -2);
	VEC3_SET(size, 4, 12, 4);
	VEC2_SET(uv, 0, 16);
	VEC3_SET(pos, 3, -5, 5);
	def->leg_fr = body_part_new(org, size, uv, pos);
	if (!def->leg_fr)
		return false;
	VEC3_SET(org, -2, -6, -2);
	VEC3_SET(size, 4, 12, 4);
	VEC2_SET(uv, 0, 16);
	VEC3_SET(pos, -3, -5, -7);
	def->leg_bl = body_part_new(org, size, uv, pos);
	if (!def->leg_bl)
		return false;
	VEC3_SET(org, -2, -6, -2);
	VEC3_SET(size, 4, 12, 4);
	VEC2_SET(uv, 0, 16);
	VEC3_SET(pos, 3, -5, -7);
	def->leg_br = body_part_new(org, size, uv, pos);
	if (!def->leg_br)
		return false;
	return true;
}

void sheep_def_destroy(struct sheep_def *def)
{
	body_part_delete(def->head);
	body_part_delete(def->body);
	body_part_delete(def->leg_fl);
	body_part_delete(def->leg_fr);
	body_part_delete(def->leg_bl);
	body_part_delete(def->leg_br);
}

static void sheep_draw(struct entity *entity)
{
	entity_priv_draw(entity);
	texture_bind(&g_voxel->textures->sheep);
	struct mat4f model = entity_get_mat(entity);
	struct sheep_def *def = &g_voxel->entities->sheep;
	body_part_draw(def->head, entity->world, &model);
	body_part_draw(def->body, entity->world, &model);
	body_part_draw(def->leg_fl, entity->world, &model);
	body_part_draw(def->leg_fr, entity->world, &model);
	body_part_draw(def->leg_bl, entity->world, &model);
	body_part_draw(def->leg_br, entity->world, &model);
}

static const struct entity_vtable sheep_vtable =
{
	.draw = sheep_draw,
};

struct entity *sheep_new(struct world *world, struct chunk *chunk)
{
	struct sheep *sheep = malloc(sizeof(*sheep));
	if (!sheep)
		return NULL;
	entity_init(&sheep->entity, world, chunk);
	sheep->entity.vtable = &sheep_vtable;
	entity_set_size(&sheep->entity, (struct vec3f){0.9, 1.3, 0.9});
	return &sheep->entity;
}
