#include <sys/param.h>
#include <sys/stat.h>

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#define OPT_v (1 << 0)

struct env
{
	const char *progname;
	int dst_isdir;
	int opt;
};

static int move_file(struct env *env, const char *file, const char *dst)
{
	char path[MAXPATHLEN];
	size_t file_len = strlen(file);
	const char *end = file + file_len;
	while (end >= file && *end == '/')
		end--;
	if (end <= file)
	{
		fprintf(stderr, "%s: invalid operand\n", env->progname);
		return 1;
	}
	const char *begin = end;
	while (begin > file && *begin != '/')
		begin--;
	if (*begin == '/')
		begin++;
	if (env->dst_isdir)
		snprintf(path, sizeof(path), "%s/%.*s", dst, (int)(end - begin), begin);
	else
		strlcpy(path, dst, sizeof(path));
	if (env->opt & OPT_v)
		printf("moving '%s' to '%s'\n", file, path);
	if (rename(file, path) == -1)
	{
		if (errno == EXDEV)
		{
			/* XXX cp */
		}
		fprintf(stderr, "%s: rename: %s\n", env->progname,
		        strerror(errno));
		return 1;
	}
	return 0;
}

static void usage(const char *progname)
{
	printf("%s [-v] SOURCES DEST\n", progname);
	printf("-v: verbose operations\n");
}

int main(int argc, char **argv)
{
	struct env env;
	int c;

	memset(&env, 0, sizeof(env));
	env.progname = argv[0];
	while ((c = getopt(argc, argv, "v")) != -1)
	{
		switch (c)
		{
			case 'v':
				env.opt |= OPT_v;
				break;
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
	const char *dst = argv[argc - 1];
	struct stat dst_st;
	if (stat(dst, &dst_st) == -1)
	{
		if (errno == ENOENT)
		{
			env.dst_isdir = 0;
		}
		else
		{
			fprintf(stderr, "%s: stat(%s): %s\n", argv[0], dst,
			        strerror(errno));
			return EXIT_FAILURE;
		}
	}
	else
	{
		env.dst_isdir = S_ISDIR(dst_st.st_mode);
	}
	if (argc - optind > 2 && !env.dst_isdir)
	{
		fprintf(stderr, "%s: can't move multiple files to non-directory file\n",
		        argv[0]);
		return EXIT_FAILURE;
	}
	for (int i = optind; i < argc - 1; ++i)
	{
		if (move_file(&env, argv[i], argv[argc - 1]))
			return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
