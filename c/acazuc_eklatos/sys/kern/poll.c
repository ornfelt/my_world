#include <errno.h>
#include <file.h>
#include <poll.h>
#include <std.h>

int poller_init(struct poller *poller)
{
	spinlock_init(&poller->spinlock);
	waitq_init(&poller->waitq);
	TAILQ_INIT(&poller->entries);
	TAILQ_INIT(&poller->ready_entries);
	return 0;
}

void poller_destroy(struct poller *poller)
{
	spinlock_lock(&poller->spinlock);
	struct poll_entry *entry = TAILQ_FIRST(&poller->entries);
	while (entry)
	{
		TAILQ_REMOVE(&poller->entries, entry, poller_chain);
		TAILQ_REMOVE(entry->file_head, entry, file_chain);
		file_free(entry->file);
		entry = TAILQ_FIRST(&poller->entries);
	}
	entry = TAILQ_FIRST(&poller->ready_entries);
	while (entry)
	{
		TAILQ_REMOVE(&poller->ready_entries, entry, poller_chain);
		TAILQ_REMOVE(entry->file_head, entry, file_chain);
		file_free(entry->file);
		entry = TAILQ_FIRST(&poller->ready_entries);
	}
	waitq_destroy(&poller->waitq);
	spinlock_destroy(&poller->spinlock);
}

int poller_add(struct poll_entry *entry)
{
	file_ref(entry->file);
	entry->revents = 0;
	spinlock_lock(&entry->poller->spinlock);
	TAILQ_INSERT_TAIL(&entry->poller->entries, entry, poller_chain);
	TAILQ_INSERT_TAIL(entry->file_head, entry, file_chain);
	spinlock_unlock(&entry->poller->spinlock);
	return 0;
}

void poller_remove(struct poller_head *head)
{
	struct poll_entry *entry;
	TAILQ_FOREACH(entry, head, file_chain)
	{
		spinlock_lock(&entry->poller->spinlock);
		if (entry->revents)
			TAILQ_REMOVE(&entry->poller->ready_entries, entry, poller_chain);
		else
			TAILQ_REMOVE(&entry->poller->entries, entry, poller_chain);
		TAILQ_REMOVE(entry->file_head, entry, file_chain);
		spinlock_unlock(&entry->poller->spinlock);
	}
}

int poller_wait(struct poller *poller, struct timespec *timeout)
{
	spinlock_lock(&poller->spinlock);
	while (TAILQ_EMPTY(&poller->ready_entries))
	{
		int ret = waitq_wait_head(&poller->waitq, &poller->spinlock,
		                          timeout);
		if (ret)
		{
			spinlock_unlock(&poller->spinlock);
			return ret;
		}
	}
	spinlock_unlock(&poller->spinlock);
	return 0;
}

void poller_broadcast(struct poller_head *head, int events)
{
	if (!events)
		panic("poller broadcast no events\n");
	struct poll_entry *entry;
	TAILQ_FOREACH(entry, head, file_chain)
	{
		if (!(entry->events & events))
			continue;
		spinlock_lock(&entry->poller->spinlock);
		if (!entry->revents)
		{
			TAILQ_REMOVE(&entry->poller->entries, entry, poller_chain);
			TAILQ_INSERT_TAIL(&entry->poller->ready_entries, entry, poller_chain);
		}
		entry->revents |= events;
		waitq_broadcast(&entry->poller->waitq, 0);
		spinlock_unlock(&entry->poller->spinlock);
	}
}
