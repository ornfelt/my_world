#ifndef PARTICLES_H
#define PARTICLES_H

#include <jks/array.h>

#include <gfx/objects.h>

#include <sys/queue.h>

struct particle;
struct chunk;

struct particles
{
	TAILQ_HEAD(, particle) particles;
	struct jks_array vertexes; /* struct shader_particles_vertex */
	struct jks_array indices; /* uint32_t */
	gfx_attributes_state_t attributes_state;
	gfx_buffer_t vertexes_buffer;
	gfx_buffer_t indices_buffer;
	gfx_buffer_t uniform_buffer;
	struct chunk *chunk;
	uint32_t vertexes_size;
	uint32_t indices_size;
};

bool particles_init(struct particles *particles, struct chunk *chunk);
void particles_destroy(struct particles *particles);
void particles_tick(struct particles *particles);
void particles_draw(struct particles *particles);
void particles_add(struct particles *particles, struct particle *particle);
void particles_remove(struct particles *particles, struct particle *particle);

#endif
