#include <stdlib.h>

void *bsearch(const void *key, const void *base, size_t nmemb, size_t size,
              int (*compar)(const void *, const void *))
{
	if (!nmemb)
		return NULL;
	size_t range = nmemb;
	size_t offset = 0;
	while (1)
	{
		size_t half = range / 2;
		void *cmp = &((uint8_t*)base)[size * offset + half];
		int ret = compar(key, cmp);
		if (!ret)
			return cmp;
		if (!half)
			break;
		if (ret < 0)
		{
			range = half;
		}
		else
		{
			offset += half;
			range -= half;
		}
	}
	return NULL;
}
