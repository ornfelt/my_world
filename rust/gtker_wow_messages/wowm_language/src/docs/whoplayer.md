# WhoPlayer

## Client Version 1.12

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/social/smsg_who.wowm:1`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/social/smsg_who.wowm#L1).
```rust,ignore
struct WhoPlayer {
    CString name;
    CString guild;
    Level32 level;
    Class class;
    Race race;
    Area area;
    u32 party_status;
}
```
### Body

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| 0x00 | - / - | CString | name |  |
| - | - / - | CString | guild |  |
| - | 4 / Little | Level32 | level |  |
| - | 1 / - | [Class](class.md) | class |  |
| - | 1 / - | [Race](race.md) | race |  |
| - | 4 / - | [Area](area.md) | area |  |
| - | 4 / Little | u32 | party_status |  |


Used in:
* [SMSG_WHO](smsg_who.md)

## Client Version 2.4.3

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/social/smsg_who.wowm:13`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/social/smsg_who.wowm#L13).
```rust,ignore
struct WhoPlayer {
    CString name;
    CString guild;
    Level32 level;
    Class class;
    Race race;
    Gender gender;
    Area area;
}
```
### Body

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| 0x00 | - / - | CString | name |  |
| - | - / - | CString | guild |  |
| - | 4 / Little | Level32 | level |  |
| - | 1 / - | [Class](class.md) | class |  |
| - | 1 / - | [Race](race.md) | race |  |
| - | 1 / - | [Gender](gender.md) | gender |  |
| - | 4 / - | [Area](area.md) | area |  |


Used in:
* [SMSG_WHO](smsg_who.md)

## Client Version 3.3.5

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/social/smsg_who.wowm:13`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/social/smsg_who.wowm#L13).
```rust,ignore
struct WhoPlayer {
    CString name;
    CString guild;
    Level32 level;
    Class class;
    Race race;
    Gender gender;
    Area area;
}
```
### Body

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| 0x00 | - / - | CString | name |  |
| - | - / - | CString | guild |  |
| - | 4 / Little | Level32 | level |  |
| - | 1 / - | [Class](class.md) | class |  |
| - | 1 / - | [Race](race.md) | race |  |
| - | 1 / - | [Gender](gender.md) | gender |  |
| - | 4 / - | [Area](area.md) | area |  |


Used in:
* [SMSG_WHO](smsg_who.md)
