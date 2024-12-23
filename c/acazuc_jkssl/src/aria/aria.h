#ifndef JKSSL_ARIA_H
#define JKSSL_ARIA_H

#include <stdint.h>

struct aria_ctx
{
	uint8_t ekeys[17][16];
	uint8_t dkeys[17][16];
	uint8_t key_len;
};

void aria_encrypt(struct aria_ctx *ctx, uint8_t *out, const uint8_t *in);
void aria_decrypt(struct aria_ctx *ctx, uint8_t *out, const uint8_t *in);
void aria_keyschedule(struct aria_ctx *ctx, const uint8_t *key, const uint8_t len);

#endif
