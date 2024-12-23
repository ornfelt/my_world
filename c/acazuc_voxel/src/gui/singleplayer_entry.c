#include "gui/singleplayer_entry.h"
#include "gui/singleplayer.h"
#include "gui/gui.h"

#include "world/level.h"

#include "textures.h"
#include "voxel.h"
#include "log.h"

#include <gfx/events.h>

#include <stdlib.h>
#include <time.h>

struct gui_singleplayer_entry *gui_singleplayer_entry_new(struct gui_singleplayer *parent,
                                                          int32_t pos,
                                                          const char *name)
{
	struct gui_singleplayer_entry *entry = calloc(sizeof(*entry), 1);
	if (!entry)
	{
		LOG_ERROR("singleplayer entry allocation failed");
		return NULL;
	}
	struct level *level = level_new(name);
	if (!level)
	{
		free(entry);
		return NULL;
	}
	entry->parent = parent;
	entry->pos = pos;
	entry->hover = false;
	gui_sprite_init(&entry->background);
	gui_sprite_set_texture(&entry->background, &g_voxel->textures->whitepixel);
	gui_sprite_set_size(&entry->background, 270 - 2, 36 - 2);
	gui_sprite_set_color(&entry->background, 0, 0, 0, 1);
	gui_sprite_init(&entry->border);
	gui_sprite_set_texture(&entry->border, &g_voxel->textures->whitepixel);
	gui_sprite_set_size(&entry->border, 270, 36);
	gui_sprite_set_color(&entry->border, 0.666, 0.666, 0.666, 1);
	gui_sprite_init(&entry->image);
	gui_sprite_set_texture(&entry->image, &g_voxel->textures->chicken);
	gui_sprite_set_size(&entry->image, 16 * 2, 16 * 2);
	gui_label_init(&entry->dir_date);
	struct tm tm;
	char time_buf[512];
	char buf[512];
	time_t timev = level->nbt.LastPlayed->value / 1000;
	gmtime_r(&timev, &tm);
	strftime(time_buf, sizeof(time_buf), "%c", &tm);
	snprintf(buf, sizeof(buf), "%s (%s)", level->nbt.Version.Name->value, time_buf);
	gui_label_set_text(&entry->dir_date, buf);
	gui_label_set_color(&entry->dir_date, '7');
	gui_label_init(&entry->other);
	snprintf(buf, sizeof(buf), "Version: %s", level->nbt.Version.Name->value);
	gui_label_set_text(&entry->other, buf);
	gui_label_set_color(&entry->other, '7');
	gui_label_init(&entry->name);
	gui_label_set_text(&entry->name, level->nbt.LevelName->value);
	level_delete(level);
	return entry;
}

void gui_singleplayer_entry_delete(struct gui_singleplayer_entry *entry)
{
	if (!entry)
		return;
	gui_sprite_destroy(&entry->background);
	gui_sprite_destroy(&entry->border);
	gui_sprite_destroy(&entry->image);
	gui_label_destroy(&entry->dir_date);
	gui_label_destroy(&entry->other);
	gui_label_destroy(&entry->name);
	free(entry);
}

void gui_singleplayer_entry_draw(struct gui_singleplayer_entry *entry)
{
	int32_t x = (g_voxel->gui->width - 270) / 2;
	int32_t y = 17 * 2 + entry->pos * 36;
	if (entry->parent->selected == entry->pos)
	{
		gui_sprite_set_pos(&entry->border, x, y);
		gui_sprite_draw(&entry->border);
		gui_sprite_set_pos(&entry->background, x + 1, y + 1);
		gui_sprite_draw(&entry->background);
	}
	gui_sprite_set_pos(&entry->image, x + 2, y + 2);
	gui_sprite_draw(&entry->image);
	gui_label_set_pos(&entry->name, x + 37, y + 2);
	gui_label_draw(&entry->name);
	gui_label_set_pos(&entry->dir_date, x + 37, y + 13);
	gui_label_draw(&entry->dir_date);
	gui_label_set_pos(&entry->other, x + 37, y + 9 + 13);
	gui_label_draw(&entry->other);
}

void gui_singleplayer_entry_mouse_move(struct gui_singleplayer_entry *entry,
                                       struct gfx_pointer_event *event)
{
	if (!event->used
	 && event->x >= (g_voxel->gui->width - 270) / 2
	 && event->x <= (g_voxel->gui->width + 270) / 2
	 && event->y >= 17 * 2 + entry->pos * 36
	 && event->y <= 17 * 2 + entry->pos * 36 + 36)
	{
		entry->hover = true;
		event->used = true;
	}
	else
	{
		entry->hover = false;
	}
}

void gui_singleplayer_entry_mouse_down(struct gui_singleplayer_entry *entry,
                                       struct gfx_mouse_event *event)
{
	if (event->used)
		return;
	if (entry->hover)
	{
		entry->parent->selected = entry->pos;
		event->used = true;
	}
}
