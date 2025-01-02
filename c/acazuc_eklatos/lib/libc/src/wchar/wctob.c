#include <wchar.h>
#include <stdio.h>

int wctob(wint_t c)
{
	if (c <= 0x7F)
		return c;
	return EOF;
}
