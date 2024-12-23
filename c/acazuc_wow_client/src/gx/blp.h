#ifndef GX_BLP_H
#define GX_BLP_H

#include "refcount.h"

#include <gfx/objects.h>

#include <stdbool.h>

struct wow_blp_file;
struct gx_blp_mipmap;

enum gx_blp_flag
{
	GX_BLP_FLAG_LOAD_ASKED  = (1 << 0),
	GX_BLP_FLAG_LOADED      = (1 << 1),
	GX_BLP_FLAG_INITIALIZED = (1 << 2),
};

struct gx_blp
{
	struct gx_blp_mipmap *mipmaps;
	enum gfx_format format;
	uint32_t mipmaps_nb;
	enum gx_blp_flag flags;
	gfx_texture_t texture;
	char *filename;
	void *data;
	uint32_t width;
	uint32_t height;
	refcount_t refcount;
};

struct gx_blp *gx_blp_from_filename(char *filename);
struct gx_blp *gx_blp_from_data(uint8_t *data, uint32_t width, uint32_t height);
void gx_blp_free(struct gx_blp *blp);
void gx_blp_ref(struct gx_blp *blp);
void gx_blp_ask_load(struct gx_blp *blp);
bool gx_blp_load(struct gx_blp *blp, struct wow_blp_file *file);
void gx_blp_bind(struct gx_blp *blp, uint8_t slot);

static inline bool gx_blp_flag_get(struct gx_blp *blp, enum gx_blp_flag flag)
{
	return (__atomic_load_n(&blp->flags, __ATOMIC_RELAXED) & flag) == flag;
}

static inline bool gx_blp_flag_set(struct gx_blp *blp, enum gx_blp_flag flag)
{
	return (__atomic_fetch_or(&blp->flags, flag, __ATOMIC_SEQ_CST) & flag) == flag;
}

static inline bool gx_blp_flag_clear(struct gx_blp *blp, enum gx_blp_flag flag)
{
	return (__atomic_fetch_and(&blp->flags, ~flag, __ATOMIC_RELAXED) & flag) == flag;
}

#endif
