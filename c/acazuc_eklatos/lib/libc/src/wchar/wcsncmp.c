#include <wchar.h>

int wcsncmp(const wchar_t *s1, const wchar_t *s2, size_t n)
{
	while (n && *s1 && *s2)
	{
		if (*s1 != *s2)
			return *s1 - *s2;
		s1++;
		s2++;
		n--;
	}
	if (!n)
		return 0;
	return *s1 - *s2;
}
