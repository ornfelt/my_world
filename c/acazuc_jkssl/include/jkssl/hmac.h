#ifndef JKSSL_HMAC_H
#define JKSSL_HMAC_H

#include <stdint.h>
#include <stddef.h>

struct evp_md_ctx;
struct hmac_ctx;
struct evp_md;

struct hmac_ctx *hmac_ctx_new(void);
void hmac_ctx_free(struct hmac_ctx *ctx);
int hmac_init(struct hmac_ctx *ctx, const void *key, size_t key_len,
              const struct evp_md *evp_md);
int hmac_update(struct hmac_ctx *ctx, const uint8_t *data, size_t len);
int hmac_final(struct hmac_ctx *ctx, uint8_t *md, unsigned *len);
size_t hmac_size(const struct hmac_ctx *ctx);

uint8_t *hmac(const struct evp_md *evp_md, const void *key, size_t key_len,
              const uint8_t *d, size_t n, uint8_t *md, unsigned *md_len);

#endif
