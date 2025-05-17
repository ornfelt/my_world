#include <sys/param.h>

#include <fnmatch.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <glob.h>

static int
push_entry(glob_t *globp, const char *str)
{
	size_t off;
	char **pathv;
	char *dup;

	dup = strdup(str);
	if (!dup)
		return 1;
	off = globp->gl_pathc + globp->gl_offs;
	pathv = realloc(globp->gl_pathv, sizeof(*pathv) * (off + 2));
	if (!pathv)
	{
		free(dup);
		return 1;
	}
	pathv[off] = dup;
	pathv[off + 1] = NULL;
	globp->gl_pathc++;
	globp->gl_pathv = pathv;
	return 0;
}

static int
is_dir(DIR *dir, struct dirent *dirent)
{
	switch (dirent->d_type)
	{
		case DT_DIR:
			return 1;
		case DT_LNK:
		case DT_UNKNOWN:
		{
			struct stat st;
			if (fstatat(dirfd(dir), dirent->d_name, &st, 0) == -1)
				return -1;
			return S_ISDIR(st.st_mode) != 0;
		}
		default:
			return 0;
	}
}

static int
glob_dir(char *buf,
         size_t buf_pos,
         char *pattern,
         int flags,
         int (*errfn)(const char *path, int err),
         glob_t *globp)
{
	struct dirent *dirent;
	DIR *dir;

	while (pattern[0] == '/')
	{
		if (buf_pos + 1 >= MAXPATHLEN)
			return GLOB_NOSPACE;
		buf[buf_pos++] = '/';
		pattern++;
	}
	buf[buf_pos] = '\0';
	if (!pattern[0])
	{
		if (push_entry(globp, buf))
			return GLOB_NOSPACE;
		return 0;
	}
	char *next_pattern = strchrnul(pattern, '/');
	if (next_pattern - pattern >= MAXPATHLEN)
		return GLOB_NOSPACE;
	/* XXX shortcut if no *?[] chars in name */
	char old_sep = *next_pattern;
	dir = opendir(buf_pos ? buf : ".");
	if (!dir)
	{
		if (errfn)
		{
			if (errfn(buf, errno))
				return GLOB_ABORTED;
		}
		if (flags & GLOB_ERR)
			return GLOB_ABORTED;
		return 0;
	}
	int fnmatch_flags = FNM_PERIOD;
	if (flags & GLOB_NOESCAPE)
		fnmatch_flags |= FNM_NOESCAPE;
	while ((dirent = readdir(dir)))
	{
		*next_pattern = '\0';
		if (fnmatch(pattern, dirent->d_name, fnmatch_flags))
		{
			*next_pattern = old_sep;
			continue;
		}
		*next_pattern = old_sep;
		size_t len = strlen(dirent->d_name);
		if (buf_pos + len >= MAXPATHLEN)
		{
			closedir(dir);
			return GLOB_NOSPACE;
		}
		memcpy(&buf[buf_pos], dirent->d_name, len);
		switch (is_dir(dir, dirent))
		{
			case -1:
				if (errfn)
				{
					if (errfn(buf, errno))
						return GLOB_ABORTED;
				}
				if (flags & GLOB_ERR)
					return GLOB_ABORTED;
				continue;
			case 0:
				if (!old_sep)
					break;
				if (errfn)
				{
					if (errfn(buf, ENOTDIR))
						return GLOB_ABORTED;
				}
				if (flags & GLOB_ERR)
					return GLOB_ABORTED;
				continue;
			case 1:
				if (!(flags & GLOB_MARK))
					break;
				if (old_sep == '/')
					break;
				if (buf_pos + len + 1 >= MAXPATHLEN)
				{
					closedir(dir);
					return GLOB_NOSPACE;
				}
				buf[buf_pos + len] = '/';
				len++;
				break;
		}
		buf[buf_pos + len] = '\0';
		int ret = glob_dir(buf, buf_pos + len, next_pattern, flags,
		                   errfn, globp);
		if (ret)
		{
			closedir(dir);
			return ret;
		}
	}
	closedir(dir);
	return 0;
}

static int
str_cmp(const void *a, const void *b)
{
	return strcmp(*(char**)a, *(char**)b);
}

int
glob(const char *pattern,
     int flags,
     int (*errfn)(const char *path, int err),
     glob_t *globp)
{
	char pattern_buf[MAXPATHLEN];
	char buf[MAXPATHLEN];
	int ret;

	if (!(flags & GLOB_APPEND))
	{
		globp->gl_pathc = 0;
		size_t n = 1;
		if (flags & GLOB_DOOFFS)
			n += globp->gl_offs;
		else
			globp->gl_offs = 0;
		globp->gl_pathv = calloc(n, sizeof(*globp->gl_pathv));
		if (!globp->gl_pathv)
			return GLOB_NOSPACE;
	}
	size_t org_pathc = globp->gl_pathc;
	if (strlcpy(pattern_buf, pattern, sizeof(pattern_buf)) >= sizeof(pattern_buf))
		return GLOB_NOSPACE;
	ret = glob_dir(buf, 0, pattern_buf, flags, errfn, globp);
	if (ret)
		return ret;
	if (globp->gl_pathc == org_pathc)
	{
		if (!(flags & GLOB_NOCHECK))
			return GLOB_NOMATCH;
		if (push_entry(globp, pattern))
			return GLOB_NOSPACE;
	}
	if (!(flags & GLOB_NOSORT))
		qsort(&globp->gl_pathv[globp->gl_offs], globp->gl_pathc,
		      sizeof(*globp->gl_pathv), str_cmp);
	return 0;
}
