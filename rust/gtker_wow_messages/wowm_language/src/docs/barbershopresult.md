# BarberShopResult

## Client Version 3.3.5

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/character_screen/smsg_barber_shop_result.wowm:1`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/character_screen/smsg_barber_shop_result.wowm#L1).

```rust,ignore
enum BarberShopResult : u8 {
    OK = 0;
    NOT_ENOUGH_MONEY = 1;
    MUST_BE_SEATED_IN_BARBER_CHAIR = 2;
    NOT_ENOUGH_MONEY2 = 3;
}
```
### Type
The basic type is `u8`, a 1 byte (8 bit) integer.
### Enumerators
| Enumerator | Value  | Comment |
| --------- | -------- | ------- |
| `OK` | 0 (0x00) |  |
| `NOT_ENOUGH_MONEY` | 1 (0x01) |  |
| `MUST_BE_SEATED_IN_BARBER_CHAIR` | 2 (0x02) |  |
| `NOT_ENOUGH_MONEY2` | 3 (0x03) |  |

Used in:
* [SMSG_BARBER_SHOP_RESULT](smsg_barber_shop_result.md)
