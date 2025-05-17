#include <wctype.h>
#include <wchar.h>

int
wcsncasecmp(const wchar_t *s1, const wchar_t *s2, size_t n)
{
	size_t i = 0;

	while (i < n && s1[i] && s2[i])
	{
		wchar_t c1 = towlower(s1[i]);
		wchar_t c2 = towlower(s2[i]);
		if (c1 != c2)
			return c1 - c2;
		i++;
	}
	if (i == n)
		return 0;
	return s1[i] - s2[i];
}
