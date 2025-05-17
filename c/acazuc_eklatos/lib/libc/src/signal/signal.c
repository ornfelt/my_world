#include <signal.h>

sighandler_t
signal(int signum, sighandler_t handler)
{
	struct sigaction old_action;
	struct sigaction action;
	sigset_t sigset;

	sigemptyset(&sigset);
	action.sa_handler = handler;
	action.sa_mask = sigset;
	action.sa_flags = SA_RESTORER;
	action.sa_restorer = sigreturn;
	if (sigaction(signum, &action, &old_action) == -1)
		return NULL;
	return old_action.sa_handler;
}
