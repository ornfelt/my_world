# SMSG_QUESTGIVER_OFFER_REWARD

## Client Version 1.12

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/quest/smsg_questgiver_offer_reward.wowm:1`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/quest/smsg_questgiver_offer_reward.wowm#L1).
```rust,ignore
smsg SMSG_QUESTGIVER_OFFER_REWARD = 0x018D {
    Guid npc;
    u32 quest_id;
    CString title;
    CString offer_reward_text;
    Bool32 auto_finish;
    u32 amount_of_emotes;
    NpcTextUpdateEmote[amount_of_emotes] emotes;
    u32 amount_of_choice_item_rewards;
    QuestItemRequirement[amount_of_choice_item_rewards] choice_item_rewards;
    u32 amount_of_item_rewards;
    QuestItemRequirement[amount_of_item_rewards] item_rewards;
    Gold money_reward;
    Spell reward_spell;
    Spell reward_spell_cast;
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
| 0x04 | 8 / Little | [Guid](../types/packed-guid.md) | npc |  |
| 0x0C | 4 / Little | u32 | quest_id |  |
| 0x10 | - / - | CString | title |  |
| - | - / - | CString | offer_reward_text |  |
| - | 4 / Little | Bool32 | auto_finish |  |
| - | 4 / Little | u32 | amount_of_emotes |  |
| - | ? / - | [NpcTextUpdateEmote](npctextupdateemote.md)[amount_of_emotes] | emotes |  |
| - | 4 / Little | u32 | amount_of_choice_item_rewards |  |
| - | ? / - | [QuestItemRequirement](questitemrequirement.md)[amount_of_choice_item_rewards] | choice_item_rewards |  |
| - | 4 / Little | u32 | amount_of_item_rewards |  |
| - | ? / - | [QuestItemRequirement](questitemrequirement.md)[amount_of_item_rewards] | item_rewards |  |
| - | 4 / Little | Gold | money_reward |  |
| - | 4 / Little | Spell | reward_spell |  |
| - | 4 / Little | Spell | reward_spell_cast | mangoszero and cmangos disagree about which field is _cast, although they both agree that the _cast field should not be in zero (vanilla). They still both include both fields in the code though. |

## Client Version 2.4.3

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/quest/smsg_questgiver_offer_reward.wowm:23`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/quest/smsg_questgiver_offer_reward.wowm#L23).
```rust,ignore
smsg SMSG_QUESTGIVER_OFFER_REWARD = 0x018D {
    Guid npc;
    u32 quest_id;
    CString title;
    CString offer_reward_text;
    Bool32 auto_finish;
    u32 suggested_players;
    u32 amount_of_emotes;
    NpcTextUpdateEmote[amount_of_emotes] emotes;
    u32 amount_of_choice_item_rewards;
    QuestItemRequirement[amount_of_choice_item_rewards] choice_item_rewards;
    u32 amount_of_item_rewards;
    QuestItemRequirement[amount_of_item_rewards] item_rewards;
    Gold money_reward;
    u32 honor_reward;
    u32 unknown1;
    Spell reward_spell;
    Spell reward_spell_cast;
    u32 title_reward;
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
| 0x04 | 8 / Little | [Guid](../types/packed-guid.md) | npc |  |
| 0x0C | 4 / Little | u32 | quest_id |  |
| 0x10 | - / - | CString | title |  |
| - | - / - | CString | offer_reward_text |  |
| - | 4 / Little | Bool32 | auto_finish |  |
| - | 4 / Little | u32 | suggested_players |  |
| - | 4 / Little | u32 | amount_of_emotes |  |
| - | ? / - | [NpcTextUpdateEmote](npctextupdateemote.md)[amount_of_emotes] | emotes |  |
| - | 4 / Little | u32 | amount_of_choice_item_rewards |  |
| - | ? / - | [QuestItemRequirement](questitemrequirement.md)[amount_of_choice_item_rewards] | choice_item_rewards |  |
| - | 4 / Little | u32 | amount_of_item_rewards |  |
| - | ? / - | [QuestItemRequirement](questitemrequirement.md)[amount_of_item_rewards] | item_rewards |  |
| - | 4 / Little | Gold | money_reward |  |
| - | 4 / Little | u32 | honor_reward |  |
| - | 4 / Little | u32 | unknown1 | mangostwo: unused by client?<br/>mangostwo sets to 0x08. |
| - | 4 / Little | Spell | reward_spell |  |
| - | 4 / Little | Spell | reward_spell_cast | mangoszero and cmangos disagree about which field is _cast, although they both agree that the _cast field should not be in zero (vanilla). They still both include both fields in the code though. |
| - | 4 / Little | u32 | title_reward |  |

## Client Version 3.3.5

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/quest/smsg_questgiver_offer_reward.wowm:51`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/quest/smsg_questgiver_offer_reward.wowm#L51).
```rust,ignore
smsg SMSG_QUESTGIVER_OFFER_REWARD = 0x018D {
    Guid npc;
    u32 quest_id;
    CString title;
    CString offer_reward_text;
    Bool32 auto_finish;
    u32 flags1;
    u32 suggested_players;
    u32 amount_of_emotes;
    NpcTextUpdateEmote[amount_of_emotes] emotes;
    u32 amount_of_choice_item_rewards;
    QuestItemRequirement[amount_of_choice_item_rewards] choice_item_rewards;
    u32 amount_of_item_rewards;
    QuestItemRequirement[amount_of_item_rewards] item_rewards;
    Gold money_reward;
    u32 experience_reward;
    u32 honor_reward;
    f32 honor_reward_multiplier;
    u32 unknown1;
    Spell reward_spell;
    Spell reward_spell_cast;
    u32 title_reward;
    u32 reward_talents;
    u32 reward_arena_points;
    u32 reward_reputation_mask;
    u32[5] reward_factions;
    u32[5] reward_reputations;
    u32[5] reward_reputations_override;
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
| - | 8 / Little | [Guid](../types/packed-guid.md) | npc |  |
| - | 4 / Little | u32 | quest_id |  |
| - | - / - | CString | title |  |
| - | - / - | CString | offer_reward_text |  |
| - | 4 / Little | Bool32 | auto_finish |  |
| - | 4 / Little | u32 | flags1 |  |
| - | 4 / Little | u32 | suggested_players |  |
| - | 4 / Little | u32 | amount_of_emotes |  |
| - | ? / - | [NpcTextUpdateEmote](npctextupdateemote.md)[amount_of_emotes] | emotes |  |
| - | 4 / Little | u32 | amount_of_choice_item_rewards |  |
| - | ? / - | [QuestItemRequirement](questitemrequirement.md)[amount_of_choice_item_rewards] | choice_item_rewards |  |
| - | 4 / Little | u32 | amount_of_item_rewards |  |
| - | ? / - | [QuestItemRequirement](questitemrequirement.md)[amount_of_item_rewards] | item_rewards |  |
| - | 4 / Little | Gold | money_reward |  |
| - | 4 / Little | u32 | experience_reward |  |
| - | 4 / Little | u32 | honor_reward |  |
| - | 4 / Little | f32 | honor_reward_multiplier |  |
| - | 4 / Little | u32 | unknown1 | mangostwo: unused by client?<br/>mangostwo sets to 0x08. |
| - | 4 / Little | Spell | reward_spell |  |
| - | 4 / Little | Spell | reward_spell_cast | mangoszero and cmangos disagree about which field is _cast, although they both agree that the _cast field should not be in zero (vanilla). They still both include both fields in the code though. |
| - | 4 / Little | u32 | title_reward |  |
| - | 4 / Little | u32 | reward_talents |  |
| - | 4 / Little | u32 | reward_arena_points |  |
| - | 4 / Little | u32 | reward_reputation_mask |  |
| - | 20 / - | u32[5] | reward_factions |  |
| - | 20 / - | u32[5] | reward_reputations | mangostwo: columnid in QuestFactionReward.dbc (if negative, from second row) |
| - | 20 / - | u32[5] | reward_reputations_override | mangostwo: reward reputation override. No diplomacy bonus is expected given, reward also does not display in chat window |
