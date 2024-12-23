#ifndef WORLD_TESSELLATOR_H
#define WORLD_TESSELLATOR_H

#include <jks/array.h>

struct tessellator
{
	struct jks_array indices; /* uint32_t */
	struct jks_array vertexes; /* struct shader_blocks_vertex */
};

#endif
