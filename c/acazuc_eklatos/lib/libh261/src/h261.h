#ifndef H261_H
#define H261_H

#include "bitstream.h"

#include <libh261/h261.h>

#include <stdint.h>
#include <stdio.h>

#define MTYPE_INTRA  (1 << 0)
#define MTYPE_MQUANT (1 << 1)
#define MTYPE_MVD    (1 << 2)
#define MTYPE_CBP    (1 << 3)
#define MTYPE_TCOEFF (1 << 4)
#define MTYPE_FIL    (1 << 5)

#define H261_RSHIFT(v, n) (((v) + (1 << ((n) - 1))) >> (n))

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
	char errbuf[128];
};

#define H261_ERR(h261, fmt, ...) snprintf((h261)->errbuf, sizeof((h261)->errbuf), fmt, ##__VA_ARGS__)

#endif
