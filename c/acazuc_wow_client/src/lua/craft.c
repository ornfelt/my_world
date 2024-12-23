#include "functions.h"

#include "wow_lua.h"

LUA_FN(GetCraftSlots)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetCraftSlots()");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(GetCraftFilter)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: GetCraftFilter(index)");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, 1); //checked
	return 1;
}

LUA_FN(SetCraftItemNameFilter)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: SetCraftItemNameFilter(\"filter\")");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

void register_craft_functions(lua_State *L)
{
	LUA_REGISTER_FN(GetCraftSlots);
	LUA_REGISTER_FN(GetCraftFilter);
	LUA_REGISTER_FN(SetCraftItemNameFilter);
}
