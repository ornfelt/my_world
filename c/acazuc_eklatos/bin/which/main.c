#include <sys/param.h>

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

#define OPT_a (1 << 0)

struct env
{
	const char *progname;
	int opt;
};

extern char **environ;

static void
print_which(struct env *env, char **path, const char *name)
{
	for (size_t i = 0; path[i]; ++i)
	{
		char pathname[MAXPATHLEN];

		if (snprintf(pathname, sizeof(pathname), "%s/%s", path[i], name) >= (int)sizeof(pathname))
		{
			fprintf(stderr, "%s: path too long\n", env->progname);
			continue;
		}
		if (!access(pathname, X_OK))
		{
			puts(pathname);
			if (!(env->opt & OPT_a))
				return;
		}
	}
}

static char **
build_path(const char *progname)
{
	size_t ret_size = 0;
	const char *path;
	const char *tmp;
	char **ret;

	ret = calloc(1, sizeof(*ret));
	if (!ret)
		return NULL;
	path = getenv("PATH");
	if (!path)
		path = "/bin:/usr/bin";
	while ((tmp = strchrnul(path, ':')))
	{
		char **new_ret;

		new_ret = realloc(ret, sizeof(*ret) * (ret_size + 2));
		if (!new_ret)
		{
			fprintf(stderr, "%s: malloc: %s\n",
			        progname,
			        strerror(errno));
			goto err;
		}
		ret = new_ret;
		new_ret[ret_size] = strndup(path, tmp - path);
		if (!new_ret[ret_size])
		{
			fprintf(stderr, "%s: malloc: %s\n",
			        progname,
			        strerror(errno));
			goto err;
		}
		ret_size++;
		new_ret[ret_size] = NULL;
		if (!*tmp)
			break;
		path = tmp + 1;
	}
	return ret;

err:
	for (size_t i = 0; ret[i]; ++i)
		free(ret[i]);
	free(ret);
	return NULL;
}

static void
usage(const char *progname)
{
	printf("%s [-a] FILES\n", progname);
	printf("-a: display all the matching files\n");
}

int
main(int argc, char **argv)
{
	struct env env;
	char **path = NULL;
	int c;

	memset(&env, 0, sizeof(env));
	env.progname = argv[0];
	while ((c = getopt(argc, argv, "a")) != -1)
	{
		switch (c)
		{
			case 'a':
				env.opt |= OPT_a;
				break;
			default:
				usage(argv[0]);
				return EXIT_FAILURE;
		}
	}
	path = build_path(argv[0]);
	if (!path)
		return EXIT_FAILURE;
	for (int i = optind; i < argc; ++i)
		print_which(&env, path, argv[i]);
	return EXIT_SUCCESS;
}
