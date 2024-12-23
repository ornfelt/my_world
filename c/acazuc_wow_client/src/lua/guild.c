#include "game/guild.h"

#include "lua/functions.h"

#include "wow_lua.h"
#include "wow.h"
#include "log.h"

LUA_FN(SetGuildRosterSelection)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: SetGuildRosterSelection(index)");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(IsInGuild)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: IsInGuild()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, true);
	return 1;
}

LUA_FN(GetGuildRosterMOTD)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetGuildRosterMOTD()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushstring(L, "Guild MOTD");
	return 1;
}

LUA_FN(GuildControlGetNumRanks)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GuildControlGetNumRanks()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushinteger(L, 3);
	return 1;
}

LUA_FN(GetGuildInfo)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: GetGuildInfo(\"player\")");
	LUA_UNIMPLEMENTED_FN();
	lua_pushstring(L, "My guild");
	lua_pushstring(L, "GuildRank");
	lua_pushinteger(L, 1); //Rank index
	return 3;
}

LUA_FN(GetNumGuildMembers)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc > 1)
		return luaL_error(L, "Usage: GetNumGuildMembers([showOffline])");
	bool show_offline;
	if (argc == 1)
		show_offline = lua_toboolean(L, 1);
	else
		show_offline = true;
	/* XXX show offline */
	(void)show_offline;
	lua_pushinteger(L, g_wow->guild->members.size);
	return 1;
}

LUA_FN(GetGuildRosterSelection)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetGuildRosterSelectection()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushinteger(L, 1); //Selected guild member
	return 1;
}

LUA_FN(GetGuildRosterInfo)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: GetGuildRosterInfo(index)");
	LUA_UNIMPLEMENTED_FN();
	lua_pushstring(L, "MemberName");
	lua_pushstring(L, "Rank");
	lua_pushinteger(L, 1); //RankIndex
	lua_pushinteger(L, 70); //Level
	lua_pushinteger(L, 1); //Class
	lua_pushinteger(L, 1); //Zone
	lua_pushstring(L, "note");
	lua_pushstring(L, "officerNote");
	lua_pushboolean(L, true); //Online
	return 9;
}

LUA_FN(CanEditPublicNote)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: CanEditPublicNote()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, true);
	return 1;
}

LUA_FN(CanViewOfficerNote)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: CanViewOfficerNote()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, true);
	return 1;
}

LUA_FN(CanEditOfficerNote)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: CanEditOfficerNote()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, true);
	return 1;
}

LUA_FN(CanGuildPromote)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: CanGuildPromote()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, true);
	return 1;
}

LUA_FN(CanGuildDemote)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: CanGuildDemote()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, true);
	return 1;
}

LUA_FN(CanGuildRemove)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: CanGuildRemove()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, true);
	return 1;
}

LUA_FN(CanEditMOTD)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: CanEditMOTD()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, true);
	return 1;
}

LUA_FN(IsGuildLeader)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: IsGuildLeader()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, true);
	return 1;
}

LUA_FN(CanGuildInvite)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: CanGuildInvite()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, true);
	return 1;
}

LUA_FN(GuildRoster)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GuildRoster()");
	LUA_UNIMPLEMENTED_FN();
	//TODO load roster from server
	return 0;
}

LUA_FN(GuildControlGetRankFlags)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GuildControlGetRankFlags()");
	LUA_UNIMPLEMENTED_FN();
	for (size_t i = 0; i < 13; ++i)
		lua_pushboolean(L, true);
	return 13;
}

LUA_FN(GuildControlGetRankName)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: GuildControlGetRankName(index)");
	LUA_UNIMPLEMENTED_FN();
	lua_pushstring(L, "MyRank");
	return 1;
}

LUA_FN(GetNumGuildBankTabs)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetNumGuildBankTabs()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushinteger(L, 3);
	return 1;
}

LUA_FN(GetGuildInfoText)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetGuildInfoText()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushstring(L, "info text");
	return 1;
}

LUA_FN(CanEditGuildInfo)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: CanEditGuildInfo()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, 1);
	return 1;
}

LUA_FN(SortGuildRoster)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: SortGuildRoster(\"sort\")");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(GuildInvite)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: GuildInvite(\"name\")");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(QueryGuildEventLog)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: QueryGuildEventLog()");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(SetGuildInfoText)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: SetGuildInfoText(\"text\"");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(SetGuildRosterShowOffline)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: SetGuildRosterShowOffline(offline)");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(GetGuildBankMoney)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetGuildBankMoney()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushinteger(L, 13371337);
	return 1;
}

LUA_FN(GetGuildBankWithdrawMoney)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetGuildBankWithdrawMoney()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushinteger(L, 1337);
	return 1;
}

LUA_FN(CanGuildBankRepair)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: CanGuildBankRepair()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, true);
	return 1;
}

LUA_FN(GetCurrentGuildBankTab)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetCurrentGuildBankTab()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushinteger(L, 1);
	return 1;
}

LUA_FN(GetGuildBankTabInfo)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: GetGuildBankTabInfo(tab)");
	LUA_UNIMPLEMENTED_FN();
	lua_pushstring(L, "name");
	lua_pushstring(L, "Interface\\Icons\\Spell_Shadow_SoulGem.blp");
	lua_pushboolean(L, true); //viewable
	lua_pushboolean(L, true); //deposit
	lua_pushnumber(L, 10); //num withdraw
	lua_pushnumber(L, 5); //remaining withdraw
	return 6;
}

LUA_FN(CanWithdrawGuildBankMoney)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: CanWithdrawGuildBankMoney()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, true);
	return 1;
}

LUA_FN(GetGuildTabardFileNames)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetGuildTabardFileNames()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushstring(L, ""); //bg upper
	lua_pushstring(L, ""); //bg lower
	lua_pushstring(L, ""); //emblem upper
	lua_pushstring(L, ""); //emblem lower
	lua_pushstring(L, ""); //border upper
	lua_pushstring(L, ""); //border lower
	return 6;
}

LUA_FN(GuildPromote)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: GuildPromote(\"player\")");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(GuildDemote)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: GuildDemote(\"player\")");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(GuildUninvite)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: GuildUninvite(\"player\")");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(GetGuildBankTabPermissions)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: GetGuildBankTabPermissions(tab)");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, true); //view tab
	lua_pushboolean(L, true); //can deposit
	lua_pushboolean(L, true); //can update text
	lua_pushnumber(L, 1337); //num withdrawals
	return 4;
}

LUA_FN(GetGuildBankWithdrawLimit)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetGuildBankWithdrawLimit()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushnumber(L, 13371337);
	return 1;
}

LUA_FN(GuildControlSetRankFlag)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 2)
		return luaL_error(L, "Usage: GuildControlSetRankFlag(rank, checked)");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(SetGuildBankWithdrawLimit)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: SetGuildBankWithdrawLimit(limit)");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(GetGuildRosterShowOffline)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetGuildRosterShowOffline()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, false);
	return 1;
}

LUA_FN(SetGuildRecruitmentMode)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: SetGuildRecruitmentMode(mode)");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(SetGuildBankText)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 2)
		return luaL_error(L, "Usage: SetGuildBankText(tab, \"text\")");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

void register_guild_functions(lua_State *L)
{
	LUA_REGISTER_FN(SetGuildRosterSelection);
	LUA_REGISTER_FN(IsInGuild);
	LUA_REGISTER_FN(GetGuildRosterMOTD);
	LUA_REGISTER_FN(GuildControlGetNumRanks);
	LUA_REGISTER_FN(GetGuildInfo);
	LUA_REGISTER_FN(GetNumGuildMembers);
	LUA_REGISTER_FN(GetGuildRosterSelection);
	LUA_REGISTER_FN(GetGuildRosterInfo);
	LUA_REGISTER_FN(CanEditPublicNote);
	LUA_REGISTER_FN(CanViewOfficerNote);
	LUA_REGISTER_FN(CanEditOfficerNote);
	LUA_REGISTER_FN(CanGuildPromote);
	LUA_REGISTER_FN(CanGuildDemote);
	LUA_REGISTER_FN(CanGuildRemove);
	LUA_REGISTER_FN(CanEditMOTD);
	LUA_REGISTER_FN(IsGuildLeader);
	LUA_REGISTER_FN(CanGuildInvite);
	LUA_REGISTER_FN(GuildRoster);
	LUA_REGISTER_FN(GuildControlGetRankFlags);
	LUA_REGISTER_FN(GuildControlGetRankName);
	LUA_REGISTER_FN(GetNumGuildBankTabs);
	LUA_REGISTER_FN(GetGuildInfoText);
	LUA_REGISTER_FN(CanEditGuildInfo);
	LUA_REGISTER_FN(SortGuildRoster);
	LUA_REGISTER_FN(GuildInvite);
	LUA_REGISTER_FN(QueryGuildEventLog);
	LUA_REGISTER_FN(SetGuildInfoText);
	LUA_REGISTER_FN(SetGuildRosterShowOffline);
	LUA_REGISTER_FN(GetGuildBankMoney);
	LUA_REGISTER_FN(GetGuildBankWithdrawMoney);
	LUA_REGISTER_FN(CanGuildBankRepair);
	LUA_REGISTER_FN(GetCurrentGuildBankTab);
	LUA_REGISTER_FN(GetGuildBankTabInfo);
	LUA_REGISTER_FN(CanWithdrawGuildBankMoney);
	LUA_REGISTER_FN(GetGuildTabardFileNames);
	LUA_REGISTER_FN(GuildPromote);
	LUA_REGISTER_FN(GuildDemote);
	LUA_REGISTER_FN(GuildUninvite);
	LUA_REGISTER_FN(GetGuildBankTabPermissions);
	LUA_REGISTER_FN(GetGuildBankWithdrawLimit);
	LUA_REGISTER_FN(GuildControlSetRankFlag);
	LUA_REGISTER_FN(SetGuildBankWithdrawLimit);
	LUA_REGISTER_FN(GetGuildRosterShowOffline);
	LUA_REGISTER_FN(SetGuildRecruitmentMode);
	LUA_REGISTER_FN(SetGuildBankText);
}
