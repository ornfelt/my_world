#include "functions.h"

#include "wow_lua.h"
#include "log.h"

static int selection = 0;

LUA_FN(GetQuestLogSelection)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetQuestLogSelection()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushinteger(L, selection);
	return 1;
}

LUA_FN(GetNumQuestLogEntries)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetNumQuestLogEntries()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushinteger(L, 4);
	return 1;
}

LUA_FN(GetDailyQuestsCompleted)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetDailyQuestsCompleted()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushinteger(L, 2);
	return 1;
}

LUA_FN(GetMaxDailyQuests)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetMaxDailyQuests()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushinteger(L, 5);
	return 1;
}

LUA_FN(GetQuestLogTitle)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: GetQuestLogTitle(index)");
	LUA_UNIMPLEMENTED_FN();
	lua_pushstring(L, "test"); //questLogTitleText
	lua_pushinteger(L, 15); //level
	lua_pushstring(L, "test"); //questTag
	lua_pushinteger(L, 3); //suggestedGroup
	lua_pushboolean(L, false); //isHeader
	lua_pushboolean(L, false); //isCollapsed
	lua_pushboolean(L, false); //isComplete
	lua_pushboolean(L, true); //isDaily
	return 8;
}

LUA_FN(SelectQuestLogEntry)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: SelectQuestLogEntry(questID)");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(IsCurrentQuestFailed)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: IsCurrentQuestFailed()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, false);
	return 1;
}

LUA_FN(GetQuestResetTime)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetQuestResetTime()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushinteger(L, 10);
	return 1;
}

LUA_FN(GetQuestLogQuestText)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetQuestLogQuestText()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushstring(L, "description"); //description
	lua_pushstring(L, "les objectifs"); //objectives
	return 2;
}

LUA_FN(GetQuestLogTimeLeft)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetQuestLogTimeLeft()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushinteger(L, 5); //seconds, nil if no timer
	return 1;
}

LUA_FN(GetNumQuestLeaderBoards)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetNumQuestLeaderBoards()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushinteger(L, 2);
	return 1;
}

LUA_FN(GetQuestLogLeaderBoard)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: GetQuestLogLeaderBoard(index)");
	LUA_UNIMPLEMENTED_FN();
	lua_pushstring(L, "objective"); //text
	lua_pushstring(L, "killing npc"); //type
	lua_pushboolean(L, false); //finished
	return 3;
}

LUA_FN(GetQuestLogRequiredMoney)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetQuestLogRequiredMoney()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushinteger(L, 10);
	return 1;
}

LUA_FN(GetQuestLogGroupNum)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetQuestLogGroupNum()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushinteger(L, 1);
	return 1;
}

LUA_FN(GetNumQuestLogRewards)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetNumQuestLogRewards()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushinteger(L, 2);
	return 1;
}

LUA_FN(GetNumQuestLogChoices)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetNumQuestLogChoices()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushinteger(L, 1);
	return 1;
}

LUA_FN(GetQuestLogRewardMoney)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetQuestLogRewardMoney()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushinteger(L, 13371337);
	return 1;
}

LUA_FN(GetQuestLogRewardHonor)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetQuestLogRewardHonor()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushinteger(L, 347589);
	return 1;
}

LUA_FN(GetQuestLogRewardTitle)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetQuestLogRewardTitle()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushinteger(L, 12);
	return 1;
}

LUA_FN(GetQuestLogRewardSpell)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetQuestLogRewardSpell()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushstring(L, "Interface/Minimap/Tracking/Auctioneer.blp"); //texture
	lua_pushstring(L, "test"); //name
	lua_pushboolean(L, false); //isTradeskillSpell
	lua_pushboolean(L, false); //isSpellLearned
	return 4;
}

LUA_FN(GetQuestBackgroundMaterial)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetQuestBackgroundMaterial()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushstring(L, "Bronze"); //nil for default (parchment); Bronze, Marble, Silver, Stone, Valentine
	return 1;
}

LUA_FN(GetQuestLogChoiceInfo)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: GetQuestLogChoiceInfo(index)");
	LUA_UNIMPLEMENTED_FN();
	lua_pushstring(L, "test"); //name
	lua_pushstring(L, "Interface/Minimap/Tracking/Auctioneer.blp"); //texture
	lua_pushinteger(L, 2); //numItems
	lua_pushinteger(L, 4); //quality
	lua_pushboolean(L, false); //isUsable
	return 5;
}

LUA_FN(GetQuestLogRewardInfo)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: GetQuestLogRewardInfo(index)");
	LUA_UNIMPLEMENTED_FN();
	lua_pushstring(L, "test"); //name
	lua_pushstring(L, "Interface/Minimap/Tracking/Auctioneer.blp"); //texture
	lua_pushinteger(L, 2); //numItems
	lua_pushinteger(L, 4); //quality
	lua_pushboolean(L, false); //isUsable
	return 5;
}

LUA_FN(IsQuestWatched)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: IsQuestWatched(index)");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, true);
	return 1;
}

LUA_FN(GetQuestLogPushable)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetQuestLogPushable()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, false);
	return 1;
}

LUA_FN(ExpandQuestHeader)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: ExpandQuestHeader(questID)");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(CollapseQuestHeader)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: CollapseQuestHeader(questID)");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(GetQuestGreenRange)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetQuestGreenRange()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushnumber(L, 20);
	return 1;
}

LUA_FN(CloseQuest)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: CloseQuest()");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(GetQuestTimers)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetQuestTimers()");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

void register_quest_functions(lua_State *L)
{
	LUA_REGISTER_FN(GetQuestLogSelection);
	LUA_REGISTER_FN(GetNumQuestLogEntries);
	LUA_REGISTER_FN(GetDailyQuestsCompleted);
	LUA_REGISTER_FN(GetMaxDailyQuests);
	LUA_REGISTER_FN(GetQuestLogTitle);
	LUA_REGISTER_FN(SelectQuestLogEntry);
	LUA_REGISTER_FN(IsCurrentQuestFailed);
	LUA_REGISTER_FN(GetQuestResetTime);
	LUA_REGISTER_FN(GetQuestLogQuestText);
	LUA_REGISTER_FN(GetQuestLogTimeLeft);
	LUA_REGISTER_FN(GetNumQuestLeaderBoards);
	LUA_REGISTER_FN(GetQuestLogLeaderBoard);
	LUA_REGISTER_FN(GetQuestLogRequiredMoney);
	LUA_REGISTER_FN(GetQuestLogGroupNum);
	LUA_REGISTER_FN(GetNumQuestLogRewards);
	LUA_REGISTER_FN(GetNumQuestLogChoices);
	LUA_REGISTER_FN(GetQuestLogRewardMoney);
	LUA_REGISTER_FN(GetQuestLogRewardHonor);
	LUA_REGISTER_FN(GetQuestLogRewardTitle);
	LUA_REGISTER_FN(GetQuestLogRewardSpell);
	LUA_REGISTER_FN(GetQuestBackgroundMaterial);
	LUA_REGISTER_FN(GetQuestLogChoiceInfo);
	LUA_REGISTER_FN(GetQuestLogRewardInfo);
	LUA_REGISTER_FN(IsQuestWatched);
	LUA_REGISTER_FN(GetQuestLogPushable);
	LUA_REGISTER_FN(ExpandQuestHeader);
	LUA_REGISTER_FN(CollapseQuestHeader);
	LUA_REGISTER_FN(GetQuestGreenRange);
	LUA_REGISTER_FN(CloseQuest);
	LUA_REGISTER_FN(GetQuestTimers);
}
