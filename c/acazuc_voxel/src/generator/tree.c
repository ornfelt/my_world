#include "generator/generator.h"

#include "world/world.h"
#include "world/chunk.h"

#include "log.h"

#include <stdlib.h>

struct generator_tree
{
	struct generator generator;
};

static void generate(struct generator *generator, struct chunk *chunk, uint8_t x, uint8_t y, uint8_t z);

static const struct generator_op op =
{
	.generate = generate,
};

struct generator *generator_tree_new(void)
{
	struct generator_tree *generator = malloc(sizeof(*generator));
	if (!generator)
	{
		LOG_ERROR("tree generator allocation failed");
		return NULL;
	}
	generator->generator.op = &op;
	return &generator->generator;
}

static void generate(struct generator *generator, struct chunk *chunk, uint8_t x, uint8_t y, uint8_t z)
{
	chunk_set_block(chunk, x, y - 1, z, 3);
	for (int8_t y2 = 3; y2 <= 6; ++y2)
	{
		int8_t x2_min = -2;
		int8_t x2_max = 2;
		int8_t z2_min = -2;
		int8_t z2_max = 2;
		if (y2 >= 5)
		{
			++x2_min;
			--x2_max;
			++z2_min;
			--z2_max;
		}
		for (int8_t x2 = x2_min; x2 <= x2_max; ++x2)
		{
			for (int8_t z2 = z2_min; z2 <= z2_max; ++z2)
			{
				if (x2 == 0 && z2 == 0 && y2 <= 5)
					continue;
				if (y2 == 6 && x2 && (x2 == z2 || x2 == -z2))
					continue;
				world_set_block_if_replaceable(chunk->world, chunk->x + x + x2, y + y2, chunk->z + z + z2, 18);
			}
		}
	}
	for (int32_t y2 = y; y2 < y + 6; ++y2)
		chunk_set_block_if_replaceable(chunk, x, y2, z, 17);
}
