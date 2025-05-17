#include <wchar.h>

size_t
wcslcpy(wchar_t *d, const wchar_t *s, size_t n)
{
	size_t i = 0;

	if (!n)
		return wcslen(s);
	while (i < n - 1)
	{
		wchar_t v = s[i];
		d[i] = v;
		if (!v)
			return i;
		i++;
	}
	d[i] = 0;
	return i + wcslen(&s[i]);
}
