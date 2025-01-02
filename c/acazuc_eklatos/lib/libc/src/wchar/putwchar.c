#include <wchar.h>
#include <stdio.h>

wint_t putwchar(wchar_t wc)
{
	return putwc(wc, stdout);
}
