# QuestGiverReward

## Client Version 3.3.5

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/quest/smsg_questgiver_quest_details.wowm:52`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/quest/smsg_questgiver_quest_details.wowm#L52).
```rust,ignore
struct QuestGiverReward {
    Item item;
    u32 item_count;
    u32 display_id;
}
```
### Body

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| 0x00 | 4 / Little | Item | item |  |
| 0x04 | 4 / Little | u32 | item_count |  |
| 0x08 | 4 / Little | u32 | display_id |  |


Used in:
* [SMSG_LFG_PLAYER_REWARD](smsg_lfg_player_reward.md)
* [SMSG_QUESTGIVER_QUEST_DETAILS](smsg_questgiver_quest_details.md)
