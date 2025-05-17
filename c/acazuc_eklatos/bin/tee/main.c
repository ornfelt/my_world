#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdio.h>
#include <errno.h>

#define OPT_a (1 << 0)

struct env
{
	const char *progname;
	int opt;
	TAILQ_HEAD(, output_file) files;
};

struct output_file
{
	FILE *fp;
	TAILQ_ENTRY(output_file) chain;
};

static void
usage(const char *progname)
{
	printf("%s [-h] [-a] FILES\n", progname);
	printf("-h: show this help\n");
	printf("-a: open the given files in append mode\n");
}

int
main(int argc, char **argv)
{
	struct env env;
	int c;

	memset(&env, 0, sizeof(env));
	env.progname = argv[0];
	TAILQ_INIT(&env.files);
	while ((c = getopt(argc, argv, "ah")) != -1)
	{
		switch (c)
		{
			case 'a':
				env.opt |= OPT_a;
				break;
			case 'h':
				usage(argv[0]);
				return EXIT_SUCCESS;
			default:
				usage(argv[0]);
				return EXIT_FAILURE;
		}
	}
	for (int i = optind; i < argc; ++i)
	{
		struct output_file *f = calloc(1, sizeof(*f));
		if (!f)
		{
			fprintf(stderr, "%s: calloc %s\n", argv[0],
			        strerror(errno));
			return EXIT_FAILURE;
		}
		f->fp = fopen(argv[i], (env.opt & OPT_a) ? "ab" : "wb");
		if (!f->fp)
		{
			fprintf(stderr, "%s: fopen(%s): %s\n", argv[0], argv[i],
			        strerror(errno));
			free(f);
			continue;
		}
		TAILQ_INSERT_TAIL(&env.files, f, chain);
	}
	while (1)
	{
		static uint8_t buf[1024 * 1024];
		ssize_t rd = read(0, buf, sizeof(buf));
		if (rd == -1)
		{
			if (errno == EINTR)
				continue;
			fprintf(stderr, "%s: fread: %s\n", argv[0], strerror(errno));
			return EXIT_FAILURE;
		}
		write(1, buf, rd);
		struct output_file *f;
		TAILQ_FOREACH(f, &env.files, chain)
			fwrite(buf, 1, rd, f->fp);
	}
	return EXIT_SUCCESS;
}
