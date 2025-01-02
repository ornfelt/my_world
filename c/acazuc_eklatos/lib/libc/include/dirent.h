#ifndef DIRENT_H
#define DIRENT_H

#include <sys/types.h>
#include <sys/stat.h>

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DIR DIR;

struct dirent
{
	ino_t d_ino;
	off_t d_off;
	unsigned short d_reclen;
	unsigned char d_type;
	char d_name[256];
};

DIR *opendir(const char *name);
DIR *fdopendir(int fd);
struct dirent *readdir(DIR *dirp);
int closedir(DIR *dirp);
int dirfd(DIR *dirp);
void rewinddir(DIR *dirp);
long telldir(DIR *dirp);
void seekdir(DIR *dirp, long loc);

int alphasort(const struct dirent **a, const struct dirent **b);
int scandir(const char *dirp, struct dirent ***namelist,
            int (*filter)(const struct dirent *),
            int (*cmp)(const struct dirent **, const struct dirent **));

#ifdef __cplusplus
}
#endif

#endif
