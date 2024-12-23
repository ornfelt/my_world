#ifndef LEVEL_H
#define LEVEL_H

#include <nbt/nbt.h>

struct level_nbt_version
{
	struct nbt_tag_compound *nbt;
	struct nbt_tag_byte *Snapshot;
	struct nbt_tag_int *Id;
	struct nbt_tag_string *Name;
};

struct level_nbt_dimension_data
{
	struct nbt_tag_compound *nbt;
};

struct level_nbt_game_rules
{
	struct nbt_tag_compound *nbt;
	struct nbt_tab_string *doTileDrops;
	struct nbt_tab_string *doFireTick;
	struct nbt_tab_string *reducedDebugInfo;
	struct nbt_tab_string *naturalRegeneration;
	struct nbt_tab_string *disableElytraMovementCheck;
	struct nbt_tab_string *doMobLoot;
	struct nbt_tab_string *keepInventory;
	struct nbt_tab_string *doEntityDrops;
	struct nbt_tab_string *mobGriefing;
	struct nbt_tab_string *randomTickSpeed;
	struct nbt_tab_string *commandBlockOutput;
	struct nbt_tab_string *spawnRadius;
	struct nbt_tab_string *doMobSpawning;
	struct nbt_tab_string *logAdminCommands;
	struct nbt_tab_string *spectatorsGenerateChunks;
	struct nbt_tab_string *sendCommandFeedback;
	struct nbt_tab_string *doDaylightCycle;
	struct nbt_tab_string *showDeathMessages;
};

struct level_nbt_player_abilities
{
	struct nbt_tag_compound *nbt;
	struct nbt_tag_byte *invulnerable;
	struct nbt_tag_byte *mayfly;
	struct nbt_tag_byte *instabuild;
	struct nbt_tag_float *walkSpeed;
	struct nbt_tag_byte *mayBuild;
	struct nbt_tag_byte *flying;
	struct nbt_tag_float *flySpeed;
};

struct level_nbt_player
{
	struct nbt_tag_compound *nbt;
	struct nbt_tag_int *HurtByTimestamp;
	struct nbt_tag_short *SleepTimer;
	struct nbt_tag_list *Attributes;
	struct nbt_tag_byte *Invulnerable;
	struct nbt_tag_byte *FallFlying;
	struct nbt_tag_int *PortalCooldown;
	struct nbt_tag_float *AbsorptionAmount;
	struct level_nbt_player_abilities abilities;
	struct nbt_tag_float *FallDistance;
	struct nbt_tag_short *DeathTime;
	struct nbt_tag_int *XpSeed;
	struct nbt_tag_int *XpTotal;
	struct nbt_tag_int *playerGameType;
	struct nbt_tag_list *Motion;
	struct nbt_tag_long *UUIDLeast;
	struct nbt_tag_float *Health;
	struct nbt_tag_float *foodSaturationLevel;
	struct nbt_tag_short *Air;
	struct nbt_tag_byte *OnGround;
	struct nbt_tag_int *Dimension;
	struct nbt_tag_list *Rotation;
	struct nbt_tag_int *XpLevel;
	struct nbt_tag_int *Score;
	struct nbt_tag_long *UUIDMost;
	struct nbt_tag_byte *Sleeping;
	struct nbt_tag_list *Pos;
	struct nbt_tag_short *Fire;
	struct nbt_tag_float *XpP;
	struct nbt_tag_list *EnderItems;
	struct nbt_tag_int *DataVersion;
	struct nbt_tag_int *foodLevel;
	struct nbt_tag_float *foodExhaustionLevel;
	struct nbt_tag_short *HurtTime;
	struct nbt_tag_int *SelectedItemSlot;
	struct nbt_tag_list *Inventory;
	struct nbt_tag_int *foodTickTimer;
};

struct level_nbt
{
	struct nbt_tag_compound *nbt;
	struct nbt_tag_compound *Data;
	struct nbt_tag_long *RandomSeed;
	struct nbt_tag_string *generatorName;
	struct nbt_tag_double *BorderCenterZ;
	struct nbt_tag_byte *Difficulty;
	struct nbt_tag_long *BorderSizeLerpTime;
	struct nbt_tag_byte *raining;
	struct level_nbt_dimension_data DimensionData;
	struct nbt_tag_long *Time;
	struct nbt_tag_int *GameType;
	struct nbt_tag_byte *MapFeatures;
	struct nbt_tag_double *BorderCenterX;
	struct nbt_tag_double *BorderDamagePerBlock;
	struct nbt_tag_double *BorderWarningBlocks;
	struct nbt_tag_double *BorderSizeLerpTarget;
	struct level_nbt_version Version;
	struct nbt_tag_long *DayTime;
	struct nbt_tag_byte *initialized;
	struct nbt_tag_byte *allowCommands;
	struct nbt_tag_long *SizeOnDisk;
	struct level_nbt_game_rules GameRules;
	struct level_nbt_player Player;
	struct nbt_tag_int *SpawnY;
	struct nbt_tag_int *rainTime;
	struct nbt_tag_int *thunderTime;
	struct nbt_tag_int *SpawnZ;
	struct nbt_tag_byte *hardcode;
	struct nbt_tag_byte *DifficultyLocked;
	struct nbt_tag_int *SpawnX;
	struct nbt_tag_int *clearWeatherTime;
	struct nbt_tag_byte *thundering;
	struct nbt_tag_int *generatorVersion;
	struct nbt_tag_int *version;
	struct nbt_tag_double *BorderSafeZone;
	struct nbt_tag_string *generatorOptions;
	struct nbt_tag_long *LastPlayed;
	struct nbt_tag_double *BorderWarningTime;
	struct nbt_tag_string *LevelName;
	struct nbt_tag_double *BorderSize;
	struct nbt_tag_int *DataVersion;
};

struct level
{
	char *name;
	FILE *lock;
	struct level_nbt nbt;
};

struct level *level_new(const char *name);
void level_delete(struct level *level);

#endif
