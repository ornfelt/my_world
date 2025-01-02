#ifndef SYS_ENDIAN_H
#define SYS_ENDIAN_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline uint16_t be16dec(const void *ptr)
{
	const uint8_t *b = ptr;
	return ((uint16_t)b[0] << 8)
	     | ((uint16_t)b[1] << 0);
}

static inline uint32_t be32dec(const void *ptr)
{
	const uint8_t *b = ptr;
	return ((uint32_t)b[0] << 24)
	     | ((uint32_t)b[1] << 16)
	     | ((uint32_t)b[2] <<  8)
	     | ((uint32_t)b[3] <<  0);
}

static inline uint64_t be64dec(const void *ptr)
{
	const uint8_t *b = ptr;
	return ((uint64_t)b[0] << 56)
	     | ((uint64_t)b[1] << 48)
	     | ((uint64_t)b[2] << 40)
	     | ((uint64_t)b[3] << 32)
	     | ((uint64_t)b[4] << 24)
	     | ((uint64_t)b[5] << 16)
	     | ((uint64_t)b[6] <<  8)
	     | ((uint64_t)b[7] <<  0);
}

static inline uint16_t le16dec(const void *ptr)
{
	const uint8_t *b = ptr;
	return ((uint16_t)b[0] << 0)
	     | ((uint16_t)b[1] << 8);
}

static inline uint32_t le32dec(const void *ptr)
{
	const uint8_t *b = ptr;
	return ((uint32_t)b[0] <<  0)
	     | ((uint32_t)b[1] <<  8)
	     | ((uint32_t)b[2] << 16)
	     | ((uint32_t)b[3] << 24);
}

static inline uint64_t le64dec(const void *ptr)
{
	const uint8_t *b = ptr;
	return ((uint64_t)b[0] <<  0)
	     | ((uint64_t)b[1] <<  8)
	     | ((uint64_t)b[2] << 16)
	     | ((uint64_t)b[3] << 24)
	     | ((uint64_t)b[4] << 32)
	     | ((uint64_t)b[5] << 40)
	     | ((uint64_t)b[6] << 48)
	     | ((uint64_t)b[7] << 56);
}

static inline void be16enc(void *ptr, uint16_t v)
{
	uint8_t *b = ptr;
	b[0] = v >> 8;
	b[1] = v >> 0;
}

static inline void be32enc(void *ptr, uint32_t v)
{
	uint8_t *b = ptr;
	b[0] = v >> 24;
	b[1] = v >> 16;
	b[2] = v >>  8;
	b[3] = v >>  0;
}

static inline void be64enc(void *ptr, uint64_t v)
{
	uint8_t *b = ptr;
	b[0] = v >> 56;
	b[1] = v >> 48;
	b[2] = v >> 40;
	b[3] = v >> 32;
	b[4] = v >> 24;
	b[5] = v >> 16;
	b[6] = v >>  8;
	b[7] = v >>  0;
}

static inline void le16enc(void *ptr, uint16_t v)
{
	uint8_t *b = ptr;
	b[0] = v >> 0;
	b[1] = v >> 8;
}

static inline void le32enc(void *ptr, uint32_t v)
{
	uint8_t *b = ptr;
	b[0] = v >>  0;
	b[1] = v >>  8;
	b[2] = v >> 16;
	b[3] = v >> 24;
}

static inline void le64enc(void *ptr, uint64_t v)
{
	uint8_t *b = ptr;
	b[0] = v >>  0;
	b[1] = v >>  8;
	b[2] = v >> 16;
	b[3] = v >> 24;
	b[4] = v >> 32;
	b[5] = v >> 40;
	b[6] = v >> 48;
	b[7] = v >> 56;
}

static inline uint16_t swap16(uint16_t v)
{
	return ((v & 0xFF00) >> 8)
	     | ((v & 0x00FF) << 8);
}

static inline uint32_t swap32(uint32_t v)
{
	return ((v & 0xFF000000) >> 24)
	     | ((v & 0x00FF0000) >>  8)
	     | ((v & 0x0000FF00) <<  8)
	     | ((v & 0x000000FF) << 24);
}

static inline uint64_t swap64(uint64_t v)
{
	return ((v & 0xFF00000000000000ULL) >> 56)
	     | ((v & 0x00FF000000000000ULL) >> 40)
	     | ((v & 0x0000FF0000000000ULL) >> 24)
	     | ((v & 0x000000FF00000000ULL) >>  8)
	     | ((v & 0x00000000FF000000ULL) <<  8)
	     | ((v & 0x0000000000FF0000ULL) << 24)
	     | ((v & 0x000000000000FF00ULL) << 40)
	     | ((v & 0x00000000000000FFULL) << 56);
}

static inline uint16_t htobe16(uint16_t v)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	return swap16(v);
#else
	return v;
#endif
}

static inline uint32_t htobe32(uint32_t v)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	return swap32(v);
#else
	return v;
#endif
}

static inline uint64_t htobe64(uint64_t v)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	return swap64(v);
#else
	return v;
#endif
}

static inline uint16_t be16toh(uint16_t v)
{
	return htobe16(v);
}

static inline uint32_t be32toh(uint32_t v)
{
	return htobe32(v);
}

static inline uint64_t be64toh(uint64_t v)
{
	return htobe64(v);
}

static inline uint16_t htole16(uint16_t v)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	return v;
#else
	return swap16(v);
#endif
}

static inline uint32_t htole32(uint32_t v)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	return v;
#else
	return swap32(v);
#endif
}

static inline uint64_t htole64(uint64_t v)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	return v;
#else
	return swap64(v);
#endif
}

static inline uint16_t le16toh(uint16_t v)
{
	return htole16(v);
}

static inline uint32_t le32toh(uint32_t v)
{
	return htole32(v);
}

static inline uint64_t le64toh(uint64_t v)
{
	return htole64(v);
}

#ifdef __cplusplus
}
#endif

#endif
