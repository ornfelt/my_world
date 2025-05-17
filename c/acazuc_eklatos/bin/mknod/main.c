#include <sys/stat.h>

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

struct env
{
	mode_t mode;
};

static void
usage(const char *progname)
{
	printf("%s [-m mode] name type major minor\n", progname);
	printf("-m: the mode of the created node\n");
	printf("type: 'b' for block device, 'c' for character device\n");
}

static int
parse_mode(const char *progname, const char *str, mode_t *mode)
{
	*mode = 0;
	for (size_t i = 0; str[i]; ++i)
	{
		if (str[i] < '0' || str[i] > '7')
		{
			fprintf(stderr, "%s: invalid mode\n", progname);
			return 1;
		}
		*mode = *mode * 8 + str[i] - '0';
		if (*mode > 07777)
		{
			fprintf(stderr, "%s: mode out of bounds\n", progname);
			return 1;
		}
	}
	return 0;
}

int
main(int argc, char **argv)
{
	struct env env;
	unsigned long min;
	unsigned long maj;
	char *endptr;
	int c;

	memset(&env, 0, sizeof(env));
	env.mode = 0777;
	while ((c = getopt(argc, argv, "m:")) != -1)
	{
		switch (c)
		{
			case 'm':
				if (parse_mode(argv[0], optarg, &env.mode))
					return EXIT_FAILURE;
				break;
			default:
				usage(argv[0]);
				return EXIT_FAILURE;
		}
	}
	if (argc - optind < 4)
	{
		fprintf(stderr, "%s: missing operand\n", argv[0]);
		return EXIT_FAILURE;
	}
	if (!strcmp(argv[optind + 1], "b"))
	{
		env.mode |= S_IFBLK;
	}
	else if (!strcmp(argv[optind + 1], "c"))
	{
		env.mode |= S_IFCHR;
	}
	else
	{
		fprintf(stderr, "%s: invalid node type\n", argv[0]);
		return EXIT_FAILURE;
	}
	errno = 0;
	maj = strtoul(argv[optind + 2], &endptr, 0);
	if (errno)
	{
		fprintf(stderr, "%s: strtoul: %s\n", argv[0], strerror(errno));
		return EXIT_FAILURE;
	}
	if (endptr == argv[optind + 2] || *endptr || maj > 0xFFFF)
	{
		fprintf(stderr, "%s: invalid operand\n", argv[0]);
		return EXIT_FAILURE;
	}
	errno = 0;
	min = strtoul(argv[optind + 3], &endptr, 0);
	if (errno)
	{
		fprintf(stderr, "%s: strtoul: %s\n", argv[0], strerror(errno));
		return EXIT_FAILURE;
	}
	if (endptr == argv[optind + 3] || *endptr || min > 0xFFFF)
	{
		fprintf(stderr, "%s: invalid operand\n", argv[0]);
		return EXIT_FAILURE;
	}
	if (mknod(argv[optind], env.mode, makedev(maj, min)) == -1)
	{
		fprintf(stderr, "%s: mknod(%s): %s\n",
		        argv[0],
		        argv[optind],
		        strerror(errno));
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
