#include <wchar.h>

wint_t
getwc(FILE *fp)
{
	return fgetwc(fp);
}
