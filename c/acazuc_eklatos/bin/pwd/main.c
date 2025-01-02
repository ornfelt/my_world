#include <sys/param.h>

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct env
{
	int logical;
};

static void usage(const char *progname)
{
	printf("%s [-L] [-P]\n", progname);
	printf("-L: print logical cwd\n");
	printf("-P: print physical cwd\n");
}

int main(int argc, char **argv)
{
	struct env env;
	int c;

	memset(&env, 0, sizeof(env));
	while ((c = getopt(argc, argv, "LP")) != -1)
	{
		switch (c)
		{
			case 'L':
				env.logical = 1;
				break;
			case 'P':
				env.logical = 0;
				break;
			default:
				usage(argv[0]);
				return EXIT_FAILURE;
		}
	}
	if (env.logical)
	{
		char *pwd = getenv("PWD");
		if (!pwd)
			return EXIT_FAILURE;
		puts(pwd);
		return EXIT_SUCCESS;
	}
	char path[MAXPATHLEN];
	if (!getcwd(path, sizeof(path)))
		return EXIT_FAILURE;
	puts(path);
	return EXIT_SUCCESS;
}
