#include <wchar.h>

size_t
wcslcat(wchar_t *d, const wchar_t *s, size_t n)
{
	size_t j = 0;
	size_t i;

	if (!n)
		return wcslen(d) + wcslen(s);
	i = wcslen(d);
	if (i >= n)
		return i + wcslen(s);
	while (i < n - 1)
	{
		wchar_t v = s[j];
		d[i] = v;
		if (!v)
			return i;
		i++;
		j++;
	}
	d[i] = 0;
	return i + wcslen(&s[j]);
}
