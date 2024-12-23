#include "biome/biome.h"

#include "generator/generator.h"

#include "world/block.h"
#include "world/world.h"
#include "world/chunk.h"

#include "log.h"

#include <stdlib.h>

struct biome *biome_new(uint8_t id, const char *name)
{
	struct biome *biome = calloc(sizeof(*biome), 1);
	if (!biome)
	{
		LOG_ERROR("biome allocation failed");
		return NULL;
	}
	biome->id = id;
	biome->name = name;
	biome->diamond_ore_generator = generator_minable_new(56, 2);
	if (!biome->diamond_ore_generator)
		goto err;
	biome->coal_ore_generator = generator_minable_new(16, 2);
	if (!biome->coal_ore_generator)
		goto err;
	biome->iron_ore_generator = generator_minable_new(15, 2);
	if (!biome->iron_ore_generator)
		goto err;
	biome->gold_ore_generator = generator_minable_new(15, 2);
	if (!biome->gold_ore_generator)
		goto err;
	biome->tree_generator = generator_tree_new();
	if (!biome->tree_generator)
		goto err;
	return biome;

err:
	biome_delete(biome);
	return NULL;
}

void biome_delete(struct biome *biome)
{
	if (!biome)
		return;
	generator_delete(biome->diamond_ore_generator);
	generator_delete(biome->coal_ore_generator);
	generator_delete(biome->iron_ore_generator);
	generator_delete(biome->gold_ore_generator);
	generator_delete(biome->tree_generator);
	free(biome);
}

static void generate(struct biome *biome, struct chunk *chunk)
{
	for (int32_t x = 0; x < CHUNK_WIDTH; ++x)
	{
		for (int32_t z = 0; z < CHUNK_WIDTH; ++z)
		{
			int32_t top = chunk_get_top_block(chunk, x, z);
			struct block block;
			if (chunk_get_block(chunk, x, top, z, &block)
			 && block.type == 2)
			{
				if (nrand48(chunk->world->random) < (long)((2ULL << 31) / 50))
					generator_generate(biome->tree_generator, chunk, x, top + 1, z);
						chunk_set_block_if_replaceable(chunk, x, top + 1, z, 58);
#if 0
				switch (nrand48(chunk->world->random) % 8)
				{
					case 0:
						chunk_set_block_if_replaceable(chunk, x, top + 1, z, 37);
						break;
					case 1:
						chunk_set_block_if_replaceable(chunk, x, top + 1, z, 38);
						break;
					case 2:
						chunk_set_block_if_replaceable(chunk, x, top + 1, z, 39);
						break;
					case 3:
						chunk_set_block_if_replaceable(chunk, x, top + 1, z, 40);
						break;
					case 4:
						chunk_set_block_if_replaceable(chunk, x, top + 1, z, 6);
						break;
					case 5:
						chunk_set_block_if_replaceable(chunk, x, top + 1, z, 26);
						break;
					case 6:
						chunk_set_block_if_replaceable(chunk, x, top + 1, z, 27);
						break;
					case 7:
						chunk_set_block_if_replaceable(chunk, x, top + 1, z, 66);
						break;
				}
#endif
			}
		}
	}
	for (size_t i = 0; i < 10; ++i)
	{
		int32_t x = erand48(chunk->world->random) * CHUNK_WIDTH * 2;
		int32_t z = erand48(chunk->world->random) * CHUNK_WIDTH * 2;
		if (x < CHUNK_WIDTH && z < CHUNK_WIDTH)
		{
			int32_t top = chunk_get_top_block(chunk, x, z);
			int32_t y = erand48(chunk->world->random) * top;
			if (y > 2)
				y -= 2;
			else
				y = 0;
			generator_generate(biome->iron_ore_generator, chunk, x, y, z);
		}
	}
	for (size_t i = 0; i < 10; ++i)
	{
		int32_t x = erand48(chunk->world->random) * CHUNK_WIDTH * 2;
		int32_t z = erand48(chunk->world->random) * CHUNK_WIDTH * 2;
		if (x < CHUNK_WIDTH && z < CHUNK_WIDTH)
		{
			int32_t top = chunk_get_top_block(chunk, x, z);
			int32_t y = erand48(chunk->world->random) * top;
			if (y > 5)
				y -= 5;
			else
				y = 0;
			generator_generate(biome->coal_ore_generator, chunk, x, y, z);
		}
	}
	for (size_t i = 0; i < 5; ++i)
	{
		int32_t x = erand48(chunk->world->random) * CHUNK_WIDTH * 2;
		int32_t z = erand48(chunk->world->random) * CHUNK_WIDTH * 2;
		if (x < CHUNK_WIDTH && z < CHUNK_WIDTH)
		{
			int32_t top = chunk_get_top_block(chunk, x, z);
			int32_t y = erand48(chunk->world->random) * top;
			if (y > 2)
				y -= 2;
			else
				y = 0;
			generator_generate(biome->gold_ore_generator, chunk, x, y, z);
		}
	}
	for (size_t i = 0; i < 4; ++i)
	{
		int32_t x = erand48(chunk->world->random) * CHUNK_WIDTH * 2;
		int32_t z = erand48(chunk->world->random) * CHUNK_WIDTH * 2;
		if (x < CHUNK_WIDTH && z < CHUNK_WIDTH)
		{
			int32_t top = chunk_get_top_block(chunk, x, z);
			int32_t y = erand48(chunk->world->random) * top;
			if (y > 2)
				y -= 2;
			else
				y = 0;
			generator_generate(biome->diamond_ore_generator, chunk, x, y, z);
		}
	}
}

void biome_generate(struct biome *biome, struct chunk *chunk)
{
	//biome->op->generate(biome, chunk);
	generate(biome, chunk);
}
