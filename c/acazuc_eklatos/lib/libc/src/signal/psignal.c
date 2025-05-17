#include <signal.h>
#include <string.h>
#include <stdio.h>

void
psignal(int signum, const char *str)
{
	if (str && *str)
		fprintf(stderr, "%s: %s\n", str, strsignal(signum));
	else
		fprintf(stderr, "%s\n", strsignal(signum));
}
