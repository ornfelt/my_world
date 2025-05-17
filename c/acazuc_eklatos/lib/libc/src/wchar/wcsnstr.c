#include <wchar.h>

wchar_t *
wcsnstr(const wchar_t *s, const wchar_t *needle, size_t n)
{
	if (!needle)
		return (wchar_t*)s;
	for (size_t i = 0; i < n && s[i]; ++i)
	{
		for (size_t j = 0; (s[i + j] == needle[j] || !needle[j]); ++j)
		{
			if (!needle[j])
				return (wchar_t*)s + i;
			if (i + j >= n)
				return NULL;
		}
	}
	return NULL;
}
