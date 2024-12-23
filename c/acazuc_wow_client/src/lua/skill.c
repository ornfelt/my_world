#include "functions.h"

#include "wow_lua.h"
#include "log.h"

static int selected_skill = 0;

LUA_FN(GetSelectedSkill)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetSelectedSkill()");
	lua_pushinteger(L, selected_skill);
	return 1;
}

LUA_FN(SetSelectedSkill)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: SetSelectedSkill(index)");
	selected_skill = lua_tointeger(L, 1);
	return 0;
}

LUA_FN(GetNumSkillLines)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetNumSkillLines()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushinteger(L, 3);
	return 1;
}

LUA_FN(GetAdjustedSkillPoints)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetAdjustedSkillPoints()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushinteger(L, 2);
	return 1;
}

LUA_FN(GetSkillLineInfo)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: GetSkillLineInfo(line)");
	if (!lua_isinteger(L, 1))
		return luaL_argerror(L, 1, "integer expected");
	LUA_UNIMPLEMENTED_FN();
	lua_pushstring(L, "test"); //name
	lua_pushboolean(L, false); //header
	lua_pushboolean(L, true); //isExpanded
	lua_pushinteger(L, 2); //skillRank
	lua_pushinteger(L, 1); //numTempPoints
	lua_pushinteger(L, 2); //modifier
	lua_pushinteger(L, 255); //skillMaxRank
	lua_pushboolean(L, false); //isAbandonable
	lua_pushinteger(L, 2); //stepCost
	lua_pushinteger(L, 1); //rankCost
	lua_pushinteger(L, 10); //minLevel
	lua_pushinteger(L, 1); //skillCostType: 0, 1, 2, 3
	return 12;
}

LUA_FN(CollapseSkillHeader)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: CollapseSkillHeader(index)");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(CancelSkillUps)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: CancelSkillUps()");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

void register_skill_functions(lua_State *L)
{
	LUA_REGISTER_FN(GetSelectedSkill);
	LUA_REGISTER_FN(SetSelectedSkill);
	LUA_REGISTER_FN(GetNumSkillLines);
	LUA_REGISTER_FN(GetAdjustedSkillPoints);
	LUA_REGISTER_FN(GetSkillLineInfo);
	LUA_REGISTER_FN(CollapseSkillHeader);
	LUA_REGISTER_FN(CancelSkillUps);
}
