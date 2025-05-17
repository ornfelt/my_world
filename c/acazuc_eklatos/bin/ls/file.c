#include "ls.h"

#include <sys/param.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>

time_t
file_time(const struct env *env, const struct stat *st)
{
	if (env->opt & OPT_u)
		return st->st_atime;
	else if (env->opt & OPT_c)
		return st->st_ctime;
	return st->st_mtime;
}

void
free_file(struct file *file)
{
	if (!file)
		return;
	free(file->lnk_name);
	free(file->name);
	free(file);
}

static void
load_date(struct file *file, struct env *env, const struct stat *st)
{
	char *raw_time;
	time_t current_time;
	time_t ftime;
	long delta_time;
	size_t raw_len;

	ftime = file_time(env, st);
	raw_time = ctime(&ftime);
	if (!raw_time)
	{
		snprintf(file->date, sizeof(file->date), "???");
		return;
	}
	raw_len = strlen(raw_time);
	current_time = time(NULL);
	delta_time = current_time - ftime;
	if (delta_time <=  60 * 60 * 24 * 30 * 6
	 && delta_time >= -60 * 60 * 24 * 30 * 6)
		snprintf(file->date, sizeof(file->date), "%.12s", raw_time + 4);
	else
		snprintf(file->date, sizeof(file->date), "%6.6s %.*s",
		         raw_time + 4, 5, raw_time + raw_len - 6);
}

static char
get_perm_0(mode_t mode)
{
	if (S_ISLNK(mode))
		return 'l';
	else if (S_ISSOCK(mode))
		return 's';
	else if (S_ISDIR(mode))
		return 'd';
	else if (S_ISCHR(mode))
		return 'c';
	else if (S_ISBLK(mode))
		return 'b';
	else if (S_ISFIFO(mode))
		return 'p';
	return '-';
}

static char
get_perm_3(mode_t mode)
{
	if (mode & S_ISUID)
		return mode & S_IXUSR ? 's' : 'S';
	else
		return mode & S_IXUSR ? 'x' : '-';
}

static char
get_perm_6(mode_t mode)
{
	if (mode & S_ISGID)
		return mode & S_IXGRP ? 's' : 'S';
	else
		return mode & S_IXGRP ? 'x' : '-';
}

static char
get_perm_9(mode_t mode)
{
	if (mode & S_ISVTX)
		return mode & S_IXOTH ? 't' : 'T';
	else
		return mode & S_IXOTH ? 'x' : '-';
}

static void
load_perms(struct file *file, mode_t mode)
{
	file->perms[0] = get_perm_0(mode);
	file->perms[1] = mode & S_IRUSR ? 'r' : '-';
	file->perms[2] = mode & S_IWUSR ? 'w' : '-';
	file->perms[3] = get_perm_3(mode);
	file->perms[4] = mode & S_IRGRP ? 'r' : '-';
	file->perms[5] = mode & S_IWGRP ? 'w' : '-';
	file->perms[6] = get_perm_6(mode);
	file->perms[7] = mode & S_IROTH ? 'r' : '-';
	file->perms[8] = mode & S_IWOTH ? 'w' : '-';
	file->perms[9] = get_perm_9(mode);
	file->perms[10] = '\0';
}

static void
load_size_n(struct file *file, unsigned v, const char *unit)
{
	if (v < 100)
		snprintf(file->size, sizeof(file->size), "%u.%u%s",
		         v / 10, v % 10, unit);
	else
		snprintf(file->size, sizeof(file->size), "%u%s", v / 10, unit);
}

static void
load_size(struct env *env, struct file *file, const struct stat *st)
{
	if (!(env->opt & OPT_h))
	{
		snprintf(file->size, sizeof(file->size), "%ld", (long)st->st_size);
		return;
	}
#if __SIZEOF_LONG__ == 8
	if (st->st_size >= 1000000000000000000LL)
		load_size_n(file, st->st_size / 100000000000000000ULL, "E");
	else if (st->st_size >= 1000000000000000LL)
		load_size_n(file, st->st_size / 100000000000000ULL, "P");
	else if (st->st_size >= 1000000000000LL)
		load_size_n(file, st->st_size / 100000000000ULL, "T");
#endif
	if (st->st_size >= 1000000000LL)
		load_size_n(file, st->st_size / 100000000ULL, "G");
	else if (st->st_size >= 1000000LL)
		load_size_n(file, st->st_size / 100000ULL, "M");
	else if (st->st_size >= 1000LL)
		load_size_n(file, st->st_size / 100ULL, "K");
	else
		snprintf(file->size, sizeof(file->size), "%ld", (long)st->st_size);
}

static void
setinfos(struct env *env, struct file *file, const struct stat *st)
{
	if (env->opt & OPT_n)
	{
		snprintf(file->user, sizeof(file->user), "%ld",
		         (long)st->st_uid);
		snprintf(file->group, sizeof(file->group), "%ld",
		         (long)st->st_gid);
	}
	else
	{
		struct passwd *pw = getpwuid(st->st_uid);
		struct group *gr = getgrgid(st->st_gid);
		if (pw && pw->pw_name)
			strlcpy(file->user, pw->pw_name, sizeof(file->user));
		else
			snprintf(file->user, sizeof(file->user), "%ld",
			         (long)st->st_uid);
		if (gr && gr->gr_name)
			strlcpy(file->group, gr->gr_name, sizeof(file->group));
		else
			snprintf(file->group, sizeof(file->group), "%ld",
			         (long)st->st_gid);
	}
	load_perms(file, st->st_mode);
	if (file->perms[0] == 'c' || file->perms[0] == 'b')
		snprintf(file->size, sizeof(file->size), "%3d, %3d",
		         (int)major(st->st_rdev), (int)minor(st->st_rdev));
	else
		load_size(env, file, st);
	load_date(file, env, st);
}

static void
load_symb(struct env *env,
          struct file *file,
          const char *name,
          const char *rpath,
          const struct stat *st,
          struct dir *dir)
{
	struct stat lst;
	size_t len;
	ssize_t r;
	char *linkname;

	if (st->st_size)
		len = st->st_size + 1;
	else
		len = 4096;
	linkname = malloc(len);
	if (!linkname)
	{
		fprintf(stderr, "%s: malloc: %s\n",
		        env->progname,
		        strerror(errno));
		exit(EXIT_FAILURE);
	}
	r = readlink(rpath, linkname, len);
	if (r == -1)
	{
		fprintf(stderr, "%s: readlink(%s): %s\n",
		        env->progname,
		        rpath,
		        strerror(errno));
		exit(EXIT_FAILURE);
	}
	if ((size_t)r >= len)
		r = len - 1;
	linkname[r] = '\0';
	file->lnk_name = linkname;
	r = fstatat(dir->dir ? dirfd(dir->dir) : AT_FDCWD,
	            dir->dir ? name : rpath, &lst, 0);
	if (r == -1)
		file->lnk_mode = (mode_t)-1;
	else
		file->lnk_mode = lst.st_mode;
}

int
load_file(struct env *env,
          struct file *file,
          const char *name,
          struct dir *dir)
{
	struct stat st;
	char path[MAXPATHLEN];

	file->name = strdup(name);
	if (name[0] == '/')
		strlcpy(path, name, sizeof(path));
	else
		snprintf(path, sizeof(path), "%s/%s", dir->path, name);
	if (env->opt & OPT_L)
	{
		if (fstatat(dir->dir ? dirfd(dir->dir) : AT_FDCWD,
		            dir->dir ? name : path, &st, 0) == -1)
			return 1; /* XXX fill with ??? */
	}
	else
	{
		if (fstatat(dir->dir ? dirfd(dir->dir) : AT_FDCWD,
		            dir->dir ? name : path, &st, AT_SYMLINK_NOFOLLOW) == -1)
			return 1; /* XXX fill with ??? */
	}
	snprintf(file->links, sizeof(file->links), "%ld", (long)st.st_nlink);
	snprintf(file->ino, sizeof(file->ino), "%lld", (long long)st.st_ino);
	snprintf(file->blocks, sizeof(file->blocks), "%ld", (long)st.st_blocks);
	file->sort_size = st.st_size;
	file->sort_date = file_time(env, &st);
	file->mode = st.st_mode;
	if (S_ISLNK(st.st_mode))
		load_symb(env, file, name, path, &st, dir);
	else
		file->lnk_name = NULL;
	if (env->opt & OPT_l)
		setinfos(env, file, &st);
	if (env->opt & OPT_l)
		dir->total_links += st.st_blocks;
	return 0;
}
