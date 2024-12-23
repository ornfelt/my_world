#include "sm3/sm3.h"
#include "evp/md.h"

const struct evp_md *evp_sm3(void)
{
	static const struct evp_md evp =
	{
		.name = "SM3",
		.init = (evp_md_init_t)sm3_init,
		.update = (evp_md_update_t)sm3_update,
		.final = (evp_md_final_t)sm3_final,
		.digest_size = SM3_DIGEST_LENGTH,
		.block_size = 64,
		.ctx_size = sizeof(struct sm3_ctx),
	};
	return &evp;
}
