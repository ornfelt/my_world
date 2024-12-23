#include "functions.h"

#include "wow_lua.h"
#include "log.h"

LUA_FN(GetNumSpellTabs)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetNumSpellTabs()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushinteger(L, 3);
	return 1;
}

LUA_FN(GetSpellTabInfo)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: GetSpellTabInfo(tab)");
	LUA_UNIMPLEMENTED_FN();
	lua_pushstring(L, "test"); //Name
	lua_pushstring(L, "Interface\\Icons\\Spell_Shadow_SoulGem.blp"); //Texture
	lua_pushinteger(L, 0); //Offset
	lua_pushinteger(L, 5); //Number
	return 4;
}

LUA_FN(SpellIsTargeting)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: SpellIsTargeting()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, false);
	return 1;
}

LUA_FN(GetSpellBonusDamage)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: GetSpellBonusDamage(school)");
	LUA_UNIMPLEMENTED_FN();
	lua_pushinteger(L, 10);
	return 1;
}

LUA_FN(GetSpellTexture)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 2)
		return luaL_error(L, "Usage: GetSpellTexture(id, book)");
	LUA_UNIMPLEMENTED_FN();
	lua_pushstring(L, "Interface\\Icons\\Spell_Shadow_SoulGem.blp");
	return 1;
}

LUA_FN(GetSpellCooldown)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 2)
		return luaL_error(L, "Usage: GetSpellCooldown(id, book)");
	LUA_UNIMPLEMENTED_FN();
	lua_pushnumber(L, 13); //start
	lua_pushnumber(L, 16); //duration
	lua_pushnumber(L, rand() & 1); //enabled
	return 3;
}

LUA_FN(GetSpellAutocast)
{
	int argc = lua_gettop(L);
	if (argc != 2)
		return luaL_error(L, "Usage: GetSpellAutocast(id, book)");
	LUA_UNIMPLEMENTED_FN();
	lua_pushnumber(L, rand() & 1); //allowed
	lua_pushnumber(L, rand() & 1); //enabled
	return 2;
}

LUA_FN(GetSpellName)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 2)
		return luaL_error(L, "Usage: GetSpellName(id, book)");
	LUA_UNIMPLEMENTED_FN();
	lua_pushstring(L, "Spell name"); //name
	lua_pushstring(L, "Spell sub name"); //sub name
	return 2;
}

LUA_FN(IsPassiveSpell)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 2)
		return luaL_error(L, "Usage: IsPassiveSpell(id, book)");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, rand() & 1);
	return 1;
}

LUA_FN(IsSelectedSpell)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 2)
		return luaL_error(L, "Usage: IsSelectedSpell(id, book)");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, rand() & 1);
	return 1;
}

LUA_FN(CastSpell)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 2)
		return luaL_error(L, "Usage: CastSpell(id, book)");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(UpdateSpells)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: UpdateSpells()");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(SpellCanTargetItem)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: SpellCanTargetItem()");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

LUA_FN(GetSpellInfo)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: GetSpellInfo(spellId)");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

void register_spell_functions(lua_State *L)
{
	LUA_REGISTER_FN(GetNumSpellTabs);
	LUA_REGISTER_FN(GetSpellTabInfo);
	LUA_REGISTER_FN(SpellIsTargeting);
	LUA_REGISTER_FN(GetSpellBonusDamage);
	LUA_REGISTER_FN(GetSpellTexture);
	LUA_REGISTER_FN(GetSpellCooldown);
	LUA_REGISTER_FN(GetSpellAutocast);
	LUA_REGISTER_FN(GetSpellName);
	LUA_REGISTER_FN(IsPassiveSpell);
	LUA_REGISTER_FN(IsSelectedSpell);
	LUA_REGISTER_FN(CastSpell);
	LUA_REGISTER_FN(UpdateSpells);
	LUA_REGISTER_FN(SpellCanTargetItem);
	LUA_REGISTER_FN(GetSpellInfo);
}
