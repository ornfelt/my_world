#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <fetch.h>
#include <errno.h>

static void
usage(const char *progname)
{
	printf("%s [-h] [-o output] URL\n", progname);
	printf("-h       : show this help\n");
	printf("-o output: output to the given file\n");
}

int
main(int argc, char **argv)
{
	FILE *fp;
	FILE *out = stdout;
	int c;

	while ((c = getopt(argc, argv, "ho:")) != -1)
	{
		switch (c)
		{
			case 'h':
				usage(argv[0]);
				return EXIT_SUCCESS;
			case 'o':
				out = fopen(optarg, "w");
				if (!out)
				{
					fprintf(stderr, "%s: open(%s): %s\n", argv[0],
					        optarg, strerror(errno));
					return EXIT_FAILURE;
				}
				break;
			default:
				usage(argv[0]);
				return EXIT_FAILURE;
		}
	}
	if (argc - optind < 1)
	{
		fprintf(stderr, "%s: missing operand\n", argv[0]);
		return EXIT_FAILURE;
	}
	if (argc - optind > 1)
	{
		fprintf(stderr, "%s: extra operandn", argv[0]);
		return EXIT_FAILURE;
	}
	fp = fetchGetURL(argv[optind], NULL);
	if (!fp)
	{
		fprintf(stderr, "%s: fetch: %s\n", argv[0], strerror(errno));
		return EXIT_FAILURE;
	}
	while (!feof(fp))
	{
		uint8_t buf[4096];
		size_t rd;

		rd = fread(buf, 1, sizeof(buf), fp);
		if (ferror(fp))
		{
			fprintf(stderr, "%s: read: %s\n", argv[0], strerror(errno));
			return EXIT_FAILURE;
		}
		fwrite(buf, 1, rd, out);
	}
	return EXIT_FAILURE;
}
