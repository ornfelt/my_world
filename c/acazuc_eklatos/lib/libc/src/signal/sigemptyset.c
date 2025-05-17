#include <signal.h>
#include <string.h>

int
sigemptyset(sigset_t *set)
{
	memset(set->set, 0, sizeof(set->set));
	return 0;
}
