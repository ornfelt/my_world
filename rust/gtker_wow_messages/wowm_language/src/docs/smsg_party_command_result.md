# SMSG_PARTY_COMMAND_RESULT

## Client Version 1.12

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/social/smsg_party_command_result.wowm:39`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/social/smsg_party_command_result.wowm#L39).
```rust,ignore
smsg SMSG_PARTY_COMMAND_RESULT = 0x007F {
    (u32)PartyOperation operation;
    CString member;
    (u32)PartyResult result;
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
| 0x04 | 4 / - | [PartyOperation](partyoperation.md) | operation |  |
| 0x08 | - / - | CString | member |  |
| - | 4 / - | [PartyResult](partyresult.md) | result |  |

## Client Version 2.4.3

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/social/smsg_party_command_result.wowm:49`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/social/smsg_party_command_result.wowm#L49).
```rust,ignore
smsg SMSG_PARTY_COMMAND_RESULT = 0x007F {
    (u32)PartyOperation operation;
    CString member;
    (u32)PartyResult result;
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
| 0x04 | 4 / - | [PartyOperation](partyoperation.md) | operation |  |
| 0x08 | - / - | CString | member |  |
| - | 4 / - | [PartyResult](partyresult.md) | result |  |

## Client Version 3

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/social/smsg_party_command_result.wowm:49`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/social/smsg_party_command_result.wowm#L49).
```rust,ignore
smsg SMSG_PARTY_COMMAND_RESULT = 0x007F {
    (u32)PartyOperation operation;
    CString member;
    (u32)PartyResult result;
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
| - | 4 / - | [PartyOperation](partyoperation.md) | operation |  |
| - | - / - | CString | member |  |
| - | 4 / - | [PartyResult](partyresult.md) | result |  |
