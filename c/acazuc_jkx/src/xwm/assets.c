#include "xwm.h"

#include <X11/extensions/XShm.h>

#include <sys/param.h>
#include <sys/shm.h>

#include <libpng/png.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#define EXPAND_SIZE 2048 /* that's a bit too much, but it does save so much CopyArea requests.. */

static Pixmap create_pixmap(struct xwm *xwm, uint32_t *width, uint32_t *height,
                            const uint8_t *data, int expand_width,
                            int expand_height)
{
	XSync(xwm->display, False);
	XSynchronize(xwm->display, False);
	uint32_t dst_width;
	if (expand_width)
	{
		dst_width = EXPAND_SIZE + *width - 1;
		dst_width -= dst_width % *width;
	}
	else
	{
		dst_width = *width;
	}
	uint32_t dst_height;
	if (expand_height)
	{
		dst_height = EXPAND_SIZE + *height - 1;
		dst_height -= dst_height % *height;
	}
	else
	{
		dst_height = *height;
	}
	Pixmap pixmap = XCreatePixmap(xwm->display, xwm->root,
	                              xwm->screen_width, xwm->screen_height,
	                              24);
	if (!pixmap)
	{
		fprintf(stderr, "%s: failed to create pimap\n", xwm->progname);
		return None;
	}
	XShmSegmentInfo shminfo;
	XImage *image = XShmCreateImage(xwm->display, xwm->vi.visual, 24,
	                                ZPixmap, NULL, &shminfo, *width,
	                                *height);
	if (!image)
	{
		fprintf(stderr, "%s: failed to create image\n", xwm->progname);
		goto err;
	}
	shminfo.shmid = shmget(IPC_PRIVATE, dst_width * dst_height * 4,
	                       IPC_CREAT | 0777);
	if (shminfo.shmid == -1)
	{
		fprintf(stderr, "%s: shmget: %s\n", xwm->progname,
		        strerror(errno));
		goto err;
	}
	image->data = shmat(shminfo.shmid, 0, 0);
	if (shmctl(shminfo.shmid, IPC_RMID, NULL) == -1)
	{
		fprintf(stderr, "%s: shmctl: %s\n", xwm->progname,
		        strerror(errno));
		goto err;
	}
	if (image->data == (void*)-1)
	{
		fprintf(stderr, "%s: shmat: %s\n", xwm->progname,
		        strerror(errno));
		goto err;
	}
	for (size_t y = 0; y < *height; ++y)
	{
		size_t n = y * *width * 4;
		for (size_t x = 0; x < *width; ++x)
		{
			uint8_t *dst = &((uint8_t*)image->data)[n];
			uint8_t *src = &((uint8_t*)data)[n];
			dst[0] = src[2];
			dst[1] = src[1];
			dst[2] = src[0];
			dst[3] = src[3];
			n += 4;
		}
	}
	shminfo.shmaddr = image->data;
	shminfo.readOnly = False;
	XShmAttach(xwm->display, &shminfo);
	for (size_t y = 0; y < dst_height; y += *height)
	{
		for (size_t x = 0; x < dst_width; x += *width)
		{
			XShmPutImage(xwm->display, pixmap, xwm->bitmap_gc,
			             image, 0, 0, x, y, *width, *height, False);
		}
	}
	XShmDetach(xwm->display, &shminfo);
	XSync(xwm->display, False);
	XSynchronize(xwm->display, True);
	shmdt(image->data);
	image->data = NULL;
	XDestroyImage(image);
	*width = dst_width;
	*height = dst_height;
	return pixmap;

err:
	XFreePixmap(xwm->display, pixmap);
	image->data = NULL;
	XDestroyImage(image);
	return None;
}

static int load_png(struct xwm *xwm, const char *file, uint8_t **data,
                    uint32_t *width, uint32_t *height)
{
	int ret = 1;
	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;
	png_bytep *row_pointers = NULL;
	FILE *fp = NULL;
	int row_bytes;
	fp = fopen(file, "rb");
	if (!fp)
	{
		fprintf(stderr, "%s: open(%s): %s\n", xwm->progname, file,
		        strerror(errno));
		goto end;
	}
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr)
	{
		fprintf(stderr, "%s: failed to create png struct\n",
		        xwm->progname);
		goto end;
	}
	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		fprintf(stderr, "%s: failed to create png info struct\n",
		        xwm->progname);
		goto end;
	}
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		fprintf(stderr, "%s: failed to set png longjmp\n",
		        xwm->progname);
		goto end;
	}
	png_init_io(png_ptr, fp);
	png_read_info(png_ptr, info_ptr);
	png_get_IHDR(png_ptr, info_ptr, width, height, NULL, NULL, NULL, NULL, NULL);
	png_read_update_info(png_ptr, info_ptr);
	row_bytes = png_get_rowbytes(png_ptr, info_ptr);
	*data = malloc(row_bytes * *height);
	if (!*data)
	{
		fprintf(stderr, "%s: malloc: %s\n", xwm->progname,
		        strerror(errno));
		goto end;
	}
	row_pointers = malloc(sizeof(png_bytep) * *height);
	if (!row_pointers)
	{
		fprintf(stderr, "%s: malloc: %s\n", xwm->progname,
		        strerror(errno));
		goto end;
	}
	for (uint32_t i = 0; i < *height; ++i)
		row_pointers[i] = &(*data)[i * row_bytes];
	png_read_image(png_ptr, row_pointers);
	ret = 0;

end:
	free(row_pointers);
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	if (fp)
		fclose(fp);
	return ret;
}

int setup_background(struct xwm *xwm)
{
	uint8_t *data;
	char path[MAXPATHLEN];
	snprintf(path, sizeof(path), "%s/background.png", xwm->assets_path);
	if (load_png(xwm, path, &data, &xwm->assets.background.width,
	             &xwm->assets.background.height))
		return 1;
	uint8_t *scaled_data = malloc(xwm->screen_width * xwm->screen_height * 4);
	if (!scaled_data)
	{
		fprintf(stderr, "%s: malloc: %s\n", xwm->progname,
		        strerror(errno));
		return 1;
	}
	for (size_t y = 0; y < xwm->screen_height; ++y)
	{
		uint32_t yy = y / (float)xwm->screen_height
		            * xwm->assets.background.height;
		if (yy >= xwm->assets.background.height)
			break;
		uint32_t *dst = &((uint32_t*)scaled_data)[y * xwm->screen_width];
		for (size_t x = 0; x < xwm->screen_width; ++x)
		{
			uint32_t xx = x / (float)xwm->screen_width
			            * xwm->assets.background.width;
			if (xx >= xwm->assets.background.width)
				break;
			*dst = ((uint32_t*)data)[yy * xwm->assets.background.width + xx];
			dst++;
		}
	}
	xwm->assets.background.pixmap = create_pixmap(xwm, &xwm->screen_width,
	                                              &xwm->screen_height,
	                                              scaled_data, 0, 0);
	free(data);
	if (!xwm->assets.background.pixmap)
		return 1;
	return 0;
}

static int load_asset(struct xwm *xwm, struct asset *asset, const char *file,
                      int expand_width, int expand_height)
{
	uint8_t *data;
	char path[MAXPATHLEN];
	snprintf(path, sizeof(path), "%s/%s.png", xwm->assets_path, file);
	if (load_png(xwm, path, &data, &asset->width, &asset->height))
		return 1;
	asset->pixmap = create_pixmap(xwm, &asset->width, &asset->height, data,
	                              expand_width, expand_height);
	free(data);
	if (!asset->pixmap)
		return 1;
	return 0;
}

int setup_assets(struct xwm *xwm)
{
#define LOAD_ASSET(name, expw, exph) \
do \
{ \
	if (load_asset(xwm, &xwm->assets.name, #name, expw, exph)) \
		return 1; \
} while (0)

	LOAD_ASSET(header, 1, 0);
	LOAD_ASSET(header_resize_l, 0, 0);
	LOAD_ASSET(header_resize_r, 0, 0);
	LOAD_ASSET(iconify, 0, 0);
	LOAD_ASSET(iconify_hovered, 0, 0);
	LOAD_ASSET(iconify_clicked, 0, 0);
	LOAD_ASSET(maximize, 0, 0);
	LOAD_ASSET(maximize_hovered, 0, 0);
	LOAD_ASSET(maximize_clicked, 0, 0);
	LOAD_ASSET(close, 0, 0);
	LOAD_ASSET(close_hovered, 0, 0);
	LOAD_ASSET(close_clicked, 0, 0);
	LOAD_ASSET(taskbar, 1, 0);
	LOAD_ASSET(taskbar_l, 0, 0);
	LOAD_ASSET(taskbar_r, 0, 0);
	LOAD_ASSET(taskbar_button, 1, 0);
	LOAD_ASSET(taskbar_button_l, 0, 0);
	LOAD_ASSET(taskbar_button_r, 0, 0);
	LOAD_ASSET(taskbar_button_hovered, 0, 0);
	LOAD_ASSET(taskbar_button_disabled, 0, 0);
	LOAD_ASSET(resize_tl, 0, 0);
	LOAD_ASSET(resize_tr, 0, 0);
	LOAD_ASSET(resize_bl, 0, 0);
	LOAD_ASSET(resize_br, 0, 0);
	LOAD_ASSET(resize_l, 0, 1);
	LOAD_ASSET(resize_r, 0, 1);
	LOAD_ASSET(resize_t, 1, 0);
	LOAD_ASSET(resize_b, 1, 0);

#undef LOAD_ASSET

	return 0;
}
