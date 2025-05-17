#include <wchar.h>

wchar_t *
wcscat(wchar_t *d, const wchar_t *s)
{
	wchar_t *ret = d;
	size_t i = 0;

	while (*d)
		d++;
	while (1)
	{
		d[i] = s[i];
		if (!s[i])
			break;
		i++;
	}
	return ret;
}
