#include "gui/multiplayer_entry.h"
#include "gui/multiplayer.h"
#include "gui/gui.h"

#include "textures.h"
#include "voxel.h"
#include "log.h"

#include <gfx/events.h>

#include <stdlib.h>

struct gui_multiplayer_entry *gui_multiplayer_entry_new(struct gui_multiplayer *parent, int32_t pos)
{
	struct gui_multiplayer_entry *entry = calloc(sizeof(*entry), 1);
	if (!entry)
	{
		LOG_ERROR("multiplayer entry allocation failed");
		return NULL;
	}
	entry->parent = parent;
	entry->pos = pos;
	return entry;
}

void gui_multiplayer_entry_delete(struct gui_multiplayer_entry *entry)
{
	if (!entry)
		return;
	free(entry);
}

void gui_multiplayer_entry_draw(struct gui_multiplayer_entry *entry)
{
}

void gui_multiplayer_entry_mouse_move(struct gui_multiplayer_entry *entry,
                                      struct gfx_pointer_event *event)
{
}

void gui_multiplayer_entry_mouse_down(struct gui_multiplayer_entry *entry,
                                      struct gfx_mouse_event *event)
{
}
