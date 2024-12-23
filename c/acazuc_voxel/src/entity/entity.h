#ifndef ENTITY_H
#define ENTITY_H

#include <gfx/objects.h>

#include <jks/vec3.h>
#include <jks/aabb.h>

#include <sys/queue.h>

struct entity;
struct world;
struct chunk;

struct entity_nbt
{
#if 0
	NBTTagString *id;
	NBTTagList *Pos;
	NBTTagList *Motion;
	NBTTagList *Rotation;
	NBTTagFloat *FallDistance;
	NBTTagShort *Fire;
	NBTTagShort *Air;
	NBTTagByte *OnGround;
	NBTTagByte *NoGravity;
	NBTTagInt *Dimension;
	NBTTagByte *Invulnerable;
	NBTTagInt *PortalCooldown;
	NBTTagLong *UUIDMost;
	NBTTagLong *UUIDLeast;
	NBTTagString *CustomName;
	NBTTagByte *CustomNameVisible;
	NBTTagByte *Silent;
	NBTTagList *Passengers;
	NBTTagByte *Glowing;
	NBTTagList *Tags;
	NBTTagCompound *CommandStats;
#endif
};

struct entity_vtable
{
	void (*destroy)(struct entity *entity);
	void (*tick)(struct entity *entity);
	void (*draw)(struct entity *entity);
	void (*update_parent_chunk)(struct entity *entity);
};

struct entity
{
	const struct entity_vtable *vtable;
	struct world *world;
	struct chunk *chunk;
	gfx_buffer_t uniform_buffer;
	struct vec3f sliperness;
	struct vec3f pos_org;
	struct vec3f pos_dst;
	struct vec3f size;
	struct vec3f pos;
	struct vec3f rot;
	struct aabb aabb;
	float gravity;
	bool is_on_floor;
	bool in_water;
	bool deleted;
	bool flying;
	TAILQ_ENTRY(entity) chain;
};

void entity_init(struct entity *entity, struct world *world, struct chunk *chunk);
void entity_delete(struct entity *entity);
void entity_priv_tick(struct entity *entity);
void entity_draw(struct entity *entity);
void entity_priv_draw(struct entity *entity);
void entity_tick(struct entity *entity);
void entity_jump(struct entity *entity);
void entity_move(struct entity *entity, struct vec3f dst);
void entity_set_pos(struct entity *entity, struct vec3f pos);
void entity_set_size(struct entity *entity, struct vec3f size);
struct vec3f entity_get_real_pos(struct entity *entity);
struct mat4f entity_get_mat(struct entity *entity);
void entity_priv_update_parent_chunk(struct entity *entity);

struct entity *pigzombie_new(struct world *world, struct chunk *chunk);
struct entity *skeleton_new(struct world *world, struct chunk *chunk);
struct entity *creeper_new(struct world *world, struct chunk *chunk);
struct entity *zombie_new(struct world *world, struct chunk *chunk);
struct entity *pigman_new(struct world *world, struct chunk *chunk);
struct entity *human_new(struct world *world, struct chunk *chunk);
struct entity *sheep_new(struct world *world, struct chunk *chunk);
struct entity *slime_new(struct world *world, struct chunk *chunk);
struct entity *pig_new(struct world *world, struct chunk *chunk);
struct entity *cow_new(struct world *world, struct chunk *chunk);

#endif
