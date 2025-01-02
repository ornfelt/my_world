#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

int main(int argc, char **argv)
{
	if (argc > 2)
	{
		fprintf(stderr, "%s: extra operand '%s'\n", argv[0], argv[2]);
		return EXIT_FAILURE;
	}
	if (unlink(argv[1]) == -1)
	{
		fprintf(stderr, "%s: unlink: %s\n", argv[0], strerror(errno));
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
