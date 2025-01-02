#include <sys/utsname.h>

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

#define OPT_s (1 << 0)
#define OPT_n (1 << 1)
#define OPT_r (1 << 2)
#define OPT_v (1 << 3)
#define OPT_m (1 << 4)
#define OPT_o (1 << 5)

struct env
{
	int opt;
};

static void usage(const char *progname)
{
	printf("%s [-a] [-s] [-n] [-r] [-v] [-m] [-o]\n", progname);
	printf("-a: print all the fields\n");
	printf("-s: print the kernel name\n");
	printf("-n: print the hostname\n");
	printf("-r: print the kernel release\n");
	printf("-v: print the kernel version\n");
	printf("-m: print the machine name\n");
	printf("-o: print the operating system\n");
}

int main(int argc, char **argv)
{
	struct utsname utsname;
	struct env env;
	int c;

	memset(&env, 0, sizeof(env));
	while ((c = getopt(argc, argv, "asnrvmo")) != -1)
	{
		switch (c)
		{
			case 'a':
				env.opt |= OPT_s | OPT_n | OPT_r | OPT_v | OPT_m | OPT_o;
				break;
			case 's':
				env.opt |= OPT_s;
				break;
			case 'n':
				env.opt |= OPT_n;
				break;
			case 'r':
				env.opt |= OPT_r;
				break;
			case 'v':
				env.opt |= OPT_v;
				break;
			case 'm':
				env.opt |= OPT_m;
				break;
			case 'o':
				env.opt |= OPT_o;
				break;
			default:
				usage(argv[0]);
				return EXIT_FAILURE;
		}
	}
	if (!env.opt)
		env.opt = OPT_s;
	if (uname(&utsname) == -1)
	{
		fprintf(stderr, "%s: uname: %s\n", argv[0], strerror(errno));
		return EXIT_FAILURE;
	}
	if (env.opt & OPT_s)
		printf("%s ", utsname.sysname);
	if (env.opt & OPT_n)
		printf("%s ", utsname.nodename);
	if (env.opt & OPT_r)
		printf("%s ", utsname.release);
	if (env.opt & OPT_v)
		printf("%s ", utsname.version);
	if (env.opt & OPT_m)
		printf("%s ", utsname.machine);
	if (env.opt & OPT_o)
		printf("%s ", "eklatOS");
	printf("\n");
	return EXIT_SUCCESS;
}
