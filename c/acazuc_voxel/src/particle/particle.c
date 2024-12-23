#include "particle/particle.h"

#include "world/block.h"
#include "world/world.h"
#include "world/chunk.h"

#include "shaders.h"
#include "voxel.h"
#include "log.h"

#include <stdlib.h>
#include <math.h>

static void particle_tick(struct entity *entity);
static void particle_update_parent_chunk(struct entity *entity);

static const struct entity_vtable particle_vtable =
{
	.tick = particle_tick,
	.update_parent_chunk = particle_update_parent_chunk,
};

struct entity *particle_new(struct world *world, struct chunk *chunk, struct vec3f pos, struct vec2f size, struct vec3f vel, struct vec2f uv, struct vec2f uv_size, struct vec4f color, uint8_t light)
{
	struct particle *particle = malloc(sizeof(*particle));
	if (!particle)
		return NULL;
	entity_init(&particle->entity, world, chunk);
	particle->entity.vtable = &particle_vtable;
	particle->uv_size = uv_size;
	particle->size = size;
	particle->uv = uv;
	particle->age = 0;
	particles_add(&chunk->particles, particle);
	particle->entity.gravity = 0.04;
	entity_set_size(&particle->entity, (struct vec3f){0.2, 0.2, 0.2});
	entity_set_pos(&particle->entity, pos);
	particle->entity.pos_dst = vel;
	particle->duration = 4 / (rand() * 0.9 / RAND_MAX + 0.1);
	float light_val = voxel_light_value(light);
	VEC3_MULV(particle->color, color, light_val);
	particle->color.w = color.w;
	return &particle->entity;
}

static void particle_tick(struct entity *entity)
{
	struct particle *particle = (struct particle*)entity;
	if (particle->age > particle->duration)
	{
		entity->deleted = true;
		return;
	}
	particle->age++;
	entity_priv_tick(entity);
	VEC3_MULV(entity->pos_dst, entity->pos_dst, .98);
	if (entity->is_on_floor)
		VEC3_MULV(entity->pos_dst, entity->pos_dst, .7);
}

void particle_draw(struct particle *particle, struct jks_array *vertexes_array, struct jks_array *indices_array, const struct mat4f *player_mat)
{
	struct vec3f pos = entity_get_real_pos(&particle->entity);
	uint32_t current_indice = vertexes_array->size;
	struct shader_particles_vertex *vertexes = jks_array_grow(vertexes_array, 4);
	if (!vertexes)
	{
		LOG_ERROR("allocation failed");
		abort();
	}
#define SET_VERTEX(idx, uv1, uv2) \
do \
{ \
	struct vec4f tmp; \
	struct vec4f res; \
	VEC4_SET(tmp, uv1, uv2, 0, 1); \
	MAT4_VEC4_MUL(res, *player_mat, tmp); \
	VEC3_ADD(res, res, pos); \
	VEC4_CPY(vertexes[idx].position, res); \
} while (0)
	SET_VERTEX(0, -particle->size.x / 2, -particle->size.y / 2);
	SET_VERTEX(1,  particle->size.x / 2, -particle->size.y / 2);
	SET_VERTEX(2,  particle->size.x / 2,  particle->size.y / 2);
	SET_VERTEX(3, -particle->size.x / 2,  particle->size.y / 2);
#undef SET_VERTEX
	VEC4_CPY(vertexes[0].color, particle->color);
	VEC4_CPY(vertexes[1].color, particle->color);
	VEC4_CPY(vertexes[2].color, particle->color);
	VEC4_CPY(vertexes[3].color, particle->color);
	VEC2_SET(vertexes[0].uv, particle->uv.x                      , particle->uv.y + particle->uv_size.y);
	VEC2_SET(vertexes[1].uv, particle->uv.x + particle->uv_size.x, particle->uv.y + particle->uv_size.y);
	VEC2_SET(vertexes[2].uv, particle->uv.x + particle->uv_size.x, particle->uv.y);
	VEC2_SET(vertexes[3].uv, particle->uv.x                      , particle->uv.y);
	uint32_t *indices = jks_array_grow(indices_array, 6);
	if (!indices)
	{
		LOG_ERROR("allocation failed");
		abort();
	}
	indices[0] = current_indice + 0;
	indices[1] = current_indice + 3;
	indices[2] = current_indice + 1;
	indices[3] = current_indice + 2;
	indices[4] = current_indice + 1;
	indices[5] = current_indice + 3;
}

static void particle_update_parent_chunk(struct entity *entity)
{
	struct particle *particle = (struct particle*)entity;
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
			particles_remove(&entity->chunk->particles, particle);
		entity->chunk = chunk;
		particles_add(&entity->chunk->particles, particle);
	}
}
