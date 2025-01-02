#include <wchar.h>

int mbsinit(mbstate_t *ps)
{
	if (!ps)
		return 1;
	ps->dummy = 0;
	return 0;
}
