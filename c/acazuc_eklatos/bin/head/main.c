#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

struct env
{
	const char *progname;
	int print_lines;
	unsigned long count;
};

static int head_fp_lines(struct env *env, FILE *fp)
{
	char *buf = NULL;
	size_t n = 0;
	for (unsigned long i = 0; i < env->count; ++i)
	{
		if (getline(&buf, &n, fp) < 0)
		{
			fprintf(stderr, "%s: read: %s\n", env->progname, strerror(errno));
			free(buf);
			return 1;
		}
		fputs(buf, stdout);
	}
	free(buf);
	return 0;
}

static int head_fp_bytes(struct env *env, FILE *fp)
{
	for (unsigned long i = 0; i < env->count;)
	{
		uint8_t buf[4096];
		size_t n = sizeof(buf);
		if (n > env->count - i)
			n = env->count - i;
		size_t rd = fread(buf, 1, n, fp);
		if (ferror(fp))
		{
			fprintf(stderr, "%s: read: %s", env->progname, strerror(errno));
			return 1;
		}
		fwrite(buf, 1, rd, stdout);
		if (feof(fp))
			return 0;
		i += rd;
	}
	return 0;
}

static int head_fp(struct env *env, FILE *fp)
{
	if (env->print_lines)
		return head_fp_lines(env, fp);
	return head_fp_bytes(env, fp);
}

static int head_file(struct env *env, const char *file)
{
	FILE *fp = fopen(file, "r");
	if (!fp)
	{
		fprintf(stderr, "%s: open: %s\n", env->progname, strerror(errno));
		return 1;
	}
	int ret = head_fp(env, fp);
	fclose(fp);
	return ret;
}

static int parse_count(const char *progname, const char *str, unsigned long *count)
{
	errno = 0;
	char *endptr;
	*count = strtoul(str, &endptr, 10);
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
	return 0;
}

static void usage(const char *progname)
{
	printf("%s [-c num] [-n num] FILES\n", progname);
	printf("-c num: print the first num bytes\n");
	printf("-n num: print the first num lines\n");
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
		if (head_fp(&env, stdin))
			return EXIT_FAILURE;
		return EXIT_SUCCESS;
	}
	int multi = optind + 1 < argc;
	for (int i = optind; i < argc; ++i)
	{
		if (multi)
			printf("==> %s <==\n", argv[i]);
		if (head_file(&env, argv[i]))
			return EXIT_FAILURE;
		if (multi && optind + 1 != argc)
			printf("\n");
	}
	return EXIT_SUCCESS;
}
