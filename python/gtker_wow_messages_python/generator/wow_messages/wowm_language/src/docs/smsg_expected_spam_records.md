# SMSG_EXPECTED_SPAM_RECORDS

## Client Version 1.12

Not implemented in Wrath or TBC emus. Only implemented in cmangos.

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/chat/smsg_expected_spam_records.wowm:4`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/chat/smsg_expected_spam_records.wowm#L4).
```rust,ignore
smsg SMSG_EXPECTED_SPAM_RECORDS = 0x0332 {
    u32 amount_of_records;
    CString[amount_of_records] records;
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
| 0x04 | 4 / Little | u32 | amount_of_records |  |
| 0x08 | ? / - | CString[amount_of_records] | records |  |
