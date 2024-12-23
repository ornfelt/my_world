#ifndef VOXEL_H
#define VOXEL_H

#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>

struct entities_def;
struct blocks_def;
struct lagometer;
struct textures;
struct graphics;
struct shaders;
struct clouds;
struct screen;
struct biomes;
struct world;
struct gui;

struct voxel
{
	struct entities_def *entities;
	struct blocks_def *blocks;
	struct lagometer *lagometer;
	struct gfx_window *window;
	struct gfx_device *device;
	struct textures *textures;
	struct graphics *graphics;
	struct shaders *shaders;
	struct clouds *clouds;
	struct screen *next_screen;
	struct screen *screen;
	struct biomes *biomes;
	struct world *world;
	struct gui *gui;
	int64_t frametime;
	uint64_t last_frame_update_duration;
	uint64_t last_frame_draw_duration;
	bool disable_tex;
	bool smooth;
	bool ssao;
	bool grabbed;
	bool vsync;
	uint64_t chunk_updates;
	uint64_t fps;
	float delta;
};

extern struct voxel *g_voxel;

uint64_t nanotime(void);
uint32_t npot32(uint32_t v);
float voxel_light_value(int8_t light);

void voxel_grab_cursor(struct voxel *voxel);
void voxel_ungrab_cursor(struct voxel *voxel);

int voxel_asprintf(char **strp, const char *fmt, ...);
int voxel_vasprintf(char **strp, const char *fmt, va_list args);

#endif
