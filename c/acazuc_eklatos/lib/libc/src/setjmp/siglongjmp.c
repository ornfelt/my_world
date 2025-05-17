#include <setjmp.h>

void
siglongjmp(sigjmp_buf env, int val)
{
	/* XXX */
	longjmp(env.buf, val);
}
