#ifndef BIOMES_H
#define BIOMES_H

#include <stdint.h>

struct biome;

struct biomes
{
	struct biome *biomes[256];
};

struct biomes *biomes_new(void);
void biomes_delete(struct biomes *biomes);
uint8_t biomes_get_biome(float temp, float rain);

#endif
