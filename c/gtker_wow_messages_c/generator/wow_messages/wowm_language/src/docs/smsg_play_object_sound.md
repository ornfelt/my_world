# SMSG_PLAY_OBJECT_SOUND

## Client Version 1, Client Version 2, Client Version 3

vmangos: Nostalrius: ignored by client if unit is not loaded

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/gameobject/smsg_player_object_sound.wowm:4`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/gameobject/smsg_player_object_sound.wowm#L4).
```rust,ignore
smsg SMSG_PLAY_OBJECT_SOUND = 0x0278 {
    u32 sound_id;
    Guid guid;
}
```
### Header

SMSG have a header of 4 bytes.

#### SMSG Header

| Offset | Size / Endianness | Type   | Name   | Description |
| ------ | ----------------- | ------ | ------ | ----------- |
| 0x00   | 2 / Big           | uint16 | size   | Size of the rest of the message including the opcode field but not including the size field.|
| 0x02   | 2 / Little        | uint16 | opcode | Opcode that determines which fields the message contains.|

### Body

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| 0x04 | 4 / Little | u32 | sound_id |  |
| 0x08 | 8 / Little | [Guid](../types/packed-guid.md) | guid |  |
