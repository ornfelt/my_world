#ifndef JKSSL_DES_H
#define JKSSL_DES_H

#include <stdint.h>

struct evp_cipher;

struct des_ctx
{
	uint64_t keys[16];
};

struct des_ede_ctx
{
	struct des_ctx ctx1;
	struct des_ctx ctx2;
};

struct des_ede3_ctx
{
	struct des_ctx ctx1;
	struct des_ctx ctx2;
	struct des_ctx ctx3;
};

void des_generate_keys(struct des_ctx *ctx, const uint8_t *key);
void des_operate_block(struct des_ctx *ctx, uint8_t *out, const uint8_t *in,
                       int mode);

int des_init(struct des_ctx *ctx, const uint8_t *key, const uint8_t *iv);
int des_update(struct des_ctx *ctx, uint8_t *out, const uint8_t *in, int enc);
int des_final(struct des_ctx *ctx);
int des_ede_init(struct des_ede_ctx *ctx, const uint8_t *key,
                 const uint8_t *iv);
int des_ede_update(struct des_ede_ctx *ctx, uint8_t *out, const uint8_t *in,
                   int enc);
int des_ede_final(struct des_ede_ctx *ctx);
int des_ede3_init(struct des_ede3_ctx *ctx, const uint8_t *key,
                  const uint8_t *iv);
int des_ede3_update(struct des_ede3_ctx *ctx, uint8_t *out, const uint8_t *in,
                    int enc);
int des_ede3_final(struct des_ede3_ctx *ctx);

#endif
