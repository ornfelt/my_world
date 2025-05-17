#include "_dirent.h"

#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct dirent *
readdir(DIR *dirp)
{
	struct sys_dirent *sysd;
	struct dirent *dirent;
	size_t namesize;
	size_t reclen;

	if (dirp->buf_pos == dirp->buf_end)
	{
		ssize_t ret;

		dirp->buf_pos = 0;
		dirp->buf_end = 0;
		ret = getdents(dirp->fd,
		               (struct sys_dirent*)dirp->buf,
		               sizeof(dirp->buf));
		if (ret <= 0)
			return NULL;
		dirp->buf_end = ret;
	}
	sysd = (struct sys_dirent*)&dirp->buf[dirp->buf_pos];
	namesize = sysd->reclen - offsetof(struct sys_dirent, name);
	reclen = offsetof(struct dirent, d_name) + namesize + 1;
	if (reclen < sizeof(struct dirent))
		reclen = sizeof(struct dirent);
	dirent = realloc(dirp->dirent, reclen);
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
