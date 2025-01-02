#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <math.h>
#include <png.h>

#define MAX_BITS 16

#define JPEG_CHUNK_SOF0 0xC0
#define JPEG_CHUNK_SOF1 0xC2
#define JPEG_CHUNK_DHT  0xC4
#define JPEG_CHUNK_RST0 0xD0
#define JPEG_CHUNK_RST1 0xD1
#define JPEG_CHUNK_RST2 0xD2
#define JPEG_CHUNK_RST3 0xD3
#define JPEG_CHUNK_RST4 0xD4
#define JPEG_CHUNK_RST5 0xD5
#define JPEG_CHUNK_RST6 0xD6
#define JPEG_CHUNK_RST7 0xD7
#define JPEG_CHUNK_SOI  0xD8
#define JPEG_CHUNK_EOI  0xD9
#define JPEG_CHUNK_SOS  0xDA
#define JPEG_CHUNK_DQT  0xDB
#define JPEG_CHUNK_DRI  0xDD
#define JPEG_CHUNK_APP0 0xE0
#define JPEG_CHUNK_APP1 0xE1
#define JPEG_CHUNK_APP2 0xE2
#define JPEG_CHUNK_APP3 0xE3
#define JPEG_CHUNK_APP4 0xE4
#define JPEG_CHUNK_APP5 0xE5
#define JPEG_CHUNK_APP6 0xE6
#define JPEG_CHUNK_APP7 0xE7
#define JPEG_CHUNK_APP8 0xE8
#define JPEG_CHUNK_APP9 0xE9
#define JPEG_CHUNK_APPA 0xEA
#define JPEG_CHUNK_APPB 0xEB
#define JPEG_CHUNK_APPC 0xEC
#define JPEG_CHUNK_APPD 0xED
#define JPEG_CHUNK_APPE 0xEE
#define JPEG_CHUNK_APPF 0xEF
#define JPEG_CHUNK_COM  0xFE

#define JPEG_SUBSAMPLING_444 0x1
#define JPEG_SUBSAMPLING_440 0x2
#define JPEG_SUBSAMPLING_422 0x3
#define JPEG_SUBSAMPLING_420 0x4
#define JPEG_SUBSAMPLING_411 0x5
#define JPEG_SUBSAMPLING_410 0x6

static const uint8_t luma_dqt_table[64] =
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

static const uint8_t chroma_dqt_table[64] =
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

static const uint8_t zigzag_table[64] =
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

static const uint8_t inv_zigzag_table[64] =
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

static const uint8_t huffman_luma_dc_counts[16] =
{
	0, 1, 5, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
};

static const uint8_t huffman_luma_dc_values[] =
{
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0A, 0x0B,
};

static const uint8_t huffman_luma_ac_counts[16] =
{
	0, 2, 1, 3, 3, 2, 4, 3, 5, 5, 4, 4, 0, 0, 1, 125,
};

static const uint8_t huffman_luma_ac_values[] =
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

static const uint8_t huffman_chroma_dc_counts[16] =
{
	0, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
};

static const uint8_t huffman_chroma_dc_values[] =
{
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0A, 0x0B,
};

static const uint8_t huffman_chroma_ac_counts[16] =
{
	0, 2, 1, 2, 4, 4, 3, 4, 7, 5, 4, 4, 0, 1, 2, 119,
};

static const uint8_t huffman_chroma_ac_values[] =
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

struct huffman
{
	uint16_t offsets[MAX_BITS];
	uint16_t counts[MAX_BITS];
	uint16_t codes[MAX_BITS];
	uint8_t values[256];
	uint8_t sizes[256];
	uint16_t map[256];
};

struct jpeg
{
	uint8_t unit;
	uint8_t bpp;
	uint8_t components_count;
	uint8_t sos;
	uint16_t density_x;
	uint16_t density_y;
	uint16_t width;
	uint16_t height;
	uint16_t restart_interval;
	uint16_t restart_count;
	struct
	{
		uint8_t scale_x : 4;
		uint8_t scale_y : 4;
		uint8_t table;
		uint8_t dct : 4;
		uint8_t act : 4;
		uint8_t dc_huff : 4;
		uint8_t ac_huff : 4;
		int32_t prev_dc;
		int32_t *data;
		uint16_t width;
		uint16_t height;
		uint8_t iscale_x;
		uint8_t iscale_y;
	} components[3];
	struct
	{
		struct huffman huffman;
		uint8_t class : 4;
		uint8_t dst : 4;
	} huff_tables[4];
	int32_t dqt[2][64];
	int32_t idqt[2][64];
	uint8_t huff_count;
	uint8_t bit_len;
	uint8_t eof;
	uint32_t bit_buf;
	uint32_t component_width;
	uint32_t component_height;
	uint32_t block_width;
	uint32_t block_height;
	uint32_t block_x;
	uint32_t block_y;
	uint8_t thumbnail_width;
	uint8_t thumbnail_height;
	uint8_t restart_id;
	uint8_t *thumbnail;
	FILE *fp;
};

static int64_t nanotime(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec * 1000000000 + ts.tv_nsec;
}

static void generate_qdt(int32_t *dqt, int q, const uint8_t *tb)
{
	if (q < 1)
		q = 1;
	if (q > 100)
		q = 100;
	float s;
	if (q < 50)
		s = 5000 / q;
	else
		s = 200 - 2 * q;
	for (int i = 0; i < 64; ++i)
	{
		int32_t result = (s * tb[i] + 50) / 100;
		if (result <= 0)
			dqt[i] = 1;
		else if (result > 255)
			dqt[i] = 255;
		else
			dqt[i] = result;
	}
}

static int putbyte(struct jpeg *jpeg, uint8_t v)
{
	if (putc(v, jpeg->fp) == EOF)
	{
		fprintf(stderr, "file write failed\n");
		return 1;
	}
	return 0;
}

static int putbytes(struct jpeg *jpeg)
{
	while (jpeg->bit_len <= (sizeof(jpeg->bit_buf) * 8) - 8)
	{
		uint8_t byte = jpeg->bit_buf >> ((sizeof(jpeg->bit_buf) * 8) - 8);
		if (putbyte(jpeg, byte))
			return 1;
		if (byte == 0xFF)
		{
			if (putbyte(jpeg, 0))
				return 1;
		}
		jpeg->bit_buf <<= 8;
		jpeg->bit_len += 8;
	}
	return 0;
}

static int putbits(struct jpeg *jpeg, uint32_t value, uint32_t len)
{
	while (1)
	{
		uint32_t mask = (1 << len) - 1;
		if (len < jpeg->bit_len)
		{
			jpeg->bit_len -= len;
			jpeg->bit_buf |= (value & mask) << jpeg->bit_len;
			break;
		}
		len -= jpeg->bit_len;
		jpeg->bit_buf |= (value & mask) >> len;
		jpeg->bit_len = 0;
		if (putbytes(jpeg))
			return 1;
	}
	return 0;
}

static int flushbits(struct jpeg *jpeg)
{
	if (jpeg->bit_len == sizeof(jpeg->bit_buf) * 8)
		return 0;
	uint8_t cur = sizeof(jpeg->bit_buf) * 8 - jpeg->bit_len;
	if (putbits(jpeg, 0, 8 - (cur % 8)))
		return 1;
	return putbytes(jpeg);
}

static int getbyte(struct jpeg *jpeg)
{
	int c = getc(jpeg->fp);
	if (c == EOF)
	{
		if (ferror(jpeg->fp))
		{
			fprintf(stderr, "file read failed\n");
			return -1;
		}
		return -2;
	}
	return c;
}

static int pullbyte(struct jpeg *jpeg)
{
	int c = getbyte(jpeg);
	if (c < 0)
		return c;
	if (c == 0xFF)
	{
		int c2 = getbyte(jpeg);
		if (c2 < 0)
			return c2;
		switch (c2)
		{
			case 0x00:
				c = 0xFF;
				break;
			case JPEG_CHUNK_EOI:
				jpeg->eof = 1;
				return -2;
			case JPEG_CHUNK_RST0:
			case JPEG_CHUNK_RST1:
			case JPEG_CHUNK_RST2:
			case JPEG_CHUNK_RST3:
			case JPEG_CHUNK_RST4:
			case JPEG_CHUNK_RST5:
			case JPEG_CHUNK_RST6:
			case JPEG_CHUNK_RST7:
				return pullbyte(jpeg);
			default:
				printf("unknown marker: %x\n", c2);
				c = c2;
				break;
		}
	}
	jpeg->bit_buf <<= 8;
	jpeg->bit_buf |= c;
	jpeg->bit_len += 8;
	return 0;
}

static int pullbytes(struct jpeg *jpeg)
{
	while (jpeg->bit_len < sizeof(jpeg->bit_buf) * 8)
	{
		switch (pullbyte(jpeg))
		{
			case -1:
				return -1;
			case -2:
				return jpeg->bit_len ? 0 : -2;
		}
	}
	return 0;
}

static int getbit(struct jpeg *jpeg)
{
	if (!jpeg->bit_len)
	{
		int ret = pullbytes(jpeg);
		if (ret)
			return ret;
	}
	return (jpeg->bit_buf >> (--jpeg->bit_len)) & 1;
}

static int getbits(struct jpeg *jpeg, uint32_t *p, uint32_t len)
{
	uint32_t v = 0;
	while (1)
	{
		uint32_t mask = (1 << len) - 1;
		if (len <= jpeg->bit_len)
		{
			jpeg->bit_len -= len;
			v |= (jpeg->bit_buf >> jpeg->bit_len) & mask;
			break;
		}
		len -= jpeg->bit_len;
		v |= (jpeg->bit_buf << len) & mask;
		jpeg->bit_len = 0;
		int ret = pullbytes(jpeg);
		if (ret)
		{
			*p = v;
			return ret;
		}
	}
	*p = v;
	return 0;
}

static void huffman_generate(struct huffman *huff, const uint8_t *counts, const uint8_t *values)
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

static int huffman_decode(struct jpeg *jpeg, struct huffman *huff)
{
	uint16_t value = 0;
	for (size_t i = 0; i < MAX_BITS; ++i)
	{
		value <<= 1;
		int ret = getbit(jpeg);
		if (ret < 0)
			return ret;
		value |= ret;
		uint32_t counts = huff->counts[i];
		uint32_t base = huff->codes[i];
		if (value >= base && value < base + counts)
			return huff->values[huff->offsets[i] + (value - base)];
	}
	return -1;
}

static int parse_app0(struct jpeg *jpeg, uint16_t length)
{
	if (length < 16)
	{
		fprintf(stderr, "invalid length\n");
		return 1;
	}
	uint8_t bytes[14];
	if (fread(&bytes, 1, sizeof(bytes), jpeg->fp) != sizeof(bytes))
	{
		fprintf(stderr, "unexpected EOF\n");
		return 1;
	}
	if (bytes[0] != 'J' || bytes[1] != 'F'
	 || bytes[2] != 'I' || bytes[3] != 'F'
	 || bytes[4] != '\0')
	{
		fprintf(stderr, "invalid jfif\n");
		return 1;
	}
	if (bytes[5] != 1 || bytes[6] != 1)
	{
		fprintf(stderr, "invalid version\n");
		return 1;
	}
	if (bytes[7] > 2)
	{
		fprintf(stderr, "invalid unit\n");
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
			fprintf(stderr, "invalid app0 size\n");
			return 1;
		}
		free(jpeg->thumbnail);
		jpeg->thumbnail = malloc(thumbnail_size);
		if (!jpeg->thumbnail)
		{
			fprintf(stderr, "thumbnail allocation failed\n");
			return 1;
		}
		if (fread(jpeg->thumbnail, 1, thumbnail_size, jpeg->fp) != thumbnail_size)
		{
			fprintf(stderr, "thumbnail read failed\n");
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

static int parse_dqt(struct jpeg *jpeg, uint16_t length)
{
	if (length != 67)
	{
		fprintf(stderr, "invalid length\n");
		return 1;
	}
	int dst = getbyte(jpeg);
	if (dst == -1)
		return 1;
	if (dst == -2)
	{
		fprintf(stderr, "unexpected EOF\n");
		return 1;
	}
	if (dst < 0 || dst > 1)
	{
		fprintf(stderr, "invalid qt dst\n");
		return 1;
	}
	uint8_t dqt[64];
	if (fread(&dqt[0], 1, 64, jpeg->fp) != 64)
	{
		fprintf(stderr, "unexpected EOF\n");
		return 1;
	}
	for (size_t i = 0; i < 64; ++i)
	{
		if (!dqt[i])
		{
			fprintf(stderr, "invalid dqt values\n");
			return 1;
		}
		jpeg->dqt[dst][i] = dqt[i];
	}
	return 0;
}

static int parse_sof0(struct jpeg *jpeg, uint16_t length)
{
	if (length != 17 && length != 11)
	{
		fprintf(stderr, "invalid length\n");
		return 1;
	}
	uint8_t bytes[15];
	if (fread(&bytes, 1, length - 2, jpeg->fp) != length - 2u)
	{
		fprintf(stderr, "unexpected EOF\n");
		return 1;
	}
	jpeg->bpp = bytes[0];
	jpeg->height = (bytes[1] << 8) | bytes[2];
	jpeg->width = (bytes[3] << 8) | bytes[4];
	jpeg->components_count = bytes[5];
	printf("bpp: %u\n", jpeg->bpp);
	printf("width: %u\n", jpeg->width);
	printf("heigth: %u\n", jpeg->height);
	printf("components: %u\n", jpeg->components_count);
	if (jpeg->bpp != 8)
	{
		fprintf(stderr, "invalid bpp\n");
		return 1;
	}
	if (jpeg->components_count != 3
	 && jpeg->components_count != 1)
	{
		fprintf(stderr, "invalid components\n");
		return 1;
	}
	for (int i = 0; i < jpeg->components_count; ++i)
	{
		uint8_t base = 6 + i * 3;
		uint8_t id = bytes[base];
		if (!id || id > jpeg->components_count)
		{
			fprintf(stderr, "invalid component id\n");
			return 1;
		}
		id--;
		if (jpeg->components[id].data)
		{
			fprintf(stderr, "duplicated component\n");
			return 1;
		}
		jpeg->components[id].scale_x = bytes[base + 1] >> 4;
		jpeg->components[id].scale_y = bytes[base + 1] & 0xF;
		jpeg->components[id].table = bytes[base + 2];
		if (jpeg->components[id].table > 1)
		{
			fprintf(stderr, "invalid component table\n");
			return 1;
		}
		if (jpeg->components[id].scale_x != 1
		 && jpeg->components[id].scale_x != 2
		 && jpeg->components[id].scale_x != 4)
		{
			fprintf(stderr, "invalid component scale x\n");
			return 1;
		}
		if (jpeg->components[id].scale_y != 1
		 && jpeg->components[id].scale_y != 2
		 && jpeg->components[id].scale_y != 4)
		{
			fprintf(stderr, "invalid component scale y\n");
			return 1;
		}
		printf("comp[%d] %dx%d %d\n", id,
		       jpeg->components[id].scale_x,
		       jpeg->components[id].scale_y,
		       jpeg->components[id].table);
	}
	if (jpeg->components_count > 1)
	{
		if (jpeg->components[1].scale_x > jpeg->components[0].scale_x
		 || jpeg->components[1].scale_y > jpeg->components[0].scale_y
		 || jpeg->components[2].scale_x > jpeg->components[0].scale_x
		 || jpeg->components[2].scale_y > jpeg->components[0].scale_y)
		{
			fprintf(stderr, "invalid components scales\n");
			return 1;
		}
	}
	jpeg->block_width = 8 * jpeg->components[0].scale_x;
	jpeg->block_height = 8 * jpeg->components[0].scale_y;
	jpeg->component_width = jpeg->width + jpeg->block_width - 1;
	jpeg->component_width -= jpeg->component_width % jpeg->block_width;
	jpeg->component_height = jpeg->height + jpeg->block_height - 1;
	jpeg->component_height -= jpeg->component_height % jpeg->block_height;
	for (int i = 0; i < jpeg->components_count; ++i)
	{
		jpeg->components[i].width = jpeg->block_width / jpeg->components[i].scale_x;
		jpeg->components[i].height = jpeg->block_height / jpeg->components[i].scale_y;
		jpeg->components[i].iscale_x = (8 * jpeg->components[i].scale_x) / jpeg->components[0].scale_x;
		jpeg->components[i].iscale_y = (8 * jpeg->components[i].scale_y) / jpeg->components[0].scale_y;
		free(jpeg->components[i].data);
		jpeg->components[i].data = malloc(jpeg->component_width
		                                * jpeg->component_height
		                                * sizeof(*jpeg->components[i].data));
		if (!jpeg->components[i].data)
		{
			fprintf(stderr, "malloc failed\n");
			return 1;
		}
	}
	return 0;
}

static int parse_dht(struct jpeg *jpeg, uint16_t length)
{
	if (jpeg->huff_count >= 4)
		return 1;
	if (length < 19 || length > 19 + 255)
	{
		fprintf(stderr, "invalid length\n");
		return 1;
	}
	uint8_t bytes[17 + 255];
	if (fread(&bytes, 1, length - 2, jpeg->fp) != length - 2u)
	{
		fprintf(stderr, "unexpected EOF\n");
		return 1;
	}
	jpeg->huff_tables[jpeg->huff_count].class = bytes[0] >> 4;
	jpeg->huff_tables[jpeg->huff_count].dst = bytes[0] & 0xF;
	printf("dht[%u] class %u, dst %u\n",
	       jpeg->huff_count,
	       jpeg->huff_tables[jpeg->huff_count].class,
	       jpeg->huff_tables[jpeg->huff_count].dst);
	size_t sum = 0;
	for (size_t i = 0; i < 16; ++i)
		sum += bytes[1 + i];
	if (length != 19 + sum)
	{
		fprintf(stderr, "invalid length\n");
		return 1;
	}
	huffman_generate(&jpeg->huff_tables[jpeg->huff_count].huffman, &bytes[1], &bytes[17]);
	jpeg->huff_count++;
	return 0;
}

static int get_huffman(struct jpeg *jpeg, int class, int dst)
{
	for (int i = 0; i < jpeg->huff_count; ++i)
	{
		if (jpeg->huff_tables[i].class == class
		 && jpeg->huff_tables[i].dst == dst)
			return i;
	}
	fprintf(stderr, "failed to get huffman\n");
	return -1;
}

static int encode_component(struct jpeg *jpeg, int component, int32_t *values)
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
					if (putbits(jpeg, huffman->map[0x00], huffman->sizes[0x00]))
						return 1;
					return 0;
				}
			} while (!values[n]);
			if (fwd_zero >= 16)
			{
				n -= fwd_zero - 16;
				if (putbits(jpeg, huffman->map[0xF0], huffman->sizes[0xF0]))
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
		if (putbits(jpeg, huffman->map[code], huffman->sizes[code]))
			return 1;
		if (putbits(jpeg, value, length))
			return 1;
		huffman = act;
		n++;
	}
	return 0;
}

static int decode_component(struct jpeg *jpeg, int component, int32_t *values)
{
	struct huffman *dct = &jpeg->huff_tables[jpeg->components[component].dc_huff].huffman;
	struct huffman *act = &jpeg->huff_tables[jpeg->components[component].ac_huff].huffman;
	struct huffman *huffman = dct;
	size_t n = 0;
	while (n < 64)
	{
		int code = huffman_decode(jpeg, huffman);
		switch (code)
		{
			case -1:
				fprintf(stderr, "huffman failed\n");
				return 1;
			case -2:
				if (!jpeg->eof)
					fprintf(stderr, "unexpected eof\n");
				return 1;
			default:
				break;
		}
		huffman = act;
		if (n)
		{
			uint8_t nzero;
			if (!code)
				nzero = 64 - n;
			else
				nzero = (code & 0xF0) >> 4;
			if (nzero)
			{
				if (!n)
				{
					fprintf(stderr, "rle on dct\n");
					return 1;
				}
				if (n + nzero > 64)
				{
					fprintf(stderr, "rle overflow %d + %d\n", (int)n, (int)nzero);
					return 1;
				}
				for (int i = 0; i < nzero; ++i)
					values[n++] = 0;
			}
			code &= 0xF;
		}
		if (!code)
		{
			values[n++] = 0;
			continue;
		}
		uint32_t v;
		switch (getbits(jpeg, &v, code))
		{
			case -1:
				fprintf(stderr, "read failed\n");
				return 1;
			case -2:
				fprintf(stderr, "unexpected eof\n");
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

static void quantify(struct jpeg *jpeg, int component, int32_t *restrict dst, const int32_t *restrict src)
{
	int32_t *qt = &jpeg->dqt[jpeg->components[component].table][0];
	int32_t *iqt = &jpeg->idqt[jpeg->components[component].table][0];
	for (int i = 0; i < 64; ++i)
		dst[i] = ((src[i] + (qt[i] >> 1)) * iqt[i]) >> 16;
}

static void dequantify(struct jpeg *jpeg, int component, int32_t *restrict dst, const int32_t *restrict src)
{
	int32_t *qt = &jpeg->dqt[jpeg->components[component].table][0];
	for (int i = 0; i < 64; ++i)
		dst[i] = src[i] * qt[i];
}

static void zigzag(int32_t *restrict dst, const int32_t *restrict src)
{
	for (int i = 0; i < 64; ++i)
		dst[i] = src[zigzag_table[i]];
}

static void dezigzag(int32_t *restrict dst, const int32_t *restrict src)
{
	for (int i = 0; i < 64; ++i)
		dst[i] = src[inv_zigzag_table[i]];
}

static void dct1(int32_t *restrict dst, const int32_t *restrict src)
{
	for (int x = 0; x < 8; ++x)
	{
		const int32_t *col = &src[x];
		int32_t v0 = col[8 * 0] * 2048;
		int32_t v1 = col[8 * 1] * 2048;
		int32_t v2 = col[8 * 2] * 2048;
		int32_t v3 = col[8 * 3] * 2048;
		int32_t v4 = col[8 * 4] * 2048;
		int32_t v5 = col[8 * 5] * 2048;
		int32_t v6 = col[8 * 6] * 2048;
		int32_t v7 = col[8 * 7] * 2048;

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

		int32_t t8  =  ((z5 + z6) * (int32_t)(0.707106 * 1024) + 512) >> 10;
		int32_t t9  =  ((t2 + t3) * (int32_t)(0.707106 * 1024) + 512) >> 10;
		int32_t t10 =  ((t4 + t5) * (int32_t)(0.382683 * 1024) + 512) >> 10;
		int32_t t11 = ((( 0 - t4) * (int32_t)(0.541196 * 1024) + 512) >> 10) - t10;
		int32_t t12 = ((( 0 + t5) * (int32_t)(1.306562 * 1024) + 512) >> 10) - t10;

		int32_t t13 =  t9 + t3;
		int32_t t14 =  t3 - t9;
		int32_t t15 =  t8 + z7;
		int32_t t16 =  z7 - t8;
		int32_t t17 = t11 + t16;
		int32_t t18 = t15 + t12;
		int32_t t19 = t15 - t12;
		int32_t t20 = t16 - t11;

		dst[ 0 + x] = ( t6 * (int32_t)(0.353553 * 2048) + 2048) >> 12;
		dst[ 8 + x] = (t18 * (int32_t)(0.254897 * 2048) + 2048) >> 12;
		dst[16 + x] = (t13 * (int32_t)(0.270598 * 2048) + 2048) >> 12;
		dst[24 + x] = (t20 * (int32_t)(0.300672 * 2048) + 2048) >> 12;
		dst[32 + x] = ( t7 * (int32_t)(0.353553 * 2048) + 2048) >> 12;
		dst[40 + x] = (t17 * (int32_t)(0.449988 * 2048) + 2048) >> 12;
		dst[48 + x] = (t14 * (int32_t)(0.653281 * 2048) + 2048) >> 12;
		dst[56 + x] = (t19 * (int32_t)(1.281457 * 2048) + 2048) >> 12;
	}
}

static void dct2(int32_t *restrict dst, const int32_t *restrict src)
{
	for (int y = 0; y < 8; ++y)
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

		int32_t t8  =  ((z5 + z6) * (int32_t)(0.707106 * 1024) + 512) >> 10;
		int32_t t9  =  ((t2 + t3) * (int32_t)(0.707106 * 1024) + 512) >> 10;
		int32_t t10 =  ((t4 + t5) * (int32_t)(0.382683 * 1024) + 512) >> 10;
		int32_t t11 = ((( 0 - t4) * (int32_t)(0.541196 * 1024) + 512) >> 10) - t10;
		int32_t t12 = ((( 0 + t5) * (int32_t)(1.306562 * 1024) + 512) >> 10) - t10;

		int32_t t13 =  t9 + t3;
		int32_t t14 =  t3 - t9;
		int32_t t15 =  t8 + z7;
		int32_t t16 =  z7 - t8;
		int32_t t17 = t11 + t16;
		int32_t t18 = t15 + t12;
		int32_t t19 = t15 - t12;
		int32_t t20 = t16 - t11;

		int32_t *out = &dst[y * 8];
		out[0] = ( t6 * (int32_t)(0.353553 * 2048) + 1048576) >> 21;
		out[1] = (t18 * (int32_t)(0.254897 * 2048) + 1048576) >> 21;
		out[2] = (t13 * (int32_t)(0.270598 * 2048) + 1048576) >> 21;
		out[3] = (t20 * (int32_t)(0.300672 * 2048) + 1048576) >> 21;
		out[4] = ( t7 * (int32_t)(0.353553 * 2048) + 1048576) >> 21;
		out[5] = (t17 * (int32_t)(0.449988 * 2048) + 1048576) >> 21;
		out[6] = (t14 * (int32_t)(0.653281 * 2048) + 1048576) >> 21;
		out[7] = (t19 * (int32_t)(1.281457 * 2048) + 1048576) >> 21;
	}
}

static void dct(int32_t *restrict dst, const int32_t *restrict src)
{
	int32_t tmp[64];
	dct1(tmp, src);
	dct2(dst, tmp);
}

static void idct1(int32_t *restrict dst, const int32_t *restrict src)
{
	for (int x = 0; x < 8; ++x)
	{
		const int32_t *col = &src[x];
		int32_t v0 = col[8 * 0] * (int32_t)(2048 / 0.353553);
		int32_t v1 = col[8 * 1] * (int32_t)(2048 / 0.254897);
		int32_t v2 = col[8 * 2] * (int32_t)(2048 / 0.270598);
		int32_t v3 = col[8 * 3] * (int32_t)(2048 / 0.300672);
		int32_t v4 = col[8 * 4] * (int32_t)(2048 / 0.353553);
		int32_t v5 = col[8 * 5] * (int32_t)(2048 / 0.449988);
		int32_t v6 = col[8 * 6] * (int32_t)(2048 / 0.653281);
		int32_t v7 = col[8 * 7] * (int32_t)(2048 / 1.281457);

		int32_t z0 = (v5 - v3 + 1) >> 1;
		int32_t z1 = (v1 - v7 + 1) >> 1;
		int32_t z2 = (v1 + v7 + 1) >> 1;
		int32_t z3 = (v5 + v3 + 1) >> 1;
		int32_t z4 = (z2 + z3 + 1) >> 1;
		int32_t z5 = (v2 + v6 + 1) >> 1;
		int32_t z6 = (z2 - z3 + 1) >> 1;
		int32_t z7 = (v2 - v6 + 1) >> 1;

		int32_t t0 = (v0 + v4 + 1) >> 1;
		int32_t t1 = (v0 - v4 + 1) >> 1;

		int32_t t2 = (((z0 - z1) * (int32_t)(0.382683 * 512) + 256) >> 9);
		int32_t t3 = (((z1 -  0) * (int32_t)(0.541196 * 512) + 256) >> 9) - z4 - t2;
		int32_t t4 = (((z6 -  0) * (int32_t)(1.414215 * 512) + 256) >> 9) - t3;
		int32_t t5 = (((z0 -  0) * (int32_t)(1.306562 * 512) + 256) >> 9) - t2 - t4;
		int32_t t6 = (((z7 -  0) * (int32_t)(1.414215 * 512) + 256) >> 9) - z5;

		int32_t t7  = (t0 + z5 + 1) >> 1;
		int32_t t8  = (t1 + t6 + 1) >> 1;
		int32_t t9  = (t1 - t6 + 1) >> 1;
		int32_t t10 = (t0 - z5 + 1) >> 1;

		dst[ 0 + x] = (t7  + z4 + 4) >> 3;
		dst[ 8 + x] = (t8  + t3 + 4) >> 3;
		dst[16 + x] = (t9  + t4 + 4) >> 3;
		dst[24 + x] = (t10 + t5 + 4) >> 3;
		dst[32 + x] = (t10 - t5 + 4) >> 3;
		dst[40 + x] = (t9  - t4 + 4) >> 3;
		dst[48 + x] = (t8  - t3 + 4) >> 3;
		dst[56 + x] = (t7  - z4 + 4) >> 3;
	}
}

static void idct2(int32_t *restrict dst, const int32_t *restrict src)
{
	for (int y = 0; y < 8; ++y)
	{
		const int32_t *row = &src[y * 8];
		int32_t v0 = (row[0] * (int32_t)(2048 / 0.353553) + 1024) >> 10;
		int32_t v1 = (row[1] * (int32_t)(2048 / 0.254897) + 1024) >> 10;
		int32_t v2 = (row[2] * (int32_t)(2048 / 0.270598) + 1024) >> 10;
		int32_t v3 = (row[3] * (int32_t)(2048 / 0.300672) + 1024) >> 10;
		int32_t v4 = (row[4] * (int32_t)(2048 / 0.353553) + 1024) >> 10;
		int32_t v5 = (row[5] * (int32_t)(2048 / 0.449988) + 1024) >> 10;
		int32_t v6 = (row[6] * (int32_t)(2048 / 0.653281) + 1024) >> 10;
		int32_t v7 = (row[7] * (int32_t)(2048 / 1.281457) + 1024) >> 10;

		int32_t z0 = (v5 - v3 + 1) >> 1;
		int32_t z1 = (v1 - v7 + 1) >> 1;
		int32_t z2 = (v1 + v7 + 1) >> 1;
		int32_t z3 = (v5 + v3 + 1) >> 1;
		int32_t z4 = (z2 + z3 + 1) >> 1;
		int32_t z5 = (v2 + v6 + 1) >> 1;
		int32_t z6 = (z2 - z3 + 1) >> 1;
		int32_t z7 = (v2 - v6 + 1) >> 1;

		int32_t t0 = (v0 + v4 + 1) >> 1;
		int32_t t1 = (v0 - v4 + 1) >> 1;

		int32_t t2 = (((z0 - z1) * (int32_t)(0.382683 * 512) + 256) >> 9);
		int32_t t3 = (((z1 -  0) * (int32_t)(0.541196 * 512) + 256) >> 9) - z4 - t2;
		int32_t t4 = (((z6 -  0) * (int32_t)(1.414215 * 512) + 256) >> 9) - t3;
		int32_t t5 = (((z0 -  0) * (int32_t)(1.306562 * 512) + 256) >> 9) - t2 - t4;
		int32_t t6 = (((z7 -  0) * (int32_t)(1.414215 * 512) + 256) >> 9) - z5;

		int32_t t7  = (t0 + z5 + 1) >> 1;
		int32_t t8  = (t1 + t6 + 1) >> 1;
		int32_t t9  = (t1 - t6 + 1) >> 1;
		int32_t t10 = (t0 - z5 + 1) >> 1;

		int32_t *out = &dst[y * 8];
		out[0] = (t7  + z4 + 1024) >> 11;
		out[1] = (t8  + t3 + 1024) >> 11;
		out[2] = (t9  + t4 + 1024) >> 11;
		out[3] = (t10 + t5 + 1024) >> 11;
		out[4] = (t10 - t5 + 1024) >> 11;
		out[5] = (t9  - t4 + 1024) >> 11;
		out[6] = (t8  - t3 + 1024) >> 11;
		out[7] = (t7  - z4 + 1024) >> 11;
	}
}

static void idct(int32_t *restrict dst, const int32_t *restrict src)
{
	int32_t tmp[64];
	idct1(tmp, src);
	idct2(dst, tmp);
}

static int encode_block_component(struct jpeg *jpeg, const int32_t *values, size_t component)
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

static void prepare_block_component(struct jpeg *jpeg, int32_t *values, size_t component, size_t bx, size_t by)
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
	}
	else
	{
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
}

static int encode_block(struct jpeg *jpeg)
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
			for (int i = 0; i < jpeg->components_count; ++i)
				jpeg->components[i].prev_dc = 0;
			if (flushbits(jpeg)
			 || putbyte(jpeg, 0xFF)
			 || putbyte(jpeg, JPEG_CHUNK_RST0 + jpeg->restart_id))
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

static int decode_block_component(struct jpeg *jpeg, int32_t *values, size_t component)
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

static void apply_block_component(struct jpeg *jpeg, int32_t *values, size_t component, size_t bx, size_t by)
{
	int32_t *data = jpeg->components[component].data;
	data += (jpeg->block_y + by * jpeg->components[component].height) * jpeg->component_width;
	data += jpeg->block_x + bx * jpeg->components[component].width;
	for (size_t y = 0; y < jpeg->components[component].height; ++y)
	{
		int32_t *dst = &data[y * jpeg->component_width];
		int32_t *src = &values[((y * jpeg->components[component].iscale_y) / 8) * 8];
		for (size_t x = 0; x < jpeg->components[component].width; ++x)
			dst[x] = src[(x * jpeg->components[component].iscale_x) / 8];
	}
}

static int decode_block(struct jpeg *jpeg)
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
			for (int i = 0; i < jpeg->components_count; ++i)
				jpeg->components[i].prev_dc = 0;
			jpeg->bit_len = 0;
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

static void merge_gray(struct jpeg *jpeg, uint8_t *data)
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

static void merge_rgb(struct jpeg *jpeg, uint8_t *data)
{
	for (size_t y = 0; y < jpeg->height; ++y)
	{
		const int32_t *Y_data = &jpeg->components[0].data[y * jpeg->component_width];
		const int32_t *Cb_data = &jpeg->components[1].data[y * jpeg->component_width];
		const int32_t *Cr_data = &jpeg->components[2].data[y * jpeg->component_width];
		uint8_t *row = &data[y * jpeg->width * 3];
		for (size_t x = 0; x < jpeg->width; ++x)
		{
			uint8_t *dst = &row[x * 3];
			int32_t Y = Y_data[x];
			int32_t Cb = Cb_data[x];
			int32_t Cr = Cr_data[x];
			int32_t r = (128 + Y) * 65536;
			int32_t g = (128 + Y) * 65536;
			int32_t b = (128 + Y) * 65536;
			r += Cr * (int32_t)(1.402   * 65536);
			g -= Cr * (int32_t)(0.71414 * 65536);
			b += Cb * (int32_t)(1.772   * 65536);
			g -= Cb * (int32_t)(0.34414 * 65536);
			r = (r + 32768) >> 16;
			g = (g + 32768) >> 16;
			b = (b + 32768) >> 16;
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
	}
}

static void split_gray(struct jpeg *jpeg, const uint8_t *data)
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

static void split_rgb(struct jpeg *jpeg, const uint8_t *data)
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
			Y_data[x]  = (Y_data[x]  + 32768) >> 16;
			Cb_data[x] = (Cb_data[x] + 32768) >> 16;
			Cr_data[x] = (Cr_data[x] + 32768) >> 16;
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

static int parse_sos(struct jpeg *jpeg, uint16_t length)
{
	if (length != 12 && length != 8)
	{
		fprintf(stderr, "invalid length\n");
		return 1;
	}
	uint8_t bytes[10];
	if (fread(&bytes, 1, length - 2, jpeg->fp) != length - 2u)
	{
		fprintf(stderr, "unexpected EOF\n");
		return 1;
	}
	if (bytes[0] != jpeg->components_count)
	{
		fprintf(stderr, "invalid components count\n");
		return 1;
	}
	for (int i = 0; i < bytes[0]; ++i)
	{
		uint8_t base = 1 + i * 2;
		uint8_t id = bytes[base];
		if (id <= 0 || id > bytes[0])
		{
			fprintf(stderr, "invalid component id\n");
			return 1;
		}
		id--;
		jpeg->components[id].dct = bytes[base + 1] >> 4;
		jpeg->components[id].act = bytes[base + 1] & 0xF;
		printf("component[%u] %d %d\n", id,
		       jpeg->components[id].dct,
		       jpeg->components[id].act);
	}
	/* XXX spectral select, successive approx */
	return 0;
}

static int parse_dri(struct jpeg *jpeg, uint16_t length)
{
	if (length != 4)
	{
		fprintf(stderr, "invalid length\n");
		return 1;
	}
	uint8_t bytes[2];
	if (fread(&bytes, 1, sizeof(bytes), jpeg->fp) != sizeof(bytes))
	{
		fprintf(stderr, "unexpected EOF\n");
		return 1;
	}
	jpeg->restart_interval = (bytes[0] << 8) | bytes[1];
	printf("restart interval: %u\n", jpeg->restart_interval);
	return 0;
}

static int jpeg_read_headers(struct jpeg *jpeg)
{
	if (!jpeg->fp)
	{
		fprintf(stderr, "no file to read from\n");
		return 1;
	}
	jpeg->components_count = 0;
	jpeg->huff_count = 0;
	while (!jpeg->sos)
	{
		uint8_t chunk[2];
		if (fread(chunk, 1, 2, jpeg->fp) != 2)
		{
			if (feof(jpeg->fp))
				fprintf(stderr, "unexpected EOF\n");
			else
				fprintf(stderr, "fread: %s\n", strerror(errno));
			return 1;
		}
		if (chunk[0] != 0xFF)
		{
			fprintf(stderr, "invalid chunk id\n");
			return 1;
		}
		if (chunk[1] == JPEG_CHUNK_SOI)
		{
			printf("SOI\n");
			continue;
		}
		uint8_t length_bytes[2];
		if (fread(length_bytes, 1, 2, jpeg->fp) != 2)
		{
			fprintf(stderr, "failed to read chunk length\n");
			return 1;
		}
		uint16_t length = (length_bytes[0] << 8) | length_bytes[1];
		switch (chunk[1])
		{
			case JPEG_CHUNK_APP0:
				printf("APP\n");
				if (parse_app0(jpeg, length))
					return 1;
				break;
			case JPEG_CHUNK_APP2:
				printf("APP2\n");
				fseek(jpeg->fp, length - 2, SEEK_CUR);
				break;
			case JPEG_CHUNK_DQT:
				printf("DQT\n");
				if (parse_dqt(jpeg, length))
					return 1;
				break;
			case JPEG_CHUNK_SOF0:
				printf("SOF0\n");
				if (parse_sof0(jpeg, length))
					return 1;
				break;
			case JPEG_CHUNK_DHT:
				printf("DHT\n");
				if (parse_dht(jpeg, length))
					return 1;
				break;
			case JPEG_CHUNK_SOS:
				printf("SOS\n");
				if (parse_sos(jpeg, length))
					return 1;
				jpeg->sos = 1;
				break;
			case JPEG_CHUNK_DRI:
				printf("DRI\n");
				if (parse_dri(jpeg, length))
					return 1;
				break;
			default:
				printf("unknown chunk: %02x%02x\n", chunk[0], chunk[1]);
				fseek(jpeg->fp, length - 2, SEEK_CUR);
				break;
		}
	}
	if (!jpeg->huff_count)
	{
		fprintf(stderr, "no huffman table read\n");
		return 1;
	}
	if (!jpeg->components_count)
	{
		fprintf(stderr, "no component read\n");
		return 1;
	}
	for (int i = 0; i < jpeg->components_count; ++i)
	{
		if (!jpeg->dqt[jpeg->components[i].table][0])
		{
			fprintf(stderr, "missing dqt\n");
			return 1;
		}
		int dct = get_huffman(jpeg, 0, jpeg->components[i].dct);
		int act = get_huffman(jpeg, 1, jpeg->components[i].act);
		if (dct == -1 || act == -1)
		{
			fprintf(stderr, "missing huffman table\n");
			return 1;
		}
		jpeg->components[i].dc_huff = dct;
		jpeg->components[i].ac_huff = act;
	}
	return 0;
}

static int jpeg_read_data(struct jpeg *jpeg, void *data)
{
	if (!jpeg->fp || !jpeg->sos)
		return 0;
	jpeg->restart_count = jpeg->restart_interval;
	while (1)
	{
		if (decode_block(jpeg))
		{
			if (!jpeg->eof)
			{
				fprintf(stderr, "unexpected eof\n");
				return 1;
			}
			break;
		}
	}
	if (jpeg->components_count == 1)
		merge_gray(jpeg, data);
	else
		merge_rgb(jpeg, data);
	return 0;
}

static void jpeg_get_info(struct jpeg *jpeg, uint32_t *width, uint32_t *height, uint8_t *components)
{
	if (width)
		*width = jpeg->width;
	if (height)
		*height = jpeg->height;
	if (components)
		*components = jpeg->components_count;
}

static void jpeg_set_quality(struct jpeg *jpeg, int quality)
{
	if (quality < 1)
		quality = 1;
	if (quality > 100)
		quality = 100;
	generate_qdt(&jpeg->dqt[0][0], quality, luma_dqt_table);
	generate_qdt(&jpeg->dqt[1][0], quality, chroma_dqt_table);
}

static void jpeg_set_restart_interval(struct jpeg *jpeg, uint16_t restart_interval)
{
	jpeg->restart_interval = restart_interval;
}

static int jpeg_set_subsampling(struct jpeg *jpeg, int subsampling)
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
			return 1;
	}
}

static int jpeg_set_info(struct jpeg *jpeg, uint32_t width, uint32_t height, uint8_t components)
{
	if (!width
	 || width > UINT16_MAX
	 || !height
	 || height > UINT16_MAX)
	{
		fprintf(stderr, "invalid dimensions\n");
		return 1;
	}
	if (components != 1 && components != 3)
	{
		fprintf(stderr, "invalid components count\n");
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
	for (int i = 0; i < components; ++i)
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
			fprintf(stderr, "malloc failed\n");
			return 1;
		}
	}
	jpeg->huff_tables[0].class = 0;
	jpeg->huff_tables[0].dst = 0;
	huffman_generate(&jpeg->huff_tables[0].huffman, huffman_luma_dc_counts, huffman_luma_dc_values);
	jpeg->huff_tables[1].class = 1;
	jpeg->huff_tables[1].dst = 0;
	huffman_generate(&jpeg->huff_tables[1].huffman, huffman_luma_ac_counts, huffman_luma_ac_values);
	jpeg->huff_tables[2].class = 0;
	jpeg->huff_tables[2].dst = 1;
	huffman_generate(&jpeg->huff_tables[2].huffman, huffman_chroma_dc_counts, huffman_chroma_dc_values);
	jpeg->huff_tables[3].class = 1;
	jpeg->huff_tables[3].dst = 1;
	huffman_generate(&jpeg->huff_tables[3].huffman, huffman_chroma_ac_counts, huffman_chroma_ac_values);
	jpeg->huff_count = 4;
	return 0;
}

static int write_chunk(struct jpeg *jpeg, uint8_t id, size_t size)
{
	if (size > UINT16_MAX)
		return 1;
	size += 2;
	if (putbyte(jpeg, 0xFF)
	 || putbyte(jpeg, id)
	 || putbyte(jpeg, (size >> 8) & 0xFF)
	 || putbyte(jpeg, (size >> 0) & 0xFF))
		return 1;
	return 0;
}

static int write_soi(struct jpeg *jpeg)
{
	if (putbyte(jpeg, 0xFF)
	 || putbyte(jpeg, JPEG_CHUNK_SOI))
		return 1;
	return 0;
}

static int write_app0(struct jpeg *jpeg)
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
	if (write_chunk(jpeg, JPEG_CHUNK_APP0, length)
	 || fwrite(bytes, 1, sizeof(bytes), jpeg->fp) != sizeof(bytes))
		return 1;
	if (jpeg->thumbnail)
	{
		if (fwrite(jpeg->thumbnail, 1, thumbnail_size, jpeg->fp) != thumbnail_size)
			return 1;
	}
	return 0;
}

static int write_dqt(struct jpeg *jpeg, int id)
{
	uint8_t dqt[64];
	for (int i = 0; i < 64; ++i)
		dqt[i] = jpeg->dqt[id][i];
	if (write_chunk(jpeg, JPEG_CHUNK_DQT, 64 + 1)
	 || putbyte(jpeg, id)
	 || fwrite(&dqt[0], 1, 64, jpeg->fp) != 64)
		return 1;
	return 0;
}

static int write_sof0(struct jpeg *jpeg)
{
	uint8_t bytes[15];
	size_t length = 6 + 3 * jpeg->components_count;
	bytes[0] = jpeg->bpp;
	bytes[1] = jpeg->height >> 8;
	bytes[2] = jpeg->height >> 0;
	bytes[3] = jpeg->width >> 8;
	bytes[4] = jpeg->width >> 0;
	bytes[5] = jpeg->components_count;
	for (int i = 0; i < jpeg->components_count; ++i)
	{
		uint8_t base = 6 + 3 * i;
		bytes[base + 0] = i + 1;
		bytes[base + 1] = (jpeg->components[i].scale_x << 4)
		                | (jpeg->components[i].scale_y << 0);
		bytes[base + 2] = jpeg->components[i].table;
	}
	if (write_chunk(jpeg, JPEG_CHUNK_SOF0, length)
	 || fwrite(bytes, 1, length, jpeg->fp) != length)
		return 1;
	return 0;
}

static int write_dht(struct jpeg *jpeg, int id)
{
	struct huffman *huffman = &jpeg->huff_tables[id].huffman;
	size_t sum = 0;
	for (int i = 0; i < 16; ++i)
		sum += huffman->counts[i];
	uint8_t bytes[17 + 255];
	size_t length = 17 + sum;
	bytes[0] = (jpeg->huff_tables[id].class << 4)
	         | (jpeg->huff_tables[id].dst << 0);
	for (int i = 0; i < 16; ++i)
		bytes[i + 1] = huffman->counts[i];
	size_t n = 0;
	for (int i = 0; i < 16; ++i)
	{
		for (int j = 0; j < huffman->counts[i]; ++j)
		{
			bytes[17 + n] = huffman->values[n];
			n++;
		}
	}
	if (write_chunk(jpeg, JPEG_CHUNK_DHT, length)
	 || fwrite(bytes, 1, length, jpeg->fp) != length)
		return 1;
	return 0;
}

static int write_dri(struct jpeg *jpeg)
{
	if (!jpeg->restart_interval)
		return 0;
	uint8_t bytes[2];
	bytes[0] = jpeg->restart_interval >> 8;
	bytes[1] = jpeg->restart_interval >> 0;
	if (write_chunk(jpeg, JPEG_CHUNK_DRI, sizeof(bytes))
	 || fwrite(bytes, 1, sizeof(bytes), jpeg->fp) != sizeof(bytes))
		return 1;
	return 0;
}

static int write_sos(struct jpeg *jpeg)
{
	uint8_t bytes[10];
	size_t length = 4 + 2 * jpeg->components_count;
	bytes[0] = jpeg->components_count;
	for (int i = 0; i < jpeg->components_count; ++i)
	{
		size_t base = 1 + 2 * i;
		bytes[base + 0] = i + 1;
		bytes[base + 1] = (jpeg->components[i].dct << 4)
		                | (jpeg->components[i].act << 0);
	}
	bytes[length - 3] = 0;
	bytes[length - 2] = 0;
	bytes[length - 1] = 0;
	if (write_chunk(jpeg, JPEG_CHUNK_SOS, length)
	 || fwrite(bytes, 1, length, jpeg->fp) != length)
		return 1;
	return 0;
}

static int jpeg_write_headers(struct jpeg *jpeg)
{
	if (!jpeg->components_count || !jpeg->fp)
		return 1;
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

static int jpeg_write_data(struct jpeg *jpeg, const void *data)
{
	if (!jpeg->components_count || !jpeg->fp || !jpeg->sos)
		return 1;
	for (int i = 0; i < 64; ++i)
	{
		jpeg->idqt[0][i] = 65536 / jpeg->dqt[0][i];
		jpeg->idqt[1][i] = 65536 / jpeg->dqt[1][i];
	}
	if (jpeg->components_count == 1)
		split_gray(jpeg, data);
	else
		split_rgb(jpeg, data);
	jpeg->restart_count = jpeg->restart_interval;
	jpeg->bit_buf = 0;
	jpeg->bit_len = sizeof(jpeg->bit_buf) * 8;
	jpeg->restart_id = 0;
	while (jpeg->block_y < jpeg->height)
	{
		if (encode_block(jpeg))
			return 1;
	}
	if (flushbits(jpeg)
	 || putbyte(jpeg, 0xFF)
	 || putbyte(jpeg, JPEG_CHUNK_EOI))
		return 1;
	return 0;
}

static struct jpeg *jpeg_new(void)
{
	struct jpeg *jpeg = calloc(1, sizeof(*jpeg));
	if (!jpeg)
		return NULL;
	generate_qdt(&jpeg->dqt[0][0], 50, luma_dqt_table);
	generate_qdt(&jpeg->dqt[1][0], 50, chroma_dqt_table);
	return jpeg;
}

static void jpeg_init_io(struct jpeg *jpeg, FILE *fp)
{
	jpeg->fp = fp;
	jpeg->bit_len = 0;
	jpeg->bit_buf = 0;
	jpeg->eof = 0;
	jpeg->sos = 0;
	jpeg->block_x = 0;
	jpeg->block_y = 0;
	jpeg->restart_count = 0;
	for (int i = 0; i < 3; ++i)
		jpeg->components[i].prev_dc = 0;
}

static const uint8_t *jpeg_get_thumbnail(struct jpeg *jpeg, uint8_t *width, uint8_t *height)
{
	if (width)
		*width = jpeg->thumbnail_width;
	if (height)
		*height = jpeg->thumbnail_height;
	return jpeg->thumbnail;
}

static int jpeg_set_thumbnail(struct jpeg *jpeg, const uint8_t *data, uint8_t width, uint8_t height)
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
		return 1;
	memcpy(dup, data, width * height);
	free(jpeg->thumbnail);
	jpeg->thumbnail = dup;
	jpeg->thumbnail_width = width;
	jpeg->thumbnail_height = height;
	return 0;
}

static void jpeg_free(struct jpeg *jpeg)
{
	if (!jpeg)
		return;
	for (size_t i = 0; i < jpeg->components_count; ++i)
		free(jpeg->components[i].data);
	free(jpeg->thumbnail);
	free(jpeg);
}

static void png_write(const char *file, const void *data, uint32_t width, uint32_t height, int components)
{
	png_bytep row_pointers[height];
	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;
	FILE *fp = NULL;
	if (!(fp = fopen(file, "wb")))
		goto error1;
	if (!(png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL)))
		goto error2;
	if (!(info_ptr = png_create_info_struct(png_ptr)))
		goto error3;
	if (setjmp(png_jmpbuf(png_ptr)))
		goto error3;
	png_init_io(png_ptr, fp);
	if (setjmp(png_jmpbuf(png_ptr)))
		goto error3;
	int color_type;
	switch (components)
	{
		case 1:
			color_type = PNG_COLOR_TYPE_GRAY;
			break;
		case 3:
			color_type = PNG_COLOR_TYPE_RGB;
			break;
		default:
			goto error3;
	}
	png_set_IHDR(png_ptr, info_ptr, width, height, 8, color_type, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_write_info(png_ptr, info_ptr);
	if (setjmp(png_jmpbuf(png_ptr)))
		goto error3;
	for (uint32_t i = 0; i < height; ++i)
		row_pointers[i] = (uint8_t*)data + i * width * components;
	png_write_image(png_ptr, row_pointers);
	if (setjmp(png_jmpbuf(png_ptr)))
		goto error3;
	png_write_end(png_ptr, NULL);
	fclose(fp);
	return;
error3:
	png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)0);
error2:
	fclose(fp);
error1:
	return;
}

static int rd(const char *filename, uint8_t **data, uint32_t *width, uint32_t *height, uint8_t *components)
{
	FILE *fp = NULL;
	struct jpeg *jpeg = NULL;
	int ret = 1;

	fp = fopen(filename, "rb");
	if (!fp)
	{
		fprintf(stderr, "fopen(%s): %s\n", filename,
		        strerror(errno));
		goto end;
	}
	jpeg = jpeg_new();
	if (!jpeg)
	{
		fprintf(stderr, "jpeg open failed\n");
		goto end;
	}
	jpeg_init_io(jpeg, fp);
	if (jpeg_read_headers(jpeg))
	{
		fprintf(stderr, "failed to read jpeg headers\n");
		goto end;
	}
	jpeg_get_info(jpeg, width, height, components);
	*data = malloc(*width * *height * *components);
	if (!*data)
	{
		fprintf(stderr, "image data allocation failed\n");
		goto end;
	}
	if (jpeg_read_data(jpeg, *data))
	{
		fprintf(stderr, "failed to read jpeg data\n");
		goto end;
	}
	ret = 0;

end:
	if (fp)
		fclose(fp);
	jpeg_free(jpeg);
	return ret;
}

static int wr(const char *filename, const uint8_t *data, uint32_t width, uint32_t height, uint8_t components, int quality)
{
	FILE *fp = NULL;
	struct jpeg *jpeg = NULL;
	int ret = 1;

	fp = fopen(filename, "wr");
	if (!fp)
	{
		fprintf(stderr, "fopen(%s): %s\n", filename,
		        strerror(errno));
		goto end;
	}
	jpeg = jpeg_new();
	if (!jpeg)
	{
		fprintf(stderr, "jpeg open failed\n");
		goto end;
	}
	jpeg_init_io(jpeg, fp);
	jpeg_set_quality(jpeg, quality);
	jpeg_set_restart_interval(jpeg, 10);
	if (jpeg_set_subsampling(jpeg, JPEG_SUBSAMPLING_444))
	{
		fprintf(stderr, "failed to set subsampling\n");
		goto end;
	}
	if (jpeg_set_info(jpeg, width, height, components))
	{
		fprintf(stderr, "failed to set jpeg info\n");
		goto end;
	}
	if (jpeg_write_headers(jpeg))
	{
		fprintf(stderr, "failed to write jpeg components\n");
		goto end;
	}
	if (jpeg_write_data(jpeg, data))
	{
		fprintf(stderr, "failed to write jpeg data\n");
		goto end;
	}
	ret = 0;

end:
	if (fp)
		fclose(fp);
	jpeg_free(jpeg);
	return ret;
}

int main(int argc, char **argv)
{
	if (argc < 3)
	{
		fprintf(stderr, "%s <src> <dst> [quality]\n", argv[0]);
		return EXIT_FAILURE;
	}
	int quality = 50;
	if (argc >= 4)
		quality = atoi(argv[3]);
	for (int i = 0; i < 2; ++i)
	{
		uint8_t *data;
		uint32_t width;
		uint32_t height;
		uint8_t components;
		int64_t s = nanotime();
		if (rd(argv[1], &data, &width, &height, &components))
			return EXIT_FAILURE;
		int64_t e = nanotime();
		printf("rd: %" PRId64 " us\n", (e - s) / 1000);
		png_write("test.png", data, width, height, components);
		s = nanotime();
		if (wr(argv[2], data, width, height, components, quality))
			return EXIT_FAILURE;
		e = nanotime();
		printf("wr: %" PRId64 " us\n", (e - s) / 1000);
		free(data);
	}
	return EXIT_SUCCESS;
}
