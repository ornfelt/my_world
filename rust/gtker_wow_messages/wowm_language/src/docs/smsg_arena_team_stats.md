# SMSG_ARENA_TEAM_STATS

## Client Version 2.4.3, Client Version 3

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/arena/smsg_arena_team_stats.wowm:1`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/arena/smsg_arena_team_stats.wowm#L1).
```rust,ignore
smsg SMSG_ARENA_TEAM_STATS = 0x035B {
    u32 arena_team;
    u32 rating;
    u32 games_played_this_week;
    u32 games_won_this_week;
    u32 games_played_this_season;
    u32 games_won_this_season;
    u32 ranking;
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
| 0x04 | 4 / Little | u32 | arena_team |  |
| 0x08 | 4 / Little | u32 | rating |  |
| 0x0C | 4 / Little | u32 | games_played_this_week |  |
| 0x10 | 4 / Little | u32 | games_won_this_week |  |
| 0x14 | 4 / Little | u32 | games_played_this_season |  |
| 0x18 | 4 / Little | u32 | games_won_this_season |  |
| 0x1C | 4 / Little | u32 | ranking |  |
