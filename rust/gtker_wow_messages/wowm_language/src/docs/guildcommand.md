# GuildCommand

## Client Version 1, Client Version 2

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/guild/smsg_guild_command_result.wowm:1`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/guild/smsg_guild_command_result.wowm#L1).

```rust,ignore
enum GuildCommand : u8 {
    CREATE = 0x00;
    INVITE = 0x01;
    QUIT = 0x03;
    FOUNDER = 0x0E;
    UNKNOWN19 = 0x13;
    UNKNOWN20 = 0x14;
}
```
### Type
The basic type is `u8`, a 1 byte (8 bit) integer.
### Enumerators
| Enumerator | Value  | Comment |
| --------- | -------- | ------- |
| `CREATE` | 0 (0x00) |  |
| `INVITE` | 1 (0x01) |  |
| `QUIT` | 3 (0x03) |  |
| `FOUNDER` | 14 (0x0E) |  |
| `UNKNOWN19` | 19 (0x13) | cmangos claims this triggers UI event EVENT_GUILD_ROSTER_UPDATE |
| `UNKNOWN20` | 20 (0x14) | cmangos claims this triggers UI event EVENT_GUILD_ROSTER_UPDATE |

Used in:
* [SMSG_GUILD_COMMAND_RESULT](smsg_guild_command_result.md)
* [SMSG_GUILD_COMMAND_RESULT](smsg_guild_command_result.md)

## Client Version 3.3.5

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/guild/smsg_guild_command_result.wowm:14`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/guild/smsg_guild_command_result.wowm#L14).

```rust,ignore
enum GuildCommand : u8 {
    CREATE = 0x00;
    INVITE = 0x01;
    QUIT = 0x02;
    PROMOTE = 0x03;
    FOUNDER = 0x0C;
    MEMBER = 0x0D;
    PUBLIC_NOTE_CHANGED = 0x13;
    OFFICER_NOTE_CHANGED = 0x14;
}
```
### Type
The basic type is `u8`, a 1 byte (8 bit) integer.
### Enumerators
| Enumerator | Value  | Comment |
| --------- | -------- | ------- |
| `CREATE` | 0 (0x00) |  |
| `INVITE` | 1 (0x01) |  |
| `QUIT` | 2 (0x02) |  |
| `PROMOTE` | 3 (0x03) |  |
| `FOUNDER` | 12 (0x0C) |  |
| `MEMBER` | 13 (0x0D) |  |
| `PUBLIC_NOTE_CHANGED` | 19 (0x13) |  |
| `OFFICER_NOTE_CHANGED` | 20 (0x14) |  |

Used in:
* [SMSG_GUILD_COMMAND_RESULT](smsg_guild_command_result.md)
