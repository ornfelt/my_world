#include "png.h"

#include <arpa/inet.h>

#include <inttypes.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define IS_CHUNK(t, a, b, c, d) \
   ((t)[0] == (a) \
 && (t)[1] == (b) \
 && (t)[2] == (c) \
 && (t)[3] == (d))

static int
parse_ihdr(png_structp png,
           png_const_bytep payload,
           png_uint_32 length)
{
	if (length != 13)
	{
		png_err(png, "invalid ihdr length: %" PRIu32, length);
		return 1;
	}
	png->ihdr.width = ntohl(*(png_uint_32*)&payload[0]);
	png->ihdr.height = ntohl(*(png_uint_32*)&payload[4]);
	png->ihdr.depth = payload[8];
	png->ihdr.color_type = payload[9];
	png->ihdr.compression = payload[10];
	png->ihdr.filter = payload[11];
	png->ihdr.interlace = payload[12];
	if (png->ihdr.depth != 8
	 && png->ihdr.depth != 16)
	{
		png_err(png, "unsupported depth: %" PRIu8, png->ihdr.depth);
		return 1;
	}
	switch (png->ihdr.color_type)
	{
		case PNG_COLOR_TYPE_GRAY:
			png->channels = 1;
			break;
		case PNG_COLOR_TYPE_GA:
			png->channels = 2;
			break;
		case PNG_COLOR_TYPE_RGB:
			png->channels = 3;
			break;
		case PNG_COLOR_TYPE_RGBA:
			png->channels = 4;
			break;
		default:
			png_err(png, "unsupported color type: %" PRIu8,
			        png->ihdr.color_type);
			return 1;
	}
	if (png->ihdr.compression != PNG_COMPRESSION_TYPE_BASE)
	{
		png_err(png, "unsupported compression: %" PRIu8,
		        png->ihdr.compression);
		return 1;
	}
	if (png->ihdr.filter != PNG_FILTER_TYPE_BASE)
	{
		png_err(png, "unsupported filter: %" PRIu8,
		        png->ihdr.filter);
		return 1;
	}
	if (png->ihdr.interlace != PNG_INTERLACE_NONE
	 && png->ihdr.interlace != PNG_INTERLACE_ADAM7)
	{
		png_err(png, "unsupported interlace: %" PRIu8,
		        png->ihdr.interlace);
		return 1;
	}
#if 0
	printf("width: %" PRIu32 "\n", png->ihdr.width);
	printf("height: %" PRIu32 "\n", png->ihdr.height);
	printf("depth: %" PRIu8 "\n", png->ihdr.depth);
	printf("color type: %" PRIu8 "\n", png->ihdr.color_type);
	printf("compression: %" PRIu8 "\n", png->ihdr.compression);
	printf("filter: %" PRIu8 "\n", png->ihdr.filter);
	printf("interlace: %" PRIu8 "\n", png->ihdr.interlace);
#endif
	png->data_size = png->ihdr.height * png_get_rowbytes(png, NULL);
	if (png->ihdr.interlace == PNG_INTERLACE_ADAM7)
	{
		for (png_size_t i = 0; i < 7; ++i)
		{
			if (adam_x_offsets[i] < png->ihdr.width)
				png->data_size += (png->ihdr.height - adam_y_offsets[i] + adam_y_masks[i]) >> adam_y_shifts[i];
		}
	}
	else
	{
		png->data_size += png->ihdr.height;
	}
	png->data = png->malloc_fn(png, png->data_size);
	if (!png->data
	 || png_alloc_rows(png))
	{
		png_err(png, "malloc failed");
		return 1;
	}
	png->data_it = png->data;
	png->adam_pass = 0;
	return 0;
}

static int
parse_idat(struct png_struct *png,
           png_const_voidp payload,
           png_size_t length)
{
	png->zstream.next_in = (png_voidp)payload;
	png->zstream.avail_in = length;
	png->zstream.next_out = &((png_bytep)png->data)[png->data_pos];
	png->zstream.avail_out = png->data_size - png->data_pos;
	int ret = inflate(&png->zstream, Z_NO_FLUSH);
	if (ret != Z_OK && ret != Z_STREAM_END)
	{
		png_err(png, "zlib error: %d", ret);
		return 1;
	}
	if (png->zstream.avail_in)
	{
		png_err(png, "zlib seems stuffed UwU: %u\n",
		        (unsigned)png->zstream.avail_in);
		return 1;
	}
	png->data_pos = png->data_size - png->zstream.avail_out;
	return 0;
}

static int
parse_chunk(png_structp png)
{
	png_uint_32 length;
	png_bytep payload = NULL;
	png_byte type[4];
	int ret = -1;

	if (fread(&length, 1, 4, png->fp) != 4)
	{
		png_err(png, "failed to read chunk length");
		goto end;
	}
	length = ntohl(length);
	if (fread(&type, 1, 4, png->fp) != 4)
	{
		png_err(png, "failed to read chunk type");
		goto end;
	}
#if 0
	printf("chunk %c%c%c%c\n", type[0], type[1], type[2], type[3]);
#endif
	if (IS_CHUNK(type, 'I', 'E', 'N', 'D'))
		return 1;
	payload = png->malloc_fn(png, length);
	if (!payload)
	{
		png_err(png, "malloc failed");
		goto end;
	}
	if (fread(payload, 1, length, png->fp) != length)
	{
		png_err(png, "failed to read chunk data");
		goto end;
	}
	png_uint_32 crc;
	if (fread(&crc, 1, 4, png->fp) != 4)
	{
		png_err(png, "failed to read chunk crc");
		goto end;
	}
	crc = ntohl(crc);
	if (crc32_z(crc32(crc32(0, NULL, 0), type, 4), payload, length) != crc)
	{
		png_err(png, "invalid chunk crc");
		goto end;
	}
	if (!png->parsed_ihdr)
	{
		if (IS_CHUNK(type, 'I', 'H', 'D', 'R'))
		{
			if (parse_ihdr(png, payload, length))
				goto end;
			png->parsed_ihdr = 1;
		}
		else
		{
			png_err(png, "first chunk isn't ihdr");
			goto end;
		}
	}
	else if (IS_CHUNK(type, 'I', 'H', 'D', 'R'))
	{
		png_err(png, "multiple ihdr");
		goto end;
	}
	else if (IS_CHUNK(type, 'I', 'D', 'A', 'T'))
	{
		if (parse_idat(png, payload, length))
			goto end;
	}
	else if (!(type[0] & (1 << 5)))
	{
		png_err(png, "unhandled critical chunk");
		goto end;
	}
	ret = 0;

end:
	png->free_fn(png, payload);
	return ret;
}

static png_byte
paeth(png_byte a, png_byte b, png_byte c)
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

static void
defilter_left(png_bytep dst,
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
	png_const_bytep left = dst - bpp;
	while (n < pitch)
	{
		dst[n] = src[n] + left[n];
		n++;
	}
}

static void
defilter_up(png_bytep dst,
            png_const_bytep src,
            png_const_bytep prv,
            png_size_t y,
            png_size_t pitch)
{
	if (!y)
	{
		memcpy(dst, src, pitch);
		return;
	}
	png_size_t n = 0;
	while (n < pitch)
	{
		dst[n] = src[n] + prv[n];
		n++;
	}
}

static void
defilter_average(png_bytep dst,
                 png_const_bytep src,
                 png_const_bytep prv,
                 png_size_t y,
                 png_size_t bpp,
                 png_size_t pitch)
{
	png_size_t n = 0;
	png_const_bytep left = dst - bpp;
	if (!y)
	{
		while (n < bpp)
		{
			dst[n] = src[n];
			n++;
		}
		while (n < pitch)
		{
			dst[n] = src[n] + left[n] / 2;
			n++;
		}
		return;
	}
	while (n < bpp)
	{
		dst[n] = src[n] + prv[n] / 2;
		n++;
	}
	while (n < pitch)
	{
		dst[n] = src[n] + (left[n] + prv[n]) / 2;
		n++;
	}
}

static void
defilter_paeth(png_bytep dst,
               png_const_bytep src,
               png_const_bytep prv,
               png_size_t y,
               png_size_t bpp,
               png_size_t pitch)
{
	png_size_t n = 0;
	png_const_bytep left = dst - bpp;
	png_const_bytep corner = prv - bpp;
	if (!y)
	{
		while (n < bpp)
		{
			dst[n] = src[n] + paeth(0, 0, 0);
			n++;
		}
		while (n < pitch)
		{
			dst[n] = src[n] + paeth(left[n], 0, 0);
			n++;
		}
		return;
	}
	while (n < bpp)
	{
		dst[n] = src[n] + paeth(0, prv[n], 0);
		n++;
	}
	while (n < pitch)
	{
		dst[n] = src[n] + paeth(left[n], prv[n], corner[n]);
		n++;
	}
}

png_structp
png_create_read_struct(png_const_charp version,
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
	if (inflateInit(&png->zstream) != Z_OK)
	{
		free(png); /* XXX free_fn */
		return NULL;
	}
	return png;
}

void
png_destroy_read_struct(png_structpp png,
                        png_infopp info,
                        png_infopp end)
{
	if (!png || !*png)
		return;
	if (info)
	{
		(*png)->free_fn(*png, *info);
		*info = NULL;
	}
	if (end)
	{
		(*png)->free_fn(*png, *end);
		*end = NULL;
	}
	png_free(*png);
	*png = NULL;
}

void
png_set_sig_bytes(png_structp png, int bytes)
{
	if (bytes < 0)
		bytes = 0;
	if (bytes > 8)
		bytes = 8;
	png->sig_bytes = bytes;
}

void
png_read_info(png_structp png, png_infop info)
{
	(void)info;
	if (png->sig_bytes < 8)
	{
		png_byte magic[8];
		if (fread(magic, 1, 8 - png->sig_bytes, png->fp) != 8)
		{
			png_err(png, "failed to read magic");
			return;
		}
		if (png_sig_cmp(magic, png->sig_bytes, 8 - png->sig_bytes))
		{
			png_err(png, "invalid magic");
			return;
		}
		png->sig_bytes = 8;
	}
	while (1)
	{
		switch (parse_chunk(png))
		{
			case -1:
				return;
			case 0:
				break;
			case 1:
				if (png->data_pos < png->data_size)
				{
					png_err(png, "not enough IDAT data");
					return;
				}
				inflateEnd(&png->zstream);
				return;
		}
	}
}

png_uint_32
png_get_IHDR(png_structp png,
             png_infop info,
             png_uint_32p width,
             png_uint_32p height,
             int *depth,
             int *color_type,
             int *interlace,
             int *compression,
             int *filter)
{
	(void)info;
	if (!png->parsed_ihdr)
		return 0;
	if (width)
		*width = png->ihdr.width;
	if (height)
		*height = png->ihdr.height;
	if (depth)
		*depth = png->ihdr.depth;
	if (color_type)
		*color_type = png->ihdr.color_type;
	if (compression)
		*compression = png->ihdr.compression;
	if (filter)
		*filter = png->ihdr.filter;
	if (interlace)
		*interlace = png->ihdr.interlace;
	return 1;
}

void
png_read_row(png_structp png, png_bytep row, png_bytep display_row)
{
	png_read_rows(png, &row, &display_row, 1);
}

static void
adam7_sparkle(png_structp png,
              png_bytep row,
              png_bytep src,
              png_size_t bpp,
              png_size_t adam_x_offset,
              png_size_t adam_x_mask)
{
	png_size_t x = adam_x_offset;
	png_size_t spacing = adam_x_mask;
	png_size_t xx = 0;
	png_size_t pitch = png->ihdr.width * bpp;
	while (x < pitch)
	{
		for (png_size_t i = 0; i < bpp; ++i)
		{
			row[x] = src[xx];
			x++;
			xx++;
		}
		x += spacing;
	}
}

static void
adam7_rectangle(png_structp png,
                png_bytep row,
                png_bytep src,
                png_size_t bpp,
                png_size_t adam_x_offset,
                png_size_t adam_x_mask)
{
	png_size_t x = adam_x_offset;
	png_size_t spacing = adam_x_mask;
	png_size_t xx = 0;
	png_size_t pitch = png->ihdr.width * bpp;
	png_size_t repeat = spacing - x;
	spacing -= repeat;
	while (x < pitch)
	{
		for (png_size_t i = 0; i < bpp; ++i)
		{
			row[x] = src[xx];
			x++;
			xx++;
		}
		for (png_size_t i = 0; i < repeat; ++i)
		{
			if (x >= pitch)
				return;
			row[x] = row[x - bpp];
			x++;
		}
		x += spacing;
	}
}

void
png_read_rows(png_structp png,
              png_bytepp rows,
              png_bytepp display_rows,
              png_uint_32 count)
{
	(void)display_rows;
	png_size_t bpp = png_get_bpp(png);
	png_size_t pitch;
	png_size_t adam_y_mask;
	png_size_t adam_y_offset;
	png_size_t adam_x_mask;
	png_size_t adam_x_offset;
	if (png->ihdr.interlace == PNG_INTERLACE_ADAM7)
	{
		pitch = png->ihdr.width - adam_x_offsets[png->adam_pass];
		pitch += adam_x_masks[png->adam_pass];
		pitch >>= adam_x_shifts[png->adam_pass];
		adam_y_mask = adam_y_masks[png->adam_pass];
		adam_y_offset = adam_y_offsets[png->adam_pass];
		adam_x_mask = adam_x_masks[png->adam_pass] * bpp;
		adam_x_offset = adam_x_offsets[png->adam_pass] * bpp;
	}
	else
	{
		pitch = png->ihdr.width;
		adam_y_mask = 0;
		adam_y_offset = 0;
		adam_x_mask = 0;
		adam_x_offset = 0;
	}
	pitch *= bpp;
	png_const_bytep src = png->data_it;
	for (png_size_t y = 0; y < count; ++y)
	{
		if (png->ihdr.interlace == PNG_INTERLACE_ADAM7
		 && (((png->row_idx - adam_y_offset) & adam_y_mask) || !pitch))
		{
			if (display_rows
			 && (png->row_idx & adam_y_mask) >= adam_y_offset)
			{
				png_bytep prv = png->wrk_rows[!png->row_ff];
				adam7_rectangle(png, display_rows[y], prv, bpp,
				                adam_x_offset, adam_x_mask);
			}
			png->row_idx++;
			if (png->row_idx == png->ihdr.height)
			{
				png->row_idx = 0;
				png->adam_pass++;
			}
			continue;
		}
		png_byte filter = *src;
		src++;
		png_bytep dst = png->wrk_rows[png->row_ff];
		png_bytep prv = png->wrk_rows[!png->row_ff];
		switch (filter)
		{
			case 0:
				memcpy(dst, src, pitch);
				break;
			case 1:
				defilter_left(dst, src, bpp, pitch);
				break;
			case 2:
				defilter_up(dst, src, prv, png->row_idx, pitch);
				break;
			case 3:
				defilter_average(dst, src, prv, png->row_idx, bpp, pitch);
				break;
			case 4:
				defilter_paeth(dst, src, prv, png->row_idx, bpp, pitch);
				break;
			default:
				png_err(png, "unknown filtering: %u", filter);
				return;
		}
		png->row_idx++;
		if (png->ihdr.interlace == PNG_INTERLACE_ADAM7)
		{
			if (rows)
				adam7_sparkle(png, rows[y], dst, bpp,
				              adam_x_offset, adam_x_mask);
			if (display_rows)
				adam7_rectangle(png, display_rows[y], dst, bpp,
				                adam_x_offset, adam_x_mask);
			if (png->row_idx == png->ihdr.height)
			{
				png->row_idx = 0;
				png->adam_pass++;
			}
		}
		else
		{
			memcpy(rows[y], dst, pitch);
		}
		src += pitch;
		png->row_ff = !png->row_ff;
	}
	png->data_it = src;
}

void
png_read_image(png_structp png, png_bytepp rows)
{
	png_size_t passes = png_set_interlace_handling(png);
	for (png_size_t i = 0; i < passes; ++i)
		png_read_rows(png, rows, NULL, png->ihdr.height);
}

void
png_read_update_info(png_structp png, png_infop info)
{
	(void)png;
	(void)info;
}
