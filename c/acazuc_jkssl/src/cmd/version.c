#include "cmd/cmd.h"

#include <stdlib.h>
#include <stdio.h>

int cmd_version(int argc, char **argv)
{
	(void)argc;
	(void)argv;
	printf("jkssl 0.0.1\n");
	return EXIT_SUCCESS;
}
