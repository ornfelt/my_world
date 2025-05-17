#include "png.h"

#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

const png_size_t
adam_x_offsets[] =
{
	0, 4, 0, 2, 0, 1, 0,
};

const png_size_t
adam_y_offsets[] =
{
	0, 0, 4, 0, 2, 0, 1,
};

const png_size_t
adam_x_shifts[] =
{
	3, 3, 2, 2, 1, 1, 0,
};

const png_size_t
adam_y_shifts[] =
{
	3, 3, 3, 2, 2, 1, 1,
};

const png_size_t
adam_x_masks[] =
{
	7, 7, 3, 3, 1, 1, 0,
};

const png_size_t
adam_y_masks[] =
{
	7, 7, 7, 3, 3, 1, 1,
};

png_voidp
png_malloc_fn(png_structp png, png_size_t size)
{
	(void)png;
	return malloc(size);
}

void
png_free_fn(png_structp png, png_voidp ptr)
{
	(void)png;
	free(ptr);
}

png_infop
png_create_info_struct(png_structp png)
{
	png_infop info = png->malloc_fn(png, sizeof(*info));
	if (!info)
		return NULL;
	memset(info, 0, sizeof(*info));
	info->png = png;
	return info;
}

void
png_init_io(png_structp png, FILE *fp)
{
	png->fp = fp;
}

int
png_sig_cmp(png_bytep sig, png_size_t start, png_size_t num)
{
	if (start != 0 || num != 8)
		return 1;
	if (memcmp(sig, PNG_MAGIC, 8))
		return 1;
	return 0;
}

jmp_buf *
png_jmpbufp(png_structp png)
{
	return &png->jmpbuf;
}

png_voidp
png_get_error_ptr(png_structp png)
{
	return png->err_ptr;
}

void
png_set_error_fn(png_structp png,
                 png_voidp err_ptr,
                 png_error_ptr err_fn,
                 png_error_ptr warn_fn)
{
	png->err_ptr = err_ptr;
	png->err_fn = err_fn;
	png->warn_fn = warn_fn;
}

png_voidp
png_get_mem_ptr(png_structp png)
{
	return png->mem_ptr;
}

void
png_set_mem_fn(png_structp png,
               png_voidp mem_ptr,
               png_malloc_ptr malloc_fn,
               png_free_ptr free_fn)
{
	png->mem_ptr = mem_ptr;
	png->malloc_fn = malloc_fn ? malloc_fn : png_malloc_fn;
	png->free_fn = free_fn ? free_fn : png_free_fn;
}

void
png_err(png_const_structp png, const char *fmt, ...)
{
	char buf[4096];
	va_list va_arg;
	va_start(va_arg, fmt);
	vsnprintf(buf, sizeof(buf), fmt, va_arg);
	if (png->err_fn)
		png->err_fn((png_structp)png, buf);
	va_end(va_arg);
	longjmp(((png_structp)png)->jmpbuf, 1);
}

png_size_t
png_get_bpp(png_structp png)
{
	return (png->ihdr.depth + 7) / 8 * png->channels;
}

png_size_t
png_get_rowbytes(png_structp png, png_infop info)
{
	(void)info;
	return png->ihdr.width * png_get_bpp(png);
}

int
png_alloc_rows(png_structp png)
{
	png_size_t row_size = png_get_rowbytes(png, NULL);
	png->free_fn(png, png->wrk_rows[0]);
	png->free_fn(png, png->wrk_rows[1]);
	png->free_fn(png, png->tmp_rows[0]);
	png->free_fn(png, png->tmp_rows[1]);
	png->free_fn(png, png->tmp_rows[2]);
	png->free_fn(png, png->tmp_rows[3]);
	png->wrk_rows[0] = png->malloc_fn(png, row_size);
	png->wrk_rows[1] = png->malloc_fn(png, row_size);
	png->tmp_rows[0] = png->malloc_fn(png, row_size);
	png->tmp_rows[1] = png->malloc_fn(png, row_size);
	png->tmp_rows[2] = png->malloc_fn(png, row_size);
	png->tmp_rows[3] = png->malloc_fn(png, row_size);
	if (!png->wrk_rows[0]
	 || !png->wrk_rows[1]
	 || !png->tmp_rows[0]
	 || !png->tmp_rows[1]
	 || !png->tmp_rows[2]
	 || !png->tmp_rows[3])
		return 1;
	return 0;
}

void
png_free(png_structp png)
{
	if (!png)
		return;
	png->free_fn(png, png->data);
	png->free_fn(png, png->wrk_rows[0]);
	png->free_fn(png, png->wrk_rows[1]);
	png->free_fn(png, png->tmp_rows[0]);
	png->free_fn(png, png->tmp_rows[1]);
	png->free_fn(png, png->tmp_rows[2]);
	png->free_fn(png, png->tmp_rows[3]);
	free(png); /* XXX free_fn */
}

int
png_set_interlace_handling(png_structp png)
{
	if (png->ihdr.interlace == PNG_INTERLACE_ADAM7)
		return 7;
	return 1;
}
