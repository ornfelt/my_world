#ifndef WAITQ_H
#define WAITQ_H

#include <spinlock.h>
#include <queue.h>

struct timespec;
struct mutex;

struct waitq
{
	struct spinlock spinlock;
	TAILQ_HEAD(, thread) watchers;
};

void waitq_check_timeout(const struct timespec *ts);
void waitq_init(struct waitq *waitq);
void waitq_destroy(struct waitq *waitq);
int waitq_wait_tail(struct waitq *waitq, struct spinlock *spinlock,
                    const struct timespec *timeout);
int waitq_wait_head(struct waitq *waitq, struct spinlock *spinlock,
                    const struct timespec *timeout);
int waitq_wait_tail_mutex(struct waitq *waitq, struct mutex *mutex,
                          const struct timespec *timeout);
int waitq_wait_head_mutex(struct waitq *waitq, struct mutex *mutex,
                          const struct timespec *timeout);
int waitq_signal(struct waitq *waitq, int reason);
int waitq_broadcast(struct waitq *waitq, int reason);
void waitq_wakeup_thread(struct waitq *waitq, struct thread *thread,
                         int reason);

#endif
