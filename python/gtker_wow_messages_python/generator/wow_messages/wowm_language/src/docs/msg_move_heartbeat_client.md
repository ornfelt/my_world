# MSG_MOVE_HEARTBEAT_Client

## Client Version 1.12

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/movement/msg/msg_move_heartbeat.wowm:1`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/movement/msg/msg_move_heartbeat.wowm#L1).
```rust,ignore
cmsg MSG_MOVE_HEARTBEAT_Client = 0x00EE {
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
238, 0, 0, 0, // opcode (238)
1, 0, 0, 0, // MovementInfo.flags: MovementFlags  FORWARD (1)
70, 49, 122, 1, // MovementInfo.timestamp: u32
25, 199, 11, 198, // Vector3d.x: f32
254, 110, 224, 194, // Vector3d.y: f32
26, 245, 165, 66, // Vector3d.z: f32
3, 81, 36, 64, // MovementInfo.orientation: f32
133, 3, 0, 0, // MovementInfo.fall_time: f32
```
## Client Version 2.4.3

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/movement/msg/msg_move_heartbeat.wowm:33`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/movement/msg/msg_move_heartbeat.wowm#L33).
```rust,ignore
cmsg MSG_MOVE_HEARTBEAT_Client = 0x00EE {
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
