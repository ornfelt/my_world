#ifndef JKSSL_CRC32_H
#define JKSSL_CRC32_H

#include <stdint.h>
#include <stddef.h>

#define CRC32_DIGEST_LENGTH 4

struct evp_md;

struct crc32_ctx
{
	uint32_t value;
};

int crc32_init(struct crc32_ctx *ctx);
int crc32_update(struct crc32_ctx *ctx, const void *data, size_t len);
int crc32_final(uint8_t *md, struct crc32_ctx *ctx);
int crc32(const uint8_t *data, size_t len, uint8_t *md);

#endif
