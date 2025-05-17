#include "_getopt.h"

#include <getopt.h>

int
getopt_long(int argc,
            char * const argv[],
            const char *optstring,
            const struct option *longopts,
            int *longindex)
{
	return getopt_impl(argc,
	                   argv,
	                   optstring,
	                   longopts,
	                   longindex,
	                   LONGOPT_COMPAT);
}
