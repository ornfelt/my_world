#ifndef SND_H
#define SND_H

#include <refcount.h>
#include <pipebuf.h>
#include <types.h>

#define SND_NBUFS 4

/* XXX snd buf might not be a great thing to do
 * it's been useful for ac97 / hda drivers, but it isn't a core sound thing
 * to map PAGE_SIZE buffers
 * it's probably better to let drivers allocates their dma buffers
 */

struct dma_buf;

struct snd
{
	refcount_t refcount;
	struct dma_buf *bufs[SND_NBUFS];
	size_t nbufs;
	struct pipebuf pipebuf;
	struct mutex mutex;
	struct waitq waitq;
	struct cdev *cdev;
	char name[16];
};

int snd_alloc(struct snd **snd);
void snd_free(struct snd *snd);
void snd_read(struct snd *snd, void *data, size_t size);
void snd_fill_buf(struct snd *snd, struct dma_buf *buf);

#endif
