#include <wchar.h>

wchar_t *
wcstok(wchar_t *s, const wchar_t *delim, wchar_t **saveptr)
{
	wchar_t *src;

	if (s)
	{
		src = s;
	}
	else
	{
		if (!*saveptr)
			return NULL;
		src = *saveptr + 1;
		if (!*src)
		{
			*saveptr = NULL;
			return NULL;
		}
	}
	while (wcschr(delim, *src))
		src++;
	wchar_t *tok = wcspbrk(src, delim);
	if (!tok)
	{
		*saveptr = NULL;
		return *src ? src : NULL;
	}
	*tok = 0;
	*saveptr = tok;
	return src;
}
