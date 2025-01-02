#include <errno.h>
#include <file.h>
#include <proc.h>
#include <uio.h>
#include <vfs.h>
#include <cpu.h>
#include <mem.h>
#include <fb.h>

static struct fb *getfb(struct file *file)
{
	if (file->cdev)
		return file->cdev->userdata;
	if (file->node && S_ISCHR(file->node->attr.mode))
		return file->node->cdev->userdata;
	return NULL;
}

static ssize_t fb_read(struct file *file, struct uio *uio)
{
	struct fb *fb = getfb(file);
	if (!fb)
		return -EINVAL;
	if (uio->off < 0)
		return -EINVAL;
	if ((size_t)uio->off >= fb->size)
		return 0;
	size_t rem = fb->size - uio->off;
	size_t count = uio->count;
	if (rem < count)
		count = rem;
	return uio_copyin(uio, &fb->data[uio->off], count);
}

static ssize_t fb_write(struct file *file, struct uio *uio)
{
	struct fb *fb = getfb(file);
	if (!fb)
		return -EINVAL;
	if (uio->off < 0)
		return -EINVAL;
	if ((size_t)uio->off >= fb->size)
		return 0;
	size_t rem = fb->size - uio->off;
	size_t count = uio->count;
	if (rem < count)
		count = rem;
	return uio_copyout(&fb->data[uio->off], uio, count);
}

static off_t fb_seek(struct file *file, off_t off, int whence)
{
	struct fb *fb = getfb(file);
	if (!fb)
		return -EINVAL;
	switch (whence)
	{
		case SEEK_SET:
			if (off < 0)
				return -EINVAL;
			file->off = off;
			return file->off;
		case SEEK_CUR:
			if (off < 0 && off < -file->off)
				return -EINVAL;
			file->off += off;
			return file->off;
		case SEEK_END:
			if (off < -(off_t)fb->size)
				return -EINVAL;
			file->off = fb->size + off;
			return file->off;
		default:
			return -EINVAL;
	}
}

static int fb_ioctl(struct file *file, unsigned long request, uintptr_t data)
{
	struct fb *fb = getfb(file);
	if (!fb)
		return -EINVAL;
	switch (request)
	{
		case FBIOGET_INFO:
		{
			struct fb_info dst;
			dst.width = fb->width;
			dst.height = fb->height;
			dst.pitch = fb->pitch;
			dst.bpp = fb->bpp;
			return vm_copyout(curcpu()->thread->proc->vm_space,
			                  (void*)data, &dst, sizeof(dst));
		}
		case FBIO_FLUSH:
		{
			struct fb_rect rect;
			int ret = vm_copyin(curcpu()->thread->proc->vm_space,
			                    &rect, (void*)data, sizeof(rect));
			if (ret)
				return ret;
			return fb_flush(fb, rect.x, rect.y,
			                rect.width, rect.height);
		}
		default:
			return -EINVAL;
	}
}

static int fb_fault(struct vm_zone *zone, off_t off, struct page **page)
{
	off_t foff;
	if (__builtin_add_overflow(off, zone->off, &foff))
		return -EOVERFLOW;
	if (foff < 0)
		return -EOVERFLOW;
	struct fb *fb = getfb(zone->file);
	if (!fb)
		return -EINVAL;
	foff /= PAGE_SIZE;
	if ((size_t)foff >= fb->pages_count)
		return -EOVERFLOW;
	*page = fb->pages[foff];
	return 0;
}

const struct vm_zone_op fb_vm_op =
{
	.fault = fb_fault,
};

static int fb_mmap(struct file *file, struct vm_zone *zone)
{
	struct fb *fb = getfb(file);
	zone->op = &fb_vm_op;
	zone->prot |= fb->prot;
	return 0;
}

static const struct file_op fb_fop =
{
	.read = fb_read,
	.write = fb_write,
	.seek = fb_seek,
	.ioctl = fb_ioctl,
	.mmap = fb_mmap,
};

int fb_alloc(const struct fb_op *op, struct fb **fbp)
{
	int ret;
	struct fb *fb = malloc(sizeof(*fb), M_ZERO);
	if (!fb)
	{
		printf("fb: allocation failed\n");
		return -ENOMEM;
	}
	for (size_t i = 0; ; ++i)
	{
		if (i == 128)
		{
			free(fb);
			return -ENOMEM;
		}
		snprintf(fb->name, sizeof(fb->name), "fb%zu", i);
		ret = cdev_alloc(fb->name, 0, 0, 0600, makedev(29, i),
		                 &fb_fop, &fb->cdev);
		if (!ret)
			break;
		if (ret != -EEXIST)
		{
			free(fb);
			return ret;
		}
	}
	fb->cdev->userdata = fb;
	fb->op = op;
	*fbp = fb;
	return 0;
}

void fb_free(struct fb *fb)
{
	if (!fb)
		return;
	/* XXX */
	if (fb->data)
		vm_unmap(fb->data, PAGE_SIZE * fb->pages_count);
	free(fb);
}

int fb_update(struct fb *fb, uint32_t width, uint32_t height,
              enum fb_format format, uint32_t pitch, uint32_t bpp,
              struct page **pages, size_t pages_count, uint32_t prot)
{
	void *data = vm_map_pages(pages, pages_count, VM_PROT_RW | prot);
	if (!data)
	{
		printf("fb: failed to map framebuffer\n");
		return -ENOMEM;
	}
	fb->data = data;
	fb->format = format;
	fb->width = width;
	fb->height = height;
	fb->pitch = pitch;
	fb->bpp = bpp;
	fb->pages = pages;
	fb->pages_count = pages_count;
	fb->size = pitch * height;
	fb->prot = prot;
	return 0;
}

int fb_flush(struct fb *fb, uint32_t x, uint32_t y,
             uint32_t width, uint32_t height)
{
	if (!fb->op->flush)
		return 0;
	return fb->op->flush(fb, x, y, width, height);
}
