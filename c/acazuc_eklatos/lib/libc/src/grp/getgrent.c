#include "_grp.h"

#include <grp.h>

struct group *
getgrent(void)
{
	if (!grent_fp)
	{
		grent_fp = fopen("/etc/group", "rb");
		if (!grent_fp)
			return NULL;
	}
	return fgetgrent(grent_fp);
}
