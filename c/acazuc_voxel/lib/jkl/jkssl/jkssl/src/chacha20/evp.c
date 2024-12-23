#include "chacha20/chacha20.h"
#include "evp/cipher.h"

const struct evp_cipher *evp_chacha20(void)
{
	static const struct evp_cipher evp =
	{
		.name = "CHACHA20",
		.init = (evp_cipher_init_t)chacha20_init,
		.update = (evp_cipher_update_t)chacha20_update,
		.final = (evp_cipher_final_t)chacha20_final,
		.mod = &g_evp_mod_ecb_nopad_iv,
		.block_size = 64,
		.key_size = 32,
		.ctx_size = sizeof(struct chacha20_ctx),
	};
	return &evp;
}

int chacha20_init(struct chacha20_ctx *ctx, const uint8_t *key,
                         const uint8_t *iv)
{
	chacha20_init_state(ctx, key, iv);
	return 1;
}

int chacha20_update(struct chacha20_ctx *ctx, uint8_t *out, const uint8_t *in,
                    int enc)
{
	(void)enc;
	chacha20_operate_block(ctx, out, in);
	return 1;
}

int chacha20_final(struct chacha20_ctx *ctx)
{
	(void)ctx;
	return 1;
}
