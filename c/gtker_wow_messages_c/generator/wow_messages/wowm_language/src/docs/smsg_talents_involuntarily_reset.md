# SMSG_TALENTS_INVOLUNTARILY_RESET

## Client Version 3.3.5

Only exists as comment in azerothcore/trinitycore.

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/spell/smsg_talents_involuntarily_reset.wowm:2`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/spell/smsg_talents_involuntarily_reset.wowm#L2).
```rust,ignore
smsg SMSG_TALENTS_INVOLUNTARILY_RESET = 0x04FA {
    u8 unknown;
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
| 0x04 | 1 / - | u8 | unknown |  |
