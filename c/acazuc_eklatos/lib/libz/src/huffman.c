#include "bitstream.h"
#include "huffman.h"

#include <zlib.h>

void huffman_generate(struct huffman *huff, uint8_t *sizes, uint32_t count)
{
	huff->count = count;
	for (size_t i = 0; i <= MAX_BITS; ++i)
		huff->counts[i] = 0;
	for (size_t i = 0; i < huff->count; ++i)
		huff->counts[sizes[i]]++;
	huff->counts[0] = 0;
	uint32_t code = 0;
	for (size_t i = 0; i <= MAX_BITS; ++i)
	{
		huff->codes[i] = code;
		code = (code + huff->counts[i]) << 1;
	}
	uint16_t offsets[MAX_BITS + 1];
	uint32_t offset = 0;
	for (size_t i = 0; i <= MAX_BITS; ++i)
	{
		offsets[i] = offset;
		offset += huff->counts[i];
	}
	for (size_t i = 0; i < huff->count; ++i)
	{
		if (sizes[i])
			huff->values[offsets[sizes[i]]++] = i;
	}
}

int huffman_decode(struct bitstream *bs, struct huffman *huff)
{
	uint16_t value = 0;
	uint32_t pos = 0;
	for (size_t i = 1; i <= MAX_BITS; ++i)
	{
		if (!bitstream_has_read(bs, i))
			return Z_NEED_MORE;
		value = (value << 1) | bitstream_getbit(bs, i - 1);
		uint32_t counts = huff->counts[i];
		uint32_t base = huff->codes[i];
		if (value >= base && value < base + counts)
		{
			bitstream_skip(bs, i);
			return huff->values[pos + (value - base)];
		}
		pos += counts;
	}
	return Z_DATA_ERROR;
}
