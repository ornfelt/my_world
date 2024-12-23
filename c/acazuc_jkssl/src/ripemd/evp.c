#include "ripemd/ripemd.h"
#include "evp/md.h"

const struct evp_md *evp_ripemd128(void)
{
	static const struct evp_md evp =
	{
		.name = "RIPEMD128",
		.init = (evp_md_init_t)ripemd128_init,
		.update = (evp_md_update_t)ripemd128_update,
		.final = (evp_md_final_t)ripemd128_final,
		.digest_size = 16,
		.block_size = 64,
		.ctx_size = sizeof(struct ripemd128_ctx),
	};
	return &evp;
}

const struct evp_md *evp_ripemd160(void)
{
	static const struct evp_md evp =
	{
		.name = "RIPEMD160",
		.init = (evp_md_init_t)ripemd160_init,
		.update = (evp_md_update_t)ripemd160_update,
		.final = (evp_md_final_t)ripemd160_final,
		.digest_size = 20,
		.block_size = 64,
		.ctx_size = sizeof(struct ripemd160_ctx),
	};
	return &evp;
}

const struct evp_md *evp_ripemd256(void)
{
	static const struct evp_md evp =
	{
		.name = "RIPEMD256",
		.init = (evp_md_init_t)ripemd256_init,
		.update = (evp_md_update_t)ripemd256_update,
		.final = (evp_md_final_t)ripemd256_final,
		.digest_size = 32,
		.block_size = 64,
		.ctx_size = sizeof(struct ripemd256_ctx),
	};
	return &evp;
}

const struct evp_md *evp_ripemd320(void)
{
	static const struct evp_md evp =
	{
		.name = "RIPEMD320",
		.init = (evp_md_init_t)ripemd320_init,
		.update = (evp_md_update_t)ripemd320_update,
		.final = (evp_md_final_t)ripemd320_final,
		.digest_size = 40,
		.block_size = 64,
		.ctx_size = sizeof(struct ripemd320_ctx),
	};
	return &evp;
}
