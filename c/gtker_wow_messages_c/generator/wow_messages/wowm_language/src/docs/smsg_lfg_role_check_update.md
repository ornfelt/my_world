# SMSG_LFG_ROLE_CHECK_UPDATE

## Client Version 3.3.5

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/lfg/smsg_lfg_role_check_update.wowm:1`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/lfg/smsg_lfg_role_check_update.wowm#L1).
```rust,ignore
smsg SMSG_LFG_ROLE_CHECK_UPDATE = 0x0363 {
    u32 rolecheck_state;
    u8 rolecheck_initializing;
    u8 amount_of_dungeon_entries;
    u32[amount_of_dungeon_entries] dungeon_entries;
    u8 amount_of_roles;
    LfgRole[amount_of_roles] roles;
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
| - | 4 / Little | u32 | rolecheck_state |  |
| - | 1 / - | u8 | rolecheck_initializing |  |
| - | 1 / - | u8 | amount_of_dungeon_entries |  |
| - | ? / - | u32[amount_of_dungeon_entries] | dungeon_entries |  |
| - | 1 / - | u8 | amount_of_roles |  |
| - | ? / - | [LfgRole](lfgrole.md)[amount_of_roles] | roles | azerothcore: Leader info MUST be sent first. |
