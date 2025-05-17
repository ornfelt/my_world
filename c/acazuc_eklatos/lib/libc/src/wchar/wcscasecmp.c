#include <wctype.h>
#include <wchar.h>

int
wcscasecmp(const wchar_t *s1, const wchar_t *s2)
{
	size_t i = 0;
	while (s1[i] && s2[i])
	{
		wchar_t c1 = towlower(s1[i]);
		wchar_t c2 = towlower(s2[i]);
		if (c1 != c2)
			return c1 - c2;
		i++;
	}
	return s1[i] - s2[i];
}
