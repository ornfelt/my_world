# BankTab

## Client Version 2.4.3, Client Version 3

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/guild/msg_guild_permissions.wowm:9`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/guild/msg_guild_permissions.wowm#L9).
```rust,ignore
struct BankTab {
    u32 flags;
    u32 stacks_per_day;
}
```
### Body

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| 0x00 | 4 / Little | u32 | flags |  |
| 0x04 | 4 / Little | u32 | stacks_per_day |  |


Used in:
* [MSG_GUILD_PERMISSIONS_Server](msg_guild_permissions_server.md)
