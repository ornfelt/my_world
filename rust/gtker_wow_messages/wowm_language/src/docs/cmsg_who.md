# CMSG_WHO

## Client Version 1, Client Version 2, Client Version 3

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/social/cmsg_who.wowm:3`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/social/cmsg_who.wowm#L3).
```rust,ignore
cmsg CMSG_WHO = 0x0062 {
    Level32 minimum_level;
    Level32 maximum_level;
    CString player_name;
    CString guild_name;
    u32 race_mask;
    u32 class_mask;
    u32 amount_of_zones;
    u32[amount_of_zones] zones;
    u32 amount_of_strings;
    CString[amount_of_strings] search_strings;
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
| 0x06 | 4 / Little | Level32 | minimum_level |  |
| 0x0A | 4 / Little | Level32 | maximum_level |  |
| 0x0E | - / - | CString | player_name |  |
| - | - / - | CString | guild_name |  |
| - | 4 / Little | u32 | race_mask |  |
| - | 4 / Little | u32 | class_mask |  |
| - | 4 / Little | u32 | amount_of_zones |  |
| - | ? / - | u32[amount_of_zones] | zones |  |
| - | 4 / Little | u32 | amount_of_strings |  |
| - | ? / - | CString[amount_of_strings] | search_strings |  |
