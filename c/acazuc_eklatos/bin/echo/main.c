#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#define OPT_n (1 << 0)
#define OPT_e (1 << 1)

struct env
{
	int opt;
};

static void
usage(const char *progname)
{
	printf("%s [-n] [-e] strings\n", progname);
	printf("-n: don't output newline\n");
	printf("-e: enable escape sequence\n");
}

static void
print_x(const char *str, size_t *i)
{
	uint8_t byte = 0;

	if (str[*i + 2])
	{
		(*i)++;
		char c = tolower(str[(*i) + 1]);
		if (c >= '0' && c <= '9')
		{
			byte += c - '0';
		}
		else if (c >= 'a' && c <= 'f')
		{
			byte += c - 'a' + 10;
		}
		else
		{
			putchar('\\');
			putchar(str[(*i) + 0]);
			putchar(str[(*i) + 1]);
			return;
		}
		if (str[(*i) + 2])
		{
			(*i)++;
			c = tolower(str[(*i) + 1]);
			if (c >= '0' && c <= '9')
			{
				byte += c - '0';
			}
			else if (c >= 'a' && c <= 'f')
			{
				byte += c - 'a' + 10;
			}
			else
			{
				putchar(byte);
				putchar(str[(*i) + 1]);
				return;
			}
		}
		putchar(byte);
	}
}

static int
echo_str(struct env *env, const char *str, int last)
{
	if (env->opt & OPT_e)
	{
		for (size_t i = 0; str[i]; ++i)
		{
			if (str[i] != '\\')
			{
				putchar(str[i]);
				continue;
			}
			switch (str[i + 1])
			{
				case '\\':
					putchar('\\');
					break;
				case 'a':
					putchar('\a');
					break;
				case 'b':
					putchar('\b');
					break;
				case 'c':
					return 1;
				case 'e':
					putchar('\033');
					break;
				case 'f':
					putchar('\f');
					break;
				case 'n':
					putchar('\n');
					break;
				case 'r':
					putchar('\r');
					break;
				case 't':
					putchar('\t');
					break;
				case 'v':
					putchar('\v');
					break;
				case '0':
					break;
				case 'x':
				{
					print_x(str, &i);
					break;
				}
				default:
					putchar('\\');
					putchar(str[i + 1]);
					break;
			}
			i++;
		}
	}
	else
	{
		fputs(str, stdout);
	}
	if (env->opt & OPT_n)
	{
		putchar(' ');
	}
	else
	{
		if (last)
			putchar('\n');
		else
			putchar(' ');
	}
	return 0;
}

int
main(int argc, char **argv)
{
	struct env env;
	int c;

	memset(&env, 0, sizeof(env));
	while ((c = getopt(argc, argv, "ne")) != -1)
	{
		switch (c)
		{
			case 'n':
				env.opt |= OPT_n;
				break;
			case 'e':
				env.opt |= OPT_e;
				break;
			default:
				usage(argv[0]);
				return EXIT_FAILURE;
		}
	}
	for (int i = optind; i < argc; ++i)
	{
		if (echo_str(&env, argv[i], i == argc - 1))
			break;
	}
	return EXIT_SUCCESS;
}
