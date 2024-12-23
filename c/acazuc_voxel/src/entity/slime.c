#include "entity/body_part.h"
#include "entity/entity.h"
#include "entity/def.h"

#include "world/world.h"

#include "textures.h"
#include "voxel.h"

#include <stdlib.h>

struct slime
{
	struct entity entity;
};

bool slime_def_init(struct slime_def *def)
{
	struct vec3f org;
	struct vec3f size;
	struct vec2f uv;
	struct vec3f pos;
	VEC3_SET(org, -4, -4, -4);
	VEC3_SET(size, 8, 8, 8);
	VEC2_SET(uv, 0, 0);
	VEC3_SET(pos, 0, -4.4, 0);
	def->body = body_part_new(org, size, uv, pos);
	if (!def->body)
		return false;
	return true;
}

void slime_def_destroy(struct slime_def *def)
{
	body_part_delete(def->body);
}

static void slime_draw(struct entity *entity)
{
	entity_priv_draw(entity);
	texture_bind(&g_voxel->textures->slime);
	struct mat4f model = entity_get_mat(entity);
	struct slime_def *def = &g_voxel->entities->slime;
	body_part_draw(def->body, entity->world, &model);
}

static const struct entity_vtable slime_vtable =
{
	.draw = slime_draw,
};

struct entity *slime_new(struct world *world, struct chunk *chunk)
{
	struct slime *slime = malloc(sizeof(*slime));
	if (!slime)
		return NULL;
	entity_init(&slime->entity, world, chunk);
	slime->entity.vtable = &slime_vtable;
	entity_set_size(&slime->entity, (struct vec3f){1.01, 1.01, 1.01});
	return &slime->entity;
}
