#include <wchar.h>

size_t wcscspn(const wchar_t *s, const wchar_t *reject)
{
	const wchar_t *org = s;
	while (*s)
	{
		if (wcschr(reject, *s))
			return s - org;
		s++;
	}
	return s - org;
}
