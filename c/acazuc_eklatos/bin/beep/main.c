#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

static void
usage(const char *progname)
{
	printf("%s [-f frequency] [-l length] [-r repeat] [-d delay] [-h]\n", progname);
	printf("-f frequency: set the frequency in Hz (default 440Hz)\n");
	printf("-l length   : set the length of sound in ms (default 100ms)\n");
	printf("-r repeat   : set the number of times sound is played (default 1)\n");
	printf("-d delay    : set the delay between two sounds in ms (default 100ms)\n");
}

int
main(int argc, char **argv)
{
	unsigned freq = 440;
	unsigned len = 200;
	unsigned repeat = 1;
	unsigned delay = 100;
	int fd;
	int c;

	while ((c = getopt(argc, argv, "f:l:r:d:h")) != -1)
	{
		switch (c)
		{
			case 'f':
			{
				char *endptr;
				errno = 0;
				freq = strtoul(optarg, &endptr, 10);
				if (errno)
				{
					fprintf(stderr,
					        "%s: invalid frequency\n",
					        argv[0]);
					return EXIT_FAILURE;
				}
				if (*endptr)
				{
					fprintf(stderr,
					        "%s: invalid frequency\n",
					        argv[0]);
					return EXIT_FAILURE;
				}
				break;
			}
			case 'l':
			{
				char *endptr;
				errno = 0;
				len = strtoul(optarg, &endptr, 10);
				if (errno)
				{
					fprintf(stderr,
					        "%s: invalid length\n",
					        argv[0]);
					return EXIT_FAILURE;
				}
				if (*endptr)
				{
					fprintf(stderr,
					        "%s: invalid length\n",
					        argv[0]);
					return EXIT_FAILURE;
				}
				break;
			}
			case 'r':
			{
				char *endptr;
				errno = 0;
				repeat = strtoul(optarg, &endptr, 10);
				if (errno)
				{
					fprintf(stderr,
					        "%s: invalid repeat\n",
					        argv[0]);
					return EXIT_FAILURE;
				}
				if (*endptr)
				{
					fprintf(stderr,
					        "%s: invalid repeat\n",
					        argv[0]);
					return EXIT_FAILURE;
				}
				break;
			}
			case 'd':
			{
				char *endptr;
				errno = 0;
				delay = strtoul(optarg, &endptr, 10);
				if (errno)
				{
					fprintf(stderr,
					        "%s: invalid delay\n",
					        argv[0]);
					return EXIT_FAILURE;
				}
				if (*endptr)
				{
					fprintf(stderr,
					        "%s: invalid delay\n",
					        argv[0]);
					return EXIT_FAILURE;
				}
				break;
			}
			case 'h':
				usage(argv[0]);
				return EXIT_SUCCESS;
			default:
				usage(argv[0]);
				return EXIT_FAILURE;
		}
	}
	fd = open("/dev/pcspkr", O_WRONLY);
	if (fd == -1)
	{
		fprintf(stderr, "%s: open: %s\n", argv[0], strerror(errno));
		return EXIT_FAILURE;
	}
	for (unsigned i = 0; i < repeat; ++i)
	{
		uint16_t n = freq;
		uint16_t z = 0;
		write(fd, &n, 2);
		usleep(len * 1000);
		write(fd, &z, 2);
		if (i != repeat - 1)
			usleep(delay * 1000);
	}
	close(fd);
	return EXIT_SUCCESS;
}
