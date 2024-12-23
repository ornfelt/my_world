#ifndef CLOUDS_H
#define CLOUDS_H

#include <gfx/objects.h>

struct world;

struct clouds
{
	struct world *world;
	gfx_attributes_state_t attributes_state;
	gfx_buffer_t vertexes_buffer;
	gfx_buffer_t indices_buffer;
	gfx_buffer_t uniform_buffer;
	uint32_t indices_nb;
	int64_t last_x_offset;
	float last_player_x;
	float last_player_z;
};

void clouds_init(struct clouds *clouds, struct world *world);
void clouds_destroy(struct clouds *clouds);
void clouds_tick(struct clouds *clouds);
void clouds_draw(struct clouds *clouds);
void clouds_rebuild(struct clouds *clouds);

#endif
