#include "_dirent.h"

#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct dirent *readdir(DIR *dirp)
{
	if (dirp->buf_pos == dirp->buf_end)
	{
		dirp->buf_pos = 0;
		dirp->buf_end = 0;
		ssize_t res = getdents(dirp->fd, (struct sys_dirent*)dirp->buf,
		                       sizeof(dirp->buf));
		if (res <= 0)
			return NULL;
		dirp->buf_end = res;
	}
	struct sys_dirent *sysd = (struct sys_dirent*)&dirp->buf[dirp->buf_pos];
	size_t namesize = sysd->reclen - offsetof(struct sys_dirent, name);
	size_t reclen = offsetof(struct dirent, d_name) + namesize + 1;
	if (reclen < sizeof(struct dirent))
		reclen = sizeof(struct dirent);
	struct dirent *dirent = realloc(dirp->dirent, reclen);
	if (!dirent)
		return NULL;
	dirent->d_ino = sysd->ino;
	dirent->d_off = sysd->off;
	dirent->d_reclen = reclen;
	dirent->d_type = sysd->type;
	memcpy(dirent->d_name, sysd->name, namesize);
	dirent->d_name[namesize] = '\0';
	dirp->dirent = dirent;
	dirp->buf_pos += sysd->reclen;
	return dirent;
}
