# CMSG_REPAIR_ITEM

## Client Version 1

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/item/cmsg_repair_item.wowm:1`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/item/cmsg_repair_item.wowm#L1).
```rust,ignore
cmsg CMSG_REPAIR_ITEM = 0x02A8 {
    Guid npc;
    Guid item;
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
| 0x06 | 8 / Little | [Guid](../types/packed-guid.md) | npc |  |
| 0x0E | 8 / Little | [Guid](../types/packed-guid.md) | item |  |

## Client Version 2.3.2, Client Version 2.3.3, Client Version 2.3.4, Client Version 2.4, Client Version 3

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/item/cmsg_repair_item.wowm:8`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/item/cmsg_repair_item.wowm#L8).
```rust,ignore
cmsg CMSG_REPAIR_ITEM = 0x02A8 {
    Guid npc;
    Guid item;
    Bool from_guild_bank;
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
| 0x06 | 8 / Little | [Guid](../types/packed-guid.md) | npc |  |
| 0x0E | 8 / Little | [Guid](../types/packed-guid.md) | item |  |
| 0x16 | 1 / - | Bool | from_guild_bank |  |
