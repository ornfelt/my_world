#include <wchar.h>

wchar_t *wmemmove(wchar_t *d, const wchar_t *s, size_t n)
{
	if (!n)
		return d;
	if (d == s)
		return d;
	if (d < s)
		return wmemcpy(d, s, n);
	while (n)
	{
		n--;
		d[n] = s[n];
	}
	return d;
}
