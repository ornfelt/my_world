#include <ioctl.h>
#include <proc.h>
#include <file.h>
#include <tty.h>
#include <vfs.h>
#include <sma.h>
#include <uio.h>
#include <std.h>
#include <cpu.h>
#include <mem.h>

struct pty
{
	struct tty *tty;
	refcount_t refcount;
	struct pipebuf pipebuf; /* from pts to ptmx, if that makes sense */
	struct mutex mutex;
	struct waitq rwaitq;
	struct waitq wwaitq;
	int lock;
	int id;
	TAILQ_ENTRY(pty) chain;
};

static struct cdev *ptmx;
static struct sma pty_sma;
static struct spinlock ptys_lock = SPINLOCK_INITIALIZER(); /* XXX rwlock */
static TAILQ_HEAD(, pty) ptys = TAILQ_HEAD_INITIALIZER(ptys);

void pty_init_sma(void)
{
	sma_init(&pty_sma, sizeof(struct pty), NULL, NULL, "pty");
}

static ssize_t pty_tty_write(struct tty *tty, const void *data, size_t len)
{
	struct pty *pty = tty->userdata;
	struct uio uio;
	struct iovec iov;
	uio_fromkbuf(&uio, &iov, (void*)data, len, 0);
	return pipebuf_write(&pty->pipebuf, &uio, 1, NULL);
}

static int pty_ioctl(struct pty *pty, unsigned long req, uintptr_t data)
{
	struct thread *thread = curcpu()->thread;
	switch (req)
	{
		case TIOCSPTLCK:
		{
			int v;
			int ret = vm_copyin(thread->proc->vm_space,
			                    &v, (int*)data, sizeof(v));
			if (ret)
				return ret;
			pty->lock = !!v;
			return 0;
		}
		case TIOCGPTLCK:
		{
			int v = pty->lock;
			return vm_copyout(thread->proc->vm_space,
			                  (int*)data, &v, sizeof(v));
		}
		case TIOCGPTN:
			return vm_copyout(thread->proc->vm_space,
			                  (int*)data, &pty->id, sizeof(int));
		case TIOCGPTPEER:
		{
			/* XXX verify flags */
			struct file *file;
			int ret = file_fromcdev(pty->tty->cdev, data, &file);
			if (ret)
				return ret;
			int fd = proc_allocfd(thread->proc, file,
			                      data & O_CLOEXEC);
			file_free(file);
			return fd;
		}
	}
	return -ENOSYS;
}

static int pty_tty_ioctl(struct tty *tty, unsigned long req, uintptr_t data)
{
	struct pty *pty = tty->userdata;
	return pty_ioctl(pty, req, data);
}

static const struct tty_op pty_tty_op =
{
	.write = pty_tty_write,
	.ioctl = pty_tty_ioctl,
};

static int find_id(void)
{
	for (int i = 0; i < 256; ++i)
	{
		struct pty *pty;
		TAILQ_FOREACH(pty, &ptys, chain)
		{
			if (pty->id == i)
				break;
		}
		if (!pty)
			return i;
	}
	return -1;
}

static void pty_free(struct pty *pty)
{
	if (!pty)
		return;
	if (refcount_dec(&pty->refcount))
		return;
	spinlock_lock(&ptys_lock);
	if (refcount_get(&pty->refcount))
	{
		spinlock_unlock(&ptys_lock);
		return;
	}
	TAILQ_REMOVE(&ptys, pty, chain);
	spinlock_unlock(&ptys_lock);
	mutex_destroy(&pty->mutex);
	waitq_destroy(&pty->rwaitq);
	waitq_destroy(&pty->wwaitq);
	pipebuf_destroy(&pty->pipebuf);
	tty_free(pty->tty);
	sma_free(&pty_sma, pty);
}

static int ptmx_open(struct file *file, struct node *node)
{
	(void)node;
	int ret;
	struct pty *pty = sma_alloc(&pty_sma, M_ZERO);
	spinlock_lock(&ptys_lock);
	pty->id = find_id();
	if (pty->id == -1)
	{
		spinlock_unlock(&ptys_lock);
		return -ENOMEM;
	}
	TAILQ_INSERT_TAIL(&ptys, pty, chain);
	spinlock_unlock(&ptys_lock);
	refcount_init(&pty->refcount, 1);
	mutex_init(&pty->mutex, 0);
	waitq_init(&pty->rwaitq);
	waitq_init(&pty->wwaitq);
	ret = pipebuf_init(&pty->pipebuf, PIPE_BUF * 2, &pty->mutex,
	                   &pty->rwaitq, &pty->wwaitq);
	if (ret)
		goto err;
	pty->pipebuf.nreaders = 1;
	pty->pipebuf.nwriters = 1;
	char name[64];
	snprintf(name, sizeof(name), "pts/%d", pty->id);
	ret = tty_alloc(name, makedev(136, pty->id), &pty_tty_op, &pty->tty);
	if (ret)
		goto err;
	pty->tty->userdata = pty;
	pty->tty->flags |= TTY_NOCTRL;
	file->userdata = pty;
	return 0;

err:
	pty_free(pty);
	return ret;
}

static ssize_t ptmx_read(struct file *file, struct uio *uio)
{
	struct pty *pty = file->userdata;
	return pipebuf_read(&pty->pipebuf, uio,
	                    file->flags & O_NONBLOCK ? 0 : uio->count,
	                    NULL);
}

static ssize_t ptmx_write(struct file *file, struct uio *uio)
{
	struct pty *pty = file->userdata;
	return tty_write(pty->tty, uio);
}

static int ptmx_ioctl(struct file *file, unsigned long req, uintptr_t data)
{
	struct pty *pty = file->userdata;
	int ret = pty_ioctl(pty, req, data);
	if (ret == -ENOSYS)
		return -EINVAL;
	return ret;
}

static int ptmx_poll(struct file *file, struct poll_entry *entry)
{
	struct pty *pty = file->userdata;
	int ret = pipebuf_poll(&pty->pipebuf, entry->events & ~POLLOUT)
	        | pipebuf_poll(&pty->tty->pipebuf, entry->events & ~POLLIN);
	if (ret)
		return ret;
	entry->file_head = &pty->tty->poll_entries;
	return poller_add(entry);
}

static int ptmx_release(struct file *file)
{
	struct pty *pty = file->userdata;
	pty_free(pty);
	return 0;
}

static const struct file_op ptmx_fop =
{
	.open = ptmx_open,
	.read = ptmx_read,
	.write = ptmx_write,
	.ioctl = ptmx_ioctl,
	.poll = ptmx_poll,
	.release = ptmx_release,
};

int pty_init(void)
{
	return cdev_alloc("ptmx", 0, 0, 0666, makedev(5, 2), &ptmx_fop, &ptmx);
}
