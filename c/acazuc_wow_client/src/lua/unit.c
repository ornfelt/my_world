#include "lua/functions.h"

#include "obj/update_fields.h"
#include "obj/player.h"
#include "obj/unit.h"

#include "wow_lua.h"
#include "log.h"
#include "dbc.h"
#include "wow.h"
#include "wdb.h"

#include <wow/wdb.h>

#include <inttypes.h>
#include <string.h>

struct unit *lua_get_unit(lua_State *L, int arg)
{
	const char *name = lua_tostring(L, arg);
	if (!name)
		return NULL;
	if (!strcasecmp(name, "player"))
		return (struct unit*)g_wow->player;
	if (!strcasecmp(name, "party1"))
		return NULL;
	if (!strcasecmp(name, "party2"))
		return NULL;
	if (!strcasecmp(name, "party3"))
		return NULL;
	if (!strcasecmp(name, "party4"))
		return NULL;
	if (!strcasecmp(name, "party5"))
		return NULL;
	if (!strcasecmp(name, "target"))
		return NULL;
	if (!strcasecmp(name, "targettarget"))
		return NULL;
	if (!strcasecmp(name, "pet"))
		return NULL;
	if (!strcasecmp(name, "raid1"))
		return NULL;
	return NULL;
}

static inline struct player *get_player(lua_State *L, int arg)
{
	struct unit *unit = lua_get_unit(L, arg);
	if (!unit)
		return NULL;
	return object_as_player((struct object*)unit);
}

LUA_FN(UnitName)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: UnitName(\"unit\")");
	struct unit *unit = lua_get_unit(L, 1);
	if (!unit)
		return 0;
	const struct wow_wdb_name *name = wdb_get_name(g_wow->wdb, object_guid((struct object*)unit));
	if (name)
		lua_pushstring(L, name->name);
	else
		lua_getglobal(L, "UNKNOWNOBJECT");
	return 1;
}

static struct
{
	int id;
	const char *name;
	bool available;
} const races[] =
{
	{1 , "HUMAN"   , true},
	{3 , "DWARF"   , true},
	{4 , "NIGHTELF", true},
	{7 , "GNOME"   , true},
	{11, "DRAENEI" , true},
	{2 , "ORC"     , true},
	{5 , "SCOURGE" , true},
	{6 , "TAUREN"  , true},
	{8 , "TROLL"   , true},
	{10, "BLOODELF", true},
};

LUA_FN(UnitRace)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: UnitRace(\"unit\")");
	struct unit *unit = lua_get_unit(L, 1);
	if (!unit)
		return 0;
	uint8_t race = unit_get_race(unit);
	uint8_t gender = unit_get_gender(unit);
	if (race >= sizeof(races) / sizeof(*races))
	{
		LOG_ERROR("invalid race: %d", (int)race);
		return 0;
	}
	struct wow_dbc_row row;
	if (!dbc_get_row_indexed(g_wow->dbc.chr_races, &row, races[race].id))
	{
		LOG_ERROR("unknown race: %d", races[race].id);
		return 0;
	}
	lua_pushstring(L, wow_dbc_get_str(&row, gender == 2 ? 132 : 200)); //RaceName
	lua_pushstring(L, races[race].name); //RaceFileName: BloodElf, Draenei, Gnome, Human, NightElf, Orc, Troll, Scourge, Tauren
	lua_pushinteger(L, races[race].id); //RaceID
	return 3;
}

LUA_FN(UnitXP)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: UnitXP(\"unit\")");
	struct player *player = get_player(L, 1);
	if (player)
		lua_pushinteger(L, object_fields_get_u32(&((struct object*)player)->fields, PLAYER_FIELD_XP));
	else
		lua_pushinteger(L, 0);
	return 1;
}

LUA_FN(UnitXPMax)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: UnitXPMax(\"unit\")");
	struct player *player = get_player(L, 1);
	if (player)
		lua_pushinteger(L, object_fields_get_u32(&((struct object*)player)->fields, PLAYER_FIELD_NEXT_LEVEL_XP));
	else
		lua_pushinteger(L, 0);
	return 1;
}

LUA_FN(UnitLevel)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: UnitLevel(\"unit\")");
	struct unit *unit = lua_get_unit(L, 1);
	if (unit)
		lua_pushinteger(L, object_fields_get_u32(&((struct object*)unit)->fields, UNIT_FIELD_LEVEL));
	else
		lua_pushinteger(L, 0);
	return 1;
}

LUA_FN(UnitPVPName)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: UnitPVPName(\"unit\")");
	LUA_UNIMPLEMENTED_FN();
	struct unit *unit = lua_get_unit(L, 1);
	if (!unit)
		return 0;
	//XXX title
	const struct wow_wdb_name *name = wdb_get_name(g_wow->wdb, object_guid((struct object*)unit));
	if (name)
		lua_pushstring(L, name->name);
	else
		lua_getglobal(L, "UNKNOWNOBJECT");
	return 1;
}

LUA_FN(UnitIsDeadOrGhost)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: UnitIsDeadOrGhost(\"unit\")");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, false);
	return 1;
}

static const char *class_names[] =
{
	"WARRIOR",
	"PALADIN",
	"HUNTER",
	"ROGUE",
	"PRIEST",
	"SHAMAN",
	"MAGE",
	"WARLOCK",
	"DRUID",
};

LUA_FN(UnitClass)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: UnitClass(\"unit\")");
	LUA_UNIMPLEMENTED_FN();
	struct unit *unit = lua_get_unit(L, 1);
	if (!unit)
	{
		LOG_ERROR("can't find unit");
		return 0;
	}
	uint8_t class = unit_get_class(unit);
	uint8_t gender = unit_get_gender(unit);
	struct wow_dbc_row class_row;
	if (!dbc_get_row_indexed(g_wow->dbc.chr_classes, &class_row, class))
	{
		LOG_ERROR("unknown class: %d", class);
		return 0;
	}
	lua_pushstring(L, wow_dbc_get_str(&class_row, gender ? 160 : 92)); //ClassName
	lua_pushstring(L, class_names[class - 1]); //ClassFileName
	lua_pushinteger(L, class); //ClassID
	return 3;
}

LUA_FN(UnitFactionGroup)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: UnitFactionGroup(\"unit\")");
	LUA_UNIMPLEMENTED_FN();
	lua_pushstring(L, "Alliance");
	lua_pushstring(L, "Alliance"); //Localized
	return 2;
}

LUA_FN(UnitIsPossessed)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: UnitIsPossessed(\"unit\")");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, false);
	return 1;
}

LUA_FN(UnitExists)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: UnitExists(\"unit\")");
	struct unit *unit = lua_get_unit(L, 1);
	lua_pushboolean(L, unit != NULL);
	return 1;
}

LUA_FN(UnitIsUnit)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 2)
		return luaL_error(L, "Usage: UnitIsUnit(\"unit\", \"target\")");
	struct unit *unit1 = lua_get_unit(L, 1);
	struct unit *unit2 = lua_get_unit(L, 2);
	lua_pushboolean(L, unit1 && unit1 == unit2);
	return 1;
}

LUA_FN(UnitIsPlayer)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: UnitIsPlayer(\"unit\")");
	struct player *player = get_player(L, 1);
	lua_pushboolean(L, player != NULL);
	return 1;
}

LUA_FN(UnitHealth)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: UnitHealth(\"unit\")");
	struct unit *unit = lua_get_unit(L, 1);
	if (unit)
		lua_pushnumber(L, object_fields_get_u32(&((struct object*)unit)->fields, UNIT_FIELD_HEALTH));
	else
		lua_pushnumber(L, 0);
	return 1;
}

LUA_FN(UnitHealthMax)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: UnitHealthMax(\"unit\")");
	struct unit *unit = lua_get_unit(L, 1);
	if (unit)
		lua_pushnumber(L, object_fields_get_u32(&((struct object*)unit)->fields, UNIT_FIELD_MAXHEALTH));
	else
		lua_pushnumber(L, 0);
	return 1;
}

LUA_FN(UnitIsConnected)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: UnitIsConnected(\"unit\")");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, true);
	return 1;
}

LUA_FN(UnitInBattleground)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: UnitIsInBattleground(\"unit\")");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, false);
	return 1;
}

LUA_FN(UnitMana)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: UnitMana(\"unit\")");
	struct unit *unit = lua_get_unit(L, 1);
	if (unit)
		lua_pushnumber(L, object_fields_get_u32(&((struct object*)unit)->fields, UNIT_FIELD_POWER1));
	else
		lua_pushnumber(L, 0);
	return 1;
}

LUA_FN(UnitManaMax)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: UnitManaMax(\"unit\")");
	struct unit *unit = lua_get_unit(L, 1);
	if (unit)
		lua_pushnumber(L, object_fields_get_u32(&((struct object*)unit)->fields, UNIT_FIELD_MAXPOWER1));
	else
		lua_pushnumber(L, 0);
	return 1;
}

LUA_FN(UnitIsVisible)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: UnitIsVisible(\"unit\")");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, true);
	return 1;
}

LUA_FN(UnitCanCooperate)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 2)
		return luaL_error(L, "Usage: UnitCanCooperate(\"unit\", \"target\")");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, true);
	return 1;
}

LUA_FN(UnitCanAttack)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 2)
		return luaL_error(L, "Usage: UnitCanAttack(\"unit\", \"target\")");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, false);
	return 1;
}

LUA_FN(UnitIsDead)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: UnitIsDead(\"unit\")");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, false);
	return 1;
}

LUA_FN(UnitPlayerControlled)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: UnitPlayerControlled(\"unit\")");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, false);
	return 1;
}

LUA_FN(UnitReaction)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 2)
		return luaL_error(L, "Usage: UnitReaction(\"unit\", \"target\")");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, false);
	return 1;
}

LUA_FN(UnitPowerType)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: UnitPowerType(\"unit\")");
	LUA_UNIMPLEMENTED_FN();
	lua_pushinteger(L, 1);
	return 1;
}

LUA_FN(UnitBuff)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc < 2 || argc > 3)
		return luaL_error(L, "Usage: UnitBuff(\"unit\", index, flag)");
	LUA_UNIMPLEMENTED_FN();
	lua_pushstring(L, "test"); //name
	lua_pushinteger(L, 1); //rank
	lua_pushstring(L, "Interface\\Icons\\Spell_Shadow_SoulGem.blp"); //icon
	lua_pushinteger(L, 1); //count
	lua_pushinteger(L, 1); //duration
	lua_pushinteger(L, 1); //timeleft
	return 6;
}

LUA_FN(UnitDebuff)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc < 2 || argc > 3)
		return luaL_error(L, "Usage: UnitDebuff(\"unit\", index, flag)");
	LUA_UNIMPLEMENTED_FN();
	lua_pushstring(L, "test"); //name
	lua_pushinteger(L, 1); //rank
	lua_pushstring(L, "Interface\\Icons\\Spell_Shadow_SoulGem.blp"); //icon
	lua_pushinteger(L, 1); //count
	lua_pushstring(L, "none"); //debufftype: none, Magic, Curse, Disease, Poison
	lua_pushinteger(L, 1); //duration
	lua_pushinteger(L, 1); //timeleft
	return 7;
}

LUA_FN(UnitCharacterPoints)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: UnitCharacterPoints(\"unit\")");
	LUA_UNIMPLEMENTED_FN();
	lua_pushinteger(L, 2); //talentPoints
	lua_pushinteger(L, 5); //skillPoints
	return 2;
}

LUA_FN(UnitSex)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: UnitSex(\"unit\")");
	LUA_UNIMPLEMENTED_FN();
	lua_pushinteger(L, 2); //1 = unknown, 2 = male, 3 = female
	return 1;
}

LUA_FN(UnitIsGhost)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: UnitIsGhost(\"unit\")");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, false);
	return 1;
}

LUA_FN(UnitIsPVPFreeForAll)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: UnitIsPVPFreeForAll(\"unit\")");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, false);
	return 1;
}

LUA_FN(UnitIsPVP)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: UnitIsPVP(\"unit\")");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, false);
	return 1;
}

LUA_FN(UnitIsTalking)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: UnitIsTalking(\"unit\")");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, false);
	return 1;
}

LUA_FN(GetReadyCheckStatus)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: GetReadyCheckStatus(\"unit\")");
	LUA_UNIMPLEMENTED_FN();
	lua_pushstring(L, "ready"); //nil, ready, notready, waiting
	return 1;
}

LUA_FN(UnitResistance)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 2)
		return luaL_error(L, "Usage: UnitResistance(\"unit\", idx)");
	int index = lua_tointeger(L, 2);
	if (index < 0 || index > 6)
		return luaL_argerror(L, 2, "invalid resistance id");
	struct unit *unit = lua_get_unit(L, 1);
	if (!unit)
	{
		lua_pushnumber(L, 0);
		lua_pushnumber(L, 0);
		lua_pushnumber(L, 0);
		lua_pushnumber(L, 0);
		return 4;
	}
	lua_pushnumber(L, object_fields_get_u32(&((struct object*)unit)->fields, UNIT_FIELD_RESISTANCES + index));
	lua_pushnumber(L, object_fields_get_u32(&((struct object*)unit)->fields, UNIT_FIELD_RESISTANCES + index));
	lua_pushnumber(L, object_fields_get_u32(&((struct object*)unit)->fields, UNIT_FIELD_RESISTANCEBUFFMODSPOSITIVE + index));
	lua_pushnumber(L, object_fields_get_u32(&((struct object*)unit)->fields, UNIT_FIELD_RESISTANCEBUFFMODSNEGATIVE + index));
	return 4;
}

LUA_FN(UnitHasRelicSlot)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: UnitHasRelicSlot(\"unit\")");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, true);
	return 1;
}

LUA_FN(UnitStat)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 2)
		return luaL_error(L, "Usage: UnitStat(\"unit\", stat)");
	LUA_UNIMPLEMENTED_FN();
	lua_pushinteger(L, 1337); //stat
	lua_pushinteger(L, 512); //effectiveStat
	lua_pushinteger(L, 12); //positiveBuff
	lua_pushinteger(L, 600); //negativeBuff
	return 4;
}

LUA_FN(UnitAttackSpeed)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: UnitAttackSpeed(\"unit\")");
	LUA_UNIMPLEMENTED_FN();
	lua_pushnumber(L, 15.5); //speed
	lua_pushnumber(L, 1.12); //offHandSpeed
	return 1;
}

LUA_FN(UnitArmor)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: UnitArmor(\"unit\")");
	LUA_UNIMPLEMENTED_FN();
	lua_pushinteger(L, 1337); //base
	lua_pushinteger(L, 512); //effectiveArmor
	lua_pushinteger(L, 192); //armor
	lua_pushinteger(L, 12); //positiveBuff
	lua_pushinteger(L, 600); //negativeBuff
	return 5;
}

LUA_FN(UnitDamage)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: UnitDamage(\"unit\")");
	LUA_UNIMPLEMENTED_FN();
	lua_pushinteger(L, 14); //minDamage
	lua_pushinteger(L, 56); //maxDamage
	lua_pushinteger(L, 1); //minOffHandDamage
	lua_pushinteger(L, 16); //maxOffHandDamage
	lua_pushinteger(L, 14); //physicalBonusPositive
	lua_pushinteger(L, 13); //physicalBonusNegative
	lua_pushnumber(L, 114);
	return 7;
}

LUA_FN(UnitDefense)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: UnitDefense(\"unit\")");
	LUA_UNIMPLEMENTED_FN();
	lua_pushinteger(L, 1634); //base
	lua_pushinteger(L, -67); //modifier
	return 2;
}

LUA_FN(GetPlayerBuff)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 2)
		return luaL_error(L, "Usage: GetPlayerBuff(index, filter)");
	LUA_UNIMPLEMENTED_FN();
	lua_pushinteger(L, 0); //buff index
	lua_pushinteger(L, 0); //until cancelled
	return 2;
}

LUA_FN(UnitCreatureFamily)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: UnitCreatureFamily(creature)");
	LUA_UNIMPLEMENTED_FN();
	lua_pushstring(L, "pet"); //family
	return 1;
}

LUA_FN(GetCritChanceFromAgility)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: GetCritChanceFromAgility(\"unit\")");
	LUA_UNIMPLEMENTED_FN();
	lua_pushnumber(L, 50);
	return 1;
}

LUA_FN(GetUnitMaxHealthModifier)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: GetUnitMaxHealthModifier(\"unit\")");
	LUA_UNIMPLEMENTED_FN();
	lua_pushnumber(L, 50);
	return 1;
}

LUA_FN(GetUnitHealthRegenRateFromSpirit)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: GetUnitHealthRegenRateFromSpirit(\"unit\")");
	LUA_UNIMPLEMENTED_FN();
	lua_pushnumber(L, 50);
	return 1;
}

LUA_FN(UnitAttackPower)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: UnitAttackPower(\"unit\")");
	LUA_UNIMPLEMENTED_FN();
	lua_pushnumber(L, 50);
	lua_pushnumber(L, 20);
	lua_pushnumber(L, 10);
	return 3;
}

LUA_FN(GetArmorPenetration)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetArmorPenetration()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushnumber(L, 50);
	return 1;
}

LUA_FN(UnitRangedAttackPower)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: UnitRangedAttackPower(\"unit\")");
	LUA_UNIMPLEMENTED_FN();
	lua_pushnumber(L, 50);
	lua_pushnumber(L, 20);
	lua_pushnumber(L, 10);
	return 3;
}

LUA_FN(GetCritChance)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetCritChance()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushnumber(L, 50);
	return 1;
}

LUA_FN(GetRangedCritChance)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetRangedCritChance()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushnumber(L, 50);
	return 1;
}

LUA_FN(GetExpertise)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetExpertise()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushnumber(L, 50);
	lua_pushnumber(L, 50); //offhand
	return 2;
}

LUA_FN(GetExpertisePercent)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetExpertisePercent()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushnumber(L, 50);
	lua_pushnumber(L, 50); //offhand
	return 2;
}

LUA_FN(GetSpellBonusHealing)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetSpellBonusHealing()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushnumber(L, 1);
	return 1;
}

LUA_FN(GetSpellPenetration)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetSpellPenetration()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushnumber(L, 1);
	return 1;
}

LUA_FN(GetSpellCritChance)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: GetSpellCritChance(category)");
	LUA_UNIMPLEMENTED_FN();
	lua_pushnumber(L, 1);
	return 1;
}

LUA_FN(GetDodgeChance)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetDodgeChance()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushnumber(L, 0.5);
	return 1;
}

LUA_FN(GetParryChance)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetParryChance()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushnumber(L, 0.5);
	return 1;
}

LUA_FN(GetBlockChance)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetBlockChance()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushnumber(L, 0.5);
	return 1;
}

LUA_FN(GetShieldBlock)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 0)
		return luaL_error(L, "Usage: GetShieldBlock()");
	LUA_UNIMPLEMENTED_FN();
	lua_pushnumber(L, 0.5);
	return 1;
}

LUA_FN(UnitCanAssist)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 2)
		return luaL_error(L, "Usage: UnitCanAssist(\"unit\", \"target\")");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, false);
	return 1;
}

LUA_FN(GetUnitHealthModifier)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: GetUnitHealthModifier(\"unit\")");
	LUA_UNIMPLEMENTED_FN();
	lua_pushnumber(L, 10);
	return 1;
}

LUA_FN(GetSpellCritChanceFromIntellect)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: GetSpellCritChanceFromIntellect(\"unit\")");
	LUA_UNIMPLEMENTED_FN();
	lua_pushnumber(L, 10);
	return 1;
}

LUA_FN(UnitIsFriend)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 2)
		return luaL_error(L, "Usage: UnitIsFriend(\"unit\", \"target\")");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, true);
	return 1;
}

LUA_FN(UnitClassification)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: UnitClassification(\"unit\")");
	LUA_UNIMPLEMENTED_FN();
	lua_pushstring(L, "rareelite");
	return 1;
}

LUA_FN(UnitClassBase)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: UnitClassBase(\"unit\")");
	LUA_UNIMPLEMENTED_FN();
	lua_pushstring(L, "Priest"); //class
	lua_pushstring(L, "PRIEST"); //filename
	return 2;
}

LUA_FN(GetInventoryItemTexture)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 2)
		return luaL_error(L, "Usage GetInventoryItemTexture(\"unit\", slot)");
	if (!lua_isinteger(L, 2))
		return luaL_argerror(L, 2, "integer expected");
	int slot = lua_tointeger(L, 2);
	if (slot < 1 || slot > 23)
		return luaL_argerror(L, 2, "invalid slot: < 1 || > 23");
	struct player *player = get_player(L, 1);
	if (!player)
		return 0;
	uint32_t item;
	if (slot < 20)
	{
		item = object_fields_get_u32(&((struct object*)player)->fields, PLAYER_FIELD_VISIBLE_ITEM_1_0 + 16 * (slot - 1));
	}
	else
	{
		uint64_t guid = object_fields_get_u64(&((struct object*)player)->fields, PLAYER_FIELD_INV_SLOT_BAG0 + 2 * (slot - 20));
		if (!guid)
			return 0;
		struct item *item_obj = object_get_item(guid);
		if (!item_obj)
		{
			LOG_ERROR("item not found: %lu", guid);
			return 0;
		}
		item = object_fields_get_u32(&((struct object*)item_obj)->fields, OBJECT_FIELD_ENTRY);
	}
	if (item == 0)
		return 0;
	struct wow_dbc_row item_row;
	if (!dbc_get_row_indexed(g_wow->dbc.item, &item_row, item))
	{
		LOG_ERROR("unknown item: %" PRIu32, item);
		return 0;
	}
	struct wow_dbc_row item_display_row;
	if (!dbc_get_row_indexed(g_wow->dbc.item_display_info, &item_display_row, wow_dbc_get_u32(&item_row, 4)))
	{
		LOG_ERROR("unkwown item display: %" PRIu32, item);
		return 0;
	}
	char tmp[512];
	snprintf(tmp, sizeof(tmp), "Interface/Icons/%s", wow_dbc_get_str(&item_display_row, 20));
	lua_pushstring(L, tmp);
	return 1;
}

LUA_FN(GetInventoryItemCount)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 2)
		return luaL_error(L, "Usage: GetInventoryItemCount(\"unit\", id)");
	LUA_UNIMPLEMENTED_FN();
	if (!lua_isinteger(L, 2))
		return luaL_argerror(L, 2, "integer expected");
	int slot = lua_tointeger(L, 2);
	if (slot < 1 || slot > 23)
		return luaL_argerror(L, 2, "invalid slot: < 1 || > 23");
	if (slot >= 20)
	{
		lua_pushinteger(L, 0);
		return 1;
	}
	struct player *player = get_player(L, 1);
	if (!player)
	{
		lua_pushinteger(L, 0);
		return 1;
	}
	uint32_t item = object_fields_get_u32(&((struct object*)player)->fields, PLAYER_FIELD_VISIBLE_ITEM_1_0 + 16 * (slot - 1));
	if (item == 0)
	{
		lua_pushinteger(L, 0);
		return 1;
	}
	lua_pushinteger(L, 1);
	return 1;
}

LUA_FN(GetInventoryItemBroken)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 2)
		return luaL_error(L, "Usage: GetInventoryItemBroken(\"unit\", id)");
	LUA_UNIMPLEMENTED_FN();
	lua_pushnumber(L, 0);
	return 1;
}

LUA_FN(GetInventoryItemCooldown)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 2)
		return luaL_error(L, "Usage: GetInventoryItemCooldown(\"unit\", id)");
	LUA_UNIMPLEMENTED_FN();
	lua_pushnumber(L, 13);
	lua_pushnumber(L, 16);
	lua_pushnumber(L, 0);
	return 3;
}

LUA_FN(IsReferAFriendLinked)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: IsReferAFriendLinked(\"unit\")");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, false);
	return 1;
}

LUA_FN(UnitIsTapped)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: UnitIsTapped(\"unit\")");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, false);
	return 1;
}

LUA_FN(IsUnitOnQuest)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 2)
		return luaL_error(L, "Usage: IsUnitOnQuest(questid, \"unit\")");
	LUA_UNIMPLEMENTED_FN();
	lua_pushboolean(L, false);
	return 1;
}

LUA_FN(UnitPVPRank)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: UnitPVPRank(\"unit\"");
	LUA_UNIMPLEMENTED_FN();
	lua_pushinteger(L, 0);
	return 1;
}

LUA_FN(UnitChannelInfo)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: UnitChannelInfo(\"unit\")");
	LUA_UNIMPLEMENTED_FN();
	lua_pushnil(L);
	return 1;
}

LUA_FN(UnitCastingInfo)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: UnitCastingInfo(\"unit\")");
	LUA_UNIMPLEMENTED_FN();
	lua_pushnil(L);
	return 1;
}

LUA_FN(UnitIsInMyGuild)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: UnitIsInMyGuild(\"unit\")");
	lua_pushboolean(L, false);
	LUA_UNIMPLEMENTED_FN();
	return 1;
}

LUA_FN(TargetUnit)
{
	LUA_VERBOSE_FN();
	int argc = lua_gettop(L);
	if (argc != 1)
		return luaL_error(L, "Usage: TargetUnit(\"unit\")");
	LUA_UNIMPLEMENTED_FN();
	return 0;
}

void register_unit_functions(lua_State *L)
{
	LUA_REGISTER_FN(UnitName);
	LUA_REGISTER_FN(UnitRace);
	LUA_REGISTER_FN(UnitXP);
	LUA_REGISTER_FN(UnitXPMax);
	LUA_REGISTER_FN(UnitLevel);
	LUA_REGISTER_FN(UnitPVPName);
	LUA_REGISTER_FN(UnitIsDeadOrGhost);
	LUA_REGISTER_FN(UnitClass);
	LUA_REGISTER_FN(UnitFactionGroup);
	LUA_REGISTER_FN(UnitIsPossessed);
	LUA_REGISTER_FN(UnitExists);
	LUA_REGISTER_FN(UnitIsUnit);
	LUA_REGISTER_FN(UnitIsPlayer);
	LUA_REGISTER_FN(UnitHealth);
	LUA_REGISTER_FN(UnitHealthMax);
	LUA_REGISTER_FN(UnitIsConnected);
	LUA_REGISTER_FN(UnitInBattleground);
	LUA_REGISTER_FN(UnitMana);
	LUA_REGISTER_FN(UnitManaMax);
	LUA_REGISTER_FN(UnitIsVisible);
	LUA_REGISTER_FN(UnitCanCooperate);
	LUA_REGISTER_FN(UnitCanAttack);
	LUA_REGISTER_FN(UnitIsDead);
	LUA_REGISTER_FN(UnitPlayerControlled);
	LUA_REGISTER_FN(UnitReaction);
	LUA_REGISTER_FN(UnitPowerType);
	LUA_REGISTER_FN(UnitBuff);
	LUA_REGISTER_FN(UnitDebuff);
	LUA_REGISTER_FN(UnitCharacterPoints);
	LUA_REGISTER_FN(UnitSex);
	LUA_REGISTER_FN(UnitIsGhost);
	LUA_REGISTER_FN(UnitIsPVPFreeForAll);
	LUA_REGISTER_FN(UnitIsPVP);
	LUA_REGISTER_FN(UnitIsTalking);
	LUA_REGISTER_FN(GetReadyCheckStatus);
	LUA_REGISTER_FN(UnitResistance);
	LUA_REGISTER_FN(UnitHasRelicSlot);
	LUA_REGISTER_FN(UnitStat);
	LUA_REGISTER_FN(UnitAttackSpeed);
	LUA_REGISTER_FN(UnitArmor);
	LUA_REGISTER_FN(UnitDamage);
	LUA_REGISTER_FN(UnitDefense);
	LUA_REGISTER_FN(GetPlayerBuff);
	LUA_REGISTER_FN(UnitCreatureFamily);
	LUA_REGISTER_FN(GetCritChanceFromAgility);
	LUA_REGISTER_FN(GetUnitMaxHealthModifier);
	LUA_REGISTER_FN(GetUnitHealthRegenRateFromSpirit);
	LUA_REGISTER_FN(UnitAttackPower);
	LUA_REGISTER_FN(GetArmorPenetration);
	LUA_REGISTER_FN(UnitRangedAttackPower);
	LUA_REGISTER_FN(GetCritChance);
	LUA_REGISTER_FN(GetRangedCritChance);
	LUA_REGISTER_FN(GetExpertise);
	LUA_REGISTER_FN(GetExpertisePercent);
	LUA_REGISTER_FN(GetSpellBonusHealing);
	LUA_REGISTER_FN(GetSpellPenetration);
	LUA_REGISTER_FN(GetSpellCritChance);
	LUA_REGISTER_FN(GetDodgeChance);
	LUA_REGISTER_FN(GetParryChance);
	LUA_REGISTER_FN(GetBlockChance);
	LUA_REGISTER_FN(GetShieldBlock);
	LUA_REGISTER_FN(UnitCanAssist);
	LUA_REGISTER_FN(GetUnitHealthModifier);
	LUA_REGISTER_FN(GetSpellCritChanceFromIntellect);
	LUA_REGISTER_FN(UnitIsFriend);
	LUA_REGISTER_FN(UnitClassification);
	LUA_REGISTER_FN(UnitClassBase);
	LUA_REGISTER_FN(GetInventoryItemTexture);
	LUA_REGISTER_FN(GetInventoryItemCount);
	LUA_REGISTER_FN(GetInventoryItemBroken);
	LUA_REGISTER_FN(GetInventoryItemCooldown);
	LUA_REGISTER_FN(IsReferAFriendLinked);
	LUA_REGISTER_FN(UnitIsTapped);
	LUA_REGISTER_FN(IsUnitOnQuest);
	LUA_REGISTER_FN(UnitPVPRank);
	LUA_REGISTER_FN(UnitChannelInfo);
	LUA_REGISTER_FN(UnitCastingInfo);
	LUA_REGISTER_FN(UnitIsInMyGuild);
	LUA_REGISTER_FN(TargetUnit);
}
