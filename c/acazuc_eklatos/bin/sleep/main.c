#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv)
{
	int total = 0;
	if (argc < 2)
	{
		fprintf(stderr, "%s: missing operand\n", argv[0]);
		return EXIT_FAILURE;
	}
	for (int i = 1; i < argc; ++i)
		total += atoi(argv[i]);
	if (total < 0)
	{
		fprintf(stderr, "%s: invalid interval\n", argv[0]);
		return EXIT_FAILURE;
	}
	sleep(total);
	return EXIT_SUCCESS;
}
