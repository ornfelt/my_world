# UpdateFlag

## Client Version 1.12, Client Version 2.4.3

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/gameobject/smsg_update_object.wowm:14`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/gameobject/smsg_update_object.wowm#L14).

```rust,ignore
flag UpdateFlag : u8 {
    NONE = 0x00;
    SELF = 0x01;
    TRANSPORT = 0x02;
    MELEE_ATTACKING = 0x04;
    HIGH_GUID = 0x08;
    ALL = 0x10;
    LIVING = 0x20;
    HAS_POSITION = 0x40;
}
```
### Type
The basic type is `u8`, a 1 byte (8 bit) integer.
### Enumerators
| Enumerator | Value  | Comment |
| --------- | -------- | ------- |
| `NONE` | 0 (0x00) |  |
| `SELF` | 1 (0x01) |  |
| `TRANSPORT` | 2 (0x02) |  |
| `MELEE_ATTACKING` | 4 (0x04) |  |
| `HIGH_GUID` | 8 (0x08) |  |
| `ALL` | 16 (0x10) |  |
| `LIVING` | 32 (0x20) |  |
| `HAS_POSITION` | 64 (0x40) |  |

Used in:
* [MovementBlock](movementblock.md)
* [MovementBlock](movementblock.md)
## Client Version 3.3.5

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/gameobject/smsg_update_object_3_3_5.wowm:12`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/gameobject/smsg_update_object_3_3_5.wowm#L12).

```rust,ignore
flag UpdateFlag : u16 {
    NONE = 0x0000;
    SELF = 0x0001;
    TRANSPORT = 0x0002;
    HAS_ATTACKING_TARGET = 0x0004;
    LOW_GUID = 0x0008;
    HIGH_GUID = 0x0010;
    LIVING = 0x0020;
    HAS_POSITION = 0x0040;
    VEHICLE = 0x0080;
    POSITION = 0x0100;
    ROTATION = 0x0200;
}
```
### Type
The basic type is `u16`, a 2 byte (16 bit) little endian integer.
### Enumerators
| Enumerator | Value  | Comment |
| --------- | -------- | ------- |
| `NONE` | 0 (0x00) |  |
| `SELF` | 1 (0x01) |  |
| `TRANSPORT` | 2 (0x02) |  |
| `HAS_ATTACKING_TARGET` | 4 (0x04) |  |
| `LOW_GUID` | 8 (0x08) |  |
| `HIGH_GUID` | 16 (0x10) |  |
| `LIVING` | 32 (0x20) |  |
| `HAS_POSITION` | 64 (0x40) |  |
| `VEHICLE` | 128 (0x80) |  |
| `POSITION` | 256 (0x100) |  |
| `ROTATION` | 512 (0x200) |  |

Used in:
* [MovementBlock](movementblock.md)