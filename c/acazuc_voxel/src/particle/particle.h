#ifndef PARTICLE_H
#define PARTICLE_H

#include "entity/entity.h"

#include <jks/array.h>
#include <jks/vec2.h>
#include <jks/vec3.h>

#include <sys/queue.h>

struct particle
{
	struct entity entity;
	struct vec2f uv_size;
	struct vec4f color;
	struct vec2f size;
	struct vec2f uv;
	int64_t duration;
	int64_t age;
	TAILQ_ENTRY(particle) chain;
};

struct entity *particle_new(struct world *world, struct chunk *chunk, struct vec3f pos, struct vec2f size, struct vec3f vel, struct vec2f uv, struct vec2f uv_size, struct vec4f color, uint8_t light);
void particle_draw(struct particle *particle, struct jks_array *vertexes_array, struct jks_array *indices_array, const struct mat4f *player_mat);

#endif
