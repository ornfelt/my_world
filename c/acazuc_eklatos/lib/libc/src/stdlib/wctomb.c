#include <wchar.h>

int
wctomb(char *s, wchar_t wc)
{
	return wcrtomb(s, wc, NULL);
}
