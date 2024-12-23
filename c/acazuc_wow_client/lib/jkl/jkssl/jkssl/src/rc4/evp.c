#include "evp/cipher.h"
#include "rc4/rc4.h"

const struct evp_cipher *evp_rc4(void)
{
	static const struct evp_cipher evp =
	{
		.name = "RC4",
		.init = (evp_cipher_init_t)rc4_init,
		.update = (evp_cipher_update_t)rc4_update,
		.final = (evp_cipher_final_t)rc4_final,
		.mod = &g_evp_mod_ecb_nopad,
		.block_size = 1,
		.key_size = 16,
		.ctx_size = sizeof(struct rc4_ctx),
	};
	return &evp;
}

int rc4_init(struct rc4_ctx *ctx, const uint8_t *key, const uint8_t *iv)
{
	(void)iv;
	rc4_init_state(ctx, key);
	return 1;
}

int rc4_update(struct rc4_ctx *ctx, uint8_t *out, const uint8_t *in, int enc)
{
	(void)enc;
	rc4_operate_block(ctx, out, in);
	return 1;
}

int rc4_final(struct rc4_ctx *ctx)
{
	(void)ctx;
	return 1;
}
