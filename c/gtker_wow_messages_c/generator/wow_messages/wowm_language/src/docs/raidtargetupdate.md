# RaidTargetUpdate

## Client Version 1, Client Version 2, Client Version 3

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/raid/raid_target.wowm:21`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/raid/raid_target.wowm#L21).
```rust,ignore
struct RaidTargetUpdate {
    RaidTargetIndex index;
    Guid guid;
}
```
### Body

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| 0x00 | 1 / - | [RaidTargetIndex](raidtargetindex.md) | index |  |
| 0x01 | 8 / Little | [Guid](../types/packed-guid.md) | guid |  |


Used in:
* [MSG_RAID_TARGET_UPDATE_Server](msg_raid_target_update_server.md)
