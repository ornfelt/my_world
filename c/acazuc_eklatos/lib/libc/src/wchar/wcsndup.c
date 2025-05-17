#include <stdlib.h>
#include <wchar.h>

wchar_t *
wcsndup(const wchar_t *s, size_t n)
{
	size_t len;
	wchar_t *dup;

	len = wcsnlen(s, n);
	dup = malloc((len + 1) * sizeof(*dup));
	if (!dup)
		return NULL;
	wmemcpy(dup, s, len + 1);
	return dup;
}
