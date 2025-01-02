#include <stdio.h>
#include <wchar.h>

wint_t btowc(int c)
{
	if (c < 0 || c >= 0x80 || c == EOF)
		return WEOF;
	return c;
}
