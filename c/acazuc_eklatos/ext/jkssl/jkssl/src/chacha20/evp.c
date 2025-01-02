#include "chacha20/chacha20.h"
#include "evp/cipher.h"

static int chacha20_init(void *ctx, const uint8_t *key, const uint8_t *iv)
{
	chacha20_init_state(ctx, key, iv);
	return 1;
}

static int chacha20_update(void *ctx, uint8_t *out, const uint8_t *in, int enc)
{
	(void)enc;
	chacha20_operate_block(ctx, out, in);
	return 1;
}

static int chacha20_final(void *ctx)
{
	(void)ctx;
	return 1;
}

const struct evp_cipher *evp_chacha20(void)
{
	static const struct evp_cipher evp =
	{
		.name = "CHACHA20",
		.init = chacha20_init,
		.update = chacha20_update,
		.final = chacha20_final,
		.mod = &g_evp_mod_ecb_nopad_iv,
		.block_size = 64,
		.key_size = 32,
		.ctx_size = sizeof(struct chacha20_ctx),
	};
	return &evp;
}
