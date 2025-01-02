#include "_alloc.h"

#include <stdlib.h>

void *malloc(size_t size)
{
	if (!size)
		return NULL;
	return _malloc(size);
}
