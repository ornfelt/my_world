#ifndef _STDIO_H
#define _STDIO_H

#include <sys/queue.h>

#include <stdio.h>

#include "../_lock.h"

struct FILE
{
	cookie_io_functions_t io_funcs;
	int (*flush)(FILE *fp);
	void *cookie;
	int fd;
	int mode;
	int wide;
	char *buf; /* XXX should be a ringbuf */
	size_t buf_size;
	size_t buf_pos;
	int buf_owned;
	int buf_mode;
	int buf_type; /* 1 (write), 0 (read) */
	int eof;
	int err;
	int unget;
	struct _libc_lock lock;
	TAILQ_ENTRY(FILE) chain;
};

struct popen_data
{
	int fds[2];
	pid_t pid;
	FILE *fp;
};

ssize_t io_read(void *cookie, char *buf, size_t size);
ssize_t io_write(void *cookie, const char *buf, size_t size);
int io_seek(void *cookie, off_t off, int whence);
int io_close(void *cookie);

void initfp(FILE *fp);
FILE *mkfp(void);
int parse_flags(const char *mode, int *flags);

size_t write_data(FILE *fp, const void *data, size_t count);

extern TAILQ_HEAD(files_head, FILE) files;

#endif
