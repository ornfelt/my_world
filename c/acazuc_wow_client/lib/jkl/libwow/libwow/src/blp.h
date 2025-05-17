#ifndef WOW_BLP_H
#define WOW_BLP_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct wow_mpq_file;

struct wow_blp_header
{
	uint8_t magic[4];
	uint32_t type;
	uint8_t compression;
	uint8_t alpha_depth;
	uint8_t alpha_type;
	uint8_t has_mipmaps;
	uint32_t width;
	uint32_t height;
	uint32_t mipmap_offsets[16];
	uint32_t mipmap_lengths[16];
	uint32_t palette[256];
};

struct wow_blp_mipmap
{
	uint32_t width;
	uint32_t height;
	uint8_t *data;
	uint32_t data_len;
};

struct wow_blp_file
{
	struct wow_blp_header header;
	struct wow_blp_mipmap *mipmaps;
	uint32_t mipmaps_nb;
};

struct wow_blp_file *wow_blp_file_new(struct wow_mpq_file *mpq);
void wow_blp_file_delete(struct wow_blp_file *file);

bool wow_blp_decode_rgba(const struct wow_blp_file *file,
                         uint8_t mipmap_id,
                         uint32_t *width,
                         uint32_t *height,
                         uint8_t **data);

#ifdef __cplusplus
}
#endif

#endif
