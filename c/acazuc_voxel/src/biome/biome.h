#ifndef BIOME_H
#define BIOME_H

#include <stdint.h>

struct generator;
struct biome;
struct chunk;

struct biome_op
{
	void (*generate)(struct biome biome, struct chunk *chunk);
};

struct biome
{
	const struct biome_op *op;
	struct generator *diamond_ore_generator;
	struct generator *coal_ore_generator;
	struct generator *iron_ore_generator;
	struct generator *gold_ore_generator;
	struct generator *tree_generator;
	const char *name;
	uint8_t id;
};

struct biome *biome_new(uint8_t id, const char *name);
void biome_delete(struct biome *biome);
void biome_generate(struct biome *biome, struct chunk *chunk);

#endif
