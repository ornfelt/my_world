#include <signal.h>

int
sigwait(const sigset_t *set, int *sig)
{
	(void)set;
	(void)sig;
	/* XXX */
	return -1;
}
