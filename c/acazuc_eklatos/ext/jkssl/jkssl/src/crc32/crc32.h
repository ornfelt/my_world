#ifndef CRC32_CRC32_H
#define CRC32_CRC32_H

#include <stdint.h>
#include <stddef.h>

struct crc32_ctx
{
	uint32_t value;
};

int crc32_init(struct crc32_ctx *ctx);
int crc32_update(struct crc32_ctx *ctx, const void *data, size_t len);
int crc32_final(uint8_t *md, struct crc32_ctx *ctx);
int crc32(const uint8_t *data, size_t len, uint8_t *md);

#endif
