#include "functions.h"

#include "net/packets.h"
#include "net/network.h"
#include "net/packet.h"

#include "wow_lua.h"
#include "log.h"
#include "wow.h"

LUA_FN(GetRealNumPartyMembers)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetRealNumPartyMembers()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushinteger(L, 3);
	return 1;
}

LUA_FN(GetNumPartyMembers)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetNumPartyMembers()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushinteger(L, 4);
	return 1;
}

LUA_FN(GetNumRaidMembers)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetNumRaidMembers()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushinteger(L, 20);
	return 1;
}

LUA_FN(IsPartyLeader)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: IsPartyLeader()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, true);
	return 1;
}

LUA_FN(GetPartyMember)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: GetPartyMember(index)");
	LUA_UNIMPLEMENTED_FN();
	if (!lua_isinteger(L, 1)) //Member index (1-4)
		return luaL_argerror(L, 1, "integer expected");
	lua_pushboolean(L, true);
	return 1;
}

LUA_FN(IsRaidOfficer)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: IsRaidOfficer()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, true);
	return 1;
}

LUA_FN(InviteUnit)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: InviteUnit(\"unit\")");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(RequestRaidInfo)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: RequestRaidInfo()");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(GetPartyLeaderIndex)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetPartyLeaderIndex()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushinteger(L, 1);
	return 1;
}

LUA_FN(GetMasterLootCandidate)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: GetMasterLootCandidate(index)");
	LUA_UNIMPLEMENTED_FN();
	lua_pushstring(L, "myself"); //name
	return 1;
}

LUA_FN(IsRealPartyLeader)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: IsRealPartyLeader()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, true);
	return 1;
}

LUA_FN(GetRealNumRaidMembers)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetRealNumRaidMembers()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushnumber(L, 3);
	return 1;
}

LUA_FN(ConvertToRaid)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: ConvertToRaid()");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(IsRealRaidLeader)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: IsRealRaidLeader()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, true);
	return 1;
}

LUA_FN(IsRaidLeader)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: IsRaidLeader()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, true);
	return 1;
}

LUA_FN(IsRaidOfficier)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: IsRaidOfficer()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, true);
	return 1;
}

LUA_FN(GetRaidRosterInfo)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: GetRaidRosterInfo(index)");
	LUA_UNIMPLEMENTED_FN();
	lua_pushstring(L, "bonsoir"); //namne
	lua_pushnumber(L, rand() % 3); //rank
	lua_pushnumber(L, 3); //subgroup
	lua_pushnumber(L, 70); //level
	lua_pushstring(L, "Priest"); //class
	lua_pushstring(L, "PRIEST"); //fileName
	lua_pushstring(L, "Azshara"); //zone
	lua_pushboolean(L, rand() & 1); //online
	lua_pushboolean(L, rand() & 1); //dead
	lua_pushstring(L, (rand() & 1) ? "maintank" : ((rand() & 1) ? "mainassist" : "")); //role
	return 10;
}

LUA_FN(DoReadyCheck)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: DoReadyCheck()");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(GetRaidTargetIndex)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: GetRaidTargetIndex(unit)");
	LUA_UNIMPLEMENTED_FN();
	lua_pushinteger(L, 1);
	return 1;
}

LUA_FN(AcceptGroup)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: AcceptGroup()");
	struct net_packet_writer packet;
	if (!net_cmsg_group_accept(&packet)
	 || !net_send_packet(g_wow->network, &packet))
		LOG_WARN("error while sending CMSG_GROUP_ACCEPT packet");
	net_packet_writer_destroy(&packet);
	return 0;
}

LUA_FN(DeclineGroup)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: DeclineGroup()");
	struct net_packet_writer packet;
	if (!net_cmsg_group_decline(&packet)
	 || !net_send_packet(g_wow->network, &packet))
		LOG_WARN("error while sending CMSG_GROUP_DECLINE packet");
	return 0;
}

void register_party_functions(lua_State *L)
{
	LUA_REGISTER_FN(GetRealNumPartyMembers);
	LUA_REGISTER_FN(GetNumPartyMembers);
	LUA_REGISTER_FN(GetNumRaidMembers);
	LUA_REGISTER_FN(IsPartyLeader);
	LUA_REGISTER_FN(GetPartyMember);
	LUA_REGISTER_FN(IsRaidOfficer);
	LUA_REGISTER_FN(InviteUnit);
	LUA_REGISTER_FN(RequestRaidInfo);
	LUA_REGISTER_FN(GetPartyLeaderIndex);
	LUA_REGISTER_FN(GetMasterLootCandidate);
	LUA_REGISTER_FN(IsRealPartyLeader);
	LUA_REGISTER_FN(GetRealNumRaidMembers);
	LUA_REGISTER_FN(ConvertToRaid);
	LUA_REGISTER_FN(IsRealRaidLeader);
	LUA_REGISTER_FN(IsRaidLeader);
	LUA_REGISTER_FN(IsRaidOfficier);
	LUA_REGISTER_FN(GetRaidRosterInfo);
	LUA_REGISTER_FN(DoReadyCheck);
	LUA_REGISTER_FN(GetRaidTargetIndex);
	LUA_REGISTER_FN(AcceptGroup);
	LUA_REGISTER_FN(DeclineGroup);
}
