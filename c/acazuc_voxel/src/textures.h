#ifndef TEXTURES_H
#define TEXTURES_H

#include <gfx/objects.h>

struct texture
{
	gfx_texture_t texture;
	uint8_t *data;
	uint32_t width;
	uint32_t height;
};

struct textures
{
	struct texture whitepixel;
	struct texture inventory;
	struct texture character;
	struct texture pigzombie;
	struct texture skeleton;
	struct texture crafting;
	struct texture chicken;
	struct texture creeper;
	struct texture terrain;
	struct texture pigman;
	struct texture spider;
	struct texture zombie;
	struct texture clouds;
	struct texture ghast;
	struct texture sheep;
	struct texture slime;
	struct texture squid;
	struct texture icons;
	struct texture water;
	struct texture ascii;
	struct texture logo;
	struct texture pack;
	struct texture moon;
	struct texture cow;
	struct texture pig;
	struct texture gui;
	struct texture sun;
	struct texture bg;
};

struct textures *textures_new(void);
void textures_delete(struct textures *textures);
void texture_bind(struct texture *texture);

#endif
