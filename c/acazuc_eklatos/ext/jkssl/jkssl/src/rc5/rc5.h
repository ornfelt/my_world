#ifndef JKSSL_RC5_H
#define JKSSL_RC5_H

#include <stdint.h>

struct rc5_ctx
{
	uint32_t s[26];
};

void rc5_encrypt(struct rc5_ctx *ctx, uint8_t *out, const uint8_t *in);
void rc5_decrypt(struct rc5_ctx *ctx, uint8_t *out, const uint8_t *in);
void rc5_keyschedule(struct rc5_ctx *ctx, const uint8_t *key);

#endif
