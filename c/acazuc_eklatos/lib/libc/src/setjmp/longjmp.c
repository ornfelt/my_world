#include <setjmp.h>

void longjmp(jmp_buf env, int val)
{
	env.val = val;
	__builtin_longjmp(&env.data[0], 1);
}
