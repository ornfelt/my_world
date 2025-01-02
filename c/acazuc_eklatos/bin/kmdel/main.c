#include <sys/stat.h>

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

static void usage(const char *progname)
{
	printf("%s modulename\n", progname);
}

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		usage(argv[0]);
		return EXIT_FAILURE;
	}
	int ret = kmunload(argv[1], 0);
	if (ret == -1)
	{
		fprintf(stderr, "%s: kmunload: %s\n", argv[0], strerror(errno));
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
