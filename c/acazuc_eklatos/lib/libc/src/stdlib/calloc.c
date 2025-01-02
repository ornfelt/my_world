#include "_alloc.h"

#include <stdlib.h>
#include <string.h>

void *calloc(size_t nmemb, size_t size)
{
	size_t ret;
	if (__builtin_mul_overflow(nmemb, size, &ret))
		return NULL;
	if (!ret)
		return NULL;
	void *ptr = _malloc(ret);
	if (!ptr)
		return NULL;
	memset(ptr, 0, ret);
	return ptr;
}
