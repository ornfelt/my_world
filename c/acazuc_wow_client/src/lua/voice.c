#include "functions.h"

#include "wow_lua.h"
#include "log.h"

LUA_FN(IsVoiceChatEnabled)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: IsVoiceChatEnabled()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, false);
	return 1;
}

LUA_FN(IsVoiceChatAllowedByServer)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: IsVoiceChatAllowedByServer()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, false);
	return 1;
}

LUA_FN(VoiceEnumerateOutputDevices)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: VoiceEnumerateOutputDevices(index)");
	LUA_UNIMPLEMENTED_FN();
	int device = lua_tointeger(L, 1);
	if (device > 0)
		lua_pushnil(L);
	else
		lua_pushstring(L, "Default Output");
	return 1;
}

LUA_FN(VoiceEnumerateCaptureDevices)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: VoiceEnumerateCaptureDevices(index");
	LUA_UNIMPLEMENTED_FN();
	int device = lua_tointeger(L, 1);
	if (device > 0)
		lua_pushnil(L);
	else
		lua_pushstring(L, "Default Capture");
	return 1;
}

LUA_FN(GetNumVoiceSessions)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetNumVoiceSessions()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushinteger(L, 0);
	return 1;
}

LUA_FN(GetVoiceCurrentSessionID)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetVoiceCurrentSessionID()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushinteger(L, 1);
	return 1;
}

LUA_FN(GetVoiceStatus)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1 && argc != 2)
		return luaL_error(L, "Usage: GetVoiceStatus(\"unit\", mode)");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, false);
	return 1;
}

LUA_FN(GetMuteStatus)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1 && argc != 2)
		return luaL_error(L, "Usage: GetMuteStatus(\"unit\", mode)");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, false);
	return 1;
}

LUA_FN(VoiceChat_StopPlayingLoopbackSound)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: VoiceChat_StopPlayingLoopbackSound()");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(VoiceGetCurrentOutputDevice)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: VoiceGetCurrentOutputDevice()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushinteger(L, 1);
	return 1;
}

LUA_FN(VoiceGetCurrentCaptureDevice)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: VoiceGetCurrentCaptureDevice()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushinteger(L, 1);
	return 1;
}

LUA_FN(GetVoiceSessionInfo)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: GetVoiceSessionInfo(index)");
	LUA_UNIMPLEMENTED_FN();
	lua_pushstring(L, "test"); //name
	lua_pushboolean(L, false); //active
	return 2;
}

LUA_FN(GetNumVoiceSessionMembersBySessionID)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: GetNumVoiceSessionMembersBySessionID(\"session\")");
	lua_pushinteger(L, 0);
	return 1;
}

LUA_FN(VoiceSelectCaptureDevice)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: VoiceSelectCaptureDevice(\"device\")");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(VoiceSelectOutputDevice)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: VoiceSelectOutputDevice(\"device\")");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

void register_voice_functions(lua_State *L)
{
	LUA_REGISTER_FN(IsVoiceChatEnabled);
	LUA_REGISTER_FN(IsVoiceChatAllowedByServer);
	LUA_REGISTER_FN(VoiceEnumerateOutputDevices);
	LUA_REGISTER_FN(VoiceEnumerateCaptureDevices);
	LUA_REGISTER_FN(GetNumVoiceSessions);
	LUA_REGISTER_FN(GetVoiceCurrentSessionID);
	LUA_REGISTER_FN(GetVoiceStatus);
	LUA_REGISTER_FN(GetMuteStatus);
	LUA_REGISTER_FN(VoiceChat_StopPlayingLoopbackSound);
	LUA_REGISTER_FN(VoiceGetCurrentOutputDevice);
	LUA_REGISTER_FN(VoiceGetCurrentCaptureDevice);
	LUA_REGISTER_FN(GetVoiceSessionInfo);
	LUA_REGISTER_FN(GetNumVoiceSessionMembersBySessionID);
	LUA_REGISTER_FN(VoiceSelectCaptureDevice);
	LUA_REGISTER_FN(VoiceSelectOutputDevice);
}
