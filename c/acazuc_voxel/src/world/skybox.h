#ifndef SKYBOX_H
#define SKYBOX_H

#include <gfx/objects.h>

struct world;

struct skybox
{
	gfx_attributes_state_t skybox_attributes_state;
	gfx_attributes_state_t moon_attributes_state;
	gfx_attributes_state_t sun_attributes_state;
	gfx_buffer_t sunmoon_uniform_buffer;
	gfx_buffer_t skybox_uniform_buffer;
	gfx_buffer_t skybox_vertexes_buffer;
	gfx_buffer_t skybox_indices_buffer;
	gfx_buffer_t moon_vertexes_buffer;
	gfx_buffer_t moon_indices_buffer;
	gfx_buffer_t sun_vertexes_buffer;
	gfx_buffer_t sun_indices_buffer;
	struct world *world;
};

void skybox_init(struct skybox *skybox, struct world *world);
void skybox_destroy(struct skybox *skybox);
void skybox_draw(struct skybox *skybox);

#endif
