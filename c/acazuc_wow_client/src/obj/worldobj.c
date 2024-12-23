#include "worldobj.h"

#include "obj/update_fields.h"

#include "net/network.h"
#include "net/packet.h"

#include "gx/frame.h"
#include "gx/m2.h"

#include "map/map.h"

#include "const.h"
#include "wow.h"
#include "log.h"

#include <gfx/window.h>

#include <wow/m2.h>

#include <string.h>
#include <math.h>

#define WORLDOBJ ((struct worldobj*)object)

static bool ctr(struct object *object, uint64_t guid)
{
	if (!object_vtable.ctr(object, guid))
		return false;
	WORLDOBJ->worldobj_vtable = &worldobj_vtable;
	WORLDOBJ->m2 = NULL;
	VEC3_SET(WORLDOBJ->position, 0, 0, 0);
	WORLDOBJ->orientation = 0;
	WORLDOBJ->slope = 0;
	WORLDOBJ->transport_guid = 0;
	memset(&WORLDOBJ->movement_data, 0, sizeof(WORLDOBJ->movement_data));
	VEC3_SET(WORLDOBJ->transport_position, 0, 0, 0);
	WORLDOBJ->transport_orientation = 0;
	WORLDOBJ->run_speed = 7;
	WORLDOBJ->walk_speed = 2.5f;
	WORLDOBJ->walk_back_speed = 4.5f;
	WORLDOBJ->swim_speed = 4.722222f;
	WORLDOBJ->swim_back_speed = 2.5f;
	WORLDOBJ->flight_speed = 7.0f;
	WORLDOBJ->flight_back_speed = 4.5f;
	WORLDOBJ->turn_rate = 3.141594f;
	WORLDOBJ->in_render_list = 0;
	return true;
}

static void dtr(struct object *object)
{
	if (WORLDOBJ->m2)
		gx_m2_instance_free(WORLDOBJ->m2);
	object_vtable.dtr(object);
}

void worldobj_set_m2(struct worldobj *worldobj, const char *file)
{
	if (worldobj->m2)
		gx_m2_instance_free(worldobj->m2);
	worldobj->m2 = gx_m2_instance_new_filename(file);
	if (!worldobj->m2)
	{
		LOG_ERROR("failed to create worldobj m2");
		return;
	}
	gx_m2_instance_flag_set(worldobj->m2, GX_M2_INSTANCE_FLAG_DYN_BATCHES);
	gx_m2_instance_set_sequence(worldobj->m2, ANIM_STAND);
	gx_m2_ask_load(worldobj->m2->parent);
}

static void add_to_render(struct object *object)
{
	if (!WORLDOBJ->m2)
		return;
	gx_m2_instance_add_to_render(WORLDOBJ->m2, g_wow->cull_frame, false, &g_wow->cull_frame->m2_params);
}

void worldobj_add_to_render(struct worldobj *worldobj)
{
	if (__atomic_fetch_or(&worldobj->in_render_list, 1, __ATOMIC_RELAXED))
		return;
	worldobj->worldobj_vtable->add_to_render((struct object*)worldobj);
}

void worldobj_send_move_packet(struct worldobj *worldobj, uint32_t opcode)
{
	if (!g_wow->network->world_socket)
		return;
	struct net_packet_writer packet;
	net_packet_writer_init(&packet, opcode);
	if (worldobj_write_movement_data(worldobj, &packet))
		net_send_packet(g_wow->network, &packet);
	net_packet_writer_destroy(&packet);
}

bool worldobj_write_movement_data(struct worldobj *worldobj, struct net_packet_writer *packet)
{
	struct world_movement_data *data = &worldobj->movement_data;
	if (!net_write_u32(packet, data->flags)
	 || !net_write_u8(packet, data->flags2)
	 || !net_write_u32(packet, data->time)
	 || !net_write_flt(packet, data->x)
	 || !net_write_flt(packet, data->y)
	 || !net_write_flt(packet, data->z)
	 || !net_write_flt(packet, data->w))
		return false;
	if (data->flags & MOVEFLAG_ONTRANSPORT)
	{
		if (!net_write_u64(packet, data->transport_guid)
		 || !net_write_flt(packet, data->transport_x)
		 || !net_write_flt(packet, data->transport_y)
		 || !net_write_flt(packet, data->transport_z)
		 || !net_write_flt(packet, data->transport_w)
		 || !net_write_u32(packet, data->transport_time))
			return false;
	}
	if (data->flags & MOVEFLAG_SWIMMING || data->flags & MOVEFLAG_FLYING)
	{
		if (!net_write_flt(packet, data->pitch))
			return false;
	}
	if (!net_write_u32(packet, data->fall_time))
		return false;
	if (data->flags & MOVEFLAG_FALLING)
	{
		if (!net_write_flt(packet, data->jump_velocity)
		 || !net_write_flt(packet, data->jump_sin_angle)
		 || !net_write_flt(packet, data->jump_cos_angle)
		 || !net_write_flt(packet, data->jump_xy_speed))
			return false;
	}
	if (data->flags & MOVEFLAG_SPLINE_ELEVATION)
	{
		if (!net_write_flt(packet, data->unk1))
			return false;
	}
	return true;
}

bool worldobj_read_movement_data(struct worldobj *worldobj, struct net_packet_reader *packet)
{
	struct world_movement_data *data = &worldobj->movement_data;
	if (!net_read_u32(packet, &data->flags)
	 || !net_read_u8(packet, &data->flags2)
	 || !net_read_u32(packet, &data->time)
	 || !net_read_flt(packet, &data->x)
	 || !net_read_flt(packet, &data->y)
	 || !net_read_flt(packet, &data->z)
	 || !net_read_flt(packet, &data->w))
		return false;
	worldobj_set_position(worldobj, (struct vec3f){data->x, data->z, -data->y});
	worldobj_set_orientation(worldobj, data->w - M_PI / 2);
	if (data->flags & MOVEFLAG_ONTRANSPORT)
	{
		if (!net_read_u64(packet, &data->transport_guid)
		 || !net_read_flt(packet, &data->transport_x)
		 || !net_read_flt(packet, &data->transport_y)
		 || !net_read_flt(packet, &data->transport_z)
		 || !net_read_flt(packet, &data->transport_w)
		 || !net_read_u32(packet, &data->transport_time))
			return false;
	}
	if (data->flags & (MOVEFLAG_SWIMMING | MOVEFLAG_FLYING))
	{
		if (!net_read_flt(packet, &data->pitch))
			return false;
	}
	if (!net_read_u32(packet, &data->fall_time))
		return false;
	if (data->flags & MOVEFLAG_FALLING)
	{
		if (!net_read_flt(packet, &data->jump_velocity)
		 || !net_read_flt(packet, &data->jump_sin_angle)
		 || !net_read_flt(packet, &data->jump_cos_angle)
		 || !net_read_flt(packet, &data->jump_xy_speed))
			return false;
	}
	if (data->flags & MOVEFLAG_SPLINE_ELEVATION)
	{
		if (!net_read_flt(packet, &data->unk1))
			return false;
	}
	return true;
}

void worldobj_set_position(struct worldobj *worldobj, struct vec3f position)
{
	worldobj->position = position;
	worldobj->movement_data.x =  position.x;
	worldobj->movement_data.y = -position.z;
	worldobj->movement_data.z =  position.y;
}

void worldobj_set_orientation(struct worldobj *worldobj, float angle)
{
	worldobj->orientation = fmod(angle, M_PI * 2);
	worldobj->movement_data.w = worldobj->orientation + M_PI / 2;
}

void worldobj_set_slope(struct worldobj *worldobj, float angle)
{
	if (angle > M_PI / 2)
		angle = M_PI / 2;
	else if (angle < -M_PI / 2)
		angle = -M_PI / 2;
	worldobj->slope = angle;
}

float worldobj_get_speed(struct worldobj *worldobj, bool backward)
{
	float speed;
	if (worldobj->movement_data.flags & MOVEFLAG_FLYING)
		speed = backward ? worldobj->flight_back_speed : worldobj->flight_speed;
	else if (worldobj->movement_data.flags & MOVEFLAG_SWIMMING)
		speed = backward ? worldobj->swim_back_speed : worldobj->swim_speed;
	else if (worldobj->movement_data.flags & MOVEFLAG_WALK_MODE)
		speed = backward ? worldobj->walk_back_speed : worldobj->walk_speed;
	else
		speed = backward ? worldobj->walk_back_speed : worldobj->run_speed;
	if (gfx_is_key_down(g_wow->window, GFX_KEY_LCONTROL))
		speed *= 10;
	if (gfx_is_key_down(g_wow->window, GFX_KEY_RCONTROL))
		speed *= 10;
	if (gfx_is_key_down(g_wow->window, GFX_KEY_RSHIFT))
		speed /= 10;
	return speed;
}

static void add_object_point(struct gx_m2_instance *m2, struct vec3f *point, uint16_t idx)
{
	struct vec4f tmp;
	VEC3_CPY(tmp, m2->parent->collision_vertexes[idx]);
	tmp.w = 1;
	struct vec4f out;
	MAT4_VEC4_MUL(out, m2->m, tmp);
	VEC3_CPY(*point, out);
}

static void add_object(struct gx_m2_instance *m2, struct jks_array *triangles)
{
	if (!m2->parent->collision_triangles_nb)
		return;
	struct collision_triangle *tmp = jks_array_grow(triangles, m2->parent->collision_triangles_nb / 3);
	if (!tmp)
	{
		LOG_ERROR("triangles allocation failed");
		return;
	}
	for (size_t i = 0; i < m2->parent->collision_triangles_nb;)
	{
		add_object_point(m2, &tmp->points[0], m2->parent->collision_triangles[i++]);
		add_object_point(m2, &tmp->points[1], m2->parent->collision_triangles[i++]);
		add_object_point(m2, &tmp->points[2], m2->parent->collision_triangles[i++]);
		tmp->touched = false;
		tmp++;
	}
}

void worldobj_collect_collisions_triangles(struct worldobj *worldobj, struct collision_params *params, struct jks_array *triangles)
{
	if (!worldobj->m2)
		return;
	if (!gx_m2_flag_get(worldobj->m2->parent, GX_M2_FLAG_LOADED))
		return;
	if (!aabb_intersect_sphere(&worldobj->m2->caabb, params->center, params->radius)
	 || !aabb_intersect_aabb(&worldobj->m2->caabb, &params->aabb))
		return;
	add_object(worldobj->m2, triangles);
}

static void on_field_changed(struct object *object, uint32_t field)
{
	return object_vtable.on_field_changed(object, field);
}

const struct worldobj_vtable worldobj_vtable =
{
	.add_to_render = add_to_render,
};

const struct object_vtable worldobj_object_vtable =
{
	.ctr = ctr,
	.dtr = dtr,
	.on_field_changed = on_field_changed,
};
