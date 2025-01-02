#include <wchar.h>

wchar_t *wcscpy(wchar_t *d, const wchar_t *s)
{
	wchar_t *org = d;
	size_t i = 0;
	while (s[i])
	{
		d[i] = s[i];
		i++;
	}
	d[i] = 0;
	return org;
}
