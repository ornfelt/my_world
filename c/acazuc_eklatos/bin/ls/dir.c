#include "ls.h"

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>

static void
check_lengths(struct env *env, struct dir *dir, struct file *file)
{
	int len;

	if (!(env->opt & OPT_l))
		return;
	if ((len = strlen(file->links)) > dir->links_len)
		dir->links_len = len;
	if ((len = strlen(file->user)) > dir->user_len)
		dir->user_len = len;
	if ((len = strlen(file->group)) > dir->group_len)
		dir->group_len = len;
	if ((len = strlen(file->size)) > dir->size_len)
		dir->size_len = len;
	if ((len = strlen(file->date)) > dir->date_len)
		dir->date_len = len;
	if ((len = strlen(file->ino)) > dir->ino_len)
		dir->ino_len = len;
	if ((len = strlen(file->blocks)) > dir->blocks_len)
		dir->blocks_len = len;
}

static int
insert(struct env *env, struct file *lf, struct file *cf)
{
	if (env->opt & OPT_U)
		return 0;
	if ((env->opt & OPT_S) && cf->sort_size != lf->sort_size)
	{
		if (env->opt & OPT_r)
			return cf->sort_size < lf->sort_size;
		else
			return cf->sort_size > lf->sort_size;
	}
	if ((env->opt & OPT_t) && cf->sort_date != lf->sort_date)
	{
		if (env->opt & OPT_r)
			return cf->sort_date < lf->sort_date;
		else
			return cf->sort_date > lf->sort_date;
	}
	else
	{
		if (env->opt & OPT_r)
			return strcmp(cf->name, lf->name) > 0;
		else
			return strcmp(cf->name, lf->name) < 0;
	}
}

static void
add_file(struct env *env, struct dir *dir, struct file *file)
{
	struct file *lst;

	if (TAILQ_EMPTY(&dir->files))
	{
		TAILQ_INSERT_HEAD(&dir->files, file, chain);
		return;
	}
	TAILQ_FOREACH(lst, &dir->files, chain)
	{
		if (insert(env, lst, file))
		{
			TAILQ_INSERT_BEFORE(lst, file, chain);
			return;
		}
	}
	TAILQ_INSERT_TAIL(&dir->files, file, chain);
}

void
dir_add_file(struct env *env, struct dir *dir, const char *name)
{
	struct file *file;

	file = malloc(sizeof(*file));
	if (!file)
	{
		fprintf(stderr, "%s: malloc: %s\n",
		        env->progname,
		        strerror(errno));
		exit(EXIT_FAILURE);
	}
	if (load_file(env, file, name, dir))
	{
		free(file);
		return;
	}
	check_lengths(env, dir, file);
	add_file(env, dir, file);
}

void
dir_init(struct dir *dir, const char *path)
{
	memset(dir, 0, sizeof(*dir));
	dir->path = path;
	TAILQ_INIT(&dir->files);
}

struct dir *
load_dir(struct env *env, const char *path)
{
	struct dir *dir;
	DIR *fdir;
	struct dirent *dirent;

	fdir = opendir(path);
	if (!fdir)
	{
		fprintf(stderr, "%s: opendir(%s): %s\n",
		        env->progname,
		        path,
		        strerror(errno));
		return NULL;
	}
	dir = malloc(sizeof(*dir));
	if (!dir)
	{
		fprintf(stderr, "%s: malloc: %s\n",
		        env->progname,
		        strerror(errno));
		exit(EXIT_FAILURE);
	}
	dir_init(dir, path);
	dir->dir = fdir;
	while ((dirent = readdir(fdir)))
	{
		if (dirent->d_name[0] == '.')
		{
			if (!strcmp(dirent->d_name, ".") || !strcmp(dirent->d_name, ".."))
			{
				if (!(env->opt & OPT_a))
					continue;
			}
			if (!(env->opt & OPT_A))
				continue;
		}
		dir_add_file(env, dir, dirent->d_name);
	}
	closedir(fdir);
	dir->dir = NULL;
	return dir;
}
