# LfgJoinLockedDungeon

## Client Version 3.3.5

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/lfg/smsg_lfg_join_result.wowm:17`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/lfg/smsg_lfg_join_result.wowm#L17).
```rust,ignore
struct LfgJoinLockedDungeon {
    u32 dungeon_entry;
    u32 reason;
}
```
### Body

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| 0x00 | 4 / Little | u32 | dungeon_entry |  |
| 0x04 | 4 / Little | u32 | reason |  |


Used in:
* [LfgJoinPlayer](lfgjoinplayer.md)
* [LfgPartyInfo](lfgpartyinfo.md)
* [SMSG_LFG_PLAYER_INFO](smsg_lfg_player_info.md)
