# SMSG_GM_MESSAGECHAT

## Client Version 2.4.3

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/chat/smsg_gm_messagechat.wowm:1`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/chat/smsg_gm_messagechat.wowm#L1).
```rust,ignore
smsg SMSG_GM_MESSAGECHAT = 0x03B2 {
    ChatType chat_type;
    (u32)Language language;
    if (chat_type == MONSTER_SAY
        || chat_type == MONSTER_PARTY
        || chat_type == MONSTER_YELL
        || chat_type == MONSTER_WHISPER
        || chat_type == RAID_BOSS_WHISPER
        || chat_type == RAID_BOSS_EMOTE
        || chat_type == MONSTER_EMOTE) {
        SizedCString sender;
        NamedGuid target1;
        SizedCString message1;
        PlayerChatTag chat_tag1;
    }
    else if (chat_type == BG_SYSTEM_NEUTRAL
        || chat_type == BG_SYSTEM_ALLIANCE
        || chat_type == BG_SYSTEM_HORDE) {
        NamedGuid target2;
        SizedCString message2;
        PlayerChatTag chat_tag2;
    }
    else if (chat_type == CHANNEL) {
        CString channel_name;
        Guid target4;
        SizedCString message3;
        PlayerChatTag chat_tag3;
    }
    else {
        Guid target5;
        SizedCString message4;
        PlayerChatTag chat_tag4;
        SizedCString sender_name;
    }
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
| 0x04 | 1 / - | [ChatType](chattype.md) | chat_type |  |
| 0x05 | 4 / - | [Language](language.md) | language |  |

If chat_type is equal to `MONSTER_SAY` **or** 
is equal to `MONSTER_PARTY` **or** 
is equal to `MONSTER_YELL` **or** 
is equal to `MONSTER_WHISPER` **or** 
is equal to `RAID_BOSS_WHISPER` **or** 
is equal to `RAID_BOSS_EMOTE` **or** 
is equal to `MONSTER_EMOTE`:

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| 0x09 | - / - | SizedCString | sender |  |
| - | - / - | [NamedGuid](../types/packed-guid.md) | target1 |  |
| - | - / - | SizedCString | message1 |  |
| - | 1 / - | [PlayerChatTag](playerchattag.md) | chat_tag1 |  |

Else If chat_type is equal to `BG_SYSTEM_NEUTRAL` **or** 
is equal to `BG_SYSTEM_ALLIANCE` **or** 
is equal to `BG_SYSTEM_HORDE`:

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| - | - / - | [NamedGuid](../types/packed-guid.md) | target2 |  |
| - | - / - | SizedCString | message2 |  |
| - | 1 / - | [PlayerChatTag](playerchattag.md) | chat_tag2 |  |

Else If chat_type is equal to `CHANNEL`:

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| - | - / - | CString | channel_name |  |
| - | 8 / Little | [Guid](../types/packed-guid.md) | target4 |  |
| - | - / - | SizedCString | message3 |  |
| - | 1 / - | [PlayerChatTag](playerchattag.md) | chat_tag3 |  |

Else: 
| - | 8 / Little | [Guid](../types/packed-guid.md) | target5 |  |
| - | - / - | SizedCString | message4 |  |
| - | 1 / - | [PlayerChatTag](playerchattag.md) | chat_tag4 |  |
| - | - / - | SizedCString | sender_name |  |

## Client Version 3.3.5

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/chat/smsg_gm_messagechat.wowm:40`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/chat/smsg_gm_messagechat.wowm#L40).
```rust,ignore
smsg SMSG_GM_MESSAGECHAT = 0x03B3 {
    ChatType chat_type;
    (u32)Language language;
    Guid sender;
    u32 flags;
    if (chat_type == MONSTER_SAY
        || chat_type == MONSTER_PARTY
        || chat_type == MONSTER_YELL
        || chat_type == MONSTER_WHISPER
        || chat_type == RAID_BOSS_WHISPER
        || chat_type == RAID_BOSS_EMOTE
        || chat_type == MONSTER_EMOTE
        || chat_type == BATTLENET) {
        SizedCString sender1;
        NamedGuid target1;
    }
    else if (chat_type == WHISPER_FOREIGN) {
        SizedCString sender2;
        Guid target2;
    }
    else if (chat_type == BG_SYSTEM_NEUTRAL
        || chat_type == BG_SYSTEM_ALLIANCE
        || chat_type == BG_SYSTEM_HORDE) {
        NamedGuid target3;
    }
    else if (chat_type == ACHIEVEMENT
        || chat_type == GUILD_ACHIEVEMENT) {
        Guid target4;
    }
    else if (chat_type == CHANNEL) {
        CString channel_name;
        Guid target5;
    }
    else {
        SizedCString sender_name;
        Guid target6;
    }
    SizedCString message;
    PlayerChatTag chat_tag;
    if (chat_type == ACHIEVEMENT
        || chat_type == GUILD_ACHIEVEMENT) {
        u32 achievement_id;
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
| - | 1 / - | [ChatType](chattype.md) | chat_type |  |
| - | 4 / - | [Language](language.md) | language |  |
| - | 8 / Little | [Guid](../types/packed-guid.md) | sender |  |
| - | 4 / Little | u32 | flags | azerothcore sets to 0. |

If chat_type is equal to `MONSTER_SAY` **or** 
is equal to `MONSTER_PARTY` **or** 
is equal to `MONSTER_YELL` **or** 
is equal to `MONSTER_WHISPER` **or** 
is equal to `RAID_BOSS_WHISPER` **or** 
is equal to `RAID_BOSS_EMOTE` **or** 
is equal to `MONSTER_EMOTE` **or** 
is equal to `BATTLENET`:

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| - | - / - | SizedCString | sender1 |  |
| - | - / - | [NamedGuid](../types/packed-guid.md) | target1 |  |

Else If chat_type is equal to `WHISPER_FOREIGN`:

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| - | - / - | SizedCString | sender2 |  |
| - | 8 / Little | [Guid](../types/packed-guid.md) | target2 |  |

Else If chat_type is equal to `BG_SYSTEM_NEUTRAL` **or** 
is equal to `BG_SYSTEM_ALLIANCE` **or** 
is equal to `BG_SYSTEM_HORDE`:

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| - | - / - | [NamedGuid](../types/packed-guid.md) | target3 |  |

Else If chat_type is equal to `ACHIEVEMENT` **or** 
is equal to `GUILD_ACHIEVEMENT`:

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| - | 8 / Little | [Guid](../types/packed-guid.md) | target4 |  |

Else If chat_type is equal to `CHANNEL`:

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| - | - / - | CString | channel_name |  |
| - | 8 / Little | [Guid](../types/packed-guid.md) | target5 |  |

Else: 
| - | - / - | SizedCString | sender_name |  |
| - | 8 / Little | [Guid](../types/packed-guid.md) | target6 |  |
| - | - / - | SizedCString | message |  |
| - | 1 / - | [PlayerChatTag](playerchattag.md) | chat_tag |  |

If chat_type is equal to `ACHIEVEMENT` **or** 
is equal to `GUILD_ACHIEVEMENT`:

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| - | 4 / Little | u32 | achievement_id |  |
