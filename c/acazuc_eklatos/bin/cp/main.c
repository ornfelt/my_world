#include <sys/param.h>
#include <sys/stat.h>

#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>

#define OPT_P (1 << 0)
#define OPT_R (1 << 1)
#define OPT_L (1 << 2)
#define OPT_v (1 << 3)

struct env
{
	const char *progname;
	int dst_isdir;
	int opt;
};

static int copy_file(struct env *env, const char *path, const char *dst);

static int
copy_dir(struct env *env, const char *src, const char *dst)
{
	char path[MAXPATHLEN];
	struct dirent *dirent;
	DIR *dir = NULL;
	int ret = 1;

	if (!(env->opt & OPT_R))
	{
		fprintf(stderr, "%s: -R not specified, ignoring '%s'\n",
		        env->progname, src);
		goto end;
	}
	/* XXX mkdir */
	dir = opendir(src);
	if (!dir)
	{
		fprintf(stderr, "%s: opendir(%s): %s\n",
		        env->progname,
		        src,
		        strerror(errno));
		goto end;
	}
	while ((dirent = readdir(dir)))
	{
		if (!strcmp(dirent->d_name, ".")
		 || !strcmp(dirent->d_name, ".."))
			continue;
		if (snprintf(path, sizeof(path), "%s/%s", src, dirent->d_name) >= (int)sizeof(path))
		{
			fprintf(stderr, "%s: path too long\n", env->progname);
			goto end;
		}
		/* XXX dst should be updated if recursive */
		if (copy_file(env, path, dst))
			goto end;
	}
	ret = 0;

end:
	if (dir)
		closedir(dir);
	return ret;
}

static int
copy_reg(struct env *env, const char *src, const char *dst, struct stat *st)
{
	int fdsrc = -1;
	int fddst = -1;
	int ret = 0;

	if (env->opt & OPT_v)
		printf("copying '%s' to '%s'\n", src, dst);
	fdsrc = open(src, O_RDONLY);
	if (fdsrc == -1)
	{
		fprintf(stderr, "%s: open(%s): %s\n",
		        env->progname,
		        src,
		        strerror(errno));
		goto end;
	}
	fddst = open(dst, O_WRONLY | O_TRUNC | O_CREAT, st->st_mode);
	if (fddst == -1)
	{
		fprintf(stderr, "%s: open(%s): %s\n",
		        env->progname,
		        dst,
		        strerror(errno));
		goto end;
	}
	while (1)
	{
		static char buf[1024 * 1024];
		ssize_t wr;
		ssize_t n;

		n = read(fdsrc, buf, sizeof(buf));
		if (n == -1)
		{
			fprintf(stderr, "%s: read(%s): %s\n",
			        env->progname,
			        src,
			        strerror(errno));
			goto end;
		}
		if (!n)
			break;
		wr = 0;
		while (wr < n)
		{
			ssize_t w = write(fddst, buf, n - wr);
			if (w == -1)
			{
				fprintf(stderr, "%s: write(%s): %s\n",
				        env->progname,
				        dst,
				        strerror(errno));
				goto end;
			}
			wr += w;
		}
	}
	ret = 0;

end:
	if (fdsrc != -1)
		close(fdsrc);
	if (fddst != -1)
		close(fddst);
	return ret;
}

static int
copy_lnk(struct env *env, const char *src, const char *dst)
{
	char target[MAXPATHLEN];
	ssize_t ret;

	if (env->opt & OPT_v)
		printf("copying '%s' to '%s'\n", src, dst);
	ret = readlink(src, target, sizeof(target));
	if (ret == -1)
	{
		fprintf(stderr, "%s: readlink(%s): %s\n",
		        env->progname,
		        src,
		        strerror(errno));
		return 1;
	}
	if (ret >= (ssize_t)sizeof(target))
	{
		fprintf(stderr, "%s: symbolic link target too long\n",
		        env->progname);
		return 1;
	}
	target[ret] = '\0';
	if (symlink(target, dst) == -1)
	{
		fprintf(stderr, "%s: symlink(%s, %s): %s\n",
		        env->progname,
		        target,
		        dst,
		        strerror(errno));
		return 1;
	}
	return 0;
}

static int
copy_file(struct env *env, const char *file, const char *dst)
{
	char path[MAXPATHLEN];
	struct stat st;
	size_t file_len;
	const char *begin;
	const char *end;

	file_len = strlen(file);
	end = file + file_len;
	while (end >= file && *end == '/')
		end--;
	if (end <= file)
	{
		fprintf(stderr, "%s: invalid operand\n", env->progname);
		return 1;
	}
	begin = end;
	while (begin > file && *begin != '/')
		begin--;
	if (*begin == '/')
		begin++;
	if (env->dst_isdir)
		snprintf(path, sizeof(path), "%s/%.*s", dst, (int)(end - begin), begin);
	else
		strlcpy(path, dst, sizeof(path));
	if (env->opt & OPT_P)
	{
		if (lstat(file, &st) == -1)
		{
			fprintf(stderr, "%s: lstat(%s): %s\n",
			        env->progname,
			        file,
			        strerror(errno));
			return 1;
		}
	}
	else
	{
		if (stat(file, &st) == -1)
		{
			fprintf(stderr, "%s: stat(%s): %s\n",
			        env->progname,
			        file,
			        strerror(errno));
			return 1;
		}
	}
	if (S_ISDIR(st.st_mode))
		return copy_dir(env, file, path);
	if (S_ISREG(st.st_mode))
		return copy_reg(env, file, path, &st);
	if (S_ISLNK(st.st_mode))
		return copy_lnk(env, file, path);
	if (S_ISCHR(st.st_mode) || S_ISBLK(st.st_mode) || S_ISFIFO(st.st_mode))
	{
		if (env->opt & OPT_v)
			printf("copying '%s' to '%s'\n", file, path);
		if (mknod(path, st.st_mode, st.st_dev) == -1)
		{
			fprintf(stderr, "%s: mknod(%s): %s\n",
			        env->progname,
			        path,
			        strerror(errno));
			return 1;
		}
	}
	else
	{
		fprintf(stderr, "%s: unknown source file type", env->progname);
		return 1;
	}
	return 0;
}

static void
usage(const char *progname)
{
	printf("%s [-r] [-R] [-P] [-L] [-v] FILES DIRECTORY\n", progname);
	printf("-r: synonym of -R\n");
	printf("-R: enable recursive copy\n");
	printf("-P: don't follow symbolic links in sources\n");
	printf("-L: follow symbolic links in sources\n");
	printf("-v: verbose operations\n");
}

int
main(int argc, char **argv)
{
	const char *dst;
	struct stat st;
	struct env env;
	int c;

	memset(&env, 0, sizeof(env));
	env.progname = argv[0];
	env.opt = OPT_P;
	while ((c = getopt(argc, argv, "PrRLvfin")) != -1)
	{
		switch (c)
		{
			case 'P':
				env.opt |= OPT_P;
				env.opt &= ~OPT_L;
				break;
			case 'r':
			case 'R':
				env.opt |= OPT_R;
				break;
			case 'L':
				env.opt |= OPT_L;
				env.opt &= ~OPT_P;
				break;
			case 'v':
				env.opt |= OPT_v;
				break;
			default:
				usage(argv[0]);
				return EXIT_FAILURE;
		}
	}
	dst = argv[argc - 1];
	if (stat(dst, &st) == -1)
	{
		if (errno == ENOENT)
		{
			env.dst_isdir = 0;
		}
		else
		{
			fprintf(stderr, "%s: stat(%s): %s\n",
			        argv[0],
			        dst,
			        strerror(errno));
			return EXIT_FAILURE;
		}
	}
	else
	{
		env.dst_isdir = S_ISDIR(st.st_mode);
	}
	if (argc - optind > 2 && !env.dst_isdir)
	{
		fprintf(stderr, "%s: can't move multiple files to non-directory file\n",
		        argv[0]);
		return EXIT_FAILURE;
	}
	for (int i = optind; i < argc - 1; ++i)
	{
		if (copy_file(&env, argv[i], argv[argc - 1]))
			return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
