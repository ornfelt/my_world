# PetitionSignature

## Client Version 1, Client Version 2, Client Version 3

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/guild/smsg_petition_show_signatures.wowm:3`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/guild/smsg_petition_show_signatures.wowm#L3).
```rust,ignore
struct PetitionSignature {
    Guid signer;
    u32 unknown1;
}
```
### Body

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| 0x00 | 8 / Little | [Guid](../types/packed-guid.md) | signer |  |
| 0x08 | 4 / Little | u32 | unknown1 |  |


Used in:
* [SMSG_PETITION_SHOW_SIGNATURES](smsg_petition_show_signatures.md)
