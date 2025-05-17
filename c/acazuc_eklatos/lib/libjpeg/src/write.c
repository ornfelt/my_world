#include "jpeg.h"

static const uint8_t
zigzag_table[64] =
{
	 0,  1,  8, 16,  9,  2,  3, 10,
	17, 24, 32, 25, 18, 11,  4,  5,
	12, 19, 26, 33, 40, 48, 41, 34,
	27, 20, 13,  6,  7, 14, 21, 28,
	35, 42, 49, 56, 57, 50, 43, 36,
	29, 22, 15, 23, 30, 37, 44, 51,
	58, 59, 52, 45, 38, 31, 39, 46,
	53, 60, 61, 54, 47, 55, 62, 63,
};

static int
write_chunk(struct jpeg *jpeg, uint8_t id, size_t size)
{
	if (size > UINT16_MAX)
	{
		JPEG_ERR(jpeg, "invalid chunk length");
		return 1;
	}
	size += 2;
	if (jpeg_putc(jpeg, 0xFF)
	 || jpeg_putc(jpeg, id)
	 || jpeg_putc(jpeg, (size >> 8) & 0xFF)
	 || jpeg_putc(jpeg, (size >> 0) & 0xFF))
		return 1;
	return 0;
}

static int
write_soi(struct jpeg *jpeg)
{
	if (jpeg_putc(jpeg, 0xFF)
	 || jpeg_putc(jpeg, JPEG_CHUNK_SOI))
		return 1;
	return 0;
}

static int
write_app0(struct jpeg *jpeg)
{
	uint8_t bytes[14];
	bytes[0] = 'J';
	bytes[1] = 'F';
	bytes[2] = 'I';
	bytes[3] = 'F';
	bytes[4] = '\0';
	bytes[5] = 1;
	bytes[6] = 1;
	bytes[7] = jpeg->unit;
	bytes[8] = jpeg->density_x >> 8;
	bytes[9] = jpeg->density_x >> 0;
	bytes[10] = jpeg->density_y >> 8;
	bytes[11] = jpeg->density_y >> 0;
	bytes[12] = jpeg->thumbnail_width;
	bytes[13] = jpeg->thumbnail_height;
	size_t thumbnail_size = jpeg->thumbnail_width * jpeg->thumbnail_height;
	size_t length = sizeof(bytes) + thumbnail_size;
	if (write_chunk(jpeg, JPEG_CHUNK_APP0, length))
		return 1;
	if (fwrite(bytes, 1, sizeof(bytes), jpeg->fp) != sizeof(bytes))
	{
		JPEG_ERR(jpeg, "failed to write to file");
		return 1;
	}
	if (jpeg->thumbnail)
	{
		if (fwrite(jpeg->thumbnail, 1, thumbnail_size, jpeg->fp) != thumbnail_size)
		{
			JPEG_ERR(jpeg, "failed to write to file");
			return 1;
		}
	}
	return 0;
}

static int
write_dqt(struct jpeg *jpeg, size_t id)
{
	uint8_t dqt[64];
	for (size_t i = 0; i < 64; ++i)
		dqt[i] = jpeg->dqt[id][i];
	if (write_chunk(jpeg, JPEG_CHUNK_DQT, 64 + 1)
	 || jpeg_putc(jpeg, id))
		return 1;
	if (fwrite(&dqt[0], 1, 64, jpeg->fp) != 64)
	{
		JPEG_ERR(jpeg, "failed to write to file");
		return 1;
	}
	return 0;
}

static int
write_sof0(struct jpeg *jpeg)
{
	uint8_t bytes[15];
	size_t length = 6 + 3 * jpeg->components_count;
	bytes[0] = jpeg->bpp;
	bytes[1] = jpeg->height >> 8;
	bytes[2] = jpeg->height >> 0;
	bytes[3] = jpeg->width >> 8;
	bytes[4] = jpeg->width >> 0;
	bytes[5] = jpeg->components_count;
	for (size_t i = 0; i < jpeg->components_count; ++i)
	{
		uint8_t base = 6 + 3 * i;
		bytes[base + 0] = i + 1;
		bytes[base + 1] = (jpeg->components[i].scale_x << 4)
		                | (jpeg->components[i].scale_y << 0);
		bytes[base + 2] = jpeg->components[i].table;
	}
	if (write_chunk(jpeg, JPEG_CHUNK_SOF0, length))
		return 1;
	if (fwrite(bytes, 1, length, jpeg->fp) != length)
	{
		JPEG_ERR(jpeg, "failed to write to file");
		return 1;
	}
	return 0;
}

static int
write_dht(struct jpeg *jpeg, size_t id)
{
	struct huffman *huffman = &jpeg->huff_tables[id].huffman;
	size_t sum = 0;
	for (size_t i = 0; i < 16; ++i)
		sum += huffman->counts[i];
	uint8_t bytes[17 + 255];
	size_t length = 17 + sum;
	bytes[0] = (jpeg->huff_tables[id].class << 4)
	         | (jpeg->huff_tables[id].dst << 0);
	for (size_t i = 0; i < 16; ++i)
		bytes[i + 1] = huffman->counts[i];
	size_t n = 0;
	for (size_t i = 0; i < 16; ++i)
	{
		for (size_t j = 0; j < huffman->counts[i]; ++j)
		{
			bytes[17 + n] = huffman->values[n];
			n++;
		}
	}
	if (write_chunk(jpeg, JPEG_CHUNK_DHT, length))
		return 1;
	if (fwrite(bytes, 1, length, jpeg->fp) != length)
	{
		JPEG_ERR(jpeg, "failed to write to file");
		return 1;
	}
	return 0;
}

static int
write_dri(struct jpeg *jpeg)
{
	if (!jpeg->restart_interval)
		return 0;
	uint8_t bytes[2];
	bytes[0] = jpeg->restart_interval >> 8;
	bytes[1] = jpeg->restart_interval >> 0;
	if (write_chunk(jpeg, JPEG_CHUNK_DRI, sizeof(bytes)))
		return 1;
	if (fwrite(bytes, 1, sizeof(bytes), jpeg->fp) != sizeof(bytes))
	{
		JPEG_ERR(jpeg, "failed to write to file");
		return 1;
	}
	return 0;
}

static int
write_sos(struct jpeg *jpeg)
{
	uint8_t bytes[10];
	size_t length = 4 + 2 * jpeg->components_count;
	bytes[0] = jpeg->components_count;
	for (size_t i = 0; i < jpeg->components_count; ++i)
	{
		size_t base = 1 + 2 * i;
		bytes[base + 0] = i + 1;
		bytes[base + 1] = (jpeg->components[i].dct << 4)
		                | (jpeg->components[i].act << 0);
	}
	bytes[length - 3] = 0;
	bytes[length - 2] = 0;
	bytes[length - 1] = 0;
	if (write_chunk(jpeg, JPEG_CHUNK_SOS, length))
		return 1;
	if (fwrite(bytes, 1, length, jpeg->fp) != length)
	{
		JPEG_ERR(jpeg, "failed to write to file");
		return 1;
	}
	return 0;
}

int
jpeg_write_headers(struct jpeg *jpeg)
{
	if (!jpeg->components_count)
	{
		JPEG_ERR(jpeg, "headers not initialized");
		return 1;
	}
	if (!jpeg->fp)
	{
		JPEG_ERR(jpeg, "IO not initialized");
		return 1;
	}
	if (write_soi(jpeg))
		return 1;
	if (write_app0(jpeg))
		return 1;
	if (write_dqt(jpeg, 0))
		return 1;
	if (write_dqt(jpeg, 1))
		return 1;
	if (write_sof0(jpeg))
		return 1;
	if (write_dht(jpeg, 0))
		return 1;
	if (write_dht(jpeg, 1))
		return 1;
	if (write_dht(jpeg, 2))
		return 1;
	if (write_dht(jpeg, 3))
		return 1;
	if (write_dri(jpeg))
		return 1;
	if (write_sos(jpeg))
		return 1;
	jpeg->sos = 1;
	return 0;
}

static void
split_gray(struct jpeg *jpeg, const uint8_t *data)
{
	for (size_t y = 0; y < jpeg->height; ++y)
	{
		int32_t *Y_data = &jpeg->components[0].data[y * jpeg->component_width];
		const uint8_t *row = &data[y * jpeg->width];
		for (size_t x = 0; x < jpeg->width; ++x)
		{
			int32_t *dst = &Y_data[x];
			uint8_t v = row[x];
			dst[0] = (int32_t)v - 128;
		}
	}
	for (size_t y = jpeg->height; y < jpeg->component_height; ++y)
	{
		for (size_t x = 0; x < jpeg->width; ++x)
		{
			size_t dst = (y * jpeg->component_width + x);
			size_t src = dst - jpeg->component_width;
			jpeg->components[0].data[dst] = jpeg->components[0].data[src];
		}
	}
	for (size_t x = jpeg->width; x < jpeg->component_width; ++x)
	{
		for (size_t y = 0; y < jpeg->height; ++y)
		{
			size_t dst = (y * jpeg->component_width + x);
			size_t src = dst - 1;
			jpeg->components[0].data[dst] = jpeg->components[0].data[src];
		}
	}
	for (size_t y = jpeg->height; y < jpeg->component_height; ++y)
	{
		for (size_t x = jpeg->width; x < jpeg->component_width; ++x)
		{
			size_t dst = y * jpeg->component_width + x;
			size_t src = (jpeg->height - 1) * jpeg->component_width + jpeg->width - 1;
			jpeg->components[0].data[dst] = jpeg->components[0].data[src];
		}
	}
}

static void
split_rgb(struct jpeg *jpeg, const uint8_t *data)
{
	for (size_t y = 0; y < jpeg->height; ++y)
	{
		int32_t *Y_data = &jpeg->components[0].data[y * jpeg->component_width];
		int32_t *Cb_data = &jpeg->components[1].data[y * jpeg->component_width];
		int32_t *Cr_data = &jpeg->components[2].data[y * jpeg->component_width];
		const uint8_t *row = &data[y * jpeg->width * 3];
		for (size_t x = 0; x < jpeg->width; ++x)
		{
			const uint8_t *src = &row[x * 3];
			int32_t r = src[0] - 128;
			int32_t g = src[1] - 128;
			int32_t b = src[2] - 128;
			Y_data[x] = 0;
			Cb_data[x] = 0;
			Cr_data[x] = 0;
			Y_data[x]  += r * (int32_t)(0.299  * 65536);
			Y_data[x]  += g * (int32_t)(0.587  * 65536);
			Y_data[x]  += b * (int32_t)(0.114  * 65536);
			Cb_data[x] -= r * (int32_t)(0.1687 * 65536);
			Cb_data[x] -= g * (int32_t)(0.3313 * 65536);
			Cb_data[x] += b * (int32_t)(0.5    * 65536);
			Cr_data[x] += r * (int32_t)(0.5    * 65536);
			Cr_data[x] -= g * (int32_t)(0.4187 * 65536);
			Cr_data[x] -= b * (int32_t)(0.0813 * 65536);
			Y_data[x]  = JPEG_RSHIFT(Y_data[x], 16);
			Cb_data[x] = JPEG_RSHIFT(Cb_data[x], 16);
			Cr_data[x] = JPEG_RSHIFT(Cr_data[x], 16);
		}
	}
	for (size_t y = jpeg->height; y < jpeg->component_height; ++y)
	{
		for (size_t x = 0; x < jpeg->width; ++x)
		{
			size_t dst = (y * jpeg->component_width + x);
			size_t src = dst - jpeg->component_width;
			jpeg->components[0].data[dst] = jpeg->components[0].data[src];
			jpeg->components[1].data[dst] = jpeg->components[1].data[src];
			jpeg->components[2].data[dst] = jpeg->components[2].data[src];
		}
	}
	for (size_t x = jpeg->width; x < jpeg->component_width; ++x)
	{
		for (size_t y = 0; y < jpeg->height; ++y)
		{
			size_t dst = (y * jpeg->component_width + x);
			size_t src = dst - 1;
			jpeg->components[0].data[dst] = jpeg->components[0].data[src];
			jpeg->components[1].data[dst] = jpeg->components[1].data[src];
			jpeg->components[2].data[dst] = jpeg->components[2].data[src];
		}
	}
	for (size_t y = jpeg->height; y < jpeg->component_height; ++y)
	{
		for (size_t x = jpeg->width; x < jpeg->component_width; ++x)
		{
			size_t dst = y * jpeg->component_width + x;
			size_t src = (jpeg->height - 1) * jpeg->component_width + jpeg->width - 1;
			jpeg->components[0].data[dst] = jpeg->components[0].data[src];
			jpeg->components[1].data[dst] = jpeg->components[1].data[src];
			jpeg->components[2].data[dst] = jpeg->components[2].data[src];
		}
	}
}

static void
dct1(int32_t * restrict dst, const int32_t * restrict src)
{
	for (size_t x = 0; x < 8; ++x)
	{
		const int32_t *col = &src[x];
		int32_t v0 = col[8 * 0] * 512;
		int32_t v1 = col[8 * 1] * 512;
		int32_t v2 = col[8 * 2] * 512;
		int32_t v3 = col[8 * 3] * 512;
		int32_t v4 = col[8 * 4] * 512;
		int32_t v5 = col[8 * 5] * 512;
		int32_t v6 = col[8 * 6] * 512;
		int32_t v7 = col[8 * 7] * 512;

		int32_t z0 = v0 + v7;
		int32_t z1 = v1 + v6;
		int32_t z2 = v2 + v5;
		int32_t z3 = v3 + v4;
		int32_t z4 = v3 - v4;
		int32_t z5 = v2 - v5;
		int32_t z6 = v1 - v6;
		int32_t z7 = v0 - v7;

		int32_t t0 =  z0 + z3;
		int32_t t1 =  z1 + z2;
		int32_t t2 =  z1 - z2;
		int32_t t3 =  z0 - z3;
		int32_t t4 = -z4 - z5;
		int32_t t5 =  z6 + z7;
		int32_t t6 =  t0 + t1;
		int32_t t7 =  t0 - t1;

		int32_t t8  =  JPEG_RSHIFT((z5 + z6) * (int32_t)(0.707106 * 4096), 12);
		int32_t t9  =  JPEG_RSHIFT((t2 + t3) * (int32_t)(0.707106 * 4096), 12);
		int32_t t10 =  JPEG_RSHIFT((t4 + t5) * (int32_t)(0.382683 * 4096), 12);
		int32_t t11 = (JPEG_RSHIFT(( 0 - t4) * (int32_t)(0.541196 * 4096), 12)) - t10;
		int32_t t12 = (JPEG_RSHIFT(( 0 + t5) * (int32_t)(1.306562 * 4096), 12)) - t10;

		int32_t t13 =  t9 + t3;
		int32_t t14 =  t3 - t9;
		int32_t t15 =  t8 + z7;
		int32_t t16 =  z7 - t8;
		int32_t t17 = t11 + t16;
		int32_t t18 = t15 + t12;
		int32_t t19 = t15 - t12;
		int32_t t20 = t16 - t11;

		dst[ 0 + x] = JPEG_RSHIFT( t6 * (int32_t)(0.353553 * 2048), 12);
		dst[ 8 + x] = JPEG_RSHIFT(t18 * (int32_t)(0.254897 * 2048), 12);
		dst[16 + x] = JPEG_RSHIFT(t13 * (int32_t)(0.270598 * 2048), 12);
		dst[24 + x] = JPEG_RSHIFT(t20 * (int32_t)(0.300672 * 2048), 12);
		dst[32 + x] = JPEG_RSHIFT( t7 * (int32_t)(0.353553 * 2048), 12);
		dst[40 + x] = JPEG_RSHIFT(t17 * (int32_t)(0.449988 * 2048), 12);
		dst[48 + x] = JPEG_RSHIFT(t14 * (int32_t)(0.653281 * 2048), 12);
		dst[56 + x] = JPEG_RSHIFT(t19 * (int32_t)(1.281457 * 2048), 12);
	}
}

static void
dct2(int32_t * restrict dst, const int32_t * restrict src)
{
	for (size_t y = 0; y < 8; ++y)
	{
		const int32_t *row = &src[y * 8];
		int32_t v0 = row[0];
		int32_t v1 = row[1];
		int32_t v2 = row[2];
		int32_t v3 = row[3];
		int32_t v4 = row[4];
		int32_t v5 = row[5];
		int32_t v6 = row[6];
		int32_t v7 = row[7];

		int32_t z0 = v0 + v7;
		int32_t z1 = v1 + v6;
		int32_t z2 = v2 + v5;
		int32_t z3 = v3 + v4;
		int32_t z4 = v3 - v4;
		int32_t z5 = v2 - v5;
		int32_t z6 = v1 - v6;
		int32_t z7 = v0 - v7;

		int32_t t0 =  z0 + z3;
		int32_t t1 =  z1 + z2;
		int32_t t2 =  z1 - z2;
		int32_t t3 =  z0 - z3;
		int32_t t4 = -z4 - z5;
		int32_t t5 =  z6 + z7;
		int32_t t6 =  t0 + t1;
		int32_t t7 =  t0 - t1;

		int32_t t8  =  JPEG_RSHIFT((z5 + z6) * (int32_t)(0.707106 * 4096), 12);
		int32_t t9  =  JPEG_RSHIFT((t2 + t3) * (int32_t)(0.707106 * 4096), 12);
		int32_t t10 =  JPEG_RSHIFT((t4 + t5) * (int32_t)(0.382683 * 4096), 12);
		int32_t t11 = (JPEG_RSHIFT(( 0 - t4) * (int32_t)(0.541196 * 4096), 12)) - t10;
		int32_t t12 = (JPEG_RSHIFT(( 0 + t5) * (int32_t)(1.306562 * 4096), 12)) - t10;

		int32_t t13 =  t9 + t3;
		int32_t t14 =  t3 - t9;
		int32_t t15 =  t8 + z7;
		int32_t t16 =  z7 - t8;
		int32_t t17 = t11 + t16;
		int32_t t18 = t15 + t12;
		int32_t t19 = t15 - t12;
		int32_t t20 = t16 - t11;

		int32_t *out = &dst[y * 8];
		out[0] = JPEG_RSHIFT( t6 * (int32_t)(0.353553 * 2048), 19);
		out[1] = JPEG_RSHIFT(t18 * (int32_t)(0.254897 * 2048), 19);
		out[2] = JPEG_RSHIFT(t13 * (int32_t)(0.270598 * 2048), 19);
		out[3] = JPEG_RSHIFT(t20 * (int32_t)(0.300672 * 2048), 19);
		out[4] = JPEG_RSHIFT( t7 * (int32_t)(0.353553 * 2048), 19);
		out[5] = JPEG_RSHIFT(t17 * (int32_t)(0.449988 * 2048), 19);
		out[6] = JPEG_RSHIFT(t14 * (int32_t)(0.653281 * 2048), 19);
		out[7] = JPEG_RSHIFT(t19 * (int32_t)(1.281457 * 2048), 19);
	}
}

static void
dct(int32_t * restrict dst, const int32_t * restrict src)
{
	int32_t tmp[64];
	dct1(tmp, src);
	dct2(dst, tmp);
}

static void
zigzag(int32_t * restrict dst, int32_t * restrict src)
{
	for (size_t i = 0; i < 64; ++i)
		dst[i] = src[zigzag_table[i]];
}

static void
quantify(struct jpeg * restrict jpeg,
         size_t component,
         int32_t * restrict dst,
         int32_t * restrict src)
{
	const int32_t *iqt = &jpeg->idqt[jpeg->components[component].table][0];
	for (size_t i = 0; i < 64; ++i)
		dst[i] = JPEG_RSHIFT(src[i] * iqt[i], 16);
}

static int
encode_component(struct jpeg *jpeg, size_t component, int32_t *values)
{
	struct huffman *dct = &jpeg->huff_tables[jpeg->components[component].dc_huff].huffman;
	struct huffman *act = &jpeg->huff_tables[jpeg->components[component].ac_huff].huffman;
	struct huffman *huffman = dct;
	size_t n = 0;
	while (n < 64)
	{
		uint8_t code;
		if (n && !values[n])
		{
			uint8_t fwd_zero = 0;
			do
			{
				n++;
				fwd_zero++;
				if (n == 64)
				{
					if (bs_putbits(&jpeg->bs, huffman->map[0x00], huffman->sizes[0x00]))
						return 1;
					return 0;
				}
			} while (!values[n]);
			if (fwd_zero >= 16)
			{
				n -= fwd_zero - 16;
				if (bs_putbits(&jpeg->bs, huffman->map[0xF0], huffman->sizes[0xF0]))
					return 1;
				huffman = act;
				continue;
			}
			code = fwd_zero << 4;
		}
		else
		{
			code = 0;
		}
		uint32_t length = 0;
		uint32_t tmp = values[n] > 0 ? values[n] : -values[n];
		length = tmp ? 32 - __builtin_clz(tmp) : 0;
		uint32_t value;
		if (values[n] < 0)
			value = (1 << length) + values[n] - 1;
		else
			value = values[n];
		code |= length;
		if (bs_putbits(&jpeg->bs, huffman->map[code], huffman->sizes[code]))
			return 1;
		if (bs_putbits(&jpeg->bs, value, length))
			return 1;
		huffman = act;
		n++;
	}
	return 0;
}

static int
encode_block_component(struct jpeg *jpeg, int32_t *values, size_t component)
{
	int32_t tmp1[64];
	int32_t tmp2[64];
	dct(&tmp2[0], values);
	zigzag(&tmp1[0], &tmp2[0]);
	quantify(jpeg, component, &tmp2[0], &tmp1[0]);
	int32_t prev_dc = jpeg->components[component].prev_dc;
	jpeg->components[component].prev_dc = tmp2[0];
	tmp2[0] -= prev_dc;
	if (encode_component(jpeg, component, &tmp2[0]))
		return 1;
	return 0;
}

static void
prepare_block_component(struct jpeg *jpeg,
                        int32_t *values,
                        size_t component,
                        size_t bx,
                        size_t by)
{
	int32_t *data = jpeg->components[component].data;
	data += (jpeg->block_y + by * jpeg->components[component].height) * jpeg->component_width;
	data += jpeg->block_x + bx * jpeg->components[component].width;
	if (jpeg->components[component].width == 8
	 && jpeg->components[component].height == 8)
	{
		for (size_t y = 0; y < 8; ++y)
		{
			int32_t *src = &data[y * jpeg->component_width];
			int32_t *dst = &values[y * 8];
			for (size_t x = 0; x < 8; ++x)
				dst[x] = src[x];
		}
		return;
	}
	for (size_t y = 0; y < 8; ++y)
	{
		int32_t *dst = &values[y * 8];
		for (size_t x = 0; x < 8; ++x)
		{
			dst[x] = 0;
			size_t startx = x * jpeg->components[component].width / 8;
			size_t endx = (x + 1) * jpeg->components[component].width / 8;
			size_t starty = y * jpeg->components[component].height / 8;
			size_t endy = (y + 1) * jpeg->components[component].height / 8;
			for (size_t yy = starty; yy < endy; ++yy)
			{
				int32_t *src = &data[yy * jpeg->component_width];
				for (size_t xx = startx; xx < endx; ++xx)
					dst[x] += src[xx];
			}
			dst[x] /= (endx - startx) * (endy - starty);
		}
	}
}

static int
encode_block(struct jpeg *jpeg)
{
	for (size_t i = 0; i < jpeg->components_count; ++i)
	{
		for (size_t y = 0; y < jpeg->components[i].scale_y; ++y)
		{
			for (size_t x = 0; x < jpeg->components[i].scale_x; ++x)
			{
				int32_t values[64];
				prepare_block_component(jpeg, values, i, x, y);
				if (encode_block_component(jpeg, values, i))
					return 1;
			}
		}
	}
	if (jpeg->restart_count)
	{
		if (jpeg->restart_interval == 1)
		{
			jpeg->restart_interval = jpeg->restart_count;
			for (size_t i = 0; i < jpeg->components_count; ++i)
				jpeg->components[i].prev_dc = 0;
			if (bs_flushbits(&jpeg->bs)
			 || jpeg_putc(jpeg, 0xFF)
			 || jpeg_putc(jpeg, JPEG_CHUNK_RST0 + jpeg->restart_id))
				return 1;
			jpeg->restart_id = (jpeg->restart_id + 1) % 8;
		}
		else
		{
			jpeg->restart_interval--;
		}
	}
	jpeg->block_x += jpeg->block_width;
	if (jpeg->block_x >= jpeg->width)
	{
		jpeg->block_x = 0;
		jpeg->block_y += jpeg->block_height;
	}
	return 0;
}

int
jpeg_write_data(struct jpeg *jpeg, const void *data)
{
	if (!jpeg->components_count)
	{
		JPEG_ERR(jpeg, "headers not initialized");
		return 1;
	}
	if (!jpeg->fp)
	{
		JPEG_ERR(jpeg, "IO not initialized");
		return 1;
	}
	if (!jpeg->sos)
	{
		JPEG_ERR(jpeg, "headers not written");
		return 1;
	}
	for (size_t i = 0; i < 64; ++i)
	{
		jpeg->idqt[0][i] = 65536 / jpeg->dqt[0][i];
		jpeg->idqt[1][i] = 65536 / jpeg->dqt[1][i];
	}
	if (jpeg->components_count == 1)
		split_gray(jpeg, data);
	else
		split_rgb(jpeg, data);
	jpeg->restart_count = jpeg->restart_interval;
	bs_init_write(&jpeg->bs);
	jpeg->restart_id = 0;
	while (jpeg->block_y < jpeg->height)
	{
		if (encode_block(jpeg))
			return 1;
	}
	if (bs_flushbits(&jpeg->bs)
	 || jpeg_putc(jpeg, 0xFF)
	 || jpeg_putc(jpeg, JPEG_CHUNK_EOI))
		return 1;
	return 0;
}
