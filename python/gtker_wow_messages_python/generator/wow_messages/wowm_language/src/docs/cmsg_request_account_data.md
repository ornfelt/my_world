# CMSG_REQUEST_ACCOUNT_DATA

## Client Version 1, Client Version 2, Client Version 3

Respond with [SMSG_UPDATE_ACCOUNT_DATA](./smsg_update_account_data.md)

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/login_logout/cmsg_request_account_data.wowm:2`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/login_logout/cmsg_request_account_data.wowm#L2).
```rust,ignore
cmsg CMSG_REQUEST_ACCOUNT_DATA = 0x020A {
    u32 data_type;
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
| 0x06 | 4 / Little | u32 | data_type | The type of account data being requested. You can check this against the [CacheMask](./cachemask.md) to know if this is character-specific data or account-wide data. |

### Examples

#### Example 1

```c
0, 8, // size
10, 2, 0, 0, // opcode (522)
6, 0, 0, 0, // data_type: u32
```