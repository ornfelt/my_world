#include <signal.h>
#include <unistd.h>

int
raise(int sig)
{
	/* XXX make a tgkill syscall to let kill only kill process ? */
	return kill(gettid(), sig);
}
