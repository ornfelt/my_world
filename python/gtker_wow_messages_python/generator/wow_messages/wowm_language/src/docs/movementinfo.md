# MovementInfo

## Client Version 1.12

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/movement/common_movement.wowm:40`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/movement/common_movement.wowm#L40).
```rust,ignore
struct MovementInfo {
    MovementFlags flags;
    u32 timestamp;
    Vector3d position;
    f32 orientation;
    if (flags & ON_TRANSPORT) {
        TransportInfo transport;
    }
    if (flags & SWIMMING) {
        f32 pitch;
    }
    f32 fall_time;
    if (flags & JUMPING) {
        f32 z_speed;
        f32 cos_angle;
        f32 sin_angle;
        f32 xy_speed;
    }
    if (flags & SPLINE_ELEVATION) {
        f32 spline_elevation;
    }
}
```
### Body

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| 0x00 | 4 / - | [MovementFlags](movementflags.md) | flags |  |
| 0x04 | 4 / Little | u32 | timestamp |  |
| 0x08 | 12 / - | [Vector3d](vector3d.md) | position |  |
| 0x14 | 4 / Little | f32 | orientation |  |

If flags contains `ON_TRANSPORT`:

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| 0x18 | - / - | [TransportInfo](transportinfo.md) | transport |  |

If flags contains `SWIMMING`:

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| - | 4 / Little | f32 | pitch |  |
| - | 4 / Little | f32 | fall_time |  |

If flags contains `JUMPING`:

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| - | 4 / Little | f32 | z_speed |  |
| - | 4 / Little | f32 | cos_angle |  |
| - | 4 / Little | f32 | sin_angle |  |
| - | 4 / Little | f32 | xy_speed |  |

If flags contains `SPLINE_ELEVATION`:

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| - | 4 / Little | f32 | spline_elevation |  |


Used in:
* [CMSG_FORCE_MOVE_ROOT_ACK](cmsg_force_move_root_ack.md)
* [CMSG_FORCE_MOVE_UNROOT_ACK](cmsg_force_move_unroot_ack.md)
* [CMSG_FORCE_RUN_BACK_SPEED_CHANGE_ACK](cmsg_force_run_back_speed_change_ack.md)
* [CMSG_FORCE_RUN_SPEED_CHANGE_ACK](cmsg_force_run_speed_change_ack.md)
* [CMSG_FORCE_SWIM_BACK_SPEED_CHANGE_ACK](cmsg_force_swim_back_speed_change_ack.md)
* [CMSG_FORCE_SWIM_SPEED_CHANGE_ACK](cmsg_force_swim_speed_change_ack.md)
* [CMSG_FORCE_TURN_RATE_CHANGE_ACK](cmsg_force_turn_rate_change_ack.md)
* [CMSG_FORCE_WALK_SPEED_CHANGE_ACK](cmsg_force_walk_speed_change_ack.md)
* [CMSG_MOVE_FALL_RESET](cmsg_move_fall_reset.md)
* [CMSG_MOVE_FEATHER_FALL_ACK](cmsg_move_feather_fall_ack.md)
* [CMSG_MOVE_HOVER_ACK](cmsg_move_hover_ack.md)
* [CMSG_MOVE_KNOCK_BACK_ACK](cmsg_move_knock_back_ack.md)
* [CMSG_MOVE_NOT_ACTIVE_MOVER](cmsg_move_not_active_mover.md)
* [CMSG_MOVE_SPLINE_DONE](cmsg_move_spline_done.md)
* [CMSG_MOVE_WATER_WALK_ACK](cmsg_move_water_walk_ack.md)
* [MSG_MOVE_FALL_LAND_Client](msg_move_fall_land_client.md)
* [MSG_MOVE_FALL_LAND_Server](msg_move_fall_land_server.md)
* [MSG_MOVE_FEATHER_FALL_Server](msg_move_feather_fall_server.md)
* [MSG_MOVE_HEARTBEAT_Client](msg_move_heartbeat_client.md)
* [MSG_MOVE_HEARTBEAT_Server](msg_move_heartbeat_server.md)
* [MSG_MOVE_JUMP_Client](msg_move_jump_client.md)
* [MSG_MOVE_JUMP_Server](msg_move_jump_server.md)
* [MSG_MOVE_SET_FACING_Client](msg_move_set_facing_client.md)
* [MSG_MOVE_SET_FACING_Server](msg_move_set_facing_server.md)
* [MSG_MOVE_SET_PITCH_Client](msg_move_set_pitch_client.md)
* [MSG_MOVE_SET_PITCH_Server](msg_move_set_pitch_server.md)
* [MSG_MOVE_SET_RUN_MODE_Client](msg_move_set_run_mode_client.md)
* [MSG_MOVE_SET_RUN_MODE_Server](msg_move_set_run_mode_server.md)
* [MSG_MOVE_SET_WALK_MODE_Client](msg_move_set_walk_mode_client.md)
* [MSG_MOVE_SET_WALK_MODE_Server](msg_move_set_walk_mode_server.md)
* [MSG_MOVE_START_BACKWARD_Client](msg_move_start_backward_client.md)
* [MSG_MOVE_START_BACKWARD_Server](msg_move_start_backward_server.md)
* [MSG_MOVE_START_FORWARD_Client](msg_move_start_forward_client.md)
* [MSG_MOVE_START_FORWARD_Server](msg_move_start_forward_server.md)
* [MSG_MOVE_START_PITCH_DOWN_Client](msg_move_start_pitch_down_client.md)
* [MSG_MOVE_START_PITCH_DOWN_Server](msg_move_start_pitch_down_server.md)
* [MSG_MOVE_START_PITCH_UP_Client](msg_move_start_pitch_up_client.md)
* [MSG_MOVE_START_PITCH_UP_Server](msg_move_start_pitch_up_server.md)
* [MSG_MOVE_START_STRAFE_LEFT_Client](msg_move_start_strafe_left_client.md)
* [MSG_MOVE_START_STRAFE_LEFT_Server](msg_move_start_strafe_left_server.md)
* [MSG_MOVE_START_STRAFE_RIGHT_Client](msg_move_start_strafe_right_client.md)
* [MSG_MOVE_START_STRAFE_RIGHT_Server](msg_move_start_strafe_right_server.md)
* [MSG_MOVE_START_SWIM_Client](msg_move_start_swim_client.md)
* [MSG_MOVE_START_SWIM_Server](msg_move_start_swim_server.md)
* [MSG_MOVE_START_TURN_LEFT_Client](msg_move_start_turn_left_client.md)
* [MSG_MOVE_START_TURN_LEFT_Server](msg_move_start_turn_left_server.md)
* [MSG_MOVE_START_TURN_RIGHT_Client](msg_move_start_turn_right_client.md)
* [MSG_MOVE_START_TURN_RIGHT_Server](msg_move_start_turn_right_server.md)
* [MSG_MOVE_STOP_Client](msg_move_stop_client.md)
* [MSG_MOVE_STOP_PITCH_Client](msg_move_stop_pitch_client.md)
* [MSG_MOVE_STOP_PITCH_Server](msg_move_stop_pitch_server.md)
* [MSG_MOVE_STOP_STRAFE_Client](msg_move_stop_strafe_client.md)
* [MSG_MOVE_STOP_STRAFE_Server](msg_move_stop_strafe_server.md)
* [MSG_MOVE_STOP_SWIM_Client](msg_move_stop_swim_client.md)
* [MSG_MOVE_STOP_SWIM_Server](msg_move_stop_swim_server.md)
* [MSG_MOVE_STOP_Server](msg_move_stop_server.md)
* [MSG_MOVE_STOP_TURN_Client](msg_move_stop_turn_client.md)
* [MSG_MOVE_STOP_TURN_Server](msg_move_stop_turn_server.md)
* [MSG_MOVE_TELEPORT_ACK_Server](msg_move_teleport_ack_server.md)
* [MSG_MOVE_WATER_WALK](msg_move_water_walk.md)

## Client Version 2.4.3

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/movement/common_movement_2_4_3.wowm:32`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/movement/common_movement_2_4_3.wowm#L32).
```rust,ignore
struct MovementInfo {
    MovementFlags flags;
    u8 extra_flags;
    u32 timestamp;
    Vector3d position;
    f32 orientation;
    if (flags & ON_TRANSPORT) {
        TransportInfo transport;
    }
    if (flags & SWIMMING) {
        f32 pitch1;
    }
    else if (flags & ONTRANSPORT) {
        f32 pitch2;
    }
    f32 fall_time;
    if (flags & JUMPING) {
        f32 z_speed;
        f32 cos_angle;
        f32 sin_angle;
        f32 xy_speed;
    }
    if (flags & SPLINE_ELEVATION) {
        f32 spline_elevation;
    }
}
```
### Body

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| 0x00 | 4 / - | [MovementFlags](movementflags.md) | flags |  |
| 0x04 | 1 / - | u8 | extra_flags |  |
| 0x05 | 4 / Little | u32 | timestamp |  |
| 0x09 | 12 / - | [Vector3d](vector3d.md) | position |  |
| 0x15 | 4 / Little | f32 | orientation |  |

If flags contains `ON_TRANSPORT`:

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| 0x19 | - / - | [TransportInfo](transportinfo.md) | transport |  |

If flags contains `SWIMMING`:

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| - | 4 / Little | f32 | pitch1 |  |

Else If flags contains `ONTRANSPORT`:

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| - | 4 / Little | f32 | pitch2 |  |
| - | 4 / Little | f32 | fall_time |  |

If flags contains `JUMPING`:

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| - | 4 / Little | f32 | z_speed |  |
| - | 4 / Little | f32 | cos_angle |  |
| - | 4 / Little | f32 | sin_angle |  |
| - | 4 / Little | f32 | xy_speed |  |

If flags contains `SPLINE_ELEVATION`:

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| - | 4 / Little | f32 | spline_elevation |  |


Used in:
* [CMSG_FORCE_FLIGHT_BACK_SPEED_CHANGE_ACK](cmsg_force_flight_back_speed_change_ack.md)
* [CMSG_FORCE_FLIGHT_SPEED_CHANGE_ACK](cmsg_force_flight_speed_change_ack.md)
* [CMSG_FORCE_MOVE_ROOT_ACK](cmsg_force_move_root_ack.md)
* [CMSG_FORCE_MOVE_UNROOT_ACK](cmsg_force_move_unroot_ack.md)
* [CMSG_FORCE_RUN_BACK_SPEED_CHANGE_ACK](cmsg_force_run_back_speed_change_ack.md)
* [CMSG_FORCE_RUN_SPEED_CHANGE_ACK](cmsg_force_run_speed_change_ack.md)
* [CMSG_FORCE_SWIM_BACK_SPEED_CHANGE_ACK](cmsg_force_swim_back_speed_change_ack.md)
* [CMSG_FORCE_SWIM_SPEED_CHANGE_ACK](cmsg_force_swim_speed_change_ack.md)
* [CMSG_FORCE_TURN_RATE_CHANGE_ACK](cmsg_force_turn_rate_change_ack.md)
* [CMSG_FORCE_WALK_SPEED_CHANGE_ACK](cmsg_force_walk_speed_change_ack.md)
* [CMSG_MOVE_CHNG_TRANSPORT](cmsg_move_chng_transport.md)
* [CMSG_MOVE_FALL_RESET](cmsg_move_fall_reset.md)
* [CMSG_MOVE_FEATHER_FALL_ACK](cmsg_move_feather_fall_ack.md)
* [CMSG_MOVE_HOVER_ACK](cmsg_move_hover_ack.md)
* [CMSG_MOVE_KNOCK_BACK_ACK](cmsg_move_knock_back_ack.md)
* [CMSG_MOVE_NOT_ACTIVE_MOVER](cmsg_move_not_active_mover.md)
* [CMSG_MOVE_SET_CAN_FLY_ACK](cmsg_move_set_can_fly_ack.md)
* [CMSG_MOVE_SET_FLY](cmsg_move_set_fly.md)
* [CMSG_MOVE_SPLINE_DONE](cmsg_move_spline_done.md)
* [CMSG_MOVE_WATER_WALK_ACK](cmsg_move_water_walk_ack.md)
* [MSG_MOVE_FALL_LAND_Client](msg_move_fall_land_client.md)
* [MSG_MOVE_FALL_LAND_Server](msg_move_fall_land_server.md)
* [MSG_MOVE_FEATHER_FALL_Server](msg_move_feather_fall_server.md)
* [MSG_MOVE_HEARTBEAT_Client](msg_move_heartbeat_client.md)
* [MSG_MOVE_HEARTBEAT_Server](msg_move_heartbeat_server.md)
* [MSG_MOVE_HOVER](msg_move_hover.md)
* [MSG_MOVE_JUMP_Client](msg_move_jump_client.md)
* [MSG_MOVE_JUMP_Server](msg_move_jump_server.md)
* [MSG_MOVE_KNOCK_BACK_Server](msg_move_knock_back_server.md)
* [MSG_MOVE_ROOT_Server](msg_move_root_server.md)
* [MSG_MOVE_SET_FACING_Client](msg_move_set_facing_client.md)
* [MSG_MOVE_SET_FACING_Server](msg_move_set_facing_server.md)
* [MSG_MOVE_SET_FLIGHT_BACK_SPEED](msg_move_set_flight_back_speed.md)
* [MSG_MOVE_SET_FLIGHT_SPEED_Server](msg_move_set_flight_speed_server.md)
* [MSG_MOVE_SET_PITCH_Client](msg_move_set_pitch_client.md)
* [MSG_MOVE_SET_PITCH_Server](msg_move_set_pitch_server.md)
* [MSG_MOVE_SET_RUN_MODE_Client](msg_move_set_run_mode_client.md)
* [MSG_MOVE_SET_RUN_MODE_Server](msg_move_set_run_mode_server.md)
* [MSG_MOVE_SET_WALK_MODE_Client](msg_move_set_walk_mode_client.md)
* [MSG_MOVE_SET_WALK_MODE_Server](msg_move_set_walk_mode_server.md)
* [MSG_MOVE_START_ASCEND_Client](msg_move_start_ascend_client.md)
* [MSG_MOVE_START_ASCEND_Server](msg_move_start_ascend_server.md)
* [MSG_MOVE_START_BACKWARD_Client](msg_move_start_backward_client.md)
* [MSG_MOVE_START_BACKWARD_Server](msg_move_start_backward_server.md)
* [MSG_MOVE_START_DESCEND_Client](msg_move_start_descend_client.md)
* [MSG_MOVE_START_DESCEND_Server](msg_move_start_descend_server.md)
* [MSG_MOVE_START_FORWARD_Client](msg_move_start_forward_client.md)
* [MSG_MOVE_START_FORWARD_Server](msg_move_start_forward_server.md)
* [MSG_MOVE_START_PITCH_DOWN_Client](msg_move_start_pitch_down_client.md)
* [MSG_MOVE_START_PITCH_DOWN_Server](msg_move_start_pitch_down_server.md)
* [MSG_MOVE_START_PITCH_UP_Client](msg_move_start_pitch_up_client.md)
* [MSG_MOVE_START_PITCH_UP_Server](msg_move_start_pitch_up_server.md)
* [MSG_MOVE_START_STRAFE_LEFT_Client](msg_move_start_strafe_left_client.md)
* [MSG_MOVE_START_STRAFE_LEFT_Server](msg_move_start_strafe_left_server.md)
* [MSG_MOVE_START_STRAFE_RIGHT_Client](msg_move_start_strafe_right_client.md)
* [MSG_MOVE_START_STRAFE_RIGHT_Server](msg_move_start_strafe_right_server.md)
* [MSG_MOVE_START_SWIM_Client](msg_move_start_swim_client.md)
* [MSG_MOVE_START_SWIM_Server](msg_move_start_swim_server.md)
* [MSG_MOVE_START_TURN_LEFT_Client](msg_move_start_turn_left_client.md)
* [MSG_MOVE_START_TURN_LEFT_Server](msg_move_start_turn_left_server.md)
* [MSG_MOVE_START_TURN_RIGHT_Client](msg_move_start_turn_right_client.md)
* [MSG_MOVE_START_TURN_RIGHT_Server](msg_move_start_turn_right_server.md)
* [MSG_MOVE_STOP_ASCEND_Client](msg_move_stop_ascend_client.md)
* [MSG_MOVE_STOP_ASCEND_Server](msg_move_stop_ascend_server.md)
* [MSG_MOVE_STOP_Client](msg_move_stop_client.md)
* [MSG_MOVE_STOP_PITCH_Client](msg_move_stop_pitch_client.md)
* [MSG_MOVE_STOP_PITCH_Server](msg_move_stop_pitch_server.md)
* [MSG_MOVE_STOP_STRAFE_Client](msg_move_stop_strafe_client.md)
* [MSG_MOVE_STOP_STRAFE_Server](msg_move_stop_strafe_server.md)
* [MSG_MOVE_STOP_SWIM_Client](msg_move_stop_swim_client.md)
* [MSG_MOVE_STOP_SWIM_Server](msg_move_stop_swim_server.md)
* [MSG_MOVE_STOP_Server](msg_move_stop_server.md)
* [MSG_MOVE_STOP_TURN_Client](msg_move_stop_turn_client.md)
* [MSG_MOVE_STOP_TURN_Server](msg_move_stop_turn_server.md)
* [MSG_MOVE_TELEPORT_ACK_Server](msg_move_teleport_ack_server.md)
* [MSG_MOVE_TELEPORT_Server](msg_move_teleport_server.md)
* [MSG_MOVE_UNROOT_Server](msg_move_unroot_server.md)
* [MSG_MOVE_UPDATE_CAN_FLY_Server](msg_move_update_can_fly_server.md)
* [MSG_MOVE_WATER_WALK](msg_move_water_walk.md)

## Client Version 3.3.5

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/movement/common_movement_3_3_5.wowm:79`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/movement/common_movement_3_3_5.wowm#L79).
```rust,ignore
struct MovementInfo {
    MovementFlags flags;
    u32 timestamp;
    Vector3d position;
    f32 orientation;
    if (flags & ON_TRANSPORT_AND_INTERPOLATED_MOVEMENT) {
        TransportInfo transport_info;
        u32 transport_time;
    }
    else if (flags & ON_TRANSPORT) {
        TransportInfo transport;
    }
    if (flags & SWIMMING) {
        f32 pitch1;
    }
    else if (flags & FLYING) {
        f32 pitch2;
    }
    else if (flags & ALWAYS_ALLOW_PITCHING) {
        f32 pitch3;
    }
    f32 fall_time;
    if (flags & FALLING) {
        f32 z_speed;
        f32 cos_angle;
        f32 sin_angle;
        f32 xy_speed;
    }
    if (flags & SPLINE_ELEVATION) {
        f32 spline_elevation;
    }
}
```
### Body

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| 0x00 | 6 / - | [MovementFlags](movementflags.md) | flags |  |
| 0x06 | 4 / Little | u32 | timestamp |  |
| 0x0A | 12 / - | [Vector3d](vector3d.md) | position |  |
| 0x16 | 4 / Little | f32 | orientation |  |

If flags contains `ON_TRANSPORT_AND_INTERPOLATED_MOVEMENT`:

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| 0x1A | - / - | [TransportInfo](transportinfo.md) | transport_info |  |
| - | 4 / Little | u32 | transport_time |  |

Else If flags contains `ON_TRANSPORT`:

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| - | - / - | [TransportInfo](transportinfo.md) | transport |  |

If flags contains `SWIMMING`:

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| - | 4 / Little | f32 | pitch1 |  |

Else If flags contains `FLYING`:

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| - | 4 / Little | f32 | pitch2 |  |

Else If flags contains `ALWAYS_ALLOW_PITCHING`:

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| - | 4 / Little | f32 | pitch3 |  |
| - | 4 / Little | f32 | fall_time |  |

If flags contains `FALLING`:

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| - | 4 / Little | f32 | z_speed |  |
| - | 4 / Little | f32 | cos_angle |  |
| - | 4 / Little | f32 | sin_angle |  |
| - | 4 / Little | f32 | xy_speed |  |

If flags contains `SPLINE_ELEVATION`:

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| - | 4 / Little | f32 | spline_elevation |  |


Used in:
* [CMSG_CAST_SPELL](cmsg_cast_spell.md)
* [CMSG_CHANGE_SEATS_ON_CONTROLLED_VEHICLE](cmsg_change_seats_on_controlled_vehicle.md)
* [CMSG_FORCE_FLIGHT_BACK_SPEED_CHANGE_ACK](cmsg_force_flight_back_speed_change_ack.md)
* [CMSG_FORCE_FLIGHT_SPEED_CHANGE_ACK](cmsg_force_flight_speed_change_ack.md)
* [CMSG_FORCE_MOVE_ROOT_ACK](cmsg_force_move_root_ack.md)
* [CMSG_FORCE_MOVE_UNROOT_ACK](cmsg_force_move_unroot_ack.md)
* [CMSG_FORCE_RUN_BACK_SPEED_CHANGE_ACK](cmsg_force_run_back_speed_change_ack.md)
* [CMSG_FORCE_RUN_SPEED_CHANGE_ACK](cmsg_force_run_speed_change_ack.md)
* [CMSG_FORCE_SWIM_BACK_SPEED_CHANGE_ACK](cmsg_force_swim_back_speed_change_ack.md)
* [CMSG_FORCE_SWIM_SPEED_CHANGE_ACK](cmsg_force_swim_speed_change_ack.md)
* [CMSG_FORCE_TURN_RATE_CHANGE_ACK](cmsg_force_turn_rate_change_ack.md)
* [CMSG_FORCE_WALK_SPEED_CHANGE_ACK](cmsg_force_walk_speed_change_ack.md)
* [CMSG_MOVE_CHNG_TRANSPORT](cmsg_move_chng_transport.md)
* [CMSG_MOVE_FALL_RESET](cmsg_move_fall_reset.md)
* [CMSG_MOVE_FEATHER_FALL_ACK](cmsg_move_feather_fall_ack.md)
* [CMSG_MOVE_GRAVITY_DISABLE_ACK](cmsg_move_gravity_disable_ack.md)
* [CMSG_MOVE_GRAVITY_ENABLE_ACK](cmsg_move_gravity_enable_ack.md)
* [CMSG_MOVE_HOVER_ACK](cmsg_move_hover_ack.md)
* [CMSG_MOVE_KNOCK_BACK_ACK](cmsg_move_knock_back_ack.md)
* [CMSG_MOVE_NOT_ACTIVE_MOVER](cmsg_move_not_active_mover.md)
* [CMSG_MOVE_SET_CAN_FLY_ACK](cmsg_move_set_can_fly_ack.md)
* [CMSG_MOVE_SET_CAN_TRANSITION_BETWEEN_SWIM_AND_FLY_ACK](cmsg_move_set_can_transition_between_swim_and_fly_ack.md)
* [CMSG_MOVE_SET_COLLISION_HGT_ACK](cmsg_move_set_collision_hgt_ack.md)
* [CMSG_MOVE_SET_FLY](cmsg_move_set_fly.md)
* [CMSG_MOVE_SPLINE_DONE](cmsg_move_spline_done.md)
* [CMSG_MOVE_WATER_WALK_ACK](cmsg_move_water_walk_ack.md)
* [CMSG_PET_CAST_SPELL](cmsg_pet_cast_spell.md)
* [CMSG_USE_ITEM](cmsg_use_item.md)
* [MSG_MOVE_FALL_LAND](msg_move_fall_land.md)
* [MSG_MOVE_FEATHER_FALL_Server](msg_move_feather_fall_server.md)
* [MSG_MOVE_GRAVITY_CHNG_Server](msg_move_gravity_chng_server.md)
* [MSG_MOVE_HEARTBEAT](msg_move_heartbeat.md)
* [MSG_MOVE_HOVER](msg_move_hover.md)
* [MSG_MOVE_JUMP](msg_move_jump.md)
* [MSG_MOVE_KNOCK_BACK_Server](msg_move_knock_back_server.md)
* [MSG_MOVE_ROOT_Server](msg_move_root_server.md)
* [MSG_MOVE_SET_FACING](msg_move_set_facing.md)
* [MSG_MOVE_SET_FLIGHT_BACK_SPEED](msg_move_set_flight_back_speed.md)
* [MSG_MOVE_SET_FLIGHT_SPEED_Server](msg_move_set_flight_speed_server.md)
* [MSG_MOVE_SET_PITCH](msg_move_set_pitch.md)
* [MSG_MOVE_SET_PITCH_RATE_Server](msg_move_set_pitch_rate_server.md)
* [MSG_MOVE_SET_RUN_MODE](msg_move_set_run_mode.md)
* [MSG_MOVE_SET_WALK_MODE](msg_move_set_walk_mode.md)
* [MSG_MOVE_START_ASCEND](msg_move_start_ascend.md)
* [MSG_MOVE_START_BACKWARD](msg_move_start_backward.md)
* [MSG_MOVE_START_DESCEND](msg_move_start_descend.md)
* [MSG_MOVE_START_FORWARD](msg_move_start_forward.md)
* [MSG_MOVE_START_PITCH_DOWN](msg_move_start_pitch_down.md)
* [MSG_MOVE_START_PITCH_UP](msg_move_start_pitch_up.md)
* [MSG_MOVE_START_STRAFE_LEFT](msg_move_start_strafe_left.md)
* [MSG_MOVE_START_STRAFE_RIGHT](msg_move_start_strafe_right.md)
* [MSG_MOVE_START_SWIM](msg_move_start_swim.md)
* [MSG_MOVE_START_TURN_LEFT](msg_move_start_turn_left.md)
* [MSG_MOVE_START_TURN_RIGHT](msg_move_start_turn_right.md)
* [MSG_MOVE_STOP](msg_move_stop.md)
* [MSG_MOVE_STOP_ASCEND](msg_move_stop_ascend.md)
* [MSG_MOVE_STOP_PITCH](msg_move_stop_pitch.md)
* [MSG_MOVE_STOP_STRAFE](msg_move_stop_strafe.md)
* [MSG_MOVE_STOP_SWIM](msg_move_stop_swim.md)
* [MSG_MOVE_STOP_TURN](msg_move_stop_turn.md)
* [MSG_MOVE_TELEPORT_ACK_Server](msg_move_teleport_ack_server.md)
* [MSG_MOVE_TELEPORT_Server](msg_move_teleport_server.md)
* [MSG_MOVE_UNROOT_Server](msg_move_unroot_server.md)
* [MSG_MOVE_UPDATE_CAN_FLY_Server](msg_move_update_can_fly_server.md)
* [MSG_MOVE_WATER_WALK](msg_move_water_walk.md)
