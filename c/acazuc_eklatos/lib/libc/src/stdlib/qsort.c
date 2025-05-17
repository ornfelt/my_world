#include <stdlib.h>

void
qsort(void *base,
      size_t nmemb,
      size_t size,
      int (*cmp)(const void *, const void *))
{
	void *pivot;
	size_t left, right;

	if (nmemb < 2)
		return;
	pivot = &((uint8_t*)base)[size * ((nmemb - 1) / 2)];
	right = nmemb;
	left = (size_t)-1;
	while (1)
	{
		do
		{
			right--;
		} while (cmp(&((uint8_t*)base)[size * right], pivot) > 0);
		do
		{
			left++;
		} while (cmp(&((uint8_t*)base)[size * left], pivot) < 0);
		if (left >= right)
			break;
		uint8_t *src = &((uint8_t*)base)[size * left];
		uint8_t *dst = &((uint8_t*)base)[size * right];
		for (size_t i = 0; i < size; ++i)
		{
			uint8_t tmp = src[i];
			src[i] = dst[i];
			dst[i] = tmp;
		}
		if (pivot == src)
			pivot = dst;
		else if (pivot == dst)
			pivot = src;
	}
	qsort(base, right + 1, size, cmp);
	qsort(&((uint8_t*)base)[size * (right + 1)], nmemb - (right + 1), size,
	      cmp);
}
