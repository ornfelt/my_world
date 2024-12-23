#ifndef WDB_H
#define WDB_H

#include <jks/hmap.h>

#include <pthread.h>
#include <stdint.h>

struct wow_wdb_gameobject;
struct wow_wdb_creature;
struct wow_wdb_guild;
struct wow_wdb_name;
struct wow_wdb_item;

struct wdb_list
{
	struct jks_hmap hmap;
	pthread_mutex_t mutex;
	bool dirty;
};

struct wdb
{
	struct wdb_list names; /* uint64_t, struct wow_wdb_name */
	struct wdb_list creatures; /* uint32_t, struct wow_wdb_creature */
	struct wdb_list gameobjects; /* uint32_t, struct wow_wdb_gameobject */
	struct wdb_list items; /* uint32_t, struct wow_wdb_item */
	struct wdb_list guilds; /* uint32_t, struct wow_wdb_guild */
};

struct wdb *wdb_new(void);
void wdb_free(struct wdb *wdb);
void wdb_load(struct wdb *wdb);
void wdb_save(struct wdb *wdb);

void wdb_set_name(struct wdb *wdb, struct wow_wdb_name *name);
const struct wow_wdb_name *wdb_get_name(struct wdb *wdb, uint64_t id);

void wdb_set_creature(struct wdb *wdb, struct wow_wdb_creature *creature);
const struct wow_wdb_creature *wdb_get_creature(struct wdb *wdb, uint32_t id);

void wdb_set_item(struct wdb *wdb, struct wow_wdb_item *item);
const struct wow_wdb_item *wdb_get_item(struct wdb *wdb, uint32_t id);

void wdb_set_guild(struct wdb *wdb, struct wow_wdb_guild *guild);
const struct wow_wdb_guild *wdb_get_guild(struct wdb *wdb, uint32_t id);

const struct wow_wdb_gameobject *wdb_get_gameobject(struct wdb *wdb, uint32_t id);
void wdb_set_gameobject(struct wdb *wdb, struct wow_wdb_gameobject *gameobject);

#endif
