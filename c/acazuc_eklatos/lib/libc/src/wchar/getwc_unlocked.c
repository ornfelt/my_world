#include <wchar.h>

wint_t getwc_unlocked(FILE *fp)
{
	return fgetwc_unlocked(fp);
}
