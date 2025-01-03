# SMSG_SERVER_MESSAGE

## Client Version 1.12, Client Version 2

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/chat/smsg_server_message.wowm:11`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/chat/smsg_server_message.wowm#L11).
```rust,ignore
smsg SMSG_SERVER_MESSAGE = 0x0291 {
    ServerMessageType message_type;
    CString message;
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
| 0x04 | 4 / - | [ServerMessageType](servermessagetype.md) | message_type |  |
| 0x08 | - / - | CString | message |  |

## Client Version 3.3.5

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/chat/smsg_server_message.wowm:32`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/chat/smsg_server_message.wowm#L32).
```rust,ignore
smsg SMSG_SERVER_MESSAGE = 0x0291 {
    ServerMessageType message_type;
    CString message;
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
| - | 4 / - | [ServerMessageType](servermessagetype.md) | message_type |  |
| - | - / - | CString | message |  |

