#include <sys/stat.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

static int realpath_addpath(char *dst, const char *add);

static int realpath_adddir(char *dst, const char *dir)
{
	if (!strcmp(dir, "."))
		return 1;
	if (!strcmp(dir, ".."))
	{
		char *prv = strrchr(dst, '/');
		if (!prv) /* can't happend, but still */
			return 0;
		if (prv == dst)
			prv[1] = '\0';
		else
			*prv = '\0';
		return 1;
	}
	if (dst[1])
		strlcat(dst, "/", PATH_MAX);
	strlcat(dst, dir, PATH_MAX);
	struct stat st;
	if (lstat(dst, &st) == -1)
		return 0;
	if (!S_ISLNK(st.st_mode))
		return 1;
	char linkpath[PATH_MAX];
	ssize_t ret = readlink(dst, linkpath, sizeof(linkpath));
	if (ret == -1)
		return 0;
	linkpath[ret] = '\0';
	if (!realpath_adddir(dst, ".."))
		return 0;
	realpath_addpath(dst, linkpath);
	return 1;
}

static int realpath_addpath(char *dst, const char *add)
{
	while (*add == '/')
		add++;
	const char *prv = add;
	while (*prv)
	{
		char dir[PATH_MAX];
		const char *cur = strchrnul(prv, '/');
		snprintf(dir, sizeof(dir), "%.*s", (int)(cur - prv), prv);
		if (!realpath_adddir(dst, dir))
			return 0;
		while (*cur == '/')
			cur++;
		prv = cur;
	}
	return 1;
}

char *realpath(const char *path, char *resolved_path)
{
	if (!path)
	{
		errno = EINVAL;
		return NULL;
	}
	int allocated;
	if (!resolved_path)
	{
		resolved_path = malloc(PATH_MAX);
		if (!resolved_path)
			return NULL;
		allocated = 1;
	}
	else
	{
		allocated = 0;
	}
	resolved_path[0] = '/';
	resolved_path[1] = '\0';
	if (*path != '/')
	{
		char cwd[PATH_MAX];
		if (!getcwd(cwd, sizeof(cwd)))
			goto err;
		if (!realpath_addpath(resolved_path, cwd))
			goto err;
	}
	realpath_addpath(resolved_path, path);
	return resolved_path;

err:
	if (allocated)
		free(resolved_path);
	return NULL;
}
