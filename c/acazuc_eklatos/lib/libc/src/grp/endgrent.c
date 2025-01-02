#include "_grp.h"

#include <grp.h>

void endgrent(void)
{
	if (!grent_fp)
		return;
	fclose(grent_fp);
	grent_fp = NULL;
}
