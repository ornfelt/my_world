# JoinArenaType

## Client Version 2.4.3, Client Version 3

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/battleground/cmsg_battlemaster_join_arena.wowm:1`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/battleground/cmsg_battlemaster_join_arena.wowm#L1).

```rust,ignore
enum JoinArenaType : u8 {
    TWO_VS_TWO = 0;
    THREE_VS_THREE = 1;
    FIVE_VS_FIVE = 2;
}
```
### Type
The basic type is `u8`, a 1 byte (8 bit) integer.
### Enumerators
| Enumerator | Value  | Comment |
| --------- | -------- | ------- |
| `TWO_VS_TWO` | 0 (0x00) |  |
| `THREE_VS_THREE` | 1 (0x01) |  |
| `FIVE_VS_FIVE` | 2 (0x02) |  |

Used in:
* [CMSG_BATTLEMASTER_JOIN_ARENA](cmsg_battlemaster_join_arena.md)
