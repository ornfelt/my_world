#ifndef JKSSL_CAMELLIA_H
#define JKSSL_CAMELLIA_H

#include <stdint.h>

struct evp_cipher;

struct camellia_ctx
{
	uint8_t  key_len;
	uint64_t kw[4];
	uint64_t k[24];
	uint64_t ke[6];
};

void camellia_encrypt(struct camellia_ctx *ctx, uint8_t *out, const uint8_t *in);
void camellia_decrypt(struct camellia_ctx *ctx, uint8_t *out, const uint8_t *in);
void camellia_keyschedule(struct camellia_ctx *ctx, const uint8_t *key,
                          uint8_t len);

int camellia128_init(struct camellia_ctx *ctx, const uint8_t *key, 
                     const uint8_t *iv);
int camellia128_update(struct camellia_ctx *ctx, uint8_t *out,
                       const uint8_t *in, int enc);
int camellia128_final(struct camellia_ctx *ctx);
int camellia192_init(struct camellia_ctx *ctx, const uint8_t *key,
                     const uint8_t *iv);
int camellia192_update(struct camellia_ctx *ctx, uint8_t *out,
                       const uint8_t *in, int enc);
int camellia192_final(struct camellia_ctx *ctx);
int camellia256_init(struct camellia_ctx *ctx, const uint8_t *key,
                     const uint8_t *iv);
int camellia256_update(struct camellia_ctx *ctx, uint8_t *out,
                       const uint8_t *in, int enc);
int camellia256_final(struct camellia_ctx *ctx);

#endif
