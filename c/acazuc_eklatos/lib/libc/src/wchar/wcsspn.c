#include <wchar.h>

size_t wcsspn(const wchar_t *s, const wchar_t *accept)
{
	const wchar_t *org = s;
	while (*s)
	{
		if (!wcschr(accept, *s))
			return s - org;
		s++;
	}
	return s - org;
}
