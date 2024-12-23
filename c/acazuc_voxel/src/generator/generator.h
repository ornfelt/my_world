#ifndef GENERATOR_H
#define GENERATOR_H

#include <stdint.h>

struct generator;
struct chunk;

struct generator_op
{
	void (*generate)(struct generator *generator, struct chunk *chunk, uint8_t x, uint8_t y, uint8_t z);
};

struct generator
{
	const struct generator_op *op;
};

struct generator *generator_tree_new(void);
struct generator *generator_minable_new(uint8_t block, uint8_t size);
void generator_delete(struct generator *generator);
void generator_generate(struct generator *generator, struct chunk *chunk, uint8_t x, uint8_t y, uint8_t z);

#endif
