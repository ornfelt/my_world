#include "png.h"

#include <arpa/inet.h>

#include <stdlib.h>
#include <string.h>
#include <limits.h>

png_structp png_create_write_struct(png_const_charp version,
                                    png_voidp err_ptr,
                                    png_error_ptr err_fn,
                                    png_error_ptr warn_fn)
{
	png_structp png = calloc(1, sizeof(*png)); /* XXX malloc_fn */
	if (!png)
		return NULL;
	png->malloc_fn = png_malloc_fn;
	png->free_fn = png_free_fn;
	png->version = version;
	png->err_fn = err_fn;
	png->warn_fn = warn_fn;
	png->err_ptr = err_ptr;
	if (deflateInit(&png->zstream, 9) != Z_OK)
	{
		free(png); /* XXX free_fn */
		return NULL;
	}
	return png;
}

void png_destroy_write_struct(png_structpp png, png_infopp info)
{
	if (!png || !*png)
		return;
	if (info)
	{
		(*png)->free_fn(*png, *info);
		*info = NULL;
	}
	png_free(*png);
	*png = NULL;
}

void png_set_IHDR(png_const_structp png, png_infop info,
                  png_uint_32 width, png_uint_32 height,
                  int depth, int color_type, int interlace,
                  int compression, int filter)
{
	(void)info;
	if (depth != 8
	 && depth != 16)
	{
		png_err(png, "unsupported depth: %d", depth);
		return;
	}
	if (compression != PNG_COMPRESSION_TYPE_BASE)
	{
		png_err(png, "unsupported compression: %d",
		        compression);
		return;
	}
	if (filter != PNG_FILTER_TYPE_BASE)
	{
		png_err(png, "unsupported filter: %d",
		        filter);
		return;
	}
	if (interlace != PNG_INTERLACE_NONE
	 && interlace != PNG_INTERLACE_ADAM7)
	{
		png_err(png, "unsupported interlace: %d",
		        interlace);
		return;
	}
	switch (color_type)
	{
		case PNG_COLOR_TYPE_GRAY:
			((png_structp)png)->channels = 1;
			break;
		case PNG_COLOR_TYPE_GA:
			((png_structp)png)->channels = 2;
			break;
		case PNG_COLOR_TYPE_RGB:
			((png_structp)png)->channels = 3;
			break;
		case PNG_COLOR_TYPE_RGBA:
			((png_structp)png)->channels = 4;
			break;
		default:
			png_err(png, "unsupported color type: %d",
			        color_type);
			return;
	}
	((png_structp)png)->ihdr.width = width;
	((png_structp)png)->ihdr.height = height;
	((png_structp)png)->ihdr.depth = depth;
	((png_structp)png)->ihdr.color_type = color_type;
	((png_structp)png)->ihdr.interlace = interlace;
	((png_structp)png)->ihdr.compression = compression;
	((png_structp)png)->ihdr.filter = filter;
	if (png_alloc_rows((png_structp)png))
	{
		png_err(png, "malloc failed");
		return;
	}
}

static int write_chunk_part(png_structp png, png_const_voidp data,
                            png_size_t size, png_uint_32 *crc)
{
	if (fwrite(data, 1, size, png->fp) != size)
		return 1;
	*crc = crc32_z(*crc, data, size);
	return 0;
}

static int write_chunk(png_structp png, png_const_charp magic,
                       png_const_voidp data, png_size_t size)
{
	png_uint_32 length = ntohl(size);
	if (fwrite(&length, 1, 4, png->fp) != 4)
	{
		png_err(png, "failed to write chunk length");
		return 1;
	}
	png_uint_32 crc = crc32(0, NULL, 0);
	if (write_chunk_part(png, magic, 4, &crc))
	{
		png_err(png, "failed to write chunk magic");
		return 1;
	}
	if (size && write_chunk_part(png, data, size, &crc))
	{
		png_err(png, "failed to write chunk data");
		return 1;
	}
	crc = ntohl(crc);
	if (fwrite(&crc, 1, 4, png->fp) != 4)
	{
		png_err(png, "failed to write chunk crc");
		return 1;
	}
	return 0;
}

static int write_ihdr(png_structp png)
{
	png_byte data[13];
	*(png_uint_32*)&data[0] = ntohl(png->ihdr.width);
	*(png_uint_32*)&data[4] = ntohl(png->ihdr.height);
	data[8] = png->ihdr.depth;
	data[9] = png->ihdr.color_type;
	data[10] = png->ihdr.compression;
	data[11] = png->ihdr.filter;
	data[12] = png->ihdr.interlace;
	return write_chunk(png, "IHDR", data, 13);
}

void png_write_info(png_structp png, png_const_infop info)
{
	(void)info;
	if (fwrite(PNG_MAGIC, 1, 8, png->fp) != 8)
	{
		png_err(png, "failed to write magic");
		return;
	}
	if (write_ihdr(png))
	{
		png_err(png, "failed to write IHDR");
		return;
	}
}

static int write_data(png_structp png, int flush, png_bytepp buf,
                      png_size_t *buf_pos, png_size_t *buf_len)
{
	int ret;
	do
	{
		if (!*buf || !png->zstream.avail_out)
		{
			png_size_t newl = *buf_len + 1024 * 1024;
			png_bytep newb = png->malloc_fn(png, newl);
			if (!newb)
			{
				png_err(png, "malloc failed");
				return 1;
			}
			memmove(newb, *buf, *buf_len);
			png->free_fn(png, *buf);
			*buf = newb;
			*buf_len = newl;
		}
		png->zstream.next_out = &(*buf)[*buf_pos];
		png->zstream.avail_out = *buf_len - *buf_pos;
		ret = deflate(&png->zstream, flush);
		if (ret != Z_OK && ret != Z_STREAM_END)
		{
			png_err(png, "deflate failed");
			return 1;
		}
		*buf_pos = *buf_len - png->zstream.avail_out;
	} while (!png->zstream.avail_out || png->zstream.avail_in);
	return 0;
}

static int write_row(png_structp png,
                     png_byte filter,
                     png_const_bytep row,
                     png_size_t pitch,
                     png_bytepp buf,
                     png_size_t *buf_pos,
                     png_size_t *buf_len)
{
	png->zstream.next_in = &filter;
	png->zstream.avail_in = 1;
	if (write_data(png, Z_NO_FLUSH, buf, buf_pos, buf_len))
		return 1;
	png->zstream.next_in = (png_voidp)row;
	png->zstream.avail_in = pitch;
	if (write_data(png, Z_NO_FLUSH, buf, buf_pos, buf_len))
		return 1;
	return 0;
}

static png_byte paeth(png_byte a, png_byte b, png_byte c)
{
	png_int_16 p = (png_int_16)a + (png_int_16)b - (png_int_16)c;
	png_int_16 pa = p >= a ? p - a : a - p;
	png_int_16 pb = p >= b ? p - b : b - p;
	png_int_16 pc = p >= c ? p - c : c - p;
	if (pa <= pb && pa <= pc)
		return a;
	if (pb <= pc)
		return b;
	return c;
}

static void filter_left(png_bytep dst,
                        png_const_bytep src,
                        png_size_t bpp,
                        png_size_t pitch)
{
	png_size_t n = 0;
	while (n < bpp)
	{
		dst[n] = src[n];
		n++;
	}
	png_const_bytep left = src - bpp;
	while (n < pitch)
	{
		dst[n] = src[n] - left[n];
		n++;
	}
}

static void filter_up(png_bytep dst,
                      png_const_bytep src,
                      png_const_bytep prv,
                      png_size_t pitch)
{
	png_size_t n = 0;
	while (n < pitch)
	{
		dst[n] = src[n] - prv[n];
		n++;
	}
}

static void filter_average(png_bytep dst,
                           png_const_bytep src,
                           png_const_bytep prv,
                           png_size_t bpp,
                           png_size_t pitch)
{
	png_size_t n = 0;
	png_const_bytep left = src - bpp;
	while (n < bpp)
	{
		dst[n] = src[n] - prv[n] / 2;
		n++;
	}
	while (n < pitch)
	{
		dst[n] = src[n] - (prv[n] + left[n]) / 2;
		n++;
	}
}

static void filter_paeth(png_bytep dst,
                         png_const_bytep src,
                         png_const_bytep prv,
                         png_size_t bpp,
                         png_size_t pitch)
{
	png_size_t n = 0;
	png_const_bytep left = src - bpp;
	png_const_bytep corner = prv - bpp;
	while (n < bpp)
	{
		dst[n] = src[n] - paeth(0, prv[n], 0);
		n++;
	}
	while (n < pitch)
	{
		dst[n] = src[n] - paeth(left[n], prv[n], corner[n]);
		n++;
	}
}

static png_int_32 row_sum(png_const_bytep row,
                          png_size_t pitch)
{
	png_int_32 sum = 0;
	for (png_size_t i = 0; i < pitch; ++i)
	{
		int8_t v = ((png_const_charp)row)[i];
		if (v >= 0)
			sum += v;
		else
			sum -= v;
	}
	return sum;
}

static int filter_row(png_structp png,
                      png_const_bytep src,
                      png_size_t y,
                      png_size_t bpp,
                      png_size_t pitch,
                      png_const_bytepp row)
{
	png_bytep wrk = png->wrk_rows[png->row_ff];
	png_bytep prv = png->wrk_rows[!png->row_ff];
	if (png->ihdr.interlace == PNG_INTERLACE_ADAM7)
	{
		png_size_t x = 0;
		png_size_t spacing = adam_x_masks[png->adam_pass] * bpp;
		png_size_t xx = adam_x_offsets[png->adam_pass] * bpp;
		while (x < pitch)
		{
			for (png_size_t i = 0; i < bpp; ++i)
			{
				wrk[x] = src[xx];
				x++;
				xx++;
			}
			xx += spacing;
		}
		if (y == adam_y_offsets[png->adam_pass])
			memset(prv, 0, pitch);
	}
	else
	{
		memcpy(wrk, src, pitch);
		if (!y)
			memset(prv, 0, pitch);
	}
	filter_left(png->tmp_rows[0], wrk, bpp, pitch);
	filter_up(png->tmp_rows[1], wrk, prv, pitch);
	filter_average(png->tmp_rows[2], wrk, prv, bpp, pitch);
	filter_paeth(png->tmp_rows[3], wrk, prv, bpp, pitch);
	png_int_32 best_sum = row_sum(wrk, pitch);
	png_byte best = 0;
	for (png_size_t i = 0; i < 4; ++i)
	{
		png_int_32 sum = row_sum(png->tmp_rows[i], pitch);
		if (sum < best_sum)
		{
			best = i + 1;
			best_sum = sum;
		}
	}
	if (best)
		*row = png->tmp_rows[best - 1];
	else
		*row = wrk;
	return best;
}

void png_write_row(png_structp png, png_const_bytep row)
{
	png_write_rows(png, (png_bytepp)&row, 1);
}

void png_write_rows(png_structp png, png_bytepp rows, png_uint_32 count)
{
	png_bytep buf = NULL;
	png_size_t buf_pos = 0;
	png_size_t buf_len = 0;
	png_size_t bpp = png_get_bpp(png);
	png_size_t pitch;
	png_size_t adam_y_mask;
	png_size_t adam_y_offset;
	png_size_t written = 0;
	if (png->ihdr.interlace == PNG_INTERLACE_ADAM7)
	{
		pitch = png->ihdr.width - adam_x_offsets[png->adam_pass];
		pitch += adam_x_masks[png->adam_pass];
		pitch >>= adam_x_shifts[png->adam_pass];
		adam_y_mask = adam_y_masks[png->adam_pass];
		adam_y_offset = adam_y_offsets[png->adam_pass];
	}
	else
	{
		pitch = png->ihdr.width;
		adam_y_mask = 0;
		adam_y_offset = 0;
	}
	pitch *= bpp;
	for (png_uint_32 i = 0; i < count; ++i)
	{
		if (png->ihdr.interlace == PNG_INTERLACE_ADAM7
		 && (((png->row_idx - adam_y_offset) & adam_y_mask) || !pitch))
		{
			png->row_idx++;
			if (png->row_idx == png->ihdr.height)
			{
				png->row_idx = 0;
				png->adam_pass++;
			}
			continue;
		}
		png_const_bytep row;
		int filter = filter_row(png, rows[i], png->row_idx,
		                        bpp, pitch, &row);
		if (write_row(png, filter, row, pitch,
		              &buf, &buf_pos, &buf_len))
			goto end;
		written = 1;
		png->row_ff = !png->row_ff;
		png->row_idx++;
		if (png->ihdr.interlace == PNG_INTERLACE_ADAM7)
		{
			if (png->row_idx == png->ihdr.height)
			{
				png->row_idx = 0;
				png->adam_pass++;
			}
		}
	}
	int flush;
	if ((png->ihdr.interlace == PNG_INTERLACE_NONE && png->row_idx == png->ihdr.height)
	 || (png->ihdr.interlace == PNG_INTERLACE_ADAM7 && png->adam_pass == 7))
	{
		flush = Z_FINISH;
	}
	else
	{
		/* don't bother generating a chunk if no data has been written */
		if (!written)
			goto end;
		flush = Z_FULL_FLUSH;
	}
	png->zstream.next_in = NULL;
	png->zstream.avail_in = 0;
	if (write_data(png, flush, &buf, &buf_pos, &buf_len))
		goto end;
	if (write_chunk(png, "IDAT", buf, buf_pos))
	{
		png_err(png, "failed to write IDAT");
		goto end;
	}

end:
	png->free_fn(png, buf);
}

void png_write_image(png_structp png, png_bytepp rows)
{
	png_size_t passes = png_set_interlace_handling(png);
	for (png_size_t i = 0; i < passes; ++i)
		png_write_rows(png, rows, png->ihdr.height);
}

void png_write_end(png_structp png, png_infop info)
{
	(void)info;
	if (write_chunk(png, "IEND", NULL, 0))
	{
		png_err(png, "failed to write IEND");
		return;
	}
}
