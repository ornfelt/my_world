# SMSG_CHAR_RENAME

## Client Version 1

Response to [CMSG_CHAR_RENAME](./cmsg_char_rename.md).

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/character_screen/smsg_char_rename.wowm:2`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/character_screen/smsg_char_rename.wowm#L2).
```rust,ignore
smsg SMSG_CHAR_RENAME = 0x02C8 {
    WorldResult result;
    if (result == RESPONSE_SUCCESS) {
        Guid character;
        CString new_name;
    }
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
| 0x04 | 1 / - | [WorldResult](worldresult.md) | result |  |

If result is equal to `RESPONSE_SUCCESS`:

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| 0x05 | 8 / Little | [Guid](../types/packed-guid.md) | character |  |
| 0x0D | - / - | CString | new_name |  |

### Examples

#### Example 1

```c
0, 3, // size
200, 2, // opcode (712)
71, // result: WorldResult CHAR_NAME_TOO_LONG (0x47)
```
#### Example 2

```c
0, 20, // size
200, 2, // opcode (712)
0, // result: WorldResult RESPONSE_SUCCESS (0x00)
239, 190, 173, 222, 0, 0, 0, 0, // character: Guid
68, 101, 97, 100, 98, 101, 101, 102, 0, // new_name: CString
```
## Client Version 2.4.3

Response to [CMSG_CHAR_RENAME](./cmsg_char_rename.md).

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/character_screen/smsg_char_rename.wowm:2`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/character_screen/smsg_char_rename.wowm#L2).
```rust,ignore
smsg SMSG_CHAR_RENAME = 0x02C8 {
    WorldResult result;
    if (result == RESPONSE_SUCCESS) {
        Guid character;
        CString new_name;
    }
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
| 0x04 | 1 / - | [WorldResult](worldresult.md) | result |  |

If result is equal to `RESPONSE_SUCCESS`:

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| 0x05 | 8 / Little | [Guid](../types/packed-guid.md) | character |  |
| 0x0D | - / - | CString | new_name |  |

## Client Version 3.3.5

Response to [CMSG_CHAR_RENAME](./cmsg_char_rename.md).

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/character_screen/smsg_char_rename.wowm:2`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/character_screen/smsg_char_rename.wowm#L2).
```rust,ignore
smsg SMSG_CHAR_RENAME = 0x02C8 {
    WorldResult result;
    if (result == RESPONSE_SUCCESS) {
        Guid character;
        CString new_name;
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
| - | 1 / - | [WorldResult](worldresult.md) | result |  |

If result is equal to `RESPONSE_SUCCESS`:

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| - | 8 / Little | [Guid](../types/packed-guid.md) | character |  |
| - | - / - | CString | new_name |  |
