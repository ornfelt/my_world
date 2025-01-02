#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>

#define OPT_f (1 << 0)

struct env
{
	const char *progname;
	long min_len;
	char *buf;
	int opt;
	const char *sep;
};

static void strings_file(struct env *env, const char *file)
{
	FILE *fp = fopen(file, "r");
	if (!fp)
	{
		fprintf(stderr, "%s: %s\n", env->progname, strerror(errno));
		goto end;
	}
	long buf_sz = 0;
	int c;
	while ((c = getc(fp)) != EOF)
	{
		if (!isprint(c))
		{
			if (buf_sz == env->min_len)
				fputs(env->sep, stdout);
			buf_sz = 0;
			continue;
		}
		if (buf_sz == env->min_len)
		{
			putchar(c);
		}
		else if (buf_sz == env->min_len - 1)
		{
			if (env->opt & OPT_f)
				printf("%s: ", file);
			fwrite(env->buf, 1, env->min_len - 1, stdout);
			buf_sz++;
			putchar(c);
		}
		else
		{
			env->buf[buf_sz++] = c;
		}
	}

end:
	if (fp)
		fclose(fp);
}

static void usage(const char *progname)
{
	printf("%s [-h] [-n minlen] [-f] [-s separator] FILES\n", progname);
	printf("-h          : show this help\n");
	printf("-n minlen   : set the minimum length of a string (1 to 1024)\n");
	printf("-f          : display the filename before each match\n");
	printf("-s separator: use the given separator between each match instead of newline\n");
}

int main(int argc, char **argv)
{
	struct env env;
	int c;

	memset(&env, 0, sizeof(env));
	env.progname = argv[0];
	env.min_len = 4;
	env.sep = "\n";
	while ((c = getopt(argc, argv, "hn:fs:")) != -1)
	{
		switch (c)
		{
			case 'n':
			{
				errno = 0;
				char *endptr;
				env.min_len = strtol(optarg, &endptr, 0);
				if (errno)
				{
					fprintf(stderr, "%s: %s\n", argv[0], strerror(errno));
					return EXIT_FAILURE;
				}
				if (*endptr)
				{
					fprintf(stderr, "%s: invalid value\n", argv[0]);
					return EXIT_FAILURE;
				}
				if (env.min_len < 0 || env.min_len > 1024)
				{
					fprintf(stderr, "%s: invalid value\n", argv[0]);
					return EXIT_FAILURE;
				}
				break;
			}
			case 'f':
				env.opt |= OPT_f;
				break;
			case 's':
				env.sep = optarg;
				break;
			case 'h':
				usage(argv[0]);
				return EXIT_SUCCESS;
			default:
				usage(argv[0]);
				return EXIT_FAILURE;
		}
	}
	if (optind == argc)
	{
		fprintf(stderr, "%s: missing operand\n", argv[0]);
		return EXIT_FAILURE;
	}
	env.buf = malloc(env.min_len);
	if (!env.buf)
	{
		fprintf(stderr, "%s: %s\n", argv[0], strerror(errno));
		return EXIT_FAILURE;
	}
	for (int i = optind; i < argc; ++i)
		strings_file(&env, argv[i]);
	return EXIT_SUCCESS;
}
