# SMSG_ACHIEVEMENT_EARNED

## Client Version 3.3.5

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/achievement/smsg_achievement_earned.wowm:1`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/achievement/smsg_achievement_earned.wowm#L1).
```rust,ignore
smsg SMSG_ACHIEVEMENT_EARNED = 0x0468 {
    PackedGuid player;
    u32 achievement;
    DateTime earn_time;
    u32 unknown;
}
```
### Header

SMSG have a header of 4 bytes.

#### SMSG Header

| Offset | Size / Endianness | Type   | Name   | Description |
| ------ | ----------------- | ------ | ------ | ----------- |
| 0x00   | 2 **OR** 3 / Big           | uint16 **OR** uint16+uint8 | size | Size of the rest of the message including the opcode field but not including the size field. Wrath server messages **can** be 3 bytes. If the first (most significant) size byte has `0x80` set, the header will be 3 bytes, otherwise it is 2.|
| -      | 2 / Little| uint16 | opcode | Opcode that determines which fields the message contains. |

### Body

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| - | - / - | [PackedGuid](../types/packed-guid.md) | player |  |
| - | 4 / Little | u32 | achievement |  |
| - | 4 / Little | DateTime | earn_time |  |
| - | 4 / Little | u32 | unknown | All emus set to 0. |
