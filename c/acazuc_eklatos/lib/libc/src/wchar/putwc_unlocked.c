#include <stdio.h>
#include <wchar.h>

wint_t
putwc_unlocked(wchar_t wc, FILE *fp)
{
	return fputwc_unlocked(wc, fp);
}
