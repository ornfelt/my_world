#include "font/model.h"

#include "memory.h"
#include "cache.h"
#include "log.h"
#include "wow.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_MODULE_H

struct font;

struct font_model *font_model_new(char *filename, const char *data, size_t len)
{
	struct font_model *model = mem_zalloc(MEM_FONT, sizeof(*model));
	if (!model)
	{
		LOG_ERROR("failed to allocate font model");
		return NULL;
	}
	refcount_init(&model->refcount, 1);
	model->filename = filename;
	model->data = mem_malloc(MEM_FONT, len);
	if (!model->data)
	{
		LOG_ERROR("failed to duplicate font memory");
		goto err;
	}
	memcpy(model->data, data, len);
	if (FT_New_Memory_Face(g_wow->ft_lib, model->data, len, 0, &model->ft_face))
	{
		LOG_ERROR("failed to create new memory face");
		goto err;
	}
	if (FT_Select_Charmap(model->ft_face, ft_encoding_unicode))
	{
		LOG_ERROR("failed to set face charmap");
		FT_Done_Face(model->ft_face);
		goto err;
	}
	return model;

err:
	mem_free(MEM_FONT, model->data);
	mem_free(MEM_FONT, model);
	return NULL;
}

void font_model_free(struct font_model *model)
{
	if (!model)
		return;
	if (refcount_dec(&model->refcount))
		return;
	cache_lock_font(g_wow->cache);
	if (refcount_get(&model->refcount))
	{
		cache_unlock_font(g_wow->cache);
		return;
	}
	cache_unref_unmutexed_font(g_wow->cache, model->filename);
	cache_unlock_font(g_wow->cache);
	FT_Done_Face(model->ft_face);
	mem_free(MEM_GENERIC, model->filename);
	mem_free(MEM_FONT, model->data);
	mem_free(MEM_FONT, model);
}

void font_model_ref(struct font_model *model)
{
	refcount_inc(&model->refcount);
}

bool font_model_set_size(struct font_model *model, uint32_t size)
{
	if (FT_Set_Char_Size(model->ft_face, 0, size * 64, 0, 0))
		return false;
	return true;
}
