# SMSG_COOLDOWN_EVENT

## Client Version 1, Client Version 2, Client Version 3

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/spell/smsg_cooldown_event.wowm:3`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/spell/smsg_cooldown_event.wowm#L3).
```rust,ignore
smsg SMSG_COOLDOWN_EVENT = 0x0135 {
    Spell id;
    Guid guid;
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
| 0x04 | 4 / Little | Spell | id |  |
| 0x08 | 8 / Little | [Guid](../types/packed-guid.md) | guid |  |
