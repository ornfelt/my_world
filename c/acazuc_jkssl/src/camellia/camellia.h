#ifndef JKSSL_CAMELLIA_H
#define JKSSL_CAMELLIA_H

#include <stdint.h>

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

#endif
