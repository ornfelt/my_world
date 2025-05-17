#include <stdlib.h>
#include <stdio.h>

int
main(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	fputs("\033[2J", stdout);
	return EXIT_SUCCESS;
}
