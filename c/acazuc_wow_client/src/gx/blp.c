#include "gx/blp.h"

#include "loader.h"
#include "memory.h"
#include "cache.h"
#include "log.h"
#include "wow.h"

#include <gfx/device.h>

#include <wow/blp.h>
#include <wow/mpq.h>

#include <inttypes.h>
#include <string.h>

struct gx_blp_mipmap
{
	uint8_t *data;
	uint32_t data_len;
	uint32_t width;
	uint32_t height;
};

static void ask_init(struct gx_blp *blp);

struct gx_blp *gx_blp_from_filename(char *filename)
{
	struct gx_blp *blp = mem_zalloc(MEM_GX, sizeof(*blp));
	if (!blp)
		return NULL;
	blp->texture = GFX_TEXTURE_INIT();
	blp->filename = filename;
	refcount_init(&blp->refcount, 1);
	return blp;
}

struct gx_blp *gx_blp_from_data(uint8_t *data, uint32_t width, uint32_t height)
{
	struct gx_blp *blp = mem_zalloc(MEM_GX, sizeof(*blp));
	if (!blp)
		return NULL;
	blp->format = GFX_R8G8B8A8;
	blp->mipmaps_nb = 1;
	blp->data = data;
	blp->texture = GFX_TEXTURE_INIT();
	blp->width = width;
	blp->height = height;
	refcount_init(&blp->refcount, 1);
	blp->flags = GX_BLP_FLAG_LOAD_ASKED | GX_BLP_FLAG_LOADED;
	ask_init(blp);
	return blp;
}

static void free_mipmaps(struct gx_blp_mipmap *mipmaps, uint32_t nb)
{
	if (!mipmaps)
		return;
	for (uint32_t i = 0; i < nb; ++i)
		mem_free(MEM_GX, mipmaps[i].data);
	mem_free(MEM_GX, mipmaps);
}

static void blp_unload_task(struct wow_mpq_compound *mpq_compound, void *userdata)
{
	(void)mpq_compound;
	struct gx_blp *blp = userdata;
	free_mipmaps(blp->mipmaps, blp->mipmaps_nb);
	gfx_delete_texture(g_wow->device, &blp->texture);
	mem_free(MEM_GENERIC, blp->data);
	mem_free(MEM_GENERIC, blp->filename);
	mem_free(MEM_GX, blp);
}

void gx_blp_free(struct gx_blp *blp)
{
	if (!blp)
		return;
	if (refcount_dec(&blp->refcount))
		return;
	if (blp->filename)
	{
		cache_lock_blp(g_wow->cache);
		if (refcount_get(&blp->refcount))
		{
			cache_unlock_blp(g_wow->cache);
			return;
		}
		cache_unref_unmutexed_blp(g_wow->cache, blp->filename);
		cache_unlock_blp(g_wow->cache);
	}
	loader_push(g_wow->loader, ASYNC_TASK_BLP_UNLOAD, blp_unload_task, blp);
}

void gx_blp_ref(struct gx_blp *blp)
{
	refcount_inc(&blp->refcount);
}

static bool initialize(void *userdata)
{
	struct gx_blp *blp = userdata;
	if (blp->data)
	{
		gfx_create_texture(g_wow->device, &blp->texture, GFX_TEXTURE_2D, blp->format, 1, blp->width, blp->height, 0);
		gfx_set_texture_levels(&blp->texture, 0, 0);
		gfx_set_texture_anisotropy(&blp->texture, g_wow->anisotropy);
		gfx_set_texture_filtering(&blp->texture, GFX_FILTERING_LINEAR, GFX_FILTERING_LINEAR, GFX_FILTERING_LINEAR);
		gfx_set_texture_addressing(&blp->texture, GFX_TEXTURE_ADDRESSING_REPEAT, GFX_TEXTURE_ADDRESSING_REPEAT, GFX_TEXTURE_ADDRESSING_REPEAT);
		gfx_finalize_texture(&blp->texture);
		gfx_set_texture_data(&blp->texture, 0, 0, blp->width, blp->height, 0, blp->width * blp->height * 4, blp->data);
		mem_free(MEM_LIBWOW, blp->data);
		blp->data = NULL;
	}
	else
	{
		if (!blp->mipmaps)
			goto end;
		gfx_create_texture(g_wow->device, &blp->texture, GFX_TEXTURE_2D, blp->format, blp->mipmaps_nb, blp->mipmaps[0].width, blp->mipmaps[0].height, 0);
		gfx_set_texture_levels(&blp->texture, 0, blp->mipmaps_nb - 1);
		gfx_set_texture_anisotropy(&blp->texture, g_wow->anisotropy);
		gfx_set_texture_filtering(&blp->texture, GFX_FILTERING_LINEAR, GFX_FILTERING_LINEAR, GFX_FILTERING_LINEAR);
		gfx_set_texture_addressing(&blp->texture, GFX_TEXTURE_ADDRESSING_REPEAT, GFX_TEXTURE_ADDRESSING_REPEAT, GFX_TEXTURE_ADDRESSING_REPEAT);
		gfx_finalize_texture(&blp->texture);
		for (size_t i = 0; i < blp->mipmaps_nb; ++i)
		{
			struct gx_blp_mipmap *mipmap = &blp->mipmaps[i];
			gfx_set_texture_data(&blp->texture, i, 0, mipmap->width, mipmap->height, 0, mipmap->data_len, mipmap->data);
		}
		free_mipmaps(blp->mipmaps, blp->mipmaps_nb);
		blp->mipmaps = NULL;
	}
	gx_blp_flag_set(blp, GX_BLP_FLAG_INITIALIZED);

end:
	gx_blp_free(blp);
	return true;
}

static void ask_init(struct gx_blp *blp)
{
	gx_blp_ref(blp);
	loader_init_object(g_wow->loader, LOADER_BLP, initialize, blp);
}

static bool load_dummy_mipmap(struct gx_blp *blp)
{
	blp->mipmaps_nb = 1;
	blp->mipmaps = mem_malloc(MEM_GX, sizeof(*blp->mipmaps) * blp->mipmaps_nb);
	if (!blp->mipmaps)
		return false;
	blp->mipmaps[0].width = 1;
	blp->mipmaps[0].height = 1;
	blp->mipmaps[0].data_len = 4;
	blp->mipmaps[0].data = mem_malloc(MEM_GX, blp->mipmaps[0].data_len);
	if (!blp->mipmaps[0].data)
		return false;
	static const uint8_t tmp[4] = {0, 0xff, 0, 0xff};
	memcpy(blp->mipmaps[0].data, tmp, 4);
	return true;
}

bool gx_blp_load(struct gx_blp *blp, struct wow_blp_file *file)
{
	if (gx_blp_flag_get(blp, GX_BLP_FLAG_LOADED))
		return true;
	if (file->header.type != 1)
	{
		LOG_WARN("unsupported BLP type: %u", file->header.type);
		if (!load_dummy_mipmap(blp))
			return false;
		goto end;
	}
	blp->mipmaps_nb = file->mipmaps_nb;
	blp->mipmaps = mem_zalloc(MEM_GX, sizeof(*blp->mipmaps) * blp->mipmaps_nb);
	if (!blp->mipmaps)
		return false;
	blp->width = file->header.width;
	blp->height = file->header.height;
	switch (file->header.compression)
	{
		case 1:
		{
			blp->format = GFX_R8G8B8A8;
			for (uint32_t i = 0; i < blp->mipmaps_nb; ++i)
			{
				struct gx_blp_mipmap *mipmap = &blp->mipmaps[i];
				mipmap->width = file->mipmaps[i].width;
				mipmap->height = file->mipmaps[i].height;
				mipmap->data_len = mipmap->width * mipmap->height * 4;
				mipmap->data = mem_malloc(MEM_GX, mipmap->data_len);
				if (!mipmap->data)
					return false;
				const uint8_t *indexes = file->mipmaps[i].data;
				const uint8_t *alphas = indexes + mipmap->width * mipmap->height;
				uint32_t idx = 0;
				uint32_t n = mipmap->width * mipmap->height;
				for (uint32_t j = 0; j < n; ++j)
				{
					uint32_t p = file->header.palette[indexes[j]];
					mipmap->data[idx++] = p >> 16;
					mipmap->data[idx++] = p >> 8;
					mipmap->data[idx++] = p >> 0;
					switch (file->header.alpha_depth)
					{
						case 0:
							mipmap->data[idx++] = 0xff;
							break;
						case 1:
							mipmap->data[idx++] = ((alphas[j / 8] >> (i % 8)) & 1) * 0xff;
							break;
						case 4:
						{
							uint8_t a = ((alphas[j / 2] >> ((i % 2) * 4)) & 0xf);
							mipmap->data[idx++] = a | (a << 4);
							break;
						}
						case 8:
							mipmap->data[idx++] = alphas[j];
							break;
						default:
							mipmap->data[idx++] = 0xff;
							LOG_WARN("unsupported BLP alpha depth: %u", file->header.alpha_depth);
							break;
					}
				}
			}
			break;
		}
		case 2:
		{
			switch (file->header.alpha_type)
			{
				case 0:
					blp->format = (file->header.alpha_depth > 0) ? GFX_BC1_RGBA : GFX_BC1_RGB;
					break;
				case 1:
					blp->format = GFX_BC2_RGBA;
					break;
				case 7:
					blp->format = GFX_BC3_RGBA;
					break;
				default:
					LOG_WARN("unsupported DXT alpha type: %u", file->header.alpha_type);
					if (!load_dummy_mipmap(blp))
						return false;
					goto end;
			}
			for (uint32_t i = 0; i < blp->mipmaps_nb; ++i)
			{
				struct gx_blp_mipmap *mipmap = &blp->mipmaps[i];
				mipmap->width = file->mipmaps[i].width;
				mipmap->height = file->mipmaps[i].height;
				mipmap->data_len = file->mipmaps[i].data_len;
				mipmap->data = mem_malloc(MEM_GX, mipmap->data_len);
				if (!mipmap->data)
					return false;
				memcpy(mipmap->data, file->mipmaps[i].data, mipmap->data_len);
			}
			break;
		}
		case 3:
		{
			for (uint32_t i = 0; i < blp->mipmaps_nb; ++i)
			{
				struct gx_blp_mipmap *mipmap = &blp->mipmaps[i];
				mipmap->width = file->mipmaps[i].width;
				mipmap->height = file->mipmaps[i].height;
				mipmap->data_len = file->mipmaps[i].data_len;
				mipmap->data = mem_malloc(MEM_GX, mipmap->data_len);
				if (!mipmap->data)
					return false;
				memcpy(mipmap->data, file->mipmaps[i].data, mipmap->data_len);
			}
			break;
		}
		default:
		{
			LOG_WARN("unsupported BLP compression: %u", file->header.compression);
			if (!load_dummy_mipmap(blp))
				return false;
			break;
		}
	}

end:
	gx_blp_flag_set(blp, GX_BLP_FLAG_LOADED);
	ask_init(blp);
	return true;
}

static void load_task(struct wow_mpq_compound *mpq_compound, void *userdata)
{
	struct gx_blp *blp = userdata;
	struct wow_mpq_file *mpq_file = NULL;
	struct wow_blp_file *blp_file = NULL;

	mpq_file = wow_mpq_get_file(mpq_compound, blp->filename);
	if (!mpq_file)
	{
		LOG_WARN("BLP file not found: %s", blp->filename);
		goto end;
	}
	blp_file = wow_blp_file_new(mpq_file);
	if (!blp_file)
	{
		LOG_ERROR("failed to create blp from file %s", blp->filename);
		goto end;
	}
	if (!gx_blp_load(blp, blp_file))
	{
		LOG_ERROR("failed to load blp");
		goto end;
	}

end:
	wow_mpq_file_delete(mpq_file);
	wow_blp_file_delete(blp_file);
	gx_blp_free(blp);
}

void gx_blp_ask_load(struct gx_blp *blp)
{
	if (gx_blp_flag_set(blp, GX_BLP_FLAG_LOAD_ASKED))
		return;
	gx_blp_ref(blp);
	loader_push(g_wow->loader, ASYNC_TASK_BLP_LOAD, load_task, blp);
}

void gx_blp_bind(struct gx_blp *blp, uint8_t slot)
{
	const gfx_texture_t *texture;
	if (blp && gx_blp_flag_get(blp, GX_BLP_FLAG_INITIALIZED))
		texture = &blp->texture;
	else
		texture = &g_wow->grey_texture->texture;
	gfx_bind_samplers(g_wow->device, slot, 1, &texture);
}
