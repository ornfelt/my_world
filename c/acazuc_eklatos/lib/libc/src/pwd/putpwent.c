#include "_pwd.h"

#include <pwd.h>

int putpwent(const struct passwd *p, FILE *fp)
{
	(void)p;
	(void)fp;
	/* XXX */
	return -1;
}
