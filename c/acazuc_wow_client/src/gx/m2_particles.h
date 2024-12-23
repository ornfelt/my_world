#ifndef GX_M2_PARTICLES_H
#define GX_M2_PARTICLES_H

#include <jks/array.h>
#include <jks/vec4.h>
#include <jks/vec3.h>

#include <gfx/objects.h>

#include <stdbool.h>

#define MAX_PARTICLES 1024

struct gx_m2_render_params;
struct wow_m2_particle;
struct gx_m2_instance;
struct gx_frame;
struct gx_blp;

struct gx_m2_particle
{
	uint64_t created;
	uint64_t lifespan;
	float spin_random;
	struct vec4f position;
	struct vec4f velocity;
};

struct gx_m2_particles_emitter_frame
{
	gfx_attributes_state_t attributes_state;
	gfx_buffer_t vertexes_buffer;
	gfx_buffer_t uniform_buffer;
	struct jks_array vertexes; /* struct shader_particle_input */
};

struct gx_m2_particles_emitter
{
	struct wow_m2_particle *emitter;
	struct gx_m2_particles_emitter_frame frames[RENDER_FRAMES_COUNT];
	struct gx_blp *texture;
	struct jks_array particles; /* struct gx_m2_particle */
	uint32_t pipeline_state;
	float alpha_test;
	struct vec3f fog_color;
	uint64_t last_spawned;
	uint16_t emitter_type;
	bool fog_override;
};

struct gx_m2_particles
{
	struct gx_m2_instance *parent;
	struct jks_array emitters; /* struct gx_m2_particles_emitter */
	bool initialized;
};

struct gx_m2_particles *gx_m2_particles_new(struct gx_m2_instance *parent);
void gx_m2_particles_delete(struct gx_m2_particles *particles);
void gx_m2_particles_update(struct gx_m2_particles *particles, struct gx_frame *frame, struct gx_m2_render_params *params);
void gx_m2_particles_render(struct gx_m2_particles *particles, struct gx_frame *frame, struct gx_m2_render_params *params);

#endif
