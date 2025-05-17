#include "wdb.h"

#include "itf/interface.h"

#include "net/packets.h"
#include "net/network.h"
#include "net/packet.h"
#include "net/opcode.h"

#include "obj/object.h"

#include "wow_lua.h"
#include "memory.h"
#include "loader.h"
#include "wow.h"
#include "log.h"

#include <wow/wdb.h>

#include <inttypes.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

MEMORY_DECL(GENERIC);

struct wdb_def
{
	const char *name;
	uint32_t magic;
	uint32_t record_size;
	uint32_t record_version;
	int (*read_fn)(struct wdb *wdb, FILE *fp);
	bool (*write_fn)(FILE *fp, struct jks_hmap_iterator *it);
};

static int wmob_read(struct wdb *wdb, FILE *fp);
static bool wmob_write(FILE *fp, struct jks_hmap_iterator *it);

static const struct wdb_def wdb_wmob_def =
{
	"creaturecache",
	WOW_WDB_MAGIC_WMOB,
	0x50,
	0x1,
	wmob_read,
	wmob_write,
};

static int widb_read(struct wdb *wdb, FILE *fp);
static bool widb_write(FILE *fp, struct jks_hmap_iterator *it);

static const struct wdb_def wdb_widb_def =
{
	"itemcache",
	WOW_WDB_MAGIC_WIDB,
	0x210,
	0x5,
	widb_read,
	widb_write,
};

static int wgob_read(struct wdb *wdb, FILE *fp);
static bool wgob_write(FILE *fp, struct jks_hmap_iterator *it);

static const struct wdb_def wdb_wgob_def =
{
	"gameobjectcache",
	WOW_WDB_MAGIC_WGOB,
	0x88,
	0x1,
	wgob_read,
	wgob_write,
};

static void name_dtr(jks_hmap_key_t key, void *data)
{
	(void)key;
	wow_wdb_name_destroy((struct wow_wdb_name*)data);
}

static void creature_dtr(jks_hmap_key_t key, void *data)
{
	(void)key;
	wow_wdb_creature_destroy((struct wow_wdb_creature*)data);
}

static void gameobject_dtr(jks_hmap_key_t key, void *data)
{
	(void)key;
	wow_wdb_gameobject_destroy((struct wow_wdb_gameobject*)data);
}

static void item_dtr(jks_hmap_key_t key, void *data)
{
	(void)key;
	wow_wdb_item_destroy((struct wow_wdb_item*)data);
}

static void guild_dtr(jks_hmap_key_t key, void *data)
{
	(void)key;
	wow_wdb_guild_destroy((struct wow_wdb_guild*)data);
}

struct wdb *wdb_new(void)
{
	struct wdb *wdb = mem_malloc(MEM_GENERIC, sizeof(*wdb));
	if (!wdb)
		return NULL;
	jks_hmap_init(&wdb->names.hmap,
	              sizeof(struct wow_wdb_name),
	              name_dtr,
	              jks_hmap_hash_u64,
	              jks_hmap_cmp_u64,
	              &jks_hmap_memory_fn_GENERIC);
	pthread_mutex_init(&wdb->names.mutex, NULL);
	jks_hmap_init(&wdb->creatures.hmap,
	              sizeof(struct wow_wdb_creature),
	              creature_dtr,
	              jks_hmap_hash_u32,
	              jks_hmap_cmp_u32,
	              &jks_hmap_memory_fn_GENERIC);
	pthread_mutex_init(&wdb->creatures.mutex, NULL);
	jks_hmap_init(&wdb->gameobjects.hmap,
	              sizeof(struct wow_wdb_gameobject),
	              gameobject_dtr,
	              jks_hmap_hash_u32,
	              jks_hmap_cmp_u32,
	              &jks_hmap_memory_fn_GENERIC);
	pthread_mutex_init(&wdb->gameobjects.mutex, NULL);
	jks_hmap_init(&wdb->items.hmap,
	              sizeof(struct wow_wdb_item),
	              item_dtr,
	              jks_hmap_hash_u32,
	              jks_hmap_cmp_u32,
	              &jks_hmap_memory_fn_GENERIC);
	pthread_mutex_init(&wdb->items.mutex, NULL);
	jks_hmap_init(&wdb->guilds.hmap,
	              sizeof(struct wow_wdb_guild),
	              guild_dtr,
	              jks_hmap_hash_u32,
	              jks_hmap_cmp_u32,
	              &jks_hmap_memory_fn_GENERIC);
	pthread_mutex_init(&wdb->guilds.mutex, NULL);
	return wdb;
}

static void list_free(struct wdb_list *list)
{
	jks_hmap_destroy(&list->hmap);
	pthread_mutex_destroy(&list->mutex);
}

void wdb_free(struct wdb *wdb)
{
	if (!wdb)
		return;
	list_free(&wdb->names);
	list_free(&wdb->creatures);
	list_free(&wdb->gameobjects);
	list_free(&wdb->items);
	list_free(&wdb->guilds);
	mem_free(MEM_GENERIC, wdb);
}

static bool wdb_verify_header(const struct wow_wdb_header *header,
                              const struct wdb_def *def)
{
	if (header->id != def->magic)
	{
		LOG_ERROR("invalid wdb magic: %.*s", 4, (char*)&header->id);
		return false;
	}
	if (header->version != 8606)
	{
		LOG_ERROR("invalid wdb version: %" PRIu32, header->version);
		return false;
	}
	char locale[5];
	locale[0] = header->locale >> 24;
	locale[1] = header->locale >> 16;
	locale[2] = header->locale >> 8;
	locale[3] = header->locale >> 0;
	locale[4] = '\0';
	if (strcmp(locale, g_wow->locale))
	{
		LOG_ERROR("invalid locale: %" PRIx32, header->locale);
		return false;
	}
	if (header->record_size != def->record_size)
	{
		LOG_ERROR("invalid record size: %" PRIu32, header->record_size);
		return false;
	}
	if (header->record_version != def->record_version)
	{
		LOG_ERROR("invalid record version: %" PRIu32,
		          header->record_version);
		return false;
	}
	return true;
}

static bool wdb_read(struct wdb *wdb, const struct wdb_def *def)
{
	bool ret = false;
	char path[512];
	snprintf(path, sizeof(path), "%s/Cache/WDB/%s/%s.wdb",
	         g_wow->game_path, g_wow->locale, def->name);
	FILE *fp = fopen(path, "rb");
	if (!fp)
	{
		LOG_WARN("failed to open %s", path);
		return NULL;
	}
	struct wow_wdb_header header;
	if (fread(&header, 1, sizeof(header), fp) != sizeof(header))
	{
		LOG_ERROR("failed to read %s header", def->name);
		goto end;
	}
	if (!wdb_verify_header(&header, def))
	{
		LOG_ERROR("invalid %s header", def->name);
		goto end;
	}
	while (!feof(fp))
	{
		int res = def->read_fn(wdb, fp);
		if (res < 0)
		{
			LOG_ERROR("failed to load entry from %s", def->name);
			goto end;
		}
		if (!res)
			break;
	}
	ret = true;

end:
	if (fp)
		fclose(fp);
	return ret;
}

static bool wdb_write(struct wdb_list *entries, const struct wdb_def *def)
{
	if (!entries->dirty)
		return true;
	bool ret = false;
	char tmp_path[512];
	char path[512];
	snprintf(tmp_path, sizeof(tmp_path), "%s/Cache/WDB/%s/%s.wdb.tmp",
	         g_wow->game_path, g_wow->locale, def->name);
	snprintf(path, sizeof(path), "%s/Cache/WDB/%s/%s.wdb",
	         g_wow->game_path, g_wow->locale, def->name);
	FILE *fp = fopen(tmp_path, "wb");
	if (!fp)
	{
		LOG_WARN("failed to open %s", tmp_path);
		return true;
	}
	struct wow_wdb_header header;
	header.id = def->magic;
	header.version = 8606;
	header.locale = (g_wow->locale[0] << 24)
	              | (g_wow->locale[1] << 16)
	              | (g_wow->locale[2] << 8)
	              | (g_wow->locale[3] << 0);
	header.record_size = def->record_size;
	header.record_version = def->record_version;
	if (fwrite(&header, 1, sizeof(header), fp) != sizeof(header))
	{
		LOG_ERROR("failed to write %s header", def->name);
		goto end;
	}
	pthread_mutex_lock(&entries->mutex); /* XXX that mutex is a little bit too long */
	entries->dirty = false;
	JKS_HMAP_FOREACH(it, &entries->hmap)
	{
		if (!def->write_fn(fp, &it))
		{
			LOG_ERROR("failed to write %s entry", def->name);
			pthread_mutex_unlock(&entries->mutex);
			goto end;
		}
	}
	pthread_mutex_unlock(&entries->mutex);
	static const uint32_t zero = 0;
	if (fwrite(&zero, 1, sizeof(zero), fp) != sizeof(zero)
	 || fwrite(&zero, 1, sizeof(zero), fp) != sizeof(zero))
	{
		LOG_ERROR("failed to write %s end mark", def->name);
		goto end;
	}
	fclose(fp);
	fp = NULL;
	if (rename(tmp_path, path) == -1)
	{
		LOG_ERROR("failed to rename %s", def->name);
		goto end;
	}
	LOG_DEBUG("%s written", def->name);
	ret = true;

end:
	if (fp)
		fclose(fp);
	return ret;
}

void wdb_load(struct wdb *wdb)
{
	if (!wdb_read(wdb, &wdb_wmob_def))
		LOG_WARN("failed to read WMOB");
	wdb->creatures.dirty = false;
	if (!wdb_read(wdb, &wdb_widb_def))
		LOG_WARN("failed to read WIDB");
	wdb->items.dirty = false;
	if (!wdb_read(wdb, &wdb_wgob_def))
		LOG_WARN("failed to read WGOB");
	wdb->gameobjects.dirty = false;
}

static void wdb_save_task(struct wow_mpq_compound *mpq_compound, void *userdata)
{
	(void)mpq_compound;
	struct wdb *wdb = userdata;
	if (!wdb_write(&wdb->creatures, &wdb_wmob_def))
		LOG_WARN("failed to save WMOB");
	if (!wdb_write(&wdb->items, &wdb_widb_def))
		LOG_WARN("failed to save WIDB");
	if (!wdb_write(&wdb->gameobjects, &wdb_wgob_def))
		LOG_WARN("failed to save WGOB");
}

void wdb_save(struct wdb *wdb)
{
	loader_push(g_wow->loader, ASYNC_TASK_WDB_SAVE, wdb_save_task, wdb);
}

static void list_set_entry(struct wdb_list *list, jks_hmap_key_t key, void *data)
{
	pthread_mutex_lock(&list->mutex);
	void *ret = jks_hmap_set(&list->hmap, key, data);
	(void)ret; /* XXX return error? */
	assert(ret);
	list->dirty = true;
	pthread_mutex_unlock(&list->mutex);
}

static bool list_get_entry(struct wdb_list *list, jks_hmap_key_t key,
                            void **datap, size_t new_size)
{
	pthread_mutex_lock(&list->mutex);
	void *data = jks_hmap_get(&list->hmap, key);
	if (data)
	{
		pthread_mutex_unlock(&list->mutex);
		*datap = data;
		return true;
	}
	data = jks_hmap_set(&list->hmap, key, NULL);
	assert(data);
	memset(data, 0, new_size);
	*datap = data;
	pthread_mutex_unlock(&list->mutex);
	return false;
}

void wdb_set_name(struct wdb *wdb, struct wow_wdb_name *name)
{
	list_set_entry(&wdb->names, JKS_HMAP_KEY_U64(name->guid), name);
	const char *update = NULL;
	if (g_wow->player && name->guid == object_guid((struct object*)g_wow->player))
		update = "player";
	if (update && g_wow->interface)
	{
		lua_pushnil(g_wow->interface->L);
		lua_pushstring(g_wow->interface->L, update);
		interface_execute_event(g_wow->interface, EVENT_UNIT_NAME_UPDATE, 1);
	}
}

const struct wow_wdb_name *wdb_get_name(struct wdb *wdb, uint64_t id)
{
	const struct wow_wdb_name *name;
	if (list_get_entry(&wdb->names, JKS_HMAP_KEY_U64(id), (void*)&name, sizeof(*name)))
		return name;
	struct net_packet_writer packet;
	if (!net_cmsg_name_query(&packet, id)
	 || !net_send_packet(g_wow->network, &packet))
		LOG_WARN("can't send packet");
	net_packet_writer_destroy(&packet);
	return NULL;
}

static int wmob_read(struct wdb *wdb, FILE *fp)
{
	struct wow_wdb_creature creature;
	int ret = wow_wdb_creature_read(fp, &creature);
	if (ret <= 0)
		return ret;
	wdb_set_creature(wdb, &creature);
	return 1;
}

static bool wmob_write(FILE *fp, struct jks_hmap_iterator *it)
{
	struct wow_wdb_creature *creature = jks_hmap_iterator_get_value(it);
	if (!creature->name[0])
		return true;
	return wow_wdb_creature_write(fp, creature);
}

void wdb_set_creature(struct wdb *wdb, struct wow_wdb_creature *creature)
{
	list_set_entry(&wdb->creatures, JKS_HMAP_KEY_U32(creature->id), creature);
}

const struct wow_wdb_creature *wdb_get_creature(struct wdb *wdb, uint32_t id)
{
	const struct wow_wdb_creature *creature;
	if (list_get_entry(&wdb->creatures, JKS_HMAP_KEY_U32(id), (void*)&creature, sizeof(*creature)))
		return creature;
	struct net_packet_writer packet;
	if (!net_cmsg_creature_query(&packet, id, 0) /* TODO guid */
	 || !net_send_packet(g_wow->network, &packet))
		LOG_WARN("failed to send packet");
	net_packet_writer_destroy(&packet);
	return NULL;
}

static int widb_read(struct wdb *wdb, FILE *fp)
{
	struct wow_wdb_item item;
	int ret = wow_wdb_item_read(fp, &item);
	if (ret <= 0)
		return ret;
	wdb_set_item(wdb, &item);
	return 1;
}

static bool widb_write(FILE *fp, struct jks_hmap_iterator *it)
{
	struct wow_wdb_item *item = jks_hmap_iterator_get_value(it);
	if (!item->name[0])
		return true;
	return wow_wdb_item_write(fp, item);
}

void wdb_set_item(struct wdb *wdb, struct wow_wdb_item *item)
{
	list_set_entry(&wdb->items, JKS_HMAP_KEY_U32(item->id), item);
}

const struct wow_wdb_item *wdb_get_item(struct wdb *wdb, uint32_t id)
{
	const struct wow_wdb_item *item;
	if (list_get_entry(&wdb->items, JKS_HMAP_KEY_U32(id), (void*)&item, sizeof(*item)))
		return item;
	struct net_packet_writer packet;
	if (!net_cmsg_item_query_single(&packet, id)
	 || !net_send_packet(g_wow->network, &packet))
		LOG_WARN("failed to send packet");
	net_packet_writer_destroy(&packet);
	return NULL;
}

void wdb_set_guild(struct wdb *wdb, struct wow_wdb_guild *guild)
{
	list_set_entry(&wdb->guilds, JKS_HMAP_KEY_U32(guild->id), guild);
}

const struct wow_wdb_guild *wdb_get_guild(struct wdb *wdb, uint32_t id)
{
	const struct wow_wdb_guild *guild;
	if (list_get_entry(&wdb->guilds, JKS_HMAP_KEY_U32(id), (void*)&guild, sizeof(*guild)))
		return guild;
	struct net_packet_writer packet;
	if (!net_cmsg_guild_query(&packet, id)
	 || !net_send_packet(g_wow->network, &packet))
		LOG_WARN("failed to send packet");
	net_packet_writer_destroy(&packet);
	return NULL;
}

static int wgob_read(struct wdb *wdb, FILE *fp)
{
	struct wow_wdb_gameobject gameobject;
	int ret = wow_wdb_gameobject_read(fp, &gameobject);
	if (ret <= 0)
		return ret;
	wdb_set_gameobject(wdb, &gameobject);
	return 1;
}

static bool wgob_write(FILE *fp, struct jks_hmap_iterator *it)
{
	struct wow_wdb_gameobject *gameobject = jks_hmap_iterator_get_value(it);
	if (!gameobject->name[0])
		return true;
	return wow_wdb_gameobject_write(fp, gameobject);
}

void wdb_set_gameobject(struct wdb *wdb, struct wow_wdb_gameobject *gameobject)
{
	list_set_entry(&wdb->gameobjects, JKS_HMAP_KEY_U32(gameobject->id), gameobject);
}

const struct wow_wdb_gameobject *wdb_get_gameobject(struct wdb *wdb, uint32_t id)
{
	const struct wow_wdb_gameobject *gameobject;
	if (list_get_entry(&wdb->gameobjects, JKS_HMAP_KEY_U32(id), (void*)&gameobject, sizeof(*gameobject)))
		return gameobject;
	struct net_packet_writer packet;
	if (!net_cmsg_gameobject_query(&packet, id, 0) /* XXX guid */
	 || !net_send_packet(g_wow->network, &packet))
		LOG_WARN("failed to send packet");
	net_packet_writer_destroy(&packet);
	return NULL;
}
