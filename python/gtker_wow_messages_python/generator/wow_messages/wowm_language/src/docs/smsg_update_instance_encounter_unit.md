# SMSG_UPDATE_INSTANCE_ENCOUNTER_UNIT

## Client Version 3.3.5

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/raid/smsg_update_instance_encounter_unit.wowm:15`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/raid/smsg_update_instance_encounter_unit.wowm#L15).
```rust,ignore
smsg SMSG_UPDATE_INSTANCE_ENCOUNTER_UNIT = 0x0214 {
    EncounterFrame frame;
    if (frame == ENGAGE
        || frame == DISENGAGE
        || frame == UPDATE_PRIORITY) {
        PackedGuid guid;
        u8 parameter1;
    }
    else if (frame == ADD_TIMER
        || frame == ENABLE_OBJECTIVE
        || frame == DISABLE_OBJECTIVE) {
        u8 parameter2;
    }
    else if (frame == UPDATE_OBJECTIVE) {
        u8 parameter3;
        u8 parameter4;
    }
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
| - | 4 / - | [EncounterFrame](encounterframe.md) | frame |  |

If frame is equal to `ENGAGE` **or** 
is equal to `DISENGAGE` **or** 
is equal to `UPDATE_PRIORITY`:

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| - | - / - | [PackedGuid](../types/packed-guid.md) | guid |  |
| - | 1 / - | u8 | parameter1 |  |

Else If frame is equal to `ADD_TIMER` **or** 
is equal to `ENABLE_OBJECTIVE` **or** 
is equal to `DISABLE_OBJECTIVE`:

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| - | 1 / - | u8 | parameter2 |  |

Else If frame is equal to `UPDATE_OBJECTIVE`:

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| - | 1 / - | u8 | parameter3 |  |
| - | 1 / - | u8 | parameter4 |  |
