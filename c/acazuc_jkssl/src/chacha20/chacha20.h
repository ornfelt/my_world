#ifndef JKSSL_CHACHA20_H
#define JKSSL_CHACHA20_H

#include <stdint.h>

struct chacha20_ctx
{
	uint32_t state[16];
};

void chacha20_init_state(struct chacha20_ctx *ctx, const uint8_t *key,
                         const uint8_t *iv);
void chacha20_operate_block(struct chacha20_ctx *ctx, uint8_t *out,
                            const uint8_t *in);

#endif
