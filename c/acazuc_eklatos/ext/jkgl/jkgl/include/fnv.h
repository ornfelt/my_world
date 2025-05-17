#ifndef JKGL_FNV_H
#define JKGL_FNV_H

#include <stdint.h>
#include <stddef.h>

#define FNV_PRIME32 0x01000193U
#define FNV_BASIS32 0x811C9DC5U

static inline uint32_t fnv32(uint32_t h, uint8_t v)
{
	return (h ^ v) * FNV_PRIME32;
}

static inline uint32_t fnv32v(uint32_t h, const void *d, size_t n)
{
	for (size_t i = 0; i < n; ++i)
		h = fnv32(h, ((const uint8_t*)d)[i]);
	return h;
}

#endif
