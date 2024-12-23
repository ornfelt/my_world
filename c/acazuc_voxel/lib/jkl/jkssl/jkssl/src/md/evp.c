#include "evp/md.h"
#include "md/md.h"

const struct evp_md *evp_md2(void)
{
	static const struct evp_md evp =
	{
		.name = "MD2",
		.init = (evp_md_init_t)md2_init,
		.update = (evp_md_update_t)md2_update,
		.final = (evp_md_final_t)md2_final,
		.digest_size = MD2_DIGEST_LENGTH,
		.block_size = 16,
		.ctx_size = sizeof(struct md2_ctx),
	};
	return &evp;
}

const struct evp_md *evp_md4(void)
{
	static const struct evp_md evp =
	{
		.name = "MD4",
		.init = (evp_md_init_t)md4_init,
		.update = (evp_md_update_t)md4_update,
		.final = (evp_md_final_t)md4_final,
		.digest_size = MD4_DIGEST_LENGTH,
		.block_size = 64,
		.ctx_size = sizeof(struct md4_ctx),
	};
	return &evp;
}

const struct evp_md *evp_md5(void)
{
	static const struct evp_md evp =
	{
		.name = "MD5",
		.init = (evp_md_init_t)md5_init,
		.update = (evp_md_update_t)md5_update,
		.final = (evp_md_final_t)md5_final,
		.digest_size = MD5_DIGEST_LENGTH,
		.block_size = 64,
		.ctx_size = sizeof(struct md5_ctx),
	};
	return &evp;
}
