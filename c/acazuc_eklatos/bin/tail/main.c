#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>

struct env
{
	const char *progname;
	long count;
	int print_lines;
};

static int tail_fp_lines(struct env *env, FILE *fp)
{
	if (env->count > 0)
	{
		char *line = NULL;
		size_t buf_size = 0;
		for (long i = 0; i < env->count; ++i)
		{
			if (getline(&line, &buf_size, fp) == -1)
			{
				if (feof(fp))
					return 0;
				fprintf(stderr, "%s: read: %s\n", env->progname, strerror(errno));
				return 1;
			}
		}
		while (getline(&line, &buf_size, fp) >= 0)
			fputs(line, stdout);
		if (ferror(fp))
		{
			fprintf(stderr, "%s: read: %s\n", env->progname, strerror(errno));
			return 1;
		}
		return 0;
	}
	/* XXX */
	return 0;
}

static int tail_fp_bytes(struct env *env, FILE *fp)
{
	if (env->count < 0)
	{
		if (!fseek(fp, env->count, SEEK_END))
		{
			char buf[4096];
			size_t rd;
			while ((rd = fread(buf, 1, sizeof(buf), fp)) > 0)
				fwrite(buf, 1, rd, stdout);
			if (ferror(fp))
			{
				fprintf(stderr, "%s: read:: %s\n", env->progname, strerror(errno));
				return 1;
			}
			return 0;
		}
		/* XXX do read in buffer */
		return 1;
	}
	if (!fseek(fp, env->count, SEEK_SET))
	{
		char buf[4096];
		size_t rd;
		while ((rd = fread(buf, 1, sizeof(buf), fp)) > 0)
			fwrite(buf, 1, rd, stdout);
		if (ferror(fp))
		{
			fprintf(stderr, "%s: read:: %s\n", env->progname, strerror(errno));
			return 1;
		}
		return 0;
	}
	/* XXX do read in buffer */
	return 0;
}

static int tail_fp(struct env *env, FILE *fp)
{
	if (env->print_lines)
		return tail_fp_lines(env, fp);
	return tail_fp_bytes(env, fp);
}

static int tail_file(struct env *env, const char *file)
{
	FILE *fp = fopen(file, "r");
	if (!fp)
	{
		fprintf(stderr, "%s: open: %s\n", env->progname, strerror(errno));
		return 1;
	}
	int ret = tail_fp(env, fp);
	fclose(fp);
	return ret;
}

static int parse_count(const char *progname, const char *str, long *count)
{
	errno = 0;
	char *endptr;
	*count = strtol(str, &endptr, 10);
	if (errno)
	{
		fprintf(stderr, "%s: strtoul: %s", progname, strerror(errno));
		return 1;
	}
	if (*endptr)
	{
		fprintf(stderr, "%s: invalid number\n", progname);
		return 1;
	}
	while (isspace(*str))
		str++;
	if (isdigit(*str))
		*count = -*count;
	return 0;
}

static void usage(const char *progname)
{
	printf("%s [-c num] [-n num] FILES\n", progname);
	printf("-c num: print the last num bytes\n");
	printf("-n num: print the last num lines\n");
}

int main(int argc, char **argv)
{
	struct env env;
	int c;

	memset(&env, 0, sizeof(env));
	env.progname = argv[0];
	env.count = 10;
	env.print_lines = 1;
	while ((c = getopt(argc, argv, "c:n:")) != -1)
	{
		switch (c)
		{
			case 'c':
				env.print_lines = 0;
				if (parse_count(argv[0], optarg, &env.count))
					return EXIT_FAILURE;
				break;
			case 'n':
				env.print_lines = 1;
				if (parse_count(argv[0], optarg, &env.count))
					return EXIT_FAILURE;
				break;
			default:
				usage(argv[0]);
				return EXIT_FAILURE;
		}
	}
	if (optind == argc)
	{
		if (tail_fp(&env, stdin))
			return EXIT_FAILURE;
		return EXIT_SUCCESS;
	}
	int multi = optind + 1 < argc;
	for (int i = optind; i < argc; ++i)
	{
		if (multi)
			printf("==> %s <==\n", argv[i]);
		if (tail_file(&env, argv[i]))
			return EXIT_FAILURE;
		if (multi && optind + 1 != argc)
			printf("\n");
	}
	return EXIT_SUCCESS;
}
