# SMSG_ARENA_TEAM_QUERY_RESPONSE

## Client Version 2.4.3, Client Version 3

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/queries/smsg_arena_team_query_response.wowm:1`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/queries/smsg_arena_team_query_response.wowm#L1).
```rust,ignore
smsg SMSG_ARENA_TEAM_QUERY_RESPONSE = 0x034C {
    u32 arena_team;
    CString team_name;
    ArenaType team_type;
    u32 background_color;
    u32 emblem_style;
    u32 emblem_color;
    u32 border_style;
    u32 border_color;
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
| - | 4 / Little | u32 | arena_team |  |
| - | - / - | CString | team_name |  |
| - | 1 / - | [ArenaType](arenatype.md) | team_type |  |
| - | 4 / Little | u32 | background_color |  |
| - | 4 / Little | u32 | emblem_style |  |
| - | 4 / Little | u32 | emblem_color |  |
| - | 4 / Little | u32 | border_style |  |
| - | 4 / Little | u32 | border_color |  |
