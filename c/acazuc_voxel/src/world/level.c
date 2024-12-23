#include "world/level.h"

#include "log.h"

#include <stdlib.h>
#include <string.h>

struct level *level_new(const char *name)
{
	struct level *level = calloc(sizeof(*level), 1);
	if (!level)
	{
		LOG_ERROR("level allocation failed");
		return NULL;
	}
	level->name = strdup(name);
	if (!level->name)
	{
		LOG_ERROR("allocation failed");
		free(level);
		return NULL;
	}
	char path[512];
	snprintf(path, sizeof(path), "saves/%s/level.dat", name);
	gzFile fp = gzopen(path, "r");
	struct nbt_stream *stream = nbt_stream_gzip_new(fp);
	struct nbt_tag *tag = nbt_tag_read(stream);
	nbt_stream_delete(stream);
	if (tag->type != NBT_TAG_COMPOUND)
	{
		LOG_ERROR("invalid level root nbt tag");
		return NULL;
	}
	level->nbt.nbt = (struct nbt_tag_compound*)tag;
	const struct nbt_sanitize_entry dimension_data_entries[] =
	{
		NBT_SANITIZE_END
	};
	const struct nbt_sanitize_entry version_entries[] =
	{
		NBT_SANITIZE_BYTE(&level->nbt.Version.Snapshot, "Snapshot", 0),
		NBT_SANITIZE_INT(&level->nbt.Version.Id, "Id", 0),
		NBT_SANITIZE_STRING(&level->nbt.Version.Name, "Name", ""),
		NBT_SANITIZE_END
	};
	const struct nbt_sanitize_entry game_rules_entries[] =
	{
		NBT_SANITIZE_STRING(&level->nbt.GameRules.doTileDrops, "doTileDrops", "true"),
		NBT_SANITIZE_STRING(&level->nbt.GameRules.doFireTick, "doFireTick", "true"),
		NBT_SANITIZE_STRING(&level->nbt.GameRules.reducedDebugInfo, "reducedDebugInfo", "false"),
		NBT_SANITIZE_STRING(&level->nbt.GameRules.naturalRegeneration, "naturalRegeneration", "true"),
		NBT_SANITIZE_STRING(&level->nbt.GameRules.disableElytraMovementCheck, "disableElytraMovementCheck", "false"),
		NBT_SANITIZE_STRING(&level->nbt.GameRules.doMobLoot, "doMobLoot", "true"),
		NBT_SANITIZE_STRING(&level->nbt.GameRules.keepInventory, "keepInventory", "false"),
		NBT_SANITIZE_STRING(&level->nbt.GameRules.doEntityDrops, "doEntityDrops", "true"),
		NBT_SANITIZE_STRING(&level->nbt.GameRules.mobGriefing, "mobGriefing", "true"),
		NBT_SANITIZE_STRING(&level->nbt.GameRules.randomTickSpeed, "randomTickSpeed", "3"),
		NBT_SANITIZE_STRING(&level->nbt.GameRules.commandBlockOutput, "commandBlockOutput", "true"),
		NBT_SANITIZE_STRING(&level->nbt.GameRules.spawnRadius, "spawnRadius", "10"),
		NBT_SANITIZE_STRING(&level->nbt.GameRules.doMobSpawning, "doMobSpawning", "true"),
		NBT_SANITIZE_STRING(&level->nbt.GameRules.logAdminCommands, "logAdminCommands", "true"),
		NBT_SANITIZE_STRING(&level->nbt.GameRules.spectatorsGenerateChunks, "spectatorsGenerateChunks", "true"),
		NBT_SANITIZE_STRING(&level->nbt.GameRules.sendCommandFeedback, "sendCommandFeedback", "true"),
		NBT_SANITIZE_STRING(&level->nbt.GameRules.doDaylightCycle, "doDaylightCycle", "true"),
		NBT_SANITIZE_STRING(&level->nbt.GameRules.showDeathMessages, "showDeathMessages", "true"),
		NBT_SANITIZE_END
	};
	const struct nbt_sanitize_entry abilities_entries[] =
	{
		NBT_SANITIZE_BYTE(&level->nbt.Player.abilities.invulnerable, "invulnerable", 0),
		NBT_SANITIZE_BYTE(&level->nbt.Player.abilities.mayfly, "mayfly", 0),
		NBT_SANITIZE_BYTE(&level->nbt.Player.abilities.instabuild, "instabuild", 0),
		NBT_SANITIZE_FLOAT(&level->nbt.Player.abilities.walkSpeed, "walkSpeed", 0),
		NBT_SANITIZE_BYTE(&level->nbt.Player.abilities.mayBuild, "mayBuild", 1),
		NBT_SANITIZE_BYTE(&level->nbt.Player.abilities.flying, "flying", 0),
		NBT_SANITIZE_FLOAT(&level->nbt.Player.abilities.flySpeed, "flySpeed", 0),
		NBT_SANITIZE_END
	};
	const struct nbt_sanitize_entry player_entries[] =
	{
		NBT_SANITIZE_INT(&level->nbt.Player.HurtByTimestamp, "HurtByTimestamp", 0),
		NBT_SANITIZE_SHORT(&level->nbt.Player.SleepTimer, "SleepTimer", 0),
		NBT_SANITIZE_LIST(&level->nbt.Player.Attributes, "Attributes", NBT_TAG_COMPOUND),
		NBT_SANITIZE_BYTE(&level->nbt.Player.Invulnerable, "Invulnerable", 0),
		NBT_SANITIZE_BYTE(&level->nbt.Player.FallFlying, "FallFlying", 0),
		NBT_SANITIZE_INT(&level->nbt.Player.PortalCooldown, "PortalCooldown", 0),
		NBT_SANITIZE_FLOAT(&level->nbt.Player.AbsorptionAmount, "AbsorptionAmount", 0),
		NBT_SANITIZE_COMPOUND(&level->nbt.Player.abilities.nbt, "abilities", abilities_entries),
		NBT_SANITIZE_FLOAT(&level->nbt.Player.FallDistance, "FallDistance", 0),
		NBT_SANITIZE_SHORT(&level->nbt.Player.DeathTime, "DeathTime", 0),
		NBT_SANITIZE_INT(&level->nbt.Player.XpSeed, "XpSeed", 0),
		NBT_SANITIZE_INT(&level->nbt.Player.XpTotal, "XpTotal", 0),
		NBT_SANITIZE_INT(&level->nbt.Player.playerGameType, "playerGameType", 0),
		NBT_SANITIZE_LIST(&level->nbt.Player.Motion, "Motion", NBT_TAG_DOUBLE),
		NBT_SANITIZE_LONG(&level->nbt.Player.UUIDLeast, "UUIDLeast", 0),
		NBT_SANITIZE_FLOAT(&level->nbt.Player.Health, "Health", 0),
		NBT_SANITIZE_FLOAT(&level->nbt.Player.foodSaturationLevel, "foodSaturationLevel", 0),
		NBT_SANITIZE_SHORT(&level->nbt.Player.Air, "Air", 0),
		NBT_SANITIZE_BYTE(&level->nbt.Player.OnGround, "OnGround", 0),
		NBT_SANITIZE_INT(&level->nbt.Player.Dimension, "Dimension", 0),
		NBT_SANITIZE_LIST(&level->nbt.Player.Rotation, "Rotation", NBT_TAG_FLOAT),
		NBT_SANITIZE_INT(&level->nbt.Player.XpLevel, "XpLevel", 0),
		NBT_SANITIZE_INT(&level->nbt.Player.Score, "Score", 0),
		NBT_SANITIZE_LONG(&level->nbt.Player.UUIDMost, "UUIDMost", 0),
		NBT_SANITIZE_BYTE(&level->nbt.Player.Sleeping, "Sleeping", 0),
		NBT_SANITIZE_LIST(&level->nbt.Player.Pos, "Pos", NBT_TAG_DOUBLE),
		NBT_SANITIZE_SHORT(&level->nbt.Player.Fire, "Fire", 0),
		NBT_SANITIZE_FLOAT(&level->nbt.Player.XpP, "XpP", 0),
		NBT_SANITIZE_LIST(&level->nbt.Player.EnderItems, "EnderItems", NBT_TAG_COMPOUND),
		NBT_SANITIZE_INT(&level->nbt.Player.DataVersion, "DataVersion", 0),
		NBT_SANITIZE_INT(&level->nbt.Player.foodLevel, "foodLevel", 0),
		NBT_SANITIZE_FLOAT(&level->nbt.Player.foodExhaustionLevel, "foodExhaustionLevel", 0),
		NBT_SANITIZE_SHORT(&level->nbt.Player.HurtTime, "HurtTime", 0),
		NBT_SANITIZE_INT(&level->nbt.Player.SelectedItemSlot, "SelectedItemSlot", 0),
		NBT_SANITIZE_LIST(&level->nbt.Player.Inventory, "Inventory", NBT_TAG_COMPOUND),
		NBT_SANITIZE_INT(&level->nbt.Player.foodTickTimer, "foodTickTimer", 0),
		NBT_SANITIZE_END
	};
	const struct nbt_sanitize_entry data_entries[] =
	{
		NBT_SANITIZE_LONG(&level->nbt.RandomSeed, "RandomSeed", 0),
		NBT_SANITIZE_STRING(&level->nbt.generatorName, "generatorName", ""),
		NBT_SANITIZE_DOUBLE(&level->nbt.BorderCenterZ, "BorderCenterZ", 0),
		NBT_SANITIZE_BYTE(&level->nbt.Difficulty, "Difficulty", 0),
		NBT_SANITIZE_LONG(&level->nbt.BorderSizeLerpTime, "BorderSizeLerpTime", 0),
		NBT_SANITIZE_BYTE(&level->nbt.raining, "raining", 0),
		NBT_SANITIZE_COMPOUND(&level->nbt.DimensionData.nbt, "DimensionData", dimension_data_entries),
		NBT_SANITIZE_LONG(&level->nbt.Time, "Time", 0),
		NBT_SANITIZE_INT(&level->nbt.GameType, "GameType", 0),
		NBT_SANITIZE_BYTE(&level->nbt.MapFeatures, "MapFeatures", 0),
		NBT_SANITIZE_DOUBLE(&level->nbt.BorderCenterX, "BorderCenterX", 0),
		NBT_SANITIZE_DOUBLE(&level->nbt.BorderDamagePerBlock, "BorderDamagePerBlock", 0),
		NBT_SANITIZE_DOUBLE(&level->nbt.BorderWarningBlocks, "BorderWarningBlocks", 0),
		NBT_SANITIZE_DOUBLE(&level->nbt.BorderSizeLerpTarget, "BorderSizeLerpTarget", 0),
		NBT_SANITIZE_COMPOUND(&level->nbt.Version.nbt, "Version", version_entries),
		NBT_SANITIZE_LONG(&level->nbt.DayTime, "DayTime", 0),
		NBT_SANITIZE_BYTE(&level->nbt.initialized, "initialized", 0),
		NBT_SANITIZE_BYTE(&level->nbt.allowCommands, "allowCommands", 0),
		NBT_SANITIZE_LONG(&level->nbt.SizeOnDisk, "SizeOnDisk", 0),
		NBT_SANITIZE_COMPOUND(&level->nbt.GameRules.nbt, "GameRules", game_rules_entries),
		NBT_SANITIZE_COMPOUND(&level->nbt.Player.nbt, "Player", player_entries),
		NBT_SANITIZE_INT(&level->nbt.SpawnY, "SpawnY", 0),
		NBT_SANITIZE_INT(&level->nbt.rainTime, "rainTime", 0),
		NBT_SANITIZE_INT(&level->nbt.thunderTime, "thunderTime", 0),
		NBT_SANITIZE_INT(&level->nbt.SpawnZ, "SpawnZ", 0),
		NBT_SANITIZE_BYTE(&level->nbt.hardcode, "hardcode", 0),
		NBT_SANITIZE_BYTE(&level->nbt.DifficultyLocked, "DifficultyLocked", 0),
		NBT_SANITIZE_INT(&level->nbt.SpawnX, "SpawnX", 0),
		NBT_SANITIZE_INT(&level->nbt.clearWeatherTime, "clearWeatherTime", 0),
		NBT_SANITIZE_BYTE(&level->nbt.thundering, "thundering", 0),
		NBT_SANITIZE_INT(&level->nbt.generatorVersion, "generatorVersion", 0),
		NBT_SANITIZE_INT(&level->nbt.version, "version", 0),
		NBT_SANITIZE_DOUBLE(&level->nbt.BorderSafeZone, "BorderSafeZone", 0),
		NBT_SANITIZE_STRING(&level->nbt.generatorOptions, "generatorOptions", ""),
		NBT_SANITIZE_LONG(&level->nbt.LastPlayed, "LastPlayed", 0),
		NBT_SANITIZE_DOUBLE(&level->nbt.BorderWarningTime, "BorderWarningTime", 0),
		NBT_SANITIZE_STRING(&level->nbt.LevelName, "LevelName", ""),
		NBT_SANITIZE_DOUBLE(&level->nbt.BorderSize, "BorderSize", 0),
		NBT_SANITIZE_INT(&level->nbt.DataVersion, "DataVersion", 0),
		NBT_SANITIZE_END
	};
	const struct nbt_sanitize_entry entries[] =
	{
		NBT_SANITIZE_COMPOUND(&level->nbt.Data, "Data", data_entries),
		NBT_SANITIZE_END
	};
	if (!nbt_sanitize(level->nbt.nbt, entries))
	{
		LOG_ERROR("failed to sanitize");
		abort();
	}
	return level;
}

void level_delete(struct level *level)
{
	if (!level)
		return;
	nbt_tag_delete((struct nbt_tag*)level->nbt.nbt);
	free(level);
}
