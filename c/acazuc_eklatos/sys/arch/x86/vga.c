#include <errno.h>
#include <std.h>
#include <tty.h>
#include <mem.h>
#include <fb.h>

struct vga
{
	int rgb;
	uint32_t paddr;
	uint32_t width;
	uint32_t height;
	uint32_t pitch;
	uint32_t bpp;
	size_t pages_count;
	struct page **pages;
	struct fb *fb;
};

static struct tty *ttys[1];

static void mktty(struct vga *vga)
{
	for (uint32_t i = 0; i < sizeof(ttys) / sizeof(*ttys); ++i)
	{
		char name[16];
		snprintf(name, sizeof(name), "tty%" PRIu32, i);
		int ret = vtty_alloc(name, i, vga->fb, &ttys[i]);
		if (ret)
			printf("can't create /dev/%s: %s", name, strerror(ret));
	}
	curtty = ttys[0];
	printf_addtty(ttys[0]);
}

static int vga_fb_flush(struct fb *fb, uint32_t x, uint32_t y,
                        uint32_t width, uint32_t height)
{
	/* XXX flip cache to memory */
	(void)fb;
	(void)x;
	(void)y;
	(void)width;
	(void)height;
	return 0;
}

static const struct fb_op fb_op =
{
	.flush = vga_fb_flush,
};

static int mkfb(struct vga *vga)
{
	int ret;
	vga->pages_count = vga->pitch * vga->height;
	vga->pages_count += PAGE_SIZE - 1;
	vga->pages_count /= PAGE_SIZE;
	vga->pages = malloc((sizeof(**vga->pages) + sizeof(*vga->pages)) * vga->pages_count, M_ZERO);
	if (!vga->pages)
	{
		printf("vga: pages allocation failed\n");
		ret = -ENOMEM;
		goto err;
	}
	for (size_t i = 0; i < vga->pages_count; ++i)
	{
		vga->pages[i] = (struct page*)&((uint8_t*)vga->pages)[sizeof(*vga->pages) * vga->pages_count + sizeof(**vga->pages) * i];
		pm_init_page(vga->pages[i], vga->paddr / PAGE_SIZE + i);
	}
	ret = fb_alloc(&fb_op, &vga->fb);
	if (ret)
	{
		printf("vga: fb allocation failed\n");
		goto err;
	}
	vga->fb->userdata = vga;
	ret = fb_update(vga->fb, vga->width, vga->height,
	                vga->rgb ? FB_FMT_B8G8R8A8 : FB_FMT_VGA,
	                vga->pitch, vga->bpp, vga->pages, vga->pages_count,
	                VM_WC);
	if (ret)
	{
		printf("vga: failed to update fb\n");
		goto err;
	}
	return 0;

err:
	free(vga->pages);
	vga->pages = NULL;
	fb_free(vga->fb);
	vga->fb = NULL;
	return ret;
}

void vga_init(int rgb, uint32_t paddr, uint32_t width, uint32_t height,
              uint32_t pitch, uint32_t bpp)
{
	struct vga *vga = malloc(sizeof(*vga), M_ZERO);
	if (!vga)
	{
		printf("vga: allocation failed\n");
		return;
	}
	vga->rgb = rgb;
	vga->paddr = paddr;
	vga->width = width;
	vga->height = height;
	vga->pitch = pitch;
	vga->bpp = bpp;
	mkfb(vga);
	mktty(vga);
}
