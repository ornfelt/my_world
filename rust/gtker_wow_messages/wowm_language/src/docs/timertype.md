# TimerType

## Client Version 1, Client Version 2, Client Version 3

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/spell/spell_common.wowm:1`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/spell/spell_common.wowm#L1).

```rust,ignore
enum TimerType : u32 {
    FATIGUE = 0;
    BREATH = 1;
    FEIGN_DEATH = 2;
    ENVIRONMENTAL = 3;
}
```
### Type
The basic type is `u32`, a 4 byte (32 bit) little endian integer.
### Enumerators
| Enumerator | Value  | Comment |
| --------- | -------- | ------- |
| `FATIGUE` | 0 (0x00) |  |
| `BREATH` | 1 (0x01) |  |
| `FEIGN_DEATH` | 2 (0x02) |  |
| `ENVIRONMENTAL` | 3 (0x03) | Might be a mangos only thing. |

Used in:
* [SMSG_PAUSE_MIRROR_TIMER](smsg_pause_mirror_timer.md)
* [SMSG_START_MIRROR_TIMER](smsg_start_mirror_timer.md)
* [SMSG_STOP_MIRROR_TIMER](smsg_stop_mirror_timer.md)
