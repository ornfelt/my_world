#include "wow.h"

#include "itf/interface.h"

#include "ppe/render_target.h"
#include "ppe/render_pass.h"

#include "net/network.h"
#include "net/packet.h"
#include "net/opcode.h"

#include "game/social.h"
#include "game/group.h"
#include "game/guild.h"

#include "font/model.h"
#include "font/font.h"

#include "obj/update_fields.h"
#include "obj/player.h"

#include "gx/frame.h"
#include "gx/wmo.h"
#include "gx/blp.h"
#include "gx/m2.h"

#include "snd/snd.h"

#include "map/map.h"

#include "performance.h"
#include "lagometer.h"
#include "graphics.h"
#include "shaders.h"
#include "wow_lua.h"
#include "camera.h"
#include "loader.h"
#include "memory.h"
#include "const.h"
#include "cache.h"
#include "cvars.h"
#include "log.h"
#include "dbc.h"
#include "wdb.h"

#include <wow/mpq.h>
#include <wow/trs.h>
#include <wow/blp.h>

#include <gfx/window.h>
#include <gfx/device.h>

#include <jks/array.h>
#include <jks/hmap.h>

#include <ft2build.h>
#include FT_MODULE_H
#include FT_SYSTEM_H

#include <inttypes.h>
#include <getopt.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <math.h>
#include <png.h>

#ifdef _WIN32
# include <windows.h>
double performance_frequency;
#endif

#ifdef interface
# undef interface
#endif

MEMORY_DECL(GENERIC);
MEMORY_DECL(LIBWOW);
MEMORY_DECL(GFX);

struct wow *g_wow = NULL;

static void resize_callback(struct gfx_window *window, struct gfx_resize_event *event);
static void key_down_callback(struct gfx_window *window, struct gfx_key_event *event);
static void key_up_callback(struct gfx_window *window, struct gfx_key_event *event);
static void key_press_callback(struct gfx_window *window, struct gfx_key_event *event);
static void char_callback(struct gfx_window *window, struct gfx_char_event *event);
static void mouse_down_callback(struct gfx_window *window, struct gfx_mouse_event *event);
static void mouse_up_callback(struct gfx_window *window, struct gfx_mouse_event *event);
static void mouse_move_callback(struct gfx_window *window, struct gfx_pointer_event *event);
static void mouse_scroll_callback(struct gfx_window *window, struct gfx_scroll_event *event);
static void error_callback(const char *fmt, ...);

static struct gfx_window *wow_create_window(struct wow *wow, const char *title)
{
	LOG_INFO("creating window");
	struct gfx_window_properties properties;
	gfx_window_properties_init(&properties);
	properties.window_backend = (enum gfx_window_backend)wow->window_backend;
	properties.device_backend = (enum gfx_device_backend)wow->device_backend;
	properties.depth_bits = 24;
	properties.stencil_bits = 8;
	properties.red_bits = 8;
	properties.green_bits = 8;
	properties.blue_bits = 8;
	properties.alpha_bits = 8;
	return gfx_create_window(title, wow->render_width, wow->render_height, &properties);
}

static bool setup_icon(struct wow *wow)
{
	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;
	png_infop end_info = NULL;
	png_bytep *row_pointers = NULL;
	png_byte *image_data = NULL;
	FILE *fp = NULL;
	int rowbytes;
	uint32_t width;
	uint32_t height;
	bool ret = false;
	if (!(fp = fopen("icon.png", "rb")))
		goto end;
	if (!(png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL)))
		goto end;
	if (!(info_ptr = png_create_info_struct(png_ptr)))
		goto end;
	if (!(end_info = png_create_info_struct(png_ptr)))
		goto end;
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		mem_free(MEM_GENERIC, image_data);
		mem_free(MEM_GENERIC, row_pointers);
		fclose(fp);
		return false;
	}
	png_init_io(png_ptr, fp);
	png_read_info(png_ptr, info_ptr);
	png_get_IHDR(png_ptr, info_ptr, &width, &height, NULL, NULL, NULL, NULL, NULL);
	png_read_update_info(png_ptr, info_ptr);
	rowbytes = png_get_rowbytes(png_ptr, info_ptr);
	image_data = mem_malloc(MEM_GENERIC, rowbytes * height);
	row_pointers = mem_malloc(MEM_GENERIC, sizeof(png_bytep) * height);
	if (!image_data || !row_pointers)
		goto end;
	for (uint32_t i = 0; i < height; ++i)
		row_pointers[i] = image_data + i * rowbytes;
	png_read_image(png_ptr, row_pointers);
	gfx_window_set_icon(wow->window, image_data, width, height);
	ret = true;

end:
	png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
	mem_free(MEM_GENERIC, image_data);
	mem_free(MEM_GENERIC, row_pointers);
	if (fp)
		fclose(fp);
	return ret;
}

static bool setup_window(struct wow *wow,
                         enum gfx_window_backend window_backend,
                         enum gfx_device_backend device_backend)
{
	LOG_INFO("creating main window");
	wow->window_backend = window_backend;
	wow->device_backend = device_backend;
	wow->render_width = 1370;
	wow->render_height = 760;
	wow->window = wow_create_window(wow, "WoW");
	if (!wow->window)
	{
		LOG_ERROR("no window created");
		return false;
	}
	wow->window->userdata = wow;
	wow->window->resize_callback = resize_callback;
	wow->window->key_down_callback = key_down_callback;
	wow->window->key_up_callback = key_up_callback;
	wow->window->key_press_callback = key_press_callback;
	wow->window->char_callback = char_callback;
	wow->window->mouse_down_callback = mouse_down_callback;
	wow->window->mouse_up_callback = mouse_up_callback;
	wow->window->mouse_move_callback = mouse_move_callback;
	wow->window->scroll_callback = mouse_scroll_callback;
	gfx_window_make_current(wow->window);
	LOG_INFO("displaying window");
	gfx_window_show(wow->window);
	if (!gfx_create_device(wow->window))
	{
		LOG_ERROR("failed to create device");
		return false;
	}
	wow->device = wow->window->device;
	gfx_window_set_swap_interval(wow->window, -1);
	wow->wow_opt |= WOW_OPT_VSYNC;
	setup_icon(wow);
	return true;
}

static bool get_gfx_device_backend(const char *name,
                                   enum gfx_device_backend *backend)
{
	if (gfx_has_device_backend(GFX_DEVICE_GL4) && !strcmp(name, "gl4"))
	{
		*backend = GFX_DEVICE_GL4;
		return true;
	}
	if (gfx_has_device_backend(GFX_DEVICE_GL3) && !strcmp(name, "gl3"))
	{
		*backend = GFX_DEVICE_GL3;
		return true;
	}
	if (gfx_has_device_backend(GFX_DEVICE_GLES3) && !strcmp(name, "gles3"))
	{
		*backend = GFX_DEVICE_GLES3;
		return true;
	}
	if (gfx_has_device_backend(GFX_DEVICE_D3D11) && !strcmp(name, "d3d11"))
	{
		*backend = GFX_DEVICE_D3D11;
		return true;
	}
	if (gfx_has_device_backend(GFX_DEVICE_VK) && !strcmp(name, "vk"))
	{
		*backend = GFX_DEVICE_VK;
		return true;
	}
	if (gfx_has_device_backend(GFX_DEVICE_D3D9) && !strcmp(name, "d3d9"))
	{
		*backend = GFX_DEVICE_D3D9;
		return true;
	}
	return false;
}

static const char *get_default_gfx_window_backend(void)
{
	if (gfx_has_window_backend(GFX_WINDOW_X11))
		return "x11";
	if (gfx_has_window_backend(GFX_WINDOW_WIN32))
		return "win32";
	if (gfx_has_window_backend(GFX_WINDOW_WAYLAND))
		return "wayland";
	if (gfx_has_window_backend(GFX_WINDOW_GLFW))
		return "glfw";
	if (gfx_has_window_backend(GFX_WINDOW_SDL))
		return "sdl";
	return NULL;
}

static bool get_gfx_window_backend(const char *name,
                                   enum gfx_window_backend *backend)
{
	if (gfx_has_window_backend(GFX_WINDOW_X11) && !strcmp(name, "x11"))
	{
		*backend = GFX_WINDOW_X11;
		return true;
	}
	if (gfx_has_window_backend(GFX_WINDOW_WIN32) && !strcmp(name, "win32"))
	{
		*backend = GFX_WINDOW_WIN32;
		return true;
	}
	if (gfx_has_window_backend(GFX_WINDOW_WAYLAND) && !strcmp(name, "wayland"))
	{
		*backend = GFX_WINDOW_WAYLAND;
		return true;
	}
	if (gfx_has_window_backend(GFX_WINDOW_GLFW) && !strcmp(name, "glfw"))
	{
		*backend = GFX_WINDOW_GLFW;
		return true;
	}
	if (gfx_has_window_backend(GFX_WINDOW_GLFW) && !strcmp(name, "sdl"))
	{
		*backend = GFX_WINDOW_SDL;
		return true;
	}
	return false;
}

static bool setup_gfx(struct wow *wow, const char *windowing, const char *renderer)
{
	enum gfx_window_backend window_backend;
	if (!get_gfx_window_backend(windowing, &window_backend))
	{
		LOG_ERROR("unknown window backend: %s", windowing);
		return false;
	}
	enum gfx_device_backend device_backend;
	if (!get_gfx_device_backend(renderer, &device_backend))
	{
		LOG_ERROR("unknown device backend: %s", renderer);
		return false;
	}
	if (!setup_window(wow, window_backend, device_backend))
		return false;
	return true;
}

static bool setup_frames(struct wow *wow)
{
	wow->frames = mem_malloc(MEM_GENERIC, sizeof(*wow->frames) * RENDER_FRAMES_COUNT);
	if (!wow->frames)
	{
		LOG_ERROR("failed to allocate render frames");
		return false;
	}
	for (size_t i = 0; i < RENDER_FRAMES_COUNT; ++i)
		gx_frame_init(&wow->frames[i], i);
	return true;
}

static bool setup_loader(struct wow *wow)
{
	wow->loader = loader_new();
	if (!wow->loader)
	{
		LOG_ERROR("failed to load async loaders");
		return false;
	}
	return true;
}

static bool setup_snd(struct wow *wow)
{
	int pa_err = Pa_Initialize();
	if (pa_err)
	{
		LOG_ERROR("failed to init snd");
		return EXIT_FAILURE;
	}
	wow->snd = snd_new();
	if (!wow->snd)
	{
		LOG_ERROR("can't create sound system");
		return false;
	}
	return true;
}

static bool setup_map(struct wow *wow)
{
	LOG_INFO("loading map");
	wow->map = map_new();
	if (!wow->map)
	{
		LOG_ERROR("failed to setup map");
		return false;
	}
	return true;
}

bool wow_set_map(struct wow *wow, uint32_t mapid)
{
	if (!wow->map)
		return true;
	if (!map_setid(wow->map, mapid))
	{
		LOG_ERROR("failed to set map");
		return false;
	}
	return true;
}

static bool setup_cache(struct wow *wow)
{
	wow->cache = cache_new();
	if (!wow->cache)
	{
		LOG_ERROR("failed to setup cache");
		return false;
	}
	return true;
}

static void archive_delete(void *ptr)
{
	wow_mpq_archive_delete(*(struct wow_mpq_archive**)ptr);
}

static bool setup_game_files(struct wow *wow)
{
	wow->mpq_archives = mem_malloc(MEM_GENERIC, sizeof(*wow->mpq_archives));
	if (!wow->mpq_archives)
	{
		LOG_ERROR("mpq archives allocation failed");
		return EXIT_FAILURE;
	}
	jks_array_init(wow->mpq_archives, sizeof(struct wow_mpq_archive*), archive_delete, &jks_array_memory_fn_GENERIC);
	char files[14][256];
	snprintf(files[0] , sizeof(files[0]) , "patch-5.MPQ");
	snprintf(files[1] , sizeof(files[1]) , "patch-3.MPQ");
	snprintf(files[2] , sizeof(files[2]) , "patch-2.MPQ");
	snprintf(files[3] , sizeof(files[3]) , "patch.MPQ");
	snprintf(files[4] , sizeof(files[4]) , "%s/patch-%s-2.MPQ", wow->locale, wow->locale);
	snprintf(files[5] , sizeof(files[5]) , "%s/patch-%s.MPQ", wow->locale, wow->locale);
	snprintf(files[6] , sizeof(files[6]) , "expansion.MPQ");
	snprintf(files[7] , sizeof(files[7]) , "common.MPQ");
	snprintf(files[8] , sizeof(files[8]) , "%s/base-%s.MPQ", wow->locale, wow->locale);
	snprintf(files[9] , sizeof(files[9]) , "%s/backup-%s.MPQ", wow->locale, wow->locale);
	snprintf(files[10], sizeof(files[10]), "%s/expansion-locale-%s.MPQ", wow->locale, wow->locale);
	snprintf(files[11], sizeof(files[11]), "%s/locale-%s.MPQ", wow->locale, wow->locale);
	snprintf(files[12], sizeof(files[12]), "%s/expansion-speech-%s.MPQ", wow->locale, wow->locale);
	snprintf(files[13], sizeof(files[13]), "%s/speech-%s.MPQ", wow->locale, wow->locale);
	/*char files[8][256];
	snprintf(files[0], sizeof(files[0]), "dbc.MPQ");
	snprintf(files[1], sizeof(files[1]), "fonts.MPQ");
	snprintf(files[2], sizeof(files[2]), "interface.MPQ");
	snprintf(files[3], sizeof(files[3]), "misc.MPQ");
	snprintf(files[4], sizeof(files[4]), "model.MPQ");
	snprintf(files[5], sizeof(files[5]), "sound.MPQ");
	snprintf(files[6], sizeof(files[6]), "speech.MPQ");
	snprintf(files[7], sizeof(files[7]), "texture.MPQ");*/
	for (size_t i = 0; i < sizeof(files) / sizeof(*files); ++i)
	{
		char name[512];
		snprintf(name, sizeof(name), "%s/Data/%s", wow->game_path, files[i]);
		struct wow_mpq_archive *archive = wow_mpq_archive_new(name);
		if (!archive)
		{
			LOG_ERROR("failed to open archive \"%s\"", name);
			continue;
		}
		if (!jks_array_push_back(wow->mpq_archives, &archive))
		{
			LOG_ERROR("failed to add archive to list");
			continue;
		}
	}
	LOG_INFO("loading MPQs");
	wow->mpq_compound = wow_mpq_compound_new();
	if (!wow->mpq_compound)
	{
		LOG_ERROR("failed to get compound");
		return false;
	}
	if (!wow_load_compound(wow, wow->mpq_compound))
	{
		LOG_ERROR("failed to load main mpq compound");
		wow_mpq_compound_delete(wow->mpq_compound);
		wow->mpq_compound = NULL;
		return false;
	}
	return true;
}

static bool setup_interface(struct wow *wow)
{
	wow->interface = interface_new();
	if (!wow->interface)
	{
		LOG_ERROR("failed to setup interface");
		return false;
	}
	{
		struct gfx_resize_event event;
		event.width = wow->window->width;
		event.height = wow->window->height;
		interface_on_window_resized(wow->interface, &event);
	}
	return true;
}

static bool setup_cameras(struct wow *wow)
{
	wow->cameras[0] = camera_new();
	if (!wow->cameras[0])
	{
		LOG_ERROR("failed to setup main camera");
		return false;
	}
	wow->cameras[0]->worldobj = (struct worldobj*)wow->player;
	wow->cameras[1] = camera_new();
	if (!wow->cameras[1])
	{
		LOG_ERROR("failed to setup second camera");
		return false;
	}
	wow->view_camera = wow->cameras[0];
	wow->frustum_camera = wow->cameras[0];
	return true;
}

static bool setup_player(struct wow *wow)
{
	wow->player = player_new(50);
	if (!wow->player)
	{
		LOG_ERROR("player setup failed");
		return false;
	}
	object_fields_set_u32(&((struct object*)wow->player)->fields, UNIT_FIELD_BYTES_0, (CLASS_WARRIOR << 8) | (RACE_HUMAN << 0) | (1 << 16));
	object_fields_set_u32(&((struct object*)wow->player)->fields, UNIT_FIELD_DISPLAYID, 50);
	object_fields_set_u32(&((struct object*)wow->player)->fields, PLAYER_FIELD_VISIBLE_ITEM_1_0 + 16 * EQUIPMENT_SLOT_HEAD,      30972);
	object_fields_set_u32(&((struct object*)wow->player)->fields, PLAYER_FIELD_VISIBLE_ITEM_1_0 + 16 * EQUIPMENT_SLOT_SHOULDERS, 30979);
	object_fields_set_u32(&((struct object*)wow->player)->fields, PLAYER_FIELD_VISIBLE_ITEM_1_0 + 16 * EQUIPMENT_SLOT_CHEST,     30975);
	object_fields_set_u32(&((struct object*)wow->player)->fields, PLAYER_FIELD_VISIBLE_ITEM_1_0 + 16 * EQUIPMENT_SLOT_LEGS,      30978);
	object_fields_set_u32(&((struct object*)wow->player)->fields, PLAYER_FIELD_VISIBLE_ITEM_1_0 + 16 * EQUIPMENT_SLOT_FEET,      32345);
	object_fields_set_u32(&((struct object*)wow->player)->fields, PLAYER_FIELD_VISIBLE_ITEM_1_0 + 16 * EQUIPMENT_SLOT_HANDS,     30969);
#if 1
	object_fields_set_u32(&((struct object*)wow->player)->fields, UNIT_FIELD_MOUNTDISPLAYID, 17890);
#endif
#if 0
	object_fields_set_u32(&((struct object*)wow->player)->fields, UNIT_FIELD_MOUNTDISPLAYID, 20932);
#endif
#if 1
	((struct worldobj*)wow->player)->movement_data.flags |= MOVEFLAG_CAN_FLY;
#endif
	return true;
}

static bool setup_network(struct wow *wow)
{
	wow->network = network_new();
	if (!wow->network)
	{
		LOG_ERROR("failed to setup network");
		return false;
	}
	return true;
}

static bool setup_graphics(struct wow *wow)
{
	wow->graphics = graphics_new();
	if (!wow->graphics)
	{
		LOG_ERROR("failed to setup graphics");
		return false;
	}
	return true;
}

static bool setup_shaders(struct wow *wow)
{
	wow->shaders = shaders_new();
	if (!wow->shaders)
	{
		LOG_ERROR("failed to setup shaders");
		return false;
	}
	return true;
}

static bool setup_cvars(struct wow *wow)
{
	wow->cvars = cvars_new();
	if (!wow->cvars)
	{
		LOG_ERROR("failed to setup cvars");
		return false;
	}
	return true;
}

static bool setup_game(struct wow *wow)
{
	wow->social = social_new();
	if (!wow->social)
	{
		LOG_ERROR("failed to create social");
		return false;
	}
	wow->group = group_new();
	if (!wow->group)
	{
		LOG_ERROR("failed to create group");
		return false;
	}
	wow->guild = guild_new();
	if (!wow->guild)
	{
		LOG_ERROR("failed to create guild");
		return false;
	}
	return true;
}

static bool setup_objects(struct wow *wow)
{
	wow->objects = mem_malloc(MEM_GENERIC, sizeof(*wow->objects));
	if (!wow->objects)
	{
		LOG_ERROR("malloc failed");
		return false;
	}
	jks_hmap_init(wow->objects, sizeof(struct object*), NULL, jks_hmap_hash_u64, jks_hmap_cmp_u64, &jks_hmap_memory_fn_GENERIC);
	return true;
}

static bool setup_wdb(struct wow *wow)
{
	wow->wdb = wdb_new();
	if (!wow->wdb)
	{
		LOG_ERROR("failed to create wdb");
		return false;
	}
	wdb_load(g_wow->wdb);
	return true;
}

static bool setup_render_passes(struct wow *wow)
{
	LOG_INFO("loading MSAA render target");
	wow->post_process.msaa = render_target_new(8);
	if (!wow->post_process.msaa)
	{
		LOG_ERROR("failed to load MSAA render target");
		return false;
	}
	LOG_INFO("loading dummy render target");
	wow->post_process.dummy1 = render_target_new(0);
	if (!wow->post_process.dummy1)
	{
		LOG_ERROR("failed to load dummy render target 1");
		return false;
	}
	wow->post_process.dummy2 = render_target_new(0);
	if (!wow->post_process.dummy2)
	{
		LOG_ERROR("failed to load dummy render target 2");
		return false;
	}
	LOG_INFO("loading chromaber render pass");
	wow->post_process.chromaber = chromaber_render_pass_new();
	if (!wow->post_process.chromaber)
	{
		LOG_ERROR("failed to load chromaber render pass");
		return false;
	}
#if 0
	wow->post_process.chromaber->enabled = true;
#endif
	LOG_INFO("loading sharpen render pass");
	wow->post_process.sharpen = sharpen_render_pass_new();
	if (!wow->post_process.sharpen)
	{
		LOG_ERROR("failed to load sharpen render pass");
		return false;
	}
#if 0
	wow->post_process.sharpen->enabled = true;
#endif
	LOG_INFO("loading glow render pass");
	wow->post_process.glow = glow_render_pass_new();
	if (!wow->post_process.glow)
	{
		LOG_ERROR("failed to load glow render pass");
		return false;
	}
#if 0
	wow->post_process.glow->enabled = true;
#endif
	LOG_INFO("loading sobel render pass");
	wow->post_process.sobel = sobel_render_pass_new();
	if (!wow->post_process.sobel)
	{
		LOG_ERROR("failed to load sobel render pass");
		return false;
	}
#if 0
	wow->post_process.ssao->enabled = true;
#endif
	LOG_INFO("loading SSAO render pass");
	wow->post_process.ssao = ssao_render_pass_new();
	if (!wow->post_process.ssao)
	{
		LOG_ERROR("failed to load SSAO render pass");
		return false;
	}
#if 0
	wow->post_process.ssao->enabled = true;
#endif
	LOG_INFO("loading FXAA render pass");
	wow->post_process.fxaa = fxaa_render_pass_new();
	if (!wow->post_process.fxaa)
	{
		LOG_ERROR("failed to load FXAA render pass");
		return false;
	}
#if 0
	wow->post_process.fxaa->enabled = true;
#endif
	LOG_INFO("loading FSAA render pass");
	wow->post_process.fsaa = fsaa_render_pass_new();
	if (!wow->post_process.fsaa)
	{
		LOG_ERROR("failed to load FSAA render pass");
		return false;
	}
#if 0
	wow->post_process.fsaa->enabled = true;
#endif
	LOG_INFO("loading cel render pass");
	wow->post_process.cel = cel_render_pass_new();
	if (!wow->post_process.cel)
	{
		LOG_ERROR("failed to load cel render pass");
		return false;
	}
#if 0
	wow->post_process.cel->enabled = true;
#endif
	LOG_INFO("loading bloom render pass");
	wow->post_process.bloom = bloom_render_pass_new();
	if (!wow->post_process.bloom)
	{
		LOG_ERROR("failed to load bloom render pass");
		return false;
	}
#if 0
	wow->post_process.bloom->enabled = true;
#endif
	return true;
}

static void cleanup_render_passes(struct wow *wow)
{
	render_target_delete(wow->post_process.dummy1);
	render_target_delete(wow->post_process.dummy2);
	render_target_delete(wow->post_process.msaa);
	render_pass_delete(wow->post_process.sharpen);
	render_pass_delete(wow->post_process.bloom);
	render_pass_delete(wow->post_process.sobel);
	render_pass_delete(wow->post_process.ssao);
	render_pass_delete(wow->post_process.glow);
	render_pass_delete(wow->post_process.fxaa);
	render_pass_delete(wow->post_process.fsaa);
	render_pass_delete(wow->post_process.cel);
}

static void dirty_render_passes(struct wow *wow)
{
	if (wow->post_process.ssao)
		wow->post_process.ssao->dirty_size = true;
	if (wow->post_process.bloom)
		wow->post_process.bloom->dirty_size = true;
	if (wow->post_process.msaa)
		wow->post_process.msaa->dirty_size = true;
	if (wow->post_process.dummy1)
		wow->post_process.dummy1->dirty_size = true;
	if (wow->post_process.dummy2)
		wow->post_process.dummy2->dirty_size = true;
}

static bool setup_dbc(struct wow *wow)
{
#define DBC_LOAD(name, var) \
	if (!cache_ref_dbc(wow->cache, "DBFilesClient\\" name, &wow->dbc.var)) \
	{ \
		LOG_ERROR("can't find %s", name); \
		return false; \
	}

#define DBC_LOAD_INDEXED(name, var, index, str_index) \
	do \
	{ \
		DBC_LOAD(name, var); \
		if (!dbc_set_index(wow->dbc.var, index, str_index)) \
		{ \
			LOG_ERROR("failed to set index %d to %s", index, name); \
			return false; \
		} \
	} while (0)

	DBC_LOAD_INDEXED("AreaPOI.dbc", area_poi, 0, false);
	DBC_LOAD_INDEXED("AreaTable.dbc", area_table, 0, false);
	DBC_LOAD_INDEXED("AuctionHouse.dbc", auction_house, 0, false);
	DBC_LOAD("CharBaseInfo.dbc", char_base_info);
	DBC_LOAD_INDEXED("CharHairGeosets.dbc", char_hair_geosets, 0, false);
	DBC_LOAD("CharSections.dbc", char_sections);
	DBC_LOAD_INDEXED("CharStartOutfit.dbc", char_start_outfit, 0, false);
	DBC_LOAD("CharacterFacialHairStyles.dbc", character_facial_hair_styles);
	DBC_LOAD_INDEXED("ChrRaces.dbc", chr_races, 0, false);
	DBC_LOAD_INDEXED("ChrClasses.dbc", chr_classes, 0, false);
	DBC_LOAD_INDEXED("CreatureDisplayInfo.dbc", creature_display_info, 0, false);
	DBC_LOAD_INDEXED("CreatureDisplayInfoExtra.dbc", creature_display_info_extra, 0, false);
	DBC_LOAD_INDEXED("CreatureModelData.dbc", creature_model_data, 0, false);
	DBC_LOAD_INDEXED("GameObjectDisplayInfo.dbc", game_object_display_info, 0, false);
	DBC_LOAD_INDEXED("GroundEffectTexture.dbc", ground_effect_texture, 0, false);
	DBC_LOAD_INDEXED("GroundEffectDoodad.dbc", ground_effect_doodad, 4, false);
	DBC_LOAD_INDEXED("HelmetGeosetVisData.dbc", helmet_geoset_vis_data, 0, false);
	DBC_LOAD_INDEXED("Item.dbc", item, 0, false);
	DBC_LOAD_INDEXED("ItemClass.dbc", item_class, 0, false);
	DBC_LOAD_INDEXED("ItemDisplayInfo.dbc", item_display_info, 0, false);
	DBC_LOAD_INDEXED("ItemSubClass.dbc", item_sub_class, 0, false);
	DBC_LOAD_INDEXED("Map.dbc", map, 0, false);
	DBC_LOAD_INDEXED("NameGen.dbc", name_gen, 0, false);
	DBC_LOAD_INDEXED("SoundEntries.dbc", sound_entries, 8, true);
	DBC_LOAD_INDEXED("Spell.dbc", spell, 0, false);
	DBC_LOAD_INDEXED("SpellIcon.dbc", spell_icon, 0, false);
	DBC_LOAD_INDEXED("Talent.dbc", talent, 0, false);
	DBC_LOAD_INDEXED("TalentTab.dbc", talent_tab, 0, false);
	DBC_LOAD_INDEXED("TaxiNodes.dbc", taxi_nodes, 0, false);
	DBC_LOAD_INDEXED("TaxiPath.dbc", taxi_path, 0, false);
	DBC_LOAD_INDEXED("WorldMapArea.dbc", world_map_area, 0, false);
	DBC_LOAD_INDEXED("WorldMapContinent.dbc", world_map_continent, 0, false);
	DBC_LOAD("WorldMapOverlay.dbc", world_map_overlay);
	return true;

#undef DBC_LOAD
#undef DBC_LOAD_INDEXED
}

static void unload_dbc(struct wow *wow)
{
#define DBC_UNLOAD(name) \
	do \
	{ \
		dbc_free(wow->dbc.name); \
	} while (0)

	DBC_UNLOAD(area_poi);
	DBC_UNLOAD(area_table);
	DBC_UNLOAD(auction_house);
	DBC_UNLOAD(char_base_info);
	DBC_UNLOAD(char_sections);
	DBC_UNLOAD(char_hair_geosets);
	DBC_UNLOAD(char_start_outfit);
	DBC_UNLOAD(character_facial_hair_styles);
	DBC_UNLOAD(chr_races);
	DBC_UNLOAD(chr_classes);
	DBC_UNLOAD(creature_display_info);
	DBC_UNLOAD(creature_model_data);
	DBC_UNLOAD(creature_display_info_extra);
	DBC_UNLOAD(game_object_display_info);
	DBC_UNLOAD(ground_effect_texture);
	DBC_UNLOAD(ground_effect_doodad);
	DBC_UNLOAD(helmet_geoset_vis_data);
	DBC_UNLOAD(item);
	DBC_UNLOAD(item_class);
	DBC_UNLOAD(item_display_info);
	DBC_UNLOAD(item_sub_class);
	DBC_UNLOAD(map);
	DBC_UNLOAD(name_gen);
	DBC_UNLOAD(sound_entries);
	DBC_UNLOAD(spell);
	DBC_UNLOAD(spell_icon);
	DBC_UNLOAD(talent);
	DBC_UNLOAD(talent_tab);
	DBC_UNLOAD(taxi_nodes);
	DBC_UNLOAD(taxi_path);
	DBC_UNLOAD(world_map_area);
	DBC_UNLOAD(world_map_continent);
	DBC_UNLOAD(world_map_overlay);

#undef DBC_UNLOAD
}

static bool setup_gui(struct wow *wow)
{
	LOG_INFO("loading GUI");
	wow->lagometer = lagometer_new();
	if (!wow->lagometer)
	{
		LOG_ERROR("failed to create lagometer");
		return false;
	}
	return true;
}

static void trs_entry_dtr(jks_hmap_key_t key, void *value)
{
	mem_free(MEM_GENERIC, key.ptr);
	mem_free(MEM_GENERIC, *(char**)value);
}

static void trs_dir_dtr(jks_hmap_key_t key, void *value)
{
	mem_free(MEM_GENERIC, key.ptr);
	struct trs_dir *dir = value;
	if (!dir->entries)
		return;
	jks_hmap_destroy(dir->entries);
	free(dir->entries);
}

static bool setup_trs(struct wow *wow)
{
	/* XXX should load only current map entries */
	struct wow_mpq_file *mpq = wow_mpq_get_file(wow->mpq_compound, "TEXTURES\\MINIMAP\\MD5TRANSLATE.TRS");
	if (!mpq)
	{
		LOG_ERROR("failed to get TRS mpq file");
		return false;
	}
	struct wow_trs_file *trs = wow_trs_file_new(mpq);
	if (!trs)
	{
		LOG_ERROR("failed to parse TRS file");
		return false;
	}
	wow->trs = mem_malloc(MEM_GENERIC, sizeof(*wow->trs));
	if (!wow->trs)
	{
		LOG_ERROR("allocation failed");
		return false;
	}
	jks_hmap_init(wow->trs, sizeof(struct trs_dir), trs_dir_dtr, jks_hmap_hash_str, jks_hmap_cmp_str, &jks_hmap_memory_fn_GENERIC);
	for (size_t i = 0; i < trs->dirs_nb; ++i)
	{
		struct trs_dir dir;
		char keyname[256];
		snprintf(keyname, sizeof(keyname), "%s", trs->dirs[i].name);
		wow_mpq_normalize_mpq_fn(keyname, sizeof(keyname));
		char *key = mem_strdup(MEM_GENERIC, keyname);
		if (!key)
		{
			LOG_ERROR("allocation failed");
			goto err;
		}
		dir.entries = mem_malloc(MEM_GENERIC, sizeof(*dir.entries));
		if (!dir.entries)
		{
			LOG_ERROR("allocation failed");
			mem_free(MEM_GENERIC, key);
			goto err;
		}
		jks_hmap_init(dir.entries, sizeof(char*), trs_entry_dtr, jks_hmap_hash_str, jks_hmap_cmp_str, &jks_hmap_memory_fn_GENERIC);
		for (size_t j = 0; j < trs->dirs[i].entries_nb; ++j)
		{
			char name[256];
			char hash[256];
			snprintf(name, sizeof(name), "%s", trs->dirs[i].entries[j].name);
			snprintf(hash, sizeof(hash), "%s", trs->dirs[i].entries[j].hash);
			wow_mpq_normalize_mpq_fn(name, sizeof(name));
			wow_mpq_normalize_mpq_fn(hash, sizeof(hash));
			char *namedup = mem_strdup(MEM_GENERIC, name);
			char *hashdup = mem_strdup(MEM_GENERIC, hash);
			if (!namedup || !hashdup)
			{
				LOG_ERROR("allocation failed");
				mem_free(MEM_GENERIC, namedup);
				mem_free(MEM_GENERIC, hashdup);
				jks_hmap_destroy(dir.entries);
				mem_free(MEM_GENERIC, dir.entries);
				goto err;
			}
			if (!jks_hmap_set(dir.entries, JKS_HMAP_KEY_STR(namedup), &hashdup))
			{
				LOG_ERROR("failed to set trs entry");
				mem_free(MEM_GENERIC, namedup);
				mem_free(MEM_GENERIC, hashdup);
				jks_hmap_destroy(dir.entries);
				mem_free(MEM_GENERIC, dir.entries);
				goto err;
			}
		}
		if (!jks_hmap_set(wow->trs, JKS_HMAP_KEY_STR((char*)key), &dir))
		{
			LOG_ERROR("failed to set trs dir");
			mem_free(MEM_GENERIC, key);
			goto err;
		}
	}
	return true;

err:
	jks_hmap_destroy(wow->trs);
	mem_free(MEM_GENERIC, wow->trs);
	return false;
}

static bool load_placeholder_texture(struct wow *wow, const char *filename, struct gx_blp **blp)
{
	char *filename_dup = mem_strdup(MEM_GENERIC, filename);
	if (!filename_dup)
	{
		LOG_ERROR("failed to allocate blp filename");
		return false;
	}
	*blp = gx_blp_from_filename(filename_dup);
	if (!*blp)
	{
		LOG_ERROR("failed to create placeholder texture");
		return false;
	}
	struct wow_mpq_file *file = wow_mpq_get_file(wow->mpq_compound, filename);
	if (!file)
	{
		LOG_ERROR("failed to get %s", filename);
		return false;
	}
	struct wow_blp_file *blp_file = wow_blp_file_new(file);
	wow_mpq_file_delete(file);
	if (!blp_file)
	{
		LOG_ERROR("failed to parse %s", filename);
		return false;
	}
	if (!gx_blp_load(*blp, blp_file))
	{
		wow_blp_file_delete(blp_file);
		LOG_ERROR("failed to load placeholder texture");
		return false;
	}
	wow_blp_file_delete(blp_file);
	return true;
}

static bool load_placeholder_textures(struct wow *wow)
{
	if (!load_placeholder_texture(wow, "TILESET\\GENERIC\\GREY.BLP", &wow->grey_texture))
		return false;
	if (!load_placeholder_texture(wow, "TILESET\\GENERIC\\BLACK.BLP", &wow->black_texture))
		return false;
	return true;
}

static const char *get_default_gfx_device_backend(void)
{
	if (gfx_has_device_backend(GFX_DEVICE_GL4))
		return "gl4";
	if (gfx_has_device_backend(GFX_DEVICE_GL3))
		return "gl3";
	if (gfx_has_device_backend(GFX_DEVICE_VK))
		return "vk";
	if (gfx_has_device_backend(GFX_DEVICE_D3D11))
		return "d3d11";
	if (gfx_has_device_backend(GFX_DEVICE_D3D9))
		return "d3d9";
	if (gfx_has_device_backend(GFX_DEVICE_GLES3))
		return "gles3";
	return NULL;
}

static int get_frame_id(struct wow *wow, int offset)
{
	int id = wow->current_frame + offset;
	if (id < 0)
		id += RENDER_FRAMES_COUNT;
	return id % RENDER_FRAMES_COUNT;
}

static struct gx_frame *get_frame(struct wow *wow, int offset)
{
	return &wow->frames[get_frame_id(wow, offset)];
}

static void increment_frames(struct wow *wow)
{
	wow->current_frame++;
	wow->draw_frame = get_frame(wow, 0);
	wow->cull_frame = get_frame(wow, 1);
}

static void loop(struct wow *wow)
{
	gfx_depth_stencil_state_t depth_stencil_state = GFX_DEPTH_STENCIL_STATE_INIT();
	gfx_rasterizer_state_t rasterizer_state = GFX_RASTERIZER_STATE_INIT();
	gfx_pipeline_state_t pipeline_state = GFX_PIPELINE_STATE_INIT();
	gfx_input_layout_t input_layout = GFX_INPUT_LAYOUT_INIT();
	gfx_blend_state_t blend_state = GFX_BLEND_STATE_INIT();

	gfx_create_depth_stencil_state(wow->device, &depth_stencil_state, true, true, GFX_CMP_LEQUAL, true, -1, GFX_CMP_NOTEQUAL, 1, -1, GFX_STENCIL_KEEP, GFX_STENCIL_KEEP, GFX_STENCIL_KEEP);
	gfx_create_rasterizer_state(wow->device, &rasterizer_state, GFX_FILL_SOLID, GFX_CULL_NONE, GFX_FRONT_CCW, false);
	gfx_create_blend_state(wow->device, &blend_state, true, GFX_BLEND_SRC_ALPHA, GFX_BLEND_ONE_MINUS_SRC_ALPHA, GFX_BLEND_SRC_ALPHA, GFX_BLEND_ONE_MINUS_SRC_ALPHA, GFX_EQUATION_ADD, GFX_EQUATION_ADD, GFX_COLOR_MASK_ALL);
	gfx_create_input_layout(wow->device, &input_layout, NULL, 0, &wow->shaders->gui);
	gfx_create_pipeline_state(wow->device, &pipeline_state, &wow->shaders->gui, &rasterizer_state, &depth_stencil_state, &blend_state, &input_layout, GFX_PRIMITIVE_TRIANGLES);
	int64_t fps = 0;
	int64_t last_fps = nanotime();
	while (!wow->window->close_requested)
	{
#if 0
		LOG_INFO("x: %f, y: %f, z: %f", wow->view_camera->pos.x, wow->view_camera->pos.y, wow->view_camera->pos.z);
#endif
		uint64_t started, ended;
		started = nanotime();
		wow->lastframetime = wow->frametime;
		wow->frametime = nanotime();
		if (wow->frametime - last_fps >= 1000000000)
		{
#ifdef WITH_MEMORY
			mem_dump();
#endif
#ifdef WITH_PERFORMANCE
			performance_dump();
			performance_reset();
#endif
			wdb_save(g_wow->wdb);
			last_fps = nanotime();
			char title[64];
			snprintf(title, sizeof(title), "%" PRId64 "fps", fps);
			gfx_window_set_title(wow->window, title);
			fps = 0;
		}
		increment_frames(wow);
		wow->cull_frame->time = wow->frametime;
		wow->cull_frame->dt = wow->frametime - wow->lastframetime;
		loader_start_cull(wow->loader);
		if (wow->map && (!wow->interface || !wow->interface->is_gluescreen))
			map_render(wow->map, wow->draw_frame);
		gfx_bind_render_target(wow->device, NULL);
		gfx_set_viewport(wow->device, 0, 0, wow->render_width, wow->render_height);
		gfx_set_scissor(wow->device, 0, 0, wow->render_width, wow->render_height);
		gfx_bind_pipeline_state(wow->device, &pipeline_state);
		gfx_clear_depth_stencil(wow->device, NULL, 1, 0);
		if (!wow->map || (wow->interface && wow->interface->is_gluescreen))
			gfx_clear_color(wow->device, NULL, GFX_RENDERTARGET_ATTACHMENT_COLOR0, (struct vec4f){0, 0, 0, 1});
		if ((wow->wow_opt & WOW_OPT_RENDER_INTERFACE) && wow->interface)
			interface_render(wow->interface);
		if (wow->wow_opt & WOW_OPT_RENDER_GUI)
			lagometer_draw(wow->lagometer);
		ended = nanotime();
		wow->last_frame_draw_duration = ended - started;

		started = nanotime();
		gfx_window_swap_buffers(wow->window);
		ended = nanotime();
		wow->last_frame_update_duration = ended - started;

		started = nanotime();
		loader_wait_cull(wow->loader);
		ended = nanotime();
		wow->last_frame_cull_duration = ended - started;

		started = nanotime();
		if ((wow->wow_opt & WOW_OPT_RENDER_INTERFACE) && wow->interface)
			interface_lock(wow->interface);
		net_tick(wow->network);
		if ((wow->wow_opt & WOW_OPT_RENDER_INTERFACE) && wow->interface)
			interface_unlock(wow->interface);
		gfx_window_poll_events(wow->window);
		ended = nanotime();
		wow->last_frame_events_duration = ended - started;

		started = nanotime();
		loader_tick(wow->loader);
		gfx_device_tick(wow->device);
		ended = nanotime();
		wow->last_frame_misc_duration = ended - started;

		++fps;
	}
	gfx_delete_depth_stencil_state(wow->device, &depth_stencil_state);
	gfx_delete_rasterizer_state(wow->device, &rasterizer_state);
	gfx_delete_blend_state(wow->device, &blend_state);
	gfx_delete_input_layout(wow->device, &input_layout);
	gfx_delete_pipeline_state(wow->device, &pipeline_state);
}

void update_world_map_data(int32_t continent_id, int32_t zone_id, uint32_t zone_idx);

/*
 * 0: Azeroth
 * 1: Kalimdor
 * 13: Test
 * 30: Alterac
 * 33: Ombrecroc
 * 36: deadmines
 * 37: PVPZone02
 * 43: WailingCaverns
 * 44: Monastery
 * 47: RazorfenKraul
 * 48: Blackfathom
 * 70: Uldaman
 * 90: Gnomeragon
 * 109: SunkenTemple
 * 129: RazorfenDowns
 * 169: EmeraldDream
 * 189: MonasteryInstances
 * 209: TanarisInstance
 * 229: BlackRockSpire
 * 230: BlackrockDepths
 * 249: Onyxia
 * 269: CavernsOfTime
 * 289; SchoolofNecromancy
 * 309: Zul'Gurub
 * 329: Stratholme
 * 349: Mauradon
 * 369: DeeprunTram
 * 389: OrgrimmarInstance
 * 409: MoltenCore
 * 429: DireMaul
 * 449: AlliancePVPBarracks
 * 450: HordePVPBarracks
 * 451: dev
 * 469: BlackwingLair
 * 489: Warsong
 * 509: AhnQiraj
 * 529: Arathi
 * 530: Outland
 * 531: AhnQirajTemple
 * 532: Karazahn
 * 533: Stratholme Raid
 * 534: HyjalPast
 * 540: HellfireMilitary
 * 542: HellfireDemon
 * 543: HellfireRampart
 * 544: HellfireRaid
 * 545: CoilfangPumping
 * 546: CoilfangMarsh
 * 547: CoilfangDraenei
 * 548: CoilfangRaid
 * 550: TempestKeepRaid
 * 552: TempestKeepArcane
 * 553: TempestKeepAtrium
 * 554: TempestKeepFactory
 * 555: AuchindounShadow
 * 556: AuchindounDemon
 * 557: AuchindounEthereal
 * 558: AuchindounDraenei
 * 559: Nagrand PVP
 * 560: HillsbradPast
 * 562: bladesedgearena
 * 564: BlackTemple
 * 565: GruulsLair
 * 566: Netherstorm BG
 * 568: ZulAman
 * 572: PVPLordaeron
 * 580: SunwellPlateau
 * 585: Sunwell5ManFix
 * 598: Sunwell5Man
 */

static int wow_main(struct wow *wow, int argc, char **argv)
{
	if (!setup_cvars(wow))
		return EXIT_FAILURE;
	if (!setup_game(wow))
		return EXIT_FAILURE;
	if (!setup_objects(wow))
		return EXIT_FAILURE;
	if (!setup_wdb(wow))
		return EXIT_FAILURE;
	wow->starttime = nanotime();
	increment_frames(wow);
	uint32_t mapid = 530;
	const char *screen = NULL;
	char opt;
	const char *renderer = get_default_gfx_device_backend();
	if (!renderer)
	{
		LOG_ERROR("no device backend available");
		return EXIT_FAILURE;
	}
	const char *windowing = get_default_gfx_window_backend();
	if (!windowing)
	{
		LOG_ERROR("no window backend available");
		return EXIT_FAILURE;
	}
	while ((opt = getopt(argc, argv, "hm:p:ex:w:l:s:")) != -1)
	{
		switch (opt)
		{
			case 'h':
				printf("wow [-h] [-m <mapid>] [-p <path>] [-e] [-x <device>] [-w <window>] [-l <locale>] [-s <screen>]\n");
				printf("-h: show this help\n");
				printf("-m: set set mapid where to spawn\n");
				printf("-p: set the game path\n");
				printf("-x: set the render engine:\n");
				if (gfx_has_device_backend(GFX_DEVICE_GL3))
					printf("\tgl3: OpenGL 3\n");
				if (gfx_has_device_backend(GFX_DEVICE_GL4))
					printf("\tgl4: OpenGL 4\n");
				if (gfx_has_device_backend(GFX_DEVICE_D3D9))
					printf("\td3d9: Direct3D 9\n");
				if (gfx_has_device_backend(GFX_DEVICE_D3D11))
					printf("\td3d11: Direct3D 11\n");
				if (gfx_has_device_backend(GFX_DEVICE_VK))
					printf("\tvk: Vulkan\n");
				if (gfx_has_device_backend(GFX_DEVICE_GLES3))
					printf("\tgles3: OpenGL ES 3\n");
				printf("-w: set the windowing framework:\n");
				if (gfx_has_window_backend(GFX_WINDOW_X11))
					printf("\tx11: Native x.org\n");
				if (gfx_has_window_backend(GFX_WINDOW_WIN32))
					printf("\twin32: Native windows\n");
				if (gfx_has_window_backend(GFX_WINDOW_WAYLAND))
					printf("\twayland: Native wayland\n");
				if (gfx_has_window_backend(GFX_WINDOW_GLFW))
					printf("\tglfw: glfw library\n");
				if (gfx_has_window_backend(GFX_WINDOW_SDL))
					printf("\tsdl: sdl library\n");
				printf("-l: set the locale (frFR, enUS, ..)\n");
				printf("-s: set the boot screen (FrameXML, GlueXML)\n");
				return EXIT_SUCCESS;
			case 'm':
				mapid = atoll(optarg);
				break;
			case 'p':
				wow->game_path = optarg;
				break;
			case 'x':
				renderer = optarg;
				break;
			case 'w':
				windowing = optarg;
				break;
			case 'l':
				if (strlen(optarg) != 4)
				{
					LOG_ERROR("invalid locale");
					return EXIT_FAILURE;
				}
				strcpy(wow->locale, optarg);
				break;
			case 's':
				screen = optarg;
				break;
			default:
				LOG_ERROR("unknown parameter: %c", opt);
				return EXIT_FAILURE;
		}
	}
	if (!setup_game_files(wow))
		return EXIT_FAILURE;
	if (!setup_trs(wow))
		return EXIT_FAILURE;
	if (!setup_cache(wow))
		return EXIT_FAILURE;
	if (!setup_gfx(wow, windowing, renderer))
		return EXIT_FAILURE;
	if (!setup_shaders(wow))
		return EXIT_FAILURE;
	if (!setup_graphics(wow))
		return EXIT_FAILURE;
	if (!setup_render_passes(wow))
		return EXIT_FAILURE;
	if (!setup_frames(wow))
		return EXIT_FAILURE;
	if (!setup_loader(wow))
		return EXIT_FAILURE;
	if (!setup_dbc(wow))
		return EXIT_FAILURE;
	if (!load_placeholder_textures(wow))
		return EXIT_FAILURE;
	if (!setup_snd(wow))
		return EXIT_FAILURE;
	if (!setup_map(wow))
		return EXIT_FAILURE;
	if (!wow_set_map(wow, mapid))
		return EXIT_FAILURE;
	if (!setup_network(wow))
		return EXIT_FAILURE;
	if (!setup_interface(wow))
		return EXIT_FAILURE;
	if (screen)
	{
		if (!strcmp(screen, "FrameXML"))
		{
			wow->interface->switch_framescreen = true;
		}
		else if (!strcmp(screen, "GlueXML"))
		{
			wow->interface->switch_gluescreen = true;
		}
		else
		{
			LOG_ERROR("unknown screen");
			return EXIT_FAILURE;
		}
	}
	if (!setup_gui(wow))
		return EXIT_FAILURE;
	if (!setup_player(wow))
		return EXIT_FAILURE;
	if (!setup_cameras(wow))
		return EXIT_FAILURE;
	{ /* XXX try not to do this ? */
		struct gfx_resize_event event;
		event.width = wow->render_width;
		event.height = wow->render_height;
		resize_callback(wow->window, &event);
	}
	update_world_map_data(2, 4, 0); /* XXX remove this */
	if (cache_ref_font(wow->cache, "FONTS\\FRIZQT__.TTF", &wow->font_model_3d))
	{
		wow->font_3d = font_new(wow->font_model_3d, 50, 1, NULL);
		if (wow->font_3d)
			wow->font_3d->atlas->bpp = 8;
		else
			LOG_WARN("failed to create 3d font");
	}
	else
	{
		LOG_WARN("font not found");
		wow->font_3d = NULL;
	}
	loop(wow);
	map_delete(wow->map);
	object_delete((struct object*)wow->player);
	interface_delete(wow->interface);
	network_delete(wow->network);
	lagometer_delete(wow->lagometer);
	unload_dbc(wow);
	for (size_t i = 0; i <= RENDER_FRAMES_COUNT; ++i)
	{
		gx_frame_clear_scene(wow->cull_frame);
		increment_frames(wow);
		gx_frame_release_obj(wow->cull_frame);
	}
	while (1)
	{
		bool done = false;
		while (loader_has_loading(wow->loader))
		{
			done = true;
			loader_tick(wow->loader);
			increment_frames(wow);
		}
		while (loader_has_async(wow->loader))
		{
			done = true;
			usleep(50000);
		}
		if (!done)
			break;
	}
	cleanup_render_passes(wow);
	font_free(wow->font_3d);
	font_model_free(wow->font_model_3d);
	cache_print(wow->cache);
	cache_delete(wow->cache);
	shaders_delete(wow->shaders);
	graphics_delete(wow->graphics);
	/* XXX: wait for async end */
	gfx_device_tick(wow->device); /* finish gc cycle */
	camera_delete(wow->cameras[0]);
	camera_delete(wow->cameras[1]);
	loader_delete(wow->loader);
	for (size_t i = 0; i < RENDER_FRAMES_COUNT; ++i)
		gx_frame_destroy(&wow->frames[i]);
	mem_free(MEM_GENERIC, wow->frames);
	gfx_delete_window(wow->window);
	wow_mpq_compound_delete(wow->mpq_compound);
	jks_array_destroy(wow->mpq_archives);
	jks_hmap_destroy(wow->objects);
	wdb_free(wow->wdb);
	mem_free(MEM_GENERIC, wow->mpq_archives);
	mem_free(MEM_GENERIC, wow->objects);
	social_delete(wow->social);
	group_delete(wow->group);
	snd_delete(wow->snd);
	cvars_free(wow->cvars);
	return EXIT_SUCCESS;
}

static void resize_callback(struct gfx_window *window, struct gfx_resize_event *event)
{
	struct wow *wow = window->userdata;
	wow->render_width = event->width;
	wow->render_height = event->height;
	dirty_render_passes(wow);
	if ((wow->wow_opt & WOW_OPT_RENDER_INTERFACE) && wow->interface)
		interface_on_window_resized(wow->interface, event);
}

static void key_down_callback(struct gfx_window *window, struct gfx_key_event *event)
{
	struct wow *wow = window->userdata;
	if ((wow->wow_opt & WOW_OPT_RENDER_INTERFACE) && wow->interface)
	{
		if (interface_on_key_down(wow->interface, event))
			return;
	}
#define RENDER_OPT_FLIP(id) \
do \
{ \
	if (wow->render_opt & id) \
		wow->render_opt &= ~id; \
	else \
		wow->render_opt |= id; \
} while (0)

#define WOW_OPT_FLIP(id) \
do \
{ \
	if (wow->wow_opt & id) \
		wow->wow_opt &= ~id; \
	else \
		wow->wow_opt |= id; \
} while (0)

	switch (event->key)
	{
		case GFX_KEY_9:
			exit(EXIT_SUCCESS);
			break;
		case GFX_KEY_ESCAPE:
			wow->wow_opt &= ~WOW_OPT_FOCUS_3D;
			gfx_window_ungrab_cursor(wow->window);
			return;
		case GFX_KEY_LBRACKET:
			RENDER_OPT_FLIP(RENDER_OPT_M2_COLLISIONS);
			RENDER_OPT_FLIP(RENDER_OPT_WMO_COLLISIONS);
			return;
		case GFX_KEY_RBRACKET:
			RENDER_OPT_FLIP(RENDER_OPT_COLLISIONS);
			return;
		case GFX_KEY_P:
			RENDER_OPT_FLIP(RENDER_OPT_WMO_PORTALS);
			return;
		case GFX_KEY_O:
			RENDER_OPT_FLIP(RENDER_OPT_WMO_AABB);
			return;
		case GFX_KEY_I:
			RENDER_OPT_FLIP(RENDER_OPT_M2_AABB);
			return;
		case GFX_KEY_L:
			RENDER_OPT_FLIP(RENDER_OPT_WMO);
			return;
		case GFX_KEY_K:
			RENDER_OPT_FLIP(RENDER_OPT_M2);
			return;
		case GFX_KEY_J:
			RENDER_OPT_FLIP(RENDER_OPT_M2_RIBBONS);
			RENDER_OPT_FLIP(RENDER_OPT_M2_PARTICLES);
			break;
		case GFX_KEY_Y:
			RENDER_OPT_FLIP(RENDER_OPT_WMO_LIGHTS);
			RENDER_OPT_FLIP(RENDER_OPT_M2_LIGHTS);
			return;
		case GFX_KEY_T:
			RENDER_OPT_FLIP(RENDER_OPT_TAXI);
			return;
		case GFX_KEY_R:
			RENDER_OPT_FLIP(RENDER_OPT_ADT_AABB);
			RENDER_OPT_FLIP(RENDER_OPT_MCNK_AABB);
			RENDER_OPT_FLIP(RENDER_OPT_MCLQ_AABB);
			RENDER_OPT_FLIP(RENDER_OPT_WDL_AABB);
			return;
		case GFX_KEY_F:
			RENDER_OPT_FLIP(RENDER_OPT_FOG);
			return;
		case GFX_KEY_U:
			RENDER_OPT_FLIP(RENDER_OPT_M2_BONES);
			return;
		case GFX_KEY_Z:
			RENDER_OPT_FLIP(RENDER_OPT_WDL);
			return;
		case GFX_KEY_X:
			RENDER_OPT_FLIP(RENDER_OPT_MCLQ);
			return;
		case GFX_KEY_V:
			wow->post_process.glow->enabled = !wow->post_process.glow->enabled;
			return;
		case GFX_KEY_B:
			WOW_OPT_FLIP(WOW_OPT_VSYNC);
			gfx_window_set_swap_interval(wow->window, (wow->wow_opt & WOW_OPT_VSYNC) ? -1 : 0);
			return;
		case GFX_KEY_N:
			render_target_set_enabled(wow->post_process.msaa, !wow->post_process.msaa->enabled);
			return;
		case GFX_KEY_M:
			RENDER_OPT_FLIP(RENDER_OPT_MESH);
			graphics_build_world_rasterizer_states(wow->graphics);
			return;
		case GFX_KEY_COMMA:
			wow->post_process.sobel->enabled = !wow->post_process.sobel->enabled;
			wow->post_process.cel->enabled = !wow->post_process.cel->enabled;
			return;
		case GFX_KEY_PERIOD:
			wow->post_process.fxaa->enabled = !wow->post_process.fxaa->enabled;
			return;
		case GFX_KEY_SLASH:
			wow->post_process.ssao->enabled = !wow->post_process.ssao->enabled;
			return;
		case GFX_KEY_APOSTROPHE:
			wow->post_process.bloom->enabled = !wow->post_process.bloom->enabled;
			break;
		case GFX_KEY_BACKSLASH:
			wow->post_process.sharpen->enabled = !wow->post_process.sharpen->enabled;
			break;
		case GFX_KEY_SEMICOLON:
			wow->post_process.chromaber->enabled = !wow->post_process.chromaber->enabled;
			break;
		/* KP */
		case GFX_KEY_PAGE_UP:
			wow->view_camera->view_distance += CHUNK_WIDTH * (event->mods & GFX_KEY_MOD_CONTROL ? 10 : 1) * (event->mods & GFX_KEY_MOD_SHIFT ? 5 : 1);
			return;
		case GFX_KEY_PAGE_DOWN:
			wow->view_camera->view_distance -= CHUNK_WIDTH * (event->mods & GFX_KEY_MOD_CONTROL ? 10 : 1) * (event->mods & GFX_KEY_MOD_SHIFT ? 5 : 1);
			return;
		case GFX_KEY_HOME:
			RENDER_OPT_FLIP(RENDER_OPT_DYN_WATER);
			break;
		case GFX_KEY_INSERT:
			WOW_OPT_FLIP(WOW_OPT_M2_TRACK_BSEARCH);
			break;
		case GFX_KEY_H:
			WOW_OPT_FLIP(WOW_OPT_RENDER_GUI);
			return;
		case GFX_KEY_G:
			WOW_OPT_FLIP(WOW_OPT_RENDER_INTERFACE);
			return;
		case GFX_KEY_EQUAL:
			WOW_OPT_FLIP(WOW_OPT_DIFFERENT_CAMERAS);
			wow->view_camera = wow->cameras[!!(wow->wow_opt & WOW_OPT_DIFFERENT_CAMERAS)];
			return;
		case GFX_KEY_F5:
			wow_set_map(wow, 0);
			return;
		case GFX_KEY_F6:
			wow_set_map(wow, 1);
			return;
		case GFX_KEY_F7:
			wow_set_map(wow, 530);
			return;
		case GFX_KEY_F9:
		{
			const char *username = "ADMINISTRATOR";
			const char *password = "ADMINISTRATOR";
			net_auth_connect(wow->network, username, password);
			return;
		}
		case GFX_KEY_F10:
			WOW_OPT_FLIP(WOW_OPT_GRAVITY);
			break;
		case GFX_KEY_F12:
		{
			struct shaders *shaders = shaders_new();
			if (!shaders)
			{
				LOG_ERROR("failed to rebuild shaders");
				break;
			}
			shaders_delete(wow->shaders);
			wow->shaders = shaders;
			break;
		}
		case GFX_KEY_1:
			wow->view_camera->fov += 1.0 / 180.0 * M_PI;
			return;
		case GFX_KEY_2:
			wow->view_camera->fov -= 1.0 / 180.0 * M_PI;
			return;
		case GFX_KEY_3:
			WOW_OPT_FLIP(WOW_OPT_AABB_OPTIMIZE);
			return;
		case GFX_KEY_4:
			WOW_OPT_FLIP(WOW_OPT_ASYNC_CULL);
			return;
		case GFX_KEY_5:
			wow->fsaa -= 0.1;
			if (wow->fsaa <= 0.1)
				wow->fsaa = 0.1;
			dirty_render_passes(wow);
			break;
		case GFX_KEY_6:
			wow->fsaa += 0.1;
			if (wow->fsaa > 4.0)
				wow->fsaa = 4.0;
			dirty_render_passes(wow);
			break;
		case GFX_KEY_7:
			RENDER_OPT_FLIP(RENDER_OPT_GROUND_EFFECT);
			break;
		case GFX_KEY_8:
			RENDER_OPT_FLIP(RENDER_OPT_DYN_SHADOW);
			break;
		default:
			break;
	}
#undef WORLD_OPTION_FLIP
}

static void key_up_callback(struct gfx_window *window, struct gfx_key_event *event)
{
	struct wow *wow = window->userdata;
	if ((wow->wow_opt & WOW_OPT_RENDER_INTERFACE) && wow->interface)
	{
		if (interface_on_key_up(wow->interface, event))
			return;
	}
}

static void key_press_callback(struct gfx_window *window, struct gfx_key_event *event)
{
	struct wow *wow = window->userdata;
	if ((wow->wow_opt & WOW_OPT_RENDER_INTERFACE) && wow->interface)
	{
		if (interface_on_key_press(wow->interface, event))
			return;
	}
}

static void char_callback(struct gfx_window *window, struct gfx_char_event *event)
{
	struct wow *wow = window->userdata;
	if ((wow->wow_opt & WOW_OPT_RENDER_INTERFACE) && wow->interface)
	{
		if (interface_on_char(wow->interface, event))
			return;
	}
}

static void mouse_down_callback(struct gfx_window *window, struct gfx_mouse_event *event)
{
	struct wow *wow = window->userdata;
	if ((wow->wow_opt & WOW_OPT_RENDER_INTERFACE) && wow->interface)
	{
		if (interface_on_mouse_down(wow->interface, event))
			return;
	}
	if (!(wow->wow_opt & WOW_OPT_FOCUS_3D))
	{
		wow->view_camera->move_unit = false;
		switch (event->button)
		{
			case GFX_MOUSE_BUTTON_RIGHT:
				wow->view_camera->move_unit = true;
				/* FALLTHROUGH */
			case GFX_MOUSE_BUTTON_LEFT:
				wow->wow_opt |= WOW_OPT_FOCUS_3D;
				gfx_window_grab_cursor(wow->window);
				break;
			default:
				break;
		}
	}
}

static void mouse_up_callback(struct gfx_window *window, struct gfx_mouse_event *event)
{
	struct wow *wow = window->userdata;
	if ((wow->wow_opt & WOW_OPT_RENDER_INTERFACE) && wow->interface)
	{
		if (interface_on_mouse_up(wow->interface, event))
			return;
	}
	if ((event->button == GFX_MOUSE_BUTTON_LEFT || event->button == GFX_MOUSE_BUTTON_RIGHT) && (wow->wow_opt & WOW_OPT_FOCUS_3D))
	{
		wow->view_camera->move_unit = false;
		wow->wow_opt &= ~WOW_OPT_FOCUS_3D;
		gfx_window_ungrab_cursor(wow->window);
	}
}

static void mouse_move_callback(struct gfx_window *window, struct gfx_pointer_event *event)
{
	struct wow *wow = window->userdata;
	if ((wow->wow_opt & WOW_OPT_RENDER_INTERFACE) && wow->interface)
		interface_on_mouse_move(wow->interface, event);
}

static void mouse_scroll_callback(struct gfx_window *window, struct gfx_scroll_event *event)
{
	struct wow *wow = window->userdata;
	if ((wow->wow_opt & WOW_OPT_RENDER_INTERFACE) && wow->interface)
	{
		if (interface_on_mouse_scroll(wow->interface, event))
			return;
	}
	camera_handle_scroll(wow->view_camera, event->y);
}

static void error_callback(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	LOG_ERRORV(fmt, ap);
	va_end(ap);
}

void wow_set_player(struct wow *wow, struct player *player)
{
	wow->player = player;
	wow->cameras[0]->worldobj = (struct worldobj*)player;
}

bool wow_set_object(struct wow *wow, uint64_t guid, struct object *object)
{
	if (!object)
	{
		jks_hmap_erase(wow->objects, JKS_HMAP_KEY_U64(guid));
		return true;
	}
	if (!jks_hmap_set(wow->objects, JKS_HMAP_KEY_U64(guid), &object))
	{
		LOG_ERROR("failed to add object to hmap");
		return false;
	}
	return true;
}

struct object *wow_get_object(struct wow *wow, uint64_t guid)
{
	struct object **object = jks_hmap_get(wow->objects, JKS_HMAP_KEY_U64(guid));
	if (!object)
		return NULL;
	return *object;
}

int64_t nanotime(void)
{
#ifdef __linux__
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
	uint64_t t = ts.tv_sec * 1000000000 + ts.tv_nsec;
	return t - g_wow->starttime;
#elif defined (__unix__)
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	uint64_t t = ts.tv_sec * 1000000000 + ts.tv_nsec;
	return t - g_wow->starttime;
#elif defined (_WIN32)
	LARGE_INTEGER ret;
	QueryPerformanceCounter(&ret);
	return ret.QuadPart / performance_frequency;
#else
#error unsupported nanotime
#endif
}

uint32_t npot32(uint32_t v)
{
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	return v + 1;
}

bool wow_load_compound(struct wow *wow, struct wow_mpq_compound *compound)
{
	for (size_t i = 0; i < wow->mpq_archives->size; ++i)
	{
		struct wow_mpq_archive *archive = *JKS_ARRAY_GET(wow->mpq_archives, i, struct wow_mpq_archive*);
		if (!wow_mpq_compound_add_archive(compound, archive))
			return false;
	}
	return true;
}

int wow_asprintf(int memory_type, char **str, const char *fmt, ...)
{
	int ret;
	va_list args;
	va_start(args, fmt);
	ret = wow_vasprintf(memory_type, str, fmt, args);
	va_end(args);
	return ret;
}

int wow_vasprintf(int memory_type, char **str, const char *fmt, va_list args)
{
	int size;
	va_list tmp;
	va_copy(tmp, args);
	size = vsnprintf(NULL, 0, fmt, tmp);
	va_end(tmp);
	if (size < 0)
		return size;
	*str = mem_malloc(memory_type, size + 1);
	if (*str == NULL)
		return -1;
	return vsnprintf(*str, size + 1, fmt, args);
}

static void *freetype_malloc(FT_Memory memory, long size)
{
	(void)memory;
	return mem_malloc(MEM_FONT, size);
}

static void *freetype_realloc(FT_Memory memory, long cur_size, long new_size, void *block)
{
	(void)memory;
	(void)cur_size;
	return mem_realloc(MEM_FONT, block, new_size);
}

static void freetype_free(FT_Memory memory, void *block)
{
	(void)memory;
	mem_free(MEM_FONT, block);
}

int main(int argc, char **argv)
{
	mem_init();
	performance_init();
#ifdef __unix__
	g_log_colored = isatty(1);
#else
	g_log_colored = false;
#endif
#ifdef _WIN32
	WSADATA wsa_data;
	WSAStartup(MAKEWORD(2, 2), &wsa_data);
#endif
	setvbuf(stdout, NULL , _IOLBF, 4096 * 2);
	setvbuf(stderr, NULL , _IOLBF, 4096 * 2);
	srand(time(NULL));
	gfx_memory.malloc = mem_malloc_GFX;
	gfx_memory.realloc = mem_realloc_GFX;
	gfx_memory.free = mem_free_GFX;
	wow_memory.malloc = mem_malloc_LIBWOW;
	wow_memory.realloc = mem_realloc_LIBWOW;
	wow_memory.free = mem_free_LIBWOW;
	gfx_error_callback = error_callback;
	g_wow = mem_zalloc(MEM_GENERIC, sizeof(*g_wow));
	if (!g_wow)
	{
		LOG_ERROR("failed to alloc g_wow");
		return EXIT_FAILURE;
	}
	g_wow->game_path = "WoW";
	strcpy(g_wow->locale, "frFR");
	g_wow->anisotropy = 16;
	g_wow->fsaa = 1;
	g_wow->render_opt |= RENDER_OPT_MCNK;
	g_wow->render_opt |= RENDER_OPT_MCLQ;
	g_wow->render_opt |= RENDER_OPT_WMO;
	g_wow->render_opt |= RENDER_OPT_WDL;
	g_wow->render_opt |= RENDER_OPT_M2;
	g_wow->render_opt |= RENDER_OPT_FOG;
	g_wow->render_opt |= RENDER_OPT_WMO_LIQUIDS;
	g_wow->render_opt |= RENDER_OPT_SKYBOX;
	g_wow->render_opt |= RENDER_OPT_M2_PARTICLES;
	g_wow->render_opt |= RENDER_OPT_M2_RIBBONS;
	g_wow->render_opt |= RENDER_OPT_GROUND_EFFECT;
	g_wow->render_opt |= RENDER_OPT_DYN_SHADOW;
#if 0
	g_wow->render_opt |= RENDER_OPT_SSR;
#endif
	g_wow->wow_opt |= WOW_OPT_RENDER_INTERFACE;
	g_wow->wow_opt |= WOW_OPT_AABB_OPTIMIZE;
	g_wow->wow_opt |= WOW_OPT_RENDER_GUI;
	g_wow->wow_opt |= WOW_OPT_M2_TRACK_BSEARCH;
	g_wow->wow_opt |= WOW_OPT_ASYNC_CULL;
#if defined (_WIN32)
	{
		LARGE_INTEGER frequency;
		QueryPerformanceFrequency(&frequency);
		performance_frequency = frequency.QuadPart / 1000000000.0f;
	}
#endif
	g_wow->ft_memory = mem_malloc(MEM_FONT, sizeof(*g_wow->ft_memory));
	if (!g_wow->ft_memory)
	{
		LOG_ERROR("failed to allocate freetype memory");
		return EXIT_FAILURE;
	}
	g_wow->ft_memory->alloc = freetype_malloc;
	g_wow->ft_memory->realloc = freetype_realloc;
	g_wow->ft_memory->free = freetype_free;
	if (FT_New_Library(g_wow->ft_memory, &g_wow->ft_lib))
	{
		LOG_ERROR("failed to init freetype lib");
		return EXIT_FAILURE;
	}
	FT_Add_Default_Modules(g_wow->ft_lib);
	int ret = wow_main(g_wow, argc, argv);
	FT_Done_Library(g_wow->ft_lib);
	mem_free(MEM_FONT, g_wow->ft_memory);
	mem_free(MEM_GENERIC, g_wow);
	mem_dump();
	return ret;
}
