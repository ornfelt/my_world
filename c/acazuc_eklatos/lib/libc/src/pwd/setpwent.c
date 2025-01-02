#include "_pwd.h"

#include <pwd.h>

void setpwent(void)
{
	if (!pwent_fp)
		return;
	rewind(pwent_fp);
}
