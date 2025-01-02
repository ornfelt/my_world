#ifndef EKLAT_FB_H
#define EKLAT_FB_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FBIOGET_INFO 0x101
#define FBIO_FLUSH   0x102

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

#ifdef __cplusplus
}
#endif

#endif
