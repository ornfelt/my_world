#ifndef _GETOPT_H
#define _GETOPT_H

#include <getopt.h>

#define LONGOPT_NONE   0
#define LONGOPT_COMPAT 1
#define LONGOPT_ONLY   2

int getopt_impl(int argc,
                char * const argv[],
                const char *optstring,
                const struct option *longopts,
                int *longindex,
                int longopt_type);

#endif
