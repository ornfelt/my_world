#ifndef POLL_H
#define POLL_H

#include <spinlock.h>
#include <waitq.h>
#include <types.h>
#include <time.h>

#define POLLIN   (1 << 0)
#define POLLPRI  (1 << 1)
#define POLLOUT  (1 << 2)
#define POLLERR  (1 << 3)
#define POLLHUP  (1 << 4)
#define POLLNVAL (1 << 5)

#define POLLIN_SET  (POLLIN | POLLHUP | POLLERR)
#define POLLOUT_SET (POLLOUT | POLLERR)
#define POLLEX_SET  (POLLPRI)

#define FD_SETSIZE 1024
#define FDSET_WORDS (FD_SETSIZE / FDSET_BPW)
#define FDSET_BPW (8)

#define FD_ZERO(fds) \
do \
{ \
	for (size_t _fds_i = 0; _fds_i < FDSET_WORDS; ++_fds_i) \
		(fds)->values[_fds_i] = 0; \
} while (0)

#define FD_CLR(fd, fds) \
do \
{ \
	(fds)->values[(fd) / FDSET_BPW] &= ~(1 << ((fd) % FDSET_BPW)); \
} while (0)

#define FD_SET(fd, fds) \
do \
{ \
	(fds)->values[(fd) / FDSET_BPW] |= 1 << ((fd) % FDSET_BPW); \
} while (0)

#define FD_ISSET(fd, fds) \
	((fds)->values[(fd) / FDSET_BPW] & (1 << ((fd) % FDSET_BPW)))

struct poller;

typedef struct
{
	uint8_t values[FDSET_WORDS];
} fd_set;

typedef unsigned long nfds_t;

struct pollfd
{
	int fd;
	short events;
	short revents;
};

TAILQ_HEAD(poller_head, poll_entry);

struct poll_entry
{
	struct poller *poller;
	struct file *file;
	short events;
	short revents;
	struct poller_head *file_head;
	TAILQ_ENTRY(poll_entry) poller_chain;
	TAILQ_ENTRY(poll_entry) file_chain;
};

struct poller
{
	struct spinlock spinlock;
	struct waitq waitq;
	struct poller_head entries;
	struct poller_head ready_entries
};

int poller_init(struct poller *poller);
void poller_destroy(struct poller *poller);
int poller_add(struct poll_entry *entry);
void poller_remove(struct poller_head *head);
int poller_wait(struct poller *poller, struct timespec *timeout);
void poller_broadcast(struct poller_head *head, int events);

static inline void poller_spinlock(struct poller *poller)
{
	spinlock_lock(&poller->spinlock);
}

static inline void poller_unlock(struct poller *poller)
{
	spinlock_unlock(&poller->spinlock);
}

#endif
