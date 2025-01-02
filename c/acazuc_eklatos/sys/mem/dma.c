#include <errno.h>
#include <mem.h>
#include <sma.h>

static struct sma dma_buf_sma;

void dma_buf_init(void)
{
	sma_init(&dma_buf_sma, sizeof(struct dma_buf), NULL, NULL, "dma_buf");
}

int dma_buf_alloc(size_t size, uint32_t flags, struct dma_buf **bufp)
{
	struct dma_buf *buf = NULL;
	size_t npages;
	int ret;

	if (flags & ~(DMA_32BIT))
		return -EINVAL;
	if (!size)
		return -EINVAL;
	npages = size + PAGE_SIZE - 1;
	npages /= PAGE_SIZE;
	buf = sma_alloc(&dma_buf_sma, M_ZERO);
	if (!buf)
		return -ENOMEM;
	buf->npages = npages;
	buf->size = size;
	ret = pm_alloc_pages(&buf->pages, npages); /* XXX use DMA_32BIT */
	if (ret)
		goto err;
	buf->data = vm_map(buf->pages, buf->npages * PAGE_SIZE, VM_PROT_RW);
	if (!buf->data)
	{
		ret = -ENOMEM;
		goto err;
	}
	*bufp = buf;
	return 0;

err:
	dma_buf_free(buf);
	return ret;
}

void dma_buf_free(struct dma_buf *buf)
{
	if (!buf)
		return;
	if (buf->data)
		vm_unmap(buf->data, PAGE_SIZE * buf->npages);
	if (buf->pages)
		pm_free_pages(buf->pages, buf->npages);
	sma_free(&dma_buf_sma, buf);
}
