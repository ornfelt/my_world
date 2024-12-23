#ifndef LIBWOW_WDB_H
#define LIBWOW_WDB_H

#include "common.h"

#include <stdbool.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define WOW_WDB_MAGIC_WMOB 0x574D4F42
#define WOW_WDB_MAGIC_WIDB 0x57494442
#define WOW_WDB_MAGIC_WGOB 0x57474F42
#define WOW_WDB_MAGIC_WNAM 0x574E414D
#define WOW_WDB_MAGIC_WGLD 0x57474C44

struct wow_wdb_name
{
	uint64_t guid;
	char *name;
	uint32_t race;
	uint32_t gender;
	uint32_t class_id;
	uint8_t realm;
};

struct wow_wdb_creature
{
	uint32_t id;
	char *name[4];
	char *subname;
	char *icon;
	uint32_t type_flags;
	uint32_t type;
	uint32_t family;
	uint32_t rank;
	uint32_t unk;
	uint32_t pet_spell_data_id;
	uint32_t model_id[4];
	float health_multiplier;
	float power_multiplier;
	uint8_t racial_leader;
};

struct wow_wdb_gameobject
{
	uint32_t id;
	uint32_t type;
	uint32_t display_id;
	char *name[4];
	char *icon;
	char *cast_bar_caption;
	char *unk;
	uint32_t data[24];
	float scale;
};

struct wow_wdb_item
{
	uint32_t id;
	uint32_t type;
	uint32_t subtype;
	uint32_t unk;
	char *name[4];
	uint32_t display_id;
	uint32_t quality;
	uint32_t flags;
	uint32_t buy_price;
	uint32_t sell_price;
	uint32_t inventory_slot;
	uint32_t class_mask;
	uint32_t race_mask;
	uint32_t level;
	uint32_t required_level;
	uint32_t required_skill;
	uint32_t required_skill_rank;
	uint32_t required_spell;
	uint32_t required_honor_rank;
	uint32_t required_city_rank;
	uint32_t required_reputation_faction;
	uint32_t required_reputation_rank;
	uint32_t unique_count;
	uint32_t stack_count;
	uint32_t container_slots;
	struct
	{
		uint32_t id;
		int32_t value;
	} attributes[10];
	struct
	{
		float min;
		float max;
		uint32_t type;
	} damages[5];
	uint32_t resistances[7];
	uint32_t weapon_speed;
	uint32_t ammo_type;
	float ranged_mod_range;
	struct
	{
		uint32_t id;
		uint32_t trigger;
		uint32_t charges;
		uint32_t cooldown;
		uint32_t category;
		uint32_t category_cooldown;
	} spells[5];
	uint32_t bonding;
	char *description;
	uint32_t page_text;
	uint32_t language;
	uint32_t page_material;
	uint32_t start_quest;
	uint32_t lock;
	uint32_t material;
	uint32_t sheathe_type;
	uint32_t random_property;
	uint32_t random_suffix;
	uint32_t block;
	uint32_t item_set;
	uint32_t durability;
	uint32_t area;
	uint32_t map;
	uint32_t bag_family;
	uint32_t totem_category;
	struct
	{
		uint32_t color;
		uint32_t content;
	} sockets[3];
	uint32_t socket_bonus;
	uint32_t gem_property;
	uint32_t required_disenchant_skill;
	float armor_damage_modifier;
	uint32_t duration;
};

struct wow_wdb_guild
{
	uint32_t id;
	char *name;
	char *ranks[10];
	uint32_t emblem_style;
	uint32_t emblem_color;
	uint32_t border_style;
	uint32_t border_color;
	uint32_t background_color;
};

struct wow_wdb_header
{
	uint32_t id;
	uint32_t version;
	uint32_t locale;
	uint32_t record_size;
	uint32_t record_version;
};

int wow_wdb_creature_read(FILE *fp, struct wow_wdb_creature *creature);
bool wow_wdb_creature_write(FILE *fp, const struct wow_wdb_creature *creature);
void wow_wdb_creature_destroy(struct wow_wdb_creature *creature);

int wow_wdb_item_read(FILE *fp, struct wow_wdb_item *item);
bool wow_wdb_item_write(FILE *fp, const struct wow_wdb_item *item);
void wow_wdb_item_destroy(struct wow_wdb_item *item);

int wow_wdb_name_read(FILE *fp, struct wow_wdb_name *name);
bool wow_wdb_name_write(FILE *fp, const struct wow_wdb_name *name);
void wow_wdb_name_destroy(struct wow_wdb_name *name);

int wow_wdb_guild_read(FILE *fp, struct wow_wdb_guild *guild);
bool wow_wdb_guild_write(FILE *fp, const struct wow_wdb_guild *guild);
void wow_wdb_guild_destroy(struct wow_wdb_guild *guild);

int wow_wdb_gameobject_read(FILE *fp, struct wow_wdb_gameobject *gameobject);
bool wow_wdb_gameobject_write(FILE *fp, const struct wow_wdb_gameobject *gameobject);
void wow_wdb_gameobject_destroy(struct wow_wdb_gameobject *gameobject);

#ifdef __cplusplus
}
#endif

#endif
