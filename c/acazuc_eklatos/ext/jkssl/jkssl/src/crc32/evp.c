#include "crc32/crc32.h"
#include "evp/md.h"

const struct evp_md *evp_crc32(void)
{
	static const struct evp_md evp =
	{
		.name = "CRC32",
		.init = (evp_md_init_t)crc32_init,
		.update = (evp_md_update_t)crc32_update,
		.final = (evp_md_final_t)crc32_final,
		.digest_size = 4,
		.block_size = 1,
		.ctx_size = sizeof(struct crc32_ctx),
	};

	return &evp;
}
