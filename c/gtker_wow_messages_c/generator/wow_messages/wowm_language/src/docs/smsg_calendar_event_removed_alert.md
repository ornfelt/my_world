# SMSG_CALENDAR_EVENT_REMOVED_ALERT

## Client Version 3.3.5

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/calendar/smsg_calendar_event_removed_alert.wowm:1`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/calendar/smsg_calendar_event_removed_alert.wowm#L1).
```rust,ignore
smsg SMSG_CALENDAR_EVENT_REMOVED_ALERT = 0x0443 {
    Bool show_alert;
    Guid event_id;
    DateTime event_time;
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
| 0x04 | 1 / - | Bool | show_alert |  |
| 0x05 | 8 / Little | [Guid](../types/packed-guid.md) | event_id |  |
| 0x0D | 4 / Little | DateTime | event_time |  |
