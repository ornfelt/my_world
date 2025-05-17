#include "jpeg.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>

static const uint8_t
inv_zigzag_table[64] =
{
	 0,  1,  5,  6, 14, 15, 27, 28,
	 2,  4,  7, 13, 16, 26, 29, 42,
	 3,  8, 12, 17, 25, 30, 41, 43,
	 9, 11, 18, 24, 31, 40, 44, 53,
	10, 19, 23, 32, 39, 45, 52, 54,
	20, 22, 33, 38, 46, 51, 55, 60,
	21, 34, 37, 47, 50, 56, 59, 61,
	35, 36, 48, 49, 57, 58, 62, 63,
};

static int
parse_app0(struct jpeg *jpeg, uint16_t length)
{
	if (length < 16)
	{
		JPEG_ERR(jpeg, "invalid APP0 length");
		return 1;
	}
	uint8_t bytes[14];
	if (fread(&bytes, 1, sizeof(bytes), jpeg->fp) != sizeof(bytes))
	{
		if (ferror(jpeg->fp))
			JPEG_ERR(jpeg, "failed to read from file");
		else
			JPEG_ERR(jpeg, "unexpected APP0 EOF");
		return 1;
	}
	if (bytes[0] != 'J' || bytes[1] != 'F'
	 || bytes[2] != 'I' || bytes[3] != 'F'
	 || bytes[4] != '\0')
	{
		JPEG_ERR(jpeg, "invalid JFIF magic");
		return 1;
	}
	if (bytes[5] != 1 || bytes[6] != 1)
	{
		JPEG_ERR(jpeg, "invalid APP0 version");
		return 1;
	}
	if (bytes[7] > 2)
	{
		JPEG_ERR(jpeg, "invalid APP0 unit");
		return 1;
	}
	jpeg->unit = bytes[7];
	jpeg->density_x = (bytes[8] << 8) | bytes[9];
	jpeg->density_y = (bytes[10] << 8) | bytes[11];
	jpeg->thumbnail_width = bytes[12];
	jpeg->thumbnail_height = bytes[13];
	if (jpeg->thumbnail_width && jpeg->thumbnail_height)
	{
		size_t thumbnail_size = jpeg->thumbnail_width
		                      * jpeg->thumbnail_height;
		if (length != 16 + thumbnail_size)
		{
			JPEG_ERR(jpeg, "invalid APP0 thumbnail length");
			return 1;
		}
		free(jpeg->thumbnail);
		jpeg->thumbnail = malloc(thumbnail_size);
		if (!jpeg->thumbnail)
		{
			JPEG_ERR(jpeg, "thumbnail allocation failed");
			return 1;
		}
		if (fread(jpeg->thumbnail, 1, thumbnail_size, jpeg->fp) != thumbnail_size)
		{
			if (ferror(jpeg->fp))
				JPEG_ERR(jpeg, "failed to read from file");
			else
				JPEG_ERR(jpeg, "unexpected APP0 thumbnail EOF");
			return 1;
		}
	}
	else
	{
		free(jpeg->thumbnail);
		jpeg->thumbnail = NULL;
	}
	return 0;
}

static int
parse_dqt(struct jpeg *jpeg, uint16_t length)
{
	if (length != 67)
	{
		JPEG_ERR(jpeg, "invalid DQT length");
		return 1;
	}
	int dst = jpeg_getc(jpeg);
	if (dst == -1)
		return 1;
	if (dst < 0 || dst > 1)
	{
		JPEG_ERR(jpeg, "invalid DQT destination");
		return 1;
	}
	uint8_t dqt[64];
	if (fread(&dqt[0], 1, 64, jpeg->fp) != 64)
	{
		JPEG_ERR(jpeg, "unexpected EOF");
		return 1;
	}
	for (size_t i = 0; i < 64; ++i)
	{
		if (!dqt[i])
		{
			JPEG_ERR(jpeg, "invalid dqt values");
			return 1;
		}
		jpeg->dqt[dst][i] = dqt[i];
	}
	return 0;
}

static int
parse_sof0(struct jpeg *jpeg, uint16_t length)
{
	if (length != 17 && length != 11)
	{
		JPEG_ERR(jpeg, "invalid SOF0 length");
		return 1;
	}
	uint8_t bytes[15];
	if (fread(&bytes, 1, length - 2, jpeg->fp) != length - 2u)
	{
		if (ferror(jpeg->fp))
			JPEG_ERR(jpeg, "failed to read from file");
		else
			JPEG_ERR(jpeg, "unexpected SOF0 EOF");
		return 1;
	}
	jpeg->bpp = bytes[0];
	jpeg->height = (bytes[1] << 8) | bytes[2];
	jpeg->width = (bytes[3] << 8) | bytes[4];
	jpeg->components_count = bytes[5];
	if (jpeg->bpp != 8)
	{
		JPEG_ERR(jpeg, "invalid SOF0 bpp");
		return 1;
	}
	if (jpeg->components_count != 3
	 && jpeg->components_count != 1)
	{
		JPEG_ERR(jpeg, "invalid SOF0 components count");
		return 1;
	}
	for (size_t i = 0; i < jpeg->components_count; ++i)
	{
		uint8_t base = 6 + i * 3;
		uint8_t id = bytes[base];
		if (!id || id > jpeg->components_count)
		{
			JPEG_ERR(jpeg, "invalid SOF0 component id");
			return 1;
		}
		id--;
		if (jpeg->components[id].data)
		{
			JPEG_ERR(jpeg, "duplicated SOF0 component");
			return 1;
		}
		jpeg->components[id].scale_x = bytes[base + 1] >> 4;
		jpeg->components[id].scale_y = bytes[base + 1] & 0xF;
		jpeg->components[id].table = bytes[base + 2];
		if (jpeg->components[id].table > 1)
		{
			JPEG_ERR(jpeg, "invalid SOF0 component table");
			return 1;
		}
		if (jpeg->components[id].scale_x != 1
		 && jpeg->components[id].scale_x != 2
		 && jpeg->components[id].scale_x != 4)
		{
			JPEG_ERR(jpeg, "invalid SOF0 component scale x");
			return 1;
		}
		if (jpeg->components[id].scale_y != 1
		 && jpeg->components[id].scale_y != 2)
		{
			JPEG_ERR(jpeg, "invalid SOF0 component scale y");
			return 1;
		}
	}
	if (jpeg->components_count > 1)
	{
		if (jpeg->components[1].scale_x > jpeg->components[0].scale_x
		 || jpeg->components[1].scale_y > jpeg->components[0].scale_y
		 || jpeg->components[2].scale_x > jpeg->components[0].scale_x
		 || jpeg->components[2].scale_y > jpeg->components[0].scale_y)
		{
			JPEG_ERR(jpeg, "invalid SOF0 components scales");
			return 1;
		}
	}
	jpeg->block_width = 8 * jpeg->components[0].scale_x;
	jpeg->block_height = 8 * jpeg->components[0].scale_y;
	jpeg->component_width = jpeg->width + jpeg->block_width - 1;
	jpeg->component_width -= jpeg->component_width % jpeg->block_width;
	jpeg->component_height = jpeg->height + jpeg->block_height - 1;
	jpeg->component_height -= jpeg->component_height % jpeg->block_height;
	for (size_t i = 0; i < jpeg->components_count; ++i)
	{
		jpeg->components[i].width = jpeg->block_width / jpeg->components[i].scale_x;
		jpeg->components[i].height = jpeg->block_height / jpeg->components[i].scale_y;
		free(jpeg->components[i].data);
		jpeg->components[i].data = malloc(jpeg->component_width
		                                * jpeg->component_height
		                                * sizeof(*jpeg->components[i].data));
		if (!jpeg->components[i].data)
		{
			JPEG_ERR(jpeg, "component data allocation failed");
			return 1;
		}
	}
	return 0;
}

static int
parse_dht(struct jpeg *jpeg, uint16_t length)
{
	if (jpeg->huff_count >= 4)
	{
		JPEG_ERR(jpeg, "too much DHT found");
		return 1;
	}
	if (length < 19 || length > 19 + 255)
	{
		JPEG_ERR(jpeg, "invalid DHT length");
		return 1;
	}
	uint8_t bytes[17 + 255];
	if (fread(&bytes, 1, length - 2, jpeg->fp) != length - 2u)
	{
		if (ferror(jpeg->fp))
			JPEG_ERR(jpeg, "failed to read from file");
		else
			JPEG_ERR(jpeg, "unexpected DHT EOF");
		return 1;
	}
	jpeg->huff_tables[jpeg->huff_count].class = bytes[0] >> 4;
	jpeg->huff_tables[jpeg->huff_count].dst = bytes[0] & 0xF;
	size_t sum = 0;
	for (size_t i = 0; i < 16; ++i)
		sum += bytes[1 + i];
	if (length != 19 + sum)
	{
		JPEG_ERR(jpeg, "invalid DHT length");
		return 1;
	}
	huffman_generate(&jpeg->huff_tables[jpeg->huff_count].huffman, &bytes[1], &bytes[17]);
	jpeg->huff_count++;
	return 0;
}

static int
parse_sos(struct jpeg *jpeg, uint16_t length)
{
	if (length != 12 && length != 8)
	{
		JPEG_ERR(jpeg, "invalid SOS length");
		return 1;
	}
	uint8_t bytes[10];
	if (fread(&bytes, 1, length - 2, jpeg->fp) != length - 2u)
	{
		if (ferror(jpeg->fp))
			JPEG_ERR(jpeg, "failed to read from file");
		else
			JPEG_ERR(jpeg, "unexpected SOS EOF");
		return 1;
	}
	if (bytes[0] != jpeg->components_count)
	{
		JPEG_ERR(jpeg, "invalid SOS components count");
		return 1;
	}
	for (size_t i = 0; i < bytes[0]; ++i)
	{
		uint8_t base = 1 + i * 2;
		uint8_t id = bytes[base];
		if (id <= 0 || id > bytes[0])
		{
			JPEG_ERR(jpeg, "invalid SOS component id");
			return 1;
		}
		id--;
		jpeg->components[id].dct = bytes[base + 1] >> 4;
		jpeg->components[id].act = bytes[base + 1] & 0xF;
	}
	/* XXX spectral select, successive approx */
	return 0;
}

static int
parse_dri(struct jpeg *jpeg, uint16_t length)
{
	if (length != 4)
	{
		JPEG_ERR(jpeg, "invalid DRI length");
		return 1;
	}
	uint8_t bytes[2];
	if (fread(&bytes, 1, sizeof(bytes), jpeg->fp) != sizeof(bytes))
	{
		if (ferror(jpeg->fp))
			JPEG_ERR(jpeg, "failed to read from file");
		else
			JPEG_ERR(jpeg, "unexpected DRI EOF");
		return 1;
	}
	jpeg->restart_interval = (bytes[0] << 8) | bytes[1];
	return 0;
}

static int
get_huffman(struct jpeg *jpeg, size_t class, size_t dst)
{
	for (size_t i = 0; i < jpeg->huff_count; ++i)
	{
		if (jpeg->huff_tables[i].class == class
		 && jpeg->huff_tables[i].dst == dst)
			return i;
	}
	JPEG_ERR(jpeg, "failed to find huffman table");
	return -1;
}

int
jpeg_read_headers(struct jpeg *jpeg)
{
	if (!jpeg->fp)
	{
		JPEG_ERR(jpeg, "IO not initialized");
		return 1;
	}
	jpeg->components_count = 0;
	jpeg->huff_count = 0;
	int soi = 0;
	while (!jpeg->sos)
	{
		uint8_t chunk[2];
		if (fread(chunk, 1, 2, jpeg->fp) != 2)
		{
			if (ferror(jpeg->fp))
				JPEG_ERR(jpeg, "failed to read from file");
			else
				JPEG_ERR(jpeg, "unexpected EOF");
			return 1;
		}
		if (chunk[0] != 0xFF)
		{
			JPEG_ERR(jpeg, "invalid chunk mark");
			return 1;
		}
		if (chunk[1] == JPEG_CHUNK_SOI)
		{
			JPEG_ERR(jpeg, "multiple SOI");
			soi = 1;
			continue;
		}
		if (!soi)
		{
			JPEG_ERR(jpeg, "SOI not found");
			return 1;
		}
		uint8_t length_bytes[2];
		if (fread(length_bytes, 1, 2, jpeg->fp) != 2)
		{
			if (ferror(jpeg->fp))
				JPEG_ERR(jpeg, "failed to read from file");
			else
				JPEG_ERR(jpeg, "unexpected chunk length EOF");
			return 1;
		}
		uint16_t length = (length_bytes[0] << 8) | length_bytes[1];
		switch (chunk[1])
		{
			case JPEG_CHUNK_APP0:
				if (parse_app0(jpeg, length))
					return 1;
				break;
			case JPEG_CHUNK_APP2:
				fseek(jpeg->fp, length - 2, SEEK_CUR);
				break;
			case JPEG_CHUNK_DQT:
				if (parse_dqt(jpeg, length))
					return 1;
				break;
			case JPEG_CHUNK_SOF0:
				if (parse_sof0(jpeg, length))
					return 1;
				break;
			case JPEG_CHUNK_DHT:
				if (parse_dht(jpeg, length))
					return 1;
				break;
			case JPEG_CHUNK_SOS:
				if (parse_sos(jpeg, length))
					return 1;
				jpeg->sos = 1;
				break;
			case JPEG_CHUNK_DRI:
				if (parse_dri(jpeg, length))
					return 1;
				break;
			default:
				fseek(jpeg->fp, length - 2, SEEK_CUR);
				break;
		}
	}
	if (!jpeg->huff_count)
	{
		JPEG_ERR(jpeg, "no huffman table defined");
		return 1;
	}
	if (!jpeg->components_count)
	{
		JPEG_ERR(jpeg, "no component read");
		return 1;
	}
	for (size_t i = 0; i < jpeg->components_count; ++i)
	{
		if (!jpeg->dqt[jpeg->components[i].table][0])
		{
			JPEG_ERR(jpeg, "missing dqt");
			return 1;
		}
		int dct = get_huffman(jpeg, 0, jpeg->components[i].dct);
		int act = get_huffman(jpeg, 1, jpeg->components[i].act);
		if (dct == -1 || act == -1)
		{
			JPEG_ERR(jpeg, "missing huffman table");
			return 1;
		}
		jpeg->components[i].dc_huff = dct;
		jpeg->components[i].ac_huff = act;
	}
	return 0;
}

static int
huffman_decode(struct jpeg *jpeg, struct huffman *huff)
{
	uint16_t value = 0;
	size_t i = 0;
	while (1)
	{
		int ret = bs_getbit(&jpeg->bs);
		if (ret < 0)
			return ret;
		value |= ret;
		if (value < huff->maxcodes[i])
			break;
		value <<= 1;
		i++;
	}
	if (value < huff->codes[i])
	{
		JPEG_ERR(jpeg, "invalid huffman code");
		return -1;
	}
	return huff->values[huff->offsets[i] + (value - huff->codes[i])];
}

static int
decode_component(struct jpeg *jpeg, size_t component, int32_t *values)
{
	struct huffman *dct = &jpeg->huff_tables[jpeg->components[component].dc_huff].huffman;
	struct huffman *act = &jpeg->huff_tables[jpeg->components[component].ac_huff].huffman;
	struct huffman *huffman = dct;
	size_t n = 0;
	while (n < 64)
	{
		int code = huffman_decode(jpeg, huffman);
		if (code == -1)
			return 1;
		huffman = act;
		if (n)
		{
			uint8_t nzero;
			if (!code)
				nzero = 64 - n - 1;
			else
				nzero = (code & 0xF0) >> 4;
			if (nzero)
			{
				if (!n)
				{
					JPEG_ERR(jpeg, "rle on dct");
					return 1;
				}
				if (n + nzero >= 64)
				{
					JPEG_ERR(jpeg, "rle overflow %d + %d", (int)n, (int)nzero);
					return 1;
				}
				for (size_t i = 0; i < nzero; ++i)
					values[n++] = 0;
			}
			code &= 0xF;
		}
		if (!code)
		{
			values[n++] = 0;
			continue;
		}
		uint32_t v = bs_getbits(&jpeg->bs, code);
		switch (v)
		{
			case (uint32_t)-1:
				JPEG_ERR(jpeg, "read failed");
				return 1;
			case (uint32_t)-2:
				JPEG_ERR(jpeg, "unexpected eof");
				return 1;
		}
		if (v & (1 << (code - 1)))
			values[n] = v;
		else
			values[n] = v - (1 << code) + 1;
		n++;
	}
	return 0;
}

static void
dequantify(struct jpeg *jpeg,
           size_t component,
           int32_t * restrict dst,
           int32_t * restrict src)
{
	int32_t *qt = &jpeg->dqt[jpeg->components[component].table][0];
	for (size_t i = 0; i < 64; ++i)
		dst[i] = src[i] * qt[i];
}

static void
dezigzag(int32_t * restrict dst, int32_t * restrict src)
{
	for (size_t i = 0; i < 64; ++i)
		dst[i] = src[inv_zigzag_table[i]];
}

static void
idct1(int32_t * restrict dst, const int32_t * restrict src)
{
	for (size_t x = 0; x < 8; ++x)
	{
		const int32_t *col = &src[x];
		int32_t v0 = JPEG_RSHIFT(col[8 * 0] * (int32_t)(8192 / 0.353553), 4);
		int32_t v1 = JPEG_RSHIFT(col[8 * 1] * (int32_t)(8192 / 0.254897), 4);
		int32_t v2 = JPEG_RSHIFT(col[8 * 2] * (int32_t)(8192 / 0.270598), 4);
		int32_t v3 = JPEG_RSHIFT(col[8 * 3] * (int32_t)(8192 / 0.300672), 4);
		int32_t v4 = JPEG_RSHIFT(col[8 * 4] * (int32_t)(8192 / 0.353553), 4);
		int32_t v5 = JPEG_RSHIFT(col[8 * 5] * (int32_t)(8192 / 0.449988), 4);
		int32_t v6 = JPEG_RSHIFT(col[8 * 6] * (int32_t)(8192 / 0.653281), 4);
		int32_t v7 = JPEG_RSHIFT(col[8 * 7] * (int32_t)(8192 / 1.281457), 4);

		int32_t z0 = JPEG_RSHIFT(v5 - v3, 1);
		int32_t z1 = JPEG_RSHIFT(v1 - v7, 1);
		int32_t z2 = JPEG_RSHIFT(v1 + v7, 1);
		int32_t z3 = JPEG_RSHIFT(v5 + v3, 1);
		int32_t z4 = JPEG_RSHIFT(z2 + z3, 1);
		int32_t z5 = JPEG_RSHIFT(v2 + v6, 1);
		int32_t z6 = JPEG_RSHIFT(z2 - z3, 1);
		int32_t z7 = JPEG_RSHIFT(v2 - v6, 1);

		int32_t t0 = JPEG_RSHIFT(v0 + v4, 1);
		int32_t t1 = JPEG_RSHIFT(v0 - v4, 1);

		int32_t t2 = JPEG_RSHIFT((z0 - z1) * (int32_t)(0.382683 * 2048), 11);
		int32_t t3 = JPEG_RSHIFT((z1 -  0) * (int32_t)(0.541196 * 2048), 11) - z4 - t2;
		int32_t t4 = JPEG_RSHIFT((z6 -  0) * (int32_t)(1.414215 * 2048), 11) - t3;
		int32_t t5 = JPEG_RSHIFT((z0 -  0) * (int32_t)(1.306562 * 2048), 11) - t2 - t4;
		int32_t t6 = JPEG_RSHIFT((z7 -  0) * (int32_t)(1.414215 * 2048), 11) - z5;

		int32_t t7  = JPEG_RSHIFT(t0 + z5, 1);
		int32_t t8  = JPEG_RSHIFT(t1 + t6, 1);
		int32_t t9  = JPEG_RSHIFT(t1 - t6, 1);
		int32_t t10 = JPEG_RSHIFT(t0 - z5, 1);

		dst[ 0 + x] = JPEG_RSHIFT(t7  + z4, 1);
		dst[ 8 + x] = JPEG_RSHIFT(t8  + t3, 1);
		dst[16 + x] = JPEG_RSHIFT(t9  + t4, 1);
		dst[24 + x] = JPEG_RSHIFT(t10 + t5, 1);
		dst[32 + x] = JPEG_RSHIFT(t10 - t5, 1);
		dst[40 + x] = JPEG_RSHIFT(t9  - t4, 1);
		dst[48 + x] = JPEG_RSHIFT(t8  - t3, 1);
		dst[56 + x] = JPEG_RSHIFT(t7  - z4, 1);
	}
}

static void
idct2(int32_t * restrict dst, const int32_t * restrict src)
{
	for (size_t y = 0; y < 8; ++y)
	{
		const int32_t *row = &src[y * 8];
		int32_t v0 = JPEG_RSHIFT(row[0] * (int32_t)(2048 / 0.353553), 11);
		int32_t v1 = JPEG_RSHIFT(row[1] * (int32_t)(2048 / 0.254897), 11);
		int32_t v2 = JPEG_RSHIFT(row[2] * (int32_t)(2048 / 0.270598), 11);
		int32_t v3 = JPEG_RSHIFT(row[3] * (int32_t)(2048 / 0.300672), 11);
		int32_t v4 = JPEG_RSHIFT(row[4] * (int32_t)(2048 / 0.353553), 11);
		int32_t v5 = JPEG_RSHIFT(row[5] * (int32_t)(2048 / 0.449988), 11);
		int32_t v6 = JPEG_RSHIFT(row[6] * (int32_t)(2048 / 0.653281), 11);
		int32_t v7 = JPEG_RSHIFT(row[7] * (int32_t)(2048 / 1.281457), 11);

		int32_t z0 = JPEG_RSHIFT(v5 - v3, 1);
		int32_t z1 = JPEG_RSHIFT(v1 - v7, 1);
		int32_t z2 = JPEG_RSHIFT(v1 + v7, 1);
		int32_t z3 = JPEG_RSHIFT(v5 + v3, 1);
		int32_t z4 = JPEG_RSHIFT(z2 + z3, 1);
		int32_t z5 = JPEG_RSHIFT(v2 + v6, 1);
		int32_t z6 = JPEG_RSHIFT(z2 - z3, 1);
		int32_t z7 = JPEG_RSHIFT(v2 - v6, 1);

		int32_t t0 = JPEG_RSHIFT(v0 + v4, 1);
		int32_t t1 = JPEG_RSHIFT(v0 - v4, 1);

		int32_t t2 = JPEG_RSHIFT((z0 - z1) * (int32_t)(0.382683 * 2048), 11);
		int32_t t3 = JPEG_RSHIFT((z1 -  0) * (int32_t)(0.541196 * 2048), 11) - z4 - t2;
		int32_t t4 = JPEG_RSHIFT((z6 -  0) * (int32_t)(1.414215 * 2048), 11) - t3;
		int32_t t5 = JPEG_RSHIFT((z0 -  0) * (int32_t)(1.306562 * 2048), 11) - t2 - t4;
		int32_t t6 = JPEG_RSHIFT((z7 -  0) * (int32_t)(1.414215 * 2048), 11) - z5;

		int32_t t7  = JPEG_RSHIFT(t0 + z5, 1);
		int32_t t8  = JPEG_RSHIFT(t1 + t6, 1);
		int32_t t9  = JPEG_RSHIFT(t1 - t6, 1);
		int32_t t10 = JPEG_RSHIFT(t0 - z5, 1);

		int32_t *out = &dst[y * 8];
		out[0] = JPEG_RSHIFT(t7  + z4, 10);
		out[1] = JPEG_RSHIFT(t8  + t3, 10);
		out[2] = JPEG_RSHIFT(t9  + t4, 10);
		out[3] = JPEG_RSHIFT(t10 + t5, 10);
		out[4] = JPEG_RSHIFT(t10 - t5, 10);
		out[5] = JPEG_RSHIFT(t9  - t4, 10);
		out[6] = JPEG_RSHIFT(t8  - t3, 10);
		out[7] = JPEG_RSHIFT(t7  - z4, 10);
	}
}

static void
idct(int32_t * restrict dst, const int32_t * restrict src)
{
	int32_t tmp[64];
	idct1(tmp, src);
	idct2(dst, tmp);
}

static int
decode_block_component(struct jpeg *jpeg, int32_t *values, size_t component)
{
	int32_t tmp1[64];
	int32_t tmp2[64];
	if (decode_component(jpeg, component, &tmp1[0]))
		return 1;
	tmp1[0] += jpeg->components[component].prev_dc;
	jpeg->components[component].prev_dc = tmp1[0];
	dequantify(jpeg, component, &tmp2[0], &tmp1[0]);
	dezigzag(&tmp1[0], &tmp2[0]);
	idct(values, &tmp1[0]);
	return 0;
}

static void
copy_block(int32_t *dst, int32_t *src, size_t dst_pitch, size_t src_shift)
{
	for (size_t y = 0; y < 8; ++y)
	{
		for (size_t x = 0; x < 8; ++x)
			dst[x << src_shift] = src[x];
		dst += dst_pitch;
		src += 8;
	}
}

static void
apply_block_component(struct jpeg *jpeg,
                      int32_t *values,
                      size_t component,
                      size_t bx,
                      size_t by)
{
	int32_t *dst = jpeg->components[component].data;
	dst += (jpeg->block_y + by * jpeg->components[component].height) * jpeg->component_width;
	dst += jpeg->block_x + bx * jpeg->components[component].width;
	size_t scale_x = jpeg->components[component].height / 8;
	size_t scale_y = jpeg->components[component].height / 8;
	size_t src_shift;
	switch (jpeg->components[component].width / 8)
	{
		default:
		case 1:
			src_shift = 0;
			break;
		case 2:
			src_shift = 1;
			break;
		case 4:
			src_shift = 2;
			break;
	}
	size_t dst_pitch = jpeg->component_width * scale_y;
	for (size_t y = 0; y < scale_y; ++y)
	{
		int32_t *dst_row = dst;
		for (size_t x = 0; x < scale_x; ++x)
		{
			copy_block(dst_row, values, dst_pitch, src_shift);
			dst_row++;
		}
		dst += jpeg->component_width;
	}
}

static int
decode_block(struct jpeg *jpeg)
{
	for (size_t i = 0; i < jpeg->components_count; ++i)
	{
		for (size_t y = 0; y < jpeg->components[i].scale_y; ++y)
		{
			for (size_t x = 0; x < jpeg->components[i].scale_x; ++x)
			{
				int32_t values[64];
				if (decode_block_component(jpeg, values, i))
					return 1;
				apply_block_component(jpeg, values, i, x, y);
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
			jpeg->bs.len -= jpeg->bs.len % 8;
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

static void
merge_gray(struct jpeg *jpeg, uint8_t *data)
{
	for (size_t y = 0; y < jpeg->height; ++y)
	{
		const int32_t *Y_data = &jpeg->components[0].data[y * jpeg->component_width];
		uint8_t *row = &data[y * jpeg->width];
		for (size_t x = 0; x < jpeg->width; ++x)
		{
			uint8_t *dst = &row[x];
			int32_t Y = Y_data[x];
			int32_t c = 128 + Y;
			if (c < 0)
				c = 0;
			else if (c > 255)
				c = 255;
			dst[0] = c;
		}
	}
}

static void
merge_rgb(struct jpeg *jpeg, uint8_t *data)
{
	const int32_t *Y_row = jpeg->components[0].data;
	const int32_t *Cb_row = jpeg->components[1].data;
	const int32_t *Cr_row = jpeg->components[2].data;
	uint8_t *row = data;
	for (size_t y = 0; y < jpeg->height; ++y)
	{
		for (size_t x = 0; x < jpeg->width; ++x)
		{
			uint8_t *dst = &row[x * 3];
			int32_t Y = Y_row[x];
			int32_t Cb = Cb_row[x];
			int32_t Cr = Cr_row[x];
			int32_t r = (128 + Y) * 65536;
			int32_t g = r;
			int32_t b = r;
			r += Cr * (int32_t)(1.402   * 65536);
			g -= Cr * (int32_t)(0.71414 * 65536);
			b += Cb * (int32_t)(1.772   * 65536);
			g -= Cb * (int32_t)(0.34414 * 65536);
			r = JPEG_RSHIFT(r, 16);
			g = JPEG_RSHIFT(g, 16);
			b = JPEG_RSHIFT(b, 16);
			if (r < 0)
				r = 0;
			else if (r > 255)
				r = 255;
			if (g < 0)
				g = 0;
			else if (g > 255)
				g = 255;
			if (b < 0)
				b = 0;
			else if (b > 255)
				b = 255;
			dst[0] = r;
			dst[1] = g;
			dst[2] = b;
		}
		Y_row += jpeg->component_width;
		Cb_row += jpeg->component_width;
		Cr_row += jpeg->component_width;
		row += jpeg->width * 3;
	}
}

int
jpeg_read_data(struct jpeg *jpeg, void *data)
{
	if (!jpeg->fp || !jpeg->sos)
		return 0;
	bs_init_read(&jpeg->bs);
	jpeg->restart_count = jpeg->restart_interval;
	while (1)
	{
		if (decode_block(jpeg))
			return 1;
		if (jpeg->block_y < jpeg->height)
			continue;
		if (jpeg->eof)
			break;
		uint8_t eoi_mark[2];
		if (fread(eoi_mark, 1, 2, jpeg->fp) != 2)
		{
			JPEG_ERR(jpeg, "unexpected data EOF");
			return 1;
		}
		if (eoi_mark[0] != 0xFF
		 || eoi_mark[1] != JPEG_CHUNK_EOI)
		{
			JPEG_ERR(jpeg, "invalid EOI");
			return 1;
		}
		break;
	}
	if (jpeg->components_count == 1)
		merge_gray(jpeg, data);
	else
		merge_rgb(jpeg, data);
	return 0;
}
