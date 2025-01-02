#include <errno.h>
#include <proc.h>
#include <pipe.h>
#include <file.h>
#include <cpu.h>
#include <std.h>
#include <vfs.h>
#include <uio.h>
#include <sma.h>

static struct sma pipe_sma;

void pipe_init(void)
{
	sma_init(&pipe_sma, sizeof(struct pipe), NULL, NULL, "pipe");
}

static void pipe_lock(struct pipe *pipe)
{
	mutex_lock(&pipe->mutex);
}

static void pipe_unlock(struct pipe *pipe)
{
	mutex_unlock(&pipe->mutex);
}

static struct pipe *getpipe(struct file *file)
{
	if (!file->node)
		return NULL;
	if (!S_ISFIFO(file->node->attr.mode))
		return NULL;
	return file->node->pipe;
}

static ssize_t pipe_read(struct file *file, struct uio *uio)
{
	struct pipe *pipe = getpipe(file);
	if (!pipe)
		return -EINVAL;
	ssize_t ret = pipebuf_read(&pipe->pipebuf, uio,
	                           file->flags & O_NONBLOCK ? 0 : uio->count,
	                           NULL);
	if (ret > 0)
		poller_broadcast(&pipe->poll_entries, POLLOUT);
	return ret;
}

static ssize_t pipe_write(struct file *file, struct uio *uio)
{
	struct pipe *pipe = getpipe(file);
	if (!pipe)
		return -EINVAL;
	ssize_t ret = pipebuf_write(&pipe->pipebuf, uio,
	                            file->flags & O_NONBLOCK ? 0 : uio->count,
	                            NULL);
	if (ret > 0)
		poller_broadcast(&pipe->poll_entries, POLLIN);
	return ret;
}

static int pipe_release(struct file *file)
{
	struct pipe *pipe = getpipe(file);
	if (!pipe)
		return 0;
	pipe_lock(pipe);
	switch (file->flags & 3)
	{
		case O_RDONLY:
			pipe->pipebuf.nreaders--;
			if (!pipe->pipebuf.nreaders)
			{
				waitq_broadcast(&pipe->wwaitq, 0);
				poller_broadcast(&pipe->poll_entries, POLLERR);
			}
			break;
		case O_WRONLY:
			pipe->pipebuf.nwriters--;
			if (!pipe->pipebuf.nwriters)
			{
				waitq_broadcast(&pipe->rwaitq, 0);
				poller_broadcast(&pipe->poll_entries, POLLHUP);
			}
			break;
		case O_RDWR:
			pipe->pipebuf.nreaders--;
			if (!pipe->pipebuf.nreaders)
			{
				waitq_broadcast(&pipe->wwaitq, 0);
				poller_broadcast(&pipe->poll_entries, POLLERR);
			}
			pipe->pipebuf.nwriters--;
			if (!pipe->pipebuf.nwriters)
			{
				waitq_broadcast(&pipe->rwaitq, 0);
				poller_broadcast(&pipe->poll_entries, POLLHUP);
			}
			break;
	}
	pipe_unlock(pipe);
	return 0;
}

static int pipe_fopen(struct file *file, struct node *node)
{
	(void)node;
	struct pipe *pipe = getpipe(file);
	if (!pipe)
		return -EINVAL;
	pipe_lock(pipe);
	switch (file->flags & 3)
	{
		case O_RDONLY:
			pipe->pipebuf.nreaders++;
			break;
		case O_WRONLY:
			pipe->pipebuf.nwriters++;
			break;
		case O_RDWR:
			pipe->pipebuf.nreaders++;
			pipe->pipebuf.nwriters++;
			break;
		default:
			pipe_unlock(pipe);
			return -EINVAL;
	}
	pipe_unlock(pipe);
	return 0;
}

static off_t pipe_seek(struct file *file, off_t off, int whence)
{
	(void)file;
	(void)off;
	(void)whence;
	return -ESPIPE;
}

static int pipe_poll(struct file *file, struct poll_entry *entry)
{
	struct pipe *pipe = getpipe(file);
	if (!pipe)
		return -EINVAL;
	int ret = pipebuf_poll(&pipe->pipebuf, entry->events);
	if (ret)
		return ret;
	entry->file_head = &pipe->poll_entries;
	return poller_add(entry);
}

void pipe_free(struct pipe *pipe)
{
	pipebuf_destroy(&pipe->pipebuf);
	mutex_destroy(&pipe->mutex);
	waitq_destroy(&pipe->rwaitq);
	waitq_destroy(&pipe->wwaitq);
	sma_free(&pipe_sma, pipe);
}

const struct file_op g_pipe_fop =
{
	.open = pipe_fopen,
	.read = pipe_read,
	.write = pipe_write,
	.seek = pipe_seek,
	.poll = pipe_poll,
	.release = pipe_release,
};

int pipe_alloc(struct pipe **pipep, struct node *node)
{
	int ret;
	struct pipe *pipe = sma_alloc(&pipe_sma, M_ZERO);
	if (!pipe)
	{
		return -ENOMEM;
	}
	mutex_init(&pipe->mutex, 0);
	waitq_init(&pipe->rwaitq);
	waitq_init(&pipe->wwaitq);
	ret = pipebuf_init(&pipe->pipebuf, PIPE_BUF * 2, &pipe->mutex,
	                   &pipe->rwaitq, &pipe->wwaitq);
	if (ret)
	{
		mutex_destroy(&pipe->mutex);
		waitq_destroy(&pipe->rwaitq);
		waitq_destroy(&pipe->wwaitq);
		sma_free(&pipe_sma, pipe);
		return ret;
	}
	TAILQ_INIT(&pipe->poll_entries);
	pipe->node = node;
	*pipep = pipe;
	return 0;
}

static int fifo_release(struct node *node)
{
	pipe_free(node->pipe);
	node->pipe = NULL;
	return 0;
}

static int fifo_setattr(struct node *node, fs_attr_mask_t mask,
                        const struct fs_attr *attr)
{
	if (mask & FS_ATTR_SIZE)
		return -EINVAL;
	if (mask & FS_ATTR_ATIME)
		node->attr.atime = attr->atime;
	if (mask & FS_ATTR_MTIME)
		node->attr.mtime = attr->mtime;
	if (mask & FS_ATTR_CTIME)
		node->attr.ctime = attr->ctime;
	if (mask & FS_ATTR_UID)
		node->attr.uid = attr->uid;
	if (mask & FS_ATTR_GID)
		node->attr.gid = attr->gid;
	if (mask & FS_ATTR_MODE)
		node->attr.mode = attr->mode;
	return 0;
}

static const struct node_op fifo_op =
{
	.release = fifo_release,
	.setattr = fifo_setattr,
};

static int pipe_open(struct pipe *pipe, int flags, struct file **file)
{
	int ret = file_fromnode(pipe->node, flags, file);
	if (ret)
		return ret;
	ret = file_open(*file, pipe->node);
	if (ret)
	{
		file_free(*file);
		return ret;
	}
	return 0;
}

int fifo_alloc(struct pipe **pipep, struct node **nodep,
               struct file **rfilep, struct file **wfilep)
{
	struct pipe *pipe;
	struct file *rfile;
	struct file *wfile;
	struct thread *thread = curcpu()->thread;
	struct node *node = malloc(sizeof(*node), M_ZERO);
	if (!node)
		return -ENOMEM;
	node->sb = NULL;
	node->attr.uid = thread->proc->cred.euid;
	node->attr.gid = thread->proc->cred.egid;
	node->attr.mode = S_IFIFO | 0644;
	/* XXX atime, mtime, ctime, ino, blksize, blocks, nlink, rdev */
	node->op = &fifo_op;
	node->fop = &g_pipe_fop;
	refcount_init(&node->refcount, 1);
	int ret = pipe_alloc(&pipe, node);
	if (ret)
	{
		free(node);
		return ret;
	}
	node->pipe = pipe;
	ret = pipe_open(pipe, O_RDONLY, &rfile);
	if (ret)
	{
		node_free(node);
		return ret;
	}
	ret = pipe_open(pipe, O_WRONLY, &wfile);
	if (ret)
	{
		file_free(rfile);
		node_free(node);
		return ret;
	}
	node_free(node);
	*pipep = pipe;
	*nodep = node;
	*rfilep = rfile;
	*wfilep = wfile;
	return 0;
}
