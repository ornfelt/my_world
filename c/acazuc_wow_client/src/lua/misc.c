#include "functions.h"

#include "itf/interface.h"

#include "ui/taxi_route_frame.h"
#include "ui/dress_up_model.h"
#include "ui/message_frame.h"
#include "ui/check_button.h"
#include "ui/player_model.h"
#include "ui/tabard_model.h"
#include "ui/scroll_frame.h"
#include "ui/color_select.h"
#include "ui/game_tooltip.h"
#include "ui/movie_frame.h"
#include "ui/simple_html.h"
#include "ui/status_bar.h"
#include "ui/model_ffx.h"
#include "ui/cooldown.h"
#include "ui/edit_box.h"
#include "ui/minimap.h"
#include "ui/slider.h"
#include "ui/button.h"
#include "ui/model.h"

#include "lua/lua_script.h"

#include "obj/update_fields.h"
#include "obj/player.h"

#include "wow_lua.h"
#include "memory.h"
#include "const.h"
#include "cvars.h"
#include "dbc.h"
#include "log.h"
#include "wow.h"
#include "wdb.h"

#include <gfx/window.h>

#include <jks/hmap.h>

#include <wow/wdb.h>

#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

MEMORY_DECL(LUA);

LUA_FN(GetItemQualityColor)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: GetItemQualityColor(quality)");
	if (!lua_isinteger(L, 1))
		return luaL_argerror(L, 1, "integer expected");
	int quality = lua_tointeger(L, 1);
	switch (quality)
	{
		case 0: //poor
			lua_pushnumber(L, 0.62);
			lua_pushnumber(L, 0.62);
			lua_pushnumber(L, 0.62);
			lua_pushstring(L, "ff9d9d9d");
			break;
		case 1: //common
			lua_pushnumber(L, 1);
			lua_pushnumber(L, 1);
			lua_pushnumber(L, 1);
			lua_pushstring(L, "ffffffff");
			break;
		case 2: //uncommon
			lua_pushnumber(L, 0.12);
			lua_pushnumber(L, 1);
			lua_pushnumber(L, 0);
			lua_pushstring(L, "ff1eff00");
			break;
		case 3: //rare
			lua_pushnumber(L, 0);
			lua_pushnumber(L, 0.44);
			lua_pushnumber(L, 0.87);
			lua_pushstring(L, "ff0070dd");
			break;
		case 4: //epic
			lua_pushnumber(L, 0.64);
			lua_pushnumber(L, 0.2);
			lua_pushnumber(L, 0.93);
			lua_pushstring(L, "ffa335ee");
			break;
		case 5: //legendary
			lua_pushnumber(L, 1);
			lua_pushnumber(L, 0.5);
			lua_pushnumber(L, 0);
			lua_pushstring(L, "ffff8000");
			break;
		case 6: //artifact
			lua_pushnumber(L, 0.9);
			lua_pushnumber(L, 0.8);
			lua_pushnumber(L, 0.5);
			lua_pushstring(L, "ffe6cc80");
			break;
		case 7: //heirloom
			lua_pushnumber(L, 0.9);
			lua_pushnumber(L, 0.8);
			lua_pushnumber(L, 0.5);
			lua_pushstring(L, "ffe6cc80");
			break;
		default:
			lua_pushnumber(L, 1);
			lua_pushnumber(L, 1);
			lua_pushnumber(L, 1);
			lua_pushstring(L, "|cffffffff");
			break;
	}
	return 4;
}

LUA_FN(CreateFrame)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc < 1)
		return luaL_error(L, "Usage: CreateFrame(\"type\", [\"name\"], [\"parent\" or parent], [\"inherits\"], [id])");
	const char *frame_type = lua_tostring(L, 1);
	if (!frame_type)
		return luaL_argerror(L, 1, "lua_tostring failed");
	const char *name = NULL;
	if (argc >= 2)
		name = lua_tostring(L, 2);
	struct ui_frame *parent = NULL;
	if (argc >= 3)
	{
		switch (lua_type(L, 3))
		{
			case LUA_TSTRING:
			{
				const char *s = lua_tostring(L, 3);
				if (s)
					parent = interface_get_frame(g_wow->interface, s);
				break;
			}
			case LUA_TTABLE:
				parent = ui_get_lua_frame(L, 3);
				break;
			case LUA_TNIL:
				break;
			default:
				return luaL_argerror(L, 3, "string or table expected");
		}
	}
	struct jks_array inherits; /* xml_layout_frame_t* */
	jks_array_init(&inherits, sizeof(struct xml_layout_frame*), NULL, &jks_array_memory_fn_LUA);
	if (argc >= 4)
	{
		const char *inherits_frame = lua_tostring(L, 4);
		if (!inherits_frame)
		{
			jks_array_destroy(&inherits);
			return luaL_argerror(L, 4, "string expected");
		}
		char *prv = (char*)inherits_frame;
		char *ite;
		while ((ite = strchr(prv, ',')))
		{
			if (prv[0] == ' ')
				prv++;
			char *dup = mem_malloc(MEM_LUA, ite - prv + 1);
			if (!dup)
			{
				LOG_ERROR("malloc failed");
				break;
			}
			memcpy(dup, prv, ite - prv);
			dup[ite - prv] = '\0';
			struct xml_layout_frame *inherit = interface_get_virtual_layout_frame(g_wow->interface, dup);
			if (!inherit)
			{
				LOG_WARN("unknown inherits: %s", dup);
				mem_free(MEM_LUA, dup);
				continue;
			}
			mem_free(MEM_LUA, dup);
			if (!jks_array_push_back(&inherits, &inherit))
				LOG_ERROR("failed to push inherit");
		}
		if (*prv)
		{
			if (prv[0] == ' ')
				prv++;
			if (*prv)
			{
				struct xml_layout_frame *inherit = interface_get_virtual_layout_frame(g_wow->interface, prv);
				if (inherit)
				{
					if (!jks_array_push_back(&inherits, &inherit))
						LOG_ERROR("failed to push inherit");
				}
				else
				{
					LOG_WARN("unknown inherits: %s", prv);
				}
			}
		}
	}
	struct optional_int32 id;
	OPTIONAL_UNSET(id);
	if (argc >= 5)
		OPTIONAL_CTR(id, lua_tointeger(L, 5));
	struct ui_frame *frame = NULL;
	if (!strcasecmp(frame_type, "button"))
		frame = (struct ui_frame*)ui_button_new(g_wow->interface, name, (struct ui_region*)parent);
	else if (!strcasecmp(frame_type, "checkbutton"))
		frame = (struct ui_frame*)ui_check_button_new(g_wow->interface, name, (struct ui_region*)parent);
	else if (!strcasecmp(frame_type, "colorselect"))
		frame = (struct ui_frame*)ui_color_select_new(g_wow->interface, name, (struct ui_region*)parent);
	else if (!strcasecmp(frame_type, "cooldown"))
		frame = (struct ui_frame*)ui_cooldown_new(g_wow->interface, name, (struct ui_region*)parent);
	else if (!strcasecmp(frame_type, "dressupmodel"))
		frame = (struct ui_frame*)ui_dress_up_model_new(g_wow->interface, name, (struct ui_region*)parent);
	else if (!strcasecmp(frame_type, "editbox"))
		frame = (struct ui_frame*)ui_edit_box_new(g_wow->interface, name, (struct ui_region*)parent);
	else if (!strcasecmp(frame_type, "frame"))
		frame = (struct ui_frame*)ui_frame_new(g_wow->interface, name, (struct ui_region*)parent);
	else if (!strcasecmp(frame_type, "gametooltip"))
		frame = (struct ui_frame*)ui_game_tooltip_new(g_wow->interface, name, (struct ui_region*)parent);
	else if (!strcasecmp(frame_type, "messageframe"))
		frame = (struct ui_frame*)ui_message_frame_new(g_wow->interface, name, (struct ui_region*)parent);
	else if (!strcasecmp(frame_type, "minimap"))
		frame = (struct ui_frame*)ui_minimap_new(g_wow->interface, name, (struct ui_region*)parent);
	else if (!strcasecmp(frame_type, "model"))
		frame = (struct ui_frame*)ui_model_new(g_wow->interface, name, (struct ui_region*)parent);
	else if (!strcasecmp(frame_type, "modelffx"))
		frame = (struct ui_frame*)ui_model_ffx_new(g_wow->interface, name, (struct ui_region*)parent);
	else if (!strcasecmp(frame_type, "movieframe"))
		frame = (struct ui_frame*)ui_movie_frame_new(g_wow->interface, name, (struct ui_region*)parent);
	else if (!strcasecmp(frame_type, "playermodel"))
		frame = (struct ui_frame*)ui_player_model_new(g_wow->interface, name, (struct ui_region*)parent);
	else if (!strcasecmp(frame_type, "scrollframe"))
		frame = (struct ui_frame*)ui_scroll_frame_new(g_wow->interface, name, (struct ui_region*)parent);
	else if (!strcasecmp(frame_type, "simplehtml"))
		frame = (struct ui_frame*)ui_simple_html_new(g_wow->interface, name, (struct ui_region*)parent);
	else if (!strcasecmp(frame_type, "slider"))
		frame = (struct ui_frame*)ui_slider_new(g_wow->interface, name, (struct ui_region*)parent);
	else if (!strcasecmp(frame_type, "statusbar"))
		frame = (struct ui_frame*)ui_status_bar_new(g_wow->interface, name, (struct ui_region*)parent);
	else if (!strcasecmp(frame_type, "tabardmodel"))
		frame = (struct ui_frame*)ui_tabard_model_new(g_wow->interface, name, (struct ui_region*)parent);
	else if (!strcasecmp(frame_type, "taxirouteframe"))
		frame = (struct ui_frame*)ui_taxi_route_frame_new(g_wow->interface, name, (struct ui_region*)parent);
	if (!frame)
	{
		char buf[256];
		snprintf(buf, sizeof(buf), "failed to create frame: %s", frame_type);
		jks_array_destroy(&inherits);
		return luaL_argerror(L, 1, buf);
	}
	for (size_t i = 0; i < inherits.size; ++i)
		ui_object_load_xml((struct ui_object*)frame, *JKS_ARRAY_GET(&inherits, i, struct xml_layout_frame*));
	jks_array_destroy(&inherits);
	if (OPTIONAL_ISSET(id))
		ui_frame_set_id(frame, OPTIONAL_GET(id));
	ui_object_eval_name((struct ui_object*)frame);
	ui_object_post_load((struct ui_object*)frame);
	ui_push_lua_object(L, (struct ui_object*)frame);
	return 1;
}

LUA_FN(geterrorhandler)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: geterrorhandler()");
	if (g_wow->interface->error_script)
		lua_rawgeti(L, LUA_REGISTRYINDEX, g_wow->interface->error_script->ref);
	else
		lua_pushnil(L);
	return 1;
}

LUA_FN(seterrorhandler)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: seterrorhandler(function)");
	if (lua_type(L, 1) != LUA_TFUNCTION)
		return luaL_argerror(L, 1, "function expected");
	int ref = luaL_ref(L, LUA_REGISTRYINDEX);
	struct lua_script *script = lua_script_new_ref(g_wow->interface->L, ref);
	interface_set_error_script(g_wow->interface, script);
	return 0;
}

LUA_FN(SetPortraitTexture)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 2)
		return luaL_error(L, "Usage: SetPortraitTexture(frame, \"unit\")");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(PlayerHasSpells)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: PlayerHasSpells()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, true);
	return 1;
}

LUA_FN(CloseBankFrame)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: CloseBankFrame()");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(GetCVar)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: GetCVar(\"cvar\")");
	if (!lua_isstring(L, 1))
		return luaL_argerror(L, 1, "string expected");
	const char *arg = lua_tostring(L, 1);
	if (!arg)
		return luaL_argerror(L, 1, "lua_tostring failed");
	const char *cvar = cvar_get(g_wow->cvars, arg);
	if (!cvar)
	{
		LOG_WARN("unimplemented cvar: %s", lua_tostring(L, 1));
		lua_pushstring(L, "");
	}
	else
	{
		lua_pushstring(L, cvar);
	}
	return 1;
}

LUA_FN(GetCVarDefault)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: GetCVarDefault(\"cvar\")");
	if (!lua_isstring(L, 1))
		return luaL_argerror(L, 1, "string expected");
	const char *arg = lua_tostring(L, 1);
	if (!arg)
		return luaL_argerror(L, 1, "lua_tostring failed");
	const char *cvar = cvar_get(g_wow->cvars, arg);
	if (!cvar)
	{
		LOG_WARN("unimplemented cvar: %s", lua_tostring(L, 1));
		lua_pushstring(L, "");
	}
	else
	{
		lua_pushstring(L, cvar);
	}
	return 1;
}

LUA_FN(SetCVar)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc < 2)
		return luaL_error(L, "Usage: SetCVar(\"var\", value)");
	const char *key = lua_tostring(L, 1);
	const char *val = lua_tostring(L, 2);
	if (!key)
		return luaL_argerror(L, 1, "string expected");
	cvar_set(g_wow->cvars, key, val);
	return 0;
}

LUA_FN(RegisterForSave)
{
	LUA_VERBOSE_FN();
	if (lua_gettop(L) != 1)
		return 0;
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(GetRestState)
{
	LUA_VERBOSE_FN();
	if (lua_gettop(L) != 0)
		return 0;
	LUA_UNIMPLEMENTED_FN();
	lua_pushnumber(L, 1);
	lua_pushstring(L, "Rested");
	lua_pushnumber(L, 2);
	return 3;
}

LUA_FN(GetXPExhaustion)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetXPExhaustion()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushnumber(L, 16000); //push nil if rested
	return 1;
}

LUA_FN(GetTimeToWellRested)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetTimeToWellRester()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushnumber(L, 3600); //Number of seconds ?
	return 1;
}

LUA_FN(IsResting)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: IsResting()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, true);
	return 1;
}


LUA_FN(QuitGame)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: QuitGame()");
	exit(EXIT_SUCCESS);
	return 0;
}

LUA_FN(LaunchURL)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: LaunchURL()");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(SetCurrentScreen)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: SetCurrentScreen(screen)");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(GetMovieSubtitles)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetMovieSubtitles()");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(AcceptEULA)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: AcceptEULA()");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(EULAAccepted)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: EULAAccepted()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, true);
	return 1;
}

LUA_FN(TOSAccepted)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: TOSAccepted()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, true);
	return 1;
}

LUA_FN(TerminationWithoutNoticeAccepted)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: TerminationWithoutNoticeAccepted()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, true);
	return 1;
}

LUA_FN(ScanningAccepted)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: ScanningAccepted()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, true);
	return 1;
}

LUA_FN(ContestAccepted)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: ContestAccepted()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, true);
	return 1;
}

LUA_FN(IsScanDLLFinished)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: IsScanDLLFinished()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, true);
	return 1;
}

LUA_FN(GetMovieResolution)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetMovieResolution()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushnumber(L, 1920);
	return 1;
}

LUA_FN(GetActionBarPage)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetActionBarPage()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushinteger(L, 1);
	return 1;
}

LUA_FN(ChangeActionBarPage)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: ChangeActionBarPage(page)");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(GetSendMailPrice)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetSendMailPrice()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushnumber(L, 5000);
	return 1;
}

static const char *slot_textures[] =
{
	"Interface/PaperDoll/UI-PaperDoll-Slot-Ranged.blp",
	"Interface/PaperDoll/UI-PaperDoll-Slot-Head.blp",
	"Interface/PaperDoll/UI-PaperDoll-Slot-Neck.blp",
	"Interface/PaperDoll/UI-PaperDoll-Slot-Shoulder.blp",
	"Interface/PaperDoll/UI-PaperDoll-Slot-Shirt.blp",
	"Interface/PaperDoll/UI-PaperDoll-Slot-Chest.blp",
	"Interface/PaperDoll/UI-PaperDoll-Slot-Waist.blp",
	"Interface/PaperDoll/UI-PaperDoll-Slot-Legs.blp",
	"Interface/PaperDoll/UI-PaperDoll-Slot-Feet.blp",
	"Interface/PaperDoll/UI-PaperDoll-Slot-Wrists.blp",
	"Interface/PaperDoll/UI-PaperDoll-Slot-Hands.blp",
	"Interface/PaperDoll/UI-PaperDoll-Slot-Finger.blp",
	"Interface/PaperDoll/UI-PaperDoll-Slot-RFinger.blp",
	"Interface/PaperDoll/UI-PaperDoll-Slot-Trinket.blp",
	"Interface/PaperDoll/UI-PaperDoll-Slot-Trinket.blp",
	"Interface/PaperDoll/UI-PaperDoll-Slot-Chest.blp",
	"Interface/PaperDoll/UI-PaperDoll-Slot-MainHand.blp",
	"Interface/PaperDoll/UI-PaperDoll-Slot-SecondaryHand.blp",
	"Interface/PaperDoll/UI-PaperDoll-Slot-Ranged.blp",
	"Interface/PaperDoll/UI-PaperDoll-Slot-Tabard.blp",
	"Interface/PaperDoll/UI-PaperDoll-Slot-Bag.blp",
	"Interface/PaperDoll/UI-PaperDoll-Slot-Bag.blp",
	"Interface/PaperDoll/UI-PaperDoll-Slot-Bag.blp",
	"Interface/PaperDoll/UI-PaperDoll-Slot-Bag.blp",
};

static bool invslot_bag(char *buf, size_t buf_size, int slot)
{
	int bag = slot - INVSLOT_BAG0;
	uint64_t guid = object_fields_get_u64(&((struct object*)g_wow->player)->fields, PLAYER_FIELD_INV_SLOT_BAG0 + bag * 2);
	if (guid == 0)
		return false;
	struct item *item = object_get_item(guid);
	if (item == NULL)
	{
		LOG_ERROR("item not found");
		return false;
	}
	uint32_t item_template = object_fields_get_u32(&((struct object*)item)->fields, OBJECT_FIELD_ENTRY);
	struct wow_dbc_row item_row;
	if (!dbc_get_row_indexed(g_wow->dbc.item, &item_row, item_template))
	{
		LOG_ERROR("unknown item: %" PRIu32, item_template);
		return false;
	}
	struct wow_dbc_row item_display_row;
	if (!dbc_get_row_indexed(g_wow->dbc.item_display_info, &item_display_row, wow_dbc_get_u32(&item_row, 4)))
	{
		LOG_ERROR("unknown item display: %" PRIu32, item_template);
		return false;
	}
	snprintf(buf, buf_size, "Interface/Icons/%s", wow_dbc_get_str(&item_display_row, 20));
	return true;
}

LUA_FN(GetInventorySlotInfo)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: GetInventorySlotInfo(\"slot\")");
	if (!lua_isstring(L, 1))
		return luaL_argerror(L, 1, "string expected");
	LUA_UNIMPLEMENTED_FN();
	const char *str = lua_tostring(L, 1);
	char *s = mem_strdup(MEM_LUA, str);
	if (!s)
	{
		LOG_ERROR("allocation failed");
		return 0;
	}
	for (char *t = s; *t; t++)
		*t = toupper(*t);
	enum inventory_slot slot;
	if (!inventory_slot_from_string(s, &slot))
	{
		char buf[256];
		snprintf(buf, sizeof(buf), "unknown slot: %s", s);
		mem_free(MEM_LUA, s);
		return luaL_argerror(L, 1, buf);
	}
	mem_free(MEM_LUA, s);
	if (slot >= INVSLOT_BAG0 && slot <= INVSLOT_BAG3)
	{
		char buf[256];
		if (invslot_bag(buf, sizeof(buf), slot))
		{
			lua_pushinteger(L, slot); //slot
			lua_pushstring(L, buf);
			lua_pushboolean(L, false); //CheckRelic
			return 3;
		}
	}
	lua_pushinteger(L, slot); //slot
	lua_pushstring(L, slot_textures[slot]); //Texture
	lua_pushboolean(L, false); //CheckRelic
	return 3;
}

LUA_FN(GetTabardCreationCost)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetTabardCreationCost()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushnumber(L, 5005000);
	return 1;
}

LUA_FN(GetBindingKey)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: GetBindingKey(\"action\")");
	if (!lua_isstring(L, 1))
		return luaL_argerror(L, 1, "string expected");
	LUA_UNIMPLEMENTED_FN();
	lua_pushstring(L, "L");
	return 1;
}

LUA_FN(GetActionTexture)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: GetActionTexture(slot)");
	LUA_UNIMPLEMENTED_FN();
	lua_pushstring(L, "Interface\\Icons\\Spell_Shadow_SoulGem.blp");
	return 1;
}

LUA_FN(HasAction)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: HasAction(slot)");
	int slot = lua_tointeger(L, 1);
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, slot < 10);
	return 1;
}

LUA_FN(IsCurrentAction)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: IsCurrentAction(slot)");
	int slot = lua_tointeger(L, 1);
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, slot == 6);
	return 1;
}

LUA_FN(IsAutoRepeatAction)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: IsAutoRepeatAction(slot)");
	int slot = lua_tointeger(L, 1);
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, slot % 3);
	return 1;
}

LUA_FN(IsUsableAction)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: IsUsableAction(slot)");
	int slot = lua_tointeger(L, 1);
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, slot % 2);
	return 1;
}

LUA_FN(GetActionCooldown)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: GetActionCooldown(slot)");
	LUA_UNIMPLEMENTED_FN();
	lua_pushnumber(L, 0);
	return 1;
}

LUA_FN(IsAttackAction)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: IsAttackAction(slot)");
	lua_pushboolean(L, false);
	return 1;
}

LUA_FN(IsEquippedAction)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: IsEquippedAction(slot)");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, false);
	return 1;
}

LUA_FN(IsConsumableAction)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: IsConsumableAction(slot)");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, false);
	return 1;
}

LUA_FN(IsStackableAction)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: IsStackableAction(slot)");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, false);
	return 1;
}

LUA_FN(GetActionText)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: GetActionText(slot)");
	LUA_UNIMPLEMENTED_FN();
	char tmp[256];
	snprintf(tmp, sizeof(tmp), "slot %d", (int)lua_tointeger(L, 1));
	lua_pushstring(L, tmp);
	return 1;
}

LUA_FN(GetMinimapZoneText)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetMinimapZoneText()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushstring(L, "Shattrath");
	return 1;
}

LUA_FN(GetZonePVPInfo)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetZonePVPInfo()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushstring(L, "sanctuary");
	lua_pushboolean(L, false);
	lua_pushstring(L, "");
	return 3;
}

LUA_FN(GetZoneText)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetZoneText()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushstring(L, "ZoneText");
	return 1;
}

LUA_FN(GetSubZoneText)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetSubZoneText()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushstring(L, "SubZoneText");
	return 1;
}

LUA_FN(IsInInstance)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: IsInInstance()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, false);
	return 1;
}

LUA_FN(GetTime)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetTime()");
	lua_pushnumber(L, g_wow->frametime / 1000000000);
	return 1;
}

LUA_FN(GetMoney)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetMoney()");
	lua_pushnumber(L, object_fields_get_u32(&((struct object*)g_wow->player)->fields, PLAYER_FIELD_COINAGE));
	return 1;
}

LUA_FN(GetCursorMoney)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetCursorMoney()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushnumber(L, 0);
	return 1;
}

LUA_FN(GetPlayerTradeMoney)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetPlayerTradeMoney()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushnumber(L, 0);
	return 1;
}

LUA_FN(GetTargetTradeMoney)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetTargetTradeMoney()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushnumber(L, 0);
	return 1;
}

LUA_FN(ShowingCloak)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: ShowingCloak()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, true);
	return 1;
}

LUA_FN(ShowingHelm)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: ShowingHelm()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, true);
	return 1;
}

LUA_FN(GetBonusBarOffset)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetBonusBarOffset()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushinteger(L, 1);
	return 1;
}

LUA_FN(GetCurrentTitle)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetCurrentTitle()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushinteger(L, 42);
	return 1;
}

LUA_FN(GetNumTitles)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetNumTitles()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushnumber(L, 0);
	return 1;
}

LUA_FN(IsTitleKnown)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: IsTitleKnown()");
	if (!lua_isinteger(L, 1))
		return luaL_argerror(L, 1, "integer expected");
	LUA_UNIMPLEMENTED_FN();
	int title = lua_tointeger(L, 1);
	if (title == 42)
		lua_pushboolean(L, true);
	else
		lua_pushboolean(L, false);
	return 1;
}

LUA_FN(GetTitleName)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: GetTitleName(title)");
	if (!lua_isinteger(L, 1))
		return luaL_argerror(L, 1, "integer expected");
	LUA_UNIMPLEMENTED_FN();
	int title = lua_tointeger(L, 1);
	if (title == 42)
	{
		lua_pushstring(L, "Gladiator %s");
		return 1;
	}
	return 0;
}

LUA_FN(GetActionBarToggles)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetActionBarToggles()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, true); //Bottom left
	lua_pushboolean(L, true); //Bottom right
	lua_pushboolean(L, true); //Right 1
	lua_pushboolean(L, true); //Right 2
	return 4;
}

LUA_FN(SetupFullscreenScale)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: SetupFullscreenScale(frame)");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(RestartGx)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: RestartGx()");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(InCinematic)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: InCinematic()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, false);
	return 1;
}

LUA_FN(GetDefaultLanguage)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetDefaultLanguage()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushstring(L, "Common");
	return 1;
}

LUA_FN(IsPossessBarVisible)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: IsPossessBarVisible()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, true);
	return 1;
}

LUA_FN(GetPossessInfo)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: GetPossessInfo(possessBar)");
	LUA_UNIMPLEMENTED_FN();
	lua_pushstring(L, "Interface/Minimap/Tracking/Ammunition.blp"); //texture
	lua_pushstring(L, "workaround"); //name
	return 2;
}

struct
{
	const char *name;
	const char *texture;
} trackings[14] =
{
	{"Ammunition", "Interface/Minimap/Tracking/Ammunition.blp"},
	{"Auctioneer", "Interface/Minimap/Tracking/Auctioneer.blp"},
	{"Banker", "Interface/Minimap/Tracking/Banker.blp"},
	{"BattleMaster", "Interface/Minimap/Tracking/BattleMaster.blp"},
	{"Class", "Interface/Minimap/Tracking/Class.blp"},
	{"FlightMaster", "Interface/Minimap/Tracking/FlightMaster.blp"},
	{"Food", "Interface/Minimap/Tracking/Food.blp"},
	{"InnKeeper", "Interface/Minimap/Tracking/InnKeeper.blp"},
	{"Mailbox", "Interface/Minimap/Tracking/Mailbox.blp"},
	{"Poisons", "Interface/Minimap/Tracking/Poisons.blp"},
	{"Profession", "Interface/Minimap/Tracking/Profession.blp"},
	{"Reagents", "Interface/Minimap/Tracking/Reagents.blp"},
	{"Repair", "Interface/Minimap/Tracking/Repair.blp"},
	{"StableMaster", "Interface/Minimap/Tracking/StableMaster.blp"},
};

LUA_FN(GetNumTrackingTypes)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetNumTrackingTypes()");
	lua_pushnumber(L, sizeof(trackings) / sizeof(*trackings));
	return 1;
}

static int tracking_method = 0;

LUA_FN(GetTrackingTexture)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetTrackingTexture()");
	lua_pushstring(L, trackings[tracking_method].texture);
	return 1;
}

LUA_FN(GetTrackingInfo)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: GetTrackingInfo(tracking)");
	if (!lua_isnumber(L, 1))
		return luaL_argerror(L, 1, "number expected");
	LUA_UNIMPLEMENTED_FN();
	int nb = lua_tonumber(L, 1);
	if (nb < 1 || nb > 14)
		return luaL_argerror(L, 1, "invalid tracking id");
	lua_pushstring(L, trackings[nb - 1].name);
	lua_pushstring(L, trackings[nb - 1].texture);
	lua_pushboolean(L, tracking_method == nb - 1);
	lua_pushstring(L, "other");
	return 4;
}

LUA_FN(SetTracking)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc < 0)
		return luaL_error(L, "Usage: SetTracking(tracking)");
	LUA_UNIMPLEMENTED_FN();
	if (!argc)
	{
		tracking_method = 0;
	}
	else
	{
		if (!lua_isnumber(L, 1))
			return luaL_argerror(L, 1, "number expected");
		int nb = lua_tonumber(L, 1);
		if (nb < 1 || nb > 14)
			return luaL_argerror(L, 1, "invalid tracking id");
		tracking_method = nb - 1;
	}
	interface_execute_event(g_wow->interface, EVENT_MINIMAP_UPDATE_TRACKING, 0);
	return 0;
}

LUA_FN(DropCursorMoney)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: DropCursorMoney()");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(PutItemInBackpack)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: PutItemInBackpack()");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(InRepairMode)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: InRepairMode()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, false);
	return 1;
}

static struct container *get_container(int slot)
{
	struct object *player = (struct object*)g_wow->player;
	uint64_t guid = object_fields_get_u64(&player->fields, PLAYER_FIELD_INV_SLOT_BAG0 + 2 * slot);
	if (!guid)
		return NULL;
	return object_get_container(guid);
}

LUA_FN(GetContainerNumSlots)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: GetContainerNumSlots(container)");
	if (!lua_isinteger(L, 1))
		return luaL_argerror(L, 1, "integer expected");
	int bag = lua_tointeger(L, 1);
	if (!bag)
	{
		lua_pushinteger(L, 16);
		return 1;
	}
	struct container *container = get_container(bag - 1);
	if (!container)
	{
		lua_pushinteger(L, 0);
		return 1;
	}
	lua_pushinteger(L, object_fields_get_u32(&((struct object*)container)->fields, CONTAINER_FIELD_NUM_SLOTS));
	return 1;
}

LUA_FN(GetBagName)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: GetBagName(bag)");
	if (!lua_isinteger(L, 1))
		return luaL_argerror(L, 1, "integer expected");
	int bag = lua_tointeger(L, 1);
	if (!bag)
	{
		lua_pushstring(L, "Backpack");
		return 1;
	}
	struct container *container = get_container(bag - 1);
	if (!container)
	{
		LOG_ERROR("no container");
		goto err;
	}
	uint32_t item_template = object_fields_get_u32(&((struct object*)container)->fields, OBJECT_FIELD_ENTRY);
	const struct wow_wdb_item *wdb_item = wdb_get_item(g_wow->wdb, item_template);
	if (!wdb_item)
	{
		LOG_ERROR("no db item");
		goto err;
	}
	lua_pushstring(L, wdb_item->name[0]);
	return 1;

err:
	lua_pushnil(L);
	return 1;
}

LUA_FN(SetBagPortraitTexture)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 2)
		return luaL_error(L, "Usage: SetBagPortraitTexture(frame, id)");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(GetContainerItemInfo)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 2)
		return luaL_error(L, "Usage: GetContainerItemInfo(bag, item)");
	LUA_UNIMPLEMENTED_FN();
	int bag = lua_tointeger(L, 1);
	int slot = lua_tointeger(L, 2);
	if (slot <= 0)
		return luaL_argerror(L, 2, "invalid slot 0");
	--slot;
	uint64_t guid;
	if (!bag)
	{
		struct object *player = (struct object*)g_wow->player;
		guid = object_fields_get_u64(&player->fields, PLAYER_FIELD_PACK_SLOT_1 + 2 * slot);
	}
	else
	{
		struct container *container = get_container(bag - 1);
		if (!container)
		{
			LOG_ERROR("no container");
			goto err;
		}
		guid = object_fields_get_u64(&((struct object*)container)->fields, CONTAINER_FIELD_SLOT_1 + slot);
	}
	LOG_INFO("bag %d slot %d guid: %" PRIu64, bag, slot, guid);
	if (!guid)
		goto err;
	struct item *item = object_get_item(guid);
	if (!item)
	{
		LOG_ERROR("item not found");
		goto err;
	}
	uint32_t item_template = object_fields_get_u32(&((struct object*)item)->fields, OBJECT_FIELD_ENTRY);
	struct wow_dbc_row item_row;
	if (!dbc_get_row_indexed(g_wow->dbc.item, &item_row, item_template))
	{
		LOG_ERROR("unknown item: %" PRIu32, item_template);
		goto err;
	}
	struct wow_dbc_row item_display_row;
	if (!dbc_get_row_indexed(g_wow->dbc.item_display_info, &item_display_row, wow_dbc_get_u32(&item_row, 4)))
	{
		LOG_ERROR("unknown item display: %" PRIu32, item_template);
		goto err;
	}
	char tmp[512];
	snprintf(tmp, sizeof(tmp), "Interface/Icons/%s", wow_dbc_get_str(&item_display_row, 20));
	lua_pushstring(L, tmp);
	const struct wow_wdb_item *wdb_item = wdb_get_item(g_wow->wdb, item_template);
	lua_pushinteger(L, object_fields_get_u32(&((struct object*)item)->fields, ITEM_FIELD_STACK_COUNT));
	uint32_t flags = object_fields_get_u32(&((struct object*)item)->fields, ITEM_FIELD_FLAGS);
	lua_pushboolean(L, (flags & ITEM_DYNFLAG_UNLOCKED) == 0);
	lua_pushinteger(L, wdb_item ? wdb_item->quality : 0);
	lua_pushboolean(L, (flags & ITEM_DYNFLAG_READABLE) != 0);
	return 5;

err:
	lua_pushnil(L);
	return 1;
}

LUA_FN(GetContainerItemCooldown)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 2)
		return luaL_error(L, "Usage: GetContainerItemCooldown(container, item)");
	LUA_UNIMPLEMENTED_FN();
	lua_pushnumber(L, 0); //Start
	lua_pushnumber(L, 10); //Duration
	lua_pushboolean(L, false); //Enabled
	return 3;
}

LUA_FN(GetTotemInfo)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: GetTotemInfo()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, false); //Has
	lua_pushstring(L, ""); //Name
	lua_pushinteger(L, 1); //Start
	lua_pushinteger(L, 2); //Duration
	lua_pushstring(L, ""); //Icon
	return 5;
}

LUA_FN(GetCurrentDungeonDifficulty)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetCurrentDungeonDifficulty()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushinteger(L, 1);
	return 1;
}

LUA_FN(GetCreditsText)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage :GetCreditsText(\"credits\")");
	LUA_UNIMPLEMENTED_FN();
	lua_pushstring(L, "Ceci est un texte de credits");
	return 1;
}

LUA_FN(SetCharSelectModelFrame)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: SetCharSelectModelFrame(\"frame\")");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(SetCharCustomizeFrame)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: SetCharCustomizeFrame(\"frame\")");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(AcceptChangedOptionWarnings)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: AcceptChangedOptionWarnings()");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}


LUA_FN(GetNumShapeshiftForms)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetNumShapeshiftForms()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushinteger(L, 3);
	return 1;
}

LUA_FN(GetShapeshiftFormInfo)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: GetShapeshiftFormInfo(form)");
	LUA_UNIMPLEMENTED_FN();
	lua_pushstring(L, "Interface/Minimap/Tracking/Auctioneer.blp"); //texture
	lua_pushstring(L, "laform"); //name
	lua_pushboolean(L, false); //isactive
	lua_pushboolean(L, true); //iscastable
	return 4;
}

LUA_FN(GetShapeshiftFormCooldown)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: GetShapeshiftFormCooldown(form)");
	LUA_UNIMPLEMENTED_FN();
	lua_pushnumber(L, 0); //start
	lua_pushnumber(L, 2); //duration
	lua_pushboolean(L, false); //enable
	return 3;
}

LUA_FN(UpdateAddOnMemoryUsage)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: UpdateAddOnMemoryUsage()");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(DoEmote)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc < 1 || argc > 2)
		return luaL_error(L, "Usage: DoEmote(\"emote\"[, \"msg\"])");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(GetNumFactions)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetNumFactions()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushinteger(L, 5);
	return 1;
}

LUA_FN(CollapseFactionHeader)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: CollapseFactionHeader(index)");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(ExpandFactionHeader)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: ExpandFactionHeader(index)");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(GetFactionInfo)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: GetFactionInfo()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushstring(L, "test name"); //name
	lua_pushstring(L, "test description"); //description
	lua_pushinteger(L, 1); //standingID
	lua_pushinteger(L, 1); //barMin
	lua_pushinteger(L, 100); //barMax
	lua_pushinteger(L, 20); //barValue
	lua_pushboolean(L, false); //atWarWith
	lua_pushboolean(L, false); //canToggleAtWar
	lua_pushboolean(L, false); //isHeader
	lua_pushboolean(L, false); //isCollapsed
	lua_pushboolean(L, false); //isWatched
	return 11;
}

LUA_FN(RegisterForSavePerCharacter)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: RegisterForSavePerCharacter(\"var\")");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(GetGameTime)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetGameTime()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushinteger(L, 12);
	lua_pushinteger(L, 25);
	return 2;
}

LUA_FN(IsActionInRange)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: IsActionInRange(index)");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, false);
	return 1;
}

LUA_FN(RequestBattlefieldPositions)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: RequestBattlefieldPositions()");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(CheckReadyCheckTime)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: CheckReadyCheckTime()");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(GetWeaponEnchantInfo)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetWeaponEnchantInfo()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, true); //hasMainHandEnchant
	lua_pushinteger(L, 52); //mainHandExpiration
	lua_pushinteger(L, 2); //mainHandCharges
	lua_pushboolean(L, true); //hasOffHandEnchant
	lua_pushinteger(L, 59); //offHandExpiration
	lua_pushinteger(L, 5); //offHandCharges
	return 6;
}

LUA_FN(GetNumWorldStateUI)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetNumWorldStateUI()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushinteger(L, 1);
	return 1;
}

LUA_FN(GetWorldStateUIInfo)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: GetWorldStateUIInfo(index)");
	LUA_UNIMPLEMENTED_FN();
	lua_pushinteger(L, 1); //uiType: 0 or 1 ?
	lua_pushinteger(L, 2); //state: 1, 2, 3
	lua_pushstring(L, "test"); //text
	lua_pushstring(L, "Interface/Minimap/Tracking/Auctioneer.blp"); //icon
	lua_pushstring(L, "Interface/Minimap/Tracking/Auctioneer.blp"); //dynamicIcon
	lua_pushstring(L, "tooltip"); //tooltip
	lua_pushstring(L, "test"); //dynamicTooltip
	lua_pushstring(L, "CAPTUREPOINT"); //extendedUI: CAPTUREPOINT
	lua_pushinteger(L, 10); //state1; CAPTUREPOINT: id
	lua_pushinteger(L, 25); //state2; CAPTUREPOINT: value
	lua_pushinteger(L, 50); //state3; CAPTUREPOINT: neutralPercent
	return 11;
}

LUA_FN(IsSubZonePVPPOI)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: IsSubZonePVPPOI()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, true);
	return 1;
}

LUA_FN(GetGMTicketCategories)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetGMTicketCategories()");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(GetAttackPowerForStat)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 2)
		return luaL_error(L, "Usage: GetAttackPowerForStat(index, amount)");
	LUA_UNIMPLEMENTED_FN();
	lua_pushnumber(L, 0);
	return 1;
}

LUA_FN(GetCombatRating)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: GetCombatRating(id)");
	LUA_UNIMPLEMENTED_FN();
	lua_pushnumber(L, 0);
	return 1;
}

LUA_FN(GetRepairAllCost)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetInventoryItemCooldown()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushnumber(L, 1337);
	lua_pushboolean(L, 1);
	return 2;
}

LUA_FN(IsInventoryItemLocked)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: IsInventoryItemLocked(id)");
	int id = lua_tointeger(L, 1);
	LUA_UNIMPLEMENTED_FN();
	lua_pushnumber(L, id % 6 == 5);
	return 1;
}

LUA_FN(ShowHelm)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: ShowHelm(show)");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(TutorialsEnabled)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: TutorialsEnabled()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, 1);
	return 1;
}

LUA_FN(GetCombatRatingBonus)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: GetCombatRatingBonus(id)");
	LUA_UNIMPLEMENTED_FN();
	lua_pushnumber(L, 14);
	return 1;
}

LUA_FN(GetText)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 2)
		return luaL_error(L, "Usage: GetText(\"text\", gender)");
	LUA_UNIMPLEMENTED_FN();
	lua_pushstring(L, lua_tostring(L, -1));
	return 1;
}

LUA_FN(GetSelectedFaction)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetSelectedFaction()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushnumber(L, 0);
	return 1;
}

LUA_FN(PutItemInBag)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: PutItemInBag(slot)");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, false);
	return 1;
}

LUA_FN(CreateMiniWorldMapArrowFrame)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: CreateMiniWorldMapArrowFrame(Frame)");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(UseAction)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 3)
		return luaL_error(L, "Usage: UseAction(\"action\", \"unit\", \"button\")");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(CloseMerchant)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: CloseMerchant()");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(CloseTrade)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: CloseTrade()");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(CloseItemText)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: CloseItemText()");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(CloseTaxiMap)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: CloseTaxiMap()");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(CloseTabardCreation)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: CloseTabardCreation()");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(CloseGuildRegistrar)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: CloseGuildRegistrar()");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(ClosePetition)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: ClosePetition()");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(CloseGossip)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: CloseGossip()");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(CloseMail)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: CloseMail()");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(ClosePetStables)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: ClosePetStables()");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(ClosePetitionVendor)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: ClosePetitionVendor()");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(CloseCraft)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: CloseCraft()");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(CloseGuildBankFrame)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: CloseGuildBankFrame()");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(CloseSocketInfo)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: CloseSocketInfo()");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(CloseTradeSkill)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: CloseTradeSkill()");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(CloseTrainer)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: CloseTrainer()");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(CloseLoot)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: CloseLoot()");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(CombatLog_Object_IsA)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 2)
		return luaL_error(L, "Usage: CombatLog_Object_IsA(flag1, flag2)");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, true);
	return 1;
}

LUA_FN(ClearInspectPlayer)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: ClearInspectPlayer()");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(ShowCloak)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc < 0 || argc > 1)
		return luaL_error(L, "Usage: ShowCloak([show])");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(GetCursorInfo)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetCursorInfo()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushstring(L, "money"); //money, guildbankmoney, merchant
	lua_pushnumber(L, 1337);
	return 0;
}

LUA_FN(PickupInventoryItem)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: PickupInventoryItem(slot)");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(HasKey)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: HasKey()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, 1);
	return 1;
}

LUA_FN(CursorHasItem)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: CursorHasItem()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, true);
	return 1;
}

LUA_FN(PartialPlayTime)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: PartialPlayTime()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, false);
	return 1;
}

LUA_FN(NoPlayTime)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: NoPlayTime()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, false);
	return 1;
}

LUA_FN(IsLoggedIn)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: IsLoggedIn()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, true);
	return 1;
}

LUA_FN(GetContainerNumFreeSlots)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: GetContainerNumFreeSlots(container)");
	if (!lua_isinteger(L, 1))
		return luaL_argerror(L, 1, "integer expected");
	/* XXX */
	int bag = lua_tointeger(L, 1);
	if (!bag)
	{
		lua_pushinteger(L, 16);
		return 1;
	}
	struct container *container = get_container(bag - 1);
	if (!container)
	{
		lua_pushinteger(L, 0);
		return 1;
	}
	lua_pushinteger(L, object_fields_get_u32(&((struct object*)container)->fields, CONTAINER_FIELD_NUM_SLOTS));
	return 1;
}

LUA_FN(SetActionBarToggles)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 5)
		return luaL_error(L, "Usage: SetActionBarToggles(bottomLeftState, bottomRightSTate, sideRightState, sideRight2State, alwaysShow)");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(GetInventoryAlertStatus)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: GetInventoryAlertStatus(index)");
	LUA_UNIMPLEMENTED_FN();
	lua_pushnumber(L, 0);
	return 1;
}

LUA_FN(SetEuropeanNumbers)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: SetEuropeanNumbers(flag)");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(HideNameplates)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: HideNameplates()");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(ShowNameplates)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: ShowNameplates()");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(HideFriendNameplates)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: HideFriendNameplates()");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(ShowFriendNameplates)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: ShowFriendNameplates()");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(GetMirrorTimerInfo)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: GetMirrorTimerInfo(index)");
	lua_pushstring(L, "UNKNOWN"); /* timer */
	lua_pushinteger(L, 1); /* value */
	lua_pushinteger(L, 5); /* maxvalue */
	lua_pushnumber(L, 5); /* scale */
	lua_pushboolean(L, false); /* paused */
	lua_pushstring(L, "test"); /* label */
	return 6;
}

LUA_FN(OffhandHasWeapon)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: OffhandHasWeapon()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, true);
	return 1;
}

LUA_FN(InCombatLockdown)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: InCombatLockdown()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, false);
	return 1;
}

LUA_FN(CancelItemTempEnchantment)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: CancelItemTempEnchantment(weaponHand)");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(GetTrainerServiceTypeFilter)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: GetTrainerServiceTypeFilter(\"filter\")");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(MovieRecording_GetAspectRatio)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: MovieRecording_GetAspectRatio()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushnumber(L, 1);
	return 1;
}

LUA_FN(MovieRecording_IsCodecSupported)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: MovieRecording_IsCodecSupported(\"codec\")");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, 1);
	return 1;
}

LUA_FN(MovieRecording_SearchUncompressedMovie)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: MovieRecording_SearchUncompressedMovie(?)");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

void register_misc_functions(lua_State *L)
{
	LUA_REGISTER_FN(GetItemQualityColor);
	LUA_REGISTER_FN(CreateFrame);
	LUA_REGISTER_FN(geterrorhandler);
	LUA_REGISTER_FN(seterrorhandler);
	LUA_REGISTER_FN(SetPortraitTexture);
	LUA_REGISTER_FN(PlayerHasSpells);
	LUA_REGISTER_FN(CloseBankFrame);
	LUA_REGISTER_FN(GetCVar);
	LUA_REGISTER_FN(GetCVarDefault);
	LUA_REGISTER_FN(SetCVar);
	LUA_REGISTER_FN(RegisterForSave);
	LUA_REGISTER_FN(GetRestState);
	LUA_REGISTER_FN(GetXPExhaustion);
	LUA_REGISTER_FN(GetTimeToWellRested);
	LUA_REGISTER_FN(IsResting);
	LUA_REGISTER_FN(QuitGame);
	LUA_REGISTER_FN(LaunchURL);
	LUA_REGISTER_FN(SetCurrentScreen);
	LUA_REGISTER_FN(GetMovieSubtitles);
	LUA_REGISTER_FN(AcceptEULA);
	LUA_REGISTER_FN(EULAAccepted);
	LUA_REGISTER_FN(TOSAccepted);
	LUA_REGISTER_FN(TerminationWithoutNoticeAccepted);
	LUA_REGISTER_FN(ScanningAccepted);
	LUA_REGISTER_FN(ContestAccepted);
	LUA_REGISTER_FN(IsScanDLLFinished);
	LUA_REGISTER_FN(GetMovieResolution);
	LUA_REGISTER_FN(GetActionBarPage);
	LUA_REGISTER_FN(ChangeActionBarPage);
	LUA_REGISTER_FN(GetSendMailPrice);
	LUA_REGISTER_FN(GetInventorySlotInfo);
	LUA_REGISTER_FN(GetTabardCreationCost);
	LUA_REGISTER_FN(GetBindingKey);
	LUA_REGISTER_FN(GetActionTexture);
	LUA_REGISTER_FN(HasAction);
	LUA_REGISTER_FN(IsCurrentAction);
	LUA_REGISTER_FN(IsAutoRepeatAction);
	LUA_REGISTER_FN(IsUsableAction);
	LUA_REGISTER_FN(GetActionCooldown);
	LUA_REGISTER_FN(IsAttackAction);
	LUA_REGISTER_FN(IsEquippedAction);
	LUA_REGISTER_FN(IsConsumableAction);
	LUA_REGISTER_FN(IsStackableAction);
	LUA_REGISTER_FN(GetActionText);
	LUA_REGISTER_FN(GetMinimapZoneText);
	LUA_REGISTER_FN(GetZonePVPInfo);
	LUA_REGISTER_FN(GetZoneText);
	LUA_REGISTER_FN(GetSubZoneText);
	LUA_REGISTER_FN(IsInInstance);
	LUA_REGISTER_FN(GetTime);
	LUA_REGISTER_FN(GetMoney);
	LUA_REGISTER_FN(GetCursorMoney);
	LUA_REGISTER_FN(GetPlayerTradeMoney);
	LUA_REGISTER_FN(GetTargetTradeMoney);
	LUA_REGISTER_FN(ShowingCloak);
	LUA_REGISTER_FN(ShowingHelm);
	LUA_REGISTER_FN(GetBonusBarOffset);
	LUA_REGISTER_FN(GetCurrentTitle);
	LUA_REGISTER_FN(GetNumTitles);
	LUA_REGISTER_FN(IsTitleKnown);
	LUA_REGISTER_FN(GetTitleName);
	LUA_REGISTER_FN(GetActionBarToggles);
	LUA_REGISTER_FN(SetupFullscreenScale);
	LUA_REGISTER_FN(RestartGx);
	LUA_REGISTER_FN(InCinematic);
	LUA_REGISTER_FN(GetDefaultLanguage);
	LUA_REGISTER_FN(IsPossessBarVisible);
	LUA_REGISTER_FN(GetPossessInfo);
	LUA_REGISTER_FN(GetNumTrackingTypes);
	LUA_REGISTER_FN(GetTrackingTexture);
	LUA_REGISTER_FN(GetTrackingInfo);
	LUA_REGISTER_FN(SetTracking);
	LUA_REGISTER_FN(DropCursorMoney);
	LUA_REGISTER_FN(PutItemInBackpack);
	LUA_REGISTER_FN(InRepairMode);
	LUA_REGISTER_FN(GetContainerNumSlots);
	LUA_REGISTER_FN(GetBagName);
	LUA_REGISTER_FN(SetBagPortraitTexture);
	LUA_REGISTER_FN(GetContainerItemInfo);
	LUA_REGISTER_FN(GetContainerItemCooldown);
	LUA_REGISTER_FN(GetTotemInfo);
	LUA_REGISTER_FN(GetCurrentDungeonDifficulty);
	LUA_REGISTER_FN(GetCreditsText);
	LUA_REGISTER_FN(SetCharSelectModelFrame);
	LUA_REGISTER_FN(SetCharCustomizeFrame);
	LUA_REGISTER_FN(AcceptChangedOptionWarnings);
	LUA_REGISTER_FN(GetNumShapeshiftForms);
	LUA_REGISTER_FN(GetShapeshiftFormInfo);
	LUA_REGISTER_FN(GetShapeshiftFormCooldown);
	LUA_REGISTER_FN(UpdateAddOnMemoryUsage);
	LUA_REGISTER_FN(DoEmote);
	LUA_REGISTER_FN(GetNumFactions);
	LUA_REGISTER_FN(CollapseFactionHeader);
	LUA_REGISTER_FN(ExpandFactionHeader);
	LUA_REGISTER_FN(GetFactionInfo);
	LUA_REGISTER_FN(RegisterForSavePerCharacter);
	LUA_REGISTER_FN(GetGameTime);
	LUA_REGISTER_FN(IsActionInRange);
	LUA_REGISTER_FN(RequestBattlefieldPositions);
	LUA_REGISTER_FN(CheckReadyCheckTime);
	LUA_REGISTER_FN(GetWeaponEnchantInfo);
	LUA_REGISTER_FN(GetNumWorldStateUI);
	LUA_REGISTER_FN(GetWorldStateUIInfo);
	LUA_REGISTER_FN(IsSubZonePVPPOI);
	LUA_REGISTER_FN(GetGMTicketCategories);
	LUA_REGISTER_FN(GetAttackPowerForStat);
	LUA_REGISTER_FN(GetCombatRating);
	LUA_REGISTER_FN(IsInventoryItemLocked);
	LUA_REGISTER_FN(GetRepairAllCost);
	LUA_REGISTER_FN(ShowHelm);
	LUA_REGISTER_FN(TutorialsEnabled);
	LUA_REGISTER_FN(GetCombatRatingBonus);
	LUA_REGISTER_FN(GetText);
	LUA_REGISTER_FN(GetSelectedFaction);
	LUA_REGISTER_FN(PutItemInBag);
	LUA_REGISTER_FN(CreateMiniWorldMapArrowFrame);
	LUA_REGISTER_FN(UseAction);
	LUA_REGISTER_FN(CloseMerchant);
	LUA_REGISTER_FN(CloseTrade);
	LUA_REGISTER_FN(CloseItemText);
	LUA_REGISTER_FN(CloseTaxiMap);
	LUA_REGISTER_FN(CloseTabardCreation);
	LUA_REGISTER_FN(CloseGuildRegistrar);
	LUA_REGISTER_FN(ClosePetition);
	LUA_REGISTER_FN(CloseGossip);
	LUA_REGISTER_FN(CloseMail);
	LUA_REGISTER_FN(ClosePetStables);
	LUA_REGISTER_FN(ClosePetitionVendor);
	LUA_REGISTER_FN(CloseCraft);
	LUA_REGISTER_FN(CloseGuildBankFrame);
	LUA_REGISTER_FN(CloseSocketInfo);
	LUA_REGISTER_FN(CloseTradeSkill);
	LUA_REGISTER_FN(CloseTrainer);
	LUA_REGISTER_FN(CloseLoot);
	LUA_REGISTER_FN(CombatLog_Object_IsA);
	LUA_REGISTER_FN(ClearInspectPlayer);
	LUA_REGISTER_FN(ShowCloak);
	LUA_REGISTER_FN(GetCursorInfo);
	LUA_REGISTER_FN(PickupInventoryItem);
	LUA_REGISTER_FN(HasKey);
	LUA_REGISTER_FN(CursorHasItem);
	LUA_REGISTER_FN(PartialPlayTime);
	LUA_REGISTER_FN(NoPlayTime);
	LUA_REGISTER_FN(IsLoggedIn);
	LUA_REGISTER_FN(GetContainerNumFreeSlots);
	LUA_REGISTER_FN(SetActionBarToggles);
	LUA_REGISTER_FN(GetInventoryAlertStatus);
	LUA_REGISTER_FN(SetEuropeanNumbers);
	LUA_REGISTER_FN(HideNameplates);
	LUA_REGISTER_FN(ShowNameplates);
	LUA_REGISTER_FN(HideFriendNameplates);
	LUA_REGISTER_FN(ShowFriendNameplates);
	LUA_REGISTER_FN(GetMirrorTimerInfo);
	LUA_REGISTER_FN(OffhandHasWeapon);
	LUA_REGISTER_FN(InCombatLockdown);
	LUA_REGISTER_FN(CancelItemTempEnchantment);
	LUA_REGISTER_FN(GetTrainerServiceTypeFilter);
	LUA_REGISTER_FN(MovieRecording_GetAspectRatio);
	LUA_REGISTER_FN(MovieRecording_IsCodecSupported);
	LUA_REGISTER_FN(MovieRecording_SearchUncompressedMovie);
}
