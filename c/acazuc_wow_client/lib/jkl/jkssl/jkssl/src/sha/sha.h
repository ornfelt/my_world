#ifndef JKSSL_SHA_H
#define JKSSL_SHA_H

#include <stdint.h>
#include <stddef.h>

#define SHA_DIGEST_LENGTH    20
#define SHA224_DIGEST_LENGTH 28
#define SHA256_DIGEST_LENGTH 32
#define SHA384_DIGEST_LENGTH 48
#define SHA512_DIGEST_LENGTH 64

struct evp_md;

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

struct sha224_ctx
{
	uint32_t h[8];
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

struct sha384_ctx
{
	uint64_t h[8];
	uint8_t data[128];
	uint64_t data_size;
	uint64_t total_size;
};

struct sha512_ctx
{
	uint64_t h[8];
	uint8_t data[128];
	uint64_t data_size;
	uint64_t total_size;
};

int sha0_init(struct sha0_ctx *ctx);
int sha0_update(struct sha0_ctx *ctx, const void *data, size_t size);
int sha0_final(uint8_t *md, struct sha0_ctx *ctx);
int sha0(const uint8_t *data, size_t size, uint8_t *md);

int sha1_init(struct sha1_ctx *ctx);
int sha1_update(struct sha1_ctx *ctx, const void *data, size_t size);
int sha1_final(uint8_t *md, struct sha1_ctx *ctx);
int sha1(const uint8_t *data, size_t size, uint8_t *md);

int sha224_init(struct sha224_ctx *ctx);
int sha224_update(struct sha224_ctx *ctx, const void *data, size_t size);
int sha224_final(uint8_t *md, struct sha224_ctx *ctx);
int sha224(const uint8_t *data, size_t size, uint8_t *md);

int sha256_init(struct sha256_ctx *ctx);
int sha256_update(struct sha256_ctx *ctx, const void *data, size_t size);
int sha256_final(uint8_t *md, struct sha256_ctx *ctx);
int sha256(const uint8_t *data, size_t size, uint8_t *md);

int sha384_init(struct sha384_ctx *ctx);
int sha384_update(struct sha384_ctx *ctx, const void *data, size_t size);
int sha384_final(uint8_t *md, struct sha384_ctx *ctx);
int sha384(const uint8_t *data, size_t size, uint8_t *md);

int sha512_init(struct sha512_ctx *ctx);
int sha512_update(struct sha512_ctx *ctx, const void *data, size_t size);
int sha512_final(uint8_t *md, struct sha512_ctx *ctx);
int sha512(const uint8_t *data, size_t size, uint8_t *md);

#endif
