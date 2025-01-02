#include <stdio.h>
#include <wchar.h>

wint_t getwchar_unlocked(void)
{
	return getwc_unlocked(stdin);
}
