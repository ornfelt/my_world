#include "_grp.h"

#include <grp.h>

void setgrent(void)
{
	if (!grent_fp)
		return;
	rewind(grent_fp);
}
