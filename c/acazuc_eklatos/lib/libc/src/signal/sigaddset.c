#include <signal.h>
#include <errno.h>

int
sigaddset(sigset_t *set, int signum)
{
	if (signum < 0 || signum > SIGPOLL)
	{
		errno = EINVAL;
		return -1;
	}
	set->set[signum / 8] |= (1 << (signum % 8));
	return 0;
}
