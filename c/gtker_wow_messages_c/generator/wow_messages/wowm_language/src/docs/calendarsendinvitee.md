# CalendarSendInvitee

## Client Version 3.3.5

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/calendar/smsg_calendar_send_event.wowm:1`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/calendar/smsg_calendar_send_event.wowm#L1).
```rust,ignore
struct CalendarSendInvitee {
    PackedGuid invitee;
    Level level;
    u8 status;
    u8 rank;
    u8 guild_member;
    Guid invite_id;
    DateTime status_time;
    CString text;
}
```
### Body

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| 0x00 | - / - | [PackedGuid](../types/packed-guid.md) | invitee |  |
| - | 1 / - | Level | level |  |
| - | 1 / - | u8 | status |  |
| - | 1 / - | u8 | rank |  |
| - | 1 / - | u8 | guild_member |  |
| - | 8 / Little | [Guid](../types/packed-guid.md) | invite_id |  |
| - | 4 / Little | DateTime | status_time |  |
| - | - / - | CString | text |  |


Used in:
* [SMSG_CALENDAR_SEND_EVENT](smsg_calendar_send_event.md)
