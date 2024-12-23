#include "functions.h"

#include "wow_lua.h"
#include "log.h"

LUA_FN(GetLookingForGroup)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetLookingForGroup()");
	lua_pushinteger(L, 1); //Type1
	lua_pushstring(L, "name1"); //Name1
	lua_pushinteger(L, 2); //Type2
	lua_pushstring(L, "name2"); //Name2
	lua_pushinteger(L, 3); //Type3
	lua_pushstring(L, "name3"); //Name3
	lua_pushinteger(L, 1); //lfmType
	lua_pushstring(L, "lfm"); //lfmname
	lua_pushstring(L, "comment"); //comment
	lua_pushinteger(L, 0); //queued
	lua_pushinteger(L, 1); //lfgStatus
	lua_pushinteger(L, 1); //lfmStatus
	lua_pushinteger(L, 1); //lfmStatus
	return 13;
}

LUA_FN(GetLFGTypeEntries)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: GetLFGTypeEntries(type)");
	LUA_UNIMPLEMENTED_FN();
	lua_pushinteger(L, 1);
	return 1;
}

LUA_FN(CanSendLFGQuery)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc < 1)
		return luaL_error(L, "Usage: CanSendLFGQuery(typeID[, index])");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, true);
	return 1;
}

LUA_FN(GetLFGTypes)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetLFGTypes()");
	lua_getglobal(L, "LFG_TYPE_NONE");
	lua_getglobal(L, "LFG_TYPE_DUNGEON");
	lua_getglobal(L, "LFG_TYPE_RAID");
	lua_getglobal(L, "LFG_TYPE_QUEST");
	lua_getglobal(L, "LFG_TYPE_ZONE");
	lua_getglobal(L, "LFG_TYPE_HEROIC_DUNGEON");
	return 6;
}

LUA_FN(SetLFGComment)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: SetLFGComment(\"comment\")");
	return 0;
}

LUA_FN(GetNumLFGResults)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 2)
		return luaL_error(L, "Usage: GetNumLFGResults(LFMType, \"LFMName\")");
	LUA_UNIMPLEMENTED_FN();
	lua_pushnumber(L, 2); //results
	lua_pushnumber(L, 10); //total
	return 2;
}

LUA_FN(GetLFGResults)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 3)
		return luaL_error(L, "Usage: GetLFGResults(LFMType, \"LFMName\", index)");
	LUA_UNIMPLEMENTED_FN();
	lua_pushstring(L, "test"); //name
	lua_pushnumber(L, 1); //level
	lua_pushnumber(L, 10); //zone
	lua_pushnumber(L, 1); //class
	lua_pushstring(L, "c1"); //criteria 1
	lua_pushstring(L, "c2"); //criteria 2
	lua_pushstring(L, "c3"); //criteria 3
	lua_pushstring(L, "comment"); //comment
	lua_pushinteger(L, 5); //num party members
	lua_pushinteger(L, 1); //is lfm
	lua_pushstring(L, "WARRIOR"); //class filename
	return 11;
}

LUA_FN(SetLFMType)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: SetLFMType(LFMType)");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(ClearLookingForMore)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: ClearLookingForMore()");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(LFGQuery)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 2)
		return luaL_error(L, "Usage: LFGQuery(LFMType, \"LFMName\")");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(GetLFGPartyResults)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 4)
		return luaL_error(L, "Usage: GetLFGPartyResults(LFMType, \"LFMName\", lfgIndex, member)");
	LUA_UNIMPLEMENTED_FN();
	lua_pushstring(L, "test"); //name
	lua_pushnumber(L, 70); //level
	lua_pushstring(L, "rogue"); //class
	return 3;
}

LUA_FN(SetLookingForMore)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 2)
		return luaL_error(L, "Usage: SetLookingForMore(LFMType, \"LFMName\")");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(SortLFG)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: SortLFG(\"type\")");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(SetLFGType)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 2)
		return luaL_error(L, "Usage: SetLFGType(LFMType, \"type\")");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(SetLookingForGroup)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 3)
		return luaL_error(L, "Usage: SetLookingForGroup(LFMType, \"type\", lfgIndex)");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(ClearLFMAutofill)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: ClearLFMAutofill()");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(ClearLFGAutojoin)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: ClearLFGAutojoin()");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(SetLFMAutofill)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: SetLFMAutofill()");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

void register_lfg_functions(lua_State *L)
{
	LUA_REGISTER_FN(GetLookingForGroup);
	LUA_REGISTER_FN(GetLFGTypeEntries);
	LUA_REGISTER_FN(CanSendLFGQuery);
	LUA_REGISTER_FN(GetLFGTypes);
	LUA_REGISTER_FN(SetLFGComment);
	LUA_REGISTER_FN(GetNumLFGResults);
	LUA_REGISTER_FN(GetLFGResults);
	LUA_REGISTER_FN(SetLFMType);
	LUA_REGISTER_FN(ClearLookingForMore);
	LUA_REGISTER_FN(LFGQuery);
	LUA_REGISTER_FN(GetLFGPartyResults);
	LUA_REGISTER_FN(SetLookingForMore);
	LUA_REGISTER_FN(SortLFG);
	LUA_REGISTER_FN(SetLFGType);
	LUA_REGISTER_FN(SetLookingForGroup);
	LUA_REGISTER_FN(ClearLFMAutofill);
	LUA_REGISTER_FN(ClearLFGAutojoin);
	LUA_REGISTER_FN(SetLFMAutofill);
}
