#include <wchar.h>

wchar_t *
wcsncpy(wchar_t *d, const wchar_t *s, size_t n)
{
	wchar_t *org = d;
	size_t i = 0;

	while (n && s[i])
	{
		d[i] = s[i];
		i++;
		n--;
	}
	if (n)
		d[i] = 0;
	return org;
}
