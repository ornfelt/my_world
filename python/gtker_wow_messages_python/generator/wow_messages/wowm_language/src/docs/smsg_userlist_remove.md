# SMSG_USERLIST_REMOVE

## Client Version 2.4.3

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/social/smsg_userlist_remove.wowm:1`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/social/smsg_userlist_remove.wowm#L1).
```rust,ignore
smsg SMSG_USERLIST_REMOVE = 0x03F0 {
    Guid player;
    u8 flags;
    u32 amount_of_players;
    CString name;
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
| 0x04 | 8 / Little | [Guid](../types/packed-guid.md) | player |  |
| 0x0C | 1 / - | u8 | flags |  |
| 0x0D | 4 / Little | u32 | amount_of_players |  |
| 0x11 | - / - | CString | name |  |

## Client Version 3.3.5

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/social/smsg_userlist_remove.wowm:10`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/social/smsg_userlist_remove.wowm#L10).
```rust,ignore
smsg SMSG_USERLIST_REMOVE = 0x03F1 {
    Guid player;
    u8 flags;
    u32 amount_of_players;
    CString name;
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
| - | 8 / Little | [Guid](../types/packed-guid.md) | player |  |
| - | 1 / - | u8 | flags |  |
| - | 4 / Little | u32 | amount_of_players |  |
| - | - / - | CString | name |  |
