#include "cmd/cmd.h"

#include <stdlib.h>

int cmd_help(int argc, char **argv)
{
	(void)argc;
	(void)argv;
	print_usage();
	return EXIT_SUCCESS;
}
