#include <wchar.h>
#include <stdio.h>

wint_t putwchar_unlocked(wchar_t wc)
{
	return putwc_unlocked(wc, stdout);
}
