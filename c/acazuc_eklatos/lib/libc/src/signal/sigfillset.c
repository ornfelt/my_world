#include <signal.h>
#include <string.h>

int sigfillset(sigset_t *set)
{
	memset(set->set, 0xFF, sizeof(set->set));
	return 0;
}
