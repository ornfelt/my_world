#ifndef JKSSL_DES_H
#define JKSSL_DES_H

#include <stdint.h>

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

#endif
