#include "bitstream.h"

#include <GLFW/glfw3.h>

#include <inttypes.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <png.h>

static const uint32_t mba_values[] =
{
	0x01, 0x03, 0x02, 0x03, 0x02, 0x03, 0x02, 0x07,
	0x06, 0x0B, 0x0A, 0x09, 0x08, 0x07, 0x06, 0x17,
	0x16, 0x15, 0x14, 0x13, 0x12, 0x23, 0x22, 0x21,
	0x20, 0x1F, 0x1E, 0x1D, 0x1C, 0x1B, 0x1A, 0x19,
	0x18, 0x0F,
};

static const uint8_t mba_bits[] =
{
	0x01, 0x03, 0x03, 0x04, 0x04, 0x05, 0x05, 0x07,
	0x07, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0A,
	0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0B, 0x0B, 0x0B,
	0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B,
	0x0B, 0x0B,
};

static const uint32_t mvd_values[] =
{
	0x01, 0x02, 0x03, 0x02, 0x03, 0x02, 0x03, 0x06,
	0x07, 0x0A, 0x0B, 0x08, 0x09, 0x06, 0x07, 0x16,
	0x17, 0x14, 0x15, 0x12, 0x13, 0x22, 0x23, 0x20,
	0x21, 0x1E, 0x1F, 0x1C, 0x1D, 0x1A, 0x1B, 0x19,
};

static const uint8_t mvd_bits[] =
{
	0x01, 0x03, 0x03, 0x04, 0x04, 0x05, 0x05, 0x07,
	0x07, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0A,
	0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0B, 0x0B, 0x0B,
	0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B,
};

static const uint32_t cbp_values[] =
{
	0x07, 0x0D, 0x0C, 0x0B, 0x0A, 0x13, 0x12, 0x11,
	0x10, 0x0F, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x09,
	0x08, 0x0F, 0x0E, 0x0D, 0x0C, 0x17, 0x16, 0x15,
	0x14, 0x13, 0x12, 0x11, 0x10, 0x1F, 0x1E, 0x1D,
	0x1C, 0x1B, 0x1A, 0x19, 0x18, 0x17, 0x16, 0x15,
	0x14, 0x13, 0x12, 0x11, 0x10, 0x0F, 0x0E, 0x0D,
	0x0C, 0x0B, 0x0A, 0x09, 0x08, 0x07, 0x06, 0x05,
	0x04, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02,
};

static const uint8_t cbp_bits[] =
{
	0x03, 0x04, 0x04, 0x04, 0x04, 0x05, 0x05, 0x05,
	0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
	0x05, 0x06, 0x06, 0x06, 0x06, 0x07, 0x07, 0x07,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x08, 0x08, 0x08,
	0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
	0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
	0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
	0x08, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
};

static const uint8_t cbp_val[] =
{
	60,  4,  8, 16, 32, 12, 48, 20,
	40, 28, 44, 52, 56,  1, 61,  2,
	62, 24, 36,  3, 63,  5,  9, 17,
	33,  6, 10, 18, 34,  7, 11, 19,
	35, 13, 49, 21, 41, 14, 50, 22,
	42, 15, 51, 23, 43, 25, 37, 26,
	38, 29, 45, 53, 57, 30, 46, 54,
	58, 31, 47, 55, 59, 27, 39,
};

/* XXX should be huffman-like structure to make decoding faster */
static const uint32_t tcoeff_values[] =
{
	0x02, 0x06, 0x07, 0x06, 0x07, 0x08, 0x09, 0x0A,
	0x0B, 0x01, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
	0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
	0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
	0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
	0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
	0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
	0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
	0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
	0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
	0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
};

static const uint8_t tcoeff_bits[] =
{
	0x02, 0x03, 0x03, 0x04, 0x04, 0x05, 0x05, 0x05,
	0x05, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
	0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B,
	0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B,
	0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D,
	0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D,
	0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D,
	0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D,
	0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E,
	0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E,
	0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E,
	0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E,
};

static const uint8_t tcoeff_run[] =
{
	0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x02,
	0x02, 0xFF, 0x00, 0x00, 0x04, 0x04, 0x03, 0x03,
	0x07, 0x07, 0x06, 0x06, 0x01, 0x01, 0x05, 0x05,
	0x02, 0x02, 0x09, 0x09, 0x00, 0x00, 0x08, 0x08,
	0x0D, 0x0D, 0x00, 0x00, 0x0C, 0x0C, 0x0B, 0x0B,
	0x03, 0x03, 0x01, 0x01, 0x00, 0x00, 0x0A, 0x0A,
	0x10, 0x10, 0x05, 0x05, 0x00, 0x00, 0x02, 0x02,
	0x01, 0x01, 0x0F, 0x0F, 0x0E, 0x0E, 0x04, 0x04,
	0x00, 0x00, 0x08, 0x08, 0x04, 0x04, 0x00, 0x00,
	0x02, 0x02, 0x07, 0x07, 0x15, 0x15, 0x14, 0x14,
	0x00, 0x00, 0x13, 0x13, 0x12, 0x12, 0x01, 0x01,
	0x03, 0x03, 0x00, 0x00, 0x06, 0x06, 0x11, 0x11,
	0x0A, 0x0A, 0x09, 0x09, 0x05, 0x05, 0x03, 0x03,
	0x02, 0x02, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1A, 0x1A,
	0x19, 0x19, 0x18, 0x18, 0x17, 0x17, 0x16, 0x16,
};

static const uint8_t tcoeff_level[] =
{
	0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x01,
	0x01, 0xFF, 0x03, 0x03, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x01, 0x01,
	0x02, 0x02, 0x01, 0x01, 0x04, 0x04, 0x01, 0x01,
	0x01, 0x01, 0x06, 0x06, 0x01, 0x01, 0x01, 0x01,
	0x02, 0x02, 0x03, 0x03, 0x05, 0x05, 0x01, 0x01,
	0x01, 0x01, 0x02, 0x02, 0x07, 0x07, 0x03, 0x03,
	0x04, 0x04, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02,
	0x0B, 0x0B, 0x02, 0x02, 0x03, 0x03, 0x0A, 0x0A,
	0x04, 0x04, 0x02, 0x02, 0x01, 0x01, 0x01, 0x01,
	0x09, 0x09, 0x01, 0x01, 0x01, 0x01, 0x05, 0x05,
	0x03, 0x03, 0x08, 0x08, 0x02, 0x02, 0x01, 0x01,
	0x02, 0x02, 0x02, 0x02, 0x03, 0x03, 0x04, 0x04,
	0x05, 0x05, 0x07, 0x07, 0x06, 0x06, 0x0F, 0x0F,
	0x0E, 0x0E, 0x0D, 0x0D, 0x0C, 0x0C, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
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

#define MTYPE_INTRA  (1 << 0)
#define MTYPE_MQUANT (1 << 1)
#define MTYPE_MVD    (1 << 2)
#define MTYPE_CBP    (1 << 3)
#define MTYPE_TCOEFF (1 << 4)
#define MTYPE_FIL    (1 << 5)

static const uint8_t mtype_flags[] =
{
	MTYPE_CBP | MTYPE_TCOEFF,
	MTYPE_MVD | MTYPE_CBP | MTYPE_TCOEFF | MTYPE_FIL,
	MTYPE_MVD | MTYPE_FIL,
	MTYPE_INTRA | MTYPE_TCOEFF,
	MTYPE_MQUANT | MTYPE_CBP | MTYPE_TCOEFF,
	MTYPE_MQUANT | MTYPE_MVD | MTYPE_CBP | MTYPE_TCOEFF | MTYPE_FIL,
	MTYPE_INTRA | MTYPE_MQUANT | MTYPE_TCOEFF,
	MTYPE_MVD | MTYPE_CBP | MTYPE_TCOEFF,
	MTYPE_MVD,
	MTYPE_MQUANT | MTYPE_MVD | MTYPE_CBP | MTYPE_TCOEFF,
};

struct h261_frame
{
	int32_t Y[352 * 288];
	int32_t Cb[352 * 288];
	int32_t Cr[352 * 288];
	int32_t cif;
	uint32_t width;
	uint32_t height;
};

struct h261
{
	FILE *fp;
	struct bitstream bs;
	int32_t eof;
	int32_t quant;
	struct h261_frame frames[2];
	struct h261_frame *frame;
	struct h261_frame *prev_frame;
	int32_t frame_ff;
	int32_t mvdx;
	int32_t mvdy;
};

uint64_t nanotime(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec * 1000000000 + ts.tv_nsec;
}

static void idct1(int32_t *restrict dst, const int32_t *restrict src)
{
	for (size_t x = 0; x < 8; ++x)
	{
		const int32_t *col = &src[x];
		int32_t v0 = col[8 * 0] * (int32_t)(128 / 0.353553);
		int32_t v1 = col[8 * 1] * (int32_t)(128 / 0.254897);
		int32_t v2 = col[8 * 2] * (int32_t)(128 / 0.270598);
		int32_t v3 = col[8 * 3] * (int32_t)(128 / 0.300672);
		int32_t v4 = col[8 * 4] * (int32_t)(128 / 0.353553);
		int32_t v5 = col[8 * 5] * (int32_t)(128 / 0.449988);
		int32_t v6 = col[8 * 6] * (int32_t)(128 / 0.653281);
		int32_t v7 = col[8 * 7] * (int32_t)(128 / 1.281457);

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
	for (size_t y = 0; y < 8; ++y)
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
		out[0] = (t7  + z4 + 32) >> 7;
		out[1] = (t8  + t3 + 32) >> 7;
		out[2] = (t9  + t4 + 32) >> 7;
		out[3] = (t10 + t5 + 32) >> 7;
		out[4] = (t10 - t5 + 32) >> 7;
		out[5] = (t9  - t4 + 32) >> 7;
		out[6] = (t8  - t3 + 32) >> 7;
		out[7] = (t7  - z4 + 32) >> 7;
	}
}

void idct(int32_t *restrict dst, const int32_t *restrict src)
{
	int32_t tmp[64];
	idct1(tmp, src);
	idct2(dst, tmp);
}

void dezigzag(int32_t *restrict dst, const int32_t *restrict src)
{
	for (size_t i = 0; i < 64; ++i)
		dst[i] = src[inv_zigzag_table[i]];
}

void dequantify(int32_t *restrict dst, const int32_t *restrict src,
                int32_t quant, int is_intra)
{
	if (quant & 1)
	{
		for (size_t i = 0; i < 64; ++i)
		{
			int32_t v = src[i];
			if (v > 0)
				dst[i] = quant * (2 * v + 1);
			else if (v < 0)
				dst[i] = quant * (2 * v - 1);
			else
				dst[i] = 0;
			if (dst[i] < -2048)
				dst[i] = -2048;
			else if (dst[i] > 2047)
				dst[i] = 2047;
		}
	}
	else
	{
		for (size_t i = 0; i < 64; ++i)
		{
			int32_t v = src[i];
			if (v > 0)
				dst[i] = quant * (2 * v + 1) - 1;
			else if (v < 0)
				dst[i] = quant * (2 * v - 1) + 1;
			else
				dst[i] = 0;
			if (dst[i] < -2048)
				dst[i] = -2048;
			else if (dst[i] > 2047)
				dst[i] = 2047;
		}
	}
	if (is_intra)
	{
		if (src[0] == 255)
			dst[0] = 1024;
		else
			dst[0] = src[0] * 8;
	}
}

uint32_t decode_vlc(struct h261 *h261, const uint32_t *values,
                    const uint8_t *bits, size_t codes)
{
	uint32_t v = 0;
	size_t n = 0;
	size_t b = 0;
	while (n < codes)
	{
		int ret = bs_getbit(&h261->bs);
		if (ret < 0)
			return (uint32_t)-1;
		v <<= 1;
		v |= ret;
		b++;
		while (n < codes && b == bits[n])
		{
			if (v == values[n])
				return n;
			n++;
		}
	}
	return (uint32_t)-1;
}

uint32_t decode_mtype(struct h261 *h261)
{
	size_t n = 0;
	while (n < 10)
	{
		int ret = bs_getbit(&h261->bs);
		if (ret < 0)
			return (uint32_t)-1;
		n++;
		if (ret)
			return n;
	}
	return (uint32_t)-1;
}

int decode_block(struct h261 *h261, int32_t *data, int is_intra)
{
	size_t i = 0;
	if (is_intra)
	{
		uint32_t tmp = bs_getbits(&h261->bs, 8);
		if (tmp == (uint32_t)-1)
		{
			fprintf(stderr, "intra first EOF\n");
			return 1;
		}
		if (!(tmp & 0x7F))
		{
			fprintf(stderr, "level out of range: 0x%02" PRIx32 "\n", tmp);
			return 1;
		}
		data[i++] = tmp;
	}
	else
	{
		uint32_t tmp = bs_peekbits(&h261->bs, 2);
		if (tmp == (uint32_t)-1)
		{
			fprintf(stderr, "first EOF\n");
			return 1;
		}
		if (tmp & 0x2)
		{
			if (tmp & 1)
				data[i++] = -1;
			else
				data[i++] = 1;
			bs_getbits(&h261->bs, 2);
		}
	}
	for (; ; ++i)
	{
		uint32_t tcoeff = decode_vlc(h261, tcoeff_values, tcoeff_bits,
		                             sizeof(tcoeff_bits));
		if (tcoeff == (uint32_t)-1)
		{
			fprintf(stderr, "invalid tcoeff\n");
			return 1;
		}
		if (!tcoeff)
		{
			if (!i)
			{
				data[i] = 1;
				continue;
			}
			for (; i < 64; ++i)
				data[i] = 0;
			return 0;
		}
		if (i == 64)
		{
			fprintf(stderr, "block size exceeded\n");
			return 1;
		}
		uint8_t run = tcoeff_run[tcoeff];
		int8_t level;
		if (run == 0xFF) /* escape */
		{
			uint32_t tmp = bs_getbits(&h261->bs, 6);
			if (tmp == (uint32_t)-1)
			{
				fprintf(stderr, "invalid run\n");
				return 1;
			}
			run = tmp;
			tmp = bs_getbits(&h261->bs, 8);
			if (tmp == (uint32_t)-1)
			{
				fprintf(stderr, "invalid level\n");
				return 1;
			}
			if (!(tmp & 0x7F))
			{
				fprintf(stderr, "level out of range: 0x%02" PRIx32 "\n", tmp);
				return 1;
			}
			level = (int8_t)(uint8_t)tmp;
		}
		else
		{
			if ((tcoeff & 1) == (tcoeff > 9))
				level = -(int8_t)tcoeff_level[tcoeff];
			else
				level = tcoeff_level[tcoeff];
		}
		if (i + run > 64)
		{
			fprintf(stderr, "rle too long\n");
			return 1;
		}
		for (uint8_t j = 0; j < run; ++j)
			data[i++] = 0;
		data[i] = level;
	}
	return 0;
}

int32_t decode_mvd(struct h261 *h261)
{
	uint32_t mvd = decode_vlc(h261, mvd_values, mvd_bits, sizeof(mvd_bits));
	if (mvd == (uint32_t)-1)
	{
		fprintf(stderr, "invalid mvd\n");
		return INT32_MIN;
	}
	if (!mvd)
		return 0;
	if (mvd & 1)
		return ((mvd + 1) >> 1);
	return -(int32_t)((mvd + 1) >> 1);
}

void store_intra_luma(struct h261 *h261, int32_t *restrict dst,
                      const int32_t *restrict src)
{
	struct h261_frame *frame = h261->frame;
	uint32_t w = frame->width;
	int32_t *row_dst = dst;
	const int32_t *row_src = src;
	for (size_t y = 0; y < 8; ++y)
	{
		for (size_t x = 0; x < 8; ++x)
			row_dst[x] = row_src[x];
		row_dst += w;
		row_src += 8;
	}
}

void store_intra_chroma(struct h261 *h261, int32_t *restrict dst,
                        const int32_t *restrict src)
{
	struct h261_frame *frame = h261->frame;
	uint32_t w = frame->width;
	int32_t *row_dst = dst;
	const int32_t *row_src = src;
	for (size_t y = 0; y < 8; ++y)
	{
		for (size_t x = 0; x < 8; ++x)
		{
			row_dst[(x * 2) + 0]     = row_src[x];
			row_dst[(x * 2) + 1]     = row_src[x];
			row_dst[(x * 2) + w]     = row_src[x];
			row_dst[(x * 2) + w + 1] = row_src[x];
		}
		row_dst += w * 2;
		row_src += 8;
	}
}

void store_intra_block(struct h261 *h261, int32_t blocks[6][64],
                       uint32_t x, uint32_t y)
{
	struct h261_frame *frame = h261->frame;
	uint32_t w = frame->width;
	uint32_t off = y * w + x;
	int32_t *Y_data = &frame->Y[off];
	int32_t *Cb_data = &frame->Cb[off];
	int32_t *Cr_data = &frame->Cr[off];
	store_intra_luma(h261, &Y_data[0]          , blocks[0]);
	store_intra_luma(h261, &Y_data[8]          , blocks[1]);
	store_intra_luma(h261, &Y_data[w * 8]      , blocks[2]);
	store_intra_luma(h261, &Y_data[(w + 1) * 8], blocks[3]);
	store_intra_chroma(h261, Cb_data, blocks[4]);
	store_intra_chroma(h261, Cr_data, blocks[5]);
}

void store_inter_luma(struct h261 *h261, int32_t *restrict dst,
                      const int32_t *restrict src1,
                      const int32_t *restrict src2)
{
	struct h261_frame *frame = h261->frame;
	uint32_t w = frame->width;
	int32_t *row_dst = dst;
	const int32_t *row_src1 = src1;
	const int32_t *row_src2 = src2;
	for (size_t y = 0; y < 8; ++y)
	{
		for (size_t x = 0; x < 8; ++x)
			row_dst[x] = row_src1[x] + row_src2[x];
		row_dst += w;
		row_src1 += w;
		row_src2 += 8;
	}
}

void store_inter_chroma(struct h261 *h261, int32_t *restrict dst,
                        const int32_t *restrict src1,
                        const int32_t *restrict src2)
{
	struct h261_frame *frame = h261->frame;
	uint32_t w = frame->width;
	int32_t *row_dst = dst;
	const int32_t *row_src1 = src1;
	const int32_t *row_src2 = src2;
	for (size_t y = 0; y < 8; ++y)
	{
		for (size_t x = 0; x < 8; ++x)
		{
			row_dst[(x * 2) + 0]     = row_src1[(x * 2) + 0]     + row_src2[x];
			row_dst[(x * 2) + 1]     = row_src1[(x * 2) + 1]     + row_src2[x];
			row_dst[(x * 2) + w]     = row_src1[(x * 2) + w]     + row_src2[x];
			row_dst[(x * 2) + w + 1] = row_src1[(x * 2) + w + 1] + row_src2[x];
		}
		row_dst += w * 2;
		row_src1 += w * 2;
		row_src2 += 8;
	}
}

void store_inter_block(struct h261 *h261, int32_t blocks[6][64],
                       uint32_t x, uint32_t y)
{
	struct h261_frame *frame = h261->frame;
	struct h261_frame *prev_frame = h261->prev_frame;
	uint32_t w = frame->width;
	uint32_t off = y * w + x;
	int32_t *Y_data = &frame->Y[off];
	int32_t *Cb_data = &frame->Cb[off];
	int32_t *Cr_data = &frame->Cr[off];
	uint32_t prev_off = (y + h261->mvdy) * w + (x + h261->mvdx);
	int32_t *Y_prev = &prev_frame->Y[prev_off];
	int32_t *Cb_prev = &prev_frame->Cb[prev_off];
	int32_t *Cr_prev = &prev_frame->Cr[prev_off];
	store_inter_luma(h261, &Y_data[0]          , &Y_prev[0]          , blocks[0]);
	store_inter_luma(h261, &Y_data[8]          , &Y_prev[8]          , blocks[1]);
	store_inter_luma(h261, &Y_data[w * 8]      , &Y_prev[w * 8]      , blocks[2]);
	store_inter_luma(h261, &Y_data[(w + 1) * 8], &Y_prev[(w + 1) * 8], blocks[3]);
	store_inter_chroma(h261, Cb_data, Cb_prev, blocks[4]);
	store_inter_chroma(h261, Cr_data, Cr_prev, blocks[5]);
}

void store_copy(struct h261 *h261, int32_t *restrict dst,
                const int32_t *restrict src)
{
	struct h261_frame *frame = h261->frame;
	uint32_t w = frame->width;
	int32_t *row_dst = dst;
	const int32_t *row_src = src;
	for (size_t y = 0; y < 16; ++y)
	{
		for (size_t x = 0; x < 16; ++x)
			row_dst[x] = row_src[x];
		row_dst += w;
		row_src += w;
	}
}

void store_copy_block(struct h261 *h261, uint32_t x, uint32_t y)
{
	struct h261_frame *frame = h261->frame;
	struct h261_frame *prev_frame = h261->prev_frame;
	uint32_t w = frame->width;
	uint32_t off = y * w + x;
	int32_t *Y_data = &frame->Y[off];
	int32_t *Cb_data = &frame->Cb[off];
	int32_t *Cr_data = &frame->Cr[off];
	int32_t *Y_prev = &prev_frame->Y[off];
	int32_t *Cb_prev = &prev_frame->Cb[off];
	int32_t *Cr_prev = &prev_frame->Cr[off];
	store_copy(h261, Y_data , Y_prev);
	store_copy(h261, Cb_data, Cb_prev);
	store_copy(h261, Cr_data, Cr_prev);
}

void store_moved_block(struct h261 *h261, uint32_t x, uint32_t y)
{
	struct h261_frame *frame = h261->frame;
	struct h261_frame *prev_frame = h261->prev_frame;
	uint32_t w = frame->width;
	uint32_t off = y * w + x;
	int32_t *Y_data = &frame->Y[off];
	int32_t *Cb_data = &frame->Cb[off];
	int32_t *Cr_data = &frame->Cr[off];
	uint32_t prev_off = (y + h261->mvdy) * w + (x + h261->mvdx);
	int32_t *Y_prev = &prev_frame->Y[prev_off];
	int32_t *Cb_prev = &prev_frame->Cb[prev_off];
	int32_t *Cr_prev = &prev_frame->Cr[prev_off];
	store_copy(h261, Y_data , Y_prev);
	store_copy(h261, Cb_data, Cb_prev);
	store_copy(h261, Cr_data, Cr_prev);
}

void loop_filter(int32_t *block)
{
	int32_t tmp[64];
	for (size_t x = 0; x < 8; ++x)
	{
		tmp[0 * 8 + x] = block[0 * 8 + x] * 4;
		tmp[7 * 8 + x] = block[7 * 8 + x] * 4;
	}
	for (size_t y = 1; y < 7; ++y)
	{
		for (size_t x = 0; x < 8; ++x)
		{
			tmp[y * 8 + x] = block[y * 8 + x - 8]
			               + block[y * 8 + x + 0] * 2
			               + block[y * 8 + x + 8];
		}
	}
	for (size_t y = 0; y < 8; ++y)
	{
		block[y * 8 + 0] = (tmp[y * 8 + 0] + 2) >> 2;
		for (size_t x = 1; x < 7; ++x)
		{
			block[y * 8 + x] = (tmp[y * 8 + x - 1]
			                  + tmp[y * 8 + x + 0] * 2
			                  + tmp[y * 8 + x + 1]
			                  + 8) >> 4;
		}
		block[y * 8 + 7] = (tmp[y * 8 + 7] + 2) >> 2;
	}
}

int decode_mb(struct h261 *h261, uint32_t x, uint32_t y, uint32_t mba, uint32_t mbad)
{
	uint32_t mtype = decode_mtype(h261);
	if (mtype == (uint32_t)-1)
	{
		fprintf(stderr, "invalid mtype\n");
		return 1;
	}
	uint8_t mtype_f = mtype_flags[mtype - 1];
	if (mtype_f & MTYPE_MQUANT)
	{
		uint32_t mquant = bs_getbits(&h261->bs, 5);
		if (mquant == (uint32_t)-1)
		{
			fprintf(stderr, "invalid mquant\n");
			return 1;
		}
		h261->quant = mquant;
	}
	if (mtype_f & MTYPE_MVD)
	{
		int32_t mvdx = decode_mvd(h261);
		if (mvdx == INT32_MIN)
			return 1;
		int32_t mvdy = decode_mvd(h261);
		if (mvdy == INT32_MIN)
			return 1;
		if (mba != 0 && mba != 11 && mba != 22 && mbad == 1)
		{
			h261->mvdx += mvdx;
			h261->mvdy += mvdy;
			if (h261->mvdx < -15)
				h261->mvdx += 32;
			else if (h261->mvdx > 15)
				h261->mvdx -= 32;
			if (h261->mvdy < -15)
				h261->mvdy += 32;
			else if (h261->mvdy > 15)
				h261->mvdy -= 32;
		}
		else
		{
			h261->mvdx = mvdx;
			h261->mvdy = mvdy;
		}
	}
	else
	{
		h261->mvdx = 0;
		h261->mvdy = 0;
	}
	uint8_t cbp;
	if (mtype_f & MTYPE_CBP)
	{
		uint32_t cbp_v = decode_vlc(h261, cbp_values, cbp_bits,
		                            sizeof(cbp_bits));
		if (cbp_v == (uint32_t)-1)
		{
			fprintf(stderr, "invalid CBP\n");
			return 1;
		}
		cbp = cbp_val[cbp_v];
	}
	else
	{
		cbp = 0x3F;
	}
	if (mtype_f & MTYPE_TCOEFF)
	{
		int32_t blocks[6][64];
		int is_intra = (mtype_f & MTYPE_INTRA);
		for (size_t i = 0; i < 6; ++i)
		{
			int32_t tmp1[64];
			int32_t tmp2[64];
			if (!(cbp & (0x20 >> i)))
			{
				memset(blocks[i], 0, sizeof(blocks[i]));
				continue;
			}
			if (decode_block(h261, tmp1, is_intra))
				return 1;
			dequantify(tmp2, tmp1, h261->quant, is_intra);
			dezigzag(tmp1, tmp2);
			idct(blocks[i], tmp1);
			if (mtype_f & MTYPE_FIL)
				loop_filter(blocks[i]);
		}
		if (is_intra)
			store_intra_block(h261, blocks, x, y);
		else
			store_inter_block(h261, blocks, x, y);
	}
	else
	{
		store_moved_block(h261, x, y);
	}
	return 0;
}

static void get_mba_coords(uint32_t mba, uint32_t *x, uint32_t *y)
{
	if (mba >= 22)
	{
		*y = 16 * 2;
		*x = 16 * (mba - 22);
	}
	else if (mba >= 11)
	{
		*y = 16 * 1;
		*x = 16 * (mba - 11);
	}
	else
	{
		*y = 16 * 0;
		*x = 16 * (mba - 0);
	}
}

int decode_gob(struct h261 *h261)
{
	uint32_t gbsc = bs_getbits(&h261->bs, 16);
	if (gbsc == (uint32_t)-1)
	{
		fprintf(stderr, "GBSC EOF\n");
		return 1;
	}
	if (gbsc != 1)
	{
		fprintf(stderr, "invalid gbsc: 0x%" PRIx32 "\n", gbsc);
		return 1;
	}
	uint32_t gn = bs_getbits(&h261->bs, 4);
	if (gn == (uint32_t)-1)
	{
		fprintf(stderr, "GN EOF\n");
		return 1;
	}
	if (!gn
	 || (!h261->frame->cif && (gn > 5 || !(gn & 1)))
	 || (h261->frame->cif && gn > 12))
	{
		fprintf(stderr, "invalid gn\n");
		return 1;
	}
	gn--;
	uint32_t gquant = bs_getbits(&h261->bs, 5);
	if (gquant == (uint32_t)-1)
	{
		fprintf(stderr, "GQUANT EOF\n");
		return 1;
	}
	h261->quant = gquant;
	uint32_t gei = bs_getbit(&h261->bs);
	if (gei == (uint32_t)-1)
	{
		fprintf(stderr, "GEI EOF\n");
		return 1;
	}
	while (gei)
	{
		uint32_t gspare = bs_getbits(&h261->bs, 8);
		if (gspare == (uint32_t)-1)
		{
			fprintf(stderr, "GSPARE EOF\n");
			return 1;
		}
		gei = bs_getbit(&h261->bs);
		if (gei == (uint32_t)-1)
		{
			fprintf(stderr, "GEI EOF\n");
			return 1;
		}
	}
	uint32_t prev_mba = UINT32_MAX;
	uint32_t x = 0;
	uint32_t y = 0;
	if (gn & 1)
		x += 11 * 16;
	y += 3 * 16 * (gn >> 1);
	uint32_t mb_x;
	uint32_t mb_y;
	while (1)
	{
		uint32_t mba;
		do
		{
			/* start code */
			mba = bs_peekbits(&h261->bs, 16);
			if (mba == 1)
				goto end;
			if (mba == 0)
			{
				while (h261->bs.len & 7)
					bs_getbit(&h261->bs);
				goto end;
			}
			mba = decode_vlc(h261, mba_values, mba_bits, sizeof(mba_bits));
			if (mba == (uint32_t)-1)
			{
				fprintf(stderr, "invalid mba\n");
				return 1;
			}
		} while (mba == 0x21);
		if (prev_mba != UINT32_MAX)
			mba += prev_mba + 1;
		uint32_t mbad = mba - prev_mba;
		while (++prev_mba != mba)
		{
			get_mba_coords(prev_mba, &mb_x, &mb_y);
			store_copy_block(h261, x + mb_x, y + mb_y);
		}
		get_mba_coords(mba, &mb_x, &mb_y);
		if (decode_mb(h261, x + mb_x, y + mb_y, mba, mbad))
			return 1;
	}
end:
	while (++prev_mba <= 32)
	{
		get_mba_coords(prev_mba, &mb_x, &mb_y);
		store_copy_block(h261, x + mb_x, y + mb_y);
	}
	return 0;
}

static void merge_rgb_pixel(uint8_t *restrict row,
                            const int32_t *restrict Y_row,
                            const int32_t *restrict Cb_row,
                            const int32_t *restrict Cr_row,
                            size_t x)
{
	int32_t Y = Y_row[x];
	int32_t Cr = Cr_row[x] - 128;
	int32_t Cb = Cb_row[x] - 128;
	int32_t r = Y * 65536;
	int32_t g = Y * 65536;
	int32_t b = Y * 65536;
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
	row[x * 3 + 0] = r;
	row[x * 3 + 1] = g;
	row[x * 3 + 2] = b;
}

void merge_rgb(struct h261 *h261, uint8_t *dst)
{
	struct h261_frame *frame = h261->frame;
	uint32_t w = frame->width;
	uint8_t *row = dst;
	const int32_t *Y_row = frame->Y;
	const int32_t *Cb_row = frame->Cb;
	const int32_t *Cr_row = frame->Cr;
	for (size_t y = 0; y < frame->height; ++y)
	{
		for (size_t x = 0; x < w; ++x)
			merge_rgb_pixel(row, Y_row, Cb_row, Cr_row, x);
		Cb_row += w;
		Cr_row += w;
		Y_row += w;
		row += frame->width * 3;
	}
}

int decode_frame(struct h261 *h261, void *data, uint32_t *width,
                 uint32_t *height)
{
	h261->frame = &h261->frames[h261->frame_ff];
	h261->prev_frame = &h261->frames[!h261->frame_ff];
	uint32_t psc = bs_getbits(&h261->bs, 20);
	if (psc == (uint32_t)-1)
	{
		fprintf(stderr, "PSC EOF\n");
		return 1;
	}
	if (psc != 0x10)
	{
		fprintf(stderr, "invalid psc: 0x%" PRIx32 "\n", psc);
		return 1;
	}
	uint32_t tr = bs_getbits(&h261->bs, 5);
	if (tr == (uint32_t)-1)
	{
		fprintf(stderr, "TR EOF\n");
		return 1;
	}
	/* XXX assert(tr == prev_tr + 1); */
	uint32_t ptype = bs_getbits(&h261->bs, 6);
	if (ptype == (uint32_t)-1)
	{
		fprintf(stderr, "PTYPE EOF\n");
		return 1;
	}
	if (ptype & (1 << 2))
	{
		h261->frame->cif = 1;
		h261->frame->width = 352;
		h261->frame->height = 288;
	}
	else
	{
		h261->frame->cif = 0;
		h261->frame->width = 176;
		h261->frame->height = 144;
	}
	uint32_t pei = bs_getbit(&h261->bs);
	if (pei == (uint32_t)-1)
	{
		fprintf(stderr, "PEI EOF\n");
		return 1;
	}
	while (pei)
	{
		uint32_t pspare = bs_getbits(&h261->bs, 8);
		if (pspare == (uint32_t)-1)
		{
			fprintf(stderr, "PSPARE EOF\n");
			return 1;
		}
		pei = bs_getbit(&h261->bs);
		if (pei == (uint32_t)-1)
		{
			fprintf(stderr, "PEI EOF\n");
			return 1;
		}
	}
	if (h261->frame->cif)
	{
		for (size_t i = 0; i < 12; ++i)
		{
			if (decode_gob(h261))
				return 1;
		}
	}
	else
	{
		for (size_t i = 0; i < 3; ++i)
		{
			if (decode_gob(h261))
				return 1;
		}
	}
	merge_rgb(h261, data);
	*width = h261->frame->width;
	*height = h261->frame->height;
	h261->frame_ff = !h261->frame_ff;
	return 0;
}

struct h261 *h261_new(void)
{
	struct h261 *h261 = calloc(1, sizeof(*h261));
	if (!h261)
	{
		fprintf(stderr, "malloc failed\n");
		return NULL;
	}
	return h261;
}

static int bs_get(struct bitstream *bs)
{
	struct h261 *h261 = bs->userdata;
	uint8_t avail = (sizeof(bs->buf) * 8 - bs->len) / 8;
	uint8_t buf[sizeof(bs->buf)];
	size_t rd = fread(buf, 1, avail, h261->fp);
	if (ferror(h261->fp))
	{
		fprintf(stderr, "failed to read from file\n");
		return 1;
	}
	for (size_t i = 0; i < rd; ++i)
	{
		bs->buf <<= 8;
		bs->buf |= buf[i];
	}
	bs->len += rd * 8;
	return 0;
}

static int bs_put(struct bitstream *bs)
{
	struct h261 *h261 = bs->userdata;
	uint8_t avail = (sizeof(bs->buf) * 8 - bs->len) / 8;
	uint8_t buf[sizeof(bs->buf)];
	for (size_t i = 0; i < avail; ++i)
		buf[i] = bs->buf >> ((sizeof(bs->buf) - 1 - i) * 8);
	size_t wr = fwrite(buf, 1, avail, h261->fp);
	if (ferror(h261->fp))
	{
		fprintf(stderr, "failed to read from file\n");
		return 1;
	}
	bs->buf <<= wr * 8;
	bs->len += wr * 8;
	return 0;
}

void h261_init_io(struct h261 *h261, FILE *fp)
{
	h261->fp = fp;
	bs_init_read(&h261->bs);
	h261->bs.userdata = h261;
	h261->bs.get = bs_get;
	h261->bs.put = bs_put;
}

static void png_write(const char *file, const void *data, uint32_t width, uint32_t height)
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
	png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_write_info(png_ptr, info_ptr);
	if (setjmp(png_jmpbuf(png_ptr)))
		goto error3;
	for (uint32_t i = 0; i < height; ++i)
		row_pointers[i] = (uint8_t*)data + i * width * 3;
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

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		fprintf(stderr, "%s file.h261\n", argv[0]);
		return EXIT_FAILURE;
	}
	setvbuf(stdout, NULL, _IOLBF, 0);
	FILE *fp = fopen(argv[1], "rb");
	if (!fp)
	{
		fprintf(stderr, "%s: fopen(%s): %s\n", argv[0], argv[1], strerror(errno));
		return EXIT_FAILURE;
	}
	struct h261 *h261 = h261_new();
	if (!h261)
		return EXIT_FAILURE;
	h261_init_io(h261, fp);
#if 1
	glfwInit();
	GLFWwindow *window = glfwCreateWindow(352, 288, "h261", NULL, NULL);
	if (!window)
		return (0);
	glfwMakeContextCurrent(window);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glViewport(0, 0, 352, 288);
	glMatrixMode(GL_MODELVIEW);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 352, 288, 0, 0, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0, 0, 0, 1);
	glColor3f(1, 1, 1);
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	int i = 0;
#endif
	uint8_t data[352 * 288 * 3];
	while (1)
	{
#if 0
		uint64_t s = nanotime();
#endif
		uint32_t width;
		uint32_t height;
		if (decode_frame(h261, data, &width, &height))
			return EXIT_FAILURE;
#if 0
		uint64_t e = nanotime();
		printf("frame: %lu us\n", (e - s) / 1000);
#endif
#if 0
		char path[256];
		snprintf(path, sizeof(path), "test_%03d.png", i);
		png_write(path, data, width, height);
#endif
#if 1
		i++;
		{
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			float coords[4 * 2] = {0, 0, 1, 0, 1, 1, 0, 1};
			float vertex[4 * 2] = {0, 0, 352, 0, 352, 288, 0, 288};
			glVertexPointer(2, GL_FLOAT, 0, vertex);
			glTexCoordPointer(2, GL_FLOAT, 0, coords);
			glDrawArrays(GL_QUADS, 0, 4);
		}
		glfwSwapBuffers(window);
		glfwPollEvents();
		struct timespec ts;
		ts.tv_sec = 0;
		ts.tv_nsec = 30000000;
		nanosleep(&ts, NULL);
#endif
	}
	return EXIT_SUCCESS;
}