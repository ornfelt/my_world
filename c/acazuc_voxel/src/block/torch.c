#include "block/block.h"

#include "log.h"

#include <stdlib.h>

struct block_torch *block_torch_new(struct blocks_def *blocks, uint16_t id, const char *name)
{
	struct block_torch *torch = malloc(sizeof(*torch));
	if (!torch)
	{
		LOG_ERROR("allocation failed");
		return NULL;
	}
	block_def_init(blocks, &torch->block, id, name);
	return torch;
}
