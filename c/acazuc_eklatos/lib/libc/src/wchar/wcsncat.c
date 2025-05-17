#include <wchar.h>

wchar_t *
wcsncat(wchar_t *d, const wchar_t *s, size_t n)
{
	wchar_t *org = d;
	size_t i = 0;

	while (*d)
		d++;
	while (s[i] && n)
	{
		d[i] = s[i];
		i++;
		n--;
	}
	d[i] = 0;
	return org;
}
