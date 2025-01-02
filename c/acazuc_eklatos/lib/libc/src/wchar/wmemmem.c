#include <wchar.h>

wchar_t *wmemmem(const wchar_t *haystack, size_t haystacklen,
                 const wchar_t *needle, size_t needlelen)
{
	if (needlelen > haystacklen)
		return NULL;
	haystacklen -= needlelen;
	for (size_t i = 0; i <= haystacklen; ++i)
	{
		if (!wmemcmp(&haystack[i], needle, needlelen))
			return (wchar_t*)&haystack[i];
	}
	return NULL;
}
