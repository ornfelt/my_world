#include "entity/body_part.h"
#include "entity/entity.h"
#include "entity/def.h"

#include "world/world.h"

#include "textures.h"
#include "shaders.h"
#include "voxel.h"

#include <gfx/device.h>

#include <stdlib.h>
#include <math.h>

struct cow
{
	struct entity entity;
};

bool cow_def_init(struct cow_def *def)
{
	struct vec3f org;
	struct vec3f size;
	struct vec2f uv;
	struct vec3f pos;
	struct vec3f rot;
	VEC3_SET(org, -4, -4, -3);
	VEC3_SET(size, 8, 8, 6);
	VEC2_SET(uv, 0, 0);
	VEC3_SET(pos, 0, 8.4, 12);
	def->head = body_part_new(org, size, uv, pos);
	if (!def->head)
		return false;
	VEC3_SET(org, -6, -9, -5);
	VEC3_SET(size, 12, 18, 10);
	VEC2_SET(uv, 18, 4);
	VEC3_SET(pos, 0, 5.4, 0);
	VEC3_SET(rot, M_PI / 2, 0, 0);
	def->body = body_part_new(org, size, uv, pos);
	if (!def->body)
		return false;
	def->body->rot = rot;
	VEC3_SET(org, -2, -6, -2);
	VEC3_SET(size, 4, 12, 4);
	VEC2_SET(uv, 0, 16);
	VEC3_SET(pos, -4, -5.6, 6);
	def->leg_fl = body_part_new(org, size, uv, pos);
	if (!def->leg_fl)
		return false;
	VEC3_SET(org, -2, -6, -2);
	VEC3_SET(size, 4, 12, 4);
	VEC2_SET(uv, 0, 16);
	VEC3_SET(pos, 4, -5.6, 6);
	def->leg_fr = body_part_new(org, size, uv, pos);
	if (!def->leg_fr)
		return false;
	VEC3_SET(org, -2, -6, -2);
	VEC3_SET(size, 4, 12, 4);
	VEC2_SET(uv, 0, 16);
	VEC3_SET(pos, -4, -5.6, -7);
	def->leg_bl = body_part_new(org, size, uv, pos);
	if (!def->leg_bl)
		return false;
	VEC3_SET(org, -2, -6, -2);
	VEC3_SET(size, 4, 12, 4);
	VEC2_SET(uv, 0, 16);
	VEC3_SET(pos, 4, -5.6, -7);
	def->leg_br = body_part_new(org, size, uv, pos);
	if (!def->leg_br)
		return false;
	return true;
}

void cow_def_destroy(struct cow_def *def)
{
	body_part_delete(def->head);
	body_part_delete(def->body);
	body_part_delete(def->horn_l);
	body_part_delete(def->horn_r);
	body_part_delete(def->leg_fl);
	body_part_delete(def->leg_fr);
	body_part_delete(def->leg_bl);
	body_part_delete(def->leg_br);
}

static void cow_draw(struct entity *entity)
{
	entity_priv_draw(entity);
	texture_bind(&g_voxel->textures->cow);
	struct mat4f model = entity_get_mat(entity);
	struct cow_def *def = &g_voxel->entities->cow;
	body_part_draw(def->head, entity->world, &model);
	body_part_draw(def->body, entity->world, &model);
	body_part_draw(def->leg_fl, entity->world, &model);
	body_part_draw(def->leg_fr, entity->world, &model);
	body_part_draw(def->leg_bl, entity->world, &model);
	body_part_draw(def->leg_br, entity->world, &model);
}

static const struct entity_vtable cow_vtable =
{
	.draw = cow_draw,
};

struct entity *cow_new(struct world *world, struct chunk *chunk)
{
	struct cow *cow = malloc(sizeof(*cow));
	if (!cow)
		return NULL;
	entity_init(&cow->entity, world, chunk);
	cow->entity.vtable = &cow_vtable;
	entity_set_size(&cow->entity, (struct vec3f){0.9, 1.4, 0.9});
	return &cow->entity;
}
