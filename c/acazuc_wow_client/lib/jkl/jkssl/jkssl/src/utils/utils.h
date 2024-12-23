#ifndef JKSSL_UTILS_H
#define JKSSL_UTILS_H

#include <stddef.h>
#include <stdint.h>

#define JKSSL_MIN(a, b) ((a) < (b) ? (a) : (b))
#define JKSSL_MAX(a, b) ((a) > (b) ? (a) : (b))

static inline uint16_t rol16(uint16_t v, uint16_t c)
{
	return (v << c) | (v >> (16 - c));
}

static inline uint32_t rol28(uint32_t v, uint32_t c)
{
	return ((v << c) | (v >> (28 - c))) & 0xFFFFFFF;
}

static inline uint32_t rol32(uint32_t v, uint32_t c)
{
	return (v << c) | (v >> (32 - c));
}

static inline uint64_t rol64(uint64_t v, uint64_t c)
{
	return (v << c) | (v >> (64 - c));
}

static inline uint16_t ror16(uint16_t v, uint16_t c)
{
	return (v >> c) | (v << (16 - c));
}

static inline uint32_t ror28(uint32_t v, uint32_t c)
{
	return ((v >> c) | (v << (28 - c))) & 0xFFFFFFF;
}

static inline uint32_t ror32(uint32_t v, uint32_t c)
{
	return (v >> c) | (v << (32 - c));
}

static inline uint64_t ror64(uint64_t v, uint64_t c)
{
	return (v >> c) | (v << (64 - c));
}

static inline uint64_t bswap64(uint64_t v)
{
	return ((v >> 56) & 0x000000FF)
	     | ((v >> 40) & 0x0000FF00)
	     | ((v >> 24) & 0x00FF0000)
	     | ((v >>  8) & 0xFF000000)
	     | ((v & 0xFF000000) <<  8)
	     | ((v & 0x00FF0000) << 24)
	     | ((v & 0x0000FF00) << 40)
	     | ((v & 0x000000FF) << 56);
}

static inline uint32_t bswap32(uint32_t v)
{
	return ((v >> 24) & 0x00FF)
	     | ((v >>  8) & 0xFF00)
	     | ((v & 0xFF00) <<  8)
	     | ((v & 0x00FF) << 24);
}

static inline uint16_t bswap16(uint16_t v)
{
	return ((v >>  8) & 0xFF)
	     | ((v & 0xFF) <<  8);
}

static inline uint64_t be64dec(const uint8_t *s)
{
	return ((uint64_t)s[0x0] << 56)
	     | ((uint64_t)s[0x1] << 48)
	     | ((uint64_t)s[0x2] << 40)
	     | ((uint64_t)s[0x3] << 32)
	     | ((uint64_t)s[0x4] << 24)
	     | ((uint64_t)s[0x5] << 16)
	     | ((uint64_t)s[0x6] <<  8)
	     | ((uint64_t)s[0x7] <<  0);
}

static inline uint64_t le64dec(const uint8_t *s)
{
	return ((uint64_t)s[0x0] <<  0)
	     | ((uint64_t)s[0x1] <<  8)
	     | ((uint64_t)s[0x2] << 16)
	     | ((uint64_t)s[0x3] << 24)
	     | ((uint64_t)s[0x4] << 32)
	     | ((uint64_t)s[0x5] << 40)
	     | ((uint64_t)s[0x6] << 48)
	     | ((uint64_t)s[0x7] << 56);
}

static inline void be64enc(uint8_t *d, uint64_t s)
{
	d[0x0] = s >> 56;
	d[0x1] = s >> 48;
	d[0x2] = s >> 40;
	d[0x3] = s >> 32;
	d[0x4] = s >> 24;
	d[0x5] = s >> 16;
	d[0x6] = s >> 8;
	d[0x7] = s >> 0;
}

static inline void le64enc(uint8_t *d, uint64_t s)
{
	d[0x0] = s >>  0;
	d[0x1] = s >>  8;
	d[0x2] = s >> 16;
	d[0x3] = s >> 24;
	d[0x4] = s >> 32;
	d[0x5] = s >> 40;
	d[0x6] = s >> 48;
	d[0x7] = s >> 56;
}

static inline uint32_t be32dec(const uint8_t *s)
{
	return ((uint32_t)s[0x0] << 24)
	     | ((uint32_t)s[0x1] << 16)
	     | ((uint32_t)s[0x2] <<  8)
	     | ((uint32_t)s[0x3] <<  0);
}

static inline uint32_t le32dec(const uint8_t *s)
{
	return ((uint32_t)s[0x0] <<  0)
	     | ((uint32_t)s[0x1] <<  8)
	     | ((uint32_t)s[0x2] << 16)
	     | ((uint32_t)s[0x3] << 24);
}

static inline void be32enc(uint8_t *d, uint32_t s)
{
	d[0x0] = s >> 24;
	d[0x1] = s >> 16;
	d[0x2] = s >> 8;
	d[0x3] = s >> 0;
}

static inline void le32enc(uint8_t *d, uint32_t s)
{
	d[0x0] = s >>  0;
	d[0x1] = s >>  8;
	d[0x2] = s >> 16;
	d[0x3] = s >> 24;
}

static inline uint16_t be16dec(const uint8_t *s)
{
	return ((uint16_t)s[0x0] << 8)
	     | ((uint16_t)s[0x1] << 0);
}

static inline uint16_t le16dec(const uint8_t *s)
{
	return ((uint16_t)s[0x0] << 0)
	     | ((uint16_t)s[0x1] << 8);
}

static inline void be16enc(uint8_t *d, uint16_t s)
{
	d[0x0] = s >> 8;
	d[0x1] = s >> 0;
}

static inline void le16enc(uint8_t *d, uint16_t s)
{
	d[0x0] = s >> 0;
	d[0x1] = s >> 8;
}

static inline void swap32(uint32_t *a, uint32_t *b)
{
	uint32_t t = *a;
	*a = *b;
	*b = t;
}

static inline void memxor(void *d, const void *a, const void *b, size_t n)
{
	uint8_t *d8 = d;
	const uint8_t *a8 = a;
	const uint8_t *b8 = b;
	size_t i = 0;
	while (n - i >= sizeof(size_t))
	{
		*(size_t*)&d8[i] = *(size_t*)&a8[i] ^ *(size_t*)&b8[i];
		i += sizeof(size_t);
	}
	while (i < n)
	{
		d8[i] = a8[i] ^ b8[i];
		i++;
	}
}

void bin2hex(char *dst, const uint8_t *src, size_t len);
int hex2bin(uint8_t *dst, const char *src, size_t len);
char *ask_password(void);
char *ask_password_confirm(void);

#define HASH_BUFFERIZE(ctx, data, size, block_size) \
do \
{ \
	if (ctx->data_size) \
	{ \
		if (ctx->data_size + size < block_size) \
		{ \
			memcpy(&ctx->data[ctx->data_size], data, size); \
			ctx->data_size += size; \
			ctx->total_size += size; \
			data = (uint8_t*)data + size; \
			size = 0; \
			break; \
		} \
		memcpy(&ctx->data[ctx->data_size], data, block_size - ctx->data_size); \
		chunk(ctx, ctx->data); \
		ctx->total_size += block_size - ctx->data_size; \
		data = (uint8_t*)data + block_size - ctx->data_size; \
		size -= block_size - ctx->data_size; \
		ctx->data_size = 0; \
	} \
	while (size >= block_size) \
	{ \
		chunk(ctx, data); \
		ctx->total_size += block_size; \
		data = (uint8_t*)data + block_size; \
		size -= block_size; \
	} \
	if (size) \
	{ \
		memcpy(ctx->data, data, size); \
		ctx->total_size += size; \
		ctx->data_size += size; \
		data = (uint8_t*)data + size; \
		size = 0; \
	} \
} while (0)

#define MERKLE_DAMGARD_FINALIZE(ctx, block_size, big_endian) \
do \
{ \
	ctx->data[ctx->data_size] = 0x80; \
	ctx->data_size++; \
	if (ctx->data_size > block_size - 8) \
	{ \
		while (ctx->data_size < block_size) \
			ctx->data[ctx->data_size++] = 0; \
		chunk(ctx, ctx->data); \
		ctx->data_size = 0; \
	} \
	while (ctx->data_size < block_size - 8) \
		ctx->data[ctx->data_size++] = 0; \
	if (big_endian) \
		be64enc(&ctx->data[block_size - 8], ctx->total_size * 8); \
	else \
		le64enc(&ctx->data[block_size - 8], ctx->total_size * 8); \
	chunk(ctx, ctx->data); \
} while (0)

#endif
