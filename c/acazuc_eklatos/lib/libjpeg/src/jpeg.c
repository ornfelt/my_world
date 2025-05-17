#include "jpeg.h"

#include <stdlib.h>
#include <string.h>

static const uint8_t
luma_dqt_table[64] =
{
	16, 11, 10, 16,  24,  40,  51,  61,
	12, 12, 14, 19,  26,  58,  60,  55,
	14, 13, 16, 24,  40,  57,  69,  56,
	14, 17, 22, 29,  51,  87,  80,  62,
	18, 22, 37, 56,  68, 109, 103,  77,
	24, 35, 55, 64,  81, 104, 113,  92,
	49, 64, 78, 87, 103, 121, 120, 101,
	72, 92, 95, 98, 112, 100, 103,  99,
};

static const uint8_t
chroma_dqt_table[64] =
{
	17, 18, 24, 47, 99, 99, 99, 99,
	18, 21, 26, 66, 99, 99, 99, 99,
	24, 26, 56, 99, 99, 99, 99, 99,
	47, 66, 99, 99, 99, 99, 99, 99,
	99, 99, 99, 99, 99, 99, 99, 99,
	99, 99, 99, 99, 99, 99, 99, 99,
	99, 99, 99, 99, 99, 99, 99, 99,
	99, 99, 99, 99, 99, 99, 99, 99,
};

static const uint8_t
huffman_luma_dc_counts[16] =
{
	0, 1, 5, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
};

static const uint8_t
huffman_luma_dc_values[] =
{
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0A, 0x0B,
};

static const uint8_t
huffman_luma_ac_counts[16] =
{
	0, 2, 1, 3, 3, 2, 4, 3, 5, 5, 4, 4, 0, 0, 1, 125,
};

static const uint8_t
huffman_luma_ac_values[] =
{
	0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12,
	0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07,
	0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xA1, 0x08,
	0x23, 0x42, 0xB1, 0xC1, 0x15, 0x52, 0xD1, 0xF0,
	0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0A, 0x16,
	0x17, 0x18, 0x19, 0x1A, 0x25, 0x26, 0x27, 0x28,
	0x29, 0x2A, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
	0x3A, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
	0x4A, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
	0x5A, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
	0x6A, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
	0x7A, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
	0x8A, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98,
	0x99, 0x9A, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7,
	0xA8, 0xA9, 0xAA, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6,
	0xB7, 0xB8, 0xB9, 0xBA, 0xC2, 0xC3, 0xC4, 0xC5,
	0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xD2, 0xD3, 0xD4,
	0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xE1, 0xE2,
	0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA,
	0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8,
	0xF9, 0xFA,
};

static const uint8_t
huffman_chroma_dc_counts[16] =
{
	0, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
};

static const uint8_t
huffman_chroma_dc_values[] =
{
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0A, 0x0B,
};

static const uint8_t
huffman_chroma_ac_counts[16] =
{
	0, 2, 1, 2, 4, 4, 3, 4, 7, 5, 4, 4, 0, 1, 2, 119,
};

static const uint8_t
huffman_chroma_ac_values[] =
{
	0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21,
	0x31, 0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71,
	0x13, 0x22, 0x32, 0x81, 0x08, 0x14, 0x42, 0x91,
	0xA1, 0xB1, 0xC1, 0x09, 0x23, 0x33, 0x52, 0xF0,
	0x15, 0x62, 0x72, 0xD1, 0x0A, 0x16, 0x24, 0x34,
	0xE1, 0x25, 0xF1, 0x17, 0x18, 0x19, 0x1A, 0x26,
	0x27, 0x28, 0x29, 0x2A, 0x35, 0x36, 0x37, 0x38,
	0x39, 0x3A, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
	0x49, 0x4A, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
	0x59, 0x5A, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
	0x69, 0x6A, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
	0x79, 0x7A, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
	0x88, 0x89, 0x8A, 0x92, 0x93, 0x94, 0x95, 0x96,
	0x97, 0x98, 0x99, 0x9A, 0xA2, 0xA3, 0xA4, 0xA5,
	0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xB2, 0xB3, 0xB4,
	0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xC2, 0xC3,
	0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xD2,
	0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA,
	0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9,
	0xEA, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8,
	0xF9, 0xFA,
};

static void
generate_qdt(int32_t * restrict dqt,
             size_t q,
             const uint8_t * restrict tb)
{
	if (q < 1)
		q = 1;
	if (q > 100)
		q = 100;
	int32_t s;
	if (q < 50)
		s = (5000 * 256) / q;
	else
		s = 256 * (200 - 2 * q);
	for (size_t i = 0; i < 64; ++i)
	{
		int32_t result = JPEG_RSHIFT((s * tb[i] + 50) / 100, 8);
		if (result <= 0)
			dqt[i] = 1;
		else if (result > 255)
			dqt[i] = 255;
		else
			dqt[i] = result;
	}
}

int
jpeg_getc(struct jpeg *jpeg)
{
	int c = getc(jpeg->fp);
	if (c != EOF)
		return c;
	if (ferror(jpeg->fp))
		JPEG_ERR(jpeg, "failed to read from file");
	else
		JPEG_ERR(jpeg, "unexpected EOF");
	return EOF;
}

int
jpeg_putc(struct jpeg *jpeg, uint8_t v)
{
	if (putc(v, jpeg->fp) == EOF)
	{
		JPEG_ERR(jpeg, "failed to write to file");
		return EOF;
	}
	return 0;
}

static int
jpeg_getc_unlocked(struct jpeg *jpeg)
{
	int c = getc_unlocked(jpeg->fp);
	if (c != EOF)
		return c;
	if (ferror_unlocked(jpeg->fp))
		JPEG_ERR(jpeg, "failed to read from file");
	else
		JPEG_ERR(jpeg, "unexpected EOF");
	return EOF;
}

static int
bs_getc(struct jpeg *jpeg)
{
	int c = jpeg_getc_unlocked(jpeg);
	if (c == EOF)
		return -1;
	if (c != 0xFF)
		return c;
	int c2 = jpeg_getc_unlocked(jpeg);
	if (c2 == EOF)
		return -1;
	switch (c2)
	{
		case 0x00:
			return 0xFF;
		case JPEG_CHUNK_EOI:
			jpeg->eof = 1;
			return 0;
		case JPEG_CHUNK_RST0:
		case JPEG_CHUNK_RST1:
		case JPEG_CHUNK_RST2:
		case JPEG_CHUNK_RST3:
		case JPEG_CHUNK_RST4:
		case JPEG_CHUNK_RST5:
		case JPEG_CHUNK_RST6:
		case JPEG_CHUNK_RST7:
			return bs_getc(jpeg);
		default:
			return c2;
	}
}

static int
bs_get(struct bitstream *bs)
{
	struct jpeg *jpeg = bs->userdata;
	int ret = 1;

	flockfile(jpeg->fp);
	while (!jpeg->eof && bs->len < sizeof(bs->buf) * 8 - 7)
	{
		int c = bs_getc(jpeg);
		if (c == -1)
			goto end;
		bs->buf <<= 8;
		bs->buf |= c;
		bs->len += 8;
	}
	ret = 0;

end:
	funlockfile(jpeg->fp);
	return ret;
}

static int
bs_put(struct bitstream *bs)
{
	struct jpeg *jpeg = bs->userdata;
	uint8_t buf[sizeof(bs->buf) * 2];
	size_t len = 0;

	while (bs->len < sizeof(bs->buf) * 8 - 7)
	{
		uint8_t byte = bs->buf >> (sizeof(bs->buf) * 8 - 8);
		buf[len++] = byte;
		if (byte == 0xFF)
			buf[len++] = 0;
		bs->buf <<= 8;
		bs->len += 8;
	}
	if (fwrite(buf, 1, len, jpeg->fp) != len)
	{
		JPEG_ERR(jpeg, "failed to write to file");
		return 1;
	}
	return 0;
}

struct jpeg *
jpeg_new(void)
{
	struct jpeg *jpeg = calloc(1, sizeof(*jpeg));
	if (!jpeg)
		return NULL;
	jpeg->bs.get = bs_get;
	jpeg->bs.put = bs_put;
	jpeg->bs.userdata = jpeg;
	generate_qdt(&jpeg->dqt[0][0], 50, luma_dqt_table);
	generate_qdt(&jpeg->dqt[1][0], 50, chroma_dqt_table);
	return jpeg;
}

void
jpeg_free(struct jpeg *jpeg)
{
	if (!jpeg)
		return;
	for (size_t i = 0; i < jpeg->components_count; ++i)
		free(jpeg->components[i].data);
	free(jpeg->thumbnail);
	free(jpeg);
}

void
jpeg_init_io(struct jpeg *jpeg, FILE *fp)
{
	jpeg->fp = fp;
	jpeg->eof = 0;
	jpeg->sos = 0;
	jpeg->block_x = 0;
	jpeg->block_y = 0;
	jpeg->restart_count = 0;
	for (size_t i = 0; i < 3; ++i)
		jpeg->components[i].prev_dc = 0;
}

const uint8_t *
jpeg_get_thumbnail(struct jpeg *jpeg, uint8_t *width, uint8_t *height)
{
	if (width)
		*width = jpeg->thumbnail_width;
	if (height)
		*height = jpeg->thumbnail_height;
	return jpeg->thumbnail;
}

int
jpeg_set_thumbnail(struct jpeg *jpeg,
                   const uint8_t *data,
                   uint8_t width,
                   uint8_t height)
{
	if (!width || !height)
	{
		free(jpeg->thumbnail);
		jpeg->thumbnail = NULL;
		jpeg->thumbnail_width = 0;
		jpeg->thumbnail_height = 0;
		return 0;
	}
	uint8_t *dup = malloc(width * height);
	if (!dup)
	{
		JPEG_ERR(jpeg, "thumbnail allocation failed");
		return 1;
	}
	memcpy(dup, data, width * height);
	free(jpeg->thumbnail);
	jpeg->thumbnail = dup;
	jpeg->thumbnail_width = width;
	jpeg->thumbnail_height = height;
	return 0;
}

void
jpeg_set_quality(struct jpeg *jpeg, int quality)
{
	if (quality < 1)
		quality = 1;
	if (quality > 100)
		quality = 100;
	generate_qdt(&jpeg->dqt[0][0], quality, luma_dqt_table);
	generate_qdt(&jpeg->dqt[1][0], quality, chroma_dqt_table);
}

void
jpeg_set_restart_interval(struct jpeg *jpeg, uint16_t restart_interval)
{
	jpeg->restart_interval = restart_interval;
}

int
jpeg_set_subsampling(struct jpeg *jpeg, int subsampling)
{
	switch (subsampling)
	{
		case JPEG_SUBSAMPLING_444:
			jpeg->components[0].scale_x = 1;
			jpeg->components[0].scale_y = 1;
			jpeg->components[1].scale_x = 1;
			jpeg->components[1].scale_y = 1;
			jpeg->components[2].scale_x = 1;
			jpeg->components[2].scale_y = 1;
			return 0;
		case JPEG_SUBSAMPLING_440:
			jpeg->components[0].scale_x = 1;
			jpeg->components[0].scale_y = 2;
			jpeg->components[1].scale_x = 1;
			jpeg->components[1].scale_y = 1;
			jpeg->components[2].scale_x = 1;
			jpeg->components[2].scale_y = 1;
			return 0;
		case JPEG_SUBSAMPLING_422:
			jpeg->components[0].scale_x = 2;
			jpeg->components[0].scale_y = 1;
			jpeg->components[1].scale_x = 1;
			jpeg->components[1].scale_y = 1;
			jpeg->components[2].scale_x = 1;
			jpeg->components[2].scale_y = 1;
			return 0;
		case JPEG_SUBSAMPLING_420:
			jpeg->components[0].scale_x = 2;
			jpeg->components[0].scale_y = 2;
			jpeg->components[1].scale_x = 1;
			jpeg->components[1].scale_y = 1;
			jpeg->components[2].scale_x = 1;
			jpeg->components[2].scale_y = 1;
			return 0;
		case JPEG_SUBSAMPLING_411:
			jpeg->components[0].scale_x = 4;
			jpeg->components[0].scale_y = 1;
			jpeg->components[1].scale_x = 1;
			jpeg->components[1].scale_y = 1;
			jpeg->components[2].scale_x = 1;
			jpeg->components[2].scale_y = 1;
			return 0;
		case JPEG_SUBSAMPLING_410:
			jpeg->components[0].scale_x = 4;
			jpeg->components[0].scale_y = 2;
			jpeg->components[1].scale_x = 1;
			jpeg->components[1].scale_y = 1;
			jpeg->components[2].scale_x = 1;
			jpeg->components[2].scale_y = 1;
			return 0;
		default:
			JPEG_ERR(jpeg, "unknown subsampling");
			return 1;
	}
}

void
jpeg_get_info(struct jpeg *jpeg,
              uint32_t *width,
              uint32_t *height,
              uint8_t *components)
{
	if (width)
		*width = jpeg->width;
	if (height)
		*height = jpeg->height;
	if (components)
		*components = jpeg->components_count;
}

int
jpeg_set_info(struct jpeg *jpeg,
              uint32_t width,
              uint32_t height,
              uint8_t components)
{
	if (!width
	 || width > UINT16_MAX
	 || !height
	 || height > UINT16_MAX)
	{
		JPEG_ERR(jpeg, "invalid dimensions");
		return 1;
	}
	if (components != 1 && components != 3)
	{
		JPEG_ERR(jpeg, "invalid components count");
		return 1;
	}
	jpeg->bpp = 8;
	jpeg->width = width;
	jpeg->height = height;
	jpeg->components_count = components;
	jpeg->block_width = 8 * jpeg->components[0].scale_x;
	jpeg->block_height = 8 * jpeg->components[0].scale_y;
	jpeg->component_width = jpeg->width + jpeg->block_width - 1;
	jpeg->component_width -= jpeg->component_width % jpeg->block_width;
	jpeg->component_height = jpeg->height + jpeg->block_height - 1;
	jpeg->component_height -= jpeg->component_height % jpeg->block_height;
	for (size_t i = 0; i < components; ++i)
	{
		jpeg->components[i].table = !!i;
		jpeg->components[i].dct = !!i;
		jpeg->components[i].act = !!i;
		jpeg->components[i].width = jpeg->block_width / jpeg->components[i].scale_x;
		jpeg->components[i].height = jpeg->block_height / jpeg->components[i].scale_y;
		jpeg->components[i].dc_huff = 2 * jpeg->components[i].dct;
		jpeg->components[i].ac_huff = jpeg->components[i].dc_huff + 1;
		free(jpeg->components[i].data);
		jpeg->components[i].data = malloc(jpeg->component_width
		                                * jpeg->component_height
		                                * sizeof(*jpeg->components[i].data));
		if (!jpeg->components[i].data)
		{
			JPEG_ERR(jpeg, "component buffer allocation failed");
			return 1;
		}
	}
	jpeg->huff_tables[0].class = 0;
	jpeg->huff_tables[0].dst = 0;
	huffman_generate(&jpeg->huff_tables[0].huffman,
	                 huffman_luma_dc_counts,
	                 huffman_luma_dc_values);
	jpeg->huff_tables[1].class = 1;
	jpeg->huff_tables[1].dst = 0;
	huffman_generate(&jpeg->huff_tables[1].huffman,
	                 huffman_luma_ac_counts,
	                 huffman_luma_ac_values);
	jpeg->huff_tables[2].class = 0;
	jpeg->huff_tables[2].dst = 1;
	huffman_generate(&jpeg->huff_tables[2].huffman,
	                 huffman_chroma_dc_counts,
	                 huffman_chroma_dc_values);
	jpeg->huff_tables[3].class = 1;
	jpeg->huff_tables[3].dst = 1;
	huffman_generate(&jpeg->huff_tables[3].huffman,
	                 huffman_chroma_ac_counts,
	                 huffman_chroma_ac_values);
	jpeg->huff_count = 4;
	return 0;
}

void
huffman_generate(struct huffman *huff,
                 const uint8_t *counts,
                 const uint8_t *values)
{
	size_t count = 0;
	uint32_t code = 0;
	size_t n = 0;
	for (size_t i = 0; i < MAX_BITS; ++i)
	{
		huff->offsets[i] = count;
		huff->counts[i] = counts[i];
		count += counts[i];
		huff->codes[i] = code;
		huff->maxcodes[i] = counts[i] ? code + counts[i] : 0;
		for (size_t j = 0; j < counts[i]; ++j)
		{
			huff->sizes[values[n]] = i + 1;
			huff->map[values[n]] = code + j;
			n++;
		}
		code = (code + counts[i]) << 1;
	}
	memcpy(huff->values, values, count);
}

const char *
jpeg_get_err(struct jpeg *jpeg)
{
	return jpeg->errbuf;
}
