# SMSG_ATTACKERSTATEUPDATE

## Client Version 1.12

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/combat/smsg_attackerstateupdate.wowm:41`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/combat/smsg_attackerstateupdate.wowm#L41).
```rust,ignore
smsg SMSG_ATTACKERSTATEUPDATE = 0x014A {
    HitInfo hit_info;
    PackedGuid attacker;
    PackedGuid target;
    u32 total_damage;
    u8 amount_of_damages;
    DamageInfo[amount_of_damages] damages;
    u32 damage_state;
    u32 unknown1;
    u32 spell_id;
    u32 blocked_amount;
}
```
### Header

SMSG have a header of 4 bytes.

#### SMSG Header

| Offset | Size / Endianness | Type   | Name   | Description |
| ------ | ----------------- | ------ | ------ | ----------- |
| 0x00   | 2 / Big           | uint16 | size   | Size of the rest of the message including the opcode field but not including the size field.|
| 0x02   | 2 / Little        | uint16 | opcode | Opcode that determines which fields the message contains.|

### Body

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| 0x04 | 4 / - | [HitInfo](hitinfo.md) | hit_info |  |
| 0x08 | - / - | [PackedGuid](../types/packed-guid.md) | attacker |  |
| - | - / - | [PackedGuid](../types/packed-guid.md) | target |  |
| - | 4 / Little | u32 | total_damage |  |
| - | 1 / - | u8 | amount_of_damages |  |
| - | ? / - | [DamageInfo](damageinfo.md)[amount_of_damages] | damages |  |
| - | 4 / Little | u32 | damage_state |  |
| - | 4 / Little | u32 | unknown1 |  |
| - | 4 / Little | u32 | spell_id | vmangos: spell id, seen with heroic strike and disarm as examples |
| - | 4 / Little | u32 | blocked_amount |  |

### Examples

#### Example 1

```c
0, 51, // size
74, 1, // opcode (330)
128, 0, 0, 0, // hit_info: HitInfo CRITICAL_HIT (0x00000080)
1, 23, // attacker: PackedGuid
1, 100, // target: PackedGuid
57, 5, 0, 0, // total_damage: u32
1, // amount_of_damages: u8
0, 0, 0, 0, // [0].DamageInfo.spell_school_mask: u32
0, 128, 166, 68, // [0].DamageInfo.damage_float: f32
52, 5, 0, 0, // [0].DamageInfo.damage_uint: u32
0, 0, 0, 0, // [0].DamageInfo.absorb: u32
0, 0, 0, 0, // [0].DamageInfo.resist: u32
// damages: DamageInfo[amount_of_damages]
0, 0, 0, 0, // damage_state: u32
0, 0, 0, 0, // unknown1: u32
0, 0, 0, 0, // spell_id: u32
0, 0, 0, 0, // blocked_amount: u32
```
## Client Version 2.4.3

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/combat/smsg_attackerstateupdate.wowm:41`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/combat/smsg_attackerstateupdate.wowm#L41).
```rust,ignore
smsg SMSG_ATTACKERSTATEUPDATE = 0x014A {
    HitInfo hit_info;
    PackedGuid attacker;
    PackedGuid target;
    u32 total_damage;
    u8 amount_of_damages;
    DamageInfo[amount_of_damages] damages;
    u32 damage_state;
    u32 unknown1;
    u32 spell_id;
    u32 blocked_amount;
}
```
### Header

SMSG have a header of 4 bytes.

#### SMSG Header

| Offset | Size / Endianness | Type   | Name   | Description |
| ------ | ----------------- | ------ | ------ | ----------- |
| 0x00   | 2 / Big           | uint16 | size   | Size of the rest of the message including the opcode field but not including the size field.|
| 0x02   | 2 / Little        | uint16 | opcode | Opcode that determines which fields the message contains.|

### Body

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| 0x04 | 4 / - | [HitInfo](hitinfo.md) | hit_info |  |
| 0x08 | - / - | [PackedGuid](../types/packed-guid.md) | attacker |  |
| - | - / - | [PackedGuid](../types/packed-guid.md) | target |  |
| - | 4 / Little | u32 | total_damage |  |
| - | 1 / - | u8 | amount_of_damages |  |
| - | ? / - | [DamageInfo](damageinfo.md)[amount_of_damages] | damages |  |
| - | 4 / Little | u32 | damage_state |  |
| - | 4 / Little | u32 | unknown1 |  |
| - | 4 / Little | u32 | spell_id | vmangos: spell id, seen with heroic strike and disarm as examples |
| - | 4 / Little | u32 | blocked_amount |  |

## Client Version 3.3.5

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/combat/smsg_attackerstateupdate_3_3_5.wowm:64`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/combat/smsg_attackerstateupdate_3_3_5.wowm#L64).
```rust,ignore
smsg SMSG_ATTACKERSTATEUPDATE = 0x014A {
    HitInfo hit_info;
    PackedGuid attacker;
    PackedGuid target;
    u32 total_damage;
    u32 overkill;
    u8 amount_of_damages;
    DamageInfo[amount_of_damages] damage_infos;
    if (hit_info & ALL_ABSORB) {
        u32 absorb;
    }
    if (hit_info & ALL_RESIST) {
        u32 resist;
    }
    VictimState victim_state;
    u32 unknown1;
    u32 unknown2;
    if (hit_info & BLOCK) {
        u32 blocked_amount;
    }
    if (hit_info & UNK19) {
        u32 unknown3;
    }
    if (hit_info & UNK1) {
        u32 unknown4;
        f32 unknown5;
        f32 unknown6;
        f32 unknown7;
        f32 unknown8;
        f32 unknown9;
        f32 unknown10;
        f32 unknown11;
        f32 unknown12;
        f32 unknown13;
        f32 unknown14;
        u32 unknown15;
    }
}
```
### Header

SMSG have a header of 4 bytes.

#### SMSG Header

| Offset | Size / Endianness | Type   | Name   | Description |
| ------ | ----------------- | ------ | ------ | ----------- |
| 0x00   | 2 **OR** 3 / Big           | uint16 **OR** uint16+uint8 | size | Size of the rest of the message including the opcode field but not including the size field. Wrath server messages **can** be 3 bytes. If the first (most significant) size byte has `0x80` set, the header will be 3 bytes, otherwise it is 2.|
| -      | 2 / Little| uint16 | opcode | Opcode that determines which fields the message contains. |

### Body

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| - | 4 / - | [HitInfo](hitinfo.md) | hit_info |  |
| - | - / - | [PackedGuid](../types/packed-guid.md) | attacker |  |
| - | - / - | [PackedGuid](../types/packed-guid.md) | target |  |
| - | 4 / Little | u32 | total_damage |  |
| - | 4 / Little | u32 | overkill |  |
| - | 1 / - | u8 | amount_of_damages |  |
| - | ? / - | [DamageInfo](damageinfo.md)[amount_of_damages] | damage_infos |  |

If hit_info contains `ALL_ABSORB`:

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| - | 4 / Little | u32 | absorb |  |

If hit_info contains `ALL_RESIST`:

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| - | 4 / Little | u32 | resist |  |
| - | 1 / - | [VictimState](victimstate.md) | victim_state |  |
| - | 4 / Little | u32 | unknown1 | arcemu: can be 0,1000 or -1 |
| - | 4 / Little | u32 | unknown2 |  |

If hit_info contains `BLOCK`:

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| - | 4 / Little | u32 | blocked_amount |  |

If hit_info contains `UNK19`:

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| - | 4 / Little | u32 | unknown3 |  |

If hit_info contains `UNK1`:

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| - | 4 / Little | u32 | unknown4 |  |
| - | 4 / Little | f32 | unknown5 |  |
| - | 4 / Little | f32 | unknown6 |  |
| - | 4 / Little | f32 | unknown7 |  |
| - | 4 / Little | f32 | unknown8 |  |
| - | 4 / Little | f32 | unknown9 |  |
| - | 4 / Little | f32 | unknown10 |  |
| - | 4 / Little | f32 | unknown11 |  |
| - | 4 / Little | f32 | unknown12 |  |
| - | 4 / Little | f32 | unknown13 |  |
| - | 4 / Little | f32 | unknown14 |  |
| - | 4 / Little | u32 | unknown15 |  |
