#include "ls.h"

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

static void usage(const char *progname)
{
	printf("%s [-a] [-A] [-b] [-c] [-d] [-f] [-F] [-g] [-G] [-h] [-H] [-i] [-l] [-L] [-m] [-n] [-N] [-o] [-p] [-q] [-Q] [-r] [-R] [-s] [-S] [-t] [-u] [-U] [-x] [-1] [FILES]\n", progname);
	printf("-a: display entries starting with .\n");
	printf("-A: display entries starting with ., except . and ..\n");
	printf("-b: print C-style escape codes for non printable chars\n");
	printf("-c: sort by and display ctime\n");
	printf("-d: list directories instead of their content\n");
	printf("-f: enable -a and -U, disable -l & -s\n");
	printf("-F: display type indicator after file names\n");
	printf("-g: enable -l and -G\n");
	printf("-G: don't display group in long listing\n");
	printf("-h: display sizes in human friendly format\n");
	printf("-H: dereference command line symbolic links\n");
	printf("-i: display inodes number\n");
	printf("-l: display long listing\n");
	printf("-L: dereference symbolic links\n");
	printf("-m: use coma as column separator, disable -l\n");
	printf("-n: display numerical values of uid & gid\n");
	printf("-N: never quote file names\n");
	printf("-o: enable -l, don't display files group\n");
	printf("-p: display a slash after directories names\n");
	printf("-q: print a question mask instead of non-printable characters\n");
	printf("-Q: always quote file names\n");
	printf("-r: reverse sort order\n");
	printf("-R: enable recursive listing\n");
	printf("-s: print the number of blocks allocated\n");
	printf("-S: sort by file size\n");
	printf("-t: sort by mtime\n");
	printf("-u: sort by and display atime\n");
	printf("-U: don't sort\n");
	printf("-x: output colors\n");
	printf("-1: display one file per line\n");
}

int main(int argc, char **argv)
{
	struct env env;
	int c;

	memset(&env, 0, sizeof(env));
	env.progname = argv[0];
	TAILQ_INIT(&env.sources);
	while ((c = getopt(argc, argv, "aAbcdfFgGhHilLmnNopqQrRsStuUx1")) != -1)
	{
		switch (c)
		{
			case 'a':
				env.opt |= OPT_a | OPT_A;
				break;
			case 'A':
				env.opt |= OPT_A;
				break;
			case 'b':
				env.opt |= OPT_b;
				break;
			case 'c':
				env.opt |= OPT_c;
				break;
			case 'd':
				env.opt |= OPT_d;
				break;
			case 'f':
				env.opt |= OPT_a | OPT_U;
				env.opt &= ~(OPT_l | OPT_s);
				break;
			case 'F':
				env.opt |= OPT_F;
				break;
			case 'g':
				env.opt |= OPT_l | OPT_G;
				break;
			case 'G':
				env.opt |= OPT_G;
				break;
			case 'h':
				env.opt |= OPT_h;
				break;
			case 'H':
				env.opt |= OPT_H;
				break;
			case 'i':
				env.opt |= OPT_i;
				break;
			case 'l':
				env.opt |= OPT_l;
				break;
			case 'L':
				env.opt |= OPT_L;
				break;
			case 'm':
				env.opt |= OPT_m;
				env.opt &= ~OPT_l;
				break;
			case 'n':
				env.opt |= OPT_l | OPT_n;
				break;
			case 'N':
				env.opt |= OPT_N;
				env.opt &= ~OPT_Q;
				break;
			case 'o':
				env.opt |= OPT_l | OPT_o;
				break;
			case 'p':
				env.opt |= OPT_p;
				break;
			case 'q':
				env.opt |= OPT_q;
				break;
			case 'Q':
				env.opt |= OPT_Q;
				env.opt &= ~OPT_N;
				break;
			case 'r':
				env.opt |= OPT_r;
				break;
			case 'R':
				env.opt |= OPT_R;
				break;
			case 's':
				env.opt |= OPT_s;
				break;
			case 'S':
				env.opt |= OPT_S;
				break;
			case 't':
				env.opt |= OPT_t;
				break;
			case 'u':
				env.opt |= OPT_u;
				break;
			case 'U':
				env.opt |= OPT_U;
				break;
			case 'x':
				env.opt |= OPT_x;
				break;
			case '1':
				env.opt |= OPT_1;
				break;
			default:
				usage(argv[0]);
				return EXIT_FAILURE;
		}
	}
	if (isatty(1))
		env.opt |= OPT_x;
	if (env.opt & OPT_l)
		env.opt &= ~OPT_1;
	parse_sources(&env, argc - optind, &argv[optind]);
	return EXIT_SUCCESS;
}
