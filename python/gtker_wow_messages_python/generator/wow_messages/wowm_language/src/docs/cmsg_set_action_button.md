# CMSG_SET_ACTION_BUTTON

## Client Version 1, Client Version 2, Client Version 3

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/login_logout/cmsg_set_action_button.wowm:1`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/login_logout/cmsg_set_action_button.wowm#L1).
```rust,ignore
cmsg CMSG_SET_ACTION_BUTTON = 0x0128 {
    u8 button;
    u16 action;
    u8 misc;
    u8 action_type;
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

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| 0x06 | 1 / - | u8 | button |  |
| 0x07 | 2 / Little | u16 | action |  |
| 0x09 | 1 / - | u8 | misc |  |
| 0x0A | 1 / - | u8 | action_type |  |
