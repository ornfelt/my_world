#include "_pwd.h"

#include <pwd.h>

void
endpwent(void)
{
	if (!pwent_fp)
		return;
	fclose(pwent_fp);
	pwent_fp = NULL;
}
