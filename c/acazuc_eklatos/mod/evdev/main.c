#define ENABLE_TRACE

#include "scancode.h"
#include "evdev.h"

#include <pipebuf.h>
#include <queue.h>
#include <errno.h>
#include <file.h>
#include <pipe.h>
#include <poll.h>
#include <kmod.h>
#include <utf8.h>
#include <vfs.h>
#include <uio.h>
#include <sma.h>
#include <std.h>
#include <tty.h>

struct evdev_queue
{
	struct evdev *evdev;
	struct file *file;
	struct mutex mutex;
	struct waitq rwaitq;
	struct waitq wwaitq;
	struct pipebuf pipebuf;
	TAILQ_ENTRY(evdev_queue) chain;
};

struct evdev
{
	TAILQ_HEAD(, evdev_queue) queues;
	struct spinlock lock; /* XXX rwlock ? */
	struct poller_head poll_entries;
	struct cdev *cdev;
	refcount_t refcount;
	char name[16];
};

static struct sma queue_sma;
static struct sma evdev_sma;
static struct evdev *event0;

int evdev_open(struct file *file, struct node *node);
int evdev_release(struct file *file);
ssize_t evdev_read(struct file *file, struct uio *uio);
ssize_t evdev_write(struct file *file, struct uio *uio);
int evdev_poll(struct file *file, struct poll_entry *entry);

uint32_t get_scancode(enum kbd_key key, enum kbd_mod mods)
{
	if (mods & (KBD_MOD_LSHIFT | KBD_MOD_RSHIFT))
		return scancodes_shift[key];
	if (mods & (KBD_MOD_LCONTROL | KBD_MOD_RCONTROL))
		return scancodes_control[key];
	if (mods & (KBD_MOD_LALT | KBD_MOD_RALT))
		return scancodes_alt[key];
	return scancodes_normal[key];
}

int get_mods_value(uint32_t mods)
{
	int ret = 1;
	if (mods & (KBD_MOD_LSHIFT | KBD_MOD_RSHIFT))
		ret += 1;
	if (mods & (KBD_MOD_LALT | KBD_MOD_RALT))
		ret += 2;
	if (mods & (KBD_MOD_LCONTROL | KBD_MOD_RCONTROL))
		ret += 4;
	if (mods & (KBD_MOD_LMETA | KBD_MOD_RMETA))
		ret += 8;
	return ret;
}

void print_mods_seq(char *s, size_t n, const char *code,
                    const char *seq, enum kbd_mod mods)
{
	char mods_str[8];
	if (mods)
	{
		if (!code)
			code = "1";
		snprintf(mods_str, sizeof(mods_str), ";%d",
		         get_mods_value(mods));
	}
	else
	{
		mods_str[0] = '\0';
	}
	snprintf(s, n, "\033[%s%s%s", code ? code : "", mods_str, seq);
}

int send_tty(struct tty *tty, enum kbd_key key, enum kbd_mod mods)
{
	char utf8[64];
	if (key == KBD_KEY_CURSOR_UP)
	{
		print_mods_seq(utf8, sizeof(utf8), NULL, "A", mods);
	}
	else if (key == KBD_KEY_CURSOR_DOWN)
	{
		print_mods_seq(utf8, sizeof(utf8), NULL, "B", mods);
	}
	else if (key == KBD_KEY_CURSOR_RIGHT)
	{
		print_mods_seq(utf8, sizeof(utf8), NULL, "C", mods);
	}
	else if (key == KBD_KEY_CURSOR_LEFT)
	{
		print_mods_seq(utf8, sizeof(utf8), NULL, "D", mods);
	}
	else if (key == KBD_KEY_END)
	{
		print_mods_seq(utf8, sizeof(utf8), "1", "F", mods);
	}
	else if (key == KBD_KEY_KP_5)
	{
		print_mods_seq(utf8, sizeof(utf8), "1", "G", mods);
	}
	else if (key == KBD_KEY_HOME)
	{
		print_mods_seq(utf8, sizeof(utf8), "1", "H", mods);
	}
	else if (key == KBD_KEY_INSERT)
	{
		print_mods_seq(utf8, sizeof(utf8), "2", "~", mods);
	}
	else if (key == KBD_KEY_DELETE)
	{
		print_mods_seq(utf8, sizeof(utf8), "3", "~", mods);
	}
	else if (key == KBD_KEY_PGUP)
	{
		print_mods_seq(utf8, sizeof(utf8), "5", "~", mods);
	}
	else if (key == KBD_KEY_PGDOWN)
	{
		print_mods_seq(utf8, sizeof(utf8), "6", "~", mods);
	}
	else if (key == KBD_KEY_F1)
	{
		print_mods_seq(utf8, sizeof(utf8), "1", "P", mods);
	}
	else if (key == KBD_KEY_F2)
	{
		print_mods_seq(utf8, sizeof(utf8), "1", "Q", mods);
	}
	else if (key == KBD_KEY_F3)
	{
		print_mods_seq(utf8, sizeof(utf8), "1", "R", mods);
	}
	else if (key == KBD_KEY_F4)
	{
		print_mods_seq(utf8, sizeof(utf8), "1", "S", mods);
	}
	else
	{
		uint32_t scancode = get_scancode(key, mods);
		if (!scancode)
			return 0;
		char *dst = utf8;
		memset(utf8, 0, sizeof(utf8));
		if (!utf8_encode(&dst, scancode))
			return 0;
	}
	size_t count = strlen(utf8);
	tty_input(tty, utf8, count);
	return count;
}

static const struct file_op fop =
{
	.open = evdev_open,
	.release = evdev_release,
	.read = evdev_read,
	.write = evdev_write,
	.poll = evdev_poll,
};

int evdev_alloc(struct evdev **evdevp)
{
	struct evdev *evdev = sma_alloc(&evdev_sma, M_ZERO);
	if (!evdev)
		return -ENOMEM;
	refcount_init(&evdev->refcount, 1);
	TAILQ_INIT(&evdev->queues);
	spinlock_init(&evdev->lock);
	TAILQ_INIT(&evdev->poll_entries);
	for (size_t i = 0; ; ++i)
	{
		if (i == 4096)
		{
			sma_free(&evdev_sma, evdev);
			return -ENOMEM;
		}
		snprintf(evdev->name, sizeof(evdev->name), "event%zu", i);
		int ret = cdev_alloc(evdev->name, 0, 0, 0400, makedev(13, i), &fop, &evdev->cdev);
		if (ret == -EEXIST)
			continue;
		if (!ret)
			break;
		TRACE("evdev: failed to create device: %s", strerror(ret));
		sma_free(&evdev_sma, evdev);
		return ret;
	}
	evdev->cdev->userdata = evdev;
	*evdevp = evdev;
	return 0;
}

void evdev_free(struct evdev *evdev)
{
	if (!evdev)
		return;
	if (refcount_dec(&evdev->refcount))
		return;
	cdev_free(evdev->cdev);
	spinlock_destroy(&evdev->lock);
	sma_free(&evdev_sma, evdev);
}

void evdev_ref(struct evdev *evdev)
{
	refcount_inc(&evdev->refcount);
}

void send_event(struct evdev *evdev, const struct event *event)
{
	struct evdev_queue *queue;
	spinlock_lock(&evdev->lock);
	TAILQ_FOREACH(queue, &evdev->queues, chain)
	{
		pipebuf_lock(&queue->pipebuf);
		if (ringbuf_write_size(&queue->pipebuf.ringbuf) < sizeof(*event))
		{
			pipebuf_unlock(&queue->pipebuf);
			continue;
		}
		ringbuf_write(&queue->pipebuf.ringbuf, event, sizeof(*event));
		waitq_broadcast(&queue->rwaitq, 0);
		pipebuf_unlock(&queue->pipebuf);
	}
	spinlock_unlock(&evdev->lock);
	poller_broadcast(&evdev->poll_entries, POLLIN);
}

void queue_free(struct evdev_queue *queue)
{
	if (!queue)
		return;
	evdev_free(queue->evdev);
	waitq_destroy(&queue->rwaitq);
	waitq_destroy(&queue->wwaitq);
	mutex_destroy(&queue->mutex);
	pipebuf_destroy(&queue->pipebuf);
	sma_free(&queue_sma, queue);
}

int evdev_open(struct file *file, struct node *node)
{
	struct evdev *evdev = node->cdev->userdata;
	struct evdev_queue *queue = sma_alloc(&queue_sma, M_ZERO);
	if (!queue)
	{
		TRACE("evdev: queue allocation failed");
		return -ENOMEM;
	}
	waitq_init(&queue->rwaitq);
	waitq_init(&queue->wwaitq);
	mutex_init(&queue->mutex, 0);
	int ret = pipebuf_init(&queue->pipebuf, PIPE_BUF * 2, &queue->mutex,
	                       &queue->rwaitq, &queue->wwaitq);
	if (ret)
	{
		TRACE("evdev: failed to create ringbuf: %s", strerror(ret));
		queue_free(queue);
		return ret;
	}
	queue->pipebuf.nreaders = 1;
	queue->pipebuf.nwriters = 1;
	queue->evdev = evdev;
	spinlock_lock(&evdev->lock);
	TAILQ_INSERT_TAIL(&evdev->queues, queue, chain);
	spinlock_unlock(&evdev->lock);
	file->userdata = queue;
	evdev_ref(evdev);
	return 0;
}

int evdev_release(struct file *file)
{
	struct evdev_queue *queue = file->userdata;
	if (!queue)
		return 0;
	struct evdev *evdev = queue->evdev;
	spinlock_lock(&evdev->lock);
	TAILQ_REMOVE(&evdev->queues, queue, chain);
	spinlock_unlock(&evdev->lock);
	queue_free(queue);
	return 0;
}

ssize_t evdev_read(struct file *file, struct uio *uio)
{
	struct evdev_queue *queue = file->userdata;
	return pipebuf_read(&queue->pipebuf, uio, 0, NULL);
}

ssize_t evdev_write(struct file *file, struct uio *uio)
{
	struct evdev_queue *queue = file->userdata;
	struct evdev *evdev = queue->evdev;
	struct event event;
	ssize_t wr = 0;
	while (uio->count >= sizeof(event))
	{
		ssize_t ret = uio_copyout(&event, uio, sizeof(event));
		if (ret < 0)
			return ret;
		send_event(evdev, &event);
		wr += sizeof(event);
	}
	return wr;
}

int evdev_poll(struct file *file, struct poll_entry *entry)
{
	struct evdev_queue *queue = file->userdata;
	struct evdev *evdev = queue->evdev;
	int ret = pipebuf_poll(&queue->pipebuf, entry->events & ~POLLOUT);
	if (ret)
		return ret;
	entry->file_head = &evdev->poll_entries;
	return poller_add(entry);
}

void ev_send_key_event(struct evdev *evdev, enum kbd_key key,
                       enum kbd_mod mods, int pressed)
{
	if (pressed && curtty)
		send_tty(curtty, key, mods);
	struct event evt;
	evt.type = EVENT_KEY;
	evt.key.key = key;
	evt.key.mod = mods;
	evt.key.pressed = pressed;
	evt.key.pad0 = 0;
	send_event(event0, &evt);
	send_event(evdev, &evt);
}

void ev_send_mouse_event(struct evdev *evdev, enum mouse_button button,
                         int pressed)
{
	struct event evt;
	evt.type = EVENT_MOUSE;
	evt.mouse.button = button;
	evt.mouse.pressed = pressed;
	evt.mouse.pad0 = 0;
	evt.mouse.pad1 = 0;
	send_event(event0, &evt);
	send_event(evdev, &evt);
}

void ev_send_pointer_event(struct evdev *evdev, int x, int y)
{
	struct event evt;
	evt.type = EVENT_POINTER;
	evt.pointer.x = x;
	evt.pointer.y = y;
	evt.pointer.pad0 = 0;
	evt.pointer.pad1 = 0;
	send_event(event0, &evt);
	send_event(evdev, &evt);
}

void ev_send_scroll_event(struct evdev *evdev, int x, int y)
{
	struct event evt;
	evt.type = EVENT_SCROLL;
	evt.scroll.x = x;
	evt.scroll.y = y;
	evt.scroll.pad0 = 0;
	evt.scroll.pad1 = 0;
	send_event(event0, &evt);
	send_event(evdev, &evt);
}

int init(void)
{
	sma_init(&queue_sma, sizeof(struct evdev_queue), NULL, NULL, "evdev_queue");
	sma_init(&evdev_sma, sizeof(struct evdev), NULL, NULL, "evdev");
	evdev_alloc(&event0);
	return 0;
}

void fini(void)
{
}

struct kmod_info kmod =
{
	.magic = KMOD_MAGIC,
	.version = 1,
	.name = "evdev",
	.init = init,
	.fini = fini,
};
