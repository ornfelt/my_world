# GuildLogEvent

## Client Version 2.4.3, Client Version 3

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/queries/msg_guild_event_log_query.wowm:9`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/queries/msg_guild_event_log_query.wowm#L9).
```rust,ignore
struct GuildLogEvent {
    GuildEvent event;
    Guid player1;
    if (event == JOINED
        || event == LEFT) {
        Guid player2;
    }
    else if (event == PROMOTION
        || event == DEMOTION) {
        u8 new_rank;
    }
    u32 unix_time;
}
```
### Body

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| 0x00 | 1 / - | [GuildEvent](guildevent.md) | event |  |
| 0x01 | 8 / Little | [Guid](../types/packed-guid.md) | player1 |  |

If event is equal to `JOINED` **or** 
is equal to `LEFT`:

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| 0x09 | 8 / Little | [Guid](../types/packed-guid.md) | player2 |  |

Else If event is equal to `PROMOTION` **or** 
is equal to `DEMOTION`:

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| 0x11 | 1 / - | u8 | new_rank |  |
| 0x12 | 4 / Little | u32 | unix_time |  |


Used in:
* [MSG_GUILD_EVENT_LOG_QUERY_Server](msg_guild_event_log_query_server.md)
