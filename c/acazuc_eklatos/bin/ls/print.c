#include "ls.h"

#include <sys/param.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

static void
print_name(struct env *env, const char *name, mode_t mode)
{
	/* XXX quote if non-alphanum */
	if (mode != (mode_t)-1 && (env->opt & OPT_x))
	{
		if (S_ISLNK(mode))
		{
			fputs("\033[1;36m", stdout);
		}
		else if (S_ISDIR(mode))
		{
			if ((mode & 0777) == 0777)
				fputs("\033[0;34;42m", stdout);
			else
				fputs("\033[1;34m", stdout);
		}
		else if (S_ISSOCK(mode))
		{
			fputs("\033[1;35m", stdout);
		}
		else if (S_ISFIFO(mode))
		{
			fputs("\033[0;33m", stdout);
		}
		else if (S_ISBLK(mode) || S_ISCHR(mode))
		{
			fputs("\033[1;33m", stdout);
		}
		else if (mode & S_ISUID)
		{
			fputs("\033[0;37;41m", stdout);
		}
		else if (mode & S_ISGID)
		{
			fputs("\033[0;30;43m", stdout);
		}
		else if (mode & (S_IXUSR | S_IXGRP | S_IXOTH))
		{
			fputs("\033[1;32m", stdout);
		}
	}
	if (env->opt & OPT_Q)
		putchar('"');
	if (env->opt & OPT_q)
	{
		for (size_t i = 0; name[i]; ++i)
			putchar(isgraph(name[i]) ? name[i] : '?');
	}
	else if (env->opt & OPT_b)
	{
		for (size_t i = 0; name[i]; ++i)
		{
			if (isgraph(name[i]))
				putchar(name[i]);
			else
				printf("\\%03o", (unsigned char)name[i]);
		}
	}
	else
	{
		fputs(name, stdout);
	}
	if (env->opt & OPT_Q)
		putchar('"');
	if (mode != (mode_t)-1)
	{
		fputs("\033[0m", stdout);
		if (env->opt & OPT_F)
		{
			if (S_ISLNK(mode))
				putchar('@');
			else if (S_ISDIR(mode))
				putchar('/');
			else if (S_ISSOCK(mode))
				putchar('=');
			else if (S_ISFIFO(mode))
				putchar('|');
			else if (mode & (S_IXUSR | S_IXGRP | S_IXOTH))
				putchar('*');
		}
		else if ((env->opt & OPT_p) && S_ISDIR(mode))
		{
			putchar('/');
		}
	}
}

void
print_file(struct env *env, struct file *file, struct dir *dir, int last)
{
	env->printed_file = 1;
	if (env->opt & OPT_i)
		printf("%*s ", dir->ino_len, file->ino);
	if (env->opt & OPT_s)
		printf("%*s ", dir->blocks_len, file->blocks);
	if (env->opt & OPT_l)
	{
		printf("%s %*s", file->perms, dir->links_len, file->links);
		if (!(env->opt & OPT_G))
			printf(" %-*s", dir->user_len, file->user);
		if (!(env->opt & OPT_o))
			printf(" %-*s", dir->group_len, file->group);
		printf(" %*s %*s ", dir->size_len, file->size, dir->date_len, file->date);
	}
	print_name(env, file->name, file->mode);
	if ((env->opt & OPT_l) && file->lnk_name)
	{
		printf(" -> ");
		print_name(env, file->lnk_name, file->lnk_mode);
	}
	if ((env->opt & (OPT_l | OPT_1)) || last)
		putchar('\n');
	else if (env->opt & OPT_m)
		fputs(", ", stdout);
	else
		putchar(' ');
}

void
print_subdirs(struct env *env, struct dir *dir)
{
	struct file *lst, *nxt;
	TAILQ_FOREACH_SAFE(lst, &dir->files, chain, nxt)
	{
		if (S_ISDIR(lst->mode))
		{
			if (strcmp(lst->name, ".")
			 && strcmp(lst->name, ".."))
			{
				char tmp[MAXPATHLEN];
				snprintf(tmp, sizeof(tmp), "%s/%s", dir->path, lst->name);
				print_dir(env, tmp, 1, tmp);
			}
		}
		free_file(lst);
	}
}

void
print_dir(struct env *env, const char *path, int is_recur, const char *display_path)
{
	struct dir *dir;
	struct file *lst;

	if (is_recur)
	{
		if (env->printed_file)
			putchar('\n');
		printf("%s:\n", display_path);
	}
	dir = load_dir(env, path);
	if (!dir)
		return;
	if ((env->opt & OPT_l) && !TAILQ_EMPTY(&dir->files))
		printf("total %ld\n", (long)dir->total_links);
	TAILQ_FOREACH(lst, &dir->files, chain)
		print_file(env, lst, dir, !TAILQ_NEXT(lst, chain));
	env->printed_file = 1;
	if (env->opt & OPT_R)
		print_subdirs(env, dir);
	free(dir);
}

void
print_sources(struct env *env, int recur)
{
	struct source *lst;

	TAILQ_FOREACH(lst, &env->sources, chain)
		print_dir(env, lst->path, recur, lst->display_path);
}
