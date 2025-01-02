#include "_pwd.h"

#include <pwd.h>

struct passwd *getpwent(void)
{
	if (!pwent_fp)
	{
		pwent_fp = fopen("/etc/passwd", "rb");
		if (!pwent_fp)
			return NULL;
	}
	return fgetpwent(pwent_fp);
}
