#include "common.h"

struct wow_memory wow_memory =
{
	.malloc = NULL,
	.realloc = NULL,
	.free = NULL,
};
