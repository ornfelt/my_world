# SMSG_CALENDAR_RAID_LOCKOUT_UPDATED

## Client Version 3.3.5

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/calendar/smsg_calendar_raid_lockout_updated.wowm:1`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/calendar/smsg_calendar_raid_lockout_updated.wowm#L1).
```rust,ignore
smsg SMSG_CALENDAR_RAID_LOCKOUT_UPDATED = 0x0471 {
    DateTime current_time;
    Map map;
    u32 difficulty;
    Seconds old_time_to_update;
    Seconds new_time_to_update;
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
| 0x04 | 4 / Little | DateTime | current_time |  |
| 0x08 | 4 / - | [Map](map.md) | map |  |
| 0x0C | 4 / Little | u32 | difficulty |  |
| 0x10 | 4 / Little | Seconds | old_time_to_update |  |
| 0x14 | 4 / Little | Seconds | new_time_to_update |  |
