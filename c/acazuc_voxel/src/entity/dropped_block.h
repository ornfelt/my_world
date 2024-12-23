#ifndef DROPPED_BLOCK_H
#define DROPPED_BLOCK_H

#include "entity.h"

#include <gfx/objects.h>

struct dropped_block
{
	struct entity entity;
	gfx_buffer_t vertexes_buffer;
	gfx_buffer_t indices_buffer;
	uint64_t indices_nb;
	int64_t created;
	uint8_t number;
	uint8_t type;
};

struct entity *dropped_block_new(struct world *world, struct chunk *chunk, uint8_t type, struct vec3f pos, struct vec3f vel);

#endif
