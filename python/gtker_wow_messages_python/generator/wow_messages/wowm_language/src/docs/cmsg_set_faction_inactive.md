# CMSG_SET_FACTION_INACTIVE

## Client Version 1.12

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/faction/cmsg_set_faction_inactive.wowm:1`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/faction/cmsg_set_faction_inactive.wowm#L1).
```rust,ignore
cmsg CMSG_SET_FACTION_INACTIVE = 0x0317 {
    Faction faction;
    Bool inactive;
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
| 0x06 | 2 / - | [Faction](faction.md) | faction |  |
| 0x08 | 1 / - | Bool | inactive |  |

## Client Version 2.4.3

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/faction/cmsg_set_faction_inactive.wowm:1`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/faction/cmsg_set_faction_inactive.wowm#L1).
```rust,ignore
cmsg CMSG_SET_FACTION_INACTIVE = 0x0317 {
    Faction faction;
    Bool inactive;
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
| 0x06 | 2 / - | [Faction](faction.md) | faction |  |
| 0x08 | 1 / - | Bool | inactive |  |

## Client Version 3.3.5

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/faction/cmsg_set_faction_inactive.wowm:1`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/faction/cmsg_set_faction_inactive.wowm#L1).
```rust,ignore
cmsg CMSG_SET_FACTION_INACTIVE = 0x0317 {
    Faction faction;
    Bool inactive;
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
| 0x06 | 2 / - | [Faction](faction.md) | faction |  |
| 0x08 | 1 / - | Bool | inactive |  |
