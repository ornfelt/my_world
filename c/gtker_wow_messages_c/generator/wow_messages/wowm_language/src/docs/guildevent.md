# GuildEvent

## Client Version 1

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/guild/smsg_guild_event.wowm:1`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/guild/smsg_guild_event.wowm#L1).

```rust,ignore
enum GuildEvent : u8 {
    PROMOTION = 0;
    DEMOTION = 1;
    MOTD = 2;
    JOINED = 3;
    LEFT = 4;
    REMOVED = 5;
    LEADER_IS = 6;
    LEADER_CHANGED = 7;
    DISBANDED = 8;
    TABARD_CHANGED = 9;
    UNKNOWN10 = 10;
    ROSTER_UPDATE = 11;
    SIGNED_ON = 12;
    SIGNED_OFF = 13;
}
```
### Type
The basic type is `u8`, a 1 byte (8 bit) integer.
### Enumerators
| Enumerator | Value  | Comment |
| --------- | -------- | ------- |
| `PROMOTION` | 0 (0x00) |  |
| `DEMOTION` | 1 (0x01) |  |
| `MOTD` | 2 (0x02) |  |
| `JOINED` | 3 (0x03) |  |
| `LEFT` | 4 (0x04) |  |
| `REMOVED` | 5 (0x05) |  |
| `LEADER_IS` | 6 (0x06) |  |
| `LEADER_CHANGED` | 7 (0x07) |  |
| `DISBANDED` | 8 (0x08) |  |
| `TABARD_CHANGED` | 9 (0x09) |  |
| `UNKNOWN10` | 10 (0x0A) |  |
| `ROSTER_UPDATE` | 11 (0x0B) |  |
| `SIGNED_ON` | 12 (0x0C) |  |
| `SIGNED_OFF` | 13 (0x0D) |  |

Used in:
* [SMSG_GUILD_EVENT](smsg_guild_event.md)

## Client Version 2.4.3, Client Version 3

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/guild/smsg_guild_event.wowm:20`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/guild/smsg_guild_event.wowm#L20).

```rust,ignore
enum GuildEvent : u8 {
    PROMOTION = 0x00;
    DEMOTION = 0x01;
    MOTD = 0x02;
    JOINED = 0x03;
    LEFT = 0x04;
    REMOVED = 0x05;
    LEADER_IS = 0x06;
    LEADER_CHANGED = 0x07;
    DISBANDED = 0x08;
    TABARD_CHANGED = 0x09;
    UNK1 = 0x0A;
    UNK2 = 0x0B;
    SIGNED_ON = 0x0C;
    SIGNED_OFF = 0x0D;
    GUILD_BANK_BAG_SLOTS_CHANGED = 0x0E;
    BANKTAB_PURCHASED = 0x0F;
    UNK5 = 0x10;
    GUILD_BANK_UPDATE_MONEY = 0x11;
    GUILD_BANK_MONEY_WITHDRAWN = 0x12;
    GUILD_BANK_TEXT_CHANGED = 0x13;
}
```
### Type
The basic type is `u8`, a 1 byte (8 bit) integer.
### Enumerators
| Enumerator | Value  | Comment |
| --------- | -------- | ------- |
| `PROMOTION` | 0 (0x00) |  |
| `DEMOTION` | 1 (0x01) |  |
| `MOTD` | 2 (0x02) |  |
| `JOINED` | 3 (0x03) |  |
| `LEFT` | 4 (0x04) |  |
| `REMOVED` | 5 (0x05) |  |
| `LEADER_IS` | 6 (0x06) |  |
| `LEADER_CHANGED` | 7 (0x07) |  |
| `DISBANDED` | 8 (0x08) |  |
| `TABARD_CHANGED` | 9 (0x09) |  |
| `UNK1` | 10 (0x0A) | string EVENT_GUILD_ROSTER_UPDATE tab content change? |
| `UNK2` | 11 (0x0B) | EVENT_GUILD_ROSTER_UPDATE |
| `SIGNED_ON` | 12 (0x0C) | ERR_FRIEND_ONLINE_SS |
| `SIGNED_OFF` | 13 (0x0D) | ERR_FRIEND_OFFLINE_S |
| `GUILD_BANK_BAG_SLOTS_CHANGED` | 14 (0x0E) | EVENT_GUILDBANKBAGSLOTS_CHANGED |
| `BANKTAB_PURCHASED` | 15 (0x0F) | EVENT_GUILDBANK_UPDATE_TABS |
| `UNK5` | 16 (0x10) | EVENT_GUILDBANK_UPDATE_TABS |
| `GUILD_BANK_UPDATE_MONEY` | 17 (0x11) | EVENT_GUILDBANK_UPDATE_MONEY. string 0000000000002710 is 1 gold |
| `GUILD_BANK_MONEY_WITHDRAWN` | 18 (0x12) | MSG_GUILD_BANK_MONEY_WITHDRAWN |
| `GUILD_BANK_TEXT_CHANGED` | 19 (0x13) | EVENT_GUILDBANK_TEXT_CHANGED |

Used in:
* [GuildLogEvent](guildlogevent.md)
* [SMSG_GUILD_EVENT](smsg_guild_event.md)
