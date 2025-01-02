#ifndef FB_H
#define FB_H

#include <types.h>

#define FBIOGET_INFO 0x101
#define FBIO_FLUSH   0x102

struct fb_op;

struct fb_info
{
	uint32_t width;
	uint32_t height;
	uint32_t pitch;
	uint32_t bpp;
};

struct fb_rect
{
	uint32_t x;
	uint32_t y;
	uint32_t width;
	uint32_t height;
};

enum fb_format
{
	FB_FMT_B8G8R8A8,
	FB_FMT_VGA, /* vga text mode */
};

struct fb
{
	const struct fb_op *op;
	enum fb_format format;
	char name[16];
	struct cdev *cdev;
	uint8_t *data;
	size_t size;
	uint32_t width;
	uint32_t height;
	uint32_t pitch;
	uint32_t bpp;
	void *userdata;
	struct page **pages;
	size_t pages_count;
	uint32_t prot;
};

struct fb_op
{
	int (*flush)(struct fb *fb, uint32_t x, uint32_t y,
	             uint32_t width, uint32_t height);
};

int fb_alloc(const struct fb_op *op, struct fb **fb);
void fb_free(struct fb *fb);
int fb_update(struct fb *fb, uint32_t width, uint32_t height,
              enum fb_format format, uint32_t pitch, uint32_t bpp,
              struct page **pages, size_t pages_count, uint32_t prot);
int fb_flush(struct fb *fb, uint32_t x, uint32_t y,
             uint32_t width, uint32_t height);

#endif
