# SMSG_GMTICKET_GETTICKET

## Client Version 1.12

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/gamemaster/smsg_gmticket_getticket.wowm:12`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/gamemaster/smsg_gmticket_getticket.wowm#L12).
```rust,ignore
smsg SMSG_GMTICKET_GETTICKET = 0x0212 {
    GmTicketStatus status;
    if (status == HAS_TEXT) {
        CString text;
        GmTicketType ticket_type;
        f32 days_since_ticket_creation;
        f32 days_since_oldest_ticket_creation;
        f32 days_since_last_updated;
        GmTicketEscalationStatus escalation_status;
        Bool read_by_gm;
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
| 0x04 | 4 / - | [GmTicketStatus](gmticketstatus.md) | status |  |

If status is equal to `HAS_TEXT`:

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| 0x08 | - / - | CString | text | cmangos: Ticket text: data, should never exceed 1999 bytes |
| - | 1 / - | [GmTicketType](gmtickettype.md) | ticket_type |  |
| - | 4 / Little | f32 | days_since_ticket_creation |  |
| - | 4 / Little | f32 | days_since_oldest_ticket_creation |  |
| - | 4 / Little | f32 | days_since_last_updated |  |
| - | 1 / - | [GmTicketEscalationStatus](gmticketescalationstatus.md) | escalation_status |  |
| - | 1 / - | Bool | read_by_gm |  |

## Client Version 2.4.3

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/gamemaster/smsg_gmticket_getticket.wowm:12`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/gamemaster/smsg_gmticket_getticket.wowm#L12).
```rust,ignore
smsg SMSG_GMTICKET_GETTICKET = 0x0212 {
    GmTicketStatus status;
    if (status == HAS_TEXT) {
        CString text;
        GmTicketType ticket_type;
        f32 days_since_ticket_creation;
        f32 days_since_oldest_ticket_creation;
        f32 days_since_last_updated;
        GmTicketEscalationStatus escalation_status;
        Bool read_by_gm;
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
| 0x04 | 4 / - | [GmTicketStatus](gmticketstatus.md) | status |  |

If status is equal to `HAS_TEXT`:

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| 0x08 | - / - | CString | text | cmangos: Ticket text: data, should never exceed 1999 bytes |
| - | 1 / - | [GmTicketType](gmtickettype.md) | ticket_type |  |
| - | 4 / Little | f32 | days_since_ticket_creation |  |
| - | 4 / Little | f32 | days_since_oldest_ticket_creation |  |
| - | 4 / Little | f32 | days_since_last_updated |  |
| - | 1 / - | [GmTicketEscalationStatus](gmticketescalationstatus.md) | escalation_status |  |
| - | 1 / - | Bool | read_by_gm |  |

## Client Version 3

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/gamemaster/smsg_gmticket_getticket.wowm:28`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/gamemaster/smsg_gmticket_getticket.wowm#L28).
```rust,ignore
smsg SMSG_GMTICKET_GETTICKET = 0x0212 {
    GmTicketStatus status;
    if (status == HAS_TEXT) {
        u32 id;
        CString text;
        Bool need_more_help;
        f32 days_since_ticket_creation;
        f32 days_since_oldest_ticket_creation;
        f32 days_since_last_updated;
        GmTicketEscalationStatus escalation_status;
        Bool read_by_gm;
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
| - | 4 / - | [GmTicketStatus](gmticketstatus.md) | status |  |

If status is equal to `HAS_TEXT`:

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| - | 4 / Little | u32 | id |  |
| - | - / - | CString | text | cmangos: Ticket text: data, should never exceed 1999 bytes |
| - | 1 / - | Bool | need_more_help |  |
| - | 4 / Little | f32 | days_since_ticket_creation |  |
| - | 4 / Little | f32 | days_since_oldest_ticket_creation |  |
| - | 4 / Little | f32 | days_since_last_updated |  |
| - | 1 / - | [GmTicketEscalationStatus](gmticketescalationstatus.md) | escalation_status |  |
| - | 1 / - | Bool | read_by_gm |  |
