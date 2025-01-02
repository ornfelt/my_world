#ifndef SG_H
#define SG_H

#include <queue.h>
#include <types.h>

struct dma_buf;
struct sg_head;
struct page;
struct uio;

struct sg
{
	struct page *page;
	size_t offset;
	size_t size;
	TAILQ_ENTRY(sg) chain;
};

struct sg_head
{
	TAILQ_HEAD(, sg) sg;
	size_t count;
};

void sg_init(struct sg_head *head);
void sg_free(struct sg_head *head);
void sg_reset(struct sg_head *head);
int sg_add_uio(struct sg_head *head, struct uio *uio, size_t size);
int sg_add_ubuf(struct sg_head *head, const void *data, size_t size);
int sg_add_kbuf(struct sg_head *head, const void *data, size_t size);
int sg_add_dma_buf(struct sg_head *head, struct dma_buf *dma, size_t size,
                   size_t off);

#endif
