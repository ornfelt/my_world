# CMSG_USE_ITEM

## Client Version 1.12

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/item/cmsg_use_item.wowm:1`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/item/cmsg_use_item.wowm#L1).
```rust,ignore
cmsg CMSG_USE_ITEM = 0x00AB {
    u8 bag_index;
    u8 bag_slot;
    u8 spell_index;
    SpellCastTargets targets;
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
| 0x06 | 1 / - | u8 | bag_index |  |
| 0x07 | 1 / - | u8 | bag_slot |  |
| 0x08 | 1 / - | u8 | spell_index |  |
| 0x09 | - / - | [SpellCastTargets](spellcasttargets.md) | targets |  |

### Examples

#### Example 1

```c
0, 9, // size
171, 0, 0, 0, // opcode (171)
255, // bag_index: u8
24, // bag_slot: u8
0, // spell_index: u8
0, 0, // SpellCastTargets.target_flags: SpellCastTargetFlags  SELF (0)
```
## Client Version 2.4.3

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/item/cmsg_use_item.wowm:10`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/item/cmsg_use_item.wowm#L10).
```rust,ignore
cmsg CMSG_USE_ITEM = 0x00AB {
    u8 bag_index;
    u8 bag_slot;
    u8 spell_index;
    u8 cast_count;
    Guid item;
    SpellCastTargets targets;
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
| 0x06 | 1 / - | u8 | bag_index |  |
| 0x07 | 1 / - | u8 | bag_slot |  |
| 0x08 | 1 / - | u8 | spell_index |  |
| 0x09 | 1 / - | u8 | cast_count | mangosone: next cast if exists (single or not) |
| 0x0A | 8 / Little | [Guid](../types/packed-guid.md) | item |  |
| 0x12 | - / - | [SpellCastTargets](spellcasttargets.md) | targets |  |

## Client Version 3.3.5

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/item/cmsg_use_item.wowm:36`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/item/cmsg_use_item.wowm#L36).
```rust,ignore
cmsg CMSG_USE_ITEM = 0x00AB {
    u8 bag_index;
    u8 bag_slot;
    u8 spell_index;
    u8 cast_count;
    Spell spell;
    Guid item;
    u32 glyph_index;
    ClientCastFlags cast_flags;
    if (cast_flags == EXTRA) {
        f32 elevation;
        f32 speed;
        ClientMovementData movement_data;
        if (movement_data == PRESENT) {
            u32 opcode;
            PackedGuid guid;
            MovementInfo info;
        }
    }
    SpellCastTargets targets;
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
| 0x06 | 1 / - | u8 | bag_index |  |
| 0x07 | 1 / - | u8 | bag_slot |  |
| 0x08 | 1 / - | u8 | spell_index |  |
| 0x09 | 1 / - | u8 | cast_count | mangosone: next cast if exists (single or not) |
| 0x0A | 4 / Little | Spell | spell |  |
| 0x0E | 8 / Little | [Guid](../types/packed-guid.md) | item |  |
| 0x16 | 4 / Little | u32 | glyph_index |  |
| 0x1A | 1 / - | [ClientCastFlags](clientcastflags.md) | cast_flags |  |

If cast_flags is equal to `EXTRA`:

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| 0x1B | 4 / Little | f32 | elevation |  |
| 0x1F | 4 / Little | f32 | speed |  |
| 0x23 | 1 / - | [ClientMovementData](clientmovementdata.md) | movement_data |  |

If movement_data is equal to `PRESENT`:

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| 0x24 | 4 / Little | u32 | opcode |  |
| 0x28 | - / - | [PackedGuid](../types/packed-guid.md) | guid |  |
| - | - / - | [MovementInfo](movementinfo.md) | info |  |
| - | - / - | [SpellCastTargets](spellcasttargets.md) | targets |  |
