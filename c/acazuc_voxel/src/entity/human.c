#include "entity/body_part.h"
#include "entity/entity.h"
#include "entity/def.h"

#include "world/world.h"

#include "textures.h"
#include "voxel.h"

#include <stdlib.h>
#include <math.h>

struct human
{
	struct entity entity;
};

bool human_def_init(struct human_def *def)
{
	struct vec3f org;
	struct vec3f size;
	struct vec2f uv;
	struct vec3f pos;
	VEC3_SET(org, -4, 0, -4);
	VEC3_SET(size, 8, 8, 8);
	VEC2_SET(uv, 0, 0);
	VEC3_SET(pos, 0, 9, 0);
	def->head = body_part_new(org, size, uv, pos);
	if (!def->head)
		return false;
	VEC3_SET(org, -4, -3, -2);
	VEC3_SET(size, 8, 12, 4);
	VEC2_SET(uv, 16, 16);
	VEC3_SET(pos, 0, 0, 0);
	def->body = body_part_new(org, size, uv, pos);
	if (!def->body)
		return false;
	VEC3_SET(org, 0, -10, -2);
	VEC3_SET(size, 4, 12, 4);
	VEC2_SET(uv, 40, 16);
	VEC3_SET(pos, 4, 7, 0);
	def->arm_l = body_part_new(org, size, uv, pos);
	if (!def->arm_l)
		return false;
	VEC3_SET(org, -4, -10, -2);
	VEC3_SET(size, 4, 12, 4);
	VEC2_SET(uv, 40, 16);
	VEC3_SET(pos, -4, 7, 0);
	def->arm_r = body_part_new(org, size, uv, pos);
	if (!def->arm_r)
		return false;
	VEC3_SET(org, -2, -12, -2);
	VEC3_SET(size, 4, 12, 4);
	VEC2_SET(uv, 0, 16);
	VEC3_SET(pos, -2, -3, 0);
	def->leg_l = body_part_new(org, size, uv, pos);
	if (!def->leg_l)
		return false;
	VEC3_SET(org, -2, -12, -2);
	VEC3_SET(size, 4, 12, 4);
	VEC2_SET(uv, 0, 16);
	VEC3_SET(pos, 2, -3, 0);
	def->leg_r = body_part_new(org, size, uv, pos);
	if (!def->leg_r)
		return false;
	return true;
}

void human_def_destroy(struct human_def *def)
{
	body_part_delete(def->head);
	body_part_delete(def->body);
	body_part_delete(def->arm_l);
	body_part_delete(def->arm_r);
	body_part_delete(def->leg_l);
	body_part_delete(def->leg_r);
}

static void human_draw(struct entity *entity)
{
	entity_priv_draw(entity);
	texture_bind(&g_voxel->textures->character);
	float t = g_voxel->frametime / 1000000000.0;
	struct human_def *def = &g_voxel->entities->human;
	VEC3_SET(def->arm_l->rot, 0, 0,  sinf(t * 1) * 0.05 + 0.05);
	VEC3_SET(def->arm_r->rot, 0, 0, -sinf(t * 1) * 0.05 - 0.05);
	struct mat4f model = entity_get_mat(entity);
	body_part_draw(def->head, entity->world, &model);
	body_part_draw(def->body, entity->world, &model);
	body_part_draw(def->arm_l, entity->world, &model);
	body_part_draw(def->arm_r, entity->world, &model);
	body_part_draw(def->leg_l, entity->world, &model);
	body_part_draw(def->leg_r, entity->world, &model);
}

static const struct entity_vtable human_vtable =
{
	.draw = human_draw,
};

struct entity *human_new(struct world *world, struct chunk *chunk)
{
	struct human *human = malloc(sizeof(*human));
	if (!human)
		return NULL;
	entity_init(&human->entity, world, chunk);
	human->entity.vtable = &human_vtable;
	entity_set_size(&human->entity, (struct vec3f){0.6, 1.8, 0.6});
	return &human->entity;
}
