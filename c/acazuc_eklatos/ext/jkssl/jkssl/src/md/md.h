#ifndef MD_MD_H
#define MD_MD_H

#include <stdint.h>
#include <stddef.h>

struct md2_ctx
{
	uint8_t h[16];
	uint8_t data[16];
	uint8_t checksum[16];
	uint32_t data_size;
	uint32_t total_size;
};

struct md4_ctx
{
	uint32_t h[4];
	uint8_t data[64];
	uint32_t data_size;
	uint64_t total_size;
};

struct md5_ctx
{
	uint32_t h[4];
	uint8_t data[64];
	uint32_t data_size;
	uint64_t total_size;
};

int md2_init(struct md2_ctx *ctx);
int md2_update(struct md2_ctx *ctx, const void *data, size_t size);
int md2_final(uint8_t *md, struct md2_ctx *ctx);
int md2(const uint8_t *data, size_t size, uint8_t *md);

int md4_init(struct md4_ctx *ctx);
int md4_update(struct md4_ctx *ctx, const void *data, size_t size);
int md4_final(uint8_t *md, struct md4_ctx *ctx);
int md4(const uint8_t *data, size_t size, uint8_t *md);

int md5_init(struct md5_ctx *ctx);
int md5_update(struct md5_ctx *ctx, const void *data, size_t size);
int md5_final(uint8_t *md, struct md5_ctx *ctx);
int md5(const uint8_t *data, size_t size, uint8_t *md);

#endif
