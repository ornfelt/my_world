# SMSG_AUCTION_OWNER_NOTIFICATION

## Client Version 1, Client Version 2

vmangos/cmangos/mangoszero: this message causes on client to display: 'Your auction sold'

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/auction/smsg/smsg_auction_owner_notification.wowm:2`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/auction/smsg/smsg_auction_owner_notification.wowm#L2).
```rust,ignore
smsg SMSG_AUCTION_OWNER_NOTIFICATION = 0x025F {
    u32 auction_id;
    u32 bid;
    u32 auction_out_bid;
    Guid bidder;
    Item item;
    u32 item_random_property_id;
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
| 0x04 | 4 / Little | u32 | auction_id |  |
| 0x08 | 4 / Little | u32 | bid | vmangos/cmangos/mangoszero: if 0, client shows ERR_AUCTION_EXPIRED_S, else ERR_AUCTION_SOLD_S (works only when guid==0) |
| 0x0C | 4 / Little | u32 | auction_out_bid |  |
| 0x10 | 8 / Little | [Guid](../types/packed-guid.md) | bidder |  |
| 0x18 | 4 / Little | Item | item |  |
| 0x1C | 4 / Little | u32 | item_random_property_id |  |

## Client Version 3.3.5

vmangos/cmangos/mangoszero: this message causes on client to display: 'Your auction sold'

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/auction/smsg/smsg_auction_owner_notification.wowm:15`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/auction/smsg/smsg_auction_owner_notification.wowm#L15).
```rust,ignore
smsg SMSG_AUCTION_OWNER_NOTIFICATION = 0x025F {
    u32 auction_id;
    u32 bid;
    u32 auction_out_bid;
    Guid bidder;
    Item item;
    u32 item_random_property_id;
    f32 time_left;
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
| 0x04 | 4 / Little | u32 | auction_id |  |
| 0x08 | 4 / Little | u32 | bid | vmangos/cmangos/mangoszero: if 0, client shows ERR_AUCTION_EXPIRED_S, else ERR_AUCTION_SOLD_S (works only when guid==0) |
| 0x0C | 4 / Little | u32 | auction_out_bid |  |
| 0x10 | 8 / Little | [Guid](../types/packed-guid.md) | bidder |  |
| 0x18 | 4 / Little | Item | item |  |
| 0x1C | 4 / Little | u32 | item_random_property_id |  |
| 0x20 | 4 / Little | f32 | time_left |  |
