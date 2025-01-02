#ifndef RIPEMD_RIPEMD_H
#define RIPEMD_RIPEMD_H

#include <stdint.h>
#include <stddef.h>

struct ripemd128_ctx
{
	uint32_t h[4];
	uint8_t data[64];
	uint32_t data_size;
	uint64_t total_size;
};

struct ripemd160_ctx
{
	uint32_t h[5];
	uint8_t data[64];
	uint32_t data_size;
	uint64_t total_size;
};

struct ripemd256_ctx
{
	uint32_t h[8];
	uint8_t data[64];
	uint32_t data_size;
	uint64_t total_size;
};

struct ripemd320_ctx
{
	uint32_t h[10];
	uint8_t data[64];
	uint32_t data_size;
	uint64_t total_size;
};

int ripemd128_init(struct ripemd128_ctx *ctx);
int ripemd128_update(struct ripemd128_ctx *ctx, const void *data, size_t len);
int ripemd128_final(uint8_t *md, struct ripemd128_ctx *ctx);
int ripemd128(const uint8_t *data, size_t len, uint8_t *md);

int ripemd160_init(struct ripemd160_ctx *ctx);
int ripemd160_update(struct ripemd160_ctx *ctx, const void *data, size_t len);
int ripemd160_final(uint8_t *md, struct ripemd160_ctx *ctx);
int ripemd160(const uint8_t *data, size_t len, uint8_t *md);

int ripemd256_init(struct ripemd256_ctx *ctx);
int ripemd256_update(struct ripemd256_ctx *ctx, const void *data, size_t len);
int ripemd256_final(uint8_t *md, struct ripemd256_ctx *ctx);
int ripemd256(const uint8_t *data, size_t len, uint8_t *md);

int ripemd320_init(struct ripemd320_ctx *ctx);
int ripemd320_update(struct ripemd320_ctx *ctx, const void *data, size_t len);
int ripemd320_final(uint8_t *md, struct ripemd320_ctx *ctx);
int ripemd320(const uint8_t *data, size_t len, uint8_t *md);

#endif
