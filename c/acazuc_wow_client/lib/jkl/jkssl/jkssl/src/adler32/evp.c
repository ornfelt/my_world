#include "adler32/adler32.h"
#include "evp/md.h"

const struct evp_md *evp_adler32(void)
{
	static const struct evp_md evp =
	{
		.name = "ADLER32",
		.init = (evp_md_init_t)adler32_init,
		.update = (evp_md_update_t)adler32_update,
		.final = (evp_md_final_t)adler32_final,
		.digest_size = ADLER32_DIGEST_LENGTH,
		.block_size = 1,
		.ctx_size = sizeof(struct adler32_ctx),
	};
	return &evp;
}
