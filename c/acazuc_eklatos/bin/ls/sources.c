#include "ls.h"

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

static int insert(struct env *env, struct source *ls, struct source *cs)
{
	if (env->opt & OPT_U)
		return 0;
	if ((env->opt & OPT_S) && cs->sort_size != ls->sort_size)
	{
		if (env->opt & OPT_r)
			return cs->sort_size < ls->sort_size;
		else
			return cs->sort_size > ls->sort_size;
	}
	if ((env->opt & OPT_t) && cs->sort_date != ls->sort_date)
	{
		if (env->opt & OPT_r)
			return cs->sort_date < ls->sort_date;
		else
			return cs->sort_date > ls->sort_date;
	}
	else
	{
		if (env->opt & OPT_r)
			return strcmp(cs->display_path, ls->display_path) > 0;
		else
			return strcmp(cs->display_path, ls->display_path) < 0;
	}
}

static void push(struct env *env, struct source *source)
{
	if (TAILQ_EMPTY(&env->sources))
	{
		TAILQ_INSERT_HEAD(&env->sources, source, chain);
		return;
	}
	struct source *lst;
	TAILQ_FOREACH(lst, &env->sources, chain)
	{
		if (insert(env, lst, source))
		{
			TAILQ_INSERT_BEFORE(lst, source, chain);
			return;
		}
	}
	TAILQ_INSERT_TAIL(&env->sources, source, chain);
}

static void print_dir_fast(struct env *env, struct dir *dir)
{
	struct file *lst, *nxt;
	TAILQ_FOREACH_SAFE(lst, &dir->files, chain, nxt)
	{
		print_file(env, lst, dir, !nxt);
		free_file(lst);
	}
}

static void push_source(struct env *env, char *path, char *display_path, const struct stat *st)
{
	struct source *new;

	new = malloc(sizeof(*new));
	if (!new)
	{
		fprintf(stderr, "%s: malloc: %s\n", env->progname, strerror(errno));
		exit(EXIT_FAILURE);
	}
	new->display_path = display_path;
	new->path = path;
	new->sort_date = file_time(env, st);
	new->sort_size = st->st_size;
	push(env, new);
}

static int add_source(struct env *env, char *path, struct dir *dir, char *display_path)
{
	struct stat st;

	if (env->opt & OPT_H)
	{
		if (stat(path, &st) == -1)
		{
			fprintf(stderr, "%s: stat(%s): %s\n",
			       env->progname, path, strerror(errno));
			return 0;
		}
	}
	else
	{
		if (lstat(path, &st) == -1)
		{
			fprintf(stderr, "%s: lstat(%s): %s\n",
			        env->progname, path, strerror(errno));
			return 0;
		}
	}
	if (S_ISDIR(st.st_mode) && !(env->opt & OPT_d))
	{
		push_source(env, path, display_path, &st);
		return 0;
	}
	dir_add_file(env, dir, path);
	return 1;
}

void parse_sources(struct env *env, int argc, char **argv)
{
	struct dir dir;
	char *display_path;
	int printed_file;
	int printed;

	dir_init(&dir, ".");
	printed = 0;
	printed_file = 0;
	for (int i = 0; i < argc; ++i)
	{
		display_path = strdup(argv[i]);
		size_t len = strlen(argv[i]);
		if (len > 1 && argv[i][len - 1] == '/')
			argv[i][len - 1] = '\0';
		printed_file += add_source(env, argv[i], &dir, display_path);
		printed = 1;
	}
	print_dir_fast(env, &dir);
	if (!printed)
		print_dir(env, ".", 0, ".");
	else
		print_sources(env, printed_file || argc >= 2);
}
