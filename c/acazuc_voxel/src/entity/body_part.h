#ifndef BODY_PART_H
#define BODY_PART_H

#include <jks/vec3.h>
#include <jks/vec2.h>

#include <gfx/objects.h>

struct mat4f;
struct world;

struct body_part
{
	gfx_attributes_state_t attributes_state;
	gfx_buffer_t vertexes_buffer;
	gfx_buffer_t indices_buffer;
	gfx_buffer_t uniform_buffer;
	struct vec3f pos;
	struct vec3f rot;
};

struct body_part *body_part_new(struct vec3f org, struct vec3f size, struct vec2f uv, struct vec3f pos);
void body_part_delete(struct body_part *body_part);
void body_part_draw(struct body_part *body_part, struct world *world, const struct mat4f *mat);

#endif
