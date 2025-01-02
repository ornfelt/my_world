#include "../_syscall.h"

#include <signal.h>

#if !defined(__aarch64__) && !defined(__mips__)
__attribute__((naked))
#endif
void sigreturn(void)
{
	syscall0_raw(SYS_sigreturn);
}
