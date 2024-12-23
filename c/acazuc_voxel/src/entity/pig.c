#include "entity/body_part.h"
#include "entity/entity.h"
#include "entity/def.h"

#include "world/world.h"

#include "textures.h"
#include "voxel.h"

#include <stdlib.h>
#include <math.h>

struct pig
{
	struct entity entity;
};

bool pig_def_init(struct pig_def *def)
{
	struct vec3f org;
	struct vec3f size;
	struct vec2f uv;
	struct vec3f pos;
	struct vec3f rot;
	VEC3_SET(org, -4, 0, -4);
	VEC3_SET(size, 8, 8, 8);
	VEC2_SET(uv, 0, 0);
	VEC3_SET(pos, 0, .6, 10);
	def->head = body_part_new(org, size, uv, pos);
	if (!def->head)
		return false;
	VEC3_SET(org, -5, -8, -4);
	VEC3_SET(size, 10, 16, 8);
	VEC2_SET(uv, 28, 8);
	VEC3_SET(pos, 0, 2.6, 0);
	VEC3_SET(rot, M_PI / 2, 0, 0);
	def->body = body_part_new(org, size, uv, pos);
	if (!def->body)
		return false;
	def->body->rot = rot;
	VEC3_SET(org, -2, -6, -2);
	VEC3_SET(size, 4, 6, 4);
	VEC2_SET(uv, 0, 16);
	VEC3_SET(pos, -3, -1.4, 5);
	def->leg_fl = body_part_new(org, size, uv, pos);
	if (!def->leg_fl)
		return false;
	VEC3_SET(org, -2, -6, -2);
	VEC3_SET(size, 4, 6, 4);
	VEC2_SET(uv, 0, 16);
	VEC3_SET(pos, 3, -1.4, 5);
	def->leg_fr = body_part_new(org, size, uv, pos);
	if (!def->leg_fr)
		return false;
	VEC3_SET(org, -2, -6, -2);
	VEC3_SET(size, 4, 6, 4);
	VEC2_SET(uv, 0, 16);
	VEC3_SET(pos, -3, -1.4, -7);
	def->leg_bl = body_part_new(org, size, uv, pos);
	if (!def->leg_bl)
		return false;
	VEC3_SET(org, -2, -6, -2);
	VEC3_SET(size, 4, 6, 4);
	VEC2_SET(uv, 0, 16);
	VEC3_SET(pos, 3, -1.4, -7);
	def->leg_br = body_part_new(org, size, uv, pos);
	if (!def->leg_br)
		return false;
	return true;
}

void pig_def_destroy(struct pig_def *def)
{
	body_part_delete(def->head);
	body_part_delete(def->body);
	body_part_delete(def->leg_fl);
	body_part_delete(def->leg_fr);
	body_part_delete(def->leg_bl);
	body_part_delete(def->leg_br);
}

static void pig_draw(struct entity *entity)
{
	entity_priv_draw(entity);
	texture_bind(&g_voxel->textures->pig);
	struct mat4f model = entity_get_mat(entity);
	struct pig_def *def = &g_voxel->entities->pig;
	body_part_draw(def->head, entity->world, &model);
	body_part_draw(def->body, entity->world, &model);
	body_part_draw(def->leg_fl, entity->world, &model);
	body_part_draw(def->leg_fr, entity->world, &model);
	body_part_draw(def->leg_bl, entity->world, &model);
	body_part_draw(def->leg_br, entity->world, &model);
}

static const struct entity_vtable pig_vtable =
{
	.draw = pig_draw,
};

struct entity *pig_new(struct world *world, struct chunk *chunk)
{
	struct pig *pig = malloc(sizeof(*pig));
	if (!pig)
		return NULL;
	entity_init(&pig->entity, world, chunk);
	pig->entity.vtable = &pig_vtable;
	entity_set_size(&pig->entity, (struct vec3f){0.9, 0.9, 0.9});
	return &pig->entity;
}
