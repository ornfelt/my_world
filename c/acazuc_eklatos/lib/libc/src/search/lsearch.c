#include <search.h>
#include <string.h>

void *lsearch(const void *key, void *base, size_t *nmemb, size_t size,
              int (*compare)(const void *, const void *))
{
	void *ret = lfind(key, base, nmemb, size, compare);
	if (ret)
		return ret;
	uint8_t *b = base;
	memmove(&b[*nmemb * size], key, size);
	(*nmemb)++;
	return &b[*nmemb * size];
}
