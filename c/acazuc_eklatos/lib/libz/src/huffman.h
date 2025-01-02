#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_BITS 15

struct bitstream;

struct huffman
{
	uint16_t counts[MAX_BITS + 1];
	uint16_t codes[MAX_BITS + 1];
	uint16_t values[288];
	size_t count;
};

void huffman_generate(struct huffman *huff, uint8_t *sizes, uint32_t count);
int huffman_decode(struct bitstream *bs, struct huffman *huff);

#ifdef __cplusplus
}
#endif

#endif
