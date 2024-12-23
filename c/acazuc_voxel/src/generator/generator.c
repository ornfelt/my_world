#include "generator/generator.h"

#include <stdlib.h>

void generator_delete(struct generator *generator)
{
	if (!generator)
		return;
	free(generator);
}

void generator_generate(struct generator *generator, struct chunk *chunk, uint8_t x, uint8_t y, uint8_t z)
{
	generator->op->generate(generator, chunk, x, y, z);
}
