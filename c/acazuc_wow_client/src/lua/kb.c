#include "functions.h"

#include "wow_lua.h"
#include "log.h"

LUA_FN(KBSetup_BeginLoading)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 2)
		return luaL_error(L, "Usage: KBSetup_BeginLoading(articlesPerPage, page)");
	return 0;
}

LUA_FN(GetGMStatus)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetGMStatus()");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(GetGMTicket)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetGMTicket()");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(KBSystem_GetMOTD)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: KBSystem_GetMOTD()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushstring(L, "KB MOTD");
	return 1;
}

LUA_FN(KBSystem_GetServerNotice)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: KBSystem_GetServerNotice()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushstring(L, "KB notice");
	return 1;
}

LUA_FN(KBSetup_IsLoaded)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: KBSetup_IsLoaded()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, 1);
	return 1;
}

LUA_FN(KBQuery_BeginLoading)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 5)
		return luaL_error(L, "Usage: KBQuery_BeginLoading(searchText, category, subcategory, articlesPerPage, page)");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

void register_kb_functions(lua_State *L)
{
	LUA_REGISTER_FN(KBSetup_BeginLoading);
	LUA_REGISTER_FN(GetGMStatus);
	LUA_REGISTER_FN(GetGMTicket);
	LUA_REGISTER_FN(KBSystem_GetMOTD);
	LUA_REGISTER_FN(KBSystem_GetServerNotice);
	LUA_REGISTER_FN(KBSetup_IsLoaded);
	LUA_REGISTER_FN(KBQuery_BeginLoading);
}
