#include <setjmp.h>

int sigsetjmp(sigjmp_buf env, int savesigs)
{
	/* XXX signals */
	(void)savesigs;
	return setjmp(env.buf);
}
