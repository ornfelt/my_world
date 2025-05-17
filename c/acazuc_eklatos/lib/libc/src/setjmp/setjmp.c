#include <setjmp.h>

int
setjmp(jmp_buf env)
{
	if (__builtin_setjmp(&env.data[0]))
		return env.val;
	return 0;
}
