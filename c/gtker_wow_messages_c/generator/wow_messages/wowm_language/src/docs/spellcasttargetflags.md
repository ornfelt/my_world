# SpellCastTargetFlags

## Client Version 1.12

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/common.wowm:1`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/common.wowm#L1).

```rust,ignore
flag SpellCastTargetFlags : u16 {
    SELF = 0x00000000;
    UNUSED1 = 0x00000001;
    UNIT = 0x00000002;
    UNUSED2 = 0x00000004;
    UNUSED3 = 0x00000008;
    ITEM = 0x00000010;
    SOURCE_LOCATION = 0x00000020;
    DEST_LOCATION = 0x00000040;
    OBJECT_UNK = 0x00000080;
    UNIT_UNK = 0x00000100;
    PVP_CORPSE = 0x00000200;
    UNIT_CORPSE = 0x00000400;
    GAMEOBJECT = 0x00000800;
    TRADE_ITEM = 0x00001000;
    STRING = 0x00002000;
    UNK1 = 0x00004000;
    CORPSE = 0x00008000;
}
```
### Type
The basic type is `u16`, a 2 byte (16 bit) little endian integer.
### Enumerators
| Enumerator | Value  | Comment |
| --------- | -------- | ------- |
| `SELF` | 0 (0x00) |  |
| `UNUSED1` | 1 (0x01) | not used in any spells (can be set dynamically) |
| `UNIT` | 2 (0x02) |  |
| `UNUSED2` | 4 (0x04) | not used in any spells (can be set dynamically) |
| `UNUSED3` | 8 (0x08) | not used in any spells (can be set dynamically) |
| `ITEM` | 16 (0x10) |  |
| `SOURCE_LOCATION` | 32 (0x20) |  |
| `DEST_LOCATION` | 64 (0x40) |  |
| `OBJECT_UNK` | 128 (0x80) | used in 7 spells only |
| `UNIT_UNK` | 256 (0x100) | looks like self target (389 spells) |
| `PVP_CORPSE` | 512 (0x200) |  |
| `UNIT_CORPSE` | 1024 (0x400) | 10 spells (gathering professions) |
| `GAMEOBJECT` | 2048 (0x800) | pguid, 0 spells |
| `TRADE_ITEM` | 4096 (0x1000) | pguid, 0 spells |
| `STRING` | 8192 (0x2000) | string, 0 spells |
| `UNK1` | 16384 (0x4000) | 199 spells, opening object/lock |
| `CORPSE` | 32768 (0x8000) | pguid, resurrection spells |

Used in:
* [SpellCastTargets](spellcasttargets.md)
## Client Version 2.4.3

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/common.wowm:34`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/common.wowm#L34).

```rust,ignore
flag SpellCastTargetFlags : u32 {
    SELF = 0x00000000;
    UNUSED1 = 0x00000001;
    UNIT = 0x00000002;
    UNIT_RAID = 0x00000004;
    UNIT_PARTY = 0x00000008;
    ITEM = 0x00000010;
    SOURCE_LOCATION = 0x00000020;
    DEST_LOCATION = 0x00000040;
    UNIT_ENEMY = 0x00000080;
    UNIT_ALLY = 0x00000100;
    CORPSE_ENEMY = 0x00000200;
    UNIT_DEAD = 0x00000400;
    GAMEOBJECT = 0x00000800;
    TRADE_ITEM = 0x00001000;
    STRING = 0x00002000;
    LOCKED = 0x00004000;
    CORPSE_ALLY = 0x00008000;
    UNIT_MINIPET = 0x00010000;
}
```
### Type
The basic type is `u32`, a 4 byte (32 bit) little endian integer.
### Enumerators
| Enumerator | Value  | Comment |
| --------- | -------- | ------- |
| `SELF` | 0 (0x00) |  |
| `UNUSED1` | 1 (0x01) | not used in any spells as of 2.4.3 (can be set dynamically) |
| `UNIT` | 2 (0x02) |  |
| `UNIT_RAID` | 4 (0x04) | not used in any spells as of 2.4.3 (can be set dynamically) - raid member |
| `UNIT_PARTY` | 8 (0x08) | not used in any spells as of 2.4.3 (can be set dynamically) - party member |
| `ITEM` | 16 (0x10) |  |
| `SOURCE_LOCATION` | 32 (0x20) |  |
| `DEST_LOCATION` | 64 (0x40) |  |
| `UNIT_ENEMY` | 128 (0x80) | `CanAttack` == true |
| `UNIT_ALLY` | 256 (0x100) | `CanAssist` == true |
| `CORPSE_ENEMY` | 512 (0x200) | pguid, `CanAssist` == false |
| `UNIT_DEAD` | 1024 (0x400) | skinning-like effects |
| `GAMEOBJECT` | 2048 (0x800) | pguid, 0 spells in 2.4.3 |
| `TRADE_ITEM` | 4096 (0x1000) | pguid, 0 spells |
| `STRING` | 8192 (0x2000) | string, 0 spells |
| `LOCKED` | 16384 (0x4000) | 199 spells, opening object/lock |
| `CORPSE_ALLY` | 32768 (0x8000) | pguid, `CanAssist` == true |
| `UNIT_MINIPET` | 65536 (0x10000) | pguid, not used in any spells as of 2.4.3 (can be set dynamically) |

Used in:
* [SpellCastTargets](spellcasttargets.md)
## Client Version 3.3.5

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/common.wowm:70`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/common.wowm#L70).

```rust,ignore
flag SpellCastTargetFlags : u32 {
    SELF = 0x00000000;
    UNUSED1 = 0x00000001;
    UNIT = 0x00000002;
    UNIT_RAID = 0x00000004;
    UNIT_PARTY = 0x00000008;
    ITEM = 0x00000010;
    SOURCE_LOCATION = 0x00000020;
    DEST_LOCATION = 0x00000040;
    UNIT_ENEMY = 0x00000080;
    UNIT_ALLY = 0x00000100;
    CORPSE_ENEMY = 0x00000200;
    UNIT_DEAD = 0x00000400;
    GAMEOBJECT = 0x00000800;
    TRADE_ITEM = 0x00001000;
    STRING = 0x00002000;
    LOCKED = 0x00004000;
    CORPSE_ALLY = 0x00008000;
    UNIT_MINIPET = 0x00010000;
    GLYPH_SLOT = 0x00020000;
    DEST_TARGET = 0x00040000;
    UNUSED20 = 0x00080000;
    UNIT_PASSENGER = 0x00100000;
}
```
### Type
The basic type is `u32`, a 4 byte (32 bit) little endian integer.
### Enumerators
| Enumerator | Value  | Comment |
| --------- | -------- | ------- |
| `SELF` | 0 (0x00) |  |
| `UNUSED1` | 1 (0x01) | not used in any spells as of 2.4.3 (can be set dynamically) |
| `UNIT` | 2 (0x02) | pguid |
| `UNIT_RAID` | 4 (0x04) | not used in any spells as of 2.4.3 (can be set dynamically) - raid member |
| `UNIT_PARTY` | 8 (0x08) | not used in any spells as of 2.4.3 (can be set dynamically) - party member |
| `ITEM` | 16 (0x10) | pguid |
| `SOURCE_LOCATION` | 32 (0x20) | 3xfloat |
| `DEST_LOCATION` | 64 (0x40) | 3xfloat |
| `UNIT_ENEMY` | 128 (0x80) | `CanAttack` == true |
| `UNIT_ALLY` | 256 (0x100) | `CanAssist` == true |
| `CORPSE_ENEMY` | 512 (0x200) | pguid, `CanAssist` == false |
| `UNIT_DEAD` | 1024 (0x400) | skinning-like effects |
| `GAMEOBJECT` | 2048 (0x800) | pguid, 0 spells in 2.4.3 |
| `TRADE_ITEM` | 4096 (0x1000) | pguid, 0 spells |
| `STRING` | 8192 (0x2000) | string, 0 spells |
| `LOCKED` | 16384 (0x4000) | 199 spells, opening object/lock |
| `CORPSE_ALLY` | 32768 (0x8000) | pguid, `CanAssist` == true |
| `UNIT_MINIPET` | 65536 (0x10000) | pguid, not used in any spells as of 2.4.3 (can be set dynamically) |
| `GLYPH_SLOT` | 131072 (0x20000) | used in glyph spells |
| `DEST_TARGET` | 262144 (0x40000) | sometimes appears with `DEST_TARGET` spells (may appear or not for a given spell) |
| `UNUSED20` | 524288 (0x80000) | uint32 counter loop, vec3 - screen position (?) guid, not used so far |
| `UNIT_PASSENGER` | 1048576 (0x100000) | guessed, used to validate target (if vehicle passenger) |

Used in:
* [SpellCastTargets](spellcasttargets.md)