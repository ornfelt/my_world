#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

struct env
{
	const char *progname;
};

static int
diff_files(struct env *env, const char *file1, const char *file2)
{
	FILE *fp1 = NULL;
	FILE *fp2 = NULL;
	char *line1 = NULL;
	char *line2 = NULL;
	size_t len1 = 0;
	size_t len2 = 0;
	int ret = 1;

	fp1 = fopen(file1, "r");
	if (!fp1)
	{
		fprintf(stderr, "%s: open(%s): %s\n", env->progname, file1,
		        strerror(errno));
		goto end;
	}
	fp2 = fopen(file2, "r");
	if (!fp2)
	{
		fprintf(stderr, "%s: open(%s): %s\n", env->progname, file2,
		        strerror(errno));
		goto end;
	}
	while (1)
	{
		ssize_t rd1;
		ssize_t rd2;

		errno = 0;
		rd1 = getline(&line1, &len1, fp1);
		if (rd1 < 0 && errno)
		{
			fprintf(stderr, "%s: read(%s): %s\n", env->progname, file1,
			        strerror(errno));
			goto end;
		}
		errno = 0;
		rd2 = getline(&line2, &len2, fp2);
		if (rd2 < 0 && errno)
		{
			fprintf(stderr, "%s: read(%s): %s\n", env->progname, file2,
			        strerror(errno));
			goto end;
		}
		if (rd1 != rd2)
			goto end;
		if (rd1 <= 0)
			break;
		if (strcmp(line1, line2))
			goto end;
	}
	ret = 0;

end:
	free(line1);
	free(line2);
	if (fp1)
		fclose(fp1);
	if (fp2)
		fclose(fp2);
	return ret;
}

static void
usage(const char *progname)
{
	printf("%s [-h] FILES\n", progname);
	printf("-h: show this help\n");
}

int
main(int argc, char **argv)
{
	struct env env;
	int c;

	memset(&env, 0, sizeof(env));
	env.progname = argv[0];
	while ((c = getopt(argc, argv, "h")) != -1)
	{
		switch (c)
		{
			case 'h':
				usage(argv[0]);
				return EXIT_SUCCESS;
			default:
				usage(argv[0]);
				return EXIT_FAILURE;
		}
	}
	if (argc - optind < 2)
	{
		fprintf(stderr, "%s: missing operand\n", argv[0]);
		return EXIT_FAILURE;
	}
	if (argc - optind > 2)
	{
		fprintf(stderr, "%s: extra operand\n", argv[0]);
		return EXIT_FAILURE;
	}
	if (diff_files(&env, argv[optind], argv[optind + 1]))
		return EXIT_FAILURE;
	return EXIT_SUCCESS;
}
