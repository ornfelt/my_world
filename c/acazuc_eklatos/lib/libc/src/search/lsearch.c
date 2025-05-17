#include <search.h>
#include <string.h>

void *
lsearch(const void *key,
        void *base,
        size_t *nmemb,
        size_t size,
        int (*compare)(const void *, const void *))
{
	uint8_t *b;
	void *ret;

	ret = lfind(key, base, nmemb, size, compare);
	if (ret)
		return ret;
	b = base;
	memmove(&b[*nmemb * size], key, size);
	(*nmemb)++;
	return &b[*nmemb * size];
}
