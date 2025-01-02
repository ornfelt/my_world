#include "_alloc.h"

#include <stdlib.h>

void *realloc(void *ptr, size_t size)
{
	if (!ptr)
		return _malloc(size);
	if (!size)
	{
		free(ptr);
		return NULL;
	}
	return _realloc(ptr, size);
}
