#include "_alloc.h"

#include <stdlib.h>

void free(void *ptr)
{
	if (!ptr)
		return;
	_free(ptr);
}
