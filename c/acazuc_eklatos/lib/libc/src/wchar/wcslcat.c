#include <wchar.h>

size_t wcslcat(wchar_t *d, const wchar_t *s, size_t n)
{
	if (!n)
		return wcslen(d) + wcslen(s);
	size_t i = wcslen(d);
	if (i >= n)
		return i + wcslen(s);
	size_t j = 0;
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
