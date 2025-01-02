#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>

#define OPT_f (1 << 0)
#define OPT_r (1 << 1)

struct env
{
	const char *progname;
	char **lines;
	size_t lines_nb;
	int opt;
};

static int cmp_str(const void *a, const void *b)
{
	return strcmp(*(char**)a, *(char**)b);
}

static int cmp_str_rev(const void *a, const void *b)
{
	return -strcmp(*(char**)a, *(char**)b);
}

static int cmp_str_case(const void *a, const void *b)
{
	return strcasecmp(*(char**)a, *(char**)b);
}

static int cmp_str_case_rev(const void *a, const void *b)
{
	return -strcasecmp(*(char**)a, *(char**)b);
}

static int read_fp(struct env *env, FILE *fp)
{
	char *line = NULL;
	size_t len = 0;
	while ((getline(&line, &len, fp)) != -1)
	{
		char **lines = realloc(env->lines, (env->lines_nb + 1) * sizeof(*env->lines));
		if (!lines)
		{
			fprintf(stderr, "%s: realloc: %s\n", env->progname, strerror(errno));
			free(line);
			return 1;
		}
		lines[env->lines_nb] = line;
		env->lines = lines;
		env->lines_nb++;
		line = NULL;
		len = 0;
	}
	if (errno)
	{
		fprintf(stderr, "%s: read: %s\n", env->progname, strerror(errno));
		return 1;
	}
	return 0;
}

static int read_file(struct env *env, const char *file)
{
	if (!strcmp(file, "-"))
		return read_fp(env, stdin);
	FILE *fp = fopen(file, "r");
	if (!fp)
	{
		fprintf(stderr, "%s: open: %s\n", env->progname, strerror(errno));
		return 1;
	}
	int ret = read_fp(env, fp);
	fclose(fp);
	return ret;
}

static void usage(const char *progname)
{
	printf("%s [-f] [-r] [FILES]\n", progname);
	printf("-f: case insensitive sort\n");
	printf("-r: reverse sort\n");
}

int main(int argc, char **argv)
{
	int (*cmp_fn)(const void *a, const void *b);
	struct env env;
	int c;

	memset(&env, 0, sizeof(env));
	env.progname = argv[0];
	while ((c = getopt(argc, argv, "fr")) != -1)
	{
		switch (c)
		{
			case 'f':
				env.opt |= OPT_f;
				break;
			case 'r':
				env.opt |= OPT_r;
				break;
			default:
				usage(argv[0]);
				return EXIT_FAILURE;
		}
	}
	if (optind != argc)
	{
		for (int i = optind; i < argc; ++i)
			read_file(&env, argv[i]);
	}
	else
	{
		read_fp(&env, stdin);
	}
	if (env.opt & OPT_f)
	{
		if (env.opt & OPT_r)
			cmp_fn = cmp_str_case_rev;
		else
			cmp_fn = cmp_str_case;
	}
	else
	{
		if (env.opt & OPT_r)
			cmp_fn = cmp_str_rev;
		else
			cmp_fn = cmp_str;
	}
	qsort(env.lines, env.lines_nb, sizeof(char*), cmp_fn);
	for (size_t i = 0; i < env.lines_nb; ++i)
		fputs(env.lines[i], stdout);
	return EXIT_SUCCESS;
}
