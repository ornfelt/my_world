# NewItemCreationType

## Client Version 1, Client Version 2, Client Version 3

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/item/smsg_item_push_result.wowm:8`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/item/smsg_item_push_result.wowm#L8).

```rust,ignore
enum NewItemCreationType : u32 {
    RECEIVED = 0;
    CREATED = 1;
}
```
### Type
The basic type is `u32`, a 4 byte (32 bit) little endian integer.
### Enumerators
| Enumerator | Value  | Comment |
| --------- | -------- | ------- |
| `RECEIVED` | 0 (0x00) |  |
| `CREATED` | 1 (0x01) |  |

Used in:
* [SMSG_ITEM_PUSH_RESULT](smsg_item_push_result.md)
* [SMSG_ITEM_PUSH_RESULT](smsg_item_push_result.md)
