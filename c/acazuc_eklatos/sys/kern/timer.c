#include <spinlock.h>
#include <timer.h>
#include <std.h>

TAILQ_HEAD(timer_head, timer);

static struct spinlock timers_realtime_lock = SPINLOCK_INITIALIZER();
static struct timer_head timers_realtime = TAILQ_HEAD_INITIALIZER(timers_realtime);
static struct spinlock timers_monotonic_lock = SPINLOCK_INITIALIZER();
static struct timer_head timers_monotonic = TAILQ_HEAD_INITIALIZER(timers_monotonic);

static void check_timeout(struct timer_head *timers, struct spinlock *lock,
                          int clock, const struct timespec *ts)
{
	struct timespec cur;

	spinlock_lock(lock);
	if (TAILQ_EMPTY(timers))
	{
		spinlock_unlock(lock);
		return;
	}
	spinlock_unlock(lock);

	if (ts)
	{
		cur = *ts;
	}
	else
	{
		if (clock_gettime(clock, &cur))
			panic("failed to get clock\n");
	}

	spinlock_lock(lock);
	while (1)
	{
		struct timer *timer = TAILQ_FIRST(timers);
		if (!timer)
			break;
		if (timespec_cmp(&cur, &timer->timeout) < 0)
			break;
		TAILQ_REMOVE(timers, timer, chain);
		timer->queued = 0;
		spinlock_unlock(lock);
		timer->cb(timer);
		spinlock_lock(lock);
	}
	spinlock_unlock(lock);
}

void timer_check_timeout(const struct timespec *monotonic_ts)
{
	check_timeout(&timers_realtime, &timers_realtime_lock, CLOCK_REALTIME, NULL);
	check_timeout(&timers_monotonic, &timers_monotonic_lock, CLOCK_MONOTONIC, monotonic_ts);
}

static void add_timer(struct timer *timer, struct timer_head *timers,
                      struct spinlock *lock)
{
	struct timer *it;
	spinlock_lock(lock);
	TAILQ_FOREACH(it, timers, chain)
	{
		if (timespec_cmp(&timer->timeout, &it->timeout) < 0)
		{
			TAILQ_INSERT_BEFORE(it, timer, chain);
			spinlock_unlock(lock);
			return;
		}
	}
	TAILQ_INSERT_TAIL(timers, timer, chain);
	spinlock_unlock(lock);
}

void timer_add(struct timer *timer, int clock, struct timespec timeout,
               timer_cb_t cb, void *userdata)
{
	timer->timeout = timeout;
	timer->cb = cb;
	timer->userdata = userdata;
	timer->queued = 1;
	timer->clock = clock;
	switch (clock)
	{
		case CLOCK_MONOTONIC:
			add_timer(timer, &timers_monotonic, &timers_monotonic_lock);
			break;
		case CLOCK_REALTIME:
			add_timer(timer, &timers_realtime, &timers_realtime_lock);
			break;
		default:
			panic("invalid timer\n");
	}
}

static void remove_timer(struct timer *timer, struct timer_head *timers,
                         struct spinlock *lock)
{
	spinlock_lock(lock);
	TAILQ_REMOVE(timers, timer, chain);
	timer->queued = 0;
	spinlock_unlock(lock);
}

void timer_remove(struct timer *timer)
{
	assert(timer->queued, "removing unqueued timer\n");
	switch (timer->clock)
	{
		case CLOCK_MONOTONIC:
			remove_timer(timer, &timers_monotonic, &timers_monotonic_lock);
			break;
		case CLOCK_REALTIME:
			remove_timer(timer, &timers_realtime, &timers_realtime_lock);
			break;
		default:
			panic("invalid timer\n");
	}
}
