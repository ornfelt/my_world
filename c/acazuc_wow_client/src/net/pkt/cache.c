#include "net/packets.h"
#include "net/packet.h"
#include "net/opcode.h"

#include "memory.h"
#include "log.h"
#include "wow.h"
#include "wdb.h"

#include <wow/wdb.h>

#include <string.h>

bool net_cmsg_name_query(struct net_packet_writer *packet, uint64_t guid)
{
	net_packet_writer_init(packet, CMSG_NAME_QUERY);
	if (!net_write_u64(packet, guid))
		return false;
	return true;
}

bool net_smsg_name_query_response(struct net_packet_reader *packet)
{
	struct wow_wdb_name wdb_name;
	const char *name;
	uint8_t dummy;
	if (!net_read_u64(packet, &wdb_name.guid)
	 || !net_read_str(packet, &name)
	 || !net_read_u8(packet, &wdb_name.realm)
	 || !net_read_u32(packet, &wdb_name.race)
	 || !net_read_u32(packet, &wdb_name.gender)
	 || !net_read_u32(packet, &wdb_name.class_id)
	 || !net_read_u8(packet, &dummy))
		return false;
	wdb_name.name = mem_strdup(MEM_LIBWOW, name);
	if (!wdb_name.name)
	{
		LOG_ERROR("malloc failed");
		wow_wdb_name_destroy(&wdb_name);
		return false;
	}
	wdb_set_name(g_wow->wdb, &wdb_name);
	return true;
}

bool net_cmsg_creature_query(struct net_packet_writer *packet, uint32_t id, uint64_t guid)
{
	net_packet_writer_init(packet, CMSG_CREATURE_QUERY);
	if (!net_write_u32(packet, id)
	 || !net_write_u64(packet, guid))
		return false;
	return true;
}

bool net_smsg_creature_query_response(struct net_packet_reader *packet)
{
	struct wow_wdb_creature creature;
	const char *name[4];
	const char *subname;
	const char *icon;
	if (!net_read_u32(packet, &creature.id)
	 || !net_read_str(packet, &name[0])
	 || !net_read_str(packet, &name[1])
	 || !net_read_str(packet, &name[2])
	 || !net_read_str(packet, &name[3])
	 || !net_read_str(packet, &subname)
	 || !net_read_str(packet, &icon)
	 || !net_read_u32(packet, &creature.type_flags)
	 || !net_read_u32(packet, &creature.type)
	 || !net_read_u32(packet, &creature.family)
	 || !net_read_u32(packet, &creature.rank)
	 || !net_read_u32(packet, &creature.unk)
	 || !net_read_u32(packet, &creature.pet_spell_data_id)
	 || !net_read_u32(packet, &creature.model_id[0])
	 || !net_read_u32(packet, &creature.model_id[1])
	 || !net_read_u32(packet, &creature.model_id[2])
	 || !net_read_u32(packet, &creature.model_id[3])
	 || !net_read_flt(packet, &creature.health_multiplier)
	 || !net_read_flt(packet, &creature.power_multiplier)
	 || !net_read_u8(packet, &creature.racial_leader))
		return false;
	creature.name[0] = mem_strdup(MEM_LIBWOW, name[0]);
	creature.name[1] = mem_strdup(MEM_LIBWOW, name[1]);
	creature.name[2] = mem_strdup(MEM_LIBWOW, name[2]);
	creature.name[3] = mem_strdup(MEM_LIBWOW, name[3]);
	creature.subname = mem_strdup(MEM_LIBWOW, subname);
	creature.icon = mem_strdup(MEM_LIBWOW, icon);
	if (!creature.name[0]
	 || !creature.name[1]
	 || !creature.name[2]
	 || !creature.name[3]
	 || !creature.subname
	 || !creature.icon)
	{
		LOG_ERROR("allocation failed");
		wow_wdb_creature_destroy(&creature);
		return false;
	}
	wdb_set_creature(g_wow->wdb, &creature);
	return true;
}

bool net_cmsg_item_query_single(struct net_packet_writer *packet, uint32_t id)
{
	net_packet_writer_init(packet, CMSG_ITEM_QUERY_SINGLE);
	if (!net_write_u32(packet, id))
		return false;
	return true;
}

bool net_smsg_item_query_single_response(struct net_packet_reader *packet)
{
	struct wow_wdb_item item;
	const char *name[4];
	const char *description;
	if (!net_read_u32(packet, &item.id)
	 || !net_read_u32(packet, &item.type)
	 || !net_read_u32(packet, &item.subtype)
	 || !net_read_u32(packet, &item.unk)
	 || !net_read_str(packet, &name[0])
	 || !net_read_str(packet, &name[1])
	 || !net_read_str(packet, &name[2])
	 || !net_read_str(packet, &name[3])
	 || !net_read_u32(packet, &item.display_id)
	 || !net_read_u32(packet, &item.quality)
	 || !net_read_u32(packet, &item.flags)
	 || !net_read_u32(packet, &item.buy_price)
	 || !net_read_u32(packet, &item.sell_price)
	 || !net_read_u32(packet, &item.inventory_slot)
	 || !net_read_u32(packet, &item.class_mask)
	 || !net_read_u32(packet, &item.race_mask)
	 || !net_read_u32(packet, &item.level)
	 || !net_read_u32(packet, &item.required_level)
	 || !net_read_u32(packet, &item.required_skill)
	 || !net_read_u32(packet, &item.required_skill_rank)
	 || !net_read_u32(packet, &item.required_spell)
	 || !net_read_u32(packet, &item.required_honor_rank)
	 || !net_read_u32(packet, &item.required_city_rank)
	 || !net_read_u32(packet, &item.required_reputation_faction)
	 || !net_read_u32(packet, &item.required_reputation_rank)
	 || !net_read_u32(packet, &item.unique_count)
	 || !net_read_u32(packet, &item.stack_count)
	 || !net_read_u32(packet, &item.container_slots))
		return false;
	for (size_t i = 0; i < 10; ++i)
	{
		if (!net_read_u32(packet, &item.attributes[i].id)
		 || !net_read_i32(packet, &item.attributes[i].value))
			return false;
	}
	for (size_t i = 0; i < 5; ++i)
	{
		if (!net_read_flt(packet, &item.damages[i].min)
		 || !net_read_flt(packet, &item.damages[i].max)
		 || !net_read_u32(packet, &item.damages[i].type))
			return false;
	}
	for (size_t i = 0; i < 7; ++i)
	{
		if (!net_read_u32(packet, &item.resistances[i]))
			return false;
	}
	if (!net_read_u32(packet, &item.weapon_speed)
	 || !net_read_u32(packet, &item.ammo_type)
	 || !net_read_flt(packet, &item.ranged_mod_range))
		return false;
	for (size_t i = 0; i < 5; ++i)
	{
		if (!net_read_u32(packet, &item.spells[i].id)
		 || !net_read_u32(packet, &item.spells[i].trigger)
		 || !net_read_u32(packet, &item.spells[i].charges)
		 || !net_read_u32(packet, &item.spells[i].cooldown)
		 || !net_read_u32(packet, &item.spells[i].category)
		 || !net_read_u32(packet, &item.spells[i].category_cooldown))
			return false;
	}
	if (!net_read_u32(packet, &item.bonding)
	 || !net_read_str(packet, &description)
	 || !net_read_u32(packet, &item.page_text)
	 || !net_read_u32(packet, &item.language)
	 || !net_read_u32(packet, &item.page_material)
	 || !net_read_u32(packet, &item.start_quest)
	 || !net_read_u32(packet, &item.lock)
	 || !net_read_u32(packet, &item.material)
	 || !net_read_u32(packet, &item.sheathe_type)
	 || !net_read_u32(packet, &item.random_property)
	 || !net_read_u32(packet, &item.random_suffix)
	 || !net_read_u32(packet, &item.block)
	 || !net_read_u32(packet, &item.item_set)
	 || !net_read_u32(packet, &item.durability)
	 || !net_read_u32(packet, &item.area)
	 || !net_read_u32(packet, &item.map)
	 || !net_read_u32(packet, &item.bag_family)
	 || !net_read_u32(packet, &item.totem_category))
		return false;
	for (size_t i = 0; i < 3; ++i)
	{
		if (!net_read_u32(packet, &item.sockets[i].color)
		 || !net_read_u32(packet, &item.sockets[i].content))
			return false;
	}
	if (!net_read_u32(packet, &item.socket_bonus)
	 || !net_read_u32(packet, &item.gem_property)
	 || !net_read_u32(packet, &item.required_disenchant_skill)
	 || !net_read_flt(packet, &item.armor_damage_modifier)
	 || !net_read_u32(packet, &item.duration))
		return false;
	item.name[0] = mem_strdup(MEM_LIBWOW, name[0]);
	item.name[1] = mem_strdup(MEM_LIBWOW, name[1]);
	item.name[2] = mem_strdup(MEM_LIBWOW, name[2]);
	item.name[3] = mem_strdup(MEM_LIBWOW, name[3]);
	item.description = mem_strdup(MEM_LIBWOW, description);
	if (!item.name[0]
	 || !item.name[1]
	 || !item.name[2]
	 || !item.name[3]
	 || !item.description)
	{
		LOG_ERROR("allocation failed");
		wow_wdb_item_destroy(&item);
		return false;
	}
	wdb_set_item(g_wow->wdb, &item);
	return true;
}

bool net_cmsg_guild_query(struct net_packet_writer *packet, uint32_t id)
{
	net_packet_writer_init(packet, CMSG_GUILD_QUERY);
	if (!net_write_u32(packet, id))
		return false;
	return true;
}

bool net_smsg_guild_query_response(struct net_packet_reader *packet)
{
	struct wow_wdb_guild guild;
	const char *name;
	const char *ranks[10];
	if (!net_read_u32(packet, &guild.id)
	 || !net_read_str(packet, &name))
		return false;
	for (size_t i = 0; i < sizeof(ranks) / sizeof(*ranks); ++i)
	{
		if (!net_read_str(packet, &ranks[i]))
			return false;
	}
	if (!net_read_u32(packet, &guild.emblem_style)
	 || !net_read_u32(packet, &guild.emblem_color)
	 || !net_read_u32(packet, &guild.border_style)
	 || !net_read_u32(packet, &guild.border_color)
	 || !net_read_u32(packet, &guild.background_color))
		return false;
	guild.name = mem_strdup(MEM_LIBWOW, name);
	for (size_t i = 0; i < sizeof(ranks) / sizeof(*ranks); ++i)
		guild.ranks[i] = mem_strdup(MEM_LIBWOW, ranks[i]);
	if (!guild.name
	 || !guild.ranks[0]
	 || !guild.ranks[1]
	 || !guild.ranks[2]
	 || !guild.ranks[3]
	 || !guild.ranks[4]
	 || !guild.ranks[5]
	 || !guild.ranks[6]
	 || !guild.ranks[7]
	 || !guild.ranks[8]
	 || !guild.ranks[9])
	{
		LOG_ERROR("allocation failed");
		wow_wdb_guild_destroy(&guild);
		return false;
	}
	wdb_set_guild(g_wow->wdb, &guild);
	return true;
}

bool net_cmsg_gameobject_query(struct net_packet_writer *packet, uint32_t id, uint64_t guid)
{
	net_packet_writer_init(packet, CMSG_GAMEOBJECT_QUERY);
	if (!net_write_u32(packet, id)
	 || !net_write_u64(packet, guid))
		return false;
	return true;
}

bool net_smsg_gameobject_query_response(struct net_packet_reader *packet)
{
	struct wow_wdb_gameobject gameobject;
	const char *name[4];
	const char *icon;
	const char *cast_bar_caption;
	const char *unk;
	if (!net_read_u32(packet, &gameobject.id)
	 || !net_read_u32(packet, &gameobject.type)
	 || !net_read_u32(packet, &gameobject.display_id)
	 || !net_read_str(packet, &name[0])
	 || !net_read_str(packet, &name[1])
	 || !net_read_str(packet, &name[2])
	 || !net_read_str(packet, &name[3])
	 || !net_read_str(packet, &icon)
	 || !net_read_str(packet, &cast_bar_caption)
	 || !net_read_str(packet, &unk)
	 || !net_read_u32(packet, &gameobject.data[0])
	 || !net_read_u32(packet, &gameobject.data[1])
	 || !net_read_u32(packet, &gameobject.data[2])
	 || !net_read_u32(packet, &gameobject.data[3])
	 || !net_read_u32(packet, &gameobject.data[4])
	 || !net_read_u32(packet, &gameobject.data[5])
	 || !net_read_u32(packet, &gameobject.data[6])
	 || !net_read_u32(packet, &gameobject.data[7])
	 || !net_read_u32(packet, &gameobject.data[8])
	 || !net_read_u32(packet, &gameobject.data[9])
	 || !net_read_u32(packet, &gameobject.data[10])
	 || !net_read_u32(packet, &gameobject.data[11])
	 || !net_read_u32(packet, &gameobject.data[12])
	 || !net_read_u32(packet, &gameobject.data[13])
	 || !net_read_u32(packet, &gameobject.data[14])
	 || !net_read_u32(packet, &gameobject.data[15])
	 || !net_read_u32(packet, &gameobject.data[16])
	 || !net_read_u32(packet, &gameobject.data[17])
	 || !net_read_u32(packet, &gameobject.data[18])
	 || !net_read_u32(packet, &gameobject.data[19])
	 || !net_read_u32(packet, &gameobject.data[20])
	 || !net_read_u32(packet, &gameobject.data[21])
	 || !net_read_u32(packet, &gameobject.data[22])
	 || !net_read_u32(packet, &gameobject.data[23])
	 || !net_read_flt(packet, &gameobject.scale))
		return false;
	gameobject.name[0] = mem_strdup(MEM_LIBWOW, name[0]);
	gameobject.name[1] = mem_strdup(MEM_LIBWOW, name[1]);
	gameobject.name[2] = mem_strdup(MEM_LIBWOW, name[2]);
	gameobject.name[3] = mem_strdup(MEM_LIBWOW, name[3]);
	gameobject.icon = mem_strdup(MEM_LIBWOW, icon);
	gameobject.cast_bar_caption = mem_strdup(MEM_LIBWOW, cast_bar_caption);
	gameobject.unk = mem_strdup(MEM_LIBWOW, unk);
	if (!gameobject.name[0]
	 || !gameobject.name[1]
	 || !gameobject.name[2]
	 || !gameobject.name[3]
	 || !gameobject.icon
	 || !gameobject.cast_bar_caption
	 || !gameobject.unk)
	{
		LOG_ERROR("allocation failed");
		wow_wdb_gameobject_destroy(&gameobject);
		return false;
	}
	wdb_set_gameobject(g_wow->wdb, &gameobject);
	return true;
}
