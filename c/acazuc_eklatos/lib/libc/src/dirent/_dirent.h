#ifndef _DIRENT_H
#define _DIRENT_H

struct dirent;

struct DIR
{
	int fd;
	char buf[4096];
	unsigned long buf_pos;
	unsigned long buf_end;
	struct dirent *dirent;
};

#endif
