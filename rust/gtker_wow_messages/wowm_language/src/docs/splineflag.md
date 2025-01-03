# SplineFlag

## Client Version 1.12, Client Version 2

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/gameobject/smsg_update_object.wowm:27`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/gameobject/smsg_update_object.wowm#L27).

```rust,ignore
flag SplineFlag : u32 {
    NONE = 0x00000000;
    DONE = 0x00000001;
    FALLING = 0x00000002;
    UNKNOWN3 = 0x00000004;
    UNKNOWN4 = 0x00000008;
    UNKNOWN5 = 0x00000010;
    UNKNOWN6 = 0x00000020;
    UNKNOWN7 = 0x00000040;
    UNKNOWN8 = 0x00000080;
    RUN_MODE = 0x00000100;
    FLYING = 0x00000200;
    NO_SPLINE = 0x00000400;
    UNKNOWN12 = 0x00000800;
    UNKNOWN13 = 0x00001000;
    UNKNOWN14 = 0x00002000;
    UNKNOWN15 = 0x00004000;
    UNKNOWN16 = 0x00008000;
    FINAL_POINT = 0x00010000;
    FINAL_TARGET = 0x00020000;
    FINAL_ANGLE = 0x00040000;
    UNKNOWN19 = 0x00080000;
    CYCLIC = 0x00100000;
    ENTER_CYCLE = 0x00200000;
    FROZEN = 0x00400000;
    UNKNOWN23 = 0x00800000;
    UNKNOWN24 = 0x01000000;
    UNKNOWN25 = 0x02000000;
    UNKNOWN26 = 0x04000000;
    UNKNOWN27 = 0x08000000;
    UNKNOWN28 = 0x10000000;
    UNKNOWN29 = 0x20000000;
    UNKNOWN30 = 0x40000000;
    UNKNOWN31 = 0x80000000;
}
```
### Type
The basic type is `u32`, a 4 byte (32 bit) little endian integer.
### Enumerators
| Enumerator | Value  | Comment |
| --------- | -------- | ------- |
| `NONE` | 0 (0x00) |  |
| `DONE` | 1 (0x01) |  |
| `FALLING` | 2 (0x02) | vmangos: Affects elevation computation |
| `UNKNOWN3` | 4 (0x04) |  |
| `UNKNOWN4` | 8 (0x08) |  |
| `UNKNOWN5` | 16 (0x10) |  |
| `UNKNOWN6` | 32 (0x20) |  |
| `UNKNOWN7` | 64 (0x40) |  |
| `UNKNOWN8` | 128 (0x80) |  |
| `RUN_MODE` | 256 (0x100) |  |
| `FLYING` | 512 (0x200) | vmangos: Smooth movement(Catmullrom interpolation mode), flying animation |
| `NO_SPLINE` | 1024 (0x400) |  |
| `UNKNOWN12` | 2048 (0x800) |  |
| `UNKNOWN13` | 4096 (0x1000) |  |
| `UNKNOWN14` | 8192 (0x2000) |  |
| `UNKNOWN15` | 16384 (0x4000) |  |
| `UNKNOWN16` | 32768 (0x8000) |  |
| `FINAL_POINT` | 65536 (0x10000) |  |
| `FINAL_TARGET` | 131072 (0x20000) |  |
| `FINAL_ANGLE` | 262144 (0x40000) |  |
| `UNKNOWN19` | 524288 (0x80000) | vmangos: exists, but unknown what it does |
| `CYCLIC` | 1048576 (0x100000) | vmangos: Movement by cycled spline |
| `ENTER_CYCLE` | 2097152 (0x200000) | vmangos: Everytimes appears with cyclic flag in monster move packet, erases first spline vertex after first cycle done |
| `FROZEN` | 4194304 (0x400000) | vmangos: Will never arrive |
| `UNKNOWN23` | 8388608 (0x800000) |  |
| `UNKNOWN24` | 16777216 (0x1000000) |  |
| `UNKNOWN25` | 33554432 (0x2000000) | vmangos: exists, but unknown what it does |
| `UNKNOWN26` | 67108864 (0x4000000) |  |
| `UNKNOWN27` | 134217728 (0x8000000) |  |
| `UNKNOWN28` | 268435456 (0x10000000) |  |
| `UNKNOWN29` | 536870912 (0x20000000) |  |
| `UNKNOWN30` | 1073741824 (0x40000000) |  |
| `UNKNOWN31` | 2147483648 (0x80000000) |  |

Used in:
* [MonsterMove](monstermove.md)
* [MovementBlock](movementblock.md)
* [MovementBlock](movementblock.md)
* [SMSG_MONSTER_MOVE](smsg_monster_move.md)
* [SMSG_MONSTER_MOVE_TRANSPORT](smsg_monster_move_transport.md)
## Client Version 3.3.5

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/gameobject/smsg_update_object_3_3_5.wowm:26`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/gameobject/smsg_update_object_3_3_5.wowm#L26).

```rust,ignore
flag SplineFlag : u32 {
    NONE = 0x00000000;
    DONE = 0x00000100;
    FALLING = 0x00000200;
    NO_SPLINE = 0x00000400;
    PARABOLIC = 0x00000800;
    WALK_MODE = 0x00001000;
    FLYING = 0x00002000;
    ORIENTATION_FIXED = 0x00004000;
    FINAL_POINT = 0x00008000;
    FINAL_TARGET = 0x00010000;
    FINAL_ANGLE = 0x00020000;
    CATMULLROM = 0x00040000;
    CYCLIC = 0x00080000;
    ENTER_CYCLE = 0x00100000;
    ANIMATION = 0x00200000;
    FROZEN = 0x00400000;
    TRANSPORT_ENTER = 0x00800000;
    TRANSPORT_EXIT = 0x01000000;
    UNKNOWN7 = 0x02000000;
    UNKNOWN8 = 0x04000000;
    ORIENTATION_INVERSED = 0x08000000;
    UNKNOWN10 = 0x10000000;
    UNKNOWN11 = 0x20000000;
    UNKNOWN12 = 0x40000000;
    UNKNOWN13 = 0x80000000;
}
```
### Type
The basic type is `u32`, a 4 byte (32 bit) little endian integer.
### Enumerators
| Enumerator | Value  | Comment |
| --------- | -------- | ------- |
| `NONE` | 0 (0x00) |  |
| `DONE` | 256 (0x100) |  |
| `FALLING` | 512 (0x200) | vmangos: Affects elevation computation |
| `NO_SPLINE` | 1024 (0x400) |  |
| `PARABOLIC` | 2048 (0x800) |  |
| `WALK_MODE` | 4096 (0x1000) |  |
| `FLYING` | 8192 (0x2000) |  |
| `ORIENTATION_FIXED` | 16384 (0x4000) |  |
| `FINAL_POINT` | 32768 (0x8000) |  |
| `FINAL_TARGET` | 65536 (0x10000) |  |
| `FINAL_ANGLE` | 131072 (0x20000) |  |
| `CATMULLROM` | 262144 (0x40000) | azerothcore: Used Catmullrom interpolation mode |
| `CYCLIC` | 524288 (0x80000) | azerothcore: Movement by cycled spline |
| `ENTER_CYCLE` | 1048576 (0x100000) | azerothcore: Everytimes appears with cyclic flag in monster move packet, erases first spline vertex after first cycle done |
| `ANIMATION` | 2097152 (0x200000) | azerothcore: Plays animation after some time passed |
| `FROZEN` | 4194304 (0x400000) | vmangos: Will never arrive |
| `TRANSPORT_ENTER` | 8388608 (0x800000) |  |
| `TRANSPORT_EXIT` | 16777216 (0x1000000) |  |
| `UNKNOWN7` | 33554432 (0x2000000) | vmangos: exists, but unknown what it does |
| `UNKNOWN8` | 67108864 (0x4000000) |  |
| `ORIENTATION_INVERSED` | 134217728 (0x8000000) |  |
| `UNKNOWN10` | 268435456 (0x10000000) |  |
| `UNKNOWN11` | 536870912 (0x20000000) |  |
| `UNKNOWN12` | 1073741824 (0x40000000) |  |
| `UNKNOWN13` | 2147483648 (0x80000000) |  |

Used in:
* [MovementBlock](movementblock.md)
* [SMSG_MONSTER_MOVE](smsg_monster_move.md)
* [SMSG_MONSTER_MOVE_TRANSPORT](smsg_monster_move_transport.md)
