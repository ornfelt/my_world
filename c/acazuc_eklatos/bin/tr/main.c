#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#define OPT_d (1 << 0)
#define OPT_t (1 << 1)
#define OPT_c (1 << 2)

struct env
{
	const char *progname;
	char *set1;
	size_t len1;
	char *set2;
	size_t len2;
	int opt;
};

static int
delete_chars(struct env *env)
{
	int c;

	while ((c = getc_unlocked(stdin)) != EOF)
	{
		char *pos;

		pos = memchr(env->set1, c, env->len1);
		if (pos)
			continue;
		putc_unlocked(c, stdout);
		if (ferror_unlocked(stdout))
		{
			fprintf(stderr, "%s: putc: %s\n",
			        env->progname,
			        strerror(errno));
			return 1;
		}
	}
	if (ferror(stdin))
	{
		fprintf(stderr, "%s: getc: %s",
		        env->progname,
		        strerror(errno));
		return 1;
	}
	return 0;
}

static int
translate_chars(struct env *env)
{
	char *replace;
	size_t len;
	int c;

	replace = malloc(env->len1);
	if (!replace)
	{
		fprintf(stderr, "%s: malloc: %s\n",
		        env->progname,
		        strerror(errno));
		return 1;
	}
	for (len = 0; len < env->len2; ++len)
		replace[len] = env->set2[len];
	for (; len < env->len1; ++len)
		replace[len] = replace[env->len2 - 1];
	while ((c = getc_unlocked(stdin)) != EOF)
	{
		char *pos;

		pos = memchr(env->set1, c, env->len1);
		if (pos)
			c = replace[pos - env->set1];
		putc_unlocked(c, stdout);
		if (ferror_unlocked(stdout))
		{
			fprintf(stderr, "%s: putc: %s\n",
			        env->progname,
			        strerror(errno));
			return 1;
		}
	}
	free(replace);
	if (ferror(stdin))
	{
		fprintf(stderr, "%s: getc: %s",
		        env->progname,
		        strerror(errno));
		return 1;
	}
	return 0;
}

static int
add_set_char(struct env *env, char **set, size_t *len, size_t *size, char c)
{
	if (*len + 1 >= *size)
	{
		char *dup;

		dup = realloc(*set, *size * 2);
		if (!dup)
		{
			fprintf(stderr, "%s: malloc: %s\n",
			        env->progname,
			        strerror(errno));
			return 1;
		}
		*set = dup;
		*size *= 2;
	}
	(*set)[(*len)++] = c;
	return 0;
}

static int
eval_set(struct env *env, const char *set, char **setp, size_t *lenp)
{
	size_t len = strlen(set);
	char *dup;
	size_t dup_len = 0;
	size_t dup_size = 32;

	dup = malloc(dup_size);
	if (!dup)
	{
		fprintf(stderr, "%s: malloc: %s\n",
		        env->progname,
		        strerror(errno));
		return 1;
	}
	for (size_t i = 0; i < len; ++i)
	{
		if (set[i] == '\\')
		{
			switch (set[i + 1])
			{
				case '\\':
					if (add_set_char(env, &dup, &dup_len,
					                 &dup_size, '\\'))
						return 1;
					i++;
					break;
				case 'a':
					if (add_set_char(env, &dup, &dup_len,
					                 &dup_size, '\a'))
						return 1;
					i++;
					break;
				case 'b':
					if (add_set_char(env, &dup, &dup_len,
					                 &dup_size, '\b'))
						return 1;
					i++;
					break;
				case 'f':
					if (add_set_char(env, &dup, &dup_len,
					                 &dup_size, '\f'))
						return 1;
					i++;
					break;
				case 'n':
					if (add_set_char(env, &dup, &dup_len,
					                 &dup_size, '\n'))
						return 1;
					i++;
					break;
				case 'r':
					if (add_set_char(env, &dup, &dup_len,
					                 &dup_size, '\r'))
						return 1;
					i++;
					break;
				case 't':
					if (add_set_char(env, &dup, &dup_len,
					                 &dup_size, '\t'))
						return 1;
					i++;
					break;
				case 'v':
					if (add_set_char(env, &dup, &dup_len,
					                 &dup_size, '\v'))
						return 1;
					i++;
					break;
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				{
					unsigned c = 0;
					for (int j = 0; j < 3; ++j)
					{
						if (set[i + 1] < '0'
						 || set[i + 1] > '7')
							break;
						unsigned v = set[i + 1] - '0';
						unsigned newc = c * 8 | v;
						if (newc > 256)
							break;
						c = newc;
						i++;
					}
					if (add_set_char(env, &dup, &dup_len,
					                 &dup_size, c))
						return 1;
					break;
				}
				default:
					if (add_set_char(env, &dup, &dup_len, &dup_size, set[i]))
						return 1;
					break;
			}
		}
		else if (set[i + 1] == '-' && set[i + 2])
		{
			unsigned char c1 = set[i];
			unsigned char c2 = set[i + 2];
			if (c1 > c2)
			{
				unsigned char tmp = c1;
				c1 = c2;
				c2 = tmp;
			}
			for (unsigned c = c1; c <= c2; ++c)
			{
				if (add_set_char(env, &dup, &dup_len,
				                 &dup_size, c))
					return 1;
			}
			i += 2;
		}
		else
		{
			if (add_set_char(env, &dup, &dup_len, &dup_size, set[i]))
				return 1;
		}
	}
	dup[dup_len] = '\0';
	*setp = dup;
	*lenp = dup_len;
	return 0;
}

static void
usage(const char *progname)
{
	printf("%s [-c] [-d] [-h] [-t] SET1 [SET2]\n", progname);
	printf("-c: match complement of SET1\n");
	printf("-d: delete chars from SET1\n");
	printf("-h: show this help\n");
	printf("-t: truncate SET1 to the length of SET2\n");
}

int
main(int argc, char **argv)
{
	struct env env;
	int c;

	memset(&env, 0, sizeof(env));
	env.progname = argv[0];
	while ((c = getopt(argc, argv, "hdt")) != -1)
	{
		switch (c)
		{
			case 'd':
				env.opt |= OPT_d;
				break;
			case 't':
				env.opt |= OPT_t;
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
	if (env.opt & OPT_d)
	{
		if (argc > optind + 1)
		{
			fprintf(stderr, "%s: extra operand\n", argv[0]);
			return EXIT_FAILURE;
		}
		if (eval_set(&env, argv[optind], &env.set1, &env.len1)
		 || delete_chars(&env))
			return EXIT_FAILURE;
	}
	else
	{
		if (argc > optind + 2)
		{
			fprintf(stderr, "%s: extra operand\n", argv[0]);
			return EXIT_FAILURE;
		}
		if (eval_set(&env, argv[optind], &env.set1, &env.len1)
		 || eval_set(&env, argv[optind + 1], &env.set2, &env.len2))
			return EXIT_FAILURE;
		if (env.opt & OPT_t)
		{
			if (env.len1 > env.len2)
				env.len1 = env.len2;
		}
		if (env.len2 > env.len1)
			env.len2 = env.len1;
		if (translate_chars(&env))
			return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
