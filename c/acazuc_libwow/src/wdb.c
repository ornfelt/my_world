#include "wdb.h"

#include <string.h>
#include <stdlib.h>

static bool read_u8(uint8_t *v, const char *buf, size_t n, size_t *i)
{
	if (*i + 1 > n)
		return false;
	*v = *(uint8_t*)&buf[*i];
	*i += 1;
	return true;
}

static bool read_i32(int32_t *v, const char *buf, size_t n, size_t *i)
{
	if (*i + 4 > n)
		return false;
	*v = *(int32_t*)&buf[*i];
	*i += 4;
	return true;
}

static bool read_u32(uint32_t *v, const char *buf, size_t n, size_t *i)
{
	if (*i + 4 > n)
		return false;
	*v = *(uint32_t*)&buf[*i];
	*i += 4;
	return true;
}

static bool read_u64(uint64_t *v, const char *buf, size_t n, size_t *i)
{
	if (*i + 8 > n)
		return false;
	*v = *(uint64_t*)&buf[*i];
	*i += 8;
	return true;
}

static bool read_flt(float *v, const char *buf, size_t n, size_t *i)
{
	if (*i + 4 > n)
		return false;
	*v = *(float*)&buf[*i];
	*i += 4;
	return true;
}

static bool read_str(char **str, const char *buf, size_t n, size_t *i)
{
	size_t len = strlen(&buf[*i]);
	if (*i + len + 1 > n)
		return false;
	*str = WOW_MALLOC(len + 1);
	if (!*str)
		return false;
	memcpy(*str, &buf[*i], len + 1);
	*i += len + 1;
	return true;
}

static bool write_u8(FILE *fp, uint8_t value)
{
	return fwrite(&value, 1, sizeof(value), fp) == sizeof(value);
}

static bool write_i32(FILE *fp, int32_t value)
{
	return fwrite(&value, 1, sizeof(value), fp) == sizeof(value);
}

static bool write_u32(FILE *fp, uint32_t value)
{
	return fwrite(&value, 1, sizeof(value), fp) == sizeof(value);
}

static bool write_u64(FILE *fp, uint64_t value)
{
	return fwrite(&value, 1, sizeof(value), fp) == sizeof(value);
}

static bool write_flt(FILE *fp, float value)
{
	return fwrite(&value, 1, sizeof(value), fp) == sizeof(value);
}

static bool write_str(FILE *fp, const char *str, size_t len)
{
	return fwrite(str, 1, len + 1, fp) == len + 1;
}

int wow_wdb_creature_read(FILE *fp, struct wow_wdb_creature *creature)
{
	uint32_t len;
	char buf[256];
	if (fread(&creature->id, 1, sizeof(creature->id), fp) != sizeof(creature->id)
	 || fread(&len, 1, sizeof(len), fp) != sizeof(len)
	 || len >= sizeof(buf)
	 || fread(buf, 1, len, fp) != len)
		return -1;
	if (!len)
		return 0;
	buf[len] = '\0'; /* safety first */
	size_t i = 0;
	creature->name[0] = NULL;
	creature->name[1] = NULL;
	creature->name[2] = NULL;
	creature->name[3] = NULL;
	creature->subname = NULL;
	creature->icon = NULL;
	if (!read_str(&creature->name[0], buf, len, &i)
	 || !read_str(&creature->name[1], buf, len, &i)
	 || !read_str(&creature->name[2], buf, len, &i)
	 || !read_str(&creature->name[3], buf, len, &i)
	 || !read_str(&creature->subname, buf, len, &i)
	 || !read_str(&creature->icon, buf, len, &i)
	 || !read_u32(&creature->type_flags, buf, len, &i)
	 || !read_u32(&creature->type, buf, len, &i)
	 || !read_u32(&creature->family, buf, len, &i)
	 || !read_u32(&creature->rank, buf, len, &i)
	 || !read_u32(&creature->unk, buf, len, &i)
	 || !read_u32(&creature->pet_spell_data_id, buf, len, &i)
	 || !read_u32(&creature->model_id[0], buf, len, &i)
	 || !read_u32(&creature->model_id[1], buf, len, &i)
	 || !read_u32(&creature->model_id[2], buf, len, &i)
	 || !read_u32(&creature->model_id[3], buf, len, &i)
	 || !read_flt(&creature->health_multiplier, buf, len, &i)
	 || !read_flt(&creature->power_multiplier, buf, len, &i)
	 || !read_u8(&creature->racial_leader, buf, len, &i)
	 || i != len)
	{
		wow_wdb_creature_destroy(creature);
		return -1;
	}
	return 1;
}

bool wow_wdb_creature_write(FILE *fp, const struct wow_wdb_creature *creature)
{
	uint32_t len = 0;
	len += sizeof(uint32_t) * 10;
	len += sizeof(float) * 2;
	len += sizeof(uint8_t) * 1;
	size_t name_len[4] =
	{
		strlen(creature->name[0]),
		strlen(creature->name[1]),
		strlen(creature->name[2]),
		strlen(creature->name[3]),
	};
	size_t subname_len = strlen(creature->subname);
	size_t icon_len = strlen(creature->icon);
	len += name_len[0] + 1;
	len += name_len[1] + 1;
	len += name_len[2] + 1;
	len += name_len[3] + 1;
	len += subname_len + 1;
	len += icon_len + 1;
	if (!write_u32(fp, creature->id)
	 || !write_u32(fp, len)
	 || !write_str(fp, creature->name[0], name_len[0])
	 || !write_str(fp, creature->name[1], name_len[1])
	 || !write_str(fp, creature->name[2], name_len[2])
	 || !write_str(fp, creature->name[3], name_len[3])
	 || !write_str(fp, creature->subname, subname_len)
	 || !write_str(fp, creature->icon, icon_len)
	 || !write_u32(fp, creature->type_flags)
	 || !write_u32(fp, creature->type)
	 || !write_u32(fp, creature->family)
	 || !write_u32(fp, creature->rank)
	 || !write_u32(fp, creature->unk)
	 || !write_u32(fp, creature->pet_spell_data_id)
	 || !write_u32(fp, creature->model_id[0])
	 || !write_u32(fp, creature->model_id[1])
	 || !write_u32(fp, creature->model_id[2])
	 || !write_u32(fp, creature->model_id[3])
	 || !write_flt(fp, creature->health_multiplier)
	 || !write_flt(fp, creature->power_multiplier)
	 || !write_u8(fp, creature->racial_leader))
		return false;
	return true;
}

void wow_wdb_creature_destroy(struct wow_wdb_creature *creature)
{
	if (!creature)
		return;
	WOW_FREE(creature->name[0]);
	WOW_FREE(creature->name[1]);
	WOW_FREE(creature->name[2]);
	WOW_FREE(creature->name[3]);
	WOW_FREE(creature->subname);
	WOW_FREE(creature->icon);
}

int wow_wdb_item_read(FILE *fp, struct wow_wdb_item *item)
{
	uint32_t len;
	char buf[1024];
	if (fread(&item->id, 1, sizeof(item->id), fp) != sizeof(item->id)
	 || fread(&len, 1, sizeof(len), fp) != sizeof(len)
	 || len >= sizeof(buf)
	 || fread(buf, 1, len, fp) != len)
		return -1;
	if (!len)
		return 0;
	buf[len] = '\0'; /* safety first */
	size_t i = 0;
	item->name[0] = NULL;
	item->name[1] = NULL;
	item->name[2] = NULL;
	item->name[3] = NULL;
	item->description = NULL;
	if (!read_u32(&item->type, buf, len, &i)
	 || !read_u32(&item->subtype, buf, len, &i)
	 || !read_u32(&item->unk, buf, len, &i)
	 || !read_str(&item->name[0], buf, len, &i)
	 || !read_str(&item->name[1], buf, len, &i)
	 || !read_str(&item->name[2], buf, len, &i)
	 || !read_str(&item->name[3], buf, len, &i)
	 || !read_u32(&item->display_id, buf, len, &i)
	 || !read_u32(&item->quality, buf, len, &i)
	 || !read_u32(&item->flags, buf, len, &i)
	 || !read_u32(&item->buy_price, buf, len, &i)
	 || !read_u32(&item->sell_price, buf, len, &i)
	 || !read_u32(&item->inventory_slot, buf, len, &i)
	 || !read_u32(&item->class_mask, buf, len, &i)
	 || !read_u32(&item->race_mask, buf, len, &i)
	 || !read_u32(&item->level, buf, len, &i)
	 || !read_u32(&item->required_level, buf, len, &i)
	 || !read_u32(&item->required_skill, buf, len, &i)
	 || !read_u32(&item->required_skill_rank, buf, len, &i)
	 || !read_u32(&item->required_spell, buf, len, &i)
	 || !read_u32(&item->required_honor_rank, buf, len, &i)
	 || !read_u32(&item->required_city_rank, buf, len, &i)
	 || !read_u32(&item->required_reputation_faction, buf, len, &i)
	 || !read_u32(&item->required_reputation_rank, buf, len, &i)
	 || !read_u32(&item->unique_count, buf, len, &i)
	 || !read_u32(&item->stack_count, buf, len, &i)
	 || !read_u32(&item->container_slots, buf, len, &i)
	 || !read_u32(&item->attributes[0].id, buf, len, &i)
	 || !read_i32(&item->attributes[0].value, buf, len, &i)
	 || !read_u32(&item->attributes[1].id, buf, len, &i)
	 || !read_i32(&item->attributes[1].value, buf, len, &i)
	 || !read_u32(&item->attributes[2].id, buf, len, &i)
	 || !read_i32(&item->attributes[2].value, buf, len, &i)
	 || !read_u32(&item->attributes[3].id, buf, len, &i)
	 || !read_i32(&item->attributes[3].value, buf, len, &i)
	 || !read_u32(&item->attributes[4].id, buf, len, &i)
	 || !read_i32(&item->attributes[4].value, buf, len, &i)
	 || !read_u32(&item->attributes[5].id, buf, len, &i)
	 || !read_i32(&item->attributes[5].value, buf, len, &i)
	 || !read_u32(&item->attributes[6].id, buf, len, &i)
	 || !read_i32(&item->attributes[6].value, buf, len, &i)
	 || !read_u32(&item->attributes[7].id, buf, len, &i)
	 || !read_i32(&item->attributes[7].value, buf, len, &i)
	 || !read_u32(&item->attributes[8].id, buf, len, &i)
	 || !read_i32(&item->attributes[8].value, buf, len, &i)
	 || !read_u32(&item->attributes[9].id, buf, len, &i)
	 || !read_i32(&item->attributes[9].value, buf, len, &i)
	 || !read_flt(&item->damages[0].min, buf, len, &i)
	 || !read_flt(&item->damages[0].max, buf, len, &i)
	 || !read_u32(&item->damages[0].type, buf, len, &i)
	 || !read_flt(&item->damages[1].min, buf, len, &i)
	 || !read_flt(&item->damages[1].max, buf, len, &i)
	 || !read_u32(&item->damages[1].type, buf, len, &i)
	 || !read_flt(&item->damages[2].min, buf, len, &i)
	 || !read_flt(&item->damages[2].max, buf, len, &i)
	 || !read_u32(&item->damages[2].type, buf, len, &i)
	 || !read_flt(&item->damages[3].min, buf, len, &i)
	 || !read_flt(&item->damages[3].max, buf, len, &i)
	 || !read_u32(&item->damages[3].type, buf, len, &i)
	 || !read_flt(&item->damages[4].min, buf, len, &i)
	 || !read_flt(&item->damages[4].max, buf, len, &i)
	 || !read_u32(&item->damages[4].type, buf, len, &i)
	 || !read_u32(&item->resistances[0], buf, len, &i)
	 || !read_u32(&item->resistances[1], buf, len, &i)
	 || !read_u32(&item->resistances[2], buf, len, &i)
	 || !read_u32(&item->resistances[3], buf, len, &i)
	 || !read_u32(&item->resistances[4], buf, len, &i)
	 || !read_u32(&item->resistances[5], buf, len, &i)
	 || !read_u32(&item->resistances[6], buf, len, &i)
	 || !read_u32(&item->weapon_speed, buf, len, &i)
	 || !read_u32(&item->ammo_type, buf, len, &i)
	 || !read_flt(&item->ranged_mod_range, buf, len, &i)
	 || !read_u32(&item->spells[0].id, buf, len, &i)
	 || !read_u32(&item->spells[0].trigger, buf, len, &i)
	 || !read_u32(&item->spells[0].charges, buf, len, &i)
	 || !read_u32(&item->spells[0].cooldown, buf, len, &i)
	 || !read_u32(&item->spells[0].category, buf, len, &i)
	 || !read_u32(&item->spells[0].category_cooldown, buf, len, &i)
	 || !read_u32(&item->spells[1].id, buf, len, &i)
	 || !read_u32(&item->spells[1].trigger, buf, len, &i)
	 || !read_u32(&item->spells[1].charges, buf, len, &i)
	 || !read_u32(&item->spells[1].cooldown, buf, len, &i)
	 || !read_u32(&item->spells[1].category, buf, len, &i)
	 || !read_u32(&item->spells[1].category_cooldown, buf, len, &i)
	 || !read_u32(&item->spells[2].id, buf, len, &i)
	 || !read_u32(&item->spells[2].trigger, buf, len, &i)
	 || !read_u32(&item->spells[2].charges, buf, len, &i)
	 || !read_u32(&item->spells[2].cooldown, buf, len, &i)
	 || !read_u32(&item->spells[2].category, buf, len, &i)
	 || !read_u32(&item->spells[2].category_cooldown, buf, len, &i)
	 || !read_u32(&item->spells[3].id, buf, len, &i)
	 || !read_u32(&item->spells[3].trigger, buf, len, &i)
	 || !read_u32(&item->spells[3].charges, buf, len, &i)
	 || !read_u32(&item->spells[3].cooldown, buf, len, &i)
	 || !read_u32(&item->spells[3].category, buf, len, &i)
	 || !read_u32(&item->spells[3].category_cooldown, buf, len, &i)
	 || !read_u32(&item->spells[4].id, buf, len, &i)
	 || !read_u32(&item->spells[4].trigger, buf, len, &i)
	 || !read_u32(&item->spells[4].charges, buf, len, &i)
	 || !read_u32(&item->spells[4].cooldown, buf, len, &i)
	 || !read_u32(&item->spells[4].category, buf, len, &i)
	 || !read_u32(&item->spells[4].category_cooldown, buf, len, &i)
	 || !read_u32(&item->bonding, buf, len, &i)
	 || !read_str(&item->description, buf, len, &i)
	 || !read_u32(&item->page_text, buf, len, &i)
	 || !read_u32(&item->language, buf, len, &i)
	 || !read_u32(&item->page_material, buf, len, &i)
	 || !read_u32(&item->start_quest, buf, len, &i)
	 || !read_u32(&item->lock, buf, len, &i)
	 || !read_u32(&item->material, buf, len, &i)
	 || !read_u32(&item->sheathe_type, buf, len, &i)
	 || !read_u32(&item->random_property, buf, len, &i)
	 || !read_u32(&item->random_suffix, buf, len, &i)
	 || !read_u32(&item->block, buf, len, &i)
	 || !read_u32(&item->item_set, buf, len, &i)
	 || !read_u32(&item->durability, buf, len, &i)
	 || !read_u32(&item->area, buf, len, &i)
	 || !read_u32(&item->map, buf, len, &i)
	 || !read_u32(&item->bag_family, buf, len, &i)
	 || !read_u32(&item->totem_category, buf, len, &i)
	 || !read_u32(&item->sockets[0].color, buf, len, &i)
	 || !read_u32(&item->sockets[0].content, buf, len, &i)
	 || !read_u32(&item->sockets[1].color, buf, len, &i)
	 || !read_u32(&item->sockets[1].content, buf, len, &i)
	 || !read_u32(&item->sockets[2].color, buf, len, &i)
	 || !read_u32(&item->sockets[2].content, buf, len, &i)
	 || !read_u32(&item->socket_bonus, buf, len, &i)
	 || !read_u32(&item->gem_property, buf, len, &i)
	 || !read_u32(&item->required_disenchant_skill, buf, len, &i)
	 || !read_flt(&item->armor_damage_modifier, buf, len, &i)
	 || !read_u32(&item->duration, buf, len, &i)
	 || i != len)
	{
		wow_wdb_item_destroy(item);
		return -1;
	}
	return 1;
}

bool wow_wdb_item_write(FILE *fp, const struct wow_wdb_item *item)
{
	uint32_t len = 0;
	len += sizeof(uint32_t) * (23 + 2 * 10 + 5 + 9 + 6 * 5 + 17 + 2 * 3 + 4);
	len += sizeof(float) * (2 * 5 + 2);
	size_t name_len[4] =
	{
		strlen(item->name[0]),
		strlen(item->name[1]),
		strlen(item->name[2]),
		strlen(item->name[3]),
	};
	size_t description_len = strlen(item->description);
	len += name_len[0] + 1;
	len += name_len[1] + 1;
	len += name_len[2] + 1;
	len += name_len[3] + 1;
	len += description_len + 1;
	if (!write_u32(fp, item->id)
	 || !write_u32(fp, len)
	 || !write_u32(fp, item->type)
	 || !write_u32(fp, item->subtype)
	 || !write_u32(fp, item->unk)
	 || !write_str(fp, item->name[0], name_len[0])
	 || !write_str(fp, item->name[1], name_len[1])
	 || !write_str(fp, item->name[2], name_len[2])
	 || !write_str(fp, item->name[2], name_len[3])
	 || !write_u32(fp, item->display_id)
	 || !write_u32(fp, item->quality)
	 || !write_u32(fp, item->flags)
	 || !write_u32(fp, item->buy_price)
	 || !write_u32(fp, item->sell_price)
	 || !write_u32(fp, item->inventory_slot)
	 || !write_u32(fp, item->class_mask)
	 || !write_u32(fp, item->race_mask)
	 || !write_u32(fp, item->level)
	 || !write_u32(fp, item->required_level)
	 || !write_u32(fp, item->required_skill)
	 || !write_u32(fp, item->required_skill_rank)
	 || !write_u32(fp, item->required_spell)
	 || !write_u32(fp, item->required_honor_rank)
	 || !write_u32(fp, item->required_city_rank)
	 || !write_u32(fp, item->required_reputation_faction)
	 || !write_u32(fp, item->required_reputation_rank)
	 || !write_u32(fp, item->unique_count)
	 || !write_u32(fp, item->stack_count)
	 || !write_u32(fp, item->container_slots)
	 || !write_u32(fp, item->attributes[0].id)
	 || !write_i32(fp, item->attributes[0].value)
	 || !write_u32(fp, item->attributes[1].id)
	 || !write_i32(fp, item->attributes[1].value)
	 || !write_u32(fp, item->attributes[2].id)
	 || !write_i32(fp, item->attributes[2].value)
	 || !write_u32(fp, item->attributes[3].id)
	 || !write_i32(fp, item->attributes[3].value)
	 || !write_u32(fp, item->attributes[4].id)
	 || !write_i32(fp, item->attributes[4].value)
	 || !write_u32(fp, item->attributes[5].id)
	 || !write_i32(fp, item->attributes[5].value)
	 || !write_u32(fp, item->attributes[6].id)
	 || !write_i32(fp, item->attributes[6].value)
	 || !write_u32(fp, item->attributes[7].id)
	 || !write_i32(fp, item->attributes[7].value)
	 || !write_u32(fp, item->attributes[8].id)
	 || !write_i32(fp, item->attributes[8].value)
	 || !write_u32(fp, item->attributes[9].id)
	 || !write_i32(fp, item->attributes[9].value)
	 || !write_flt(fp, item->damages[0].min)
	 || !write_flt(fp, item->damages[0].max)
	 || !write_u32(fp, item->damages[0].type)
	 || !write_flt(fp, item->damages[1].min)
	 || !write_flt(fp, item->damages[1].max)
	 || !write_u32(fp, item->damages[1].type)
	 || !write_flt(fp, item->damages[2].min)
	 || !write_flt(fp, item->damages[2].max)
	 || !write_u32(fp, item->damages[2].type)
	 || !write_flt(fp, item->damages[3].min)
	 || !write_flt(fp, item->damages[3].max)
	 || !write_u32(fp, item->damages[3].type)
	 || !write_flt(fp, item->damages[4].min)
	 || !write_flt(fp, item->damages[4].max)
	 || !write_u32(fp, item->damages[4].type)
	 || !write_u32(fp, item->resistances[0])
	 || !write_u32(fp, item->resistances[1])
	 || !write_u32(fp, item->resistances[2])
	 || !write_u32(fp, item->resistances[3])
	 || !write_u32(fp, item->resistances[4])
	 || !write_u32(fp, item->resistances[5])
	 || !write_u32(fp, item->resistances[6])
	 || !write_u32(fp, item->weapon_speed)
	 || !write_u32(fp, item->ammo_type)
	 || !write_flt(fp, item->ranged_mod_range)
	 || !write_u32(fp, item->spells[0].id)
	 || !write_u32(fp, item->spells[0].trigger)
	 || !write_u32(fp, item->spells[0].charges)
	 || !write_u32(fp, item->spells[0].cooldown)
	 || !write_u32(fp, item->spells[0].category)
	 || !write_u32(fp, item->spells[0].category_cooldown)
	 || !write_u32(fp, item->spells[1].id)
	 || !write_u32(fp, item->spells[1].trigger)
	 || !write_u32(fp, item->spells[1].charges)
	 || !write_u32(fp, item->spells[1].cooldown)
	 || !write_u32(fp, item->spells[1].category)
	 || !write_u32(fp, item->spells[1].category_cooldown)
	 || !write_u32(fp, item->spells[2].id)
	 || !write_u32(fp, item->spells[2].trigger)
	 || !write_u32(fp, item->spells[2].charges)
	 || !write_u32(fp, item->spells[2].cooldown)
	 || !write_u32(fp, item->spells[2].category)
	 || !write_u32(fp, item->spells[2].category_cooldown)
	 || !write_u32(fp, item->spells[3].id)
	 || !write_u32(fp, item->spells[3].trigger)
	 || !write_u32(fp, item->spells[3].charges)
	 || !write_u32(fp, item->spells[3].cooldown)
	 || !write_u32(fp, item->spells[3].category)
	 || !write_u32(fp, item->spells[3].category_cooldown)
	 || !write_u32(fp, item->spells[4].id)
	 || !write_u32(fp, item->spells[4].trigger)
	 || !write_u32(fp, item->spells[4].charges)
	 || !write_u32(fp, item->spells[4].cooldown)
	 || !write_u32(fp, item->spells[4].category)
	 || !write_u32(fp, item->spells[4].category_cooldown)
	 || !write_u32(fp, item->bonding)
	 || !write_str(fp, item->description, description_len)
	 || !write_u32(fp, item->page_text)
	 || !write_u32(fp, item->language)
	 || !write_u32(fp, item->page_material)
	 || !write_u32(fp, item->start_quest)
	 || !write_u32(fp, item->lock)
	 || !write_u32(fp, item->material)
	 || !write_u32(fp, item->sheathe_type)
	 || !write_u32(fp, item->random_property)
	 || !write_u32(fp, item->random_suffix)
	 || !write_u32(fp, item->block)
	 || !write_u32(fp, item->item_set)
	 || !write_u32(fp, item->durability)
	 || !write_u32(fp, item->area)
	 || !write_u32(fp, item->map)
	 || !write_u32(fp, item->bag_family)
	 || !write_u32(fp, item->totem_category)
	 || !write_u32(fp, item->sockets[0].color)
	 || !write_u32(fp, item->sockets[0].content)
	 || !write_u32(fp, item->sockets[1].color)
	 || !write_u32(fp, item->sockets[1].content)
	 || !write_u32(fp, item->sockets[2].color)
	 || !write_u32(fp, item->sockets[2].content)
	 || !write_u32(fp, item->socket_bonus)
	 || !write_u32(fp, item->gem_property)
	 || !write_u32(fp, item->required_disenchant_skill)
	 || !write_flt(fp, item->armor_damage_modifier)
	 || !write_u32(fp, item->duration))
		return false;
	return true;
}

void wow_wdb_item_destroy(struct wow_wdb_item *item)
{
	if (!item)
		return;
	WOW_FREE(item->name[0]);
	WOW_FREE(item->name[1]);
	WOW_FREE(item->name[2]);
	WOW_FREE(item->name[3]);
	WOW_FREE(item->description);
}

int wow_wdb_name_read(FILE *fp, struct wow_wdb_name *name)
{
	uint32_t len;
	char buf[1024];
	if (fread(&name->guid, 1, sizeof(name->guid), fp) != sizeof(name->guid)
	 || fread(&len, 1, sizeof(len), fp) != sizeof(len)
	 || len >= sizeof(buf)
	 || fread(buf, 1, len, fp) != len)
		return -1;
	if (!len)
		return 0;
	buf[len] = '\0'; /* safety first */
	size_t i = 0;
	name->name = NULL;
	if (!read_str(&name->name, buf, len, &i)
	 || !read_u32(&name->race, buf, len, &i)
	 || !read_u32(&name->gender, buf, len, &i)
	 || !read_u32(&name->class_id, buf, len, &i)
	 || !read_u8(&name->realm, buf, len, &i)
	 || i != len)
	{
		wow_wdb_name_destroy(name);
		return -1;
	}
	return 1;
}

bool wow_wdb_name_write(FILE *fp, const struct wow_wdb_name *name)
{
	uint32_t len = 0;
	len += sizeof(uint32_t) * 3;
	len += sizeof(uint8_t) * 1;
	size_t name_len = strlen(name->name);
	len += name_len + 1;
	if (!write_u64(fp, name->guid)
	 || !write_u32(fp, len)
	 || !write_str(fp, name->name, name_len)
	 || !write_u32(fp, name->race)
	 || !write_u32(fp, name->gender)
	 || !write_u32(fp, name->class_id)
	 || !write_u8(fp, name->realm))
		return false;
	return true;
}

void wow_wdb_name_destroy(struct wow_wdb_name *name)
{
	if (!name)
		return;
	WOW_FREE(name->name);
}

int wow_wdb_guild_read(FILE *fp, struct wow_wdb_guild *guild)
{
	uint32_t len;
	char buf[1024];
	if (fread(&guild->id, 1, sizeof(guild->id), fp) != sizeof(guild->id)
	 || fread(&len, 1, sizeof(len), fp) != sizeof(len)
	 || len >= sizeof(buf)
	 || fread(buf, 1, len, fp) != len)
		return -1;
	if (!len)
		return 0;
	buf[len] = '\0'; /* safety first */
	size_t i = 0;
	guild->name = NULL;
	if (!read_str(&guild->name, buf, len, &i)
	 || !read_str(&guild->ranks[0], buf, len, &i)
	 || !read_str(&guild->ranks[1], buf, len, &i)
	 || !read_str(&guild->ranks[2], buf, len, &i)
	 || !read_str(&guild->ranks[3], buf, len, &i)
	 || !read_str(&guild->ranks[4], buf, len, &i)
	 || !read_str(&guild->ranks[5], buf, len, &i)
	 || !read_str(&guild->ranks[6], buf, len, &i)
	 || !read_str(&guild->ranks[7], buf, len, &i)
	 || !read_str(&guild->ranks[8], buf, len, &i)
	 || !read_str(&guild->ranks[9], buf, len, &i)
	 || !read_u32(&guild->emblem_style, buf, len, &i)
	 || !read_u32(&guild->emblem_color, buf, len, &i)
	 || !read_u32(&guild->border_style, buf, len, &i)
	 || !read_u32(&guild->border_color, buf, len, &i)
	 || !read_u32(&guild->background_color, buf, len, &i)
	 || i != len)
	{
		wow_wdb_guild_destroy(guild);
		return -1;
	}
	return 1;
}

bool wow_wdb_guild_write(FILE *fp, const struct wow_wdb_guild *guild)
{
	uint32_t len = 0;
	len += sizeof(uint32_t) * 5;
	size_t name_len = strlen(guild->name);
	size_t ranks_len[10] =
	{
		strlen(guild->ranks[0]),
		strlen(guild->ranks[1]),
		strlen(guild->ranks[2]),
		strlen(guild->ranks[3]),
		strlen(guild->ranks[4]),
		strlen(guild->ranks[5]),
		strlen(guild->ranks[6]),
		strlen(guild->ranks[7]),
		strlen(guild->ranks[8]),
		strlen(guild->ranks[9]),
	};
	len += name_len + 1;
	len += ranks_len[0] + 1;
	len += ranks_len[1] + 1;
	len += ranks_len[2] + 1;
	len += ranks_len[3] + 1;
	len += ranks_len[4] + 1;
	len += ranks_len[5] + 1;
	len += ranks_len[6] + 1;
	len += ranks_len[7] + 1;
	len += ranks_len[8] + 1;
	len += ranks_len[9] + 1;
	if (!write_u32(fp, guild->id)
	 || !write_u32(fp, len)
	 || !write_str(fp, guild->name, name_len)
	 || !write_str(fp, guild->ranks[0], ranks_len[0])
	 || !write_str(fp, guild->ranks[1], ranks_len[1])
	 || !write_str(fp, guild->ranks[2], ranks_len[2])
	 || !write_str(fp, guild->ranks[3], ranks_len[3])
	 || !write_str(fp, guild->ranks[4], ranks_len[4])
	 || !write_str(fp, guild->ranks[5], ranks_len[5])
	 || !write_str(fp, guild->ranks[6], ranks_len[6])
	 || !write_str(fp, guild->ranks[7], ranks_len[7])
	 || !write_str(fp, guild->ranks[8], ranks_len[8])
	 || !write_str(fp, guild->ranks[9], ranks_len[9])
	 || !write_u32(fp, guild->emblem_style)
	 || !write_u32(fp, guild->emblem_color)
	 || !write_u32(fp, guild->border_style)
	 || !write_u32(fp, guild->border_color)
	 || !write_u32(fp, guild->background_color))
		return false;
	return true;
}

void wow_wdb_guild_destroy(struct wow_wdb_guild *guild)
{
	if (!guild)
		return;
	WOW_FREE(guild->name);
	for (int i = 0; i < 10; ++i)
		WOW_FREE(guild->ranks[i]);
}

int wow_wdb_gameobject_read(FILE *fp, struct wow_wdb_gameobject *gameobject)
{
	uint32_t len;
	char buf[1024];
	if (fread(&gameobject->id, 1, sizeof(gameobject->id), fp) != sizeof(gameobject->id)
	 || fread(&len, 1, sizeof(len), fp) != sizeof(len)
	 || len >= sizeof(buf)
	 || fread(buf, 1, len, fp) != len)
		return -1;
	if (!len)
		return 0;
	buf[len] = '\0'; /* safety first */
	size_t i = 0;
	gameobject->name[0] = NULL;
	gameobject->name[1] = NULL;
	gameobject->name[2] = NULL;
	gameobject->name[3] = NULL;
	gameobject->icon = NULL;
	gameobject->cast_bar_caption = NULL;
	gameobject->unk = NULL;
	if (!read_u32(&gameobject->type, buf, len, &i)
	 || !read_u32(&gameobject->display_id, buf, len, &i)
	 || !read_str(&gameobject->name[0], buf, len, &i)
	 || !read_str(&gameobject->name[1], buf, len, &i)
	 || !read_str(&gameobject->name[2], buf, len, &i)
	 || !read_str(&gameobject->name[3], buf, len, &i)
	 || !read_str(&gameobject->icon, buf, len, &i)
	 || !read_str(&gameobject->cast_bar_caption, buf, len, &i)
	 || !read_str(&gameobject->unk, buf, len, &i)
	 || !read_u32(&gameobject->data[0], buf, len, &i)
	 || !read_u32(&gameobject->data[1], buf, len, &i)
	 || !read_u32(&gameobject->data[2], buf, len, &i)
	 || !read_u32(&gameobject->data[3], buf, len, &i)
	 || !read_u32(&gameobject->data[4], buf, len, &i)
	 || !read_u32(&gameobject->data[5], buf, len, &i)
	 || !read_u32(&gameobject->data[6], buf, len, &i)
	 || !read_u32(&gameobject->data[7], buf, len, &i)
	 || !read_u32(&gameobject->data[8], buf, len, &i)
	 || !read_u32(&gameobject->data[9], buf, len, &i)
	 || !read_u32(&gameobject->data[10], buf, len, &i)
	 || !read_u32(&gameobject->data[11], buf, len, &i)
	 || !read_u32(&gameobject->data[12], buf, len, &i)
	 || !read_u32(&gameobject->data[13], buf, len, &i)
	 || !read_u32(&gameobject->data[14], buf, len, &i)
	 || !read_u32(&gameobject->data[15], buf, len, &i)
	 || !read_u32(&gameobject->data[16], buf, len, &i)
	 || !read_u32(&gameobject->data[17], buf, len, &i)
	 || !read_u32(&gameobject->data[18], buf, len, &i)
	 || !read_u32(&gameobject->data[19], buf, len, &i)
	 || !read_u32(&gameobject->data[20], buf, len, &i)
	 || !read_u32(&gameobject->data[21], buf, len, &i)
	 || !read_u32(&gameobject->data[22], buf, len, &i)
	 || !read_u32(&gameobject->data[23], buf, len, &i)
	 || !read_flt(&gameobject->scale, buf, len, &i)
	 || i != len)
	{
		wow_wdb_gameobject_destroy(gameobject);
		return -1;
	}
	return 1;
}

bool wow_wdb_gameobject_write(FILE *fp, const struct wow_wdb_gameobject *gameobject)
{
	uint32_t len = 0;
	len += sizeof(uint32_t) * 26;
	len += sizeof(float) * 1;
	size_t name_len[4] =
	{
		strlen(gameobject->name[0]),
		strlen(gameobject->name[1]),
		strlen(gameobject->name[2]),
		strlen(gameobject->name[3]),
	};
	size_t icon_len = strlen(gameobject->icon);
	size_t cast_bar_caption_len = strlen(gameobject->cast_bar_caption);
	size_t unk_len = strlen(gameobject->unk);
	len += name_len[0] + 1;
	len += name_len[1] + 1;
	len += name_len[2] + 1;
	len += name_len[3] + 1;
	len += icon_len + 1;
	len += cast_bar_caption_len + 1;
	len += unk_len + 1;
	if (!write_u32(fp, gameobject->id)
	 || !write_u32(fp, len)
	 || !write_u32(fp, gameobject->type)
	 || !write_u32(fp, gameobject->display_id)
	 || !write_str(fp, gameobject->name[0], name_len[0])
	 || !write_str(fp, gameobject->name[1], name_len[1])
	 || !write_str(fp, gameobject->name[2], name_len[2])
	 || !write_str(fp, gameobject->name[3], name_len[3])
	 || !write_str(fp, gameobject->icon, icon_len)
	 || !write_str(fp, gameobject->cast_bar_caption, cast_bar_caption_len)
	 || !write_str(fp, gameobject->unk, unk_len)
	 || !write_u32(fp, gameobject->data[0])
	 || !write_u32(fp, gameobject->data[1])
	 || !write_u32(fp, gameobject->data[2])
	 || !write_u32(fp, gameobject->data[3])
	 || !write_u32(fp, gameobject->data[4])
	 || !write_u32(fp, gameobject->data[5])
	 || !write_u32(fp, gameobject->data[6])
	 || !write_u32(fp, gameobject->data[7])
	 || !write_u32(fp, gameobject->data[8])
	 || !write_u32(fp, gameobject->data[9])
	 || !write_u32(fp, gameobject->data[10])
	 || !write_u32(fp, gameobject->data[11])
	 || !write_u32(fp, gameobject->data[12])
	 || !write_u32(fp, gameobject->data[13])
	 || !write_u32(fp, gameobject->data[14])
	 || !write_u32(fp, gameobject->data[15])
	 || !write_u32(fp, gameobject->data[16])
	 || !write_u32(fp, gameobject->data[17])
	 || !write_u32(fp, gameobject->data[18])
	 || !write_u32(fp, gameobject->data[19])
	 || !write_u32(fp, gameobject->data[20])
	 || !write_u32(fp, gameobject->data[21])
	 || !write_u32(fp, gameobject->data[22])
	 || !write_u32(fp, gameobject->data[23])
	 || !write_flt(fp, gameobject->scale))
		return false;
	return true;
}

void wow_wdb_gameobject_destroy(struct wow_wdb_gameobject *gameobject)
{
	if (!gameobject)
		return;
	WOW_FREE(gameobject->name[0]);
	WOW_FREE(gameobject->name[1]);
	WOW_FREE(gameobject->name[2]);
	WOW_FREE(gameobject->name[3]);
	WOW_FREE(gameobject->icon);
	WOW_FREE(gameobject->cast_bar_caption);
}
