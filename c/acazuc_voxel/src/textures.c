#include "textures.h"
#include "voxel.h"
#include "log.h"

#include <gfx/device.h>

#include <string.h>
#include <stdlib.h>
#include <png.h>

static bool load_png(const char *path, uint8_t **data, uint32_t *width, uint32_t *height)
{
	png_structp png = NULL;
	png_infop info = NULL;
	png_infop end_info = NULL;
	png_bytep *row_pointers = NULL;
	png_byte header[8];
	FILE *fp = NULL;
	int rowbytes;
	if (!(fp = fopen(path, "rb")))
		goto error1;
	if (fread(header, 1, 8, fp) != 8)
		goto error2;
	if (png_sig_cmp(header, 0, 8))
		goto error2;
	if (!(png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL)))
		goto error2;
	if (!(info = png_create_info_struct(png)))
		goto error3;
	if (!(end_info = png_create_info_struct(png)))
		goto error3;
	if (setjmp(png_jmpbuf(png)))
		goto error3;
	png_init_io(png, fp);
	png_set_sig_bytes(png, 8);
	png_read_info(png, info);
	int bit_depth, color_type;
	png_get_IHDR(png, info, width, height, &bit_depth, &color_type, NULL, NULL, NULL);
	if (bit_depth == 16)
		png_set_strip_16(png);
	if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
		png_set_expand_gray_1_2_4_to_8(png);
	if (png_get_valid(png, info, PNG_INFO_tRNS))
		png_set_tRNS_to_alpha(png);
	if (color_type == PNG_COLOR_TYPE_RGB
	 || color_type == PNG_COLOR_TYPE_GRAY
	 || color_type == PNG_COLOR_TYPE_PALETTE)
		png_set_filler(png, 0xFF, PNG_FILLER_AFTER);
	if (color_type == PNG_COLOR_TYPE_GRAY
	 || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
		png_set_gray_to_rgb(png);
	png_read_update_info(png, info);
	rowbytes = png_get_rowbytes(png, info);
	*data = (uint8_t*)malloc(rowbytes * *height);
	if (!*data)
		goto error3;
	row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * *height);
	if (!row_pointers)
		goto error4;
	for (uint32_t i = 0; i < *height; ++i)
		row_pointers[i] = *data + i * rowbytes;
	png_read_image(png, row_pointers);
	png_destroy_read_struct(&png, &info, &end_info);
	free(row_pointers);
	return true;
error4:
	free(*data);
error3:
	png_destroy_read_struct(&png, &info, &end_info);
error2:
	fclose(fp);
error1:
	return false;

}

static bool load_texture(struct texture *texture, const char *name)
{
	char path[1024];

	snprintf(path, sizeof(path), "data/textures/%s.png", name);
	if (!load_png(path, &texture->data, &texture->width, &texture->height))
	{
		LOG_ERROR("can't load image data of texture %s", name);
		return false;
	}
	texture->texture = GFX_TEXTURE_INIT();
	if (!gfx_create_texture(g_voxel->device, &texture->texture, GFX_TEXTURE_2D, GFX_R8G8B8A8, 1, texture->width, texture->height, 0))
	{
		LOG_ERROR("can't create gfx texture %s", name);
		return false;
	}
	gfx_set_texture_data(&texture->texture, 0, 0, texture->width, texture->height, 0, texture->width * texture->height * 4, texture->data);
	gfx_set_texture_addressing(&texture->texture, GFX_TEXTURE_ADDRESSING_REPEAT, GFX_TEXTURE_ADDRESSING_REPEAT, GFX_TEXTURE_ADDRESSING_REPEAT);
	gfx_set_texture_filtering(&texture->texture, GFX_FILTERING_NEAREST, GFX_FILTERING_NEAREST, GFX_FILTERING_NONE);
	//gfx_set_texture_anisotropy(&texture->texture, 16);
	gfx_set_texture_levels(&texture->texture, 0, 0);
	return true;
}

static bool build_whitepixel(struct texture *texture)
{
	texture->texture = GFX_TEXTURE_INIT();
	if (!gfx_create_texture(g_voxel->device, &texture->texture, GFX_TEXTURE_2D, GFX_R8G8B8A8, 1, 1, 1, 0))
	{
		LOG_ERROR("can't create gfx texture whitepixel");
		return false;
	}
	static const uint8_t data[] = {0xFF, 0xFF, 0xFF, 0xFF};
	gfx_set_texture_data(&texture->texture, 0, 0, 1, 1, 0, 4, data);
	gfx_set_texture_addressing(&texture->texture, GFX_TEXTURE_ADDRESSING_REPEAT, GFX_TEXTURE_ADDRESSING_REPEAT, GFX_TEXTURE_ADDRESSING_REPEAT);
	gfx_set_texture_filtering(&texture->texture, GFX_FILTERING_NEAREST, GFX_FILTERING_NEAREST, GFX_FILTERING_NONE);
	gfx_set_texture_levels(&texture->texture, 0, 0);
	return true;
}

struct textures *textures_new(void)
{
	struct textures *textures = calloc(sizeof(*textures), 1);
	if (!textures)
	{
		LOG_ERROR("textures allocation failed");
		return NULL;
	}

#define BUILD_TEXTURE(name, path) \
	do \
	{ \
		LOG_INFO("loading texture " #name); \
		if (!load_texture(&textures->name, path)) \
			goto err; \
	} while (0)

	if (!build_whitepixel(&textures->whitepixel))
		goto err;
	BUILD_TEXTURE(inventory, "gui/inventory");
	BUILD_TEXTURE(character, "mob/char");
	BUILD_TEXTURE(pigzombie, "mob/pigzombie");
	BUILD_TEXTURE(skeleton, "mob/skeleton");
	BUILD_TEXTURE(crafting, "gui/crafting");
	BUILD_TEXTURE(chicken, "mob/chicken");
	BUILD_TEXTURE(creeper, "mob/creeper");
	BUILD_TEXTURE(terrain, "terrain");
	BUILD_TEXTURE(pigman, "mob/pigman");
	BUILD_TEXTURE(spider, "mob/spider");
	BUILD_TEXTURE(zombie, "mob/zombie");
	BUILD_TEXTURE(clouds, "clouds");
	BUILD_TEXTURE(ghast, "mob/ghast");
	BUILD_TEXTURE(sheep, "mob/sheep");
	BUILD_TEXTURE(slime, "mob/slime");
	BUILD_TEXTURE(squid, "mob/squid");
	BUILD_TEXTURE(icons, "gui/icons");
	BUILD_TEXTURE(water, "misc/water");
	BUILD_TEXTURE(ascii, "font/ascii");
	BUILD_TEXTURE(logo, "title/logo");
	BUILD_TEXTURE(pack, "pack");
	BUILD_TEXTURE(moon, "terrain/moon");
	BUILD_TEXTURE(cow, "mob/cow");
	BUILD_TEXTURE(pig, "mob/pig");
	BUILD_TEXTURE(gui, "gui/gui");
	BUILD_TEXTURE(sun, "terrain/sun");
	BUILD_TEXTURE(bg, "gui/background");
	return textures;

#undef TEXTURE_BUILD

err:
	textures_delete(textures);
	return NULL;
}

static void clean_texture(struct texture *texture)
{
	free(texture->data);
	gfx_delete_texture(g_voxel->device, &texture->texture);
}

void textures_delete(struct textures *textures)
{
	if (!textures)
		return;

#define CLEAN_TEXTURE(name) \
	do \
	{ \
		LOG_INFO("cleaning texture " #name); \
		clean_texture(&textures->name); \
	} while (0)

	CLEAN_TEXTURE(whitepixel);
	CLEAN_TEXTURE(inventory);
	CLEAN_TEXTURE(character);
	CLEAN_TEXTURE(pigzombie);
	CLEAN_TEXTURE(skeleton);
	CLEAN_TEXTURE(crafting);
	CLEAN_TEXTURE(chicken);
	CLEAN_TEXTURE(creeper);
	CLEAN_TEXTURE(terrain);
	CLEAN_TEXTURE(pigman);
	CLEAN_TEXTURE(spider);
	CLEAN_TEXTURE(zombie);
	CLEAN_TEXTURE(clouds);
	CLEAN_TEXTURE(ghast);
	CLEAN_TEXTURE(sheep);
	CLEAN_TEXTURE(slime);
	CLEAN_TEXTURE(squid);
	CLEAN_TEXTURE(icons);
	CLEAN_TEXTURE(water);
	CLEAN_TEXTURE(ascii);
	CLEAN_TEXTURE(logo);
	CLEAN_TEXTURE(pack);
	CLEAN_TEXTURE(moon);
	CLEAN_TEXTURE(cow);
	CLEAN_TEXTURE(pig);
	CLEAN_TEXTURE(gui);
	CLEAN_TEXTURE(sun);
	CLEAN_TEXTURE(bg);

#undef CLEAN_TEXTURE
}

void texture_bind(struct texture *texture)
{
	const gfx_texture_t *binds[] = {&texture->texture};
	gfx_bind_samplers(g_voxel->device, 0, 1, binds);
}
