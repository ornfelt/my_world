#include <proc.h>
#include <std.h>
#include <mem.h>
#include <cpu.h>
#include <uio.h>
#include <sg.h>

void sg_init(struct sg_head *head)
{
	TAILQ_INIT(&head->sg);
	head->count = 0;
}

void sg_free(struct sg_head *head)
{
	if (!head)
		return;
	sg_reset(head);
}

void sg_reset(struct sg_head *head)
{
	struct sg *sg;

	while ((sg = TAILQ_FIRST(&head->sg)))
	{
		TAILQ_REMOVE(&head->sg, sg, chain);
		pm_free_page(sg->page);
		free(sg);
	}
	head->count = 0;
}

static int add_sg(struct sg_head *head, size_t paddr, size_t offset, size_t size)
{
	struct page *page;
	struct sg *sg;

	/* XXX this make available only "memory" pages
	 * it may be useful (I don't know for now) to be able
	 * to add mmio pages to sg)
	 *
	 * XXX merge contiguous sg ?
	 */
	page = pm_get_page(paddr / PAGE_SIZE);
	if (!page)
		return -EFAULT;
	sg = malloc(sizeof(*sg), M_ZERO);
	if (!sg)
		return -ENOMEM;
	pm_ref_page(page);
	sg->page = page;
	sg->offset = offset;
	sg->size = size;
	TAILQ_INSERT_TAIL(&head->sg, sg, chain);
	head->count++;
	return 0;
}

static int add_buf(struct sg_head *head, struct vm_space *vm_space,
                   const void *data, size_t size)
{
	uintptr_t paddr;
	uintptr_t off;
	size_t bytes;
	int ret;

	while (size)
	{
		off = (uintptr_t)data & PAGE_MASK;
		ret = vm_paddr(vm_space, (uintptr_t)data - off, &paddr);
		if (ret)
			return ret;
		bytes = PAGE_SIZE - off;
		if (bytes > size)
			bytes = size;
		ret = add_sg(head, paddr, off, bytes);
		if (ret)
			return ret;
		size -= bytes;
	}
	return 0;
}

int sg_add_uio(struct sg_head *head, struct uio *uio, size_t size)
{
	struct vm_space *vm_space = curcpu()->thread->proc->vm_space;
	struct iovec *iov;
	size_t n;
	int ret;

	for (size_t i = 0; i < uio->iovcnt && size; ++i)
	{
		iov = &uio->iov[i];
		if (!iov->iov_len)
			continue;
		n = size;
		if (n > iov->iov_len)
			n = iov->iov_len;
		ret = add_buf(head, vm_space, iov->iov_base, n);
		if (ret)
			return ret;
		size -= n;
	}
	return 0;
}

int sg_add_ubuf(struct sg_head *head, const void *data, size_t size)
{
	return add_buf(head, curcpu()->thread->proc->vm_space, data, size);
}

int sg_add_kbuf(struct sg_head *head, const void *data, size_t size)
{
	return add_buf(head, NULL, data, size);
}

int sg_add_dma_buf(struct sg_head *head, struct dma_buf *dma, size_t size,
                   size_t off)
{
	return add_sg(head, pm_page_addr(dma->pages), off, size);
}
