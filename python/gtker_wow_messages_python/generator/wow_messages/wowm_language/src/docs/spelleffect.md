# SpellEffect

## Client Version 1.12

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/spell/smsg_spelllogexecute.wowm:10`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/spell/smsg_spelllogexecute.wowm#L10).

```rust,ignore
enum SpellEffect : u32 {
    NONE = 0;
    INSTAKILL = 1;
    SCHOOL_DAMAGE = 2;
    DUMMY = 3;
    PORTAL_TELEPORT = 4;
    TELEPORT_UNITS = 5;
    APPLY_AURA = 6;
    ENVIRONMENTAL_DAMAGE = 7;
    POWER_DRAIN = 8;
    HEALTH_LEECH = 9;
    HEAL = 10;
    BIND = 11;
    PORTAL = 12;
    RITUAL_BASE = 13;
    RITUAL_SPECIALIZE = 14;
    RITUAL_ACTIVATE_PORTAL = 15;
    QUEST_COMPLETE = 16;
    WEAPON_DAMAGE_NOSCHOOL = 17;
    RESURRECT = 18;
    ADD_EXTRA_ATTACKS = 19;
    DODGE = 20;
    EVADE = 21;
    PARRY = 22;
    BLOCK = 23;
    CREATE_ITEM = 24;
    WEAPON = 25;
    DEFENSE = 26;
    PERSISTENT_AREA_AURA = 27;
    SUMMON = 28;
    LEAP = 29;
    ENERGIZE = 30;
    WEAPON_PERCENT_DAMAGE = 31;
    TRIGGER_MISSILE = 32;
    OPEN_LOCK = 33;
    SUMMON_CHANGE_ITEM = 34;
    APPLY_AREA_AURA_PARTY = 35;
    LEARN_SPELL = 36;
    SPELL_DEFENSE = 37;
    DISPEL = 38;
    LANGUAGE = 39;
    DUAL_WIELD = 40;
    SUMMON_WILD = 41;
    SUMMON_GUARDIAN = 42;
    TELEPORT_UNITS_FACE_CASTER = 43;
    SKILL_STEP = 44;
    ADD_HONOR = 45;
    SPAWN = 46;
    TRADE_SKILL = 47;
    STEALTH = 48;
    DETECT = 49;
    TRANS_DOOR = 50;
    FORCE_CRITICAL_HIT = 51;
    GUARANTEE_HIT = 52;
    ENCHANT_ITEM = 53;
    ENCHANT_ITEM_TEMPORARY = 54;
    TAMECREATURE = 55;
    SUMMON_PET = 56;
    LEARN_PET_SPELL = 57;
    WEAPON_DAMAGE = 58;
    OPEN_LOCK_ITEM = 59;
    PROFICIENCY = 60;
    SEND_EVENT = 61;
    POWER_BURN = 62;
    THREAT = 63;
    TRIGGER_SPELL = 64;
    HEALTH_FUNNEL = 65;
    POWER_FUNNEL = 66;
    HEAL_MAX_HEALTH = 67;
    INTERRUPT_CAST = 68;
    DISTRACT = 69;
    PULL = 70;
    PICKPOCKET = 71;
    ADD_FARSIGHT = 72;
    SUMMON_POSSESSED = 73;
    SUMMON_TOTEM = 74;
    HEAL_MECHANICAL = 75;
    SUMMON_OBJECT_WILD = 76;
    SCRIPT_EFFECT = 77;
    ATTACK = 78;
    SANCTUARY = 79;
    ADD_COMBO_POINTS = 80;
    CREATE_HOUSE = 81;
    BIND_SIGHT = 82;
    DUEL = 83;
    STUCK = 84;
    SUMMON_PLAYER = 85;
    ACTIVATE_OBJECT = 86;
    SUMMON_TOTEM_SLOT1 = 87;
    SUMMON_TOTEM_SLOT2 = 88;
    SUMMON_TOTEM_SLOT3 = 89;
    SUMMON_TOTEM_SLOT4 = 90;
    THREAT_ALL = 91;
    ENCHANT_HELD_ITEM = 92;
    SUMMON_PHANTASM = 93;
    SELF_RESURRECT = 94;
    SKINNING = 95;
    CHARGE = 96;
    SUMMON_CRITTER = 97;
    KNOCK_BACK = 98;
    DISENCHANT = 99;
    INEBRIATE = 100;
    FEED_PET = 101;
    DISMISS_PET = 102;
    REPUTATION = 103;
    SUMMON_OBJECT_SLOT1 = 104;
    SUMMON_OBJECT_SLOT2 = 105;
    SUMMON_OBJECT_SLOT3 = 106;
    SUMMON_OBJECT_SLOT4 = 107;
    DISPEL_MECHANIC = 108;
    SUMMON_DEAD_PET = 109;
    DESTROY_ALL_TOTEMS = 110;
    DURABILITY_DAMAGE = 111;
    SUMMON_DEMON = 112;
    RESURRECT_NEW = 113;
    ATTACK_ME = 114;
    DURABILITY_DAMAGE_PCT = 115;
    SKIN_PLAYER_CORPSE = 116;
    SPIRIT_HEAL = 117;
    SKILL = 118;
    APPLY_AREA_AURA_PET = 119;
    TELEPORT_GRAVEYARD = 120;
    NORMALIZED_WEAPON_DMG = 121;
    UNKNOWN122 = 122;
    SEND_TAXI = 123;
    PLAYER_PULL = 124;
    MODIFY_THREAT_PERCENT = 125;
    UNKNOWN126 = 126;
    UNKNOWN127 = 127;
}
```
### Type
The basic type is `u32`, a 4 byte (32 bit) little endian integer.
### Enumerators
| Enumerator | Value  | Comment |
| --------- | -------- | ------- |
| `NONE` | 0 (0x00) |  |
| `INSTAKILL` | 1 (0x01) |  |
| `SCHOOL_DAMAGE` | 2 (0x02) |  |
| `DUMMY` | 3 (0x03) |  |
| `PORTAL_TELEPORT` | 4 (0x04) |  |
| `TELEPORT_UNITS` | 5 (0x05) |  |
| `APPLY_AURA` | 6 (0x06) |  |
| `ENVIRONMENTAL_DAMAGE` | 7 (0x07) |  |
| `POWER_DRAIN` | 8 (0x08) |  |
| `HEALTH_LEECH` | 9 (0x09) |  |
| `HEAL` | 10 (0x0A) |  |
| `BIND` | 11 (0x0B) |  |
| `PORTAL` | 12 (0x0C) |  |
| `RITUAL_BASE` | 13 (0x0D) |  |
| `RITUAL_SPECIALIZE` | 14 (0x0E) |  |
| `RITUAL_ACTIVATE_PORTAL` | 15 (0x0F) |  |
| `QUEST_COMPLETE` | 16 (0x10) |  |
| `WEAPON_DAMAGE_NOSCHOOL` | 17 (0x11) |  |
| `RESURRECT` | 18 (0x12) |  |
| `ADD_EXTRA_ATTACKS` | 19 (0x13) |  |
| `DODGE` | 20 (0x14) |  |
| `EVADE` | 21 (0x15) |  |
| `PARRY` | 22 (0x16) |  |
| `BLOCK` | 23 (0x17) |  |
| `CREATE_ITEM` | 24 (0x18) |  |
| `WEAPON` | 25 (0x19) |  |
| `DEFENSE` | 26 (0x1A) |  |
| `PERSISTENT_AREA_AURA` | 27 (0x1B) |  |
| `SUMMON` | 28 (0x1C) |  |
| `LEAP` | 29 (0x1D) |  |
| `ENERGIZE` | 30 (0x1E) |  |
| `WEAPON_PERCENT_DAMAGE` | 31 (0x1F) |  |
| `TRIGGER_MISSILE` | 32 (0x20) |  |
| `OPEN_LOCK` | 33 (0x21) |  |
| `SUMMON_CHANGE_ITEM` | 34 (0x22) |  |
| `APPLY_AREA_AURA_PARTY` | 35 (0x23) |  |
| `LEARN_SPELL` | 36 (0x24) |  |
| `SPELL_DEFENSE` | 37 (0x25) |  |
| `DISPEL` | 38 (0x26) |  |
| `LANGUAGE` | 39 (0x27) |  |
| `DUAL_WIELD` | 40 (0x28) |  |
| `SUMMON_WILD` | 41 (0x29) |  |
| `SUMMON_GUARDIAN` | 42 (0x2A) |  |
| `TELEPORT_UNITS_FACE_CASTER` | 43 (0x2B) |  |
| `SKILL_STEP` | 44 (0x2C) |  |
| `ADD_HONOR` | 45 (0x2D) |  |
| `SPAWN` | 46 (0x2E) |  |
| `TRADE_SKILL` | 47 (0x2F) |  |
| `STEALTH` | 48 (0x30) |  |
| `DETECT` | 49 (0x31) |  |
| `TRANS_DOOR` | 50 (0x32) |  |
| `FORCE_CRITICAL_HIT` | 51 (0x33) |  |
| `GUARANTEE_HIT` | 52 (0x34) |  |
| `ENCHANT_ITEM` | 53 (0x35) |  |
| `ENCHANT_ITEM_TEMPORARY` | 54 (0x36) |  |
| `TAMECREATURE` | 55 (0x37) |  |
| `SUMMON_PET` | 56 (0x38) |  |
| `LEARN_PET_SPELL` | 57 (0x39) |  |
| `WEAPON_DAMAGE` | 58 (0x3A) |  |
| `OPEN_LOCK_ITEM` | 59 (0x3B) |  |
| `PROFICIENCY` | 60 (0x3C) |  |
| `SEND_EVENT` | 61 (0x3D) |  |
| `POWER_BURN` | 62 (0x3E) |  |
| `THREAT` | 63 (0x3F) |  |
| `TRIGGER_SPELL` | 64 (0x40) |  |
| `HEALTH_FUNNEL` | 65 (0x41) |  |
| `POWER_FUNNEL` | 66 (0x42) |  |
| `HEAL_MAX_HEALTH` | 67 (0x43) |  |
| `INTERRUPT_CAST` | 68 (0x44) |  |
| `DISTRACT` | 69 (0x45) |  |
| `PULL` | 70 (0x46) |  |
| `PICKPOCKET` | 71 (0x47) |  |
| `ADD_FARSIGHT` | 72 (0x48) |  |
| `SUMMON_POSSESSED` | 73 (0x49) |  |
| `SUMMON_TOTEM` | 74 (0x4A) |  |
| `HEAL_MECHANICAL` | 75 (0x4B) |  |
| `SUMMON_OBJECT_WILD` | 76 (0x4C) |  |
| `SCRIPT_EFFECT` | 77 (0x4D) |  |
| `ATTACK` | 78 (0x4E) |  |
| `SANCTUARY` | 79 (0x4F) |  |
| `ADD_COMBO_POINTS` | 80 (0x50) |  |
| `CREATE_HOUSE` | 81 (0x51) |  |
| `BIND_SIGHT` | 82 (0x52) |  |
| `DUEL` | 83 (0x53) |  |
| `STUCK` | 84 (0x54) |  |
| `SUMMON_PLAYER` | 85 (0x55) |  |
| `ACTIVATE_OBJECT` | 86 (0x56) |  |
| `SUMMON_TOTEM_SLOT1` | 87 (0x57) |  |
| `SUMMON_TOTEM_SLOT2` | 88 (0x58) |  |
| `SUMMON_TOTEM_SLOT3` | 89 (0x59) |  |
| `SUMMON_TOTEM_SLOT4` | 90 (0x5A) |  |
| `THREAT_ALL` | 91 (0x5B) |  |
| `ENCHANT_HELD_ITEM` | 92 (0x5C) |  |
| `SUMMON_PHANTASM` | 93 (0x5D) |  |
| `SELF_RESURRECT` | 94 (0x5E) |  |
| `SKINNING` | 95 (0x5F) |  |
| `CHARGE` | 96 (0x60) |  |
| `SUMMON_CRITTER` | 97 (0x61) |  |
| `KNOCK_BACK` | 98 (0x62) |  |
| `DISENCHANT` | 99 (0x63) |  |
| `INEBRIATE` | 100 (0x64) |  |
| `FEED_PET` | 101 (0x65) |  |
| `DISMISS_PET` | 102 (0x66) |  |
| `REPUTATION` | 103 (0x67) |  |
| `SUMMON_OBJECT_SLOT1` | 104 (0x68) |  |
| `SUMMON_OBJECT_SLOT2` | 105 (0x69) |  |
| `SUMMON_OBJECT_SLOT3` | 106 (0x6A) |  |
| `SUMMON_OBJECT_SLOT4` | 107 (0x6B) |  |
| `DISPEL_MECHANIC` | 108 (0x6C) |  |
| `SUMMON_DEAD_PET` | 109 (0x6D) |  |
| `DESTROY_ALL_TOTEMS` | 110 (0x6E) |  |
| `DURABILITY_DAMAGE` | 111 (0x6F) |  |
| `SUMMON_DEMON` | 112 (0x70) |  |
| `RESURRECT_NEW` | 113 (0x71) |  |
| `ATTACK_ME` | 114 (0x72) |  |
| `DURABILITY_DAMAGE_PCT` | 115 (0x73) |  |
| `SKIN_PLAYER_CORPSE` | 116 (0x74) |  |
| `SPIRIT_HEAL` | 117 (0x75) |  |
| `SKILL` | 118 (0x76) |  |
| `APPLY_AREA_AURA_PET` | 119 (0x77) |  |
| `TELEPORT_GRAVEYARD` | 120 (0x78) |  |
| `NORMALIZED_WEAPON_DMG` | 121 (0x79) |  |
| `UNKNOWN122` | 122 (0x7A) |  |
| `SEND_TAXI` | 123 (0x7B) |  |
| `PLAYER_PULL` | 124 (0x7C) |  |
| `MODIFY_THREAT_PERCENT` | 125 (0x7D) |  |
| `UNKNOWN126` | 126 (0x7E) |  |
| `UNKNOWN127` | 127 (0x7F) |  |

Used in:
* [SpellLog](spelllog.md)

## Client Version 2.4.3

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/spell/smsg_spelllogexecute.wowm:215`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/spell/smsg_spelllogexecute.wowm#L215).

```rust,ignore
enum SpellEffect : u32 {
    NONE = 0;
    INSTAKILL = 1;
    SCHOOL_DAMAGE = 2;
    DUMMY = 3;
    PORTAL_TELEPORT = 4;
    TELEPORT_UNITS = 5;
    APPLY_AURA = 6;
    ENVIRONMENTAL_DAMAGE = 7;
    POWER_DRAIN = 8;
    HEALTH_LEECH = 9;
    HEAL = 10;
    BIND = 11;
    PORTAL = 12;
    RITUAL_BASE = 13;
    RITUAL_SPECIALIZE = 14;
    RITUAL_ACTIVATE_PORTAL = 15;
    QUEST_COMPLETE = 16;
    WEAPON_DAMAGE_NOSCHOOL = 17;
    RESURRECT = 18;
    ADD_EXTRA_ATTACKS = 19;
    DODGE = 20;
    EVADE = 21;
    PARRY = 22;
    BLOCK = 23;
    CREATE_ITEM = 24;
    WEAPON = 25;
    DEFENSE = 26;
    PERSISTENT_AREA_AURA = 27;
    SUMMON = 28;
    LEAP = 29;
    ENERGIZE = 30;
    WEAPON_PERCENT_DAMAGE = 31;
    TRIGGER_MISSILE = 32;
    OPEN_LOCK = 33;
    SUMMON_CHANGE_ITEM = 34;
    APPLY_AREA_AURA_PARTY = 35;
    LEARN_SPELL = 36;
    SPELL_DEFENSE = 37;
    DISPEL = 38;
    LANGUAGE = 39;
    DUAL_WIELD = 40;
    UNKNOWN41 = 41;
    UNKNOWN42 = 42;
    TELEPORT_UNITS_FACE_CASTER = 43;
    SKILL_STEP = 44;
    UNDEFINED_45 = 45;
    SPAWN = 46;
    TRADE_SKILL = 47;
    STEALTH = 48;
    DETECT = 49;
    TRANS_DOOR = 50;
    FORCE_CRITICAL_HIT = 51;
    GUARANTEE_HIT = 52;
    ENCHANT_ITEM = 53;
    ENCHANT_ITEM_TEMPORARY = 54;
    TAMECREATURE = 55;
    SUMMON_PET = 56;
    LEARN_PET_SPELL = 57;
    WEAPON_DAMAGE = 58;
    OPEN_LOCK_ITEM = 59;
    PROFICIENCY = 60;
    SEND_EVENT = 61;
    POWER_BURN = 62;
    THREAT = 63;
    TRIGGER_SPELL = 64;
    HEALTH_FUNNEL = 65;
    POWER_FUNNEL = 66;
    HEAL_MAX_HEALTH = 67;
    INTERRUPT_CAST = 68;
    DISTRACT = 69;
    PULL = 70;
    PICKPOCKET = 71;
    ADD_FARSIGHT = 72;
    UNKNOWN73 = 73;
    UNKNOWN74 = 74;
    HEAL_MECHANICAL = 75;
    SUMMON_OBJECT_WILD = 76;
    SCRIPT_EFFECT = 77;
    ATTACK = 78;
    SANCTUARY = 79;
    ADD_COMBO_POINTS = 80;
    CREATE_HOUSE = 81;
    BIND_SIGHT = 82;
    DUEL = 83;
    STUCK = 84;
    SUMMON_PLAYER = 85;
    ACTIVATE_OBJECT = 86;
    UNKNOWN87 = 87;
    UNKNOWN88 = 88;
    UNKNOWN89 = 89;
    UNKNOWN90 = 90;
    THREAT_ALL = 91;
    ENCHANT_HELD_ITEM = 92;
    UNKNOWN93 = 93;
    SELF_RESURRECT = 94;
    SKINNING = 95;
    CHARGE = 96;
    UNKNOWN97 = 97;
    KNOCK_BACK = 98;
    DISENCHANT = 99;
    INEBRIATE = 100;
    FEED_PET = 101;
    DISMISS_PET = 102;
    REPUTATION = 103;
    SUMMON_OBJECT_SLOT1 = 104;
    SUMMON_OBJECT_SLOT2 = 105;
    SUMMON_OBJECT_SLOT3 = 106;
    SUMMON_OBJECT_SLOT4 = 107;
    DISPEL_MECHANIC = 108;
    SUMMON_DEAD_PET = 109;
    DESTROY_ALL_TOTEMS = 110;
    DURABILITY_DAMAGE = 111;
    UNKNOWN112 = 112;
    RESURRECT_NEW = 113;
    ATTACK_ME = 114;
    DURABILITY_DAMAGE_PCT = 115;
    SKIN_PLAYER_CORPSE = 116;
    SPIRIT_HEAL = 117;
    SKILL = 118;
    APPLY_AREA_AURA_PET = 119;
    TELEPORT_GRAVEYARD = 120;
    NORMALIZED_WEAPON_DMG = 121;
    UNKNOWN122 = 122;
    SEND_TAXI = 123;
    PLAYER_PULL = 124;
    MODIFY_THREAT_PERCENT = 125;
    STEAL_BENEFICIAL_BUFF = 126;
    PROSPECTING = 127;
    APPLY_AREA_AURA_FRIEND = 128;
    APPLY_AREA_AURA_ENEMY = 129;
    REDIRECT_THREAT = 130;
    PLAY_SOUND = 131;
    PLAY_MUSIC = 132;
    UNLEARN_SPECIALIZATION = 133;
    KILL_CREDIT_GROUP = 134;
    CALL_PET = 135;
    HEAL_PCT = 136;
    ENERGIZE_PCT = 137;
    LEAP_BACK = 138;
    CLEAR_QUEST = 139;
    FORCE_CAST = 140;
    UNKNOWN141 = 141;
    TRIGGER_SPELL_WITH_VALUE = 142;
    APPLY_AREA_AURA_OWNER = 143;
    KNOCKBACK_FROM_POSITION = 144;
    UNKNOWN145 = 145;
    UNKNOWN146 = 146;
    QUEST_FAIL = 147;
    UNKNOWN148 = 148;
    CHARGE2 = 149;
    UNKNOWN150 = 150;
    TRIGGER_SPELL_2 = 151;
    UNKNOWN152 = 152;
    UNKNOWN153 = 153;
}
```
### Type
The basic type is `u32`, a 4 byte (32 bit) little endian integer.
### Enumerators
| Enumerator | Value  | Comment |
| --------- | -------- | ------- |
| `NONE` | 0 (0x00) |  |
| `INSTAKILL` | 1 (0x01) |  |
| `SCHOOL_DAMAGE` | 2 (0x02) |  |
| `DUMMY` | 3 (0x03) |  |
| `PORTAL_TELEPORT` | 4 (0x04) |  |
| `TELEPORT_UNITS` | 5 (0x05) |  |
| `APPLY_AURA` | 6 (0x06) |  |
| `ENVIRONMENTAL_DAMAGE` | 7 (0x07) |  |
| `POWER_DRAIN` | 8 (0x08) |  |
| `HEALTH_LEECH` | 9 (0x09) |  |
| `HEAL` | 10 (0x0A) |  |
| `BIND` | 11 (0x0B) |  |
| `PORTAL` | 12 (0x0C) |  |
| `RITUAL_BASE` | 13 (0x0D) |  |
| `RITUAL_SPECIALIZE` | 14 (0x0E) |  |
| `RITUAL_ACTIVATE_PORTAL` | 15 (0x0F) |  |
| `QUEST_COMPLETE` | 16 (0x10) |  |
| `WEAPON_DAMAGE_NOSCHOOL` | 17 (0x11) |  |
| `RESURRECT` | 18 (0x12) |  |
| `ADD_EXTRA_ATTACKS` | 19 (0x13) |  |
| `DODGE` | 20 (0x14) |  |
| `EVADE` | 21 (0x15) |  |
| `PARRY` | 22 (0x16) |  |
| `BLOCK` | 23 (0x17) |  |
| `CREATE_ITEM` | 24 (0x18) |  |
| `WEAPON` | 25 (0x19) |  |
| `DEFENSE` | 26 (0x1A) |  |
| `PERSISTENT_AREA_AURA` | 27 (0x1B) |  |
| `SUMMON` | 28 (0x1C) |  |
| `LEAP` | 29 (0x1D) |  |
| `ENERGIZE` | 30 (0x1E) |  |
| `WEAPON_PERCENT_DAMAGE` | 31 (0x1F) |  |
| `TRIGGER_MISSILE` | 32 (0x20) |  |
| `OPEN_LOCK` | 33 (0x21) |  |
| `SUMMON_CHANGE_ITEM` | 34 (0x22) |  |
| `APPLY_AREA_AURA_PARTY` | 35 (0x23) |  |
| `LEARN_SPELL` | 36 (0x24) |  |
| `SPELL_DEFENSE` | 37 (0x25) |  |
| `DISPEL` | 38 (0x26) |  |
| `LANGUAGE` | 39 (0x27) |  |
| `DUAL_WIELD` | 40 (0x28) |  |
| `UNKNOWN41` | 41 (0x29) | old SPELL_EFFECT_SUMMON_WILD |
| `UNKNOWN42` | 42 (0x2A) | old SPELL_EFFECT_SUMMON_GUARDIAN |
| `TELEPORT_UNITS_FACE_CASTER` | 43 (0x2B) |  |
| `SKILL_STEP` | 44 (0x2C) |  |
| `UNDEFINED_45` | 45 (0x2D) |  |
| `SPAWN` | 46 (0x2E) |  |
| `TRADE_SKILL` | 47 (0x2F) |  |
| `STEALTH` | 48 (0x30) |  |
| `DETECT` | 49 (0x31) |  |
| `TRANS_DOOR` | 50 (0x32) |  |
| `FORCE_CRITICAL_HIT` | 51 (0x33) |  |
| `GUARANTEE_HIT` | 52 (0x34) |  |
| `ENCHANT_ITEM` | 53 (0x35) |  |
| `ENCHANT_ITEM_TEMPORARY` | 54 (0x36) |  |
| `TAMECREATURE` | 55 (0x37) |  |
| `SUMMON_PET` | 56 (0x38) |  |
| `LEARN_PET_SPELL` | 57 (0x39) |  |
| `WEAPON_DAMAGE` | 58 (0x3A) |  |
| `OPEN_LOCK_ITEM` | 59 (0x3B) |  |
| `PROFICIENCY` | 60 (0x3C) |  |
| `SEND_EVENT` | 61 (0x3D) |  |
| `POWER_BURN` | 62 (0x3E) |  |
| `THREAT` | 63 (0x3F) |  |
| `TRIGGER_SPELL` | 64 (0x40) |  |
| `HEALTH_FUNNEL` | 65 (0x41) |  |
| `POWER_FUNNEL` | 66 (0x42) |  |
| `HEAL_MAX_HEALTH` | 67 (0x43) |  |
| `INTERRUPT_CAST` | 68 (0x44) |  |
| `DISTRACT` | 69 (0x45) |  |
| `PULL` | 70 (0x46) |  |
| `PICKPOCKET` | 71 (0x47) |  |
| `ADD_FARSIGHT` | 72 (0x48) |  |
| `UNKNOWN73` | 73 (0x49) | old SPELL_EFFECT_SUMMON_POSSESSED |
| `UNKNOWN74` | 74 (0x4A) | old SPELL_EFFECT_SUMMON_TOTEM |
| `HEAL_MECHANICAL` | 75 (0x4B) |  |
| `SUMMON_OBJECT_WILD` | 76 (0x4C) |  |
| `SCRIPT_EFFECT` | 77 (0x4D) |  |
| `ATTACK` | 78 (0x4E) |  |
| `SANCTUARY` | 79 (0x4F) |  |
| `ADD_COMBO_POINTS` | 80 (0x50) |  |
| `CREATE_HOUSE` | 81 (0x51) |  |
| `BIND_SIGHT` | 82 (0x52) |  |
| `DUEL` | 83 (0x53) |  |
| `STUCK` | 84 (0x54) |  |
| `SUMMON_PLAYER` | 85 (0x55) |  |
| `ACTIVATE_OBJECT` | 86 (0x56) |  |
| `UNKNOWN87` | 87 (0x57) | old SPELL_EFFECT_SUMMON_TOTEM_SLOT1 |
| `UNKNOWN88` | 88 (0x58) | old SPELL_EFFECT_SUMMON_TOTEM_SLOT2 |
| `UNKNOWN89` | 89 (0x59) | old SPELL_EFFECT_SUMMON_TOTEM_SLOT3 |
| `UNKNOWN90` | 90 (0x5A) | old SPELL_EFFECT_SUMMON_TOTEM_SLOT4 |
| `THREAT_ALL` | 91 (0x5B) |  |
| `ENCHANT_HELD_ITEM` | 92 (0x5C) |  |
| `UNKNOWN93` | 93 (0x5D) | old SPELL_EFFECT_SUMMON_PHANTASM |
| `SELF_RESURRECT` | 94 (0x5E) |  |
| `SKINNING` | 95 (0x5F) |  |
| `CHARGE` | 96 (0x60) |  |
| `UNKNOWN97` | 97 (0x61) | old SPELL_EFFECT_SUMMON_CRITTER |
| `KNOCK_BACK` | 98 (0x62) |  |
| `DISENCHANT` | 99 (0x63) |  |
| `INEBRIATE` | 100 (0x64) |  |
| `FEED_PET` | 101 (0x65) |  |
| `DISMISS_PET` | 102 (0x66) |  |
| `REPUTATION` | 103 (0x67) |  |
| `SUMMON_OBJECT_SLOT1` | 104 (0x68) |  |
| `SUMMON_OBJECT_SLOT2` | 105 (0x69) |  |
| `SUMMON_OBJECT_SLOT3` | 106 (0x6A) |  |
| `SUMMON_OBJECT_SLOT4` | 107 (0x6B) |  |
| `DISPEL_MECHANIC` | 108 (0x6C) |  |
| `SUMMON_DEAD_PET` | 109 (0x6D) |  |
| `DESTROY_ALL_TOTEMS` | 110 (0x6E) |  |
| `DURABILITY_DAMAGE` | 111 (0x6F) |  |
| `UNKNOWN112` | 112 (0x70) | old SPELL_EFFECT_SUMMON_DEMON |
| `RESURRECT_NEW` | 113 (0x71) |  |
| `ATTACK_ME` | 114 (0x72) |  |
| `DURABILITY_DAMAGE_PCT` | 115 (0x73) |  |
| `SKIN_PLAYER_CORPSE` | 116 (0x74) |  |
| `SPIRIT_HEAL` | 117 (0x75) |  |
| `SKILL` | 118 (0x76) |  |
| `APPLY_AREA_AURA_PET` | 119 (0x77) |  |
| `TELEPORT_GRAVEYARD` | 120 (0x78) |  |
| `NORMALIZED_WEAPON_DMG` | 121 (0x79) |  |
| `UNKNOWN122` | 122 (0x7A) |  |
| `SEND_TAXI` | 123 (0x7B) |  |
| `PLAYER_PULL` | 124 (0x7C) |  |
| `MODIFY_THREAT_PERCENT` | 125 (0x7D) |  |
| `STEAL_BENEFICIAL_BUFF` | 126 (0x7E) |  |
| `PROSPECTING` | 127 (0x7F) |  |
| `APPLY_AREA_AURA_FRIEND` | 128 (0x80) |  |
| `APPLY_AREA_AURA_ENEMY` | 129 (0x81) |  |
| `REDIRECT_THREAT` | 130 (0x82) |  |
| `PLAY_SOUND` | 131 (0x83) |  |
| `PLAY_MUSIC` | 132 (0x84) |  |
| `UNLEARN_SPECIALIZATION` | 133 (0x85) |  |
| `KILL_CREDIT_GROUP` | 134 (0x86) |  |
| `CALL_PET` | 135 (0x87) |  |
| `HEAL_PCT` | 136 (0x88) |  |
| `ENERGIZE_PCT` | 137 (0x89) |  |
| `LEAP_BACK` | 138 (0x8A) |  |
| `CLEAR_QUEST` | 139 (0x8B) |  |
| `FORCE_CAST` | 140 (0x8C) |  |
| `UNKNOWN141` | 141 (0x8D) |  |
| `TRIGGER_SPELL_WITH_VALUE` | 142 (0x8E) |  |
| `APPLY_AREA_AURA_OWNER` | 143 (0x8F) |  |
| `KNOCKBACK_FROM_POSITION` | 144 (0x90) |  |
| `UNKNOWN145` | 145 (0x91) |  |
| `UNKNOWN146` | 146 (0x92) |  |
| `QUEST_FAIL` | 147 (0x93) |  |
| `UNKNOWN148` | 148 (0x94) |  |
| `CHARGE2` | 149 (0x95) |  |
| `UNKNOWN150` | 150 (0x96) |  |
| `TRIGGER_SPELL_2` | 151 (0x97) |  |
| `UNKNOWN152` | 152 (0x98) |  |
| `UNKNOWN153` | 153 (0x99) |  |

Used in:
* [SpellLog](spelllog.md)

## Client Version 3.3.5

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/spell/smsg_spelllogexecute.wowm:431`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/spell/smsg_spelllogexecute.wowm#L431).

```rust,ignore
enum SpellEffect : u32 {
    NONE = 0;
    INSTAKILL = 1;
    SCHOOL_DAMAGE = 2;
    DUMMY = 3;
    PORTAL_TELEPORT = 4;
    TELEPORT_UNITS = 5;
    APPLY_AURA = 6;
    ENVIRONMENTAL_DAMAGE = 7;
    POWER_DRAIN = 8;
    HEALTH_LEECH = 9;
    HEAL = 10;
    BIND = 11;
    PORTAL = 12;
    RITUAL_BASE = 13;
    RITUAL_SPECIALIZE = 14;
    RITUAL_ACTIVATE_PORTAL = 15;
    QUEST_COMPLETE = 16;
    WEAPON_DAMAGE_NOSCHOOL = 17;
    RESURRECT = 18;
    ADD_EXTRA_ATTACKS = 19;
    DODGE = 20;
    EVADE = 21;
    PARRY = 22;
    BLOCK = 23;
    CREATE_ITEM = 24;
    WEAPON = 25;
    DEFENSE = 26;
    PERSISTENT_AREA_AURA = 27;
    SUMMON = 28;
    LEAP = 29;
    ENERGIZE = 30;
    WEAPON_PERCENT_DAMAGE = 31;
    TRIGGER_MISSILE = 32;
    OPEN_LOCK = 33;
    SUMMON_CHANGE_ITEM = 34;
    APPLY_AREA_AURA_PARTY = 35;
    LEARN_SPELL = 36;
    SPELL_DEFENSE = 37;
    DISPEL = 38;
    LANGUAGE = 39;
    DUAL_WIELD = 40;
    JUMP = 41;
    JUMP2 = 42;
    TELEPORT_UNITS_FACE_CASTER = 43;
    SKILL_STEP = 44;
    ADD_HONOR = 45;
    SPAWN = 46;
    TRADE_SKILL = 47;
    STEALTH = 48;
    DETECT = 49;
    TRANS_DOOR = 50;
    FORCE_CRITICAL_HIT = 51;
    GUARANTEE_HIT = 52;
    ENCHANT_ITEM = 53;
    ENCHANT_ITEM_TEMPORARY = 54;
    TAMECREATURE = 55;
    SUMMON_PET = 56;
    LEARN_PET_SPELL = 57;
    WEAPON_DAMAGE = 58;
    OPEN_LOCK_ITEM = 59;
    PROFICIENCY = 60;
    SEND_EVENT = 61;
    POWER_BURN = 62;
    THREAT = 63;
    TRIGGER_SPELL = 64;
    APPLY_AREA_AURA_RAID = 65;
    RESTORE_ITEM_CHARGES = 66;
    HEAL_MAX_HEALTH = 67;
    INTERRUPT_CAST = 68;
    DISTRACT = 69;
    PULL = 70;
    PICKPOCKET = 71;
    ADD_FARSIGHT = 72;
    UNTRAIN_TALENTS = 73;
    APPLY_GLYPH = 74;
    HEAL_MECHANICAL = 75;
    SUMMON_OBJECT_WILD = 76;
    SCRIPT_EFFECT = 77;
    ATTACK = 78;
    SANCTUARY = 79;
    ADD_COMBO_POINTS = 80;
    CREATE_HOUSE = 81;
    BIND_SIGHT = 82;
    DUEL = 83;
    STUCK = 84;
    SUMMON_PLAYER = 85;
    ACTIVATE_OBJECT = 86;
    WMO_DAMAGE = 87;
    WMO_REPAIR = 88;
    WMO_CHANGE = 89;
    KILL_CREDIT_PERSONAL = 90;
    THREAT_ALL = 91;
    ENCHANT_HELD_ITEM = 92;
    BREAK_PLAYER_TARGETING = 93;
    SELF_RESURRECT = 94;
    SKINNING = 95;
    CHARGE = 96;
    SUMMON_ALL_TOTEMS = 97;
    KNOCK_BACK = 98;
    DISENCHANT = 99;
    INEBRIATE = 100;
    FEED_PET = 101;
    DISMISS_PET = 102;
    REPUTATION = 103;
    SUMMON_OBJECT_SLOT1 = 104;
    SUMMON_OBJECT_SLOT2 = 105;
    SUMMON_OBJECT_SLOT3 = 106;
    SUMMON_OBJECT_SLOT4 = 107;
    DISPEL_MECHANIC = 108;
    SUMMON_DEAD_PET = 109;
    DESTROY_ALL_TOTEMS = 110;
    DURABILITY_DAMAGE = 111;
    UNKNOWN112 = 112;
    RESURRECT_NEW = 113;
    ATTACK_ME = 114;
    DURABILITY_DAMAGE_PCT = 115;
    SKIN_PLAYER_CORPSE = 116;
    SPIRIT_HEAL = 117;
    SKILL = 118;
    APPLY_AREA_AURA_PET = 119;
    TELEPORT_GRAVEYARD = 120;
    NORMALIZED_WEAPON_DMG = 121;
    UNKNOWN122 = 122;
    SEND_TAXI = 123;
    PLAYER_PULL = 124;
    MODIFY_THREAT_PERCENT = 125;
    STEAL_BENEFICIAL_BUFF = 126;
    PROSPECTING = 127;
    APPLY_AREA_AURA_FRIEND = 128;
    APPLY_AREA_AURA_ENEMY = 129;
    REDIRECT_THREAT = 130;
    PLAY_SOUND = 131;
    PLAY_MUSIC = 132;
    UNLEARN_SPECIALIZATION = 133;
    KILL_CREDIT_GROUP = 134;
    CALL_PET = 135;
    HEAL_PCT = 136;
    ENERGIZE_PCT = 137;
    LEAP_BACK = 138;
    CLEAR_QUEST = 139;
    FORCE_CAST = 140;
    FORCE_CAST_WITH_VALUE = 141;
    TRIGGER_SPELL_WITH_VALUE = 142;
    APPLY_AREA_AURA_OWNER = 143;
    KNOCKBACK_FROM_POSITION = 144;
    GRAVITY_PULL = 145;
    ACTIVATE_RUNE = 146;
    QUEST_FAIL = 147;
    UNKNOWN148 = 148;
    CHARGE2 = 149;
    QUEST_OFFER = 150;
    TRIGGER_SPELL_2 = 151;
    UNKNOWN152 = 152;
    CREATE_PET = 153;
    TEACH_TAXI_NODE = 154;
    TITAN_GRIP = 155;
    ENCHANT_ITEM_PRISMATIC = 156;
    CREATE_ITEM2 = 157;
    MILLING = 158;
    ALLOW_RENAME_PET = 159;
    UNKNOWN160 = 160;
    TALENT_SPEC_COUNT = 161;
    TALENT_SPEC_SELECT = 162;
    UNKNOWN163 = 163;
    CANCEL_AURA = 164;
}
```
### Type
The basic type is `u32`, a 4 byte (32 bit) little endian integer.
### Enumerators
| Enumerator | Value  | Comment |
| --------- | -------- | ------- |
| `NONE` | 0 (0x00) |  |
| `INSTAKILL` | 1 (0x01) |  |
| `SCHOOL_DAMAGE` | 2 (0x02) |  |
| `DUMMY` | 3 (0x03) |  |
| `PORTAL_TELEPORT` | 4 (0x04) |  |
| `TELEPORT_UNITS` | 5 (0x05) |  |
| `APPLY_AURA` | 6 (0x06) |  |
| `ENVIRONMENTAL_DAMAGE` | 7 (0x07) |  |
| `POWER_DRAIN` | 8 (0x08) |  |
| `HEALTH_LEECH` | 9 (0x09) |  |
| `HEAL` | 10 (0x0A) |  |
| `BIND` | 11 (0x0B) |  |
| `PORTAL` | 12 (0x0C) |  |
| `RITUAL_BASE` | 13 (0x0D) |  |
| `RITUAL_SPECIALIZE` | 14 (0x0E) |  |
| `RITUAL_ACTIVATE_PORTAL` | 15 (0x0F) |  |
| `QUEST_COMPLETE` | 16 (0x10) |  |
| `WEAPON_DAMAGE_NOSCHOOL` | 17 (0x11) |  |
| `RESURRECT` | 18 (0x12) |  |
| `ADD_EXTRA_ATTACKS` | 19 (0x13) |  |
| `DODGE` | 20 (0x14) |  |
| `EVADE` | 21 (0x15) |  |
| `PARRY` | 22 (0x16) |  |
| `BLOCK` | 23 (0x17) |  |
| `CREATE_ITEM` | 24 (0x18) |  |
| `WEAPON` | 25 (0x19) |  |
| `DEFENSE` | 26 (0x1A) |  |
| `PERSISTENT_AREA_AURA` | 27 (0x1B) |  |
| `SUMMON` | 28 (0x1C) |  |
| `LEAP` | 29 (0x1D) |  |
| `ENERGIZE` | 30 (0x1E) |  |
| `WEAPON_PERCENT_DAMAGE` | 31 (0x1F) |  |
| `TRIGGER_MISSILE` | 32 (0x20) |  |
| `OPEN_LOCK` | 33 (0x21) |  |
| `SUMMON_CHANGE_ITEM` | 34 (0x22) |  |
| `APPLY_AREA_AURA_PARTY` | 35 (0x23) |  |
| `LEARN_SPELL` | 36 (0x24) |  |
| `SPELL_DEFENSE` | 37 (0x25) |  |
| `DISPEL` | 38 (0x26) |  |
| `LANGUAGE` | 39 (0x27) |  |
| `DUAL_WIELD` | 40 (0x28) |  |
| `JUMP` | 41 (0x29) |  |
| `JUMP2` | 42 (0x2A) |  |
| `TELEPORT_UNITS_FACE_CASTER` | 43 (0x2B) |  |
| `SKILL_STEP` | 44 (0x2C) |  |
| `ADD_HONOR` | 45 (0x2D) |  |
| `SPAWN` | 46 (0x2E) |  |
| `TRADE_SKILL` | 47 (0x2F) |  |
| `STEALTH` | 48 (0x30) |  |
| `DETECT` | 49 (0x31) |  |
| `TRANS_DOOR` | 50 (0x32) |  |
| `FORCE_CRITICAL_HIT` | 51 (0x33) |  |
| `GUARANTEE_HIT` | 52 (0x34) |  |
| `ENCHANT_ITEM` | 53 (0x35) |  |
| `ENCHANT_ITEM_TEMPORARY` | 54 (0x36) |  |
| `TAMECREATURE` | 55 (0x37) |  |
| `SUMMON_PET` | 56 (0x38) |  |
| `LEARN_PET_SPELL` | 57 (0x39) |  |
| `WEAPON_DAMAGE` | 58 (0x3A) |  |
| `OPEN_LOCK_ITEM` | 59 (0x3B) |  |
| `PROFICIENCY` | 60 (0x3C) |  |
| `SEND_EVENT` | 61 (0x3D) |  |
| `POWER_BURN` | 62 (0x3E) |  |
| `THREAT` | 63 (0x3F) |  |
| `TRIGGER_SPELL` | 64 (0x40) |  |
| `APPLY_AREA_AURA_RAID` | 65 (0x41) |  |
| `RESTORE_ITEM_CHARGES` | 66 (0x42) |  |
| `HEAL_MAX_HEALTH` | 67 (0x43) |  |
| `INTERRUPT_CAST` | 68 (0x44) |  |
| `DISTRACT` | 69 (0x45) |  |
| `PULL` | 70 (0x46) |  |
| `PICKPOCKET` | 71 (0x47) |  |
| `ADD_FARSIGHT` | 72 (0x48) |  |
| `UNTRAIN_TALENTS` | 73 (0x49) |  |
| `APPLY_GLYPH` | 74 (0x4A) |  |
| `HEAL_MECHANICAL` | 75 (0x4B) |  |
| `SUMMON_OBJECT_WILD` | 76 (0x4C) |  |
| `SCRIPT_EFFECT` | 77 (0x4D) |  |
| `ATTACK` | 78 (0x4E) |  |
| `SANCTUARY` | 79 (0x4F) |  |
| `ADD_COMBO_POINTS` | 80 (0x50) |  |
| `CREATE_HOUSE` | 81 (0x51) |  |
| `BIND_SIGHT` | 82 (0x52) |  |
| `DUEL` | 83 (0x53) |  |
| `STUCK` | 84 (0x54) |  |
| `SUMMON_PLAYER` | 85 (0x55) |  |
| `ACTIVATE_OBJECT` | 86 (0x56) |  |
| `WMO_DAMAGE` | 87 (0x57) |  |
| `WMO_REPAIR` | 88 (0x58) |  |
| `WMO_CHANGE` | 89 (0x59) |  |
| `KILL_CREDIT_PERSONAL` | 90 (0x5A) |  |
| `THREAT_ALL` | 91 (0x5B) |  |
| `ENCHANT_HELD_ITEM` | 92 (0x5C) |  |
| `BREAK_PLAYER_TARGETING` | 93 (0x5D) |  |
| `SELF_RESURRECT` | 94 (0x5E) |  |
| `SKINNING` | 95 (0x5F) |  |
| `CHARGE` | 96 (0x60) |  |
| `SUMMON_ALL_TOTEMS` | 97 (0x61) |  |
| `KNOCK_BACK` | 98 (0x62) |  |
| `DISENCHANT` | 99 (0x63) |  |
| `INEBRIATE` | 100 (0x64) |  |
| `FEED_PET` | 101 (0x65) |  |
| `DISMISS_PET` | 102 (0x66) |  |
| `REPUTATION` | 103 (0x67) |  |
| `SUMMON_OBJECT_SLOT1` | 104 (0x68) |  |
| `SUMMON_OBJECT_SLOT2` | 105 (0x69) |  |
| `SUMMON_OBJECT_SLOT3` | 106 (0x6A) |  |
| `SUMMON_OBJECT_SLOT4` | 107 (0x6B) |  |
| `DISPEL_MECHANIC` | 108 (0x6C) |  |
| `SUMMON_DEAD_PET` | 109 (0x6D) |  |
| `DESTROY_ALL_TOTEMS` | 110 (0x6E) |  |
| `DURABILITY_DAMAGE` | 111 (0x6F) |  |
| `UNKNOWN112` | 112 (0x70) | old SPELL_EFFECT_SUMMON_DEMON |
| `RESURRECT_NEW` | 113 (0x71) |  |
| `ATTACK_ME` | 114 (0x72) |  |
| `DURABILITY_DAMAGE_PCT` | 115 (0x73) |  |
| `SKIN_PLAYER_CORPSE` | 116 (0x74) |  |
| `SPIRIT_HEAL` | 117 (0x75) |  |
| `SKILL` | 118 (0x76) |  |
| `APPLY_AREA_AURA_PET` | 119 (0x77) |  |
| `TELEPORT_GRAVEYARD` | 120 (0x78) |  |
| `NORMALIZED_WEAPON_DMG` | 121 (0x79) |  |
| `UNKNOWN122` | 122 (0x7A) |  |
| `SEND_TAXI` | 123 (0x7B) |  |
| `PLAYER_PULL` | 124 (0x7C) |  |
| `MODIFY_THREAT_PERCENT` | 125 (0x7D) |  |
| `STEAL_BENEFICIAL_BUFF` | 126 (0x7E) |  |
| `PROSPECTING` | 127 (0x7F) |  |
| `APPLY_AREA_AURA_FRIEND` | 128 (0x80) |  |
| `APPLY_AREA_AURA_ENEMY` | 129 (0x81) |  |
| `REDIRECT_THREAT` | 130 (0x82) |  |
| `PLAY_SOUND` | 131 (0x83) |  |
| `PLAY_MUSIC` | 132 (0x84) |  |
| `UNLEARN_SPECIALIZATION` | 133 (0x85) |  |
| `KILL_CREDIT_GROUP` | 134 (0x86) |  |
| `CALL_PET` | 135 (0x87) |  |
| `HEAL_PCT` | 136 (0x88) |  |
| `ENERGIZE_PCT` | 137 (0x89) |  |
| `LEAP_BACK` | 138 (0x8A) |  |
| `CLEAR_QUEST` | 139 (0x8B) |  |
| `FORCE_CAST` | 140 (0x8C) |  |
| `FORCE_CAST_WITH_VALUE` | 141 (0x8D) |  |
| `TRIGGER_SPELL_WITH_VALUE` | 142 (0x8E) |  |
| `APPLY_AREA_AURA_OWNER` | 143 (0x8F) |  |
| `KNOCKBACK_FROM_POSITION` | 144 (0x90) |  |
| `GRAVITY_PULL` | 145 (0x91) |  |
| `ACTIVATE_RUNE` | 146 (0x92) |  |
| `QUEST_FAIL` | 147 (0x93) |  |
| `UNKNOWN148` | 148 (0x94) |  |
| `CHARGE2` | 149 (0x95) |  |
| `QUEST_OFFER` | 150 (0x96) |  |
| `TRIGGER_SPELL_2` | 151 (0x97) |  |
| `UNKNOWN152` | 152 (0x98) |  |
| `CREATE_PET` | 153 (0x99) |  |
| `TEACH_TAXI_NODE` | 154 (0x9A) |  |
| `TITAN_GRIP` | 155 (0x9B) |  |
| `ENCHANT_ITEM_PRISMATIC` | 156 (0x9C) |  |
| `CREATE_ITEM2` | 157 (0x9D) |  |
| `MILLING` | 158 (0x9E) |  |
| `ALLOW_RENAME_PET` | 159 (0x9F) |  |
| `UNKNOWN160` | 160 (0xA0) |  |
| `TALENT_SPEC_COUNT` | 161 (0xA1) |  |
| `TALENT_SPEC_SELECT` | 162 (0xA2) |  |
| `UNKNOWN163` | 163 (0xA3) |  |
| `CANCEL_AURA` | 164 (0xA4) |  |

Used in:
* [SpellLog](spelllog.md)
