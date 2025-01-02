#ifndef PNG_H
#define PNG_H

#include <libpng/png.h>

#include <stdio.h>
#include <zlib.h>

#define PNG_MAGIC "\x89\x50\x4E\x47\x0D\x0A\x1A\x0A"

struct png_struct
{
	png_const_charp version;
	png_error_ptr err_fn;
	png_error_ptr warn_fn;
	png_malloc_ptr malloc_fn;
	png_free_ptr free_fn;
	png_voidp err_ptr;
	png_voidp mem_ptr;
	FILE *fp;
	png_size_t sig_bytes;
	int parsed_ihdr;
	struct
	{
		png_uint_32 width;
		png_uint_32 height;
		uint8_t depth;
		uint8_t color_type;
		uint8_t compression;
		uint8_t filter;
		uint8_t interlace;
	} ihdr;
	z_stream zstream;
	png_voidp data;
	png_const_bytep data_it;
	png_size_t data_size;
	png_size_t data_pos;
	jmp_buf jmpbuf;
	png_size_t row_idx;
	png_size_t channels;
	png_voidp wrk_rows[2]; /* flip-flopped rows for filtering */
	png_voidp tmp_rows[4]; /* buffers for filtering heuristics */
	png_size_t row_ff;
	png_size_t adam_pass;
};

struct png_info
{
	png_structp png;
};

void __attribute__((format(printf, 2, 3))) png_err(png_const_structp png, const char *fmt, ...);
png_size_t png_get_bpp(png_structp png);
int png_alloc_rows(png_structp png);
void png_free(png_structp png);
png_voidp png_malloc_fn(png_structp png, png_size_t size);
void png_free_fn(png_structp png, png_voidp ptr);

extern const png_size_t adam_x_offsets[];
extern const png_size_t adam_y_offsets[];
extern const png_size_t adam_x_shifts[];
extern const png_size_t adam_y_shifts[];
extern const png_size_t adam_x_masks[];
extern const png_size_t adam_y_masks[];

#endif
