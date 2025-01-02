#include <signal.h>

sighandler_t signal(int signum, sighandler_t handler)
{
	sigset_t sigset;
	sigemptyset(&sigset);
	struct sigaction action;
	action.sa_handler = handler;
	action.sa_mask = sigset;
	action.sa_flags = SA_RESTORER;
	action.sa_restorer = sigreturn;
	struct sigaction old_action;
	if (sigaction(signum, &action, &old_action) == -1)
		return NULL;
	return old_action.sa_handler;
}
