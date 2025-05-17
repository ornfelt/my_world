#ifndef WOW_H
#define WOW_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#ifdef interface
# undef interface
#endif

typedef struct FT_LibraryRec_* FT_Library;
typedef struct FT_MemoryRec_* FT_Memory;

struct wow_mpq_compound;
struct wow_mpq_archive;
struct wow_trs_file;

struct gfx_device;
struct gfx_window;

struct render_target;
struct render_pass;
struct font_model;
struct jks_array;
struct interface;
struct lagometer;
struct gx_frame;
struct jks_hmap;
struct shaders;
struct network;
struct camera;
struct loader;
struct social;
struct object;
struct player;
struct gx_blp;
struct cache;
struct guild;
struct group;
struct cvars;
struct font;
struct dbc;
struct snd;
struct map;
struct wdb;
struct gx;

enum wow_opt
{
	WOW_OPT_DIFFERENT_CAMERAS = (1 << 0),
	WOW_OPT_RENDER_INTERFACE  = (1 << 1),
	WOW_OPT_AABB_OPTIMIZE     = (1 << 2),
	WOW_OPT_RENDER_GUI        = (1 << 3),
	WOW_OPT_ASYNC_CULL        = (1 << 4),
	WOW_OPT_FOCUS_3D          = (1 << 5),
	WOW_OPT_VSYNC             = (1 << 6),
	WOW_OPT_GRAVITY           = (1 << 7),
	WOW_OPT_M2_TRACK_BSEARCH  = (1 << 8),
};

struct dbc_list
{
	struct dbc *area_poi;
	struct dbc *area_table;
	struct dbc *auction_house;
	struct dbc *char_base_info;
	struct dbc *char_hair_geosets;
	struct dbc *char_sections;
	struct dbc *char_start_outfit;
	struct dbc *character_facial_hair_styles;
	struct dbc *chr_classes;
	struct dbc *chr_races;
	struct dbc *creature_display_info;
	struct dbc *creature_display_info_extra;
	struct dbc *creature_model_data;
	struct dbc *game_object_display_info;
	struct dbc *ground_effect_texture;
	struct dbc *ground_effect_doodad;
	struct dbc *helmet_geoset_vis_data;
	struct dbc *item;
	struct dbc *item_class;
	struct dbc *item_display_info;
	struct dbc *item_sub_class;
	struct dbc *map;
	struct dbc *name_gen;
	struct dbc *sound_entries;
	struct dbc *spell;
	struct dbc *spell_icon;
	struct dbc *talent;
	struct dbc *talent_tab;
	struct dbc *taxi_nodes;
	struct dbc *taxi_path;
	struct dbc *world_map_area;
	struct dbc *world_map_continent;
	struct dbc *world_map_overlay;
};

struct post_process
{
	struct render_target *dummy1;
	struct render_target *dummy2;
	struct render_target *shadow;
	struct render_target *msaa;
	struct render_pass *chromaber;
	struct render_pass *sharpen;
	struct render_pass *sobel;
	struct render_pass *bloom;
	struct render_pass *death;
	struct render_pass *ssao;
	struct render_pass *fxaa;
	struct render_pass *glow;
	struct render_pass *fsaa;
	struct render_pass *cel;
};

struct trs_dir
{
	struct jks_hmap *entries; /* char*, char* */
};

struct wow
{
	uint32_t wow_opt;
	struct wow_mpq_compound *mpq_compound;
	struct map *map;
	struct post_process post_process;
	struct jks_array *mpq_archives; /* struct wow_mpq_archive* */
	struct interface *interface;
	struct gfx_device *device;
	struct gfx_window *window;
	struct jks_hmap *objects; /* uint64_t, struct object* */
	struct jks_hmap *trs; /* char*, struct trs_dir */
	struct gx *gx;
	struct shaders *shaders;
	struct camera *cameras[2];
	struct loader *loader;
	struct cache *cache;
	struct wdb *wdb;
	struct gx_frame *frames;
	struct gx_frame *draw_frame;
	struct gx_frame *cull_frame;
	struct camera *frustum_camera;
	struct camera *view_camera;
	struct dbc_list dbc;
	struct lagometer *lagometer;
	struct network *network;
	struct social *social;
	struct player *player;
	struct guild *guild;
	struct group *group;
	struct cvars *cvars;
	struct font_model *font_model_3d;
	struct font *font_3d;
	struct snd *snd;
	struct gx_blp *grey_texture;
	struct gx_blp *black_texture;
	FT_Library ft_lib;
	FT_Memory ft_memory;
	int64_t starttime;
	int64_t frametime;
	int64_t lastframetime;
	uint64_t last_frame_update_duration;
	uint64_t last_frame_events_duration;
	uint64_t last_frame_draw_duration;
	uint64_t last_frame_cull_duration;
	uint64_t last_frame_misc_duration;
	uint32_t render_width;
	uint32_t render_height;
	uint32_t anisotropy;
	float fsaa;
	int current_frame;
	int window_backend;
	int device_backend;
	char locale[5];
	const char *game_path;
};

extern struct wow *g_wow;

int64_t nanotime(void);
uint32_t npot32(uint32_t v);
bool wow_load_compound(struct wow *wow, struct wow_mpq_compound *compound);
bool wow_set_map(struct wow *wow, uint32_t mapid);
void wow_set_player(struct wow *wow, struct player *player);
bool wow_set_object(struct wow *wow, uint64_t guid, struct object *object);
struct object *wow_get_object(struct wow *wow, uint64_t guid);
int wow_asprintf(int memory_type, char **strp, const char *fmt, ...);
int wow_vasprintf(int memory_type, char **strp, const char *fmt, va_list args);

#endif
