#include <stddef.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>

#define OPT_b (1 << 0)
#define OPT_e (1 << 1)
#define OPT_n (1 << 2)
#define OPT_s (1 << 3)
#define OPT_t (1 << 4)
#define OPT_v (1 << 5)

struct env
{
	const char *progname;
	uint32_t opt;
	uint8_t empty_nb;
	size_t line;
};

static void
output_filtered(struct env *env, const uint8_t *b, size_t n)
{
	for (size_t i = 0; i < n; ++i)
	{
		uint8_t c = b[i];

		if (c == '\n')
		{
			if ((env->opt & OPT_s) && env->empty_nb == 2)
				continue;
			env->empty_nb++;
			if (env->opt & OPT_e)
				putchar('$');
			putchar('\n');
			if ((env->empty_nb > 1) && env->opt & OPT_n)
				printf("%6zu\n", env->line);
			if (env->opt & OPT_b)
			{
				if (env->empty_nb != 2)
					printf("%6zu\n", env->line);
				else
					fputs("      \n", stdout);
			}
			continue;
		}
		if (env->empty_nb == 1 && (env->opt & OPT_n))
			printf("%6zu\n", env->line);
		env->empty_nb = 0;
		if (c == '\t')
		{
			if (env->opt & OPT_t)
				fputs("^I", stdout);
			else
				putchar('\t');
			continue;
		}
		if (!(env->opt & OPT_v))
		{
			putchar(c);
			continue;
		}
		if (c >= 128)
		{
			fputs("M-", stdout);
			c -= 128;
		}
		if (c < 32)
		{
			putchar('^');
			c += 64;
		}
		putchar(c);
	}
}

static void
output_standard(struct env *env, const uint8_t *b, size_t n)
{
	(void)env;
	write(1, b, n);
}

static void
print_fd(struct env *env, int fd)
{
	static uint8_t buffer[1024 * 1024];

	while (1)
	{
		ssize_t rd;

		rd = read(fd, buffer, sizeof(buffer));
		if (rd == -1)
		{
			if (errno == EINTR)
				continue;
			fprintf(stderr, "%s: read: %s\n",
			        env->progname,
			        strerror(errno));
			return;
		}
		if (!rd)
			return;
		if (env->opt & (OPT_s | OPT_e | OPT_b | OPT_t | OPT_v))
			output_filtered(env, buffer, rd);
		else
			output_standard(env, buffer, rd);
	}
}

static void
print_file(struct env *env, const char *file)
{
	int fd;

	fd = open(file, O_RDONLY);
	if (fd == -1)
	{
		perror(file);
		return;
	}
	print_fd(env, fd);
	close(fd);
}

static void
usage(const char *progname)
{
	printf("%s [-b] [-e] [-h] [-n] [-s] [-t] [-u] [-v] [FILES]\n", progname);
	printf("-b: number nonempty lines\n");
	printf("-e: implies -v, output a $ at the end of each line\n");
	printf("-h: display this help\n");
	printf("-n: prefix lines by the line number\n");
	printf("-s: don't display repeated empty lines\n");
	printf("-t: implies -v, display tabs as ^I\n");
	printf("-u: ignored\n");
	printf("-v: use ^ and M- notation, except for LFD and TAB\n");
}

int
main(int argc, char **argv)
{
	struct env env;
	int c;

	memset(&env, 0, sizeof(env));
	env.progname = argv[0];
	env.line = 1;
	env.empty_nb = 1;
	while ((c = getopt(argc, argv, "behnstuv")) != -1)
	{
		switch (c)
		{
			case 'b':
				env.opt |= OPT_b;
				env.opt &= ~OPT_n;
				break;
			case 'e':
				env.opt |= OPT_v;
				env.opt |= OPT_e;
				break;
			case 'h':
				usage(argv[0]);
				return EXIT_SUCCESS;
			case 'n':
				env.opt |= OPT_n;
				break;
			case 's':
				env.opt |= OPT_s;
				break;
			case 't':
				env.opt |= OPT_v;
				env.opt |= OPT_t;
				break;
			case 'u':
				break;
			case 'v':
				env.opt |= OPT_v;
				break;
			default:
				usage(argv[0]);
				return EXIT_FAILURE;
		}
	}
	if (optind == argc)
		print_fd(&env, 0);
	for (int i = optind; i < argc; ++i)
		print_file(&env, argv[i]);
	return 0;
}
