#include "evp/cipher.h"
#include "rc4/rc4.h"

static int rc4_init(void *ctx, const uint8_t *key, const uint8_t *iv)
{
	(void)iv;
	rc4_init_state(ctx, key);
	return 1;
}

static int rc4_update(void *ctx, uint8_t *out, const uint8_t *in, int enc)
{
	(void)enc;
	rc4_operate_block(ctx, out, in);
	return 1;
}

static int rc4_final(void *ctx)
{
	(void)ctx;
	return 1;
}

const struct evp_cipher *evp_rc4(void)
{
	static const struct evp_cipher evp =
	{
		.name = "RC4",
		.init = rc4_init,
		.update = rc4_update,
		.final = rc4_final,
		.mod = &g_evp_mod_ecb_nopad,
		.block_size = 1,
		.key_size = 16,
		.ctx_size = sizeof(struct rc4_ctx),
	};
	return &evp;
}
