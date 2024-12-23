#include "functions.h"

#include "wow_lua.h"
#include "log.h"

LUA_FN(GetPVPYesterdayStats)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetPVPYesterdayStats()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushinteger(L, 0); //HK
	lua_pushinteger(L, 0); //Contribution
	return 2;
}

LUA_FN(GetPVPLifetimeStats)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetPVPLifetimeStats()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushinteger(L, 0); //HK
	lua_pushinteger(L, 0); //Contribution
	return 2;
}

LUA_FN(GetHonorCurrency)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetHonorCurrency()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushinteger(L, 1337);
	return 1;
}

LUA_FN(GetPVPSessionStats)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetPVPSessionStats()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushinteger(L, 0); //HK
	lua_pushinteger(L, 0); //Contribution
	return 2;
}

LUA_FN(GetPVPRankInfo)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: GetPVPRankInfo(\"rank\")");
	lua_pushstring(L, "rank"); /* name */
	lua_pushinteger(L, 6); /* number */
	return 2;
}

LUA_FN(GetPVPRankProgress)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetPVPRankProgress()");
	lua_pushnumber(L, 0.5); /* progress from 0 to 1 */
	return 1;
}

void register_pvp_functions(lua_State *L)
{
	LUA_REGISTER_FN(GetPVPYesterdayStats);
	LUA_REGISTER_FN(GetPVPLifetimeStats);
	LUA_REGISTER_FN(GetHonorCurrency);
	LUA_REGISTER_FN(GetPVPSessionStats);
	LUA_REGISTER_FN(GetPVPRankInfo);
	LUA_REGISTER_FN(GetPVPRankProgress);
}
