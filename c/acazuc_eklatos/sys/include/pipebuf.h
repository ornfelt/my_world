#ifndef PIPEBUF_H
#define PIPEBUF_H

#include <ringbuf.h>
#include <mutex.h>
#include <waitq.h>

#define PIPE_BUF 4096

struct pipebuf
{
	struct ringbuf ringbuf;
	struct waitq *rwaitq;
	struct waitq *wwaitq;
	struct mutex *mutex;
	size_t nreaders;
	size_t nwriters;
	/* XXX add poller_head here ? */
};

int pipebuf_init(struct pipebuf *pipebuf, size_t size, struct mutex *mutex,
                 struct waitq *rwaitq, struct waitq *wwaitq);
void pipebuf_destroy(struct pipebuf *pipebuf);
ssize_t pipebuf_read(struct pipebuf *pipebuf, struct uio *uio, size_t min,
                     struct timespec *timeout);
ssize_t pipebuf_read_locked(struct pipebuf *pipebuf, struct uio *uio, size_t min,
                            struct timespec *timeout);
ssize_t pipebuf_write(struct pipebuf *pipebuf, struct uio *uio, size_t min,
                      struct timespec *timeout);
ssize_t pipebuf_write_locked(struct pipebuf *pipebuf, struct uio *uio, size_t min,
                             struct timespec *timeout);
int pipebuf_poll(struct pipebuf *pipebuf, int events);
int pipebuf_poll_locked(struct pipebuf *pipebuf, int events);

static inline void pipebuf_lock(struct pipebuf *pipebuf)
{
	if (pipebuf->mutex);
		mutex_lock(pipebuf->mutex);
}

static inline void pipebuf_unlock(struct pipebuf *pipebuf)
{
	if (pipebuf->mutex);
		mutex_unlock(pipebuf->mutex);
}

#endif
