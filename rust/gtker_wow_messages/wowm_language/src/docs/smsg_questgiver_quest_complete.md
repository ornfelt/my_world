# SMSG_QUESTGIVER_QUEST_COMPLETE

## Client Version 1

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/quest/smsg_questgiver_quest_complete.wowm:1`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/quest/smsg_questgiver_quest_complete.wowm#L1).
```rust,ignore
smsg SMSG_QUESTGIVER_QUEST_COMPLETE = 0x0191 {
    u32 quest_id;
    u32 unknown;
    u32 experience_reward;
    Gold money_reward;
    u32 amount_of_item_rewards;
    QuestItemReward[amount_of_item_rewards] item_rewards;
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
| 0x04 | 4 / Little | u32 | quest_id |  |
| 0x08 | 4 / Little | u32 | unknown | cmangos/vmangos/mangoszero: set to 0x03 |
| 0x0C | 4 / Little | u32 | experience_reward |  |
| 0x10 | 4 / Little | Gold | money_reward |  |
| 0x14 | 4 / Little | u32 | amount_of_item_rewards |  |
| 0x18 | ? / - | [QuestItemReward](questitemreward.md)[amount_of_item_rewards] | item_rewards |  |

## Client Version 2.4.3

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/quest/smsg_questgiver_quest_complete.wowm:15`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/quest/smsg_questgiver_quest_complete.wowm#L15).
```rust,ignore
smsg SMSG_QUESTGIVER_QUEST_COMPLETE = 0x0191 {
    u32 quest_id;
    u32 unknown;
    u32 experience_reward;
    Gold money_reward;
    u32 honor_reward;
    u32 amount_of_item_rewards;
    QuestItemReward[amount_of_item_rewards] item_rewards;
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
| 0x04 | 4 / Little | u32 | quest_id |  |
| 0x08 | 4 / Little | u32 | unknown | cmangos/vmangos/mangoszero: set to 0x03 |
| 0x0C | 4 / Little | u32 | experience_reward |  |
| 0x10 | 4 / Little | Gold | money_reward |  |
| 0x14 | 4 / Little | u32 | honor_reward |  |
| 0x18 | 4 / Little | u32 | amount_of_item_rewards |  |
| 0x1C | ? / - | [QuestItemReward](questitemreward.md)[amount_of_item_rewards] | item_rewards |  |

## Client Version 3.3.5

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/quest/smsg_questgiver_quest_complete.wowm:30`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/quest/smsg_questgiver_quest_complete.wowm#L30).
```rust,ignore
smsg SMSG_QUESTGIVER_QUEST_COMPLETE = 0x0191 {
    u32 quest_id;
    u32 unknown;
    u32 experience_reward;
    Gold money_reward;
    u32 honor_reward;
    u32 talent_reward;
    u32 arena_point_reward;
    u32 amount_of_item_rewards;
    QuestItemReward[amount_of_item_rewards] item_rewards;
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
| - | 4 / Little | u32 | quest_id |  |
| - | 4 / Little | u32 | unknown | cmangos/vmangos/mangoszero: set to 0x03 |
| - | 4 / Little | u32 | experience_reward |  |
| - | 4 / Little | Gold | money_reward |  |
| - | 4 / Little | u32 | honor_reward |  |
| - | 4 / Little | u32 | talent_reward |  |
| - | 4 / Little | u32 | arena_point_reward |  |
| - | 4 / Little | u32 | amount_of_item_rewards |  |
| - | ? / - | [QuestItemReward](questitemreward.md)[amount_of_item_rewards] | item_rewards |  |
