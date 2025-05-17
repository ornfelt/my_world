#include <sys/stat.h>

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>

static const char *
uid_str(uid_t uid)
{
	struct passwd *pw = getpwuid(uid);
	if (pw && pw->pw_name)
		return pw->pw_name;
	return "unknown";
}

static const char *
gid_str(gid_t gid)
{
	struct group *gr = getgrgid(gid);
	if (gr && gr->gr_name)
		return gr->gr_name;
	return "unknown";
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

static const char *
mode_str(mode_t mode)
{
	static char buf[11];
	buf[0] = get_perm_0(mode);
	buf[1] = mode & S_IRUSR ? 'r' : '-';
	buf[2] = mode & S_IWUSR ? 'w' : '-';
	buf[3] = get_perm_3(mode);
	buf[4] = mode & S_IRGRP ? 'r' : '-';
	buf[5] = mode & S_IWGRP ? 'w' : '-';
	buf[6] = get_perm_6(mode);
	buf[7] = mode & S_IROTH ? 'r' : '-';
	buf[8] = mode & S_IWOTH ? 'w' : '-';
	buf[9] = get_perm_9(mode);
	buf[10] = '\0';
	return buf;
}

static const char *
time_str(struct timespec *ts)
{
	static char buf[1024];
	struct tm tm;
	size_t len;
	time_t t = ts->tv_sec;

	if (!localtime_r(&t, &tm))
		return NULL;
	if (!strftime(buf, sizeof(buf), "%F %T", &tm))
		return NULL;
	len = strlen(buf);
	snprintf(&buf[len], sizeof(buf) - len, ".%09lld",
	         (long long)ts->tv_nsec);
	return buf;
}

static const char *
type_str(mode_t mode)
{
	if (S_ISREG(mode))
		return "regular file";
	if (S_ISBLK(mode))
		return "block device";
	if (S_ISCHR(mode))
		return "character device";
	if (S_ISDIR(mode))
		return "directory";
	if (S_ISFIFO(mode))
		return "FIFO";
	if (S_ISLNK(mode))
		return "symbolic link";
	if (S_ISSOCK(mode))
		return "socket";
	return "unknown";
}

static int
stat_file(const char *progname, const char *file)
{
	struct stat st;

	if (stat(file, &st) == -1)
	{
		fprintf(stderr, "%s: stat: %s\n", progname, strerror(errno));
		return 1;
	}
	printf("  file: %s\n", file);
	printf("  size: %ld\n", (long)st.st_size);
	printf("blocks: %ld\n", (long)st.st_blocks);
	printf(" blksz: %ld\n", (long)st.st_blksize);
	printf(" inode: %ld\n", (long)st.st_ino);
	printf(" links: %lu\n", (long)st.st_nlink);
	printf("access: %lo (%s)\n", (long)st.st_mode & 07777, mode_str(st.st_mode));
	if (S_ISCHR(st.st_mode) || S_ISBLK(st.st_mode))
		printf("device: %d, %d\n", (int)major(st.st_rdev), (int)minor(st.st_rdev));
	printf("  type: %s\n", type_str(st.st_mode));
	printf("   uid: %ld (%s)\n", (long)st.st_uid, uid_str(st.st_uid));
	printf("   gid: %ld (%s)\n", (long)st.st_gid, gid_str(st.st_gid));
	printf(" atime: %s\n", time_str(&st.st_atim));
	printf(" mtime: %s\n", time_str(&st.st_mtim));
	printf(" ctime: %s\n", time_str(&st.st_ctim));
	return 0;
}

int
main(int argc, char **argv)
{
	if (argc < 2)
	{
		fprintf(stderr, "%s: missing operand\n", argv[0]);
		return EXIT_FAILURE;
	}
	for (int i = 1; i < argc; ++i)
	{
		if (stat_file(argv[0], argv[i]))
			return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
