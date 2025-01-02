#include <stdio.h>
#include <wchar.h>

wint_t putwc(wchar_t wc, FILE *fp)
{
	return fputwc(wc, fp);
}
