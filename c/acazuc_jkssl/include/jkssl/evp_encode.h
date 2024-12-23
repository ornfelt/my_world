#ifndef JKSSL_EVP_ENCODE_H
#define JKSSL_EVP_ENCODE_H

#include <stdint.h>
#include <stddef.h>

#define EVP_ENCODE_MAX_SIZE(n) ((n) * 4 / 3 + (n) / 48 + 5)
#define EVP_DECODE_MAX_SIZE(n) ((n) * 3 / 4 + 4)

struct evp_encode_ctx *evp_encode_ctx_new(void);
void evp_encode_ctx_free(struct evp_encode_ctx *ctx);

void evp_encode_init(struct evp_encode_ctx *ctx);
int evp_encode_update(struct evp_encode_ctx *ctx, uint8_t *out, size_t *outl,
                      const uint8_t *in, size_t inl);
void evp_encode_final(struct evp_encode_ctx *ctx, uint8_t *out, size_t *outl);
int evp_encode_block(uint8_t *out, const uint8_t *in, size_t inl);

void evp_decode_init(struct evp_encode_ctx *ctx);
int evp_decode_update(struct evp_encode_ctx *ctx, uint8_t *out, size_t *outl,
                      const uint8_t *in, size_t inl);
int evp_decode_final(struct evp_encode_ctx *ctx, uint8_t *out, size_t *outl);

#endif
