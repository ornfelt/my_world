# SMSG_DISPEL_FAILED

## Client Version 1, Client Version 2, Client Version 3

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/spell/smsg_dispel_failed.wowm:3`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/spell/smsg_dispel_failed.wowm#L3).
```rust,ignore
smsg SMSG_DISPEL_FAILED = 0x0262 {
    Guid caster;
    Guid target;
    Spell[-] spells;
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
| - | 8 / Little | [Guid](../types/packed-guid.md) | caster |  |
| - | 8 / Little | [Guid](../types/packed-guid.md) | target |  |
| - | ? / - | Spell[-] | spells |  |
