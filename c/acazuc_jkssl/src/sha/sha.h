#ifndef JKSSL_SHA_H
#define JKSSL_SHA_H

#include <stdint.h>
#include <stddef.h>

struct sha0_ctx
{
	uint32_t h[5];
	uint8_t data[64];
	uint32_t data_size;
	uint64_t total_size;
};

struct sha1_ctx
{
	uint32_t h[5];
	uint8_t data[64];
	uint32_t data_size;
	uint64_t total_size;
};

struct sha256_ctx
{
	uint32_t h[8];
	uint8_t data[64];
	uint32_t data_size;
	uint64_t total_size;
};

struct sha512_ctx
{
	uint64_t h[8];
	uint8_t data[128];
	uint64_t data_size;
	uint64_t total_size;
};

struct sha3_ctx
{
	uint64_t h[25];
	uint8_t data[200];
	uint64_t data_size;
	uint64_t total_size;
	uint8_t block_size;
};

int sha0_init(struct sha0_ctx *ctx);
int sha0_update(struct sha0_ctx *ctx, const void *data, size_t size);
int sha0_final(uint8_t *md, struct sha0_ctx *ctx);
int sha0(const uint8_t *data, size_t size, uint8_t *md);

int sha1_init(struct sha1_ctx *ctx);
int sha1_update(struct sha1_ctx *ctx, const void *data, size_t size);
int sha1_final(uint8_t *md, struct sha1_ctx *ctx);
int sha1(const uint8_t *data, size_t size, uint8_t *md);

int sha224_init(struct sha256_ctx *ctx);
int sha224_update(struct sha256_ctx *ctx, const void *data, size_t size);
int sha224_final(uint8_t *md, struct sha256_ctx *ctx);
int sha224(const uint8_t *data, size_t size, uint8_t *md);

int sha256_init(struct sha256_ctx *ctx);
int sha256_update(struct sha256_ctx *ctx, const void *data, size_t size);
int sha256_final(uint8_t *md, struct sha256_ctx *ctx);
int sha256(const uint8_t *data, size_t size, uint8_t *md);

int sha256_192_init(struct sha256_ctx *ctx);
int sha256_192_update(struct sha256_ctx *ctx, const void *data, size_t size);
int sha256_192_final(uint8_t *md, struct sha256_ctx *ctx);
int sha256_192(const uint8_t *data, size_t size, uint8_t *md);

int sha384_init(struct sha512_ctx *ctx);
int sha384_update(struct sha512_ctx *ctx, const void *data, size_t size);
int sha384_final(uint8_t *md, struct sha512_ctx *ctx);
int sha384(const uint8_t *data, size_t size, uint8_t *md);

int sha512_init(struct sha512_ctx *ctx);
int sha512_update(struct sha512_ctx *ctx, const void *data, size_t size);
int sha512_final(uint8_t *md, struct sha512_ctx *ctx);
int sha512(const uint8_t *data, size_t size, uint8_t *md);

int sha512_224_init(struct sha512_ctx *ctx);
int sha512_224_update(struct sha512_ctx *ctx, const void *data, size_t size);
int sha512_224_final(uint8_t *md, struct sha512_ctx *ctx);
int sha512_224(const uint8_t *data, size_t size, uint8_t *md);

int sha512_256_init(struct sha512_ctx *ctx);
int sha512_256_update(struct sha512_ctx *ctx, const void *data, size_t size);
int sha512_256_final(uint8_t *md, struct sha512_ctx *ctx);
int sha512_256(const uint8_t *data, size_t size, uint8_t *md);

int sha3_224_init(struct sha3_ctx *ctx);
int sha3_224_update(struct sha3_ctx *ctx, const void *data, size_t size);
int sha3_224_final(uint8_t *md, struct sha3_ctx *ctx);
int sha3_224(const uint8_t *data, size_t size, uint8_t *md);

int sha3_256_init(struct sha3_ctx *ctx);
int sha3_256_update(struct sha3_ctx *ctx, const void *data, size_t size);
int sha3_256_final(uint8_t *md, struct sha3_ctx *ctx);
int sha3_256(const uint8_t *data, size_t size, uint8_t *md);

int sha3_384_init(struct sha3_ctx *ctx);
int sha3_384_update(struct sha3_ctx *ctx, const void *data, size_t size);
int sha3_384_final(uint8_t *md, struct sha3_ctx *ctx);
int sha3_384(const uint8_t *data, size_t size, uint8_t *md);

int sha3_512_init(struct sha3_ctx *ctx);
int sha3_512_update(struct sha3_ctx *ctx, const void *data, size_t size);
int sha3_512_final(uint8_t *md, struct sha3_ctx *ctx);
int sha3_512(const uint8_t *data, size_t size, uint8_t *md);

int shake128_init(struct sha3_ctx *ctx);
int shake128_update(struct sha3_ctx *ctx, const void *data, size_t size);
int shake128_final(uint8_t *md, struct sha3_ctx *ctx);
int shake128(const uint8_t *data, size_t size, uint8_t *md);

int shake256_init(struct sha3_ctx *ctx);
int shake256_update(struct sha3_ctx *ctx, const void *data, size_t size);
int shake256_final(uint8_t *md, struct sha3_ctx *ctx);
int shake256(const uint8_t *data, size_t size, uint8_t *md);

#endif
