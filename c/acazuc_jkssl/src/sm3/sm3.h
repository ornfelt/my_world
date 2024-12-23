#ifndef SM3_SM3_H
#define SM3_SM3_H

#include <stdint.h>
#include <stddef.h>

struct sm3_ctx
{
	uint32_t h[8];
	uint8_t data[64];
	uint32_t data_size;
	uint64_t total_size;
};

int sm3_init(struct sm3_ctx *ctx);
int sm3_update(struct sm3_ctx *ctx, const void *data, size_t size);
int sm3_final(uint8_t *md, struct sm3_ctx *ctx);
int sm3(const uint8_t *data, size_t size, uint8_t *md);

#endif
