#ifndef SMA_H
#define SMA_H

#include <queue.h>
#include <mutex.h>
#include <types.h>

struct node;
struct uio;

typedef void (*sma_ctr_t)(void *ptr, size_t size);
typedef void (*sma_dtr_t)(void *ptr, size_t size);

struct sma_stats
{
	uint64_t nalloc;
	uint64_t nfree;
	uint64_t ncurrent;
	uint64_t nmetas;
	uint64_t nslabs;
	uint64_t nallocp;
	uint64_t nfreep;
	uint64_t ncurrentp;
};

struct sma
{
	TAILQ_HEAD(, sma_meta) meta;
	sma_ctr_t ctr;
	sma_dtr_t dtr;
	size_t bitmap_words; /* number of words */
	size_t bitmap_count; /* number of effective elements */
	size_t bitmap_size; /* memory size */
	size_t slab_size; /* vmalloc size */
	size_t data_size; /* size of each element */
	size_t meta_size; /* size of slab struct + bitmap */
	struct mutex mutex;
	struct node *sysfs_node;
	struct sma_stats stats;
	const char *name;
	TAILQ_ENTRY(sma) chain;
};

int sma_init(struct sma *sma, size_t data_size, sma_ctr_t ctr, sma_dtr_t dtr,
             const char *name);
void sma_destroy(struct sma *sma);
void *sma_alloc(struct sma *sma, int flags);
int sma_free(struct sma *sma, void *ptr);
void *sma_move(struct sma *dst, struct sma *src, void *ptr, int flags);
int sma_own(struct sma *sma, void *ptr);
int sma_print(struct sma *sma, struct uio *uio);
void sma_register_sysfs(void);

#endif
