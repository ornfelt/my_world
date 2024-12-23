#ifndef ADLER32_ADLER32_H
#define ADLER32_ADLER32_H

#include <stdint.h>
#include <stddef.h>

struct adler32_ctx
{
	uint16_t v1;
	uint16_t v2;
};

int adler32_init(struct adler32_ctx *ctx);
int adler32_update(struct adler32_ctx *ctx, const void *data, size_t len);
int adler32_final(uint8_t *md, struct adler32_ctx *ctx);
int adler32(const uint8_t *data, size_t len, uint8_t *md);

#endif
