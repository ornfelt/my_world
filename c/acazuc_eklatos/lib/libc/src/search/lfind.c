#include <search.h>

void *lfind(const void *key, const void *base, size_t *nmemb, size_t size,
            int (*compare)(const void *, const void *))
{
	const uint8_t *b = base;
	for (size_t i = 0; i < *nmemb; ++i)
	{
		if (!compare(key, &b[size * i]))
			return (void*)&b[size * i];
	}
	return NULL;
}
