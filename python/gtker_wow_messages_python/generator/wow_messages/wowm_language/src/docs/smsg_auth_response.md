# SMSG_AUTH_RESPONSE

## Client Version 1

Response to [CMSG_AUTH_SESSION](./cmsg_auth_session.md).

Usually followed by [CMSG_CHAR_ENUM](./cmsg_char_enum.md) if login was successful (`AUTH_OK`).

vmangos/cmangos/mangoszero all have a variant of this message that contains fields from `AUTH_OK` for `AUTH_WAIT_QUEUE` as well (`https://github.com/vmangos/core/blob/cd896d43712ceafecdbd8f005846d7f676e55b4f/src/game/World.cpp#L322`) but this does not seem to be actually be a real thing.

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/character_screen/smsg_auth_response.wowm:4`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/character_screen/smsg_auth_response.wowm#L4).
```rust,ignore
smsg SMSG_AUTH_RESPONSE = 0x01EE {
    WorldResult result;
    if (result == AUTH_OK) {
        u32 billing_time;
        u8 billing_flags;
        u32 billing_rested;
    }
    else if (result == AUTH_WAIT_QUEUE) {
        u32 queue_position;
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
| 0x04 | 1 / - | [WorldResult](worldresult.md) | result |  |

If result is equal to `AUTH_OK`:

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| 0x05 | 4 / Little | u32 | billing_time |  |
| 0x09 | 1 / - | u8 | billing_flags |  |
| 0x0A | 4 / Little | u32 | billing_rested |  |

Else If result is equal to `AUTH_WAIT_QUEUE`:

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| 0x0E | 4 / Little | u32 | queue_position |  |

### Examples

#### Example 1

##### Comment

Authentication failed.

```c
0, 3, // size
238, 1, // opcode (494)
13, // result: WorldResult AUTH_FAILED (0x0D)
```
#### Example 2

##### Comment

Client told to wait in queue.

```c
0, 7, // size
238, 1, // opcode (494)
27, // result: WorldResult AUTH_WAIT_QUEUE (0x1B)
239, 190, 173, 222, // queue_position: u32
```
#### Example 3

##### Comment

Client can join.

```c
0, 12, // size
238, 1, // opcode (494)
12, // result: WorldResult AUTH_OK (0x0C)
239, 190, 173, 222, // billing_time: u32
0, // billing_flags: u8
0, 0, 0, 0, // billing_rested: u32
```
## Client Version 2.4.3

Response to [CMSG_AUTH_SESSION](./cmsg_auth_session.md).

Usually followed by [CMSG_CHAR_ENUM](./cmsg_char_enum.md) if login was successful (`AUTH_OK`).

vmangos/cmangos/mangoszero all have a variant of this message that contains fields from `AUTH_OK` for `AUTH_WAIT_QUEUE` as well (`https://github.com/vmangos/core/blob/cd896d43712ceafecdbd8f005846d7f676e55b4f/src/game/World.cpp#L322`) but this does not seem to be actually be a real thing.

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/character_screen/smsg_auth_response.wowm:84`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/character_screen/smsg_auth_response.wowm#L84).
```rust,ignore
smsg SMSG_AUTH_RESPONSE = 0x01EE {
    WorldResult result;
    if (result == AUTH_OK) {
        u32 billing_time;
        BillingPlanFlags billing_flags;
        u32 billing_rested;
        Expansion expansion;
    }
    else if (result == AUTH_WAIT_QUEUE) {
        u32 queue_position;
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
| 0x04 | 1 / - | [WorldResult](worldresult.md) | result |  |

If result is equal to `AUTH_OK`:

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| 0x05 | 4 / Little | u32 | billing_time |  |
| 0x09 | 1 / - | [BillingPlanFlags](billingplanflags.md) | billing_flags |  |
| 0x0A | 4 / Little | u32 | billing_rested |  |
| 0x0E | 1 / - | [Expansion](expansion.md) | expansion |  |

Else If result is equal to `AUTH_WAIT_QUEUE`:

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| 0x0F | 4 / Little | u32 | queue_position |  |

## Client Version 3.3.5

Response to [CMSG_AUTH_SESSION](./cmsg_auth_session.md).

Usually followed by [CMSG_CHAR_ENUM](./cmsg_char_enum.md) if login was successful (`AUTH_OK`).

vmangos/cmangos/mangoszero all have a variant of this message that contains fields from `AUTH_OK` for `AUTH_WAIT_QUEUE` as well (`https://github.com/vmangos/core/blob/cd896d43712ceafecdbd8f005846d7f676e55b4f/src/game/World.cpp#L322`) but this does not seem to be actually be a real thing.

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/character_screen/smsg_auth_response.wowm:110`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/character_screen/smsg_auth_response.wowm#L110).
```rust,ignore
smsg SMSG_AUTH_RESPONSE = 0x01EE {
    WorldResult result;
    if (result == AUTH_OK) {
        u32 billing_time;
        BillingPlanFlags billing_flags;
        u32 billing_rested;
        Expansion expansion;
    }
    else if (result == AUTH_WAIT_QUEUE) {
        u32 queue_position;
        Bool realm_has_free_character_migration;
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
| - | 1 / - | [WorldResult](worldresult.md) | result |  |

If result is equal to `AUTH_OK`:

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| - | 4 / Little | u32 | billing_time |  |
| - | 1 / - | [BillingPlanFlags](billingplanflags.md) | billing_flags |  |
| - | 4 / Little | u32 | billing_rested |  |
| - | 1 / - | [Expansion](expansion.md) | expansion |  |

Else If result is equal to `AUTH_WAIT_QUEUE`:

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| - | 4 / Little | u32 | queue_position |  |
| - | 1 / - | Bool | realm_has_free_character_migration |  |
