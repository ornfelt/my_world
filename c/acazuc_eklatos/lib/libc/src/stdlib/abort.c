#include <stdlib.h>
#include <signal.h>

void abort(void)
{
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGABRT);
	sigprocmask(SIG_UNBLOCK, &set, NULL);
	raise(SIGABRT);
	signal(SIGABRT, SIG_DFL);
	raise(SIGABRT);
	exit(EXIT_FAILURE);/* XXX remove */
}
