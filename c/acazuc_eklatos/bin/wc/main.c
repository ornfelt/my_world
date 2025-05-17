#include <inttypes.h>
#include <string.h>
#include <unistd.h>
#include <wctype.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <wchar.h>

#define OPT_l (1 << 0)
#define OPT_w (1 << 1)
#define OPT_c (1 << 2)
#define OPT_m (1 << 3)

#define IDX_LINES 0
#define IDX_WORDS 1
#define IDX_CHARS 2
#define IDX_BYTES 3

struct env
{
	const char *progname;
	int opt;
};

static void
wc_fp(struct env *env, FILE *fp, const char *path, uint64_t *total)
{
	char mb_buf[5];
	size_t mb_len = 0;
	uint64_t count[4] = {0};
	int c;
	int was_space = 1;

	while ((c = fgetc(fp)) != EOF)
	{
		total[IDX_BYTES]++;
		count[IDX_BYTES]++;
		wchar_t wc = 0;
		mb_buf[mb_len] = (unsigned char)c;
		if (mbrtowc(&wc, mb_buf, mb_len + 1, NULL) == (size_t)-2)
		{
			mb_len++;
			continue;
		}
		mb_len = 0;
		if (wc == L'\n')
		{
			total[IDX_LINES]++;
			count[IDX_LINES]++;
		}
		if (was_space)
		{
			if (!iswspace(wc))
			{
				was_space = 0;
				total[IDX_WORDS]++;
				count[IDX_WORDS]++;
			}
		}
		else
		{
			if (iswspace(wc))
				was_space = 1;
		}
		total[IDX_CHARS]++;
		count[IDX_CHARS]++;
	}
	if (ferror(fp))
	{
		fprintf(stderr, "%s: read: %s\n",
		        env->progname,
		        strerror(errno));
		return;
	}
	if (env->opt & OPT_l)
		printf("%" PRIu64 " ", count[IDX_LINES]);
	if (env->opt & OPT_w)
		printf("%" PRIu64 " ", count[IDX_WORDS]);
	if (env->opt & OPT_m)
		printf("%" PRIu64 " ", count[IDX_CHARS]);
	if (env->opt & OPT_c)
		printf("%" PRIu64 " ", count[IDX_BYTES]);
	puts(path);
}

static void
wc_file(struct env *env, const char *path, uint64_t *total)
{
	FILE *fp;

	fp = fopen(path, "r");
	if (!fp)
	{
		fprintf(stderr, "%s: open(%s): %s\n",
		        env->progname,
		        path,
		        strerror(errno));
		return;
	}
	wc_fp(env, fp, path, total);
	fclose(fp);
}

static void
usage(const char *progname)
{
	printf("%s [-c] [-h] [-l] [-m] [-w] [FILES]\n", progname);
	printf("-c: display the number of chars\n");
	printf("-h: show this help\n");
	printf("-l: display the number of lines\n");
	printf("-m: display the number of bytes\n");
	printf("-w: display the number of words\n");
}

int
main(int argc, char **argv)
{
	uint64_t total[4] = {0};
	struct env env;
	int c;

	memset(&env, 0, sizeof(env));
	env.progname = argv[0];
	while ((c = getopt(argc, argv, "lwcmh")) != -1)
	{
		switch (c)
		{
			case 'l':
				env.opt |= OPT_l;
				break;
			case 'w':
				env.opt |= OPT_w;
				break;
			case 'c':
				env.opt |= OPT_c;
				break;
			case 'm':
				env.opt |= OPT_m;
				break;
			case 'h':
				usage(argv[0]);
				return EXIT_SUCCESS;
			default:
				usage(argv[0]);
				return EXIT_FAILURE;
		}
	}
	if (!(env.opt & (OPT_l | OPT_w | OPT_c)))
		env.opt = OPT_l | OPT_w | OPT_c;
	if (optind == argc)
	{
		wc_fp(&env, stdout, "", total);
		return EXIT_SUCCESS;
	}
	for (int i = optind; i < argc; ++i)
		wc_file(&env, argv[i], total);
	if (optind + 1 < argc)
	{
		if (env.opt & OPT_l)
			printf("%" PRIu64 " ", total[IDX_LINES]);
		if (env.opt & OPT_w)
			printf("%" PRIu64 " ", total[IDX_WORDS]);
		if (env.opt & OPT_m)
			printf("%" PRIu64 " ", total[IDX_CHARS]);
		if (env.opt & OPT_c)
			printf("%" PRIu64 " ", total[IDX_BYTES]);
		puts("total");
	}
	return EXIT_SUCCESS;
}
