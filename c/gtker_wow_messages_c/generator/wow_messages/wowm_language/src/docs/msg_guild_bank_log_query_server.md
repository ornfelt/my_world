# MSG_GUILD_BANK_LOG_QUERY_Server

## Client Version 2.4.3

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/queries/msg_guild_bank_log_query.wowm:7`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/queries/msg_guild_bank_log_query.wowm#L7).
```rust,ignore
smsg MSG_GUILD_BANK_LOG_QUERY_Server = 0x03ED {
    u32 unix_time;
    u8 slot;
    u8 amount_of_money_logs;
    MoneyLogItem[amount_of_money_logs] money_logs;
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
| 0x04 | 4 / Little | u32 | unix_time |  |
| 0x08 | 1 / - | u8 | slot |  |
| 0x09 | 1 / - | u8 | amount_of_money_logs |  |
| 0x0A | ? / - | [MoneyLogItem](moneylogitem.md)[amount_of_money_logs] | money_logs |  |

## Client Version 3.3.5

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/queries/msg_guild_bank_log_query.wowm:31`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/queries/msg_guild_bank_log_query.wowm#L31).
```rust,ignore
smsg MSG_GUILD_BANK_LOG_QUERY_Server = 0x03EE {
    u32 unix_time;
    u8 slot;
    u8 amount_of_money_logs;
    MoneyLogItem[amount_of_money_logs] money_logs;
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
| - | 4 / Little | u32 | unix_time |  |
| - | 1 / - | u8 | slot |  |
| - | 1 / - | u8 | amount_of_money_logs |  |
| - | ? / - | [MoneyLogItem](moneylogitem.md)[amount_of_money_logs] | money_logs |  |
