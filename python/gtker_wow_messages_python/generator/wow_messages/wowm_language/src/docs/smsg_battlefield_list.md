# SMSG_BATTLEFIELD_LIST

## Client Version 1.12

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/battleground/smsg_battlefield_list.wowm:18`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/battleground/smsg_battlefield_list.wowm#L18).
```rust,ignore
smsg SMSG_BATTLEFIELD_LIST = 0x023D {
    Guid battlemaster;
    Map map;
    BattlegroundBracket bracket;
    u32 number_of_battlegrounds;
    u32[number_of_battlegrounds] battlegrounds;
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
| 0x04 | 8 / Little | [Guid](../types/packed-guid.md) | battlemaster |  |
| 0x0C | 4 / - | [Map](map.md) | map |  |
| 0x10 | 1 / - | [BattlegroundBracket](battlegroundbracket.md) | bracket |  |
| 0x11 | 4 / Little | u32 | number_of_battlegrounds |  |
| 0x15 | ? / - | u32[number_of_battlegrounds] | battlegrounds |  |

## Client Version 2.4.3

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/battleground/smsg_battlefield_list.wowm:42`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/battleground/smsg_battlefield_list.wowm#L42).
```rust,ignore
smsg SMSG_BATTLEFIELD_LIST = 0x023D {
    Guid battlemaster;
    BattlegroundType battleground_type;
    u32 number_of_battlegrounds;
    u32[number_of_battlegrounds] battlegrounds;
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
| 0x04 | 8 / Little | [Guid](../types/packed-guid.md) | battlemaster |  |
| 0x0C | 4 / - | [BattlegroundType](battlegroundtype.md) | battleground_type |  |
| 0x10 | 4 / Little | u32 | number_of_battlegrounds |  |
| 0x14 | ? / - | u32[number_of_battlegrounds] | battlegrounds |  |

## Client Version 3.3.5

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/battleground/smsg_battlefield_list.wowm:77`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/battleground/smsg_battlefield_list.wowm#L77).
```rust,ignore
smsg SMSG_BATTLEFIELD_LIST = 0x023D {
    Guid battlemaster;
    BattlegroundType battleground_type;
    u8 unknown1;
    u8 unknown2;
    u8 has_win;
    u32 win_honor;
    u32 win_arena;
    u32 loss_honor;
    RandomBg random;
    if (random == RANDOM) {
        u8 win_random;
        u32 reward_honor;
        u32 reward_arena;
        u32 honor_lost;
    }
    u32 number_of_battlegrounds;
    u32[number_of_battlegrounds] battlegrounds;
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
| - | 8 / Little | [Guid](../types/packed-guid.md) | battlemaster |  |
| - | 4 / - | [BattlegroundType](battlegroundtype.md) | battleground_type |  |
| - | 1 / - | u8 | unknown1 |  |
| - | 1 / - | u8 | unknown2 |  |
| - | 1 / - | u8 | has_win |  |
| - | 4 / Little | u32 | win_honor |  |
| - | 4 / Little | u32 | win_arena |  |
| - | 4 / Little | u32 | loss_honor |  |
| - | 1 / - | [RandomBg](randombg.md) | random |  |

If random is equal to `RANDOM`:

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| - | 1 / - | u8 | win_random |  |
| - | 4 / Little | u32 | reward_honor |  |
| - | 4 / Little | u32 | reward_arena |  |
| - | 4 / Little | u32 | honor_lost |  |
| - | 4 / Little | u32 | number_of_battlegrounds |  |
| - | ? / - | u32[number_of_battlegrounds] | battlegrounds |  |
