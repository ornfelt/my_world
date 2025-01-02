#ifndef JPEG_H
#define JPEG_H

#include "bitstream.h"

#include <libjpeg/jpeg.h>

#include <stdint.h>
#include <stdio.h>

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

#define JPEG_RSHIFT(v, n) (((v) + (1 << ((n) - 1))) >> (n))

struct huffman
{
	uint16_t offsets[MAX_BITS];
	uint16_t counts[MAX_BITS];
	uint16_t codes[MAX_BITS];
	uint16_t maxcodes[MAX_BITS];
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
		uint8_t scale_x;
		uint8_t scale_y;
		uint8_t table;
		uint8_t dct;
		uint8_t act;
		uint8_t dc_huff;
		uint8_t ac_huff;
		int32_t prev_dc;
		uint16_t width;
		uint16_t height;
		int32_t *data;
	} components[3];
	struct
	{
		struct huffman huffman;
		uint8_t class : 4;
		uint8_t dst : 4;
	} huff_tables[4];
	int32_t dqt[2][64];
	int32_t idqt[2][64];
	struct bitstream bs;
	uint8_t huff_count;
	uint8_t eof;
	uint8_t thumbnail_width;
	uint8_t thumbnail_height;
	uint8_t restart_id;
	uint32_t component_width;
	uint32_t component_height;
	uint32_t block_width;
	uint32_t block_height;
	uint32_t block_x;
	uint32_t block_y;
	uint8_t *thumbnail;
	FILE *fp;
	char errbuf[128];
};

void huffman_generate(struct huffman *huff, const uint8_t *counts, const uint8_t *values);
int jpeg_getc(struct jpeg *jpeg);
int jpeg_putc(struct jpeg *jpeg, uint8_t v);

#define JPEG_ERR(jpeg, fmt, ...) snprintf((jpeg)->errbuf, sizeof((jpeg)->errbuf), fmt, ##__VA_ARGS__)

#endif
