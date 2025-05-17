#include <wchar.h>

wchar_t *
wcpncpy(wchar_t *d, const wchar_t *s, size_t n)
{
	size_t i = 0;

	while (n)
	{
		wchar_t v = s[i];
		d[i] = v;
		if (!v)
			break;
		i++;
		n--;
	}
	if (n)
		wmemset(&d[i], 0, n);
	return &d[i];
}
