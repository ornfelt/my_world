# CMSG_LFD_PARTY_LOCK_INFO_REQUEST

## Client Version 3.3.5

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/lfg/cmsg_lfd_party_lock_info_request.wowm:1`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/lfg/cmsg_lfd_party_lock_info_request.wowm#L1).
```rust,ignore
cmsg CMSG_LFD_PARTY_LOCK_INFO_REQUEST = 0x0371 {
}
```
### Header

CMSG have a header of 6 bytes.

#### CMSG Header

| Offset | Size / Endianness | Type   | Name   | Description |
| ------ | ----------------- | ------ | ------ | ----------- |
| 0x00   | 2 / Big           | uint16 | size   | Size of the rest of the message including the opcode field but not including the size field.|
| 0x02   | 4 / Little        | uint32 | opcode | Opcode that determines which fields the message contains.|

### Body

This message has no fields in the body.
