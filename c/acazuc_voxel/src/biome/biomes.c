#include "biome/biomes.h"
#include "biome/biome.h"

#include "log.h"

#include <string.h>
#include <stdlib.h>
#include <math.h>

struct biomes *biomes_new(void)
{
	struct biomes *biomes = calloc(sizeof(*biomes), 1);
	if (!biomes)
	{
		LOG_ERROR("biomes allocation failed");
		return NULL;
	}

#define BIOME_INIT(id, name) \
	do \
	{ \
		biomes->biomes[id] = biome_new(id, name); \
		if (!biomes->biomes[id]) \
			goto err; \
	} while (0)

	BIOME_INIT(0, "ocean");
	BIOME_INIT(1, "plains");
	BIOME_INIT(2, "desert");
	BIOME_INIT(3, "extreme_hills");
	BIOME_INIT(4, "forest");
	BIOME_INIT(5, "taiga");
	BIOME_INIT(6, "swampland");
	BIOME_INIT(7, "river");
	BIOME_INIT(8, "hell");
	BIOME_INIT(9, "sky");
	BIOME_INIT(10, "frozen_ocean");
	BIOME_INIT(11, "frozen_river");
	BIOME_INIT(12, "ice_flats");
	BIOME_INIT(13, "ice_mountains");
	BIOME_INIT(14, "mushroom_island");
	return biomes;

#undef BIOME_INIT

err:
	biomes_delete(biomes);
	return NULL;
}

void biomes_delete(struct biomes *biomes)
{
	if (!biomes)
		return;
	for (uint8_t i = 0; i < 255; ++i)
		biome_delete(biomes->biomes[i]);
	free(biomes);
}

uint8_t biomes_get_biome(float temp, float rain)
{
	static uint8_t tab[] =
	{
		 1,  1,  3,  3,  6,  6,  9,  9,
		 1,  1,  3,  3,  6,  6,  9,  9,
		 3,  3,  4,  4,  7,  7,  9,  9,
		 3,  3,  4,  4,  7,  7, 10, 10,
		 6,  6,  7,  7,  5,  5, 10, 10,
		 6,  6,  7,  7,  5,  5, 10, 10,
		 9,  9,  9, 10, 10, 10,  8,  8,
		 9,  9,  9, 10, 10, 10,  8,  8,
	};
	if (temp < 0)
		temp = 0;
	else if (temp > 1)
		temp = 1;
	if (rain < 0)
		rain = 0;
	else if (rain > 1)
		rain = 1;
	return tab[(int32_t)(roundf(rain * 7) * 8 + roundf(temp * 7))];
}
