# MSG_MOVE_FALL_LAND_Client

## Client Version 1.12

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/movement/msg/msg_move_fall_land.wowm:1`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/movement/msg/msg_move_fall_land.wowm#L1).
```rust,ignore
cmsg MSG_MOVE_FALL_LAND_Client = 0x00C9 {
    MovementInfo info;
}
```
### Header

CMSG have a header of 6 bytes.

#### CMSG Header

| Offset | Size / Endianness | Type   | Name   | Description |
| ------ | ----------------- | ------ | ------ | ----------- |
| 0x00   | 2 / Big           | uint16 | size   | Size of the rest of the message including the opcode field but not including the size field.|
| 0x02   | 4 / Little        | uint32 | opcode | Opcode that determines which fields the message contains.|

### Body

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| 0x06 | - / - | [MovementInfo](movementinfo.md) | info |  |

### Examples

#### Example 1

```c
0, 32, // size
201, 0, 0, 0, // opcode (201)
0, 0, 0, 0, // MovementInfo.flags: MovementFlags  NONE (0)
165, 217, 121, 1, // MovementInfo.timestamp: u32
173, 149, 11, 198, // Vector3d.x: f32
120, 245, 2, 195, // Vector3d.y: f32
241, 246, 165, 66, // Vector3d.z: f32
75, 71, 175, 61, // MovementInfo.orientation: f32
133, 3, 0, 0, // MovementInfo.fall_time: f32
```
## Client Version 2.4.3

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/movement/msg/msg_move_fall_land.wowm:33`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/movement/msg/msg_move_fall_land.wowm#L33).
```rust,ignore
cmsg MSG_MOVE_FALL_LAND_Client = 0x00C9 {
    MovementInfo info;
}
```
### Header

CMSG have a header of 6 bytes.

#### CMSG Header

| Offset | Size / Endianness | Type   | Name   | Description |
| ------ | ----------------- | ------ | ------ | ----------- |
| 0x00   | 2 / Big           | uint16 | size   | Size of the rest of the message including the opcode field but not including the size field.|
| 0x02   | 4 / Little        | uint32 | opcode | Opcode that determines which fields the message contains.|

### Body

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| 0x06 | - / - | [MovementInfo](movementinfo.md) | info |  |
