#include "generator/generator.h"

#include "world/block.h"
#include "world/world.h"
#include "world/chunk.h"

#include "log.h"

#include <stdlib.h>

struct generator_minable
{
	struct generator generator;
	uint8_t block;
	uint8_t size;
};

static void generate(struct generator *generator, struct chunk *chunk, uint8_t x, uint8_t y, uint8_t z);

static const struct generator_op op =
{
	.generate = generate,
};

struct generator *generator_minable_new(uint8_t block, uint8_t size)
{
	struct generator_minable *generator = malloc(sizeof(*generator));
	if (!generator)
	{
		LOG_ERROR("minable generator allocation failed");
		return NULL;
	}
	generator->generator.op = &op;
	generator->block = block;
	generator->size = size;
	return &generator->generator;
}

static void generate(struct generator *generator, struct chunk *chunk, uint8_t x, uint8_t y, uint8_t z)
{
	struct generator_minable *minable = (struct generator_minable*)generator;
	for (int8_t x2 = 0; x2 < minable->size; ++x2)
	{
		for (int8_t y2 = 0; y2 < minable->size; ++y2)
		{
			for (int8_t z2 = 0; z2 < minable->size; ++z2)
			{
				struct block current_block;
				if (world_get_block(chunk->world, chunk->x + x + x2, y + y2, chunk->z + z + z2, &current_block)
				 && current_block.type == 1)
					world_set_block(chunk->world, chunk->x + x + x2, y + y2, chunk->z + z + z2, minable->block);
			}
		}
	}
}
