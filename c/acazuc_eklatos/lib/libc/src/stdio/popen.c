#include "_stdio.h"

#include <sys/wait.h>

#include <unistd.h>
#include <stdio.h>
#include <errno.h>

static ssize_t
read_fn(void *cookie, char *buf, size_t size)
{
	struct popen_data *data = cookie;
	return read(data->fds[0], buf, size);
}

static ssize_t
write_fn(void *cookie, const char *buf, size_t size)
{
	struct popen_data *data = cookie;
	return write(data->fds[1], buf, size);
}

static int
close_fn(void *cookie)
{
	struct popen_data *data = cookie;
	if (waitpid(data->pid, NULL, 0) == -1)
		errno = ECHILD;
	if ((data->fp->mode & 3) == O_RDONLY)
		close(data->fds[0]);
	else
		close(data->fds[1]);
	free(data);
	return 0;
}

FILE *
popen(const char *cmd, const char *type)
{
	struct popen_data *data;
	int mode = O_RDONLY;
	FILE *fp;
	pid_t pid;

	switch (type[0])
	{
		case 'r':
			mode = O_RDONLY;
			break;
		case 'w':
			mode = O_WRONLY;
			break;
		default:
			errno = EINVAL;
			return NULL;
	}
	if (type[1])
	{
		if (type[1] != 'e')
		{
			errno = EINVAL;
			return NULL;
		}
		mode |= O_CLOEXEC;
	}
	fp = mkfp();
	if (!fp)
		return NULL;
	fp->mode = mode;
	data = malloc(sizeof(*data));
	if (!data)
	{
		free(fp);
		return NULL;
	}
	data->fp = fp;
	fp->cookie = data;
	if ((mode & 3) == O_RDONLY)
		fp->io_funcs.read = read_fn;
	else
		fp->io_funcs.write = write_fn;
	fp->io_funcs.close = close_fn;
	if (pipe2(data->fds, mode & O_CLOEXEC) == -1)
	{
		free(data);
		free(fp);
		return NULL;
	}
	pid = vfork();
	if (pid == -1)
	{
		close(data->fds[0]);
		close(data->fds[1]);
		free(data);
		free(fp);
		return NULL;
	}
	if (pid)
	{
		data->pid = pid;
		if ((fp->mode & 3) == O_RDONLY)
			close(data->fds[1]);
		else
			close(data->fds[0]);
		return fp;
	}
	if ((fp->mode & 3) == O_RDONLY)
	{
		if (dup2(data->fds[1], 1) == -1)
			exit(EXIT_FAILURE);
	}
	else
	{
		if (dup2(data->fds[0], 0) == -1)
			exit(EXIT_FAILURE);
	}
	close(data->fds[0]);
	close(data->fds[1]);
	char * const argv[] =
	{
		"sh",
		"-c",
		(char*)cmd,
		NULL,
	};
	execv("/bin/sh", argv);
	exit(EXIT_FAILURE);
	return NULL;
}
