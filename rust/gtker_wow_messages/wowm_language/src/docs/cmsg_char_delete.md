# CMSG_CHAR_DELETE

## Client Version 1, Client Version 2, Client Version 3

Command to delete a character from the clients account. Can be sent after the client has received [SMSG_CHAR_ENUM](./smsg_char_enum.md).

Sent after the client has confirmed the character deletion.

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/character_screen/cmsg_char_delete.wowm:5`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/character_screen/cmsg_char_delete.wowm#L5).
```rust,ignore
cmsg CMSG_CHAR_DELETE = 0x0038 {
    Guid guid;
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
| 0x06 | 8 / Little | [Guid](../types/packed-guid.md) | guid |  |

### Examples

#### Example 1

```c
0, 12, // size
56, 0, 0, 0, // opcode (56)
239, 190, 173, 222, 0, 0, 0, 0, // guid: Guid
```