#include "functions.h"

#include "wow_lua.h"
#include "log.h"
#include "wow.h"
#include "dbc.h"

LUA_FN(GetNumMacros)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetNumMacros()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushnumber(L, 10);
	lua_pushnumber(L, 10);
	return 2;
}

LUA_FN(GetMacroInfo)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: GetMacroInfo(index)");
	LUA_UNIMPLEMENTED_FN();
	lua_pushstring(L, "test");
	lua_pushstring(L, "Interface\\Icons\\Spell_Shadow_SoulGem.blp");
	lua_pushstring(L, "/run coucou mdr");
	lua_pushboolean(L, true);
	return 4;
}

LUA_FN(EditMacro)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc < 3 || argc > 4)
		return luaL_error(L, "Usage: EditMacro(index, \"name\", \"icon\"[, \"text\"])");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(GetNumMacroIcons)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetNumMacroIcons()");
	lua_pushnumber(L, g_wow->dbc.spell_icon->file->header.record_count);
	return 1;
}

LUA_FN(GetMacroIconInfo)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: GetMacroIconInfo(index)");
	int index = lua_tointeger(L, 1);
	index--;
	if (index < 0 || (unsigned)index >= g_wow->dbc.spell_icon->file->header.record_count)
		return luaL_argerror(L, 1, "invalid index");
	struct wow_dbc_row row = dbc_get_row(g_wow->dbc.spell_icon, index);
	lua_pushstring(L, wow_dbc_get_str(&row, 4));
	return 1;
}

void register_macro_functions(lua_State *L)
{
	LUA_REGISTER_FN(GetNumMacros);
	LUA_REGISTER_FN(GetMacroInfo);
	LUA_REGISTER_FN(EditMacro);
	LUA_REGISTER_FN(GetNumMacroIcons);
	LUA_REGISTER_FN(GetMacroIconInfo);
}
