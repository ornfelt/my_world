#include <stdlib.h>
#include <glob.h>

void
globfree(glob_t *globp)
{
	if (!globp || !globp->gl_pathv)
		return;
	for (size_t i = 0; i < globp->gl_pathc; ++i)
		free(globp->gl_pathv[i]);
	free(globp->gl_pathv);
	globp->gl_pathv = NULL;
	globp->gl_pathc = 0;
}
