#include <string.h>

void *
memmem(const void *haystack,
       size_t haystacklen,
       const void *needle,
       size_t needlelen)
{
	if (needlelen > haystacklen)
		return NULL;
	haystacklen -= needlelen;
	for (size_t i = 0; i <= haystacklen; ++i)
	{
		if (!memcmp(&((unsigned char*)haystack)[i], needle, needlelen))
			return &((unsigned char*)haystack)[i];
	}
	return NULL;
}
