#include <sys/mount.h>

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

static void usage(const char *progname)
{
	printf("%s [-t type] [-o opt] [device] dir\n", progname);
	printf("-t type: filesystem type\n");
	printf("-o opt: mount options\n");
}

int main(int argc, char **argv)
{
	const char *type = NULL;
	const char *opt = NULL;
	const char *dev;
	int c;

	while ((c = getopt(argc, argv, "t:o:")) != -1)
	{
		switch (c)
		{
			case 't':
				type = optarg;
				break;
			case 'o':
				opt = optarg;
				break;
			default:
				usage(argv[0]);
				return EXIT_FAILURE;
		}
	}
	if (!type)
	{
		fprintf(stderr, "%s: type required\n", argv[0]);
		return EXIT_FAILURE;
	}
	switch (argc - optind)
	{
		case 0:
			fprintf(stderr, "%s: missing operand\n", argv[0]);
			return EXIT_FAILURE;
		case 1:
			dev = NULL;
			break;
		case 2:
			dev = argv[optind++];
			break;
		default:
			fprintf(stderr, "%s: extra operand\n", argv[0]);
			return EXIT_FAILURE;
	}
	if (mount(dev, argv[optind], type, 0, opt) == -1)
	{
		fprintf(stderr, "%s: %s\n", argv[0], strerror(errno));
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
