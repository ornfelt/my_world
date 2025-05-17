#include "_getopt.h"

#include <unistd.h>

int
getopt(int argc, char * const argv[], const char *optstring)
{
	return getopt_impl(argc, argv, optstring, NULL, NULL, LONGOPT_NONE);
}
