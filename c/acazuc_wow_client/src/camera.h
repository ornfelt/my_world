#ifndef CAMERA_H
#define CAMERA_H

#include <jks/frustum.h>
#include <jks/mat4.h>
#include <jks/vec3.h>

#include <stdbool.h>

#define CAMERA_MOVE_FRONT 0x01
#define CAMERA_MOVE_BACK  0x02
#define CAMERA_MOVE_LEFT  0x04
#define CAMERA_MOVE_RIGHT 0x08
#define CAMERA_TURN_LEFT  0x10
#define CAMERA_TURN_RIGHT 0x20
#define CAMERA_MOVE_UP    0x40
#define CAMERA_MOVE_DOWN  0x80

struct unit;

struct camera
{
	struct frustum shadow_frustum;
	struct frustum wdl_frustum;
	struct frustum frustum;
	struct worldobj *worldobj;
	struct mat4f shadow_vp;
	struct mat4f shadow_p;
	struct mat4f shadow_v;
	struct mat4f wdl_vp;
	struct mat4f wdl_p;
	struct mat4f vp;
	struct mat4f p;
	struct mat4f v;
	struct vec3f pos;
	struct vec3f rot;
	uint64_t last_move_update;
	uint32_t last_move_flags;
	int32_t mouse_x;
	int32_t mouse_y;
	float unit_max_distance;
	float unit_velocity;
	float unit_distance;
	float view_distance;
	float fov;
	bool move_unit;
	bool autorun;
};

struct camera *camera_new(void);
void camera_delete(struct camera *camera);
void camera_handle_keyboard(struct camera *camera);
void camera_handle_mouse(struct camera *camera);
void camera_handle_scroll(struct camera *camera, float scroll);
bool camera_update_matrixes(struct camera *camera);

#endif
