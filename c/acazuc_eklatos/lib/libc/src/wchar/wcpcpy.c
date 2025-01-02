#include <wchar.h>

wchar_t *wcpcpy(wchar_t *d, const wchar_t *s)
{
	size_t i = 0;
	while (1)
	{
		wchar_t v = s[i];
		d[i] = v;
		if (!v)
			break;
		i++;
	}
	return &d[i];
}
