#include <spinlock.h>
#include <random.h>
#include <queue.h>
#include <time.h>
#include <std.h>

#define RESEED_INTERVAL 1000

struct random_source
{
	random_collect_t collect;
	void *userdata;
	TAILQ_ENTRY(random_source) chain;
};

static TAILQ_HEAD(, random_source) sources = TAILQ_HEAD_INITIALIZER(sources);
static struct spinlock rand_value_spinlock = SPINLOCK_INITIALIZER();
static uint64_t rand_value = 1;
static size_t reseed_count;

int random_register(random_collect_t collect, void *userdata)
{
	struct random_source *source = malloc(sizeof(*source), 0);
	if (!source)
		return -ENOMEM;
	source->collect = collect;
	source->userdata = userdata;
	TAILQ_INSERT_TAIL(&sources, source, chain);
	return 0;
}

static void reseed(void)
{
	struct random_source *source;
	TAILQ_FOREACH(source, &sources, chain)
	{
		uint64_t v;
		ssize_t ret = source->collect(&v, sizeof(v), source->userdata);
		if (ret < 0)
			continue;
		if (ret != 8)
			continue;
		rand_value ^= v;
	}
}

static uint8_t getu8(void)
{
	if (!--reseed_count)
	{
		reseed();
		reseed_count = RESEED_INTERVAL;
	}
	spinlock_lock(&rand_value_spinlock);
	uint64_t value = rand_value;
	rand_value = (value * 48271) % 0x7FFFFFFF;
	spinlock_unlock(&rand_value_spinlock);
	return value;
}

ssize_t random_get(void *dst, size_t count)
{
	for (size_t i = 0; i < count; ++i)
		((uint8_t*)dst)[i] = getu8();
	return count;
}

void random_init(void)
{
	reseed_count = RESEED_INTERVAL;
	/* XXX better seed :D */
	for (time_t i = 0; i < g_boottime.tv_nsec % 10000; ++i)
	{
		uint8_t tmp;
		random_get(&tmp, 1);
	}
}
