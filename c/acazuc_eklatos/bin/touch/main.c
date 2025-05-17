#include <sys/stat.h>

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>

static void
usage(const char *progname)
{
	printf("%s [-h] files\n", progname);
	printf("-h: show this help\n");
}

static int
touch_file(const char *progname, const char *file)
{
	int fd;

	fd = open(file, O_RDONLY | O_CREAT, 0666);
	if (fd == -1)
	{
		fprintf(stderr, "%s: open: %s\n", progname, strerror(errno));
		return 1;
	}
	if (futimens(fd, NULL) == -1)
	{
		fprintf(stderr, "%s: futimens: %s\n", progname, strerror(errno));
		close(fd);
		return 1;
	}
	close(fd);
	return 0;
}

int
main(int argc, char **argv)
{
	int c;

	while ((c = getopt(argc, argv, "h")) != -1)
	{
		switch (c)
		{
			default:
				usage(argv[0]);
				return EXIT_FAILURE;
		}
	}
	if (optind == argc)
	{
		fprintf(stderr, "%s: missing file\n", argv[0]);
		return EXIT_FAILURE;
	}
	for (int i = optind; i < argc; ++i)
	{
		if (touch_file(argv[0], argv[i]))
			return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
